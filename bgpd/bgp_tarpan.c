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

struct tarpan *
tarpan_parse (u_int32_t *pnt, u_short length)
{
  // TODO: work our protobuf magic
  struct tarpan tmp;

  tmp.length = length;
  tmp.val = pnt;

  TarpanMsg *msg = tarpan_msg__unpack(NULL, length, pnt);
  if (msg == NULL) {
      abort("tarpan: AAAAAAAAAAAAAHHHHH");
  }

  tmp.deserialized = msg;

  return tarpan_intern(&tmp);
}

static struct tarpan *
tarpan_new (void)
{
  return (struct tarpan *) XCALLOC (MTYPE_TARPAN,
				       sizeof (struct tarpan));
}

void
tarpan_free (struct tarpan *tarp)
{
  if (tarp->val)
    XFREE (MTYPE_TARPAN, tarp->val);

  XFREE (MTYPE_TARPAN, tarp);
}


struct tarpan *
tarpan_intern (struct tarpan *tarp)
{
  struct tarpan *find;

  /* Assert this tarpan structure is not interned. */
  assert (tarp->refcnt == 0);

  /* Lookup tarpan hash. */
  find = (struct tarpan *) hash_get (tarpan_hash, tarp, hash_alloc_intern);

  /* Argument tarp is allocated temporary.  So when it is not used in
     hash, it should be freed.  */
  if (find != tarp)
    tarpan_free(tarp);

  /* Increment reference counter.  */
  find->refcnt++;

  return find;
}

void
tarpan_unintern (struct tarpan **tarp)
{
  struct tarpan *ret;

  if ((*tarp)->refcnt)
    (*tarp)->refcnt--;

  /* Pull off from hash.  */
  if ((*tarp)->refcnt == 0)
    {
      /* Community value tarp must exist in hash. */
      ret = (struct tarp *) hash_release (tarpan_hash, *tarp);
      assert (ret != NULL);

      tarpan_free (*tarp);
      *tarp = NULL;
    }
}

void
tarpan_init (void)
{
  tarpan_hash = hash_create (tarpan_hash_make /* TODO: vs. tarpan_hash_key_make??? */, tarpan_cmp);
}

void
tarpan_finish (void)
{
  hash_free (tarpan_hash);
  tarpan_hash = NULL;
}

