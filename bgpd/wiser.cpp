/*
 * wiser.cpp
 *
 *  Created on: Apr 9, 2017
 *      Author: hsheth
 */

// C++ includes
#include <utility>
#include <cmath>
#include <map> // (do not remove; it will cause pain)

extern "C"
{
// https://gcc.gnu.org/ml/gcc-help/2011-01/msg00052.html
#define new avoid_cxx_new_keyword

#include "log.h"
#include "bgpd/wiser.h"
#include "bgpd/common.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_aspath.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_tarpan.h"
#include "bgpd/attr_tarpan.pb-c.h"

#undef new
}

// C++ code
#include "bgpd/wiser_static_path_costs.hpp"
#include "bgpd/wiser_costs.hpp"

// C++14 does support designated initializers
struct tarpan_protocol_handler wiser_protocol_handler =
  { .packet_received_handler = wiser_packet_received_handler,
      .initialize_packet = wiser_initialize_packet, .update_packet =
	  wiser_update_packet, .protocol_info_cmp = wiser_info_cmp, };

const int wiser_thread_pool_size = 4;

void
wiser_protocol_init (void)
{
  zlog_info ("wiser_protocol_init: starting");

  // thread pool initialization
  wiser_thread_pool.resize(wiser_thread_pool_size);

  // load static path costs from file
  wiser_static_path_costs_init ();

  // initialize wiser costs table (both send and recv)
  wiser_costs_table_init ();

  // open cost portal
  if (wiser_cost_portal_init ())
    {
      // TODO: handle error
      zlog_err ("could not open wiser cost portal");
    }
}

void
wiser_packet_received_handler (struct peer * const peer,
			       struct attr * const attr)
{
  struct tarpan * tarp = attr->tarpan;

  if (!tarp->message->wiser)
    {
      // the incoming packet does not have wiser data
      return;
    }

  Wiser* wiser = tarp->message->wiser;
  zlog_debug ("wiser_packet_received_handler");

  // update recv costs
  update_recv_cost (wiser->sender_as, wiser->path_cost);

  // determine if the packet was sent via a gulf,
  // or if it was sent directly (within same island)
  if (peer->as == wiser->sender_as)
    { // not over a gulf
      // nothing
    }
  else
    { // over a gulf
      // must update other AS's sent costs
      // using the specified cost portal

      // must contact cost portal - not blocking
      wiser_thread_pool.push(std::bind(wiser_contact_cost_portal, wiser, wiser->path_cost, peer->bgp));
    }

  // apply normalization to incoming path costs
  wiser->path_cost *= normalization (peer->as);
}

void
wiser_initialize_packet (struct peer * const peer, struct tarpan * tarpan)
{
  Wiser* wiser = (Wiser*) malloc (sizeof(Wiser));
  wiser__init (wiser);
  tarpan->message->wiser = wiser;

  wiser->path_cost = wiser_get_path_cost (peer->bgp->as, peer->as);

  update_sent_cost (peer->as, wiser->path_cost);

  zlog_debug ("wiser_initialize_packet");

  // place this node's AS/IP address in wiser data
  IPAddress* local_addr = (IPAddress*) malloc (sizeof(IPAddress));
  ipaddress__init (local_addr);
  local_addr->bytes = peer->bgp->router_id.s_addr;

  wiser->sender_as = peer->bgp->as;
  wiser->sender_address = local_addr;
}

void
wiser_update_packet (struct peer * const peer, struct tarpan * tarpan)
{
  Wiser* wiser = tarpan->message->wiser;
  if (!wiser)
    {
      wiser_initialize_packet (peer, tarpan);
      return;
    }
  assert(wiser);

  wiser->path_cost += wiser_get_path_cost (peer->bgp->as, peer->as);
  update_sent_cost (peer->as, wiser->path_cost);

  zlog_debug ("wiser_update_packet");

  // place this node's AS/IP address in wiser data
  IPAddress* local_addr = (IPAddress*) malloc (sizeof(IPAddress));
  ipaddress__init (local_addr);
  local_addr->bytes = peer->bgp->router_id.s_addr;

  wiser->sender_as = peer->bgp->as;
  wiser->sender_address = local_addr;
}

