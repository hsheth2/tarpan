/*
 * bgp_tarpan.c
 *
 *  Created on: Mar 3, 2017
 *      Author: Harshal Sheth
 */

#include <zebra.h>

#include "hash.h"
#include "memory.h"
#include "log.h"

#include "bgpd/common.h"
#include "bgpd/bgp_tarpan.h"
#include "bgpd/attr_tarpan.pb-c.h"

// include possible protocols
#include "wiser.h"
// protocol initialization functions are called and
// active protocol is  set in `tarpan_protocol_init`

static struct hash *tarpan_hash;

/* the currently active protocol handler */
struct tarpan_protocol_handler * tarpan_active_handler = NULL;

void tarpan_set_protocol_handler(struct tarpan_protocol_handler * handler) {
  tarpan_active_handler = handler;
}

void tarpan_protocol_init(void)
{
  wiser_protocol_init();

  tarpan_set_protocol_handler(&wiser_protocol_handler);
}

struct tarpan *
tarpan_new (void)
{
  struct tarpan * tarp = (struct tarpan *) XCALLOC (MTYPE_TARPAN,
						       sizeof (struct tarpan));

  zlog_debug("tarpan_new %p %lu", tarp, tarp->refcnt);

  return tarp;
}

struct tarpan *
tarpan_new_default (void)
{
  struct tarpan * tarp = (struct tarpan *) XCALLOC (MTYPE_TARPAN,
						       sizeof (struct tarpan));

  tarp->message = malloc(sizeof(TarpanMsg));
  tarpan_msg__init(tarp->message);
  tarp->message->version = 12;

  zlog_debug("tarpan_new_default %p %lu", tarp, tarp->refcnt);

  return tarp;
}

/* Make hash value of tarpan attribute. This function is used by
   hash package.*/
unsigned int
tarpan_hash_make (struct tarpan * tarpan)
{
  zlog_debug("tarpan_hash_make %p %lu", tarpan, tarpan->refcnt);
  return (uintptr_t) tarpan->message;
}

int
tarpan_cmp (const struct tarpan *p1, const struct tarpan *p2)
{
  if (p1->message == p2->message) // compare pointers
    return 1;
  else
    return 0;
}

struct tarpan *
tarpan_parse (u_int8_t *pnt, u_short length)
{
  // work our protobuf magic
  struct tarpan* tmp = tarpan_new();

  TarpanMsg *msg = tarpan_msg__unpack(NULL, length, pnt);
  if (msg == NULL) {
      zlog_err("tarpan: AAAAAAAAH!");
  }
  assert(msg != NULL);

  tmp->message = msg;

  zlog_info("Received tarpan packet, version %d", msg->version);

  struct tarpan * ret = tarpan_intern(tmp);

  zlog_debug("tarpan_parse ret = %p %lu", ret, ret->refcnt);

  return ret;
}

void
tarpan_free (struct tarpan *tarp)
{
  zlog_debug("tarpan_free %p %lu", tarp, tarp->refcnt);

  if (tarp->message)
    tarpan_msg__free_unpacked(tarp->message, NULL);

  XFREE (MTYPE_TARPAN, tarp);
}

struct tarpan *
tarpan_intern (struct tarpan *tarp)
{
  struct tarpan *find;

  /* Assert this tarpan structure is not interned. */
  assert (tarp->refcnt == 0);

  zlog_debug("tarpan_intern tarp = %p %lu", tarp, tarp->refcnt);

  /* Lookup tarpan hash. */
  find = (struct tarpan *) hash_get (tarpan_hash, tarp, hash_alloc_intern);

  /* Argument tarp is allocated temporary.  So when it is not used in
     hash, it should be freed.  */
  if (find != tarp)
    tarpan_free(tarp);

  /* Increment reference counter.  */
  find->refcnt++;

  zlog_debug("tarpan_intern find = %p %lu", find, find->refcnt);

  return find;
}

void
tarpan_unintern (struct tarpan **tarp)
{
  void *ret;

  zlog_debug("tarpan_unintern (*tarp) = %p %lu", (*tarp), (*tarp)->refcnt);

  if ((*tarp)->refcnt)
    (*tarp)->refcnt--;

  zlog_debug("tarpan_unintern (*tarp) = %p %lu", (*tarp), (*tarp)->refcnt);

  /* Pull off from hash.  */
  if ((*tarp)->refcnt == 0)
    {
      /* Tarpan value tarp must exist in hash. */
      ret = hash_release (tarpan_hash, (void*) *tarp);
      assert (ret != NULL);

      tarpan_free (*tarp);
      *tarp = NULL;
    }
}

char *
tarpan_str (struct tarpan *tarp)
{
  zlog_debug("tarpan_str %p %lu", tarp, tarp->refcnt);

  return NULL;
}

void
tarpan_init (void)
{
  tarpan_hash = hash_create ((unsigned int (*) (void*)) tarpan_hash_make,
			     (int (*) (const void*, const void*)) tarpan_cmp);
}

void
tarpan_finish (void)
{
  hash_free (tarpan_hash);
  tarpan_hash = NULL;
}

