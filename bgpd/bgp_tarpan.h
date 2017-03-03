/*
 * bgp_tarpan.h
 *
 *  Created on: Mar 3, 2017
 *      Author: Harshal Sheth
 */

#ifndef _QUAGGA_BGP_TARPAN_H
#define _QUAGGA_BGP_TARPAN_H

/* Tarpan attribute */
struct tarpan
{
  /* Reference count of tarpan value. */
  unsigned long refcnt;

  /* tarpan value size.  */
  int size;

  /* tarpan value */ // TODO only for now
  u_int32_t *val;
};

#endif /* _QUAGGA_BGP_TARPAN_H */
