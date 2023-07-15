/* GNU Gama -- adjustment of geodetic networks
    Copyright (C) 1999, 2006, 2010  Ales Cepek <cepek@fsv.cvut.cz>
                  2011  Vaclav Petras <wenzeslaus@gmail.com>
                  2012, 2013, 2014, 2015, 2018, 2019, 2020, 2021, 2023
                  Ales Cepek <cepek@gnu.org>

   This file is part of the GNU Gama C++ library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/** \file network.cpp
 * \brief #GNU_gama::local::LocalNetwork class implementation
 *
 * \author Ales Cepek
 * \author Vaclav Petras (acyclic visitor pattern)
 */

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <memory>
#include <set>

#include <gnu_gama/local/network.h>
#include <gnu_gama/local/local_linearization.h>
#include <gnu_gama/local/test_linearization_visitor.h>
#include <gnu_gama/local/itstream.h>
#include <gnu_gama/local/skipcomm.h>
#include <gnu_gama/statan.h>
#include <gnu_gama/sparse/smatrix_graph.h>
#include <gnu_gama/version.h>
#include <gnu_gama/ellipsoids.h>
#include <gnu_gama/gon2deg.h>

using namespace std;
using namespace GNU_gama::local;

typedef std::list<GNU_gama::Cluster<Observation>*> ClusterList;
typedef GNU_gama::Cluster<Observation>             Cluster_;  // 1.10

namespace
{
  const char* UNKNOWN_ALGORITHM="### network.cpp : unknown algorithm ###";
}

/** \brief %Visitor for absolute term testing.
 *
 * Visit methods determine absolute term.
 * Computed value is accessible through method value().
 *
 * \note Before first (and probably each) visit
 * setIndex() and setFromTo() should be called.
 *
 * Example:
 * \code
 * // ...
 * TestAbsTermVisitor testVisitor(b, tolerance);
 * // ... get observation
 * // set index and observation from and to
 * testVisitor.setIndex(indm);
 * testVisitor.setFromTo(stan, cil);
 * // do visit
 * m->accept(&testVisitor);
 * // use result value
 * double myValue = testVisitor.value();
 * \endcode
 *
 */
class TestAbsTermVisitor : public GNU_gama::local::AllObservationsVisitor
{
public:
    /**
     *
     * \note Before first (and probably each) visit
     * setIndex() and setFromTo() should be called.
     */
    TestAbsTermVisitor(const Vec& bVector, double tolerance)
        : indm(0), stan(0), cil(0),
          b(bVector), tol_abs_(tolerance),
          val(0), d0(0)
    {
    }

    /** Result value of last visited observation. */
    double value() { return val; }

    /** Sets observation index in vector \a b (given in constructor). */
    void setIndex(int observationIndex)
    {
        indm = observationIndex;
    }

    /** Sets from point and to point. */
    void setFromTo(const LocalPoint& from, const LocalPoint& to)
    {
        stan = &from;
        cil = &to;

        double dx, dy;
        if (stan->test_xy() && cil->test_xy())
            {
                dy = stan->y() - cil->y();
                dx = stan->x() - cil->x();
                d0 = sqrt(dy*dy + dx*dx);
            }
    }

    void visit(Distance* obs)
    {
        check(fabs(obs->value() - d0)*1000);
    }
    void visit(Direction* /*obs*/)
    {
        check(fabs(b(indm)*d0/(10*R2G)));
    }
    void visit(Angle* /*obs*/)
    {
        check(fabs(b(indm)*d0/(10*R2G)));
    }
    void visit(H_Diff* obs)
    {
        const double h0 = cil->z() - stan->z();
        check(fabs(obs->value() - h0)*1000);
    }

    void visit(S_Distance* obs)
    {
        double dz = stan->z() - cil->z();
        double d3 = sqrt(dz*dz + d0*d0);
        check(fabs(d3 - obs->value())*1000);
    }

    void visit(Z_Angle* /*obs*/)
    {
        double dz = stan->z() - cil->z();
        double d3 = sqrt(dz*dz + d0*d0);
        check(fabs(b(indm)*d3/(10*R2G)));
    }

    void visit(X* obs)
    {
        check(fabs(stan->x() - obs->value())*1000);
    }

    void visit(Y* obs)
    {
        check(fabs(stan->y() - obs->value())*1000);
    }
    void visit(Z* obs)
    {
        check(fabs(stan->z() - obs->value())*1000);
    }

    void visit(Xdiff* obs)
    {
        const double dx = cil->x() - stan->x();
        check(fabs(dx - obs->value())*1000);
    }

    void visit(Ydiff* obs)
    {
        const double dy = cil->y() - stan->y();
        check(fabs(dy - obs->value())*1000);
    }

    void visit(Zdiff* obs)
    {
        const double dz = cil->z() - stan->z();
        check(fabs(dz - obs->value())*1000);
    }

    void visit(Azimuth* /*obs*/)
    {
        check(fabs(b(indm)*d0/(10*R2G)));
    }

private:
    int    indm;
    const  LocalPoint* stan;
    const  LocalPoint* cil;
    const  Vec& b;
    double tol_abs_;
    double val;
    double d0;

    void check(double value)
    {
        if (value > tol_abs_)
            val = b(indm);
        else
            val = 0;
    }
};



//##########################################################################



LocalNetwork::LocalNetwork()
  : pocbod_(0), tst_redbod_(false), pocmer_(0), tst_redmer_(false),
    m_0_apr_(10), konf_pr_(0.95), tol_abs_(1000), typ_m_0_(empiricka_),
    tst_rov_opr_(false), tst_vyrovnani_(false), min_n_(0), min_x_(nullptr),
    gons_(true)
{
  least_squares = nullptr;
  Asp = nullptr;

  set_adj_covband();
  set_max_linearization_iterations();
  // epoch_         = 0.0;
  // has_epoch_     = false;
  // latitude_      = M_PI/4.0;
  // has_latitude_  = false;
  // ellipsoid_     = "";
  // has_ellipsoid_ = false;
  clear_nullable_data();

  removed_inconsistency_ = false;
}


