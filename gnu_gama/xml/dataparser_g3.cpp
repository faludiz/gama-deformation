/*  
    Geodesy and Mapping C++ Library (GNU GaMa / GaMaLib)
    Copyright (C) 2002  Ales Cepek <cepek@fsv.cvut.cz>

    This file is part of the GNU GaMa / GaMaLib C++ Library.
    
    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
 *  $Id: dataparser_g3.cpp,v 1.3 2004/09/01 11:59:45 cepek Exp $
 */



#include <gnu_gama/xml/dataparser.h>
#include <gnu_gama/gon2deg.h>
#include <gnu_gama/radian.h>
#include <cstring>

using namespace std;
using namespace GNU_gama;

namespace GNU_gama {

  struct DataParser_g3 {

    DataParser_g3() : model(0) 
    {
    }
    ~DataParser_g3()
    {  
      delete model;
    }

    typedef g3::Model::ObservationType::CovarianceMatrix Cov;

    g3::Model*         model;
    g3::ObsCluster*    obs_cluster;
    std::list<Cov>     cov_list;
    double             from_dh;
    double             to_dh;
  };

}


void DataParser::close_g3()
{
  delete g3;
}

void DataParser::init_g3()
{
  g3 = new DataParser_g3;

  optional(g3->from_dh);
  optional(g3->to_dh);


  // .....  <g3-model>  ..............................................

  init(s_gama_data, t_g3_model,
       s_g3_model, 0, 0,
       &DataParser::g3_model, 0, &DataParser::g3_model);

  // .....  <g3-model> <unused | fixed | free | constr>  .............

  init(s_g3_model, t_unused,
       s_g3_param, 0, s_g3_model,
       &DataParser::g3_param_unused, 0, 0);

  init(s_g3_model, t_fixed,
       s_g3_param, 0, s_g3_model,
       &DataParser::g3_param_fixed, 0, 0);

  init(s_g3_model, t_free,
       s_g3_param, 0, s_g3_model,
       &DataParser::g3_param_free, 0, 0);

  init(s_g3_model, t_constr,
       s_g3_param, 0, s_g3_model,
       &DataParser::g3_param_constr, 0, 0);

  init(s_g3_param, t_n,
       s_g3_param_n, 0, 0,
       0, 0, &DataParser::g3_param_n);

  init(s_g3_param, t_e,
       s_g3_param_e, 0, 0,
       0, 0, &DataParser::g3_param_e);

  init(s_g3_param, t_u,
       s_g3_param_u, 0, 0,
       0, 0, &DataParser::g3_param_u);

  // .....  <g3-model> <point>  .....................................

  init(s_g3_model, t_point,
       s_g3_point_1, s_g3_point_2, s_g3_model,
       0, 0, &DataParser::g3_point);

  init(s_g3_point_1, t_id,
       s_g3_point_id, 0, s_g3_point_2,
       0, &DataParser::add_text, &DataParser::g3_point_id);


  init(s_g3_point_2, t_b,
       s_g3_point_b, 0, s_g3_point_after_b,
       0, &DataParser::add_text, &DataParser::g3_point_b);

  init(s_g3_point_after_b, t_l,
       s_g3_point_l, 0, s_g3_point_after_l,
       0, &DataParser::add_text, &DataParser::g3_point_l);

  init(s_g3_point_after_l, t_h,
       s_g3_point_h, 0, s_g3_point_2,
       0, &DataParser::add_text, &DataParser::g3_point_h);

  init(s_g3_point_2, t_x,
       s_g3_point_x, 0, s_g3_point_after_x,
       0, &DataParser::add_text, 0);

  init(s_g3_point_after_x, t_y,
       s_g3_point_y, 0, s_g3_point_after_y,
       0, &DataParser::add_text, 0);

  init(s_g3_point_after_y, t_z,
       s_g3_point_z, 0, s_g3_point_2,
       0, &DataParser::add_text, &DataParser::g3_point_z);

  init(s_g3_point_2, t_height,
       s_g3_point_height, 0, s_g3_point_2,
       0, &DataParser::add_text, &DataParser::g3_point_height);

  init(s_g3_point_2, t_unused,
       s_g3_point_param, 0, s_g3_point_2,
       &DataParser::g3_param_unused, 0, 0);

  init(s_g3_point_2, t_fixed,
       s_g3_point_param, 0, s_g3_point_2,
       &DataParser::g3_param_fixed, 0, 0);

  init(s_g3_point_2, t_free,
       s_g3_point_param, 0, s_g3_point_2,
       &DataParser::g3_param_free, 0, 0);

  init(s_g3_point_2, t_constr,
       s_g3_point_param, 0, s_g3_point_2,
       &DataParser::g3_param_constr, 0, 0);

  init(s_g3_point_param, t_n,
       s_g3_point_param_n, 0, 0,
       0, 0, &DataParser::g3_point_param_n);

  init(s_g3_point_param, t_e,
       s_g3_point_param_e, 0, 0,
       0, 0, &DataParser::g3_point_param_e);

  init(s_g3_point_param, t_u,
       s_g3_point_param_u, 0, 0,
       0, 0, &DataParser::g3_point_param_u);

  // .....  <g3-model> <obs>  ........................................
  
  init(s_g3_model, t_obs,
       s_g3_obs, 0, 0,
       &DataParser::g3_obs, 0, &DataParser::g3_obs);
         
  // .....  <g3-model> <obs> <cov-mat>  ..............................

  init(s_g3_obs, t_covmat,
       s_g3_obs_covmat, s_g3_obs_covmat_after_band, 0,
       0, 0, &DataParser::g3_obs_cov);

  init(s_g3_obs_covmat, t_dim,
       s_g3_obs_covmat_dim, 0, s_g3_obs_covmat_after_dim,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_covmat_after_dim, t_band,
       s_g3_obs_covmat_band, 0, s_g3_obs_covmat_after_band,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_covmat_after_band, t_flt,
       s_g3_obs_covmat_flt, 0, s_g3_obs_covmat_after_band,
       0, &DataParser::add_text, 0);

  // .....  <g3-model> <obs> <distance>  .............................

  init(s_g3_obs, t_dist,
       s_g3_obs_dist, s_g3_obs_dist_opt, 0,
       0, 0, &DataParser::g3_obs_dist);

  init(s_g3_obs_dist, t_from,
       s_g3_obs_dist_from, 0, s_g3_obs_dist_after_from,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_dist_after_from, t_to,
       s_g3_obs_dist_to, 0, s_g3_obs_dist_after_to,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_dist_after_to, t_val,
       s_g3_obs_dist_val, 0, s_g3_obs_dist_opt,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_dist_opt, t_stdev,
       s_g3_obs_dist_opt_stdev, 0, 0,
       0, &DataParser::optional_stdev, 0);
 
  init(s_g3_obs_dist_opt, t_variance,
       s_g3_obs_dist_opt_variance, 0, 0,
       0, &DataParser::optional_variance, 0);
 
  init(s_g3_obs_dist_opt, t_from_dh,
       s_g3_obs_dist_opt_from_dh, 0, 0,
       0, &DataParser::optional_from_dh, 0);
 
  init(s_g3_obs_dist_opt, t_to_dh,
       s_g3_obs_dist_opt_to_dh, 0, 0,
       0, &DataParser::optional_to_dh, 0);

  // .....  <g3-model> <obs> <zenith>  ...............................

  init(s_g3_obs, t_zenith,
       s_g3_obs_zenith, s_g3_obs_zenith_opt, 0,
       0, 0, &DataParser::g3_obs_zenith);

  init(s_g3_obs_zenith, t_from,
       s_g3_obs_zenith_from, 0, s_g3_obs_zenith_after_from,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_zenith_after_from, t_to,
       s_g3_obs_zenith_to, 0, s_g3_obs_zenith_after_to,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_zenith_after_to, t_val,
       s_g3_obs_zenith_val, 0, s_g3_obs_zenith_opt,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_zenith_opt, t_stdev,
       s_g3_obs_zenith_opt_stdev, 0, 0,
       0, &DataParser::optional_stdev, 0);
 
  init(s_g3_obs_zenith_opt, t_variance,
       s_g3_obs_zenith_opt_variance, 0, 0,
       0, &DataParser::optional_variance, 0);
 
  init(s_g3_obs_zenith_opt, t_from_dh,
       s_g3_obs_zenith_opt_from_dh, 0, 0,
       0, &DataParser::optional_from_dh, 0);
 
  init(s_g3_obs_zenith_opt, t_to_dh,
       s_g3_obs_zenith_opt_to_dh, 0, 0,
       0, &DataParser::optional_to_dh, 0);

 // .....  <g3-model> <obs> <azimuth>  ...............................

  init(s_g3_obs, t_azimuth,
       s_g3_obs_azimuth, s_g3_obs_azimuth_opt, 0,
       0, 0, &DataParser::g3_obs_azimuth);

  init(s_g3_obs_azimuth, t_from,
       s_g3_obs_azimuth_from, 0, s_g3_obs_azimuth_after_from,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_azimuth_after_from, t_to,
       s_g3_obs_azimuth_to, 0, s_g3_obs_azimuth_after_to,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_azimuth_after_to, t_val,
       s_g3_obs_azimuth_val, 0, s_g3_obs_azimuth_opt,
       0, &DataParser::add_text, 0);

  init(s_g3_obs_azimuth_opt, t_stdev,
       s_g3_obs_azimuth_opt_stdev, 0, 0,
       0, &DataParser::optional_stdev, 0);
 
  init(s_g3_obs_azimuth_opt, t_variance,
       s_g3_obs_azimuth_opt_variance, 0, 0,
       0, &DataParser::optional_variance, 0);
 
  init(s_g3_obs_azimuth_opt, t_from_dh,
       s_g3_obs_azimuth_opt_from_dh, 0, 0,
       0, &DataParser::optional_from_dh, 0);
 
  init(s_g3_obs_azimuth_opt, t_to_dh,
       s_g3_obs_azimuth_opt_to_dh, 0, 0,
       0, &DataParser::optional_to_dh, 0);

  // .....  <g3-model> <obs> <vector>  ...............................

  init(s_g3_obs, t_vector,
       s_g3_obs_vector, s_g3_obs_vector_opt, 0,
       0, 0, &DataParser::g3_obs_vector);
  
   init(s_g3_obs_vector, t_from,
        s_g3_obs_vector_from, 0, s_g3_obs_vector_after_from,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_vector_after_from, t_to,
        s_g3_obs_vector_to, 0, s_g3_obs_vector_after_to,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_vector_after_to, t_dx,
        s_g3_obs_vector_dx, 0, s_g3_obs_vector_after_dx,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_vector_after_dx, t_dy,
        s_g3_obs_vector_dy, 0, s_g3_obs_vector_after_dy,
        0, &DataParser::add_text, 0);
  
   init(s_g3_obs_vector_after_dy, t_dz,
        s_g3_obs_vector_dz, 0, s_g3_obs_vector_opt,
        0, &DataParser::add_text, 0);
  
   init(s_g3_obs_vector_opt, t_from_dh,
        s_g3_obs_vector_opt_from_dh, 0, 0,
        0, &DataParser::optional_from_dh, 0);
  
   init(s_g3_obs_vector_opt, t_to_dh,
        s_g3_obs_vector_opt_to_dh, 0, 0,
        0, &DataParser::optional_to_dh, 0);

  // .....  <g3-model> <obs> <xyz>  ..................................

   init(s_g3_obs, t_xyz,
        s_g3_obs_xyz, s_g3_obs_xyz_after_z, 0,
        0, 0, &DataParser::g3_obs_xyz);

   init(s_g3_obs_xyz, t_id,
        s_g3_obs_xyz_id, 0, s_g3_obs_xyz_after_id,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_xyz_after_id, t_x,
        s_g3_obs_xyz_x, 0, s_g3_obs_xyz_after_x,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_xyz_after_x, t_y,
        s_g3_obs_xyz_y, 0, s_g3_obs_xyz_after_y,
        0, &DataParser::add_text, 0);

   init(s_g3_obs_xyz_after_y, t_z,
        s_g3_obs_xyz_z, 0, s_g3_obs_xyz_after_z,
        0, &DataParser::add_text, 0);
}

