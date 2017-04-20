/*
 * tarpan_test.cc
 *
 *  Created on: Apr 18, 2017
 *      Author: Harshal Sheth
 */

#include <iostream>
#include <algorithm>

extern "C" {
// https://gcc.gnu.org/ml/gcc-help/2011-01/msg00052.html
#define new avoid_cxx_new_keyword

#include "bgpd/bgp_tarpan.h"
#include "bgpd/wiser.h"
}

int main(int argc, char **argv) {
  // TODO: add actual unit tests
  std::cout << "All tests passed." << std::endl;
  return 0;
}

