#ifndef HEADER
#define HEADER

#include <iostream>

struct xml {
  xml() {  std::cout <<
R"HEADER(<?xml version="1.0" ?>

<!-- 2023-06-30 -->

<gama-local xmlns="http://www.gnu.org/software/gama/gama-local">
<network axes-xy="ne">
<description>
gen-deformation-data
</description>

<parameters
   sigma-apr="10"
   conf-pr="0.95"
   tol-abs="1000"
   sigma-act="apriori"
/>

<points-observations
      distance-stdev="5.0"
      direction-stdev="10.0"
      angle-stdev="10.0"
      zenith-angle-stdev="10" >

)HEADER";
}

  ~xml() {std::cout << "\n</points-observations>\n</network>\n</gama-local>\n\n"; }
};

#endif