int DataParser::g3_model(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  g3->model = new g3::Model;
  
  return 0;
}

int DataParser::g3_model(const char *name)
{
  objects.push_back( new DataObject::g3_model(g3->model) );
  g3->model = 0;

  return  end_tag(name);
}

int DataParser::g3_param_unused(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  local_state.set_unused();

  return  0;
}

int DataParser::g3_param_fixed(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  local_state.set_fixed();

  return  0;
}

int DataParser::g3_param_free(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  local_state.set_free();

  return  0;
}

int DataParser::g3_param_constr(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  local_state.set_constr();

  return  0;
}

int DataParser::g3_param_n(const char *name)
{
  global_state_N.set_state(local_state);

  return  end_tag(name);
}

int DataParser::g3_param_e(const char *name)
{
  global_state_E.set_state(local_state);

  return  end_tag(name);
}

int DataParser::g3_param_u(const char *name)
{
  global_state_U.set_state(local_state);

  return  end_tag(name);
}

std::string DataParser::g3_get_id(std::string err)
{
  char    c;
  string id;
  string::const_iterator i=text_buffer.begin();
  string::const_iterator e=text_buffer.end();

  while (i!=e &&  isspace((c = *i))) { ++i;          }
  while (i!=e && !isspace((c = *i))) { ++i; id += c; }
  while (i!=e &&  isspace((c = *i))) { ++i;          }

  if (i!=e || id.empty()) error(err);

  text_buffer.erase();
  return id;
}

