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

#ifndef _SUPPORT_H
#define _SUPPORT_H 1

/*------------------------------------------------------------------------*/

enum {
	LD, LW, LE, LF
};
void Log(short mode, const char *fmt, ...);
void Pop(short mode, const char *fmt, ...);

/*------------------------------------------------------------------------*/

ULONG GetBlockSize(struct DosEnvec *de);
UQUAD GetNumBlocks(struct DosEnvec *de);
UQUAD GetLowBlock(struct DosEnvec *de);
UQUAD GetHighBlock(struct DosEnvec *de);

/*------------------------------------------------------------------------*/

void SendEvent(LONG event);

/*------------------------------------------------------------------------*/

void dec2hex(ULONG dec, char *hex);

void cstr2bstr_buf(const char *c, char *b, int limit);
void bstr2cstr_buf(const char *b, char *c, int limit);
char *bstr2cstr(const char *b);

/*------------------------------------------------------------------------*/

char *ParentPath(const char *s);
char *MakePath(struct ExtFileLock *fl, const char *name, LONG *error);

/*------------------------------------------------------------------------*/

#endif

