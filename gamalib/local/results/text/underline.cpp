/*  
    Geodesy and Mapping C++ Library (GNU GaMa / GaMaLib)
    Copyright (C) 2001  Ales Cepek <cepek@fsv.cvut.cz>

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
 *  $Id: underline.cpp,v 1.1 2001/12/07 12:54:43 cepek Exp $
 */

#include <string>

#include <gamalib/local/results/text/underline.h>

namespace GaMaLib {

std::string underline(std::string t, char c)
{
  std::string s(t);
  const unsigned int n = s.length();
  s.replace(0, n, n, c);
  return s;
}

std::string set_width(std::string s, int n)
{
  const int N = s.length(); 
  std::string t(s);
  while (n-- > N) t += ' ';
  return t;
}

}
