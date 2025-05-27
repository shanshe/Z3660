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

#include "include.h"

/*------------------------------------------------------------------------*/

LONG OpLockFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, LONG access, struct ExtFileLock **filelock)
{
	if (namelen)
		return LockFileByName(dirlock, name, namelen, access, filelock);
	else if (dirlock)
		return CopyLock(dirlock, filelock);
	else
		return LockRoot(access, filelock);
}

/*------------------------------------------------------------------------*/

void OpUnlockFile(struct ExtFileLock *lock)
{
	if (lock)
		FreeLock(lock);
}

/*------------------------------------------------------------------------*/

LONG OpCopyLock(struct ExtFileLock *lock, struct ExtFileLock **copy)
{
	if (lock)
		return CopyLock(lock, copy);
	else
		return LockRoot(SHARED_LOCK, copy);
}

/*------------------------------------------------------------------------*/

LONG OpLockParent(struct ExtFileLock *lock, struct ExtFileLock **parent)
{
	if (lock == NULL || lock->fl_GL->gl_IsRoot) {
		*parent = NULL;
		return 0;
	}
	if (lock->fl_GL->gl_EN->en_Parent == global->g_Volume->v_exFAT.ef_Root)
		return LockRoot(SHARED_LOCK, parent);

	{
		char *path;

		if ((path = ParentPath(lock->fl_Path))) {
			LONG error = LockFile(path, SHARED_LOCK, parent);

			FreeVecPooled(global->g_Pool, path);
			return error;
		} else
			return ERROR_OBJECT_NOT_FOUND;
	}
}

/*------------------------------------------------------------------------*/

LONG OpOpenFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, LONG action, struct ExtFileLock **filelock)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct ExtFileLock *lock;
	char *path;
	LONG err;

	if (namelen == 0) {
		if (action != ACTION_FINDINPUT)
			return ERROR_OBJECT_IN_USE;
		if (dirlock == NULL || (dirlock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_DIR))
			return ERROR_OBJECT_WRONG_TYPE;

		return CopyLock(dirlock, filelock);
	}
	err = LockFileByName(dirlock, name, namelen, action == ACTION_FINDINPUT ? SHARED_LOCK : EXCLUSIVE_LOCK, &lock);
	if (err == 0) {
#ifndef __MORPHOS__
		if (lock->fl_GL->gl_Is64Bit) {
			FreeLock(lock);
			return (action == ACTION_FINDINPUT ? ERROR_READ_PROTECTED : ERROR_WRITE_PROTECTED);
		}
#endif
		if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_DIR) {
			FreeLock(lock);
			return ERROR_OBJECT_WRONG_TYPE;
		}
		if (action != ACTION_FINDOUTPUT) {
			*filelock = lock;
			return 0;
		}
		if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO) {
			FreeLock(lock);
			return ERROR_WRITE_PROTECTED;
		}
		err = (LONG)exfat_truncate(ef, lock->fl_GL->gl_EN, 0ull);
		if (err != 0) {
			Log(LE, "error truncating file '%s' (%ld)\n", name, -err);
			return -err;
		}
		*filelock = lock;
		return 0;
	}
	else if (err != ERROR_OBJECT_NOT_FOUND)
		return err;

	if (action == ACTION_FINDINPUT)
		return ERROR_OBJECT_NOT_FOUND;

	if (!(path = MakePath(dirlock, (const char *)name, &err)))
		return err;

	//FIXME handle read-only of parent dir

	err = (LONG)exfat_mknod(ef, path);
	if (err == 0) {
		err = LockFile(path, EXCLUSIVE_LOCK, filelock);
		if (err == 0)
			(*filelock)->fl_DoNotify = TRUE;
	} else {
		Log(LE, "error creating file '%s' (%ld)\n", path, -err);
		err = -err;
	}

	FreeVecPooled(global->g_Pool, path);
	return err;
}

/*------------------------------------------------------------------------*/

