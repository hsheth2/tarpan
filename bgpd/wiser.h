/*
 * wiser.h
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#ifndef BGPD_WISER_H_
#define BGPD_WISER_H_

// note: no CXX allowed in this file

#ifdef __cplusplus
extern "C" {
#endif

#include "bgpd/common.h"
#include "bgpd/bgp_tarpan.h"

extern struct tarpan_protocol_handler wiser_protocol_handler;

void wiser_protocol_init(void);

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result);

void wiser_packet_received_handler (struct peer *const peer,
				    struct attr *const attr);

void wiser_initialize_packet (struct peer *const peer,
			      struct tarpan * tarpan);

void wiser_update_packet (struct peer *const peer,
			  struct tarpan * tarpan);

#ifdef __cplusplus
}
#endif

#endif /* BGPD_WISER_H_ */
