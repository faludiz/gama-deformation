/* GNU Gama -- adjustment of geodetic networks
   Copyright (C) 1999, 2006, 2012, 2014, 2015, 2017, 2020, 2023
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


// LocalNetwork - Network Informations class
// =========================================


#ifndef gama_local_LocalNetwork_h
#define gama_local_LocalNetwork_h

#include <fstream>
#include <iomanip>
#include <list>
#include <gnu_gama/exception.h>
#include <gnu_gama/local/gamadata.h>
#include <gnu_gama/adj/adj_basefull.h>
#include <gnu_gama/adj/adj_basesparse.h>
#include <gnu_gama/local/cluster.h>
#include <gnu_gama/local/local_revision.h>
#include <gnu_gama/adj/adj.h>

namespace GNU_gama { namespace local
{

  class LocalNetwork
  {
    using MVE = GNU_gama::Exception::matvec;
    typedef std::vector<GNU_gama::local::Observation*>       RevisedObsList;
    typedef GNU_gama::AdjBase<double,     int, MVE>          AdjBase;
    typedef GNU_gama::AdjBaseFull<double, int, MVE>          AdjBaseFull;
    typedef GNU_gama::AdjBaseSparse<double, int, MVE,
                                    GNU_gama::AdjInputData>  AdjBaseSparse;

    AdjBase                *least_squares {nullptr};
    GNU_gama::AdjInputData  input;

    struct ellipse_par
    {
        double a, b, alfa;
    };
    std::map<PointID, ellipse_par> stashed_ellipses;

  public:

    LocalNetwork();
    virtual ~LocalNetwork();

    PointData        PD;      // point list
    ObservationData  OD;      // observation list

    std::string   description;

    const GNU_gama::AdjInputData* getAdjInputData() const;


    // ...  information on points removed from adjustment  .................

    enum rm_points {
      rm_missing_xyz, rm_missing_xy,   rm_missing_z,
      rm_singular_xy, rm_singular_z,
      rm_huge_cov_xyz, rm_huge_cov_xy, rm_huge_cov_z
    };

    GNU_gama::local::PointIDList  removed_points;
    std::list<rm_points>  removed_code;

    void removed(const PointID& id, rm_points rm)
    {
      removed_points.push_back(id);
      removed_code  .push_back(rm);
      update(Points);
    }


    // ...  network configuration updates  .................................

    void update_points()       { update(Points);       }
    void update_observations() { update(Observations); }
    void update_residuals()    { update(Residuals);    }
    void update_adjustment()   { update(Adjustment);   }


    // ...  revision of points and observations  ...........................

    void  revision_points();
    const PointIDList& undefined_coordinates() const
    {
      return undefined_xy_z_;
    }
    int points_count()
    {
      revision_points(); return pocbod_;
    }
    void  revision_observations();
    const ObservationList& rejected_observations_count() const
    {
      return removed_obs;
    }
    int observations_count() const
    {
      return pocmer_;
    }
    void project_equations();
    void project_equations(std::ostream&);
    void project_equations(Mat& A, Vec& b, Vec& w);
    double conf_int_coef();
    int min_n() const
    {
      return min_n_;
    }


    // ...  unknowns  ......................................................

    PointID     unknown_pointid   (int i) const { return unknowns_[i-1].pid;  }
    char        unknown_type      (int i) const { return unknowns_[i-1].type; }
    StandPoint* unknown_standpoint(int i) const { return unknowns_[i-1].ori; }
    double      unknown_stdev     (int i)
    {
      using namespace std;
      return m_0()*sqrt(least_squares->q_xx(i, i));
    }


    // ...  observations  ..................................................

    Observation* ptr_obs(int i)
    {
      return RSM[i-1];
    }
    double weight_obs(int i)
    {
      double p = m_0_apr_/RSM[i-1]->stdDev();
      return p*p;
    }
    double test_abs_term(int i);                   // 0 or abs_term(i)
    bool huge_abs_terms()
    {
      project_equations(); return vybocujici_abscl_;
    }
    void remove_huge_abs_terms();
    double rhs(int i) const
    {
      return rhs_(i);
    }


    // ...  adjustment  ....................................................

    bool is_adjusted() const
    {
      return tst_vyrovnani_;
    }
    bool has_stashed_ellipses() const
    {
      return !stashed_ellipses.empty();
    }
    const Vec& solve()
    {
      vyrovnani_();
      return least_squares->unknowns();
    }
    const Vec& residuals()
    {
      vyrovnani_();
      return r;
    }
    int unknowns_count()
    {
      project_equations();
      return A.cols();
    }
    int observations_count()
    {
      project_equations();
      return A.rows();
    }
    int degrees_of_freedom()
    {
      vyrovnani_();
      return A.rows() - A.cols() + least_squares->defect();
    }
    int null_space();

    double trans_VWV()           { vyrovnani_(); return suma_pvv_; }
    double m_0();
    double apriori_m_0() const   { return m_0_apr_; }

    double qxx(int i, int j) { return least_squares->q_xx(i,j); }
    double qbb(int i, int j) { return least_squares->q_bb(i,j); }
    double qbx(int i, int j) { return least_squares->q_bx(i,j); }

    double cond();
    bool lindep(int i);

    void refine_approx_coordinates();

    void   apriori_m_0(double m)  { m_0_apr_ = m; }
    void   tol_abs(double m)      { tol_abs_ = m; }
    double tol_abs() const        { return tol_abs_; }

    double stdev_obs(int i) { return sigma_L(i); }
    double wcoef_res(int i) { return vahkopr(i); }
    double stdev_res(int i)
    {
      using namespace std;
      return m_0()*sqrt(fabs(wcoef_res(i)));
    }

    double studentized_residual(int i)
    {
      return stdev_res(i) > 0 ? residuals()(i)/stdev_res(i) : 0;
    }
    double obs_control(int i)
    {
      /*
       * It is supposed that standard deviation mL of adjusted
       * observation is derived from apriori reference standard
       * deviation m0 (ml is standard deviation of the
       * observation). F. Charamza: Geodet/PC, Zdiby 1990
       * p. 180.
       *
       *      f = 100*(ml - mL)/ml
       *
       *      f < 0.1%     uncontrolled observation
       *      f < 5%       weakly controlled observation
       */
      // 1.1.20 return 100*fabs((1-sqrt(q_bb(i,i)*w(i))));
      using namespace std;
      return 100*fabs(1-sqrt(least_squares->q_bb(i,i)));
    }
    void std_error_ellipse(const PointID&, double& a,
                           double& b, double& alfa);
    void stash_std_error_ellipse(
        const PointID&, double a, double b, double alfa);


    // ...  parameters of statistic analysis  ...............................

    bool m_0_apriori    () const { return typ_m_0_ == apriorni_;  }
    bool m_0_aposteriori() const { return typ_m_0_ == empiricka_; }
    double m_0_aposteriori_value();

    void set_m_0_apriori    ()   { typ_m_0_ = apriorni_; }
    void set_m_0_aposteriori()   { typ_m_0_ = empiricka_; }

    double conf_pr() const       { return konf_pr_; }
    void   conf_pr(double p);


    // ...  formated output  ...............................................

    int maxw_id () const { return 12; } // max width of point id.
    int maxw_unk() const { return  3; } // max width of index of unknown
    int maxw_obs() const { return  4; } // max width of index of observation

    bool gons()    const { return  gons_; }
    bool degrees() const { return !gons_; }
    void set_gons()      { gons_ = true;  Observation::gons = true;  }
    void set_degrees()   { gons_ = false; Observation::gons = false; }

    // sign 0  conversion without sign
    //      1  sign left-padded
    //      2  sign right-padded
    //      3  signed with leading spaces trimmed
    std::string angular_fmt(Observation* obs, int sign=0, int prec=2);

    // ...  connected network  .............................................

    bool connected_network() const { return design_matrix_graph_is_connected; }

    std::string algorithm() const;
    bool        has_algorithm() const;
    void        set_algorithm(std::string alg = std::string());
    int         adj_covband() const;
    void        set_adj_covband(int value=-1);
    double      epoch() const;
    bool        has_epoch() const;
    void        set_epoch(double=0.0);
    double      latitude() const;
    bool        has_latitude() const;
    void        set_latitude(double);
    std::string ellipsoid() const;
    bool        has_ellipsoid() const;
    void        set_ellipsoid(std::string="wgs84");
    bool        correction_to_ellipsoid() const;
    void        clear_nullable_data();

    // ... linearization iterations ........................................

    void set_max_linearization_iterations(int value=5);
    int  max_linearization_iterations() const;
    int  clear_linearization_iterations() { return (iterations_ = 0); }
    int  linearization_iterations() const { return iterations_; }
    int  increment_linearization_iterations() { return ++iterations_; }
    bool next_linearization_iterations() const { return iterations_ < max_linearization_iterations_;}

    bool refine_adjustment();

    std::string export_xml(std::string version=std::string());

    // ... verbose output ..................................................

    void set_verbose(bool val=true) { verbose_ = val; }
    bool verbose() const { return verbose_; };

    // #####################################################################

    bool   consistent() const;
    double y_sign() const;
    void   remove_inconsistency();
    void   return_inconsistency();

  private:

    void updated_xml_covmat(std::string& xml, const CovMat& C, bool always);

    int         adj_covband_;         // output XML xyz cov bandWidth
    int         max_linearization_iterations_;
    int         iterations_ {};
    std::string algorithm_;           // algorithm name or empty string
    bool        has_algorithm_;
    double      epoch_;
    bool        has_epoch_;
    double      latitude_;
    bool        has_latitude_;
    std::string ellipsoid_;
    bool        has_ellipsoid_;

    RevisedObsList          RSM;

    PointIDList undefined_xy_z_;      // revision of points
    int pocbod_;
    bool tst_redbod_;

    ObservationList removed_obs;      // revision of observations
    int pocmer_;
    bool tst_redmer_;

    // parameters of statistical analysis

    double m_0_apr_;         // a priori reference standard deviation
    double konf_pr_;         // (confidence) probability
    double tol_abs_;         // tollerance for testing absolute terms
    enum ApEm_ { apriorni_, empiricka_ };
    ApEm_ typ_m_0_;          // type of reference standard deviation

    bool tst_rov_opr_;       // project equations
    bool vybocujici_abscl_;  // outlying abs. terms in project equations

    int pocet_neznamych_;

    enum Update { Points, Observations, Residuals, Adjustment };
    void update(Update);

    struct Unknown {
      PointID     pid;
      char        type;      // 'X', 'Y', 'Z', 'R'
      StandPoint* ori;       // zero or pointer for orientation (type='R')
    };

    std::vector<Unknown> unknowns_;  // list of unknowns

    Mat A;
    Vec b;
    Vec rhs_;             // right-hand side
    Vec r;
    Vec sigma_L;          // standard deviation of adjusted observation
    Vec vahkopr;          // weight coefficient of residuals
    double suma_pvv_;
    GNU_gama::SparseMatrix<double, int>*  Asp;

    bool design_matrix_graph_is_connected;

    // solution of Least Squares

    bool tst_vyrovnani_;
    void vyrovnani_();

    int  min_n_;           // regularization of free network
    int* min_x_;

    bool   gons_;

    // preparation for design matrix

    /* ######################################################################
     * Functions choldec() and forwardSubstitution were identical in Adj and
     * LocalNetwork classes. They were declared static in Adj class and
     * commented out in LocalNetwork in version 2.08.
     * ######################################################################
    void cholesky(CovMat& chol);           // renamed to choldec()
    void forwardSubstitution(const CovMat& chol, Vec& v);
    */

    // void backwardSubstitution(const Cov& chol, Vec& v);
    void prepareProjectEquations();
    bool singular_coords(const Mat&);

    // fixing inconsitent systems

    void change_y_signs_for_inconsistent_system_();
    bool removed_inconsistency_ {false};

    bool verbose_ { false };


  };     /* class LocalNetwork */

}}         /* namespace GNU_gama::local */

#endif
