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

  tarp->message = malloc(sizeof(TarpanMsg));
  tarpan_msg__init(tarp->message);
  tarp->message->version = 1337;

  return tarp;
}

/* Make hash value of tarpan attribute. This function is used by
   hash package.*/
unsigned int
tarpan_hash_make (struct tarpan * tarpan)
{
  return (uintptr_t) tarpan->message;
}

int
tarpan_cmp (const struct tarpan *p1, const struct tarpan *p2)
{
  return p1->message == p2->message; // compare pointers
}

struct tarpan *
tarpan_parse (u_int8_t *pnt, u_short length)
{
  // work our protobuf magic
  struct tarpan* tmp = tarpan_new();

  TarpanMsg *msg = tarpan_msg__unpack(NULL, length, pnt);
  if (msg == NULL) {
      zlog_err("tarpan: AAAAAAAAH!");
      abort();
  }

  tmp->message = msg;

  zlog_info("Received tarpan packet, version %d", msg->version);

  return tarpan_intern(tmp);
}

void
tarpan_free (struct tarpan *tarp)
{
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

