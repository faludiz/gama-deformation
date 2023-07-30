#ifndef HEADER
#define HEADER

#include <iostream>

struct xml {
  xml() {  std::cout <<
R"HEADERbegin(<?xml version="1.0" ?>

<!-- 2023-06-30 -->

<gama-local xmlns="http://www.gnu.org/software/gama/gama-local">
<network axes-xy="en" angles="left-handed">
<description>
)HEADERbegin"

#ifndef EPOCH_1
#ifndef EPOCH_2
"gen-deformation-data epoch 0"
#endif
#endif

#ifdef EPOCH_1
"gen-deformation-data epoch 1"
#endif

#ifdef EPOCH_2
"gen-deformation-data epoch 2"
#endif

R"HEADERend(
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

)HEADERend";
}

  ~xml() {std::cout << "\n</points-observations>\n</network>\n</gama-local>\n\n"; }
};

#endif
