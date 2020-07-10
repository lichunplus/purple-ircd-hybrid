/*
 *  ircd-hybrid: an advanced, lightweight Internet Relay Chat Daemon (ircd)
 *
 *  Copyright (c) 1997-2020 ircd-hybrid development team
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

/*! \file memory.h
 * \brief A header for the memory functions.
 * \version $Id: memory.h 9102 2020-01-01 09:58:57Z michael $
 */

#ifndef INCLUDED_memory_h
#define INCLUDED_memory_h

extern void outofmemory(void);
extern void *xcalloc(size_t);
extern void *xrealloc(void *, size_t);
extern void xfree(void *);
extern void *xstrdup(const char *);
extern void *xstrndup(const char *, size_t);
#endif /* INCLUDED_memory_h */