LocalNetwork::~LocalNetwork()
{
  delete[] min_x_;
  delete   Asp;
  delete   least_squares;
}


/* Private member 'AdjInputData input' is defined as an automatic variable,
 * this can be changed in future versions.
 *
 * The following method getAdjInputDat() is designed to mimic situation
 * where 'input' is declared as a pointer to dynamically allocated object
 * of type AdjInputData* and getAdjInputData() may return nullptr.
 */
const GNU_gama::AdjInputData* LocalNetwork::getAdjInputData() const
{
  if (input.mat() == nullptr) return nullptr;

  return &input;
}


double LocalNetwork::cond()
{
  return least_squares->cond();
}


bool LocalNetwork::lindep(int i)
{
  return least_squares->lindep(i);
}


std::string LocalNetwork::algorithm() const
{
  return algorithm_;
}


bool LocalNetwork::has_algorithm() const
{
  return has_algorithm_;
}


void LocalNetwork::set_algorithm(std::string alg)
{
  typedef GNU_gama::local::MatVecException   MVE;
  typedef GNU_gama::AdjEnvelope<double, int, MVE> OLS_env;
  typedef GNU_gama::AdjGSO     <double, int, MVE> OLS_gso;
  typedef GNU_gama::AdjSVD     <double, int, MVE> OLS_svd;
  typedef GNU_gama::AdjCholDec <double, int, MVE> OLS_chol;

  AdjBase* adjb;
  if      (alg == "gso" )     adjb = new OLS_gso;
  else if (alg == "svd" )     adjb = new OLS_svd;
  else if (alg == "cholesky") adjb = new OLS_chol;
  else if (alg == "envelope") adjb = new OLS_env;
  else
    {
      alg  = "envelope";
      adjb = new OLS_env;
    }
  algorithm_ = alg;
  has_algorithm_ = true;

  delete least_squares;
  least_squares = adjb;

  update(Points);
}


int LocalNetwork::adj_covband() const
{
  return adj_covband_;
}


void LocalNetwork::set_adj_covband(int val)
{
  if (val < -1) val = -1;
  adj_covband_ = val;
}


int LocalNetwork::max_linearization_iterations() const
{
  return max_linearization_iterations_;
}


void LocalNetwork::set_max_linearization_iterations(int value)
{
  if (value < 0) value = 0;
  max_linearization_iterations_ = value;
}


bool LocalNetwork::refine_adjustment()
{
  clear_linearization_iterations();
  while (next_linearization_iterations())
    {
      bool refine = refine_obsdh_reductions(this);
      if (!refine) refine = TestLinearization(this);
      if (!refine) break;

      increment_linearization_iterations();
      refine_approx_coordinates();
    }

  return linearization_iterations() > 0;
}


double LocalNetwork::epoch() const
{
  return epoch_;
}


bool LocalNetwork::has_epoch() const
{
  return has_epoch_;
}


void LocalNetwork::set_epoch(double val)
{
  has_epoch_ = true;
  epoch_ = val;
}


double LocalNetwork::latitude() const
{
  return latitude_;
}


bool LocalNetwork::has_latitude() const
{
  return has_latitude_;
}


void LocalNetwork::set_latitude(double val)
{
  has_latitude_ = true;
  latitude_ = val;
}


std::string LocalNetwork::ellipsoid() const
{
  return ellipsoid_;
}


bool LocalNetwork::has_ellipsoid() const
{
return has_ellipsoid_;
}

void LocalNetwork::set_ellipsoid(std::string e)
{
  if (GNU_gama::ellipsoid(e.c_str())) ellipsoid_ = e;
  else                                set_ellipsoid();
  has_ellipsoid_ = true;
}


bool LocalNetwork::correction_to_ellipsoid() const
{
  return has_latitude_ || has_ellipsoid_;
}


void LocalNetwork::clear_nullable_data()
{
  has_algorithm_ = has_epoch_ = has_latitude_ = has_ellipsoid_ = false;
}


void LocalNetwork::revision_points()
{
  if (tst_redbod_) return;

  undefined_xy_z_.erase(undefined_xy_z_.begin(), undefined_xy_z_.end());
  pocbod_ = 0;

  for (PointData::iterator bod=PD.begin(); bod!=PD.end(); ++bod)
    {
      bool ok = true;
      LocalPoint& b = (*bod).second;

      b.set_xyz_0();       // store initial values even for unused points

      if (b.active_xy())
        {
          if (b.test_xy())
            {
              pocbod_++;
            }
          else
            {
              b.set_unused_xy();
              removed( (*bod).first, rm_missing_xy );
              ok = false;
            }
        }

      if (b.active_z())
        {
          if (b.test_z())
            {
              if (!b.active_xy() || !b.test_xy()) pocbod_++;
            }
          else
            {
              b.set_unused_z();
              removed( (*bod).first, rm_missing_z );
              ok = false;
            }
        }

      if (!ok) undefined_xy_z_.push_back((*bod).first);
    }

  tst_redbod_ = true;
  update(Observations);
}


void LocalNetwork::revision_observations()
{
  if (!tst_redbod_) revision_points();

  {
    LocalRevision local_rev(PD);
    for (ObservationData::iterator i=OD.begin(), e=OD.end(); i!=e; ++i)
      {
        Observation* m = *i;
        m->accept(&local_rev);
      }
  }

  // test cycle for StandPoint clusters with single direction
  ClusterList& clusters = OD.clusters;
  for (ClusterList::iterator cit=clusters.begin(); cit!=clusters.end(); ++cit)
    {
      if (StandPoint* sp = dynamic_cast<StandPoint*>(*cit))
        {
          // 1.3.08 *** check for directions pointing to the same targer
          std::set<PointID> targets;

          int active_directions = 0;
          for (ObservationList::iterator i=sp->observation_list.begin();
               i != sp->observation_list.end(); ++i)
            {
              if (const Direction* d = dynamic_cast<const Direction*>(*i))
                if (d->active())
                  {
                    std::set<PointID>::const_iterator s = targets.find( d->to() );
                    if (s == targets.end())
                      {
                        active_directions++;
                        targets.insert( d->to() );
                      }
                  }
            }

          if (active_directions < 2)
            {
              for (ObservationList::iterator
                     i  = sp->observation_list.begin();
                   i != sp->observation_list.end(); ++i)
                if (Direction* d = dynamic_cast<Direction*>(*i))
                  d->set_passive();
            }
        }
      (*cit)->update();
    }

  RSM.clear();
  // removed_obs.clear();  1.17b
  for (ObservationData::iterator i=OD.begin(), e=OD.end(); i!=e; ++i)
    {
      Observation* m = *i;

      if (m->active()) RSM.push_back(m);
      else             removed_obs.push_back(m);
    }
  pocmer_ = RSM.size();

  tst_redmer_ = true;
  update(Residuals);
}


