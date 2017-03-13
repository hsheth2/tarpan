/*
 * bgp_tarpan.c
 *
 *  Created on: Mar 3, 2017
 *      Author: Harshal Sheth
 */

#include <zebra.h>

#include "hash.h"
#include "memory.h"

#include "bgpd/bgp_tarpan.h"

/* Make hash value of tarpan attribute. This function is used by
   hash package.*/
unsigned int
tarpan_hash_make (struct tarpan * tarpan)
{
  unsigned char *pnt = (unsigned char *)tarpan->val;
  int size = tarpan->size * 4;
  unsigned int key = 0;
  int c;

  for (c = 0; c < size; c += 4)
    {
      key += pnt[c];
      key += pnt[c + 1];
      key += pnt[c + 2];
      key += pnt[c + 3];
    }

  return key;
}
