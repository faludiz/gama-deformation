#ifndef OBSERVATION_BLOCK_H
#define OBSERVATION_BLOCK_h

#include <iostream>
#include <cmath>
#include <map>
#include "xml.h"

//extern std::map<std::string, Point> points;

struct obs {
  obs()  { std::cout << "<obs>\n"; }
  ~obs() { std::cout << "</obs>\n"; }
};

void distance(const char* a, const char* b);
void distance(std::string a, std::string b);

#endif
