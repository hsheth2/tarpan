
/*
 * wiser.cpp
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

// C++ includes
#include <fstream>
#include <map>
#include <unordered_map>
#include <utility>
#include <cmath>

extern "C" {
// https://gcc.gnu.org/ml/gcc-help/2011-01/msg00052.html
#define new avoid_cxx_new_keyword

#include "bgpd/wiser.h"
#include "bgpd/common.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_tarpan.h"
#include "bgpd/attr_tarpan.pb-c.h"

#undef new
}

// C++ code


// C++14 does  support designated initializers
struct tarpan_protocol_handler wiser_protocol_handler = {
    .bgp_best_selection = wiser_best_selection,
    .packet_received_handler = wiser_packet_received_handler,
    .initialize_packet = wiser_initialize_packet,
};

static std::unordered_map<std::pair<int, int>, int> path_costs;

void wiser_protocol_init(void)
{
  zlog_info("wiser_protocol_init: starting");

  // read in path costs file
  std::ifstream wiser_configuration("/etc/quagga/wiser.conf");
  std::string line;
  while (std::getline(wiser_configuration, line)) {
      if (line.length() == 0) {
	  continue;
      }

      if (line[0] == '#') {
	  // comment
	  continue;
      }

      std::istringstream iss(line);
      std::string directive;
      iss >> directive;

      if (directive == "path_cost") {
	  int as1, as2, cost;
	  iss >> as1 >> as2 >> cost;

	  std::pair<int, int> as_pair = std::make_pair(std::min(as1, as2), std::max(as1, as2));
	  path_costs.insert(make_pair(as_pair, cost));
      } else {
	  zlog_err("Unknown directive %s", directive.c_str());
      }
  }

  // initialize wiser costs table (both send and recv)
  wiser_costs_table_init();

  // TODO open cost portal
}

static std::map<as_t, uint32_t> advcost_sent;
static std::map<as_t, uint32_t> advcost_recv;

// initialize wiser costs table (both send and recv)
void wiser_costs_table_init()
{
  advcost_sent.clear();
  advcost_recv.clear();
}

void increment_map_value(std::map<as_t, uint32_t>& map, as_t key, uint32_t delta)
{
  auto find = map.find(key);

  if (find == map.end()) {
      map[key] = delta;
  } else
    find->second += delta;
}

void update_sent_cost(as_t destination, uint32_t cost) {
  increment_map_value(advcost_sent, destination, cost);
}

void update_recv_cost(as_t from, uint32_t cost) {
  increment_map_value(advcost_recv, from, cost);
}

double normalization(as_t peer) {
  // multiply the costs received from this peer by the normalization factor

  // TODO: come up with a better system for handling 0
  uint32_t total_send = 1+advcost_sent[peer];
  uint32_t total_recv = 1+advcost_recv[peer];

  return (double)total_send / (double) total_recv;
}

void wiser_packet_received_handler (struct peer *const peer,
                    struct attr *const attr)
{
  struct tarpan * tarp = attr->tarpan;

  if (!tarp->message->wiser)
    {
      // the incoming packet does not have wiser data
      return;
    }

  Wiser* wiser = tarp->message->wiser;

  zlog_debug("wiser_packet_received_handler");

  // determine if the packet was sent via a gulf,
  // or if it was sent directly (within same island)
  as_t my_as = peer->bgp->as;
  as_t intended_as = wiser->sender_as;

  if (my_as == intended_as) {
      // not over a gulf
  } else {
      // over a gulf
      // TODO must contact cost portal
  }

  // TODO update cost portal data
}

void wiser_initialize_packet (struct peer *const peer,
			      struct tarpan * tarpan)
{
  Wiser* wiser = (Wiser*) malloc(sizeof(Wiser));
  wiser__init(wiser);
  tarpan->message->wiser = wiser;

  wiser->path_cost = 5; // TODO: lookup in static file

  update_sent_cost(peer->as, wiser->path_cost);

  zlog_debug("wiser_initialize_packet");

  // place this node's AS/IP address in wiser data
  IPAddress* local_addr = (IPAddress*) malloc(sizeof(IPAddress));
  ipaddress__init(local_addr);
  local_addr->bytes = peer->bgp->router_id.s_addr;

  wiser->sender_as = peer->bgp->as;
  wiser->sender_address = local_addr;
}

// TODO wiser_forward_packet (or something)
// ^ should use normalization?

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result)
{
  // TODO nothing here
}
