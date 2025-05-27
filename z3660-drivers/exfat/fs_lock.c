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

static void SetFileLock(struct FileLock *fl, LONG access)
{
	fl->fl_Link = (BPTR)NULL;
	fl->fl_Key = 0;
	fl->fl_Access = access;
	fl->fl_Task = global->g_OurPort;
	fl->fl_Volume = MKBADDR(global->g_Volume->v_DosList);
}

/*------------------------------------------------------------------------*/

LONG TestLock(struct ExtFileLock *fl)
{
	if (!fl && !global->g_Volume) {
		if (global->g_DiskInserted)
			return ERROR_NOT_A_DOS_DISK;
		else
			return ERROR_NO_DISK;
	}
	if (!global->g_Volume || global->g_DiskInhibited || (fl && fl->fl_FL.fl_Volume != MKBADDR(global->g_Volume->v_DosList)))
		return ERROR_DEVICE_NOT_MOUNTED;

	if (fl && fl->fl_Magic != ID_EXFAT_DISK)
		return ERROR_OBJECT_WRONG_TYPE;

	return 0;
}

/*------------------------------------------------------------------------*/

LONG LockFileByName(struct ExtFileLock *fl, UBYTE *name, LONG namelen, LONG access, struct ExtFileLock **lock)
{
	if (namelen == 0)
		return CopyLock(fl, lock);

	if (fl && !(fl->fl_GL->gl_EN->en_Flags & EXFAT_ATTRIB_DIR)) {
		if (name[0] == '/') {
			if (namelen == 1)
				return OpLockParent(fl, lock);
			else {
				name++;
				namelen--;
			}
		} else
			return ERROR_OBJECT_WRONG_TYPE;
	}

	{
		struct exFAT *ef = &global->g_Volume->v_exFAT;
		struct exFAT_Node *node;
		char *path;
		LONG error;

		if (!(path = MakePath(fl, (const char *)name, &error)))
			return error;

		if (*path) {
			error = (LONG)exfat_lookup(ef, &node, path);
			if (error == 0) {
				if ((node->en_Flags & EXFAT_ATTRIB_DIR) && node->en_StartCluster <= le32_to_cpu(global->g_Volume->v_exFAT.ef_SB->es_RootdirCluster)) { //FIXME ?
					exfat_put_node(ef, node);
					FreeVecPooled(global->g_Pool, path);
					return LockRoot(access, lock);
				} else {
					exfat_put_node(ef, node);
					error = LockFile(path, access, lock);
					FreeVecPooled(global->g_Pool, path);
					return error;
				}
			} else {
				FreeVecPooled(global->g_Pool, path);
				return -error;
			}
		} else {
			FreeVecPooled(global->g_Pool, path);
			return LockRoot(access, lock);
		}
	}
}

/*------------------------------------------------------------------------*/

static void SetNotify(struct VolumeInfo *vi, struct GlobalLock *gl)
{
	struct NotifyNode *nn;

	ForeachNode(&vi->vi_Notifies, nn) {
		if (nn->nn_GL == NULL) {
			char *ptr;

			if ((ptr = strchr((const char *)nn->nn_NR->nr_FullName, ':'))) {
				if (*++ptr == '\0')
					continue;
			} else
				continue;
#define stricmp strcasecmp
			if (stricmp(gl->gl_Path, ptr) == 0)
				nn->nn_GL = gl;
		}
	}
}
static void ClrNotify(struct VolumeInfo *vi, struct GlobalLock *gl)
{
	struct NotifyNode *nn;

	ForeachNode(&vi->vi_Notifies, nn) {
		if (nn->nn_GL == gl)
			nn->nn_GL = NULL;
	}
}

static struct GlobalLock *MakeGlobalLock(struct exFAT_Node *node, LONG access, char *path)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct GlobalLock *gl;

	if ((gl = AllocVecPooled(vi->vi_Pool, sizeof(struct GlobalLock)))) {
		memset(gl, 0, sizeof(struct GlobalLock));

		gl->gl_EN = node;
		gl->gl_Access = access;
		gl->gl_IsRoot = FALSE;
		gl->gl_Is64Bit = node->en_Size > FILESIZE_LIMIT_OS3;
		if (!(gl->gl_Path = AllocVecPooled(vi->vi_Pool, strlen(path) + 1))) {
			FreeVecPooled(vi->vi_Pool, gl);
			return NULL;
		}
		strcpy(gl->gl_Path, path);
		NewList((struct List *)&gl->gl_Locks);

		AddTail((struct List *)&vi->vi_Locks, (struct Node *)gl);

		SetNotify(vi, gl);
	}
	return gl;
}

struct GlobalLock *FindGlobalLock(char *path)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct GlobalLock *ptr;

	ForeachNode(&vi->vi_Locks, ptr) {
		if (stricmp(ptr->gl_Path, path) == 0)
			return ptr;
	}
	return NULL;
}

