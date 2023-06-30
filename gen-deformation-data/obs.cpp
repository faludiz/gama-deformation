#include "obs.h"
#include "point.h"
#include <map>
#include <iomanip>
#include <cmath>
#include <cstdio>

extern std::map<std::string, Point> points;

using std::log10;
using std::abs;

void distance(const char* a, const char* b)
{
  distance(std::string(a), std::string(b));
}

void distance(std::string a, std::string b)
{
  auto pa = points.find(a);
  auto pb = points.find(b);
  if (pa == points.end() || pb == points.end()) return;

  double dx = pa->second.x;
  double dy = pa->second.y;
  dx -= pb->second.x;
  dy -= pb->second.y;

  double dist = std::sqrt(dx*dx + dy*dy);

  std::cout << "<distance from='" << a << "' to='" << b << "' val='"
            << std::fixed << std::setprecision(3) << dist << "' />\n";
}
