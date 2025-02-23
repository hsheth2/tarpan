/*
 * bgp_tarpan.h
 *
 *  Created on: Mar 3, 2017
 *      Author: Harshal Sheth
 */

#ifndef _QUAGGA_BGP_TARPAN_H
#define _QUAGGA_BGP_TARPAN_H

#include <sys/types.h>

#include "bgpd/common.h"
#include "bgpd/attr_tarpan.pb-c.h"

#define TARPAN_VERSION 12

/* Tarpan attribute */
struct tarpan
{
  /* Reference count of tarpan value. */
  unsigned long refcnt;

  TarpanMsg * message;
};

/* tarpan protocol API */
struct tarpan_protocol_handler
{
  /* functions that must be implemented */

  // called within tarpan_parse
  void
  (*packet_received_handler) (struct peer * const peer,
			      struct attr * const attr);

  // called after calling tarpan_new_default
  void
  (*initialize_packet) (struct peer * const peer, struct tarpan * tarpan);

  // called when forwarding an existing tarpan attribute
  void
  (*update_packet) (struct peer * const peer, struct tarpan * tarpan);

  /* optional functions */
  int
  (*protocol_info_cmp) (struct bgp *bgp, struct bgp_info *nw,
			struct bgp_info *exist, int *paths_eq);
};

/* the currently active protocol handler */
extern struct tarpan_protocol_handler * tarpan_active_handler;

extern void
tarpan_set_protocol_handler (struct tarpan_protocol_handler * handler);
extern void
tarpan_protocol_init (void);

/* (tarpan) Prototypes of tarpan attribute functions.  */
// note: not all of these are actually implemented
extern struct tarpan *
tarpan_new (void);
extern TarpanMsg*
tarpan_message_new(void);
extern void
tarpan_initialize_packet (struct peer * const peer, struct tarpan * tarpan);
extern struct tarpan *
tarpan_update_packet (struct peer * const peer, struct tarpan * tarp);
extern void
tarpan_init (void);
extern void
tarpan_finish (void);
extern void
tarpan_free (struct tarpan *);
extern struct tarpan *
tarpan_uniq_sort (struct tarpan *);
extern struct tarpan *
tarpan_parse (u_int8_t *, u_short);
extern struct tarpan *
tarpan_intern (struct tarpan *);
extern void
tarpan_unintern (struct tarpan **);
extern char *
tarpan_str (struct tarpan *); // (only needed for debugging)
extern unsigned int
tarpan_hash_make (const struct tarpan *);
extern int
tarpan_match (const struct tarpan *, const struct tarpan *);
extern int
tarpan_cmp (const struct tarpan *, const struct tarpan *);
extern struct tarpan *
tarpan_merge (struct tarpan *, struct tarpan *);
extern struct tarpan *
tarpan_delete (struct tarpan *, struct tarpan *);
extern struct tarpan *
tarpan_dup (struct tarpan *);
extern int
tarpan_include (struct tarpan *, u_int32_t);
extern void
tarpan_del_val (struct tarpan *, u_int32_t *);
extern unsigned long
tarpan_count (void);

#endif /* _QUAGGA_BGP_TARPAN_H */
