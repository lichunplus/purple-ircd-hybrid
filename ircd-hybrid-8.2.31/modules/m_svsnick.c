/*
 *  ircd-hybrid: an advanced, lightweight Internet Relay Chat Daemon (ircd)
 *
 *  Copyright (c) 1999 Bahamut development team.
 *  Copyright (c) 2011-2020 ircd-hybrid development team
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

/*! \file m_svsnick.c
 * \brief Includes required functions for processing the SVSNICK command.
 * \version $Id: m_svsnick.c 9275 2020-02-12 18:46:44Z michael $
 */


#include "stdinc.h"
#include "client.h"
#include "ircd.h"
#include "channel.h"
#include "channel_mode.h"
#include "send.h"
#include "parse.h"
#include "modules.h"
#include "irc_string.h"
#include "user.h"
#include "hash.h"
#include "watch.h"
#include "whowas.h"


/*! \brief SVSNICK command handler
 *
 * \param source_p Pointer to allocated Client struct from which the message
 *                 originally comes from.  This can be a local or remote client.
 * \param parc     Integer holding the number of supplied arguments.
 * \param parv     Argument vector where parv[0] .. parv[parc-1] are non-NULL
 *                 pointers.
 * \note Valid arguments for this command are:
 *      - parv[0] = command
 *      - parv[1] = old nickname
 *      - parv[2] = old timestamp
 *      - parv[3] = new nickname
 *      - parv[4] = new timestamp
 */
static void
ms_svsnick(struct Client *source_p, int parc, char *parv[])
{
  const char *new_nick = parc == 5 ? parv[3] : parv[2];  /* TBR: compatibility mode */
  uintmax_t ts = 0, new_ts = 0;

  if (!HasFlag(source_p, FLAGS_SERVICE))
    return;

  if (valid_nickname(new_nick, true) == false)
    return;

  struct Client *target_p = find_person(source_p, parv[1]);
  if (target_p == NULL)
    return;

  if (parc == 5)  /* TBR: compatibility mode */
  {
    ts = strtoumax(parv[2], NULL, 10);
    if (ts && (ts != target_p->tsinfo))
      return;
  }
  else  /* parc == 4 */
    ts = strtoumax(parv[3], NULL, 10);

  if (parc == 4)  /* TBR: compatibility mode */
    new_ts = ts;
  else
    new_ts = strtoumax(parv[4], NULL, 10);

  if (!MyConnect(target_p))
  {
    if (target_p->from == source_p->from)
    {
      sendto_realops_flags(UMODE_DEBUG, L_ALL, SEND_NOTICE,
                           "Received wrong-direction SVSNICK "
                           "for %s (behind %s) from %s",
                           target_p->name, source_p->from->name,
                           client_get_name(source_p, HIDE_IP));
      return;
    }

    sendto_one(target_p, ":%s SVSNICK %s %s %s", source_p->id,
               target_p->id, new_nick, parv[3]);
    return;
  }

  struct Client *exists_p = hash_find_client(new_nick);
  if (exists_p)
  {
    if (target_p == exists_p)
    {
      if (strcmp(target_p->name, new_nick) == 0)
        return;
    }
    else if (IsUnknown(exists_p))
      exit_client(exists_p, "SVSNICK Override");
    else
    {
      exit_client(target_p, "SVSNICK Collide");
      return;
    }
  }

  target_p->tsinfo = new_ts;
  clear_ban_cache_list(&target_p->channel);
  watch_check_hash(target_p, RPL_LOGOFF);

  if (HasUMode(target_p, UMODE_REGISTERED))
  {
    const unsigned int oldmodes = target_p->umodes;
    char buf[UMODE_MAX_STR] = "";

    DelUMode(target_p, UMODE_REGISTERED);
    send_umode(target_p, true, oldmodes, buf);
  }

  sendto_common_channels_local(target_p, true, 0, 0, ":%s!%s@%s NICK :%s",
                               target_p->name, target_p->username,
                               target_p->host, new_nick);

  whowas_add_history(target_p, true);

  sendto_server(NULL, 0, 0, ":%s NICK %s :%ju",
                target_p->id, new_nick, target_p->tsinfo);

  hash_del_client(target_p);
  strlcpy(target_p->name, new_nick, sizeof(target_p->name));
  hash_add_client(target_p);

  watch_check_hash(target_p, RPL_LOGON);

  fd_note(target_p->connection->fd, "Nick: %s", target_p->name);
}

static struct Message svsnick_msgtab =
{
  .cmd = "SVSNICK",
  .args_min = 4,
  .args_max = MAXPARA,
  .handlers[UNREGISTERED_HANDLER] = m_ignore,
  .handlers[CLIENT_HANDLER] = m_ignore,
  .handlers[SERVER_HANDLER] = ms_svsnick,
  .handlers[ENCAP_HANDLER] = m_ignore,
  .handlers[OPER_HANDLER] = m_ignore
};

static void
module_init(void)
{
  mod_add_cmd(&svsnick_msgtab);
}

static void
module_exit(void)
{
  mod_del_cmd(&svsnick_msgtab);
}

struct module module_entry =
{
  .version = "$Revision: 9275 $",
  .modinit = module_init,
  .modexit = module_exit,
};