void LocalNetwork::project_equations()
{
  if (tst_rov_opr_) return;
  if (!tst_redmer_) revision_observations();

  for (PointData::iterator bod=PD.begin(); bod!=PD.end(); ++bod)
    {
      LocalPoint& b = (*bod).second;
      if (b.active_xy() || b.active_z())
        {
          // indexes of unknowns in project equations: 1, 2, ...
          b.index_y() = b.index_x() = b.index_z() = 0;
        }
    }

  ClusterList& clusters = OD.clusters;
  for (ClusterList::iterator cl=clusters.begin(); cl!=clusters.end(); ++cl)
    if (StandPoint* standpoint=dynamic_cast<StandPoint*>(*cl))
      standpoint->index_orientation(0);

  {
    LocalLinearization  loclin(PD, m_0_apr_);

    const int V = pocmer_;               // vectors
    const int M = V * loclin.max_size;   // reserved memory


    GNU_gama::SparseMatrix<double, int>*
       tmp = new GNU_gama::SparseMatrix<double, int>(M, V, 0);

    b.reset(pocmer_);   // initialisation of base class OLS
    rhs_.reset(pocmer_);

    int  r = 0;
    pocet_neznamych_ = 0;
    for (RevisedObsList::iterator m=RSM.begin(); m!=RSM.end(); ++m)
      {
        Observation* obs = *m;
        obs->accept(&loclin);
        b(++r)  = loclin.rhs;
        rhs_(r) = loclin.rhs;
        tmp->new_row();
        for (long i=0; i<loclin.size; i++)
            tmp->add_element(loclin.coeff[i], loclin.index[i]);
    }

    pocet_neznamych_ = loclin.unknowns();
    A.reset(pocmer_, pocet_neznamych_);   // initialization of base class OLS
    A.set_zero();

    delete Asp;
    Asp =  tmp->replicate(tmp->nonzeroes(), pocmer_, pocet_neznamych_ );
    delete tmp;

    {
      GNU_gama::SparseMatrixGraph<double, int> graph(Asp);

      design_matrix_graph_is_connected = graph.connected();
    }

    double* a = Asp->begin(1);
    int*  i = Asp->ibegin(1);
    for (int n, row=1; row<=pocmer_; row++)
      {
        n = Asp->size(row);
        while (n--)
          A(row, *i++) = *a++;
      }
  }

  unknowns_.erase(unknowns_.begin(), unknowns_.end());
  unknowns_.resize(pocet_neznamych_);
  Unknown unknown;

  for (ClusterList::iterator
         clptr=clusters.begin(); clptr!=clusters.end(); ++clptr)
    if (StandPoint* standpoint=dynamic_cast<StandPoint*>(*clptr))
      /*
       * in the following if (...) statement we test index of an orientation
       * to skip stations with single direction (gnu_gama/local-1.1.10)
       */
      if (standpoint->test_orientation() && standpoint->index_orientation())
        {
          const LocalPoint& station = PD[standpoint->station];
          if (station.active_xy())
            {
              unknown.pid  = standpoint->station;
              unknown.ori  = standpoint;
              unknown.type = 'R';
              unknowns_[standpoint->index_orientation()-1] = unknown;
          }
        }

  for (PointData::const_iterator i=PD.begin(); i!=PD.end(); ++i)
    {
      const LocalPoint& b = (*i).second;

      if (b.active_xy() && b.index_y())
        {
          unknown.pid = (*i).first;
          unknown.ori =  0;
          unknown.type = 'X';  unknowns_[b.index_x()-1] = unknown;
          unknown.type = 'Y';  unknowns_[b.index_y()-1] = unknown;
        }

      if (b.active_z() && b.index_z())
        {
          unknown.pid = (*i).first;
          unknown.ori =  0;
          unknown.type = 'Z';  unknowns_[b.index_z()-1] = unknown;
        }
    }

  // ...  initialization of base class OLS  .............................

  vybocujici_abscl_ = false;
  {   // for ...
    for (long r=1; r<=pocmer_; r++)
      {
        if (test_abs_term(r)) vybocujici_abscl_ = true;
      }
  }   // for ...

  prepareProjectEquations();  // [A, b] scaled by chol. dec. of weight matrix

  if (singular_coords(A))
    {
      update(Points);
      project_equations();
      return;
    }

  if (AdjBaseFull* full = dynamic_cast<AdjBaseFull*>(least_squares))
    {
      full->reset(A, b);
    }
  else if (AdjBaseSparse* sparse = dynamic_cast<AdjBaseSparse*>(least_squares))
    {
      /*********************************/
      /*  reset AdjInputData structure */
      /*********************************/

      // design matrix

      input.set_mat(Asp);
      Asp = nullptr;

      // ---  cofactors  --------------------------------------------------

      int count = 0;   // number of diagonal blocks
      int msize = 0;   // memory size

      for (ClusterList::const_iterator
             cluster=OD.clusters.begin(); cluster!=OD.clusters.end(); ++cluster)
        if (const int N = (*cluster)->activeObs())
        {
          int W = (*cluster)->covariance_matrix.bandWidth();
          W = std::min(N-1, W);         // 2.16.4 ... added to fix the bug in W
          count++;
          msize += 1+(N+N-W)*(W+1)/2;   // += number of band matrix elements
        }                               // 2.16.4 ... why plus one?

      GNU_gama::BlockDiagonal<> *bd = new GNU_gama::BlockDiagonal<>(count, msize);

      for (ClusterList::const_iterator
             cluster=OD.clusters.begin(); cluster!=OD.clusters.end(); ++cluster)
        if (const int N = (*cluster)->activeObs())
        {
          CovMat C = (*cluster)->activeCov();
          C /= (m_0_apr_*m_0_apr_);        // covariances ==> cofactors
          bd->add_block(N, C.bandWidth(), C.begin());
        }

      input.set_cov(bd);
      bd = 0;

      // ---  right-hand side  --------------------------------------------

      GNU_gama::Vec<> bb(b.dim());
      for (int i=1; i<=b.dim(); i++) bb(i) = rhs_(i);
      input.set_rhs(bb);     // right-hand side before homogenization

      sparse->reset(&input);
    }
  else
    {
      throw GNU_gama::local::Exception(UNKNOWN_ALGORITHM);
    }

  //--ofstream opr("A_scaled.bin", ios_base::trunc);
  //--int m=A.rows();
  //--int n=A.cols();
  //--opr.write((char*)(&m), sizeof(int));
  //--opr.write((char*)(&n), sizeof(int));
  //--opr.write((char*)(A.begin()), sizeof(double)*m*n);

  delete[] min_x_;
  min_x_ = nullptr;
  min_n_ = 0;

  for (PointData::iterator i=PD.begin(); i!=PD.end(); ++i)
    {
      const LocalPoint& p = (*i).second;
      if (p.constrained_xy() && p.index_x())  min_n_ += 2;
      if (p.constrained_z()  && p.index_z())  min_n_ += 1;
    }

  if (min_n_)
    {
      min_x_ = new int[min_n_];
      int n = 0;
      for (PointData::iterator i=PD.begin(); i!=PD.end(); ++i)
        {
          const LocalPoint& p = (*i).second;
          if (p.constrained_xy() && p.index_x())
            {
              min_x_[n++] = p.index_y();
              min_x_[n++] = p.index_x();
            }
          if (p.constrained_z() && p.index_z())
            {
              min_x_[n++] = p.index_z();
            }
        }
    }
  least_squares->min_x(min_n_, min_x_);

  tst_rov_opr_ = true;
  update(Adjustment);
}


