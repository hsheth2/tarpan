/*
 * wiser.c
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#include "bgpd/common.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_tarpan.h"
#include "bgpd/wiser.h"

void wiser_protocol_init(void)
{
  // nothing for now;
  // in the future, this should initialize any routing tables/etc.
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
  // TODO handle the incoming packet
}

void wiser_initialize_packet (struct peer *const peer,
			      struct tarpan * tarpan)
{
  // TODO initialize wiser packet with costs, etc.
}

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result)
{
  // TODO nothing here
}
