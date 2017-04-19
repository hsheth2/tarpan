/*
 * wiser.cpp
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#include "wiser.hpp"

#include "bgpd/common.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_tarpan.h"

void wiser_protocol_init(void)
{
  // TODO initialize wiser costs table (both send and recv)

  // TODO open cost portal
}

struct tarpan_protocol_handler wiser_protocol_handler = {
    .bgp_best_selection = wiser_best_selection,
    .packet_received_handler = wiser_packet_received_handler,
    .initialize_packet = wiser_initialize_packet,
};

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