bool LocalNetwork::singular_coords(const Mat& A)
{
  bool result = false;

  double a, b, aa, ab, bb, D;          // testing xy submatrix is sufficient
  int  indx, indy, r;

  for (PointData::iterator i=PD.begin(); i!=PD.end(); ++i)
    {
      LocalPoint&  p  = (*i).second;
      if (p.fixed_xy() || !p.active_xy()) continue;

      if(p.index_x() == 0 || p.index_y() == 0)
      {
          result = true;
          p.set_unused_xy();
          removed( (*i).first, rm_singular_xy );
          continue;
      }

      indx = p.index_x();
      indy = p.index_y();

      aa = ab = bb = 0;
      for (r=1; r<=A.rows(); r++)
        {
          a = A(r,indx);
          b = A(r,indy);
          aa += a*a;
          ab += a*b;
          bb += b*b;
        }

      if (bb > aa) std::swap(aa, bb);
      if (aa == 0)
        D = 0;
      else
        D = 1 - std::abs(ab)/std::sqrt(aa*bb);     // 1 - |cos(a,b)|

      if (D < 1e-12)
        {
          result = true;
          p.set_unused_xy();
          removed( (*i).first, rm_singular_xy );
        }

    }

  return result;
}


void LocalNetwork::project_equations(std::ostream& out)
{
  using namespace std;
  project_equations();

  out << "\n" << pocet_neznamych_ << " " << pocmer_ << "\n\n";

  int*    ib;
  int*    ie;
  double* nb;
  double* ne;
  for (int i=1; i<=Asp->rows(); i++)
    {
      out << Asp->size(i) << ' ';     // number of nonzeroes in the i-th row
      ib = Asp->ibegin(i);
      ie = Asp->iend(i);
      while (ib != ie)
        {
          out << *ib << ' ';          // indexes
          ++ib;
        }
      out << endl;

      out << weight_obs(i) << ' ';    // weight
      out << rhs(i) << ' ';           // abs. term

      nb = Asp->begin(i);
      ne = Asp->end(i);
      while (nb != ne)
        {
          out << *nb << ' ';          // coefficients
          ++nb;
        }
      out << endl;
    }
}


void LocalNetwork::project_equations(Mat& A_, Vec& b_, Vec& w_)
{
  project_equations();

  A_.reset(A.rows(), A.cols());
  A_.set_zero();
  b_.reset(A.rows());
  w_.reset(A.rows());

  int*  ib;
  double* nb;
  double* ne;
  for (int i=1; i<=Asp->rows(); i++)
    {
      ib = Asp->ibegin(i);
      nb = Asp-> begin(i);
      ne = Asp-> end(i);
      while (nb != ne)
        {
          A_(i, *ib) = *nb;
          ++nb;
          ++ib;
        }

      b_(i) = rhs(i);
      w_(i) = weight_obs(i);
    }
}


void LocalNetwork::conf_pr(double p)
{
  if (p <= 0 || p >= 1)
    throw GNU_gama::local::Exception(T_LN_undefined_confidence_level);
  konf_pr_ = p;
}


double LocalNetwork::m_0()
{
  if (m_0_apriori())
    return m_0_apr_;
  else if (m_0_aposteriori())
    {
      vyrovnani_();
      const int nadb = degrees_of_freedom();
      if (nadb > 0)
        return sqrt(trans_VWV()/nadb);
      else
        return 0;
    }
  else
    {
      throw GNU_gama::local::Exception(T_LN_undefined_type_of_actual_sigma);
    }
}


double LocalNetwork::m_0_aposteriori_value()
{
  vyrovnani_();
  const int nadb = degrees_of_freedom();
  if (nadb > 0)
    return sqrt(trans_VWV()/nadb);
  else
    return 0;
}


