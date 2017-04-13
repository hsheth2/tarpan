/*
 * wiser.c
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#include "bgpd/common.h"
#include "bgpd/bgp_tarpan.h"
#include "bgpd/wiser.h"

struct tarpan_protocol_handler wiser_protocol_handler = {
    .bgp_best_selection = NULL
};