LONG OpRead(struct ExtFileLock *lock, APTR buffer, LONG want, LONG *read)
{
	QUAD filesize = (QUAD)lock->fl_GL->gl_EN->en_Size;
	LONG bytes;

	*read = 0l;
	if (want == 0)
		return 0;

	if (lock->fl_Pos + (QUAD)want > filesize)
		want = (LONG)(filesize - lock->fl_Pos);
	if (want == 0)
		return 0;

	if ((bytes = (LONG)exfat_generic_pread(&global->g_Volume->v_exFAT, lock->fl_GL->gl_EN, buffer, (size_t)want, (UQUAD)lock->fl_Pos)) < 0)
		return ERROR_SEEK_ERROR;

	lock->fl_Pos += (QUAD)bytes;
	*read	= bytes;
	return 0;
}

/*------------------------------------------------------------------------*/

LONG OpWrite(struct ExtFileLock *lock, APTR *buffer, LONG want, LONG *written)
{
	LONG bytes;

	if (lock->fl_GL->gl_Access != EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;
	if (lock->fl_Volume->v_exFAT.ef_ReadOnly)
		return ERROR_DISK_WRITE_PROTECTED;
	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO)
		return ERROR_WRITE_PROTECTED;

	*written = 0;
	if (want == 0)
		return 0;

	if ((bytes = (LONG)exfat_generic_pwrite(&global->g_Volume->v_exFAT, lock->fl_GL->gl_EN, buffer, (size_t)want, (UQUAD)lock->fl_Pos)) < 0)
		return ERROR_SEEK_ERROR;

	lock->fl_DoNotify = TRUE;
	lock->fl_Pos += (QUAD)bytes;
	*written	= bytes;
	return 0;
}

/*------------------------------------------------------------------------*/

LONG OpAddNotify(struct NotifyRequest *nr)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct GlobalLock *gl = NULL;
	struct NotifyNode *nn;
	BOOL exists = FALSE;
	LONG err;

	if (nr->nr_FullName[strlen(nr->nr_FullName)-1] == ':')
		gl = &vi->vi_RootLock;
	else {
		char *ptr, *ptr8;

		if ((ptr = strchr((const char *)nr->nr_FullName, ':'))) {
			ptr++;
			if ((ptr8 = AllocVecPooled(global->g_Pool, strlen(ptr) * 6 + 1))) {
				struct exFAT *ef = &global->g_Volume->v_exFAT;
				struct exFAT_Node *node;

				iso8859_to_utf8(ptr8, (const char *)ptr, strlen(ptr));

				err = exfat_lookup(ef, &node, ptr8);
				if (err == 0) {
					exfat_put_node(ef, node);
					exists = TRUE;
					gl = FindGlobalLock(ptr8);
				}
				else if (err == -ERROR_OBJECT_NOT_FOUND) {
					FreeVecPooled(global->g_Pool, ptr8);
					return -err;
				} else
					exists = FALSE;

				FreeVecPooled(global->g_Pool, ptr8);
			} else
				return ERROR_NO_FREE_STORE;
		} else
			exists = FALSE;
	}

	if ((nn = AllocVecPooled(vi->vi_Pool, sizeof(struct NotifyNode))) == NULL)
		return ERROR_NO_FREE_STORE;

	nn->nn_GL = gl;
	nn->nn_NR = nr;

	AddTail((struct List *)&vi->vi_Notifies, (struct Node *)nn);

	if (exists && nr->nr_Flags & NRF_NOTIFY_INITIAL)
		SendNotify(nr);

	return 0;
}

LONG OpRemoveNotify(struct NotifyRequest *nr)
{
	struct Volume *v;
	struct NotifyNode *nn, *nn2;

	if (global->g_Volume) {
		ForeachNodeSafe(&global->g_Volume->v_Info->vi_Notifies, nn, nn2) {
			if (nn->nn_NR == nr) {
				Remove((struct Node *)nn);
				FreeVecPooled(global->g_Volume->v_Info->vi_Pool, nn);
				return 0;
			}
		}
	}
	ForeachNode(&global->g_BusyVolumes, v) {
		ForeachNodeSafe(&v->v_Info->vi_Notifies, nn, nn2) {
			if (nn->nn_NR == nr) {
				Remove((struct Node *)nn);
				FreeVecPooled(v->v_Info->vi_Pool, nn);
				AttemptDestroyVolume(v);
				return 0;
			}
		}
	}
	return 0;
}

