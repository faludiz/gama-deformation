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
 *  $Id: capi_gkfparser.cpp,v 1.1 2001/12/07 12:33:41 cepek Exp $
 */

#include <gamalib/c/api/capi_gkfparser.h>
#include <gamalib/c/api/capi_exception.h>
#include <gamalib/c/api/capi_private_exception.h>
#include <gamalib/local/network.h>
#include <gamalib/xml/gkfparser.h>

#include <cstdlib>
#include <cstring>
using namespace std;

using namespace GaMaLib;

extern "C" {

  void* Cgama_GKF_parser(void* local_network)
  {
    try 
      {
        LocalNetwork* ln = static_cast<LocalNetwork*>(local_network);
        /* constructed for point data and observation data objects*/
        return new GKFparser(ln->PD, ln->OD);
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }
  void Cgama_GKF_parser_dtor (void* parser)
  {
    /* do nothing if there was an exception */
    if (Cgama_exception()) return;

    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        delete gp;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
  }
  void Cgama_GKF_parser_parse(void* p, const char* text, int len, int isFinal)
  {
    /* do nothing if there was an exception */
    if (Cgama_exception()) return;

    try
      {
        GKFparser* gp = static_cast<GKFparser*>(p);
        gp->xml_parse(text, len, isFinal);
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
  }
  double Cgama_GKF_parser_apriori_m0(void* parser)
  {
    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        return gp->m0_apr;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }
  double Cgama_GKF_parser_conf_pr(void* parser)
  {
    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        return gp->konf_pr;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }
  double Cgama_GKF_parser_tol_abs(void* parser)
  {
    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        return gp->tol_abs;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }
  int Cgama_GKF_parser_m0_apriori(void* parser)
  {
    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        return gp->typ_m0_apriorni ? 1 : 0;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }

  char* Cgama_GKF_parser_description(void* parser)
  {
    try 
      {
        GKFparser* gp = static_cast<GKFparser*>(parser);
        if (gp->description == "") return 0;

        char* text = static_cast<char*>(malloc(gp->description.length() + 1));
        if (text) strcpy(text, gp->description.c_str());

        return text;
      }
    catch (const GaMaLib::Exception& e)
      {
        Cgama_private_set_exception(e);
      }
    catch(...)
      {
        Cgama_private_set_unknown_exception();
      }
    return 0;
  }

}


