
/*
 * wiser.cpp
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

extern "C" {
// https://gcc.gnu.org/ml/gcc-help/2011-01/msg00052.html
#define new avoid_cxx_new_keyword

#include "bgpd/wiser.h"
#include "bgpd/common.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_tarpan.h"
}

// C++

// C++11 does not support designated initializers
struct tarpan_protocol_handler wiser_protocol_handler = {
    /*.bgp_best_selection =*/ wiser_best_selection,
    /*.packet_received_handler =*/ wiser_packet_received_handler,
    /*.initialize_packet =*/ wiser_initialize_packet,
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

  // TODO determine if the packet was sent via a gulf,
  // or if it was sent directly (within same island)

  // TODO handle the incoming packet
}

void wiser_initialize_packet (struct peer *const peer,
			      struct tarpan * tarpan)
{
  // TODO place wiser's cost to destination
  // TODO update cost portal data

  // TODO place this node's IP address in wiser data
}

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result)
{
  // TODO nothing here
}
