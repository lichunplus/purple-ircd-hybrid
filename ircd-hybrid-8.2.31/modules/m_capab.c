/*
 *  ircd-hybrid: an advanced, lightweight Internet Relay Chat Daemon (ircd)
 *
 *  Copyright (c) 1998-2020 ircd-hybrid development team
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

/*! \file m_capab.c
 * \brief Includes required functions for processing the CAPAB command.
 * \version $Id: m_capab.c 9102 2020-01-01 09:58:57Z michael $
 */

#include "stdinc.h"
#include "client.h"
#include "irc_string.h"
#include "server_capab.h"
#include "parse.h"
#include "modules.h"


/*! \brief CAPAB command handler
 *
 * \param source_p Pointer to allocated Client struct from which the message
 *                 originally comes from.  This can be a local or remote client.
 * \param parc     Integer holding the number of supplied arguments.
 * \param parv     Argument vector where parv[0] .. parv[parc-1] are non-NULL
 *                 pointers.
 * \note Valid arguments for this command are:
 *      - parv[0] = command
 *      - parv[1] = space-separated list of capabilities
 */
static void
mr_capab(struct Client *source_p, int parc, char *parv[])
{
  unsigned int cap = 0;
  char *p = NULL;

  for (const char *s = strtok_r(parv[1], " ", &p); s;
                   s = strtok_r(NULL,    " ", &p))
    if ((cap = capab_find(s)))
      SetCapable(source_p, cap);
}

static struct Message capab_msgtab =
{
  .cmd = "CAPAB",
  .args_min = 2,
  .args_max = MAXPARA,
  .handlers[UNREGISTERED_HANDLER] = mr_capab,
  .handlers[CLIENT_HANDLER] = m_ignore,
  .handlers[SERVER_HANDLER] = m_ignore,
  .handlers[ENCAP_HANDLER] = m_ignore,
  .handlers[OPER_HANDLER] = m_ignore
};

static void
module_init(void)
{
  mod_add_cmd(&capab_msgtab);
}

static void
module_exit(void)
{
  mod_del_cmd(&capab_msgtab);
}

struct module module_entry =
{
  .version = "$Revision: 9102 $",
  .modinit = module_init,
  .modexit = module_exit,
};