/*------------------------------------------------------------------------*/

#define ReturnOverflow() \
	do { \
		if (last == curr) \
			error = ERROR_BUFFER_OVERFLOW; \
		\
		fib.fib_DiskKey = control->eac_LastKey; \
		goto end; \
	} while (0)

#define CopyStringSafe(_source) \
	do { \
		STRPTR source = _source; \
		\
		for (;;) { \
			if (next >= end) \
				ReturnOverflow(); \
			if (!(*next++ = *source++)) \
				break; \
		} \
	} while (0)

#define AROS_PTRALIGN 2 

#ifndef offsetof
#define offsetof(type, member)  ((size_t)(&((type *)0)->member))
#endif

LONG OpExAll(struct ExtFileLock *lock, struct ExAllData *buffer, LONG size, LONG data, struct ExAllControl *control)
{
	static const ULONG sizes[] = {
		0,
		offsetof(struct ExAllData,ed_Type),
		offsetof(struct ExAllData,ed_Size),
		offsetof(struct ExAllData,ed_Prot),
		offsetof(struct ExAllData,ed_Days),
		offsetof(struct ExAllData,ed_Comment),
		offsetof(struct ExAllData,ed_OwnerUID),
#ifdef __MORPHOS__
		offsetof(struct ExAllData,ed_Size64),
#endif
		sizeof(struct ExAllData)
	};
	struct exFAT_Node *node;
	STRPTR end  = (STRPTR)buffer + size;
	STRPTR next;
	struct ExAllData *last = buffer, *curr = buffer;
	struct FileInfoBlock fib;
	LONG error = 0l;

	control->eac_Entries = 0;

	if (control->eac_LastKey == 0) {
		node = lock->fl_GL->gl_EN;
#ifdef __MORPHOS__
		exfat_fib64((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, &fib);
#else
		exfat_fib((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, &fib);
#endif
		control->eac_LastKey = fib.fib_DiskKey;

		if (fib.fib_DirEntryType <= 0) {
			error = ERROR_OBJECT_WRONG_TYPE;
			goto end;
		}
	}

#ifdef __MORPHOS__
	if (data > ED_SIZE64)
#else
	if (data > ED_OWNER)
#endif
		error = ERROR_BAD_NUMBER;
	else {
		while (control->eac_LastKey) {
			node = (struct exFAT_Node *)control->eac_LastKey;
#ifdef __MORPHOS__
			exfat_fib64((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, &fib);
#else
			exfat_fib((const struct exFAT *)&global->g_Volume->v_exFAT, (const struct exFAT_Node *)node, &fib);
#endif
			fib.fib_DiskKey = (ULONG)node->en_Next;
			control->eac_LastKey = fib.fib_DiskKey;

			if (control->eac_MatchString && !MatchPatternNoCase(control->eac_MatchString, &fib.fib_FileName[1]))
				continue;

			next = (STRPTR)curr + sizes[data];
			if (next > end)
				ReturnOverflow();

			switch(data) {
#ifdef __MORPHOS__
				case ED_SIZE64:
					curr->ed_Size64 = fib.fib_Size64;
#endif
				case ED_OWNER:
					curr->ed_OwnerUID = fib.fib_OwnerUID;
					curr->ed_OwnerGID = fib.fib_OwnerGID;
				case ED_COMMENT:
					curr->ed_Comment = next;
					CopyStringSafe(&fib.fib_Comment[1]);
				case ED_DATE:
					curr->ed_Days  = fib.fib_Date.ds_Days;
					curr->ed_Mins  = fib.fib_Date.ds_Minute;
					curr->ed_Ticks = fib.fib_Date.ds_Tick;
				case ED_PROTECTION:
					curr->ed_Prot = fib.fib_Protection;
				case ED_SIZE:
					curr->ed_Size = fib.fib_Size;
				case ED_TYPE:
					curr->ed_Type = fib.fib_DirEntryType;
				case ED_NAME:
					curr->ed_Name = next;
					CopyStringSafe(&fib.fib_FileName[1]);
				case 0:
					curr->ed_Next = (struct ExAllData *)(((ULONG)next + AROS_PTRALIGN - 1) & ~(AROS_PTRALIGN - 1));
			}

			if (control->eac_MatchFunc && !CallHookPkt(control->eac_MatchFunc, curr, &data))
				continue;

			last = curr;
			curr = curr->ed_Next;
			control->eac_Entries++;
		}
		error = ERROR_NO_MORE_ENTRIES;
	}
	end:
	last->ed_Next = NULL;

	return error;
}

/*------------------------------------------------------------------------*/

LONG OpFormat(UBYTE *name, ULONG dostype)
{
	struct exFAT_Device *dev;
	int err;
	
	if ((dev = exfat_open(&err))) {
		UBYTE *name8;

		if ((name8 = AllocVecPooled(global->g_Pool, strlen((const char *)name) * 6 + 1))) {
			iso8859_to_utf8(name8, (const char *)name, strlen((const char *)name));

			err = exfat_format(dev, BLOCKSIZE, -1, (const char *)name8, (ULONG)0, dev->ed_First, dev->ed_Last);
			FreeVecPooled(global->g_Pool, name8);
		} else
			err = -ERROR_NO_FREE_STORE;

		exfat_close(dev);
	}
	if (err)
		return (LONG)-err;

	return 0;
}

/*------------------------------------------------------------------------*/

LONG OpDeleteFile(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen)
{
	struct ExtFileLock *lock;
	LONG err;

	if ((err = LockFileByName(dirlock, name, namelen, EXCLUSIVE_LOCK, &lock)) != 0)
		return err;
#ifndef __MORPHOS__
	if (lock->fl_GL->gl_Is64Bit) {
		FreeLock(lock);
		return ERROR_DELETE_PROTECTED;
	}
#endif
	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO) {
		FreeLock(lock);
		return ERROR_DELETE_PROTECTED;
	}

	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_DIR) {
		//FIXME check whether dir is empty
		//FreeLock(lock);
		//return ERROR_DIRECTORY_NOT_EMPTY;
	}
	//FIXME containing directory is write protected?
	//FreeLock(lock);
	//return ERROR_WRITE_PROTECTED;

	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_DIR)
		err = (LONG)exfat_rmdir(&global->g_Volume->v_exFAT, lock->fl_GL->gl_EN);
	else
		err = (LONG)exfat_unlink(&global->g_Volume->v_exFAT, lock->fl_GL->gl_EN);

	if (err) {
		FreeLock(lock);
		return -err;
	}

	SendNotifyByLock(lock->fl_Volume, lock->fl_GL);
	FreeLock(lock);
	return 0;
}

