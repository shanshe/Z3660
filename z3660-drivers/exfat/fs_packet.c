/*--------------------------------------------------------------------------

  exFAT filesystem for MorphOS:
  Copyright � 2014-2015 Rupert Hausberger

  FAT12/16/32 filesystem handler:
  Copyright � 2006 Marek Szyprowski
  Copyright � 2007-2008 The AROS Development Team

  exFAT implementation library:
  Copyright � 2010-2013 Andrew Nayenko


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

#include "include.h"

/*------------------------------------------------------------------------*/

static inline UBYTE *offset2name(LONG offset) {
	if (offset == OFFSET_BEGINNING) return "OFFSET_BEGINNING";
	if (offset == OFFSET_END) return "OFFSET_END";
	if (offset == OFFSET_CURRENT) return "OFFSET_CURRENT";
	return "<unknown>";
}

static inline UBYTE *mode2name(LONG access) {
	if (access == MODE_OLDFILE) return "MODE_OLDFILE";
	if (access == MODE_NEWFILE) return "MODE_NEWFILE";
	if (access == MODE_READWRITE) return "MODE_READWRITE";
	return "<unknown>";
}           

static inline UBYTE *access2name(LONG access) {
	if (access == ACCESS_READ) return "SHARED";
	if (access == ACCESS_WRITE) return "EXCLUSIVE";
	return "<unknown>";
}

/*------------------------------------------------------------------------*/

#define DOSERROR(error) (error == 0 ? DOSTRUE : DOSFALSE)

