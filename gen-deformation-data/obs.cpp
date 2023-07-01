#include "obs.h"
#include "point.h"
#include <map>
#include <iomanip>
#include <cmath>
#include <cstdio>

extern std::map<std::string, Point> points;

using std::log10;
using std::abs;

void point(const char* p)
{
  point(std::string(p));
}
void point(std::string p)
{
  auto ptr = points.find(p);
  if (ptr == points.end()) return;

  //auto p = points.find(p);
  double x = ptr->second.x;
  double y = ptr->second.y;
  bool   t = ptr->second.fixed;

  std::cout << "<point id='" << p << "' "
            << "x='" << std::fixed << std::setprecision(3) << x << "' "
            << "y='" << std::fixed << std::setprecision(3) << y << "' "
            << (t ? " fix='xy'" : " adj='xy'")
            << " />\n";
}


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


void direction(std::string to)
{
  auto pt =points.find(to);
  if (pt == points.end()) return;
  double dx = pt->second.x;
  double dy = pt->second.y;

  if (obs_from.empty()) return;
  auto from = points[obs_from];
  dx -= from.x;
  dy -= from.y;

  double d = std::atan2(dy, dx)/std::atan2(0,-1)*200;

  std::cout << "<direction to='" << to << "' val='"
            << std::fixed << std::setprecision(3) << d << "' />\n";
}
