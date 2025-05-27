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

#ifndef _LOCK_H
#define _LOCK_H 1

/*------------------------------------------------------------------------*/

LONG TestLock(struct ExtFileLock *fl);

struct GlobalLock *FindGlobalLock(char *path);

LONG LockFileByName(struct ExtFileLock *fl, UBYTE *name, LONG namelen, LONG access, struct ExtFileLock **lock);
LONG LockFile(char *path, LONG access, struct ExtFileLock **lock);
LONG LockRoot(LONG access, struct ExtFileLock **lock);

LONG CopyLock(struct ExtFileLock *fl, struct ExtFileLock **lock);

void FreeLock(struct ExtFileLock *fl);

/*------------------------------------------------------------------------*/

#endif

