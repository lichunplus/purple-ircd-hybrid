/*
 *  ircd-hybrid: an advanced, lightweight Internet Relay Chat Daemon (ircd)
 *
 *  Copyright (c) 1997 Jukka Santala (Donwulff)
 *  Copyright (c) 2005-2020 ircd-hybrid development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 *  USA
 */

/*! \file watch.c
 * \brief File including functions for WATCH support
 * \version $Id: watch.c 9249 2020-02-01 13:35:32Z michael $
 */

#include "stdinc.h"
#include "list.h"
#include "memory.h"
#include "client.h"
#include "hash.h"
#include "irc_string.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "watch.h"


static dlink_list watchTable[HASHSIZE];


/*
 * Rough figure of the datastructures for watch:
 *
 * NOTIFY HASH        client1
 *   |                  |- nick1
 * nick1-|- client1     |- nick2
 *   |   |- client2               client3
 *   |   |- client3   client2       |- nick1
 *   |                  |- nick1
 * nick2-|- client2     |- nick2
 *       |- client1
 */

/*! \brief Counts up memory used by watch list headers
 */
void
watch_count_memory(unsigned int *const count, size_t *const bytes)
{
  for (unsigned int i = 0; i < HASHSIZE; ++i)
    (*count) += dlink_list_length(&watchTable[i]);

  (*bytes) = *count * sizeof(struct Watch);
}

/*! \brief Notifies all clients that have client's name on
 *         their watch list.
 * \param client Pointer to Client struct
 * \param reply  Numeric to send. Either RPL_LOGON or RPL_LOGOFF
 */
void
watch_check_hash(const struct Client *client, const enum irc_numerics reply)
{
  struct Watch *watch = NULL;
  dlink_node *node = NULL;

  assert(IsClient(client));

  if ((watch = watch_find_hash(client->name)) == NULL)
    return;  /* This name isn't on watch */

  /* Update the time of last change to item */
  watch->lasttime = event_base->time.sec_real;

  /* Send notifies out to everybody on the list in header */
  DLINK_FOREACH(node, watch->watched_by.head)
    sendto_one_numeric(node->data, &me, reply, client->name,
                       client->username, client->host,
                       watch->lasttime, client->info);
}

/*! \brief Looks up the watch table for a given name
 * \param name Nick name to look up
 */
struct Watch *
watch_find_hash(const char *name)
{
  dlink_node *node = NULL;

  DLINK_FOREACH(node, watchTable[strhash(name)].head)
  {
    struct Watch *watch = node->data;

    if (irccmp(watch->name, name) == 0)
      return watch;
  }

  return NULL;
}

/*! \brief Adds a watch entry to client's watch list
 * \param name   Nick name to add
 * \param client Pointer to Client struct
 */
void
watch_add_to_hash_table(const char *name, struct Client *client)
{
  struct Watch *watch = NULL;
  dlink_node *node = NULL;

  /* If found NULL (no header for this name), make one... */
  if ((watch = watch_find_hash(name)) == NULL)
  {
    watch = xcalloc(sizeof(*watch));

    strlcpy(watch->name, name, sizeof(watch->name));
    watch->hash_value = strhash(watch->name);
    watch->lasttime = event_base->time.sec_real;

    dlinkAdd(watch, &watch->node, &watchTable[watch->hash_value]);
  }
  else
  {
    /* Is this client already on the watch-list? */
    node = dlinkFind(&watch->watched_by, client);
  }

  if (node == NULL)
  {
    /* No it isn't, so add it in the bucket and client adding it */
    dlinkAdd(client, make_dlink_node(), &watch->watched_by);
    dlinkAdd(watch, make_dlink_node(), &client->connection->watches);
  }
}

/*! \brief Removes a single entry from client's watch list
 * \param name   Name to remove
 * \param client Pointer to Client struct
 */
void
watch_del_from_hash_table(const char *name, struct Client *client)
{
  struct Watch *watch = NULL;
  dlink_node *node = NULL;

  if ((watch = watch_find_hash(name)) == NULL)
    return;  /* No header found for that name. i.e. it's not being watched */

  if ((node = dlinkFind(&watch->watched_by, client)) == NULL)
    return;  /* This name isn't being watched by client */

  dlinkDelete(node, &watch->watched_by);
  free_dlink_node(node);

  if ((node = dlinkFindDelete(&client->connection->watches, watch)))
    free_dlink_node(node);

  /* In case this header is now empty of notices, remove it */
  if (watch->watched_by.head == NULL)
  {
    assert(dlinkFind(&watchTable[watch->hash_value], watch));
    dlinkDelete(&watch->node, &watchTable[watch->hash_value]);
    xfree(watch);
  }
}

/*! \brief Removes all entries from client's watch list
 *         and deletes headers that are no longer being watched.
 * \param client Pointer to Client struct
 */
void
watch_del_watch_list(struct Client *client)
{
  dlink_node *node = NULL, *node_next = NULL;
  dlink_node *temp = NULL;

  DLINK_FOREACH_SAFE(node, node_next, client->connection->watches.head)
  {
    struct Watch *watch = node->data;

    assert(dlinkFind(&watch->watched_by, client));

    if ((temp = dlinkFindDelete(&watch->watched_by, client)))
      free_dlink_node(temp);

    /* If this leaves a header without notifies, remove it. */
    if (watch->watched_by.head == NULL)
    {
      assert(dlinkFind(&watchTable[watch->hash_value], watch));
      dlinkDelete(&watch->node, &watchTable[watch->hash_value]);

      xfree(watch);
    }

    dlinkDelete(node, &client->connection->watches);
    free_dlink_node(node);
  }

  assert(client->connection->watches.head == NULL);
  assert(client->connection->watches.tail == NULL);
}
