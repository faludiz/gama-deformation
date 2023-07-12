/*
  GNU Gama C++ library
  Copyright (C) 2019, 2023 Cepek <cepek@gnu.org

  This file is part of the GNU Gama C++ library.

  GNU Gama is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  GNU Gama is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GNU Gama.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <gnu_gama/local/lcoords.h>

using namespace GNU_gama::local;

const char* LocalCoordinateSystem::locos_tags[8]
  {
    "en", "nw", "se", "ws", "ne", "sw", "es", "wn"
  };

LocalCoordinateSystem::CS
LocalCoordinateSystem::string2locos(std::string s)
{
    if      (s == locos_tags[0]) return LocalCoordinateSystem::CS::EN;
    else if (s == locos_tags[1]) return LocalCoordinateSystem::CS::NW;
    else if (s == locos_tags[2]) return LocalCoordinateSystem::CS::SE;
    else if (s == locos_tags[3]) return LocalCoordinateSystem::CS::WS;
    else if (s == locos_tags[4]) return LocalCoordinateSystem::CS::NE;
    else if (s == locos_tags[5]) return LocalCoordinateSystem::CS::SW;
    else if (s == locos_tags[6]) return LocalCoordinateSystem::CS::ES;
    else if (s == locos_tags[7]) return LocalCoordinateSystem::CS::WN;

    else return LocalCoordinateSystem::CS::EN;
}