double LocalNetwork::conf_int_coef()
{
  using namespace GNU_gama::local;

  double pravdepodobnost = (1 - konf_pr_)/2;
  if (m_0_apriori())
    return GNU_gama::Normal(pravdepodobnost);
  else if (m_0_aposteriori())
    {
      const int nadb = degrees_of_freedom();
      if ( nadb > 0)
        return GNU_gama::Student(pravdepodobnost, nadb);
      else
        return 0;
    }
  else
    {
      throw GNU_gama::local::Exception(T_LN_undefined_type_of_actual_sigma);
    }
}


void LocalNetwork::update(Update etapa)
{
  switch(etapa)
    {
    default:
    case Points:        tst_redbod_    = false;
    case Observations:  tst_redmer_    = false;
    case Residuals:     tst_rov_opr_   = false;
    case Adjustment:    tst_vyrovnani_ = false;
    }
}

double LocalNetwork::test_abs_term(int indm)
{
  Observation* m = RSM[indm-1];

  const LocalPoint& stan = PD[m->from()];
  const LocalPoint& cil  = PD[m->to()];   // ignoring second angle target here

  TestAbsTermVisitor testVisitor(b, tol_abs_);
  testVisitor.setIndex(indm);
  testVisitor.setFromTo(stan, cil);

  m->accept(&testVisitor);

  return testVisitor.value();
}


void LocalNetwork::remove_huge_abs_terms()
{
  if (!huge_abs_terms()) return;

  int r = 0;
  for (RevisedObsList::iterator m = RSM.begin(); m!=RSM.end(); ++m)
    if (test_abs_term(++r))
      (*m)->set_passive();

  update(Observations);
}


int LocalNetwork::null_space()
{
  try
    {
      vyrovnani_();
    }
  catch(const MatVecException& vs)
    {
      if (vs.error() != GNU_gama::Exception::BadRegularization) throw;

      for (int i=1; i<=unknowns_count(); i++)
        if (lindep(i))
          {
            const char    type = unknown_type(i);
            const PointID id   = unknown_pointid(i);

            LocalPoint& p = PD[id];
            if (type == 'X' || type == 'Y' || type == 'R' )
              {
                p.set_unused_xy();
                removed(id, rm_singular_xy );
              }
            else if (type == 'Z' )
              {
                p.set_unused_z();
                removed(id, rm_missing_z );
              }

            return null_space();
          }
    }

  return least_squares->defect();
}


void LocalNetwork::std_error_ellipse(const PointID& cb,
                                     double& a, double& b, double& alfa)
{
  using namespace std;

  // stashed ellipses are needed only in SVG drawing from adjustment XML
  auto p = stashed_ellipses.find(cb);
  if (p != stashed_ellipses.end())
  {
      a = p->second.a;
      b = p->second.b;
      alfa = p->second.alfa;
      return;
  }

  const LocalPoint& bod = PD[cb];
  int iy = bod.index_y();
  int ix = bod.index_x();
  double cyy = least_squares->q_xx(iy,iy);
  double cyx = least_squares->q_xx(iy,ix);
  double cxx = least_squares->q_xx(ix,ix);
  double c = sqrt((cxx-cyy)*(cxx-cyy) + 4*cyx*cyx);
  b = (cyy+cxx-c)/2;
  if (b < 0) b = 0;

  double m = m_0();
  a = m * sqrt(b+c);
  b = m * sqrt(b);
  if (c == 0) {
     alfa = 0;
     return;
  }
  alfa = atan2(2*cyx, cxx-cyy)/2;
  if (alfa < 0) alfa += M_PI;
}

// 1.7.09 added optional update of constrained coordinates; inspired
// by adjustment of photographic observation by Jim Sutherland

void LocalNetwork::stash_std_error_ellipse(const PointID& cb,
                                           double a, double b, double alfa)
{
  ellipse_par ep;
  ep.a = a; ep.b = b; ep.alfa = alfa;
  stashed_ellipses[cb] = ep;
}

void LocalNetwork::refine_approx_coordinates()
{
  const Vec& x = least_squares->unknowns();

  for (int i=1; i<=unknowns_count(); i++)
    if (unknown_type(i) == 'X')
      {
        const PointID& cb = unknown_pointid(i);
        LocalPoint& b = PD[cb];
        b.set_xy(b.x() + x(i)/1000, b.y() + x(i+1)/1000);
      }
    else if (unknown_type(i) == 'Z')
      {
        const PointID& cb = unknown_pointid(i);
        LocalPoint& b = PD[cb];
        b.set_z(b.z() + x(i)/1000);
      }
    else if (unknown_type(i) == 'R')
      {
        StandPoint* standpoint = unknown_standpoint(i);
        double ori =
          ( standpoint->orientation() )*R2G + x(i)/10000;
        standpoint->set_orientation( ori*G2R );
      }

  update(Residuals);
}


// preparing for project equations - Cholesky decomposition of
// covariance matrix

/* ######################################################################
 * Functions choldec() and forwardSubstitution were identical in Adj and
 * LocalNetwork classes. They were declared static in Adj class and
 * commented out in LocalNetwork in version 2.08.
 * ######################################################################

void LocalNetwork::cholesky(CovMat& chol)
{
  chol.cholDec();

  using namespace std;
  const int N = chol.rows();
  const int b = chol.bandWidth();

  for (int m, j, i=1; i<=N; i++)
    {
      double d = sqrt(chol(i,i));
      chol(i,i) = d;

      m = i+b;  if(N < m) m = N;    // m = min(N, i+b);

      for (j=i+1; j<=m; j++) chol(i,j) *= d;
    }
}

void LocalNetwork::forwardSubstitution(const CovMat& chol, Vec& v)
{
  using namespace std;
  const int N = chol.rows();
  const int b = chol.bandWidth();

  for (int m{}, i=1; i<=N; i++)
    {
      if (i > b+1) m = i - b;
      else         m = 1;
      for (int j=m; j<i; j++) v(i) -= chol(i,j)*v(j);

      v(i) /= chol(i,i);
    }
}
*/


