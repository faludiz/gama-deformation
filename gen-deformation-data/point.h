#ifndef POINT_H
#define POINT_H

struct Point {
  bool   fixed;
  double x, y;

  Point() : x(0), y(0), fixed(true) {}
  Point(double px, double py, bool f=false) : x(px), y(py), fixed(f) {}
};



#endif