/*------------------------------------------------------------------------*/

LONG OpCreateDir(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, struct ExtFileLock **newdirlock)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct exFAT_Node *node;
	char *path;
	LONG err;

	if (dirlock && !dirlock->fl_GL->gl_IsRoot && (dirlock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO))
		return ERROR_WRITE_PROTECTED;

	if (!(path = MakePath(dirlock, (const char *)name, &err)))
		return err;

	err = (LONG)exfat_lookup(ef, &node, path);
	if (err == 0) {
		exfat_put_node(ef, node);
		FreeVecPooled(global->g_Pool, path);
		return ERROR_OBJECT_EXISTS;
	}

	err = (LONG)exfat_mkdir(ef, (const char *)path);
	if (err == 0) {
		err = LockFile(path, SHARED_LOCK, newdirlock);
		if (err == 0)
			SendNotifyByLock((*newdirlock)->fl_Volume, (*newdirlock)->fl_GL);
	} else {
		Log(LE, "error creating dir '%s' (%ld)\n", path, -err);
		err = -err;
	}

	FreeVecPooled(global->g_Pool, path);
	return err;
}

/*------------------------------------------------------------------------*/

LONG OpRenameFile(struct ExtFileLock *sdirlock, UBYTE *sname, ULONG snamelen, struct ExtFileLock *ddirlock, UBYTE *dname, ULONG dnamelen)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct exFAT_Node *node;
	char *spath;
	char *dpath;
	struct GlobalLock *gl;
	LONG err;

	if (sdirlock && (sdirlock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO))
		return ERROR_WRITE_PROTECTED;
	if (ddirlock && (ddirlock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO))
		return ERROR_WRITE_PROTECTED;

	if (!(spath = MakePath(sdirlock, (const char *)sname, &err)))
		return err;
	if (!(dpath = MakePath(ddirlock, (const char *)dname, &err))) {
		FreeVecPooled(global->g_Pool, spath);
		return err;
	}

	err = (LONG)exfat_lookup(ef, &node, spath);
	if (err == 0) {
#ifndef __MORPHOS__
		UQUAD size = node->en_Size;
#endif
		exfat_put_node(ef, node);

#ifndef __MORPHOS__
		if (size > FILESIZE_LIMIT_OS3 ) {
			FreeVecPooled(global->g_Pool, spath);
			FreeVecPooled(global->g_Pool, dpath);
			return ERROR_WRITE_PROTECTED;
		}
#endif
	} else  {
		FreeVecPooled(global->g_Pool, spath);
		FreeVecPooled(global->g_Pool, dpath);
		return -err;
	}
	err = (LONG)exfat_lookup(ef, &node, dpath);
	if (err == 0) {
		exfat_put_node(ef, node);
		FreeVecPooled(global->g_Pool, spath);
		FreeVecPooled(global->g_Pool, dpath);
		return ERROR_OBJECT_EXISTS;
	}
	else if (err != -ERROR_OBJECT_NOT_FOUND) {
		FreeVecPooled(global->g_Pool, spath);
		FreeVecPooled(global->g_Pool, dpath);
		return -err;
	}

	err = (LONG)exfat_rename(ef, (const char *)spath, (const char *)dpath);
	if (err == 0) {
		/* update path of global-lock */
		if ((gl = FindGlobalLock(spath))) {
			struct VolumeInfo *vi = global->g_Volume->v_Info;

			FreeVecPooled(vi->vi_Pool, gl->gl_Path);
			if (!(gl->gl_Path = AllocVecPooled(vi->vi_Pool, strlen(dpath) + 1)))
				return ERROR_NO_FREE_STORE;

			strcpy(gl->gl_Path, dpath);
		}

		err = (LONG)exfat_lookup(ef, &node, dpath);
		if (err == 0) {
			SendNotifyByNode(global->g_Volume, node);
			exfat_put_node(ef, node);
		} else
			err = -err;
	} else {
		Log(LE, "error renaming '%s' to '%s' (%ld)\n", spath, dpath, -err);
		err = -err;
	}

	FreeVecPooled(global->g_Pool, spath);
	FreeVecPooled(global->g_Pool, dpath);
	return err;
}

