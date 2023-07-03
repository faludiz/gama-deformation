#include <map>
#include <string>
#include <iostream>

#include "transformation.h"
#include "point.h"

extern std::map<std::string, Point> points;



void transformation()
{
  double cx {0}, cy {0};

  int N = 0;
  for (auto p : points)
    {
      if (p.second.fixed) continue;

      cx += p.second.x;
      cy += p.second.y;
      N++;
    }

  cx /= N;
  cy /= N;
  cx += 0.001;
  cy -= 0.002;

  std::cout << "<!-- generated point shifts\n";

  double t11 {1-2e-5}, t12 {1e-5}, t21 {-2e-5},  t22 {1 + 1e-6};
  for (auto p : points)
    {
      if (p.second.fixed) continue;

      double dx = p.second.x - cx;
      double dy = p.second.y - cy;
      double x = cx + t11*dx + t12*dy;
      double y = cy + t21*dx + t22*dy;

      std::cout << "*** " << p.first
                << "   "  << x-p.second.x << "  " << y-p.second.y << std::endl;
    }

  std::cout << "-->\n\n";
}
