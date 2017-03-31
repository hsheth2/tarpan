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
#include "bgpd/attr_tarpan.pb-c.h"

#include "log.h"

static struct hash *tarpan_hash;

static struct tarpan *
tarpan_new (void)
{
  return (struct tarpan *) XCALLOC (MTYPE_TARPAN,
				       sizeof (struct tarpan));
}

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
tarpan_cmp (const struct tarpan *p1, const struct tarpan *p2)
{
  const struct tarpan * tarpan1 = p1;
  const struct tarpan * tarpan2 = p2;

  return (tarpan1->length == tarpan2->length &&
	  memcmp (tarpan1->val, tarpan2->val, tarpan1->length) == 0);
}

struct tarpan *
tarpan_parse (u_int32_t *pnt, u_short length)
{
  // work our protobuf magic
  struct tarpan* tmp = tarpan_new();

  tmp->length = length;
  tmp->val = pnt;

  TarpanMsg *msg = tarpan_msg__unpack(NULL, length, pnt);
  if (msg == NULL) {
      zlog_err("tarpan: AAAAAAAAH!");
      abort();
  }

  tmp->deserialized = msg;

  zlog_info("Received tarpan packet, version %d", msg->version);

  return tarpan_intern(tmp);
}

void
tarpan_free (struct tarpan *tarp)
{
  if (tarp->val)
    XFREE (MTYPE_TARPAN, tarp->val);

  if (tarp->deserialized)
    tarpan_msg__free_unpacked(tarp->deserialized, NULL);

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
      /* Tarpan value tarp must exist in hash. */
      ret = (struct tarp *) hash_release (tarpan_hash, *tarp);
      assert (ret != NULL);

      tarpan_free (*tarp);
      *tarp = NULL;
    }
}

void
tarpan_mark_modified (struct tarpan *tarp)
{
  if (tarp->val)
    {
      XFREE (MTYPE_TARPAN, tarp->val);
      tarp->val = NULL;
      tarp->length = 0;
    }
}

void
tarpan_reserialize (struct tarpan *tarp)
{
  if (tarp->val)
    tarpan_mark_modified(tarp);

  // reserialize
  tarp->length = tarpan_msg__get_packed_size(tarp->deserialized);
  tarp->val = XMALLOC(MTYPE_BGP, tarp->length);
  tarpan_msg__pack(tarp->deserialized, tarp->val);
}

char *
tarpan_str (struct tarpan *tarp)
{
  return NULL;
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