static inline void SetResult(struct DosPacket *pkt, LONG result, LONG error)
{
	pkt->dp_Res1 = result;
	pkt->dp_Res2 = error;
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/* internal */

#ifdef __MORPHOS__
static void _GetDiskFSSM(struct DosPacket *pkt)
{
	//Log(LD, "_GetDiskFSSM()\n");
	SetResult(pkt, (LONG)global->g_FSSM, 0l);
}

static void _FreeDiskFSSM(struct DosPacket *pkt)
{
	//Log(LD, "_FreeDiskFSSM()\n");
	SetResult(pkt, DOSTRUE, 0l);
}
#endif

/*------------------------------------------------------------------------*/
/* handler */

static void _Die(struct DosPacket *pkt)
{
	//Log(LD, "_Die()\n");

	/* clear our message port from notification requests so DOS won't send notification-end packets to us after we're gone */
	{
		struct Volume *v;
		struct NotifyNode *nn;

		ForeachNode(&global->g_BusyVolumes, v) {
			ForeachNode(&v->v_Info->vi_Notifies, nn)
				nn->nn_NR->nr_Handler = NULL;
		}
	}

	if ((global->g_Volume != NULL && !(IsListEmpty((struct List *)&global->g_Volume->v_Info->vi_Locks) && IsListEmpty((struct List *)&global->g_Volume->v_Info->vi_Notifies)))) {
		//Log(LD, "There are remaining locks or notification requests. Shutting down is not possible\n");
		SetResult(pkt, DOSFALSE, ERROR_OBJECT_IN_USE);
		return;
	}
	//Log(LD, "Nothing pending. Shutting down the handler\n");

	DoDiskRemove();

	global->g_Quit = TRUE;
	global->g_DeathPacket = pkt;
	global->g_DevNode->dol_Task = NULL;

	SetResult(pkt, DOSTRUE, 0l);
}

static void _AddBuffers(struct DosPacket *pkt)
{
	//LONG buffers = pkt->dp_Arg1;

	//Log(LD, "_AddBuffers(%ld)\n", buffers);
	SetResult(pkt, DOSTRUE, 0l);
}

static void _Flush(struct DosPacket *pkt)
{
	//Log(LD, "_Flush()\n");
	SetResult(pkt, DOSTRUE, 0l);
}

static void _Inhibit(struct DosPacket *pkt)
{
	BOOL inhibit = pkt->dp_Arg1 == DOSTRUE;

	//Log(LD, "_Inhibit(%s)\n", inhibit ? "on" : "off");

	if (inhibit) {
		global->g_DiskInhibited++;
		if (global->g_DiskInhibited == 1)
			DoDiskRemove();
	}
	else if (global->g_DiskInhibited) {
		global->g_DiskInhibited--;
		if (global->g_DiskInhibited == 0)
			ProcessDiskChange();
	}
	SetResult(pkt, DOSTRUE, 0l);
}

static void _IsFileSystem(struct DosPacket *pkt)
{
	//Log(LD, "_IsFileSystem()\n");
	SetResult(pkt, DOSTRUE, 0l);
}


/*------------------------------------------------------------------------*/
/* basic */

static void _Write(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
#ifndef __READ_ONLY__
	APTR buffer = (APTR)pkt->dp_Arg2;
	LONG length = pkt->dp_Arg3;
	LONG written;
#endif
	LONG error;

	//Log(LD, "_Write(%p, %p, %ld)\n", lock, buffer, length);

	if ((error = TestLock(lock))) {
		SetResult(pkt, -1, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if ((error = OpWrite(lock, buffer, length, &written)) != 0)
		written = -1;

	SetResult(pkt, written, error);
#endif
}

static void _Read(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	APTR buffer = (APTR)pkt->dp_Arg2;
	LONG length = pkt->dp_Arg3;
	LONG error, read;

	//Log(LD, "_Read(%p, %p, %ld)\n", lock, buffer, length);

	if ((error = TestLock(lock))) {
		SetResult(pkt, -1, error);
		return;
	}
	if ((error = OpRead(lock, buffer, length, &read)) != 0)
		read = -1;

	SetResult(pkt, read, error);
}

static void _Seek(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	QUAD position = (QUAD)pkt->dp_Arg2;
	LONG offset = pkt->dp_Arg3;
	QUAD old, filesize;
	LONG error;

	//Log(LD, "_Seek(%p, %lld, %s)\n", lock, position, offset2name(offset));

	if ((error = TestLock(lock))) {
		SetResult(pkt, -1, error);
		return;
	}
	filesize = (QUAD)MIN(lock->fl_GL->gl_EN->en_Size, 0x7fffffff);
	old = lock->fl_Pos;

	if (offset == OFFSET_BEGINNING && position >= 0 && position <= filesize)
		lock->fl_Pos = (QUAD)position;
	else if (offset == OFFSET_CURRENT && lock->fl_Pos + position  >= 0 && lock->fl_Pos + position <= filesize)
		lock->fl_Pos += position;
	else if (offset == OFFSET_END && position <= 0 && filesize + position >= 0)
		lock->fl_Pos = filesize + position;
	else
		error = ERROR_SEEK_ERROR;

	if (error == 0) {
		SetResult(pkt, (LONG)(old & 0x7fffffff), 0);
	} else
		SetResult(pkt, -1, error);
}
#ifdef __MORPHOS__
static void _Seek64(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	QUAD position = *((QUAD *)pkt->dp_Arg2);
	LONG offset = pkt->dp_Arg3;
	QUAD *oldPosition = (QUAD *)pkt->dp_Arg4;
	QUAD old, filesize;
	LONG error;

	//Log(LD, "_Seek64(%p, %lld, %s, %p)\n", lock, position, offset2name(offset), oldPosition);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	filesize = (QUAD)lock->fl_GL->gl_EN->en_Size;
	old = lock->fl_Pos;

	if (offset == OFFSET_BEGINNING && position >= 0 && position <= filesize)
		lock->fl_Pos = position;
	else if (offset == OFFSET_CURRENT && lock->fl_Pos + position >= 0 && lock->fl_Pos + position <= filesize)
		lock->fl_Pos += position;
	else if (offset == OFFSET_END && position <= 0 && filesize + position >= 0)
		lock->fl_Pos = filesize + position;
	else
		error = ERROR_SEEK_ERROR;

	if (error == 0) {
		*oldPosition = old;
		SetResult(pkt, DOSTRUE, 0);
	} else
		SetResult(pkt, DOSFALSE, error);
}
#endif

static void _Open(struct DosPacket *pkt)
{
	struct FileHandle *fh = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
	char *bname = BADDR(pkt->dp_Arg3), *name;
	LONG access = pkt->dp_Type == ACTION_FINDINPUT ? MODE_OLDFILE : (pkt->dp_Type == ACTION_FINDOUTPUT ? MODE_NEWFILE : MODE_READWRITE);
	struct ExtFileLock *newLock;
	LONG error;

	//Log(LD, "_Open(%p, '%s', %s)\n", lock, name, mode2name(access));

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	if (access != MODE_OLDFILE) {
		SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
		return;
	}
#endif

	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	error = OpOpenFile(lock, (UBYTE *)name, strlen(name), access, &newLock);
	FreeVecPooled(global->g_Pool, name);

	if (error)
		SetResult(pkt, DOSFALSE, error);
	else {
		fh->fh_Arg1 = MKBADDR(newLock);
		fh->fh_Port = DOSFALSE;
		SetResult(pkt, DOSTRUE, 0);
	}
}

static void _Close(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	LONG error;

	//Log(LD, "_Close(%p)\n", lock);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	FreeLock(lock);
	SetResult(pkt, DOSTRUE, 0);
}

static void _SetFileSize(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
#ifndef __READ_ONLY__
	LONG position = pkt->dp_Arg2;
	LONG offset = pkt->dp_Arg3;
	LONG newSize;
#endif
	LONG error;

	//Log(LD, "_SetFileSize(%p, %ld, %s)\n", lock, position, offset2name(offset));

	if ((error = TestLock(lock))) {
		SetResult(pkt, -1, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if ((error = OpSetFileSize(lock, position, offset, &newSize))) {
		SetResult(pkt, -1, error);
		return;
	}
	SetResult(pkt, newSize, 0l);
#endif
}
#ifdef __MORPHOS__
static void _SetFileSize64(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
#ifndef __READ_ONLY__
	QUAD position = *((QUAD *)pkt->dp_Arg2);
	LONG offset = pkt->dp_Arg3;
	QUAD *newSize = (QUAD *)pkt->dp_Arg4;
#endif
	LONG error;

	//Log(LD, "_SetFileSize64(%p, %lld, %s, %p)\n", lock, position, offset2name(offset), newSize);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if ((error = OpSetFileSize64(lock, position, offset, newSize))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, DOSTRUE, 0);
#endif
}
#endif

/*------------------------------------------------------------------------*/
/* console */

/*------------------------------------------------------------------------*/
/* file/dir */

static void _Lock(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	char *bname = BADDR(pkt->dp_Arg2), *name;
	LONG access = pkt->dp_Arg3;
	struct ExtFileLock *newLock;
	LONG error;

	//Log(LD, "_Lock(%p, '%s', %s)\n", lock, name, access2name(access));

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	error = OpLockFile(lock, (UBYTE *)name, strlen(name), access, &newLock);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, error == 0 ? MKBADDR(newLock) : DOSFALSE, error);
}

static void _UnLock(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);

	//Log(LD, "_UnLock(%p)\n", lock);

	OpUnlockFile(lock);
	SetResult(pkt, DOSTRUE, 0);
}

static void _DupLock(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *newLock;
	LONG error;

	//Log(LD, "_DupLock(%p)\n", lock);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if ((error = OpCopyLock(lock, &newLock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, MKBADDR(newLock), 0l);
}

static void _SameLock(struct DosPacket *pkt)
{
	struct ExtFileLock *lock1 = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *lock2 = BADDR(pkt->dp_Arg2);

	//Log(LD, "_SameLock(%p, %p)\n", lock1, lock2);

	if (lock1 == lock2 || lock1->fl_GL == lock2->fl_GL)
		SetResult(pkt, DOSTRUE, LOCK_SAME);
	else
		SetResult(pkt, DOSFALSE, LOCK_DIFFERENT);
}

static void _ChangeMode(struct DosPacket *pkt)
{
	//LONG type = pkt->dp_Arg1;
	struct ExtFileLock *obj = BADDR(pkt->dp_Arg2);
	//LONG mode = pkt->dp_Arg3;
	LONG error;

	//Log(LD, "_ChangeMode(%ld, %p, %ld)\n", type, obj, mode);

	if ((error = TestLock(obj))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, DOSFALSE, ERROR_ACTION_NOT_KNOWN); //FIXME
}

static void _FileHandleFromLock(struct DosPacket *pkt)
{
	struct FileHandle *fh = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
	UBYTE *filename;
	LONG access;
	struct ExtFileLock *newLock;
	LONG error;

	//Log(LD, "_FileHandleFromLock(%p, %p)\n", fh, lock);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	filename = FilePart(lock->fl_GL->gl_Path);
	access = lock->fl_GL->gl_Access == ACCESS_READ ? MODE_OLDFILE : (lock->fl_GL->gl_Access == ACCESS_WRITE ? MODE_NEWFILE : MODE_READWRITE);

#ifdef __READ_ONLY__
	if (access != MODE_OLDFILE) {
		SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
		return;
	}
#endif
	if ((error = OpOpenFile(lock, filename, strlen(filename), access, &newLock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	fh->fh_Arg1 = MKBADDR(newLock);
	fh->fh_Port = DOSFALSE;
	SetResult(pkt, DOSTRUE, 0);
}

static void _ParentDir(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *newLock;
	LONG error;

	//Log(LD, "_ParentDir(%p)\n", lock);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if ((error = OpLockParent(lock, &newLock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, MKBADDR(newLock), 0l);
}            

static void _DeleteFile(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
#ifndef __READ_ONLY__
	char *bname = BADDR(pkt->dp_Arg2), *name;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_DeleteFile(%p, '%s')\n", lock, name);

	error = OpDeleteFile(lock, (UBYTE *)name, strlen(name));
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, DOSERROR(error), error);
#endif
}

static void _Rename(struct DosPacket *pkt)
{
	struct ExtFileLock *oldLock = BADDR(pkt->dp_Arg1);
	struct ExtFileLock *newLock = BADDR(pkt->dp_Arg3);
#ifndef __READ_ONLY__
	char *boldName = BADDR(pkt->dp_Arg2), *oldName;
	char *bnewName = BADDR(pkt->dp_Arg4), *newName;
#endif
	LONG error;

	if ((error = TestLock(oldLock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if ((error = TestLock(newLock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if (!(oldName = bstr2cstr(boldName))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	if (!(newName = bstr2cstr(bnewName))) {
		FreeVecPooled(global->g_Pool, oldName);
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_Rename(%p, '%s', %p '%s')\n", oldLock, oldName, newLock, newName);

	error = OpRenameFile(oldLock, (UBYTE *)oldName, strlen(oldName), newLock, (UBYTE *)newName, strlen(newName));
	FreeVecPooled(global->g_Pool, oldName);
	FreeVecPooled(global->g_Pool, newName);

	SetResult(pkt, DOSERROR(error), error);
#endif
}

static void _CreateDir(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
#ifndef __READ_ONLY__
	char *bname = BADDR(pkt->dp_Arg2), *name;
	struct ExtFileLock *newLock;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_CreateDir(%p, '%s')\n", lock, name);

	error = OpCreateDir(lock, (UBYTE *)name, strlen(name), &newLock);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, error == 0 ? MKBADDR(newLock) : DOSFALSE, error);
#endif
}

static void _SetProtection(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
#ifndef __READ_ONLY__
	char *bname = BADDR(pkt->dp_Arg3), *name;
	ULONG prot = (ULONG)pkt->dp_Arg4;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_SetProtection(%p, '%s', 0x%08lx)\n", lock, name, prot);

	error = OpSetProtect(lock, (UBYTE *)name, strlen(name), prot);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, DOSERROR(error), error);
#endif
}

static void _SetFileDate(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
#ifndef __READ_ONLY__
	char *bname = BADDR(pkt->dp_Arg3), *name;
	struct DateStamp *ds = (struct DateStamp *)pkt->dp_Arg4;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}					
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}

	/*{
		struct DateTime dt;
		char datestr[LEN_DATSTRING];

		dt.dat_Stamp = *ds;
		dt.dat_Format = FORMAT_DOS;
		dt.dat_Flags = 0;
		dt.dat_StrDay = NULL;
		dt.dat_StrDate = datestr;
		dt.dat_StrTime = NULL;
		DateToStr(&dt);

		Log(LD, "_SetFileDate(%p, '%s', %s)\n", lock, name, datestr);
	}*/

	error = OpSetDate(lock, (UBYTE *)name, strlen(name), ds);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, DOSERROR(error), error);
#endif
}

static void _SetComment(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
#ifndef __READ_ONLY__
	//char *bname = BADDR(pkt->dp_Arg3), *name;
	//char *bcomment = BADDR(pkt->dp_Arg4), *comment;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	/*if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	if (!(comment = bstr2cstr(bcomment))) {
		FreeVecPooled(global->g_Pool, name);
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_SetComment(%p, '%s', '%s')\n", lock, name, comment);

	if (strlen(comment) > AMIGA_COMMENT_MAX) {
		FreeVecPooled(global->g_Pool, name);
		FreeVecPooled(global->g_Pool, comment);
		SetResult(pkt, DOSFALSE, ERROR_COMMENT_TOO_BIG);
		return;
	}
	error = OpSetComment(lock, (UBYTE *)name, strlen(name), (UBYTE *)comment, strlen(comment));
	FreeVecPooled(global->g_Pool, name);
	FreeVecPooled(global->g_Pool, comment);

	SetResult(pkt, DOSERROR(error), error);*/

	SetResult(pkt, DOSFALSE, ERROR_ACTION_NOT_KNOWN);
#endif
}

static void _SetOwner(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg2);
#ifndef __READ_ONLY__
	//char *bname = BADDR(pkt->dp_Arg3), *name;
	//LONG info = pkt->dp_Arg4;
#endif
	LONG error;

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	/*if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_SetOwner(%p, '%s', %ld)\n", lock, name, info);

	error = OpSetOwner(lock, (UBYTE *)name, strlen(name), info);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, DOSERROR(error), error);*/

	SetResult(pkt, DOSFALSE, ERROR_ACTION_NOT_KNOWN);
#endif
}

static void _Examine(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct FileInfoBlock *fib = BADDR(pkt->dp_Arg2);
	LONG error;

	//Log(LD, "_Examine(%p, %p)\n", lock, fib);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	exfat_fib((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)lock->fl_GL->gl_EN, fib);
	SetResult(pkt, DOSTRUE, 0);
}
#ifdef __MORPHOS__
static void _Examine64(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct FileInfoBlock *fib = BADDR(pkt->dp_Arg2);
	LONG error;

	//Log(LD, "_Examine64(%p, %p)\n", lock, fib);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	exfat_fib64((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)lock->fl_GL->gl_EN, fib);
	SetResult(pkt, DOSTRUE, 0);
}
#endif

static void _ExNext(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct FileInfoBlock *fib = BADDR(pkt->dp_Arg2);
	LONG error;
	struct exFAT_Node *node = (struct exFAT_Node *)fib->fib_DiskKey;

	//Log(LD, "_ExNext(%p, %p)\n", lock, fib);
	//Log(LD, "node %p -> next %p\n", node, node->en_Next);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if (!node) {
		SetResult(pkt, DOSFALSE, ERROR_NO_MORE_ENTRIES);
		return;
	}
	exfat_fib((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, fib);
	fib->fib_DiskKey = (ULONG)node->en_Next;
	SetResult(pkt, DOSTRUE, 0);
}
#ifdef __MORPHOS__
static void _ExNext64(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct FileInfoBlock *fib = BADDR(pkt->dp_Arg2);
	LONG error;
	struct exFAT_Node *node = (struct exFAT_Node *)fib->fib_DiskKey;

	//Log(LD, "_ExNext64(%p, %p)\n", lock, fib);
	//Log(LD, "node %p -> next %p\n", node, node->en_Next);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if (!node) {
		SetResult(pkt, DOSFALSE, ERROR_NO_MORE_ENTRIES);
		return;
	}
	exfat_fib64((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, fib);
	fib->fib_DiskKey = (ULONG)node->en_Next;
	SetResult(pkt, DOSTRUE, 0);
}
#endif

static void _ExAll(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct ExAllData *buffer = (struct ExAllData *)pkt->dp_Arg2;
	LONG size = pkt->dp_Arg3;
	LONG type = pkt->dp_Arg4;
	struct ExAllControl *eac = (struct ExAllControl *)pkt->dp_Arg5;
	LONG error;

	//Log(LD, "_ExAll(%p, %p, %ld, %ld, %p)\n", lock, buffer, size, type, eac);

	if ((error = TestLock(lock))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	if ((error = OpExAll(lock, buffer, size, type, eac))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, DOSTRUE, 0l);
}
static void _ExAllEnd(struct DosPacket *pkt)
{
	//struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	//struct ExAllData *buffer	= (struct ExAllData *)pkt->dp_Arg2;
	//LONG size = pkt->dp_Arg3;
	//LONG type = pkt->dp_Arg4;
	struct ExAllControl *eac = (struct ExAllControl *)pkt->dp_Arg5;

	//Log(LD, "_ExAllEnd()\n");

	eac->eac_LastKey = 0ul;
	SetResult(pkt, DOSTRUE, 0);
}

static void _StartNotify(struct DosPacket *pkt)
{
	struct NotifyRequest *nr = (struct NotifyRequest *)pkt->dp_Arg1;
	LONG error;

	//Log(LD, "_StartNotify(%p) '%s'\n", nr, nr->nr_FullName);

	if ((error = OpAddNotify(nr))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, DOSTRUE, 0);
}

static void _EndNotify(struct DosPacket *pkt)
{
	struct NotifyRequest *nr = (struct NotifyRequest *)pkt->dp_Arg1;
	LONG error;

	//Log(LD, "_EndNotify(%p) '%s'\n", nr, nr->nr_FullName);

	if ((error = OpRemoveNotify(nr))) {
		SetResult(pkt, DOSFALSE, error);
		return;
	}
	SetResult(pkt, DOSTRUE, 0);
}

/*------------------------------------------------------------------------*/
/* volume */

static void _CurrentVolume(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);

	//Log(LD, "_CurrentVolume(%p)\n", lock);

	SetResult(pkt, (LONG)(lock ? lock->fl_FL.fl_Volume : (global->g_Volume ? MKBADDR(global->g_Volume->v_DosList) : (LONG)NULL)), 0l);
}

static void _Relabel(struct DosPacket *pkt)
{
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	char *bname = BADDR(pkt->dp_Arg1), *name;
	UBYTE *name8;
	LONG error;

	if (!global->g_Volume->v_DosList) {
		SetResult(pkt, DOSFALSE, global->g_DiskInserted ? ERROR_NOT_A_DOS_DISK : ERROR_NO_DISK);
		return;
	}

	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_Relabel('%s')\n", name);

	if (strlen(name) > EXFAT_ENAME_MAX) {
		FreeVecPooled(global->g_Pool, name);
		SetResult(pkt, DOSFALSE, ERROR_BAD_NUMBER);
	}

	if (!(name8 = AllocVecPooled(global->g_Pool, strlen(name) * 6 + 1))) {
		FreeVecPooled(global->g_Pool, name);
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	iso8859_to_utf8(name8, name, strlen(name));

	while (!AttemptLockDosList(LDF_VOLUMES | LDF_WRITE))
		ProcessPackets();

	error = (LONG)exfat_set_label(&global->g_Volume->v_exFAT, (const char *)name8);
		
	UnLockDosList(LDF_VOLUMES | LDF_WRITE);
	FreeVecPooled(global->g_Pool, name8);

	if (error) {
		FreeVecPooled(global->g_Pool, name);
		SetResult(pkt, DOSFALSE, -error);
		return;
	}

	{
		struct DosList *dol = global->g_Volume->v_DosList;
		struct VolumeInfo *vi = global->g_Volume->v_Info;
		struct VolumeMeta *vm = &global->g_Volume->v_Meta;

		cstr2bstr_buf((const char *)name, (char *)vm->vm_Label, AMIGA_LABEL_MAX-1);

		FreeVecPooled(vi->vi_Pool, BADDR(dol->dol_Name));
		if ((dol->dol_Name = MKBADDR(AllocVecPooled(vi->vi_Pool, vm->vm_Label[0] + 2)))) {
			CopyMem(vm->vm_Label, BADDR(dol->dol_Name), vm->vm_Label[0] + 1);
			*((char *)BADDR(dol->dol_Name) + vm->vm_Label[0] + 1) = '\0';
		} else {
			FreeVecPooled(global->g_Pool, name);
			SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
			return;
		}
	}

	SendEvent(IECLASS_DISKINSERTED);
	FreeVecPooled(global->g_Pool, name);
	SetResult(pkt, DOSTRUE, 0);
#endif
}

static void _DiskInfo(struct DosPacket *pkt)
{
	struct InfoData *id = BADDR(pkt->dp_Arg1);

	//Log(LD, "_DiskInfo(%p)\n", id);
//	char message[100];
//	sprintf(message,"_DiskInfo(%p) pkt(%p)\n", id, pkt);
//	Pop(LF, message);
	
	FillDiskInfo(id);
	SetResult(pkt, DOSTRUE, 0);
}

static void _Info(struct DosPacket *pkt)
{
	struct ExtFileLock *lock = BADDR(pkt->dp_Arg1);
	struct InfoData *id = BADDR(pkt->dp_Arg2);

	//Log(LD, "_Info(%p, %p)\n", lock, id);
//	char message[100];
//	sprintf(message,"_Info(%p, %p)\n", lock, id);
//	Pop(LF, message);

	if (lock && (global->g_Volume == NULL || lock->fl_FL.fl_Volume != MKBADDR(global->g_Volume->v_DosList))) {
		SetResult(pkt, DOSFALSE, ERROR_DEVICE_NOT_MOUNTED);
		return;
	}
	FillDiskInfo(id);
	SetResult(pkt, DOSTRUE, 0);
}

static void _Format(struct DosPacket *pkt)
{
#ifdef __READ_ONLY__
	SetResult(pkt, DOSFALSE, ERROR_WRITE_PROTECTED);
#else
	char *bname = BADDR(pkt->dp_Arg1), *name;
	ULONG dostype = (ULONG)pkt->dp_Arg2;
	LONG error;

	if (global->g_Volume) {
		SetResult(pkt, DOSFALSE, ERROR_OBJECT_IN_USE);
		return;
	}
	if (dostype != ID_EXFAT_DISK) {
		SetResult(pkt, DOSFALSE, ERROR_OBJECT_WRONG_TYPE);
		return;
	}

	if (!(name = bstr2cstr(bname))) {
		SetResult(pkt, DOSFALSE, ERROR_NO_FREE_STORE);
		return;
	}
	//Log(LD, "_Format('%s', 0x%08lx)\n", name, dostype);

	error = OpFormat((UBYTE *)name, dostype);
	FreeVecPooled(global->g_Pool, name);

	SetResult(pkt, DOSERROR(error), error);
#endif
}

static void _Serialize(struct DosPacket *pkt)
{
	//Log(LD, "_Serialize()\n");
	SetResult(pkt, DOSFALSE, ERROR_ACTION_NOT_KNOWN);
}

#ifdef __MORPHOS__
static void _GetFileSysAttr(struct DosPacket *pkt) /* dos/dosextens.h */
{
	LONG attr = pkt->dp_Arg1;
	APTR storage = (APTR)pkt->dp_Arg2;
	LONG storagesize = pkt->dp_Arg3;

	//Log(LD, "_GetFileSysAttr(%ld, %p, %ld)\n", attr, storage, storagesize);

	switch (attr) {
		case FQA_MaxFileNameLength: {
			if (storagesize < 4)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				*((LONG *)storage) = EXFAT_NAME_MAX; /* 256 chars */
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_MaxVolumeNameLength: {
			if (storagesize < 4)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				*((LONG *)storage) = EXFAT_ENAME_MAX; /* 15 chars */
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_MaxFileSize: {
			if (storagesize < 8)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				*((QUAD *)storage) = 0x0200000000000000; /* 128 PB */
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_IsCaseSensitive: {
			if (storagesize < 4)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				*((LONG *)storage) = FALSE;
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_DeviceType: {
			if (storagesize < 4)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				*((LONG *)storage) = DG_DIRECT_ACCESS;
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_NumBlocks: {
			if (storagesize < 8)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				if (global->g_Volume) {
					*((QUAD *)storage) = (QUAD)global->g_Volume->v_TotalSectors;
				} else {
					struct DosEnvec *de = BADDR(global->g_FSSM->fssm_Environ);

					*((QUAD *)storage) = (QUAD)GetNumBlocks(de);
				}
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		case FQA_NumBlocksUsed: {
			if (storagesize < 8)
				SetResult(pkt, DOSFALSE, ERROR_LINE_TOO_LONG);
			else {
				if (global->g_Volume) {
					*((QUAD *)storage) = (QUAD)(global->g_Volume->v_TotalSectors - global->g_Volume->v_FreeSectors);
				} else {
					struct DosEnvec *de = BADDR(global->g_FSSM->fssm_Environ);

					*((QUAD *)storage) = (QUAD)GetNumBlocks(de);
				}
				SetResult(pkt, DOSTRUE, 0);
			}
			break;
		}
		default:
			SetResult(pkt, DOSFALSE, ERROR_BAD_NUMBER);
	}
}
#endif

/*------------------------------------------------------------------------*/
/* obsolete */

static void _DiskChange(struct DosPacket **pkt)
{
	ULONG magic = (ULONG)(*pkt)->dp_Arg1;
	struct DosList *vol = (struct DosList *)(*pkt)->dp_Arg2;
	struct VolumeInfo *vol_info = BADDR(vol->dol_misc.dol_volume.dol_LockList);
	ULONG type = (*pkt)->dp_Arg3;

	//Log(LD, "_DiskChange()\n");

	if (magic == ID_EXFAT_DISK) {
		SetResult(*pkt, DOSTRUE, 0);

		if (AttemptLockDosList(LDF_VOLUMES|LDF_WRITE)) {
			if (type == ACTION_VOLUME_ADD) {
				AddDosEntry(vol);
				UnLockDosList(LDF_VOLUMES|LDF_WRITE);
				SendEvent(IECLASS_DISKINSERTED);
				//Log(LD, "DiskChange() Volume added successfuly\n");
			}
			else if (type == ACTION_VOLUME_REMOVE) {
				RemDosEntry(vol);
				DeletePool(vol_info->vi_Pool);
				UnLockDosList(LDF_VOLUMES|LDF_WRITE);
				SendEvent(IECLASS_DISKREMOVED);
				//Log(LD, "DiskChange() Volume removed successfuly.\n");
			}
			FreeDosObject(DOS_STDPKT, *pkt);
			*pkt = NULL;
			//Log(LD, "DiskChange() Packet destroyed\n");
		} else {
			//Log(LD, "DiskChange() DosList is locked\n");
			Delay(5);
			PutMsg(global->g_OurPort, (*pkt)->dp_Link);
			*pkt = NULL;
			//Log(LD, "DiskChange() Message moved to the end of the queue\n");
		}
	} else
		SetResult(*pkt, DOSFALSE, ERROR_OBJECT_WRONG_TYPE);
}

/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/
/*------------------------------------------------------------------------*/

static struct DosPacket *GetPacket(struct MsgPort *port)
{
	struct Message *msg;

	if ((msg = GetMsg(port)))
		return ((struct DosPacket *)((struct Node *)msg)->ln_Name);
	else
		return NULL;
}

void ReplyPacket(struct DosPacket *pkt)
{
	struct MsgPort *rp = pkt->dp_Port;

	pkt->dp_Port = global->g_OurPort;
	PutMsg(rp, pkt->dp_Link);
}

void ProcessPackets(void)
{
	struct DosPacket *pkt;

	while ((pkt = GetPacket(global->g_OurPort))) {
		switch (pkt->dp_Type) {
			/* internal */
			//case ACTION_NIL: /* 0 */
			//case ACTION_STARTUP: /* 0 */
			//case ACTION_TIMER: /* 30 */
			//case ACTION_READ_RETURN: /* 1001 */
			//case ACTION_WRITE_RETURN: /* 1002 */
#ifdef __MORPHOS__
			case ACTION_GET_DISK_FSSM: _GetDiskFSSM(pkt); break; /* 4201 */
			case ACTION_FREE_DISK_FSSM: _FreeDiskFSSM(pkt); break; /* 4202 */
#endif
			/* handler */
			case ACTION_DIE: _Die(pkt); break; /* 5 */
			case ACTION_MORE_CACHE: _AddBuffers(pkt); break; /* 18 */
			case ACTION_FLUSH: _Flush(pkt); break; /* 27 */
			case ACTION_INHIBIT: _Inhibit(pkt); break; /* 31 */
			//case ACTION_WRITE_PROTECT: /* 1023 */
			case ACTION_IS_FILESYSTEM: _IsFileSystem(pkt); break; /* 1027 */
			//case ACTION_GET_PREFS_TEMPLATE: /* 26500 */
			//case ACTION_GET_CURRENT_PREFS: /* 26501 */
			//case ACTION_SET_PREFS: /* 26502 */

			/* basic */
			case ACTION_WRITE: _Write(pkt); break; /* 'w' */
			case ACTION_READ: _Read(pkt); break; /* 'r' */
			case ACTION_SEEK: _Seek(pkt); break; /* 1008 */
#ifdef __MORPHOS__
			case ACTION_SEEK64: _Seek64(pkt); break;  /* 26400 */
#endif
			case ACTION_FINDUPDATE: /* 1004 */
			case ACTION_FINDINPUT: /* 1005 */
			case ACTION_FINDOUTPUT: _Open(pkt); break; /* 1006 */
			case ACTION_END: _Close(pkt); break; /* 1007 */
			case ACTION_SET_FILE_SIZE: _SetFileSize(pkt); break; /* 1022 */
#ifdef __MORPHOS__
			case ACTION_SET_FILE_SIZE64: _SetFileSize64(pkt); break; /* 26401 */
#endif
			//case ACTION_LOCK_RECORD: /* 2008 */
			//case ACTION_LOCK_RECORD64: /* 26402 */
			//case ACTION_FREE_RECORD: /* 2009 */
			//case ACTION_FREE_RECORD64: /* 26403 */

			/* file dir */
			case ACTION_LOCATE_OBJECT: _Lock(pkt); break; /* 8 */
			case ACTION_FREE_LOCK: _UnLock(pkt); break; /* 15 */
			case ACTION_DELETE_OBJECT: _DeleteFile(pkt); break; /* 16 */
			case ACTION_RENAME_OBJECT: _Rename(pkt); break; /* 17 */
			case ACTION_COPY_DIR: /* 19 */
			case ACTION_COPY_DIR_FH: _DupLock(pkt); break; /* 1030 */
			case ACTION_CREATE_DIR: _CreateDir(pkt); break; /* 22 */
			case ACTION_PARENT: /* 29 */
			case ACTION_PARENT_FH: _ParentDir(pkt); break; /* 1031 */
			case ACTION_SET_PROTECT: _SetProtection(pkt); break; /* 21 */
			case ACTION_SET_COMMENT: _SetComment(pkt); break; /* 28 */
			case ACTION_SET_DATE: _SetFileDate(pkt); break; /* 34 */
			case ACTION_SET_OWNER: _SetOwner(pkt); break; /* 1036 */
			case ACTION_SAME_LOCK: _SameLock(pkt); break; /* 40 */
			//case ACTION_MAKE_LINK:         /* 1021 */
			//case ACTION_READ_LINK:         /* 1024 */
			//case ACTION_NEW_READ_LINK:     /* 26406 */
			case ACTION_FH_FROM_LOCK: _FileHandleFromLock(pkt); break; /* 1026 */
			case ACTION_CHANGE_MODE: _ChangeMode(pkt); break; /* 1028 */
			case ACTION_EXAMINE_OBJECT: /* 23 */
			case ACTION_EXAMINE_FH: _Examine(pkt); break; /* 1034 */
#ifdef __MORPHOS__
			case ACTION_EXAMINE_OBJECT64: /* 26408 */
			case ACTION_EXAMINE_FH64: _Examine64(pkt); break; /* 26410 */
#endif
			case ACTION_EXAMINE_NEXT: _ExNext(pkt); break; /* 24 */
#ifdef __MORPHOS__
			case ACTION_EXAMINE_NEXT64: _ExNext64(pkt); break;  /* 26409 */
#endif
			case ACTION_EXAMINE_ALL: _ExAll(pkt); break; /* 1033 */
			case ACTION_EXAMINE_ALL_END: _ExAllEnd(pkt); break;  /* 1035 */
			case ACTION_ADD_NOTIFY: _StartNotify(pkt); break; /* 4097 */
			case ACTION_REMOVE_NOTIFY: _EndNotify(pkt); break; /* 4098 */

			/* volume */
			case ACTION_CURRENT_VOLUME: _CurrentVolume(pkt); break; /* 7 */
			case ACTION_RENAME_DISK: _Relabel(pkt); break; /* 9 */
			case ACTION_INFO: _Info(pkt); break; /* 26 */
			case ACTION_FORMAT: _Format(pkt); break; /* 1020 */
			case ACTION_SERIALIZE_DISK: _Serialize(pkt); break; /* 4200 */
#ifdef __MORPHOS__
			case ACTION_QUERY_ATTR: _GetFileSysAttr(pkt); break; /* 26407 */
#endif
			/* console */
			//case ACTION_WAIT_CHAR: /* 20 */
			case ACTION_DISK_INFO: _DiskInfo(pkt); break; /* 25, console */
			//case ACTION_SCREEN_MODE: /* 994 */
			//case ACTION_CHANGE_SIGNAL: /* 995 */

			/* obsolete */
			//case ACTION_GET_BLOCK: /* 2 */
			//case ACTION_SET_MAP: /* 4 */
			//case ACTION_EVENT: /* 6 */
			//case ACTION_DISK_TYPE: /* 32 */
			case ACTION_DISK_CHANGE: _DiskChange(&pkt); break; /* 33, obsolete */
			default: SetResult(pkt, DOSFALSE, ERROR_ACTION_NOT_KNOWN);
		}

		if (pkt) {
			//if (pkt->dp_Res1 == DOSFALSE && pkt->dp_Res2 == ERROR_ACTION_NOT_KNOWN)
				//Log(LD, "Unknown packet %ld\n", pkt->dp_Type);

			if (!global->g_Quit)
				ReplyPacket(pkt);
		}

		RestartTimer();
	}
}