int DataParser::g3_point(const char *name)
{
  // checks consistency of the current point

  if (!point->N.cmp_state(point->E)) 
    return error("### parameters N and E do not have common status");

  return  end_tag(name);
}

int DataParser::g3_point_id(const char *name)
{
  string id = g3_get_id("### bad point name in <id> tag");

  point = g3->model->get_point(id);

  point->N.set_state(global_state_N);
  point->E.set_state(global_state_E);
  point->U.set_state(global_state_U);

  return  end_tag(name);
}

int DataParser::g3_point_param_n(const char *name)
{
  point->N.set_state(local_state);

  return  end_tag(name);
}

int DataParser::g3_point_param_e(const char *name)
{
  point->E.set_state(local_state);

  return  end_tag(name);
}

int DataParser::g3_point_param_u(const char *name)
{
  point->U.set_state(local_state);

  return  end_tag(name);
}

int DataParser::g3_point_b(const char *name)
{
  if (!deg2gon(text_buffer, blh.b))
    {
      return error("### bad format of numerical data in <point> <b> ");
    }
  text_buffer.erase();

  return  end_tag(name);
}

int DataParser::g3_point_l(const char *name)
{
  if (!deg2gon(text_buffer, blh.l))
    {
      return error("### bad format of numerical data in <point> <l> ");
    }
  text_buffer.erase();

  return  end_tag(name);
}

