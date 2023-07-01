#ifndef OBSERVATION_BLOCK_H
#define OBSERVATION_BLOCK_h

#include <iostream>
#include <cmath>
#include <map>
#include "xml.h"


extern std::string obs_from; //the global from point id is passed to directions

struct obs {
  obs(const char* from=nullptr)
  {
    if (from) {
      obs_from = std::string(from);
    }
    else {
      obs_from.clear();
    }

    std::cout
      << "<obs"
      << (from ? " from='" + obs_from +  "'" : "")
      << ">\n";
  }
  ~obs() { std::cout << "</obs>\n\n"; }
};

void point(const char* a);
void point(std::string a);

void distance(const char* a, const char* b);
void distance(std::string a, std::string b);

void direction(std::string to);

#endif