/*------------------------------------------------------------------------*/

LONG OpSetDate(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, struct DateStamp *ds)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct exFAT_Node *node;
	char *path;
	LONG err;

	if (dirlock && dirlock->fl_GL->gl_IsRoot && namelen == 0)
		return ERROR_INVALID_LOCK;

	if (!(path = MakePath(dirlock, (const char *)name, &err)))
		return err;

	err = (LONG)exfat_lookup(ef, &node, path);
	if (err == 0) {
#ifndef __MORPHOS__
		if (node->en_Size > FILESIZE_LIMIT_OS3 ) {
			exfat_put_node(ef, node);
			FreeVecPooled(global->g_Pool, path);
			return ERROR_WRITE_PROTECTED;
		}
#endif
		exfat_set_mtime(node, ds);
		SendNotifyByNode(global->g_Volume, node);
		exfat_put_node(ef, node);
	}

	FreeVecPooled(global->g_Pool, path);
	return -err;
}

/*------------------------------------------------------------------------*/

LONG OpSetProtect(struct ExtFileLock *dirlock, UBYTE *name, ULONG namelen, ULONG prot)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct exFAT_Node *node;
	char *path;
	LONG err;

	if (dirlock && dirlock->fl_GL->gl_IsRoot && namelen == 0)
		return ERROR_INVALID_LOCK;

	if (!(path = MakePath(dirlock, (const char *)name, &err)))
		return err;

	err = (LONG)exfat_lookup(ef, &node, path);
	if (err == 0) {
		int flags = node->en_Flags;

#ifndef __MORPHOS__
		if (node->en_Size > FILESIZE_LIMIT_OS3 ) {
			exfat_put_node(ef, node);
			FreeVecPooled(global->g_Pool, path);
			return ERROR_WRITE_PROTECTED;
		}
#endif
		//Log(LD, "old flags %08lx\n", (ULONG)flags);

		flags &= ~EXFAT_ATTRIB_RO;
		/*flags &= ~(EXFAT_ATTRIB_ARCH | EXFAT_ATTRIB_RO);
		if (prot & FIBF_ARCHIVE)
			flags |= EXFAT_ATTRIB_ARCH;*/
		if ((prot & (FIBF_WRITE | FIBF_DELETE)) == (FIBF_WRITE | FIBF_DELETE))
			flags |= EXFAT_ATTRIB_RO;

		//Log(LD, "new flags %08lx\n", (ULONG)flags);

		exfat_set_flags(node, flags);
		SendNotifyByNode(global->g_Volume, node);
		exfat_put_node(ef, node);
	}

	FreeVecPooled(global->g_Pool, path);
	return -err;
}