int DataParser::g3_point_h(const char *name)
{
  stringstream istr(text_buffer);
  if (!(istr >> blh.h))
    {
      return error("### bad format of numerical data in <point> <h> ");
    }
  text_buffer.erase();

  blh.b *= GON_TO_RAD;
  blh.l *= GON_TO_RAD;
  point->set_blh(blh.b, blh.l, blh.h);

  return  end_tag(name);
}

int DataParser::g3_point_z(const char *name)
{
  stringstream istr(text_buffer);
  double x, y, z;

  if (!(istr >> x >> y >> z))
    {
      return error("### bad format of numerical data in <point> xyz ");
    }

  text_buffer.erase();

  point->set_xyz(x, y, z);

  return  end_tag(name);
}

int DataParser::g3_point_height(const char *name)
{
  stringstream istr(text_buffer);
  double h;

  if (!(istr >> h))
    {
      return error("### bad format of numerical data in <point> height ");
    }

  text_buffer.erase();

  point->set_height(h);

  return  end_tag(name);
}

int DataParser::g3_obs(const char *name, const char **atts)
{
  no_attributes( name, atts );
  state = next[state][tag(name)];

  g3->obs_cluster = new g3::ObsCluster(&g3->model->obsdata);

  return 0;
}

int DataParser::g3_obs(const char *name)
{
  using namespace g3;

  int obs_dim = 0; 
  for (List<g3::Observation*>::const_iterator
         i=g3->obs_cluster->observation_list.begin(),
         e=g3->obs_cluster->observation_list.end();  i!=e;  ++i)
    {
      obs_dim += (*i)->dimension();
    }

  int cov_dim  = 0;
  int cov_band = 0; 
  typedef std::list<Cov>::const_iterator Iterator;
  for (Iterator i=g3->cov_list.begin(), e=g3->cov_list.end(); i!=e; ++i)
    {
      const Cov& cov = *i;

      cov_dim += cov.dim();
      if (int(cov.bandWidth()) > cov_band) cov_band = cov.bandWidth();
    }

  if (obs_dim == 0)       return error("### no observations in <obs>");
  if (obs_dim != cov_dim) return error("### bad covariance matrix dimension");

  g3->obs_cluster->covariance_matrix.reset(cov_dim, cov_band);
  g3->obs_cluster->covariance_matrix.set_zero();

  int offset = 0;
  for (Iterator i=g3->cov_list.begin(), e=g3->cov_list.end(); i!=e; ++i)
    {
      const Cov& cov = *i;

      for (size_t i=1; i<=cov.dim(); i++)
        for (size_t j=0; j<=cov.bandWidth() && i+j<=cov.dim(); j++)
          g3->obs_cluster->covariance_matrix(offset+i, offset+i+j) = cov(i, i+j);

      offset += cov.dim();
    }

  /* TODO: !!! here we should better test Cholesky decomposition !!! */
  for (int N=g3->obs_cluster->covariance_matrix.dim(), i=1; i<=N; i++)
    if(g3->obs_cluster->covariance_matrix(i,i) <= 0)
      return error("### zero or negative variance");

  g3->obs_cluster->update();
  g3->model->obsdata.clusters.push_back(g3->obs_cluster);
  g3->cov_list.clear();

  return  end_tag(name);
}

