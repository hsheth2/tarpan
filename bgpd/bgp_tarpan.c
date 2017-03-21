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

static struct hash *tarpan_hash;

/* Make hash value of tarpan attribute. This function is used by
   hash package.*/
unsigned int
tarpan_hash_make (struct tarpan * tarpan)
{
  unsigned char *pnt = (unsigned char *)tarpan->val;
  int size = tarpan->length * 4;
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

int
tarpan_cmp (const void *p1, const void *p2)
{
  const struct tarpan * tarpan1 = p1;
  const struct tarpan * tarpan2 = p2;

  return (tarpan1->length == tarpan2->length &&
	  memcmp (tarpan1->val, tarpan2->val, tarpan1->length) == 0);
}

void
tarpan_init (void)
{
  tarpan_hash = hash_create (tarpan_hash_make /* TODO: vs. tarpan_hash_key_make??? */, tarpan_cmp);
}