/*------------------------------------------------------------------------*/

LONG OpSetFileSize(struct ExtFileLock *lock, LONG offset, LONG whence, LONG *newsize)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	LONG err;
	LONG size;

#ifndef __MORPHOS__
	if (lock->fl_GL->gl_Is64Bit)
		return ERROR_WRITE_PROTECTED;
#endif
	if (lock->fl_GL->gl_Access != EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;
	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO)
		return ERROR_WRITE_PROTECTED;

	if (whence == OFFSET_BEGINNING && offset >= 0)
		size = offset;
	else if (whence == OFFSET_CURRENT && (LONG)lock->fl_Pos + offset >= 0)
		size = (LONG)lock->fl_Pos + offset;
	else if (whence == OFFSET_END && offset <= 0 && (LONG)lock->fl_GL->gl_EN->en_Size + offset >= 0)
		size = (LONG)lock->fl_GL->gl_EN->en_Size + offset;
	else
		return ERROR_SEEK_ERROR;

	if (size == (LONG)(lock->fl_GL->gl_EN->en_Size & 0x7fffffffull))
		return 0;

	err = (LONG)exfat_truncate(ef, lock->fl_GL->gl_EN, (UQUAD)size);
	if (err != 0) {
		Log(LE, "error truncating file (%ld)\n", -err);
		return -err;
	}
	*newsize = size;
	return 0;
}

LONG OpSetFileSize64(struct ExtFileLock *lock, QUAD offset, LONG whence, QUAD *newsize)
{
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	LONG err;
	QUAD size;

	if (lock->fl_GL->gl_Access != EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;
	if (lock->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_RO)
		return ERROR_WRITE_PROTECTED;
	
	if (whence == OFFSET_BEGINNING && offset >= 0)
		size = offset;
	else if (whence == OFFSET_CURRENT && lock->fl_Pos + offset >= 0)
		size = lock->fl_Pos + offset;
	else if (whence == OFFSET_END && offset <= 0 && (QUAD)lock->fl_GL->gl_EN->en_Size + offset >= 0)
		size = (QUAD)lock->fl_GL->gl_EN->en_Size + offset;
	else
		return ERROR_SEEK_ERROR;

	if (size == (QUAD)lock->fl_GL->gl_EN->en_Size)
		return 0;

	err = (LONG)exfat_truncate(ef, lock->fl_GL->gl_EN, (UQUAD)size);
	if (err != 0) {
		Log(LE, "error truncating file (%ld)\n", -err);
		return -err;
	}
	*newsize = size;
	return 0;
}

