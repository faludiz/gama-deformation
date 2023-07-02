#include <iostream>
#include <map>
#include <string>
#include <iomanip>

using namespace std;

#include "xml.h"
#include "point.h"
#include "obs.h"
#include "transformation.h"

std::map<std::string, Point> points;
std::string obs_from;

int main()
{
  xml x;

  // fixed points
  points["A"] = Point(  460.513,  360.007, true);
  points["B"] = Point(  440.187, 1711.264, true);
  points["C"] = Point( 2613.062, 1220.913, true);

  // free points
  points["d"] = Point(  864.111, 1723.498 );
  points["e"] = Point(  870.341, 1290.462 );
  points["f"] = Point(  850.171,  652.319 );

  points["g"] = Point( 1213.642, 1729.537 );
  points["h"] = Point( 1250.654, 1202.445 );
  points["i"] = Point( 1200.269,  706.171 );

  points["j"] = Point( 1803.784, 1766.784 );
  points["k"] = Point( 1800.401, 1326.347 );
  points["l"] = Point( 1782.918,  766.876 );

#ifdef EPOCH_1
  points.erase("k");
#endif

#ifdef EPOCH_2
  transformation();
  points.erase("e");
#endif

  {
    for (auto p : points)  point(p.first);
    std::cout << std::endl;
  }

  {
    obs dists;

    distance("A", "d");
    distance("A", "e");
    distance("A", "f");

    distance("B", "d");
    distance("B", "e");
    distance("B", "f");

    distance("C", "j");
    distance("C", "k");
    distance("C", "l");
  }

  {
    obs dists;

    distance("h", "d");
    distance("h", "e");
    distance("h", "f");
    distance("h", "g");
    distance("h", "i");
    distance("h", "j");
    distance("h", "k");
    distance("h", "l");

    distance("g", "d");
    distance("g", "j");
    distance("g", "h");
    distance("i", "f");
    distance("i", "l");
    distance("i", "h");
  }

  {
    obs directions("h");

    direction("g");
    direction("d");
    direction("e");
    direction("f");
    direction("i");
    direction("l");
    direction("k");
    direction("j");
  }

  {
    obs directions("g");

    direction("d");
    direction("e");
    direction("h");
    direction("k");
    direction("j");
  }

  {
    obs directions("i");

    direction("f");
    direction("e");
    direction("h");
    direction("k");
    direction("l");
  }

}