void LocalNetwork::prepareProjectEquations()
{
  int ind_0 = 0;

  for (const auto cluster : OD.clusters)
    if (const int N = cluster->activeObs())
        {
          Vec t(N);
          CovMat C = cluster->activeCov();
          C /= (m_0_apr_*m_0_apr_);        // covariances ==> cofactors
          Adj::choldec(C);                 // cofactors   ==> weights

          for (int j=1; j<=A.cols(); j++)
            {
              bool empty = true;
              for (int k=1; k<=N; k++)
                {
                  double tmp = A(ind_0+k,j);
                  t(k) = tmp;
                  if (tmp) empty = false;
                }
                if (empty) continue;

                Adj::forwardSubstitution(C, t);
                for (int l=1; l<=N; l++) A(ind_0+l,j) = t(l);
            }

          for (int k=1; k<=N; k++) t(k) = b(ind_0+k);
          Adj::forwardSubstitution(C, t);
          for (int l=1; l<=N; l++) b(ind_0+l) = t(l);

          ind_0 += N;
        }
}


void LocalNetwork::vyrovnani_()
{
  using namespace GNU_gama::local;
  if (tst_vyrovnani_) return;

  do {

    project_equations();
    if (unknowns_count()     == 0)
      throw GNU_gama::local::Exception(T_GaMa_No_unknowns_defined);
    if (observations_count() == 0)
      throw GNU_gama::local::Exception(T_GaMa_No_observations_available);
    if (points_count()      == 0)
      throw GNU_gama::local::Exception(T_GaMa_No_points_available);

    tst_vyrovnani_ = true;

    /* ----------------------------------------------------------------- */
    // check for huge covariances / indefinite coordinates

    for (PointData::iterator i=PD.begin(); i!=PD.end(); ++i)
      {
        LocalPoint&     P  = (*i).second;
        const PointID&  id = (*i).first;

        if (!P.free_xy() && !P.free_z()) continue;

        double tx=0, ty=0, tz=0;
        if (int ix = P.index_x()) tx =m_0_apr_*sqrt(least_squares->q_xx(ix,ix));
        if (int iy = P.index_y()) ty =m_0_apr_*sqrt(least_squares->q_xx(iy,iy));
        if (int iz = P.index_z()) tz=m_0_apr_*sqrt(least_squares->q_xx(iz,iz));

        bool bxy = (tx > 1e4) || (ty > 1e4);
        bool bz  = (tz > 1e4);

        if (bxy && bz)
          {
            P.set_unused_xy();
            P.set_unused_z();
            removed(id, rm_huge_cov_xyz);
            update(Points);
          }
        else if (bxy)
          {
            P.set_unused_xy();
            removed(id, rm_huge_cov_xy);
            update(Points);
          }
        else if (bz)
          {
            P.set_unused_z();
            removed(id, rm_huge_cov_z);
            update(Points);
          }
      }

  } while (!tst_vyrovnani_);

  if (AdjBaseFull* full = dynamic_cast<AdjBaseFull*>(least_squares))
  { /* ----------------------------------------------------------------- */
    // r = least_squares->residuals();
    r = full->residuals();
    suma_pvv_ = 0;

    double tmp;
    int    ind_0 = 0;

    for (ClusterList::const_iterator
           cluster=OD.clusters.begin(); cluster!=OD.clusters.end(); ++cluster)
      if (const int N = (*cluster)->activeObs())
        {
          Vec t(N), u(N);
          CovMat C = (*cluster)->activeCov();
          C /= (m_0_apr_*m_0_apr_);
          Adj::choldec(C);

          for (int k=1; k<=N; k++)
            {
              tmp  = r(ind_0+k);
              t(k) = tmp;
              suma_pvv_ += tmp*tmp;
            }
          const CovMat&  CC = C;
          const int b  = CC.bandWidth();
          {
            for (int m, i=1; i<=N; i++)
              {
                double s=0;
                if (i > b+1) m = i - b;
                else         m = 1;
                for (int j=m; j<=i; j++) s += CC(i,j)*t(j);
                u(i) = s;
              }
          }
          for (int l=1; l<=N; l++) r(ind_0+l) = u(l);

          ind_0 += N;
        }
  }
  else if (AdjBaseSparse* sparse = dynamic_cast<AdjBaseSparse*>(least_squares))
    {
      r = sparse->residuals();
      suma_pvv_ = sparse->sum_of_squares();
    }
  else
    {
      throw GNU_gama::local::Exception(UNKNOWN_ALGORITHM);
    }


  { /* ----------------------------------------------------------------- */
    sigma_L.reset(pocmer_);

    double MM = m_0() / m_0_apr_;
    int ind_0 = 0;

    for (ClusterList::const_iterator
           cit=OD.clusters.begin(); cit!=OD.clusters.end(); ++cit)
      if (const int N = (*cit)->activeObs())
        {
          const Cluster_& cluster = *(*cit);
          // ??? if (cluster.covariance_matrix.bandWidth())
          // ???   {
          // ???     // calculation for correlated observations missing !!!
          // ???   }
          // ??? else
            {
              int n = ind_0+1;
              for (ObservationList::const_iterator
                     i = cluster.observation_list.begin();
                     i!= cluster.observation_list.end(); ++i)
                if ((*i)->active())
                  {
                    // sigma_L = m0() * sqrt(least_squares->q_bb(n,n)) / weight_l
                    sigma_L(n) = MM * sqrt(least_squares->q_bb(n,n)) * (*i)->stdDev();
                    n++;
                  }
            }

          ind_0 += N;
        }
  }


  { /* ----------------------------------------------------------------- */
    vahkopr.reset(pocmer_);

    for (int i=1; i<=pocmer_; i++)
      {
        // F.Charamza: Geodet/PC p. 171
        // 1.1.56 double  qv = (1.0 - q_bb(i, i))/w(i);
        double  qv = (1.0 - least_squares->q_bb(i, i))/ weight_obs(i);
        vahkopr(i) = (qv >= 0) ? qv : 0;       // removing noise
      }
  }

}


