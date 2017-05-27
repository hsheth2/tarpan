/*
 * common.h
 *
 *  Created on: Apr 13, 2017
 *      Author: hsheth
 */

#ifndef BGPD_COMMON_H_
#define BGPD_COMMON_H_

#include <zebra.h>

#include "prefix.h"
#include "linklist.h"
#include "memory.h"
#include "command.h"
#include "stream.h"
#include "filter.h"
#include "str.h"
#include "log.h"
#include "routemap.h"
#include "buffer.h"
#include "sockunion.h"
#include "plist.h"
#include "thread.h"
#include "workqueue.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_route.h"

struct bgp_info_pair
{
  struct bgp_info *old;struct bgp_info *new;
};

#endif /* BGPD_COMMON_H_ */
