/*
 * wiser.c
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

#include "bgpd/bgp_tarpan.h"
#include "bgpd/wiser.h"

struct tarpan_protocol_handler wiser = {
    .bgp_best_selection = NULL
};

