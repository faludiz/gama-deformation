/*  
    Geodesy and Mapping C++ Library (GNU GaMa / GaMaLib)
    Copyright (C) 1999  Ales Cepek <cepek@fsv.cvut.cz>

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
 *  $Id: adjusted_observations.h,v 1.5 2003/07/24 16:25:46 cepek Exp $
 */

#ifndef GaMa_GaMaProg_Vyrovnana_Pozorovani_h_
#define GaMa_GaMaProg_Vyrovnana_Pozorovani_h_

#include <gamalib/local/network.h>

namespace GaMaLib {

template <class OutStream>
void AdjustedObservations(GaMaLib::LocalNetwork* IS, OutStream& out)
{
   using namespace std;
   using namespace GaMaLib;
   // using GaMaLib::Double;

   const int y_sign = Consistent(IS->PD) ? +1 : -1;
   const Vec& v = IS->residuals();
   const int pocmer = IS->sum_observations();

   out << T_GaMa_adjobs_Adjusted_observations << "\n"
       << underline(T_GaMa_adjobs_Adjusted_observations, '*') << "\n\n";

   int minval = 12;
   int maxval = minval;   // maximal value field width (coordinates!)
   {
     for (int i=1; i<=pocmer; i++)
       {
         Observation* pm = IS->ptr_obs(i);
         int z = 0;
         double d = pm->value();
         if (d < 0)
           {
             z = 1;
             d = -d;
           }
         if (d < 1e5) continue;
         z += 6;   // ... decimal point plus 5 digits
         do {
           z++; 
           d /= 10;
         } while (d >= 1);
         if (z > maxval) maxval = z;
       }
   }

   Double kki = IS->conf_int_coef();
   out.width(IS->maxw_obs());
   out << "i" << " ";
   out.width(IS->maxw_id());
   out << T_GaMa_standpoint << " ";
   out.width(IS->maxw_id());
   out << T_GaMa_target << "       ";
   out.width(maxval);
   out << T_GaMa_adjobs_observed << " ";
   out.width(maxval);
   out << T_GaMa_adjobs_adjusted << T_GaMa_adjobs_header1;
   {   // for ...
     int kk = 13 + maxval-minval;
     for (int i=0; i < (IS->maxw_obs()+2*(IS->maxw_id())+kk); i++) out << "=";
   }   // for ...
   out << T_GaMa_adjobs_value;
   {
     for (int i=minval; i<maxval; i++) out << "=";
   }
   out << "==== [m|g] ====== [mm|cc] ==\n\n";
   out.flush();

   PointID predcs = "";   // provious standpoint ID
   for (int i=1; i<=pocmer; i++)
   {
      Observation* pm = IS->ptr_obs(i);
      out.width(IS->maxw_obs());
      out << i << " ";
      PointID cs = pm->from();
      out.width(IS->maxw_id());
      if (cs != predcs)
         out << cs.c_str();
      else
         out << " ";
      out << " ";
      PointID cc = pm->to();
      out.width(IS->maxw_id());
      out << cc.c_str();
      out.setf(ios_base::fixed, ios_base::floatfield);

      {   // ***************************************************
        if (Distance* d = dynamic_cast<Distance*>(pm))
          {
            out << T_GaMa_distance;
            out.precision(5);
            out.width(maxval);
            Double m = d->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";
          }
        else if (Direction* s = dynamic_cast<Direction*>(pm))
          {
            out << T_GaMa_direction;
            out.precision(6);
            out.width(maxval);
            Double m = R2G*(s->value());
            out << m << " ";
            out.width(maxval);
            m += v(i)/10000;
            if (m < 0) m += 400;
            if (m >= 400) m -= 400;
            out << m << " ";
          }
        else if (Angle* u = dynamic_cast<Angle*>(pm))
          {
            out << '\n';
            out.width(IS->maxw_obs() + 2 + 2*(IS->maxw_id()));
            out << (u->fs()).c_str();
            out << T_GaMa_angle;
            out.precision(6);
            out.width(maxval);
            Double m = R2G*(u->value());
            out << m << " ";
            out.width(maxval);
            m += v(i)/10000;
            if (m < 0) m += 400;
            if (m >= 400) m -= 400;
            out << m << " ";
          }
        else if (S_Distance* sd = dynamic_cast<S_Distance*>(pm))
          {
            out << T_GaMa_s_distance; 
            out.precision(5);
            out.width(maxval);
            Double m = sd->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";
          }
        else if (Z_Angle* za = dynamic_cast<Z_Angle*>(pm))
          {
            out << T_GaMa_z_angle;
            out.precision(6);
            out.width(maxval);
            Double m = R2G*(za->value());
            out << m << " ";
            out.width(maxval);
            m += v(i)/10000;
            out << m << " ";
          }
        else if (X* x = dynamic_cast<X*>(pm))
          {
            out << T_GaMa_x;
            out.precision(5);
            out.width(maxval);
            Double m = x->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";
          }
        else if (Y* y = dynamic_cast<Y*>(pm))
          {
            out << T_GaMa_y;
            out.precision(5);
            out.width(maxval);
            Double m = y->value();
            out << y_sign*m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << y_sign*m << " ";
          }
        else if (Z* z = dynamic_cast<Z*>(pm))
          {
            out << T_GaMa_z;
            out.precision(5);
            out.width(maxval);
            Double m = z->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";
          }
        else if (H_Diff* h = dynamic_cast<H_Diff*>(pm))
          {
            out << T_GaMa_levell;
            out.precision(5);
            out.width(maxval);
            Double m = h->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";            
          }
        else if (Xdiff* dx = dynamic_cast<Xdiff*>(pm))
          {
            out << T_GaMa_xdiff;
            out.precision(5);
            out.width(maxval);
            Double m = dx->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";            
          }
        else if (Ydiff* dy = dynamic_cast<Ydiff*>(pm))
          {
            out << T_GaMa_ydiff;
            out.precision(5);
            out.width(maxval);
            Double m = dy->value();
            out << y_sign*m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << y_sign*m << " ";            
          }
        else if (Zdiff* dz = dynamic_cast<Zdiff*>(pm))
          {
            out << T_GaMa_zdiff;
            out.precision(5);
            out.width(maxval);
            Double m = dz->value();
            out << m << " ";
            out.width(maxval);
            m += v(i)/1000;
            out << m << " ";            
          }
        else  
          {
            throw GaMaLib::Exception("review/adjusted_observations.h - "
                                     "unknown observation type");
          }
      }   // ***************************************************

      out.precision(1);
      out.width(7);
      Double ml = IS->stdev_obs(i);
      out << ml << " ";
      out.width(7);
      out << ml*kki;

      out << '\n';
      out.flush();

      predcs = cs;  // previous standpoint ID
   }
   out << "\n\n";
   out.flush();
}

}

#endif
