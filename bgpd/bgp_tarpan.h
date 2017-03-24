/*
 * bgp_tarpan.h
 *
 *  Created on: Mar 3, 2017
 *      Author: Harshal Sheth
 */

#ifndef _QUAGGA_BGP_TARPAN_H
#define _QUAGGA_BGP_TARPAN_H

#include "bgpd/attr_tarpan.pb-c.h"

/* Tarpan attribute */
struct tarpan
{
  /* Reference count of tarpan value. */
  unsigned long refcnt;

  /* tarpan value size.  */
  u_int16_t length;

  /* tarpan value */ // TODO(tarpan) only for now
  u_int32_t *val;

  TarpanMsg deserialized = TARPAN_MSG__INIT;
};

/* TODO(tarpan) Prototypes of tarpan attribute functions.  */
extern void tarpan_init (void);
extern void tarpan_finish (void); // TODO needed
extern void tarpan_free (struct tarpan *);
extern struct tarpan *tarpan_uniq_sort (struct tarpan *);
extern struct tarpan *tarpan_parse (u_int32_t *, u_short);
extern struct tarpan *tarpan_intern (struct tarpan *); // TODO needed
extern void tarpan_unintern (struct tarpan **); // TODO needed
extern char *tarpan_str (struct tarpan *); // TODO only needed for debugging
extern unsigned int tarpan_hash_make (struct tarpan *);
extern int tarpan_match (const struct tarpan *, const struct tarpan *);
extern int tarpan_cmp (const struct tarpan *, const struct tarpan *);
extern struct tarpan *tarpan_merge (struct tarpan *, struct tarpan *);
extern struct tarpan *tarpan_delete (struct tarpan *, struct tarpan *);
extern struct tarpan *tarpan_dup (struct tarpan *);
extern int tarpan_include (struct tarpan *, u_int32_t);
extern void tarpan_del_val (struct tarpan *, u_int32_t *);
extern unsigned long tarpan_count (void);
extern struct hash *tarpan_hash (void);

#endif /* _QUAGGA_BGP_TARPAN_H */