LONG LockFile(char *path, LONG access, struct ExtFileLock **lock)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct exFAT *ef = &global->g_Volume->v_exFAT;
	struct exFAT_Node *node;
	struct GlobalLock *gl;
	struct ExtFileLock *fl;
	int rc;

	gl = FindGlobalLock(path);
	if (gl && access == EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;

	if (gl) {
		node = gl->gl_EN;
		node = exfat_get_node(node);
	} else {
		if ((rc = exfat_lookup(ef, &node, path)) != 0)
			return -rc;

		if (!(gl = MakeGlobalLock(node, access, path))) {
			exfat_put_node(ef, node);
			return ERROR_NO_FREE_STORE;
		}
	}

	if ((fl = AllocVecPooled(vi->vi_Pool, sizeof(struct ExtFileLock)))) {
		memset(fl, 0, sizeof(struct ExtFileLock));
		SetFileLock(&fl->fl_FL, access);
		fl->fl_Magic = ID_EXFAT_DISK;
		if (!(fl->fl_Path = AllocVecPooled(vi->vi_Pool, strlen(path) + 1))) {
			FreeVecPooled(vi->vi_Pool, fl);
			return ERROR_NO_FREE_STORE;
		}
		strcpy(fl->fl_Path, path);

		if (node->en_Flags & EXFAT_ATTRIB_DIR) {
			if ((rc = exfat_opendir(ef, node, &fl->fl_EI)) != 0)
				return -rc;
		}

		fl->fl_Volume = global->g_Volume;
		fl->fl_GL = gl;
		AddTail((struct List *)&gl->gl_Locks, (struct Node *)&fl->fl_Node);

		*lock = fl;
		return 0;
	}
	return ERROR_NO_FREE_STORE;
}

/*------------------------------------------------------------------------*/

LONG LockRoot(LONG access, struct ExtFileLock **lock)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct GlobalLock *gl = &vi->vi_RootLock;
	struct ExtFileLock *fl;

	if (access == EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;

	if ((fl = AllocVecPooled(vi->vi_Pool, sizeof(struct ExtFileLock)))) {
		memset(fl, 0, sizeof(struct ExtFileLock));
		SetFileLock(&fl->fl_FL, SHARED_LOCK);
		fl->fl_Magic = ID_EXFAT_DISK;
		if (!(fl->fl_Path = AllocVecPooled(vi->vi_Pool, 1))) {
			FreeVecPooled(vi->vi_Pool, fl);
			return ERROR_NO_FREE_STORE;
		}
		fl->fl_Path[0] = '\0';

		if (IsListEmpty((struct List *)&gl->gl_Locks))
			AddTail((struct List *)&vi->vi_Locks, (struct Node *)&gl->gl_Node);

		fl->fl_Volume = global->g_Volume;
		fl->fl_GL = gl;
		AddTail((struct List *)&gl->gl_Locks, (struct Node *)&fl->fl_Node);

		*lock = fl;
		return 0;
	}
	return ERROR_NO_FREE_STORE;
}

/*------------------------------------------------------------------------*/

void FreeLock(struct ExtFileLock *fl)
{
	struct VolumeInfo *vi = global->g_Volume->v_Info;
	struct GlobalLock *gl;

	if (fl == NULL)
		return;

	gl = fl->fl_GL;

	if (fl->fl_DoNotify)
		SendNotifyByLock(fl->fl_Volume, fl->fl_GL);

	if (!gl->gl_IsRoot) {
		struct exFAT *ef = &global->g_Volume->v_exFAT;

		if (gl->gl_EN->en_Flags & EXFAT_ATTRIB_DIR)
			exfat_closedir(ef, &fl->fl_EI);

		exfat_put_node(ef, gl->gl_EN);
	}

	Remove((struct Node *)&fl->fl_Node);
	if (IsListEmpty((struct List *)&gl->gl_Locks)) {
		Remove((struct Node *)gl);

		ClrNotify(fl->fl_Volume->v_Info, gl);

		if (!gl->gl_IsRoot) {
			FreeVecPooled(vi->vi_Pool, gl->gl_Path);
			FreeVecPooled(vi->vi_Pool, gl);
		}
	}

	if (fl->fl_Volume != global->g_Volume)
		AttemptDestroyVolume(fl->fl_Volume);

	FreeVecPooled(vi->vi_Pool, fl->fl_Path);
	FreeVecPooled(vi->vi_Pool, fl);
}

/*------------------------------------------------------------------------*/

LONG CopyLock(struct ExtFileLock *fl, struct ExtFileLock **lock)
{
	if (!fl || fl->fl_GL->gl_IsRoot)
		return LockRoot(SHARED_LOCK, lock);
	if (fl->fl_FL.fl_Access == EXCLUSIVE_LOCK)
		return ERROR_OBJECT_IN_USE;

	return LockFile(fl->fl_Path, SHARED_LOCK, lock);
}

