/*
 * wiser_static_path_costs.hpp
 *
 *  Created on: Apr 25, 2017
 *      Author: hsheth
 */

#ifndef BGPD_WISER_STATIC_PATH_COSTS_HPP_
#define BGPD_WISER_STATIC_PATH_COSTS_HPP_

// C++ includes
#include <fstream>
#include <unordered_map>
#include <utility>
#include <sstream>

extern "C"
{
// https://gcc.gnu.org/ml/gcc-help/2011-01/msg00052.html
#define new avoid_cxx_new_keyword

#include "log.h"
#include "bgpd/wiser.h"

#undef new
}

// C++ Area

static std::unordered_map<uint64_t, int> path_costs;

void
wiser_static_path_costs_init ()
{
  // read in path costs file
  std::ifstream wiser_configuration ("/etc/quagga/wiser.conf");
  std::string line;
  while (std::getline (wiser_configuration, line))
    {
      if (line.length () == 0)
	{
	  continue;
	}

      if (line[0] == '#')
	{
	  // comment
	  continue;
	}

      std::istringstream iss (line);
      std::string directive;
      iss >> directive;

      zlog_debug ("Found directive: %s", directive.c_str ());

      if (directive == "path_cost")
	{
	  uint32_t as1, as2, cost;
	  iss >> as1 >> as2 >> cost;

	  zlog_debug ("path_cost %d %d %d", as1, as2, cost);

	  uint64_t as_pair = (uint64_t) std::min (as1, as2) << 32
	      | std::max (as1, as2);
	  path_costs.insert (std::make_pair (as_pair, cost));
	}
      else
	{
	  zlog_err ("Unknown directive %s", directive.c_str ());
	}
    }
}

int
wiser_get_path_cost (int as1, int as2)
{
  uint64_t as_pair = (uint64_t) std::min (as1, as2) << 32 | std::max (as1, as2);
  if (path_costs.find(as_pair) == path_costs.end())
    {
      return INT16_MAX;
    }
  return path_costs[as_pair];
}

#endif /* BGPD_WISER_STATIC_PATH_COSTS_HPP_ */
