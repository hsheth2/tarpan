
/*
 * wiser.cpp
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

// C++ includes
#include <fstream>
#include <map>

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

void wiser_protocol_init(void)
{
  zlog_info("wiser_protocol_init: starting");

  // TODO initialize wiser costs table (both send and recv)

  // TODO open cost portal
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

  struct in_addr local_ip = peer->bgp->router_id;
  as_t as_number = peer->bgp->as;

  wiser->path_cost = 5; // TODO: lookup in static file

  // TODO update cost portal data

  zlog_debug("wiser_initialize_packet");

  // place this node's AS/IP address in wiser data
  IPAddress* local_addr = (IPAddress*) malloc(sizeof(IPAddress));
  ipaddress__init(local_addr);
  local_addr->bytes = local_ip.s_addr;

  wiser->sender_as = as_number;
  wiser->sender_address = local_addr;
}

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result)
{
  // TODO nothing here
}