int DataParser::g3_obs_cov(const char *name)
{
  using namespace g3;
  stringstream istr(text_buffer);
  int     d, b;
  double  f;

  if (!(istr >> d >> b))  return error("### bad cov-mat");

  DataParser_g3::Cov cov(d, b);
  cov.set_zero();
  for (int i=1; i<=d; i++)          // upper triangular matrix by rows
    for (int j=i; j<=i+b && j<=d; j++)
      if (istr >> f)
        cov(i,j) = f;
      else        return error("### bad cov-mat / some data missing");

  if (!pure_data(istr)) return error("### bad cov-mat / redundant data");

  text_buffer.clear();
  g3->cov_list.push_back(cov);

  return  end_tag(name);
}

int DataParser::optional_stdev(const char *s, int len)
{
  using namespace g3;
  stringstream istr(string(s, len));
  DataParser_g3::Cov   cov(1,0);
  double  f;
  
  if (!pure_data(istr >> f)) return error("### bad <stdev>");

  cov(1,1) = f*f;
  g3->cov_list.push_back(cov);

  return 0;
}

int DataParser::optional_variance(const char *s, int len)
{
  using namespace g3;
  stringstream istr(string(s, len));
  DataParser_g3::Cov   cov(1,0);
  double  f;
  
  if (!pure_data(istr >> f)) return error("### bad <variance>");

  cov(1,1) = f;
  g3->cov_list.push_back(cov);

  return 0;
}

