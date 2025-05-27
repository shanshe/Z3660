/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright © 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright © 2006 Marek Szyprowski
  Copyright © 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright © 2010-2013 Andrew Nayenko


  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

--------------------------------------------------------------------------*/

#ifndef _OS3_H
#define _OS3_H 1

/*------------------------------------------------------------------------*/

typedef signed long long QUAD;
typedef unsigned long long UQUAD;

/*------------------------------------------------------------------------*/

#define SWAPWORD(x) \
	((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#define SWAPLONG(x) \
	((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define SWAPQUAD(x) \
	((((x) & 0xff00000000000000ULL) >> 56) | \
	 (((x) & 0x00ff000000000000ULL) >> 40) | \
	 (((x) & 0x0000ff0000000000ULL) >> 24) | \
	 (((x) & 0x000000ff00000000ULL) >>  8) | \
	 (((x) & 0x00000000ff000000ULL) <<  8) | \
	 (((x) & 0x0000000000ff0000ULL) << 24) | \
	 (((x) & 0x000000000000ff00ULL) << 40) | \
	 (((x) & 0x00000000000000ffULL) << 56))

/*------------------------------------------------------------------------*/

APTR AllocVecPooled(APTR pool, ULONG size);
void FreeVecPooled(APTR pool, APTR data);

/*------------------------------------------------------------------------*/

LONG DoTimer(struct timeval *tv, LONG unit, LONG cmd);
LONG TimeDelay(LONG unit, ULONG secs, ULONG micro);

/*------------------------------------------------------------------------*/

VOID kputs(CONST_STRPTR string);
VOID kprintf(CONST_STRPTR formatString, ...);

/*------------------------------------------------------------------------*/

#endif

