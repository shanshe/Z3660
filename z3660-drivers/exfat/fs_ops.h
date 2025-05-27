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

#ifndef _OPS_H
#define _OPS_H 1

/*------------------------------------------------------------------------*/

LONG OpLockFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, LONG access, struct ExtFileLock **filelock);

void OpUnlockFile(struct ExtFileLock *lock);

LONG OpCopyLock(struct ExtFileLock *lock, struct ExtFileLock **copy);

LONG OpLockParent(struct ExtFileLock *lock, struct ExtFileLock **parent);

LONG OpOpenFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, LONG action, struct ExtFileLock **filelock);

LONG OpRead(struct ExtFileLock *lock, APTR buffer, LONG want, LONG *read);
LONG OpWrite(struct ExtFileLock *lock, APTR *buffer, LONG want, LONG *written);

LONG OpAddNotify(struct NotifyRequest *nr);
LONG OpRemoveNotify(struct NotifyRequest *nr);

LONG OpExAll(struct ExtFileLock *lock, struct ExAllData *buffer, LONG size, LONG data, struct ExAllControl *control);

LONG OpFormat(UBYTE *name, ULONG dostype);

LONG OpDeleteFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen);

LONG OpCreateDir(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, struct ExtFileLock **newdirlock);

LONG OpRenameFile(struct ExtFileLock *sdirlock, UBYTE *sname, ULONG snamelen, struct ExtFileLock *ddirlock, UBYTE *dname, ULONG dnamelen);

LONG OpSetDate(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, struct DateStamp *ds);
LONG OpSetProtect(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, ULONG prot);

LONG OpSetFileSize(struct ExtFileLock *lock, LONG offset, LONG whence, LONG *newsize);
LONG OpSetFileSize64(struct ExtFileLock *lock, QUAD offset, LONG whence, QUAD *newsize);

/*------------------------------------------------------------------------*/

#endif