std::string LocalNetwork::export_xml(std::string version)
{
  std::string xml =
    "<?xml version=\"1.0\" ?>\n";

  if (!version.empty()) xml += version;  // optional xml comment

  xml +=
    "<gama-local xmlns=\"http://www.gnu.org/software/gama/gama-local\">\n"
    "<network axes-xy=";

  switch(PD.local_coordinate_system)
    {
    case LocalCoordinateSystem::CS::EN: xml += "\"en\""; break;
    case LocalCoordinateSystem::CS::NW: xml += "\"nw\""; break;
    case LocalCoordinateSystem::CS::SE: xml += "\"se\""; break;
    case LocalCoordinateSystem::CS::WS: xml += "\"ws\""; break;
    case LocalCoordinateSystem::CS::NE: xml += "\"ne\""; break;
    case LocalCoordinateSystem::CS::SW: xml += "\"sw\""; break;
    case LocalCoordinateSystem::CS::ES: xml += "\"es\""; break;
    case LocalCoordinateSystem::CS::WN: xml += "\"wn\""; break;
    default:
      xml +=  "\"ne\""; break;
    }

  xml += " angles=";
  xml += PD.left_handed_angles() ? "\"left-handed\"" : "\"right-handed\"";
  if (has_epoch()) xml += " epoch=\"" + to_xmlstr(epoch()) + "\"";
  xml += ">\n";


  if (!description.empty())
    {
      std::string descr;
      for (char c : description)
        {
          if (c == '&') descr += "&amp;";
          else descr += c;
        }

      xml += "\n<description>" + descr + "</description>\n";
    }
  xml += "\n<parameters\n";
  xml += "  sigma-apr=\"" + to_xmlstr(apriori_m_0(), 8) + "\"\n";
  xml += "  conf-pr=\""   + to_xmlstr(conf_pr(), 8)     + "\"\n";
  xml += "  tol-abs=\""   + to_xmlstr(tol_abs(), 8)     + "\"\n";
  xml += "  sigma-act=\"";
  xml +=         m_0_apriori() ? "apriori\"\n" : "aposteriori\"\n";
  xml += "  angles=\"" + std::string(gons() ? "400" : "360") + "\"\n";
  if (has_algorithm()) xml += "  algorithm=\"" + algorithm() + "\"\n";
  if (has_latitude())
    xml += "  latitude=\"" + to_xmlstr(latitude()) + "\"\n";
  if (has_ellipsoid()) xml += "  ellipsoid=\"" + ellipsoid() + "\"\n";
  xml += "  cov-band=\"" + to_xmlstr(adj_covband()) + "\"\n";
  // iterations ... not implemented in XML input
  // language .....
  // encoding .....
  xml += "/>\n";


  xml += "\n<points-observations \n";
  // implicit parameters
  // xml += "   distance-stdev=  '5.0' \n";
  // xml += "   direction-stdev= '10.0' \n";
  // xml += "   angle-stdev=     '10.0' \n";
  // xml += "   azimuth-stdev=   '10.0' \n";
  xml += "   >\n\n";


  for (auto p=PD.begin(); p!=PD.end(); ++p)
    {
      PointID    id    = p->first;
      LocalPoint point = p->second;
      if (!point.active()) continue;

      xml += "<point id=\"" + id.str() + "\"";

      if (point.test_xy()) {
        xml += " x=\"" + to_xmlstr(point.x(), 16) + "\"";
        xml += " y=\"" + to_xmlstr(y_sign()*point.y(), 16) + "\"";
      }

      if (point.test_z()) {
          xml += " z=\"" + to_xmlstr(point.z()) + "\"";
      }

      std::string fix {}, adj {};
      if      (point.fixed_xy())       fix += "xy";
      else if (point.constrained_xy()) adj += "XY";
      else if (point.free_xy())        adj += "xy";

      if      (point.fixed_z())        fix += "z";
      else if (point.constrained_z())  adj += "Z";
      else if (point.free_z())         adj += "z";

      if (!fix.empty()) xml += " fix=\"" + fix + "\"";
      if (!adj.empty()) xml += " adj=\"" + adj + "\"";

      xml += " />\n";
    }


  DisplayObservationVisitor info(this);

  for (auto c=OD.clusters.begin(); c!=OD.clusters.end(); ++c) {
    if (auto cluster = dynamic_cast<StandPoint*>(*c))
      {
        auto cluster_from = cluster->station.str();
        xml += "\n<obs";
        if (!cluster_from.empty())
          {
            xml += " from=\"" + cluster->station.str() + "\"";
          }
        xml += ">\n";

        for (auto p  = cluster->observation_list.begin();
                  p != cluster->observation_list.end(); ++p) {
          Observation* obs = *p;
          obs->accept(&info);

          xml += "<" +  info.xml_name;
          if (!info.str_from.empty()) {
              if (cluster_from != info.str_from) {
                  xml += " from=\"" + info.str_from + "\"";
                }
            }
          if (!info.str_to.empty()) {
            xml += " to=\"" + info.str_to + "\"";
            double fdh = obs->from_dh();
            double tdh = obs->to_dh();
            if (fdh) xml += " from_dh=\"" + to_xmlstr(fdh, 8) + "\"";
            if (tdh) xml +=   " to_dh=\"" + to_xmlstr(tdh, 8) + "\"";
          }
          else if (!info.str_bs.empty()) {
            xml += " bs=\"" + info.str_bs + "\"";
            xml += " fs=\"" + info.str_fs + "\"";

            Angle* a = static_cast<Angle*>(obs);
            double rdh = a->from_dh();
            double bdh = a->bs_dh();
            double fdh = a->fs_dh();
            if (rdh) xml += " from_dh=\"" + to_xmlstr(rdh, 8) + "\"";
            if (bdh) xml +=   " bs_dh=\"" + to_xmlstr(bdh, 8) + "\"";
            if (fdh) xml +=   " fs_dh=\"" + to_xmlstr(fdh, 8) + "\"";
          }
          xml += " val=\"" + info.str_val + "\"";
          xml += " stdev=\"" + info.str_stdev + "\"";
          xml += " />\n";
        }

        updated_xml_covmat(xml, cluster->covariance_matrix, false);
         xml += "</obs>\n";
      }
    else if (auto cluster = dynamic_cast<HeightDifferences*>(*c))
      {
        xml += "\n<height-differences>\n";
        for (auto p  = cluster->observation_list.begin();
                  p != cluster->observation_list.end(); ++p) {
          Observation* obs = *p;
          obs->accept(&info);

         xml += "<" +  info.xml_name;
         xml += " from=\"" + info.str_from + "\"";
         xml += " to=\"" + info.str_to + "\"";
         xml += " val=\"" + info.str_val + "\"";

         double dist = 0;
         if (H_Diff* p = dynamic_cast<H_Diff*>(obs)) dist = p->dist();
         if (dist > 0)
           xml += " dist=\"" +to_xmlstr(dist) + "\"";
         else
           xml += " stdev=\"" + info.str_stdev + "\"";
         xml += "/>\n";
        }

        updated_xml_covmat(xml, cluster->covariance_matrix, false);
        xml += "</height-differences>\n";
      }
    else if (auto cluster = dynamic_cast<Coordinates*>(*c))
      {
        xml += "\n<coordinates>\n";
        for (auto p  = cluster->observation_list.begin();
                  p != cluster->observation_list.end(); ++p) {
          Observation* obs = *p;
          obs->accept(&info);
          std::string from = info.str_from;
          xml += "<point id=\"" + from + "\"";
          if (info.xml_name == "x")
            {
              xml += " x=\"" + info.str_val + "\"";
              ++p;
              (*p)->accept(&info);

              xml += " y=\"" + info.str_val + "\"";
              auto q = p;
              ++q;
              if (q != cluster->observation_list.end())
                {
                  DisplayObservationVisitor tmp(this);
                  (*q)->accept(&tmp);
                  if (tmp.str_from == from && tmp.xml_name == "z")
                    {
                      ++p;
                      (*p)->accept(&info);
                    }
                }
            }
          if (info.xml_name == "z")
            {
              xml += " z=\"" + info.str_val + "\"";
            }
          xml += " />\n";
        }

        updated_xml_covmat(xml, cluster->covariance_matrix, true);
        xml += "</coordinates>\n";
      }
    else if (auto cluster = dynamic_cast<Vectors*>(*c))
      {
        xml += "\n<vectors>\n";

        for (auto p  = cluster->observation_list.begin();
                  p != cluster->observation_list.end(); ++p) {
          Observation* obs = *p;
          obs->accept(&info);

          xml += "<vec from=\"" +info.str_from+ "\" to=\"" +info.str_to+ "\"";
          obs->accept(&info);
          xml += " dx=\"" + info.str_val + "\"";
          (*++p)->accept(&info);
          xml += " dy=\"" + info.str_val + "\"";
          (*++p)->accept(&info);
          xml += " dz=\"" + info.str_val + "\"";

          // double fdh = (*p)->from_dh(); ... unrealistic
          // double tdh = (*p)->to_dh();
          // if (fdh) xml += " from_dh=\"" + xml_string(fdh, 8) + "\"";
          // if (tdh) xml +=   " to_dh=\"" + xml_string(tdh, 8) + "\"";
          xml += " />\n";
        }

        updated_xml_covmat(xml, cluster->covariance_matrix, true);
        xml += "\n</vectors>\n";
      }
    else
      {
        xml += "\n### Undefined cluster\n";
      }
  }


  xml +=
    "\n</points-observations>\n"
    "\n</network>\n"
    "</gama-local>\n";

  return xml;
}