int
wiser_info_cmp (struct bgp *bgp, struct bgp_info *nw, struct bgp_info *exist,
		int *paths_eq)
{
  zlog_debug ("wiser_info_cmp - starting");
  struct attr *newattr, *existattr;
  struct attr_extra *newattre, *existattre;
  bgp_peer_sort_t new_sort;
  bgp_peer_sort_t exist_sort;
  u_int32_t new_pref;
  u_int32_t exist_pref;
  u_int32_t new_med;
  u_int32_t exist_med;
  u_int32_t new_weight;
  u_int32_t exist_weight;
  uint32_t newm, existm;
  struct in_addr new_id;
  struct in_addr exist_id;
  int new_cluster;
  int exist_cluster;
  int internal_as_route;
  int confed_as_route;
  int ret;

  *paths_eq = 0;

  /* 0. Null check. */
  if (nw == NULL)
    return 0;
  if (exist == NULL)
    return 1;

  newattr = nw->attr;
  existattr = exist->attr;
  newattre = newattr->extra;
  existattre = existattr->extra;

  /* 1. Weight check. */
  new_weight = exist_weight = 0;

  if (newattre)
    new_weight = newattre->weight;
  if (existattre)
    exist_weight = existattre->weight;

  if (new_weight > exist_weight)
    return 1;
  if (new_weight < exist_weight)
    return 0;

  /* 2. Local preference check. */
  new_pref = exist_pref = bgp->default_local_pref;

  if (newattr->flag & ATTR_FLAG_BIT(BGP_ATTR_LOCAL_PREF))
    new_pref = newattr->local_pref;
  if (existattr->flag & ATTR_FLAG_BIT(BGP_ATTR_LOCAL_PREF))
    exist_pref = existattr->local_pref;

  if (new_pref > exist_pref)
    return 1;
  if (new_pref < exist_pref)
    return 0;

  /* 3. Local route check. We prefer:
   *  - BGP_ROUTE_STATIC
   *  - BGP_ROUTE_AGGREGATE
   *  - BGP_ROUTE_REDISTRIBUTE
   */
  if (!(nw->sub_type == BGP_ROUTE_NORMAL))
    return 1;
  if (!(exist->sub_type == BGP_ROUTE_NORMAL))
    return 0;

  /* (3+i). Wiser cost check */
  zlog_debug("wiser_info_cmp: (paths from AS %d and %d) new cost = %d vs old cost = %d", 
    nw->peer->as, exist->peer->as,
    newattr->tarpan->message->wiser->path_cost, existattr->tarpan->message->wiser->path_cost);
  if (newattr->tarpan->message->wiser->path_cost
      < existattr->tarpan->message->wiser->path_cost)
    {
      zlog_debug("wiser_info_cmp: choosing new path with cost");
      return 1;
    }
  else
    {
      zlog_debug("wiser_info_cmp: remaining with previous path");
      return 0;
    }

  /* 4. AS path length check. */
  if (!bgp_flag_check (bgp, BGP_FLAG_ASPATH_IGNORE))
    {
      int exist_hops = aspath_count_hops (existattr->aspath);
      int exist_confeds = aspath_count_confeds (existattr->aspath);

      if (bgp_flag_check (bgp, BGP_FLAG_ASPATH_CONFED))
	{
	  int aspath_hops;

	  aspath_hops = aspath_count_hops (newattr->aspath);
	  aspath_hops += aspath_count_confeds (newattr->aspath);

	  if (aspath_hops < (exist_hops + exist_confeds))
	    return 1;
	  if (aspath_hops > (exist_hops + exist_confeds))
	    return 0;
	}
      else
	{
	  int newhops = aspath_count_hops (newattr->aspath);

	  if (newhops < exist_hops)
	    return 1;
	  if (newhops > exist_hops)
	    return 0;
	}
    }

  /* 5. Origin check. */
  if (newattr->origin < existattr->origin)
    return 1;
  if (newattr->origin > existattr->origin)
    return 0;

  /* 6. MED check. */
  internal_as_route = (aspath_count_hops (newattr->aspath) == 0
      && aspath_count_hops (existattr->aspath) == 0);
  confed_as_route = (aspath_count_confeds (newattr->aspath) > 0
      && aspath_count_confeds (existattr->aspath) > 0
      && aspath_count_hops (newattr->aspath) == 0
      && aspath_count_hops (existattr->aspath) == 0);

  if (bgp_flag_check (bgp, BGP_FLAG_ALWAYS_COMPARE_MED)
      || (bgp_flag_check (bgp, BGP_FLAG_MED_CONFED) && confed_as_route)
      || aspath_cmp_left (newattr->aspath, existattr->aspath)
      || aspath_cmp_left_confed (newattr->aspath, existattr->aspath)
      || internal_as_route)
    {
      new_med = bgp_med_value (nw->attr, bgp);
      exist_med = bgp_med_value (exist->attr, bgp);

      if (new_med < exist_med)
	return 1;
      if (new_med > exist_med)
	return 0;
    }

  /* 7. Peer type check. */
  new_sort = nw->peer->sort;
  exist_sort = exist->peer->sort;

  if (new_sort == BGP_PEER_EBGP
      && (exist_sort == BGP_PEER_IBGP || exist_sort == BGP_PEER_CONFED))
    return 1;
  if (exist_sort == BGP_PEER_EBGP
      && (new_sort == BGP_PEER_IBGP || new_sort == BGP_PEER_CONFED))
    return 0;

  /* 8. IGP metric check. */
  newm = existm = 0;

  if (nw->extra)
    newm = nw->extra->igpmetric;
  if (exist->extra)
    existm = exist->extra->igpmetric;

  if (newm < existm)
    ret = 1;
  if (newm > existm)
    ret = 0;

  /* 9. Maximum path check. */
  if (newm == existm)
    {
      if (bgp_flag_check (bgp, BGP_FLAG_ASPATH_MULTIPATH_RELAX))
	{

	  /*
	   * For the two paths, all comparison steps till IGP metric
	   * have succeeded - including AS_PATH hop count. Since 'bgp
	   * bestpath as-path multipath-relax' knob is on, we don't need
	   * an exact match of AS_PATH. Thus, mark the paths are equal.
	   * That will trigger both these paths to get into the multipath
	   * array.
	   */
	  *paths_eq = 1;
	}
      else if (nw->peer->sort == BGP_PEER_IBGP)
	{
	  if (aspath_cmp (nw->attr->aspath, exist->attr->aspath))
	    *paths_eq = 1;
	}
      else if (nw->peer->as == exist->peer->as)
	*paths_eq = 1;
    }
  else
    {
      /*
       * TODO: If unequal cost ibgp multipath is enabled we can
       * mark the paths as equal here instead of returning
       */
      return ret;
    }

  /* 10. If both paths are external, prefer the path that was received
   first (the oldest one).  This step minimizes route-flap, since a
   newer path won't displace an older one, even if it was the
   preferred route based on the additional decision criteria below.  */
  if (!bgp_flag_check (bgp, BGP_FLAG_COMPARE_ROUTER_ID)
      && new_sort == BGP_PEER_EBGP && exist_sort == BGP_PEER_EBGP)
    {
      if (CHECK_FLAG(nw->flags, BGP_INFO_SELECTED))
	return 1;
      if (CHECK_FLAG(exist->flags, BGP_INFO_SELECTED))
	return 0;
    }

  /* 11. Rourter-ID comparision. */
  if (newattr->flag & ATTR_FLAG_BIT(BGP_ATTR_ORIGINATOR_ID))
    new_id.s_addr = newattre->originator_id.s_addr;
  else
    new_id.s_addr = nw->peer->remote_id.s_addr;
  if (existattr->flag & ATTR_FLAG_BIT(BGP_ATTR_ORIGINATOR_ID))
    exist_id.s_addr = existattre->originator_id.s_addr;
  else
    exist_id.s_addr = exist->peer->remote_id.s_addr;

  if (ntohl (new_id.s_addr) < ntohl(exist_id.s_addr))
    return 1;
  if (ntohl (new_id.s_addr) > ntohl(exist_id.s_addr))
    return 0;

  /* 12. Cluster length comparision. */
  new_cluster = exist_cluster = 0;

  if (newattr->flag & ATTR_FLAG_BIT(BGP_ATTR_CLUSTER_LIST))
    new_cluster = newattre->cluster->length;
  if (existattr->flag & ATTR_FLAG_BIT(BGP_ATTR_CLUSTER_LIST))
    exist_cluster = existattre->cluster->length;

  if (new_cluster < exist_cluster)
    return 1;
  if (new_cluster > exist_cluster)
    return 0;

  /* 13. Neighbor address comparision. */
  ret = sockunion_cmp (nw->peer->su_remote, exist->peer->su_remote);

  if (ret == 1)
    return 0;
  if (ret == -1)
    return 1;

  return 1;
}
