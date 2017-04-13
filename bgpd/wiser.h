/*
 * wiser.h
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#ifndef BGPD_WISER_H_
#define BGPD_WISER_H_

#include "bgpd/common.h"
#include "bgpd/bgp_tarpan.h"

extern struct tarpan_protocol_handler wiser_protocol_handler;

void wiser_protocol_init(void);

void wiser_best_selection (struct bgp *bgp, struct bgp_node *rn,
  		    struct bgp_maxpaths_cfg *mpath_cfg,
  		    struct bgp_info_pair *result);

#endif /* BGPD_WISER_H_ */