void LocalNetwork::updated_xml_covmat(std::string& xml, const CovMat& C,
                                      bool always)
{
  int  dim  = C.dim();
  int  band = C.bandWidth();
  if (!always && band == 0) return;

  xml += "\n<cov-mat dim=\"" + to_string(dim) + "\"";
  xml += " band=\"" + to_string(band) + "\">\n";
  for (int i=1; i<=dim; i++)
    {
      std::ostringstream out;
      out.setf(ios_base::scientific, ios_base::floatfield);
      out.precision(16);
      out << "\n";
      for (int n=1, j=i; j<=i+band && j <=dim; j++, n++)
        {
          out << setw(24) << C(i,j) << ((n%3 == 0 && j != dim) ? "\n" : " ");
        }
      out << "\n";
      xml += out.str();
    }
  xml += "\n</cov-mat>\n";
}

bool LocalNetwork::consistent() const
{
  return PD.left_handed_coordinates() == PD.left_handed_angles();
}

double LocalNetwork::y_sign() const
{
  return consistent() ? +1.0 : -1.0;
}

void LocalNetwork::remove_inconsistency()
{
  if (consistent()) return;
  if (removed_inconsistency_) return;

  change_y_signs_for_inconsistent_system_();
  removed_inconsistency_ = true;
}

void LocalNetwork::return_inconsistency()
{
  if (!removed_inconsistency_) return;

  change_y_signs_for_inconsistent_system_();
  removed_inconsistency_ = false;
}

void LocalNetwork::change_y_signs_for_inconsistent_system_()
{
  for (PointData::iterator ii=PD.begin(); ii!=PD.end(); ++ii)
    {
      LocalPoint& p = (*ii).second;

      if (p.test_xy()) p.set_xy(p.x(), -p.y());
    }

  for (ObservationData::ClusterList::iterator
         ci=OD.clusters.begin(), ei=OD.clusters.end(); ci!=ei; ++ci)
    {
      ObservationData::ClusterType *cluster = *ci;
      for (ObservationList::iterator
             m = cluster->observation_list.begin(),
             e = cluster->observation_list.end()  ; m!=e; ++m)
        {
          Observation *obs = *m;
          bool b = false;

          if      (dynamic_cast<Y*>    (obs))  b = true;
          else if (dynamic_cast<Ydiff*>(obs))  b = true;

          if (b)  obs->set_value( -obs->value() );
        }
    }
}

// sign 0  conversion without sign
//      1  sign left-padded
//      2  sign right-padded
//      3  signed with leading spaces trimmed
std::string LocalNetwork::angular_fmt(Observation* obs, int sign, int prec)
{
  if (!obs->angular()) return std::string();

  double m = R2G*obs->value();
  std::ostringstream ostr;

  if (gons())
    {
      ostr << std::fixed << std::setprecision(4 + prec);
      ostr << m;
    }
  else
    {
      ostr << GNU_gama::gon2deg(m, sign, prec);
    }

  return ostr.str();
}