int DataParser::optional_from_dh(const char *s, int len)
{
  using namespace g3;
  stringstream istr(string(s,len));

  if (pure_data(istr >> g3->from_dh)) return 0;

  return error("### bad data in <from-dh>");
}

int DataParser::optional_to_dh(const char *s, int len)
{
  using namespace g3;
  stringstream istr(string(s,len));

  if (pure_data(istr >> g3->to_dh)) return 0;

  return error("### bad data in <to-dh>");
}

int DataParser::g3_obs_dist(const char *name)
{
  using namespace g3;  
  stringstream istr(text_buffer);
  string       from, to;
  double       val;

  if (pure_data(istr >> from >> to >> val))
    {
      text_buffer.clear();

      Distance* distance = new Distance;
      distance->from = from;
      distance->to   = to;
      distance->set(val);
      distance->from_dh = optional(g3->from_dh);
      distance->to_dh   = optional(g3->to_dh);
      g3->obs_cluster->observation_list.push_back(distance);  

      return  end_tag(name);
    }

  return error("### bad <distance>");
}

int DataParser::g3_obs_zenith(const char *name)
{
  using namespace g3;  
  stringstream istr(text_buffer);
  string       from, to;
  string       sval;

  if (pure_data(istr >> from >> to >> sval))
    {
      text_buffer.clear();

      double val;
      if (!deg2gon(sval, val))
        {
          istringstream istr(sval);
          istr >> val;
        }

      ZenithAngle* zenith = new ZenithAngle;
      zenith->from = from;
      zenith->to   = to;
      zenith->set(val);
      zenith->from_dh = optional(g3->from_dh);
      zenith->to_dh   = optional(g3->to_dh);
      g3->obs_cluster->observation_list.push_back(zenith);  

      return  end_tag(name);
    }

  return error("### bad <zenith>");
}

int DataParser::g3_obs_azimuth(const char *name)
{
  using namespace g3;  
  stringstream istr(text_buffer);
  string       from, to;
  string       sval;

  if (pure_data(istr >> from >> to >> sval))
    {
      text_buffer.clear();

      double val;
      if (!deg2gon(sval, val))
        {
          istringstream istr(sval);
          istr >> val;
        }

      Azimuth* azimuth = new Azimuth;
      azimuth->from = from;
      azimuth->to   = to;
      azimuth->set(val);
      azimuth->from_dh = optional(g3->from_dh);
      azimuth->to_dh   = optional(g3->to_dh);
      g3->obs_cluster->observation_list.push_back(azimuth);  

      return  end_tag(name);
    }

  return error("### bad <azimuth>");
}

int DataParser::g3_obs_vector(const char *name)
{
  using namespace g3;  
  stringstream istr(text_buffer);
  string from, to;
  double dx, dy, dz;

  if (pure_data(istr >> from >> to >> dx >> dy >> dz))
    {
      text_buffer.clear();

      Vector* vector = new Vector;
      vector->from = from;
      vector->to   = to;
      vector->set_dxyz(dx, dy, dz);
      vector->from_dh = optional(g3->from_dh);
      vector->to_dh   = optional(g3->to_dh);

      g3->obs_cluster->observation_list.push_back(vector);

      return end_tag(name);
    }
  
  return error("### bad <vector>");
}

int DataParser::g3_obs_xyz(const char *name)
{
  using namespace g3;  
  stringstream istr(text_buffer);
  string id;
  double x, y, z;

  if (pure_data(istr >> id >> x >> y >> z))
    {
      text_buffer.clear();

      XYZ* xyz = new XYZ;
      xyz->id  = id;
      xyz->set_xyz(x, y, z);

      g3->obs_cluster->observation_list.push_back(xyz);

      return end_tag(name);
    }
  
  return error("### bad <xyz>");
}