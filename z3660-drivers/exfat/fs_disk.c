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

static LONG MountVolume(struct Volume *v)
{
	LONG err;

	if ((err = (LONG)exfat_mount(&v->v_exFAT)) != 0) {
		Log(LE, "couldn't mount volume (%ld)\n", -err);
		char message[100];
		sprintf(message,"couldn't mount volume (%ld)\n", -err);
		Pop(LF, message);

		return -err;
	}

	{
		struct exFAT_Super *sb = v->v_exFAT.ef_SB;
		ULONG free_clusters = exfat_count_free_clusters(&v->v_exFAT);

		v->v_ID = le32_to_cpu(sb->es_VolumeSerial);
		v->v_SectorSize = SECTOR_SIZE(*sb);
		v->v_TotalSectors = le64_to_cpu(sb->es_SectorCount);
		v->v_FreeSectors = (UQUAD)free_clusters * CLUSTER_SIZE(*sb) / SECTOR_SIZE(*sb);

		/*
		exfat_print_info(sb, free_clusters);

		Log(LD, "VolumeID 0x%08lx\n", v->v_ID);
		Log(LD, "SectorSize %lu\n", v->v_SectorSize);
		Log(LD, "ClusterSize %lu\n", CLUSTER_SIZE(*sb));
		Log(LD, "TotalSectors %llu\n", v->v_TotalSectors);
		Log(LD, "FreeSectors %llu\n", v->v_FreeSectors);
		*/
//	char message[100];
//	sprintf(message,"VolumeID 0x%08lx SectorSize %lu ClusterSize %lu TotalSectors %llu FreeSectors %llu", v->v_ID, v->v_SectorSize, CLUSTER_SIZE(*sb), v->v_TotalSectors, v->v_FreeSectors);
//	Pop(LF, message);

		if ((v->v_TotalSectors << BLOCKSIZE_SHIFT) != v->v_exFAT.ef_Dev->ed_Size) {
			Log(LE, "The configured disksize does NOT match the size stored in the superblock.\n");
			char message[100];
			sprintf(message,"The configured disksize does NOT match the size stored in the superblock. %llu", v->v_exFAT.ef_Dev->ed_Size);
			Pop(LF, message);
			return -ERROR_BAD_NUMBER;
		}
	}

	{
		struct VolumeMeta *vm = &v->v_Meta;

		*vm->vm_Label = 0;

		if (*v->v_exFAT.ef_Label) {
			char label[AMIGA_LABEL_MAX + 1];

			if ((err = (LONG)utf8_to_iso8859(label, (const char *)v->v_exFAT.ef_Label, strlen(v->v_exFAT.ef_Label))) != 0)
				return -err;
			cstr2bstr_buf(label, (char *)vm->vm_Label, AMIGA_LABEL_MAX-1);
		}
		if (*vm->vm_Label == 0) {
			if (strlen(global->g_DiskVendor) >= 2 && strlen(global->g_DiskProduct) >= 2) {
				char label[AMIGA_LABEL_MAX];

				strcpy(label, global->g_DiskVendor);
				strcat(label, " ");
				strcat(label, global->g_DiskProduct);

				cstr2bstr_buf(label, (char *)vm->vm_Label, AMIGA_LABEL_MAX-1);
			} else {
				char buf[8 + 1];

				dec2hex(v->v_ID, buf);
				cstr2bstr_buf(buf, (char *)vm->vm_Label, AMIGA_LABEL_MAX-1);
			}
		}
		DateStamp(&vm->vm_Created);
	}

	return 0;
}

static void UnmountVolume(struct Volume *v)
{
	exfat_unmount(&v->v_exFAT);
}

/*------------------------------------------------------------------------*/

void FillDiskInfo(struct InfoData *id)
{
	struct DosEnvec *de = BADDR(global->g_FSSM->fssm_Environ);

	id->id_NumSoftErrors = 0;
	id->id_UnitNumber = (LONG)global->g_FSSM->fssm_Unit;
#ifdef __READ_ONLY__
	id->id_DiskState = ID_WRITE_PROTECTED;
#else
	id->id_DiskState = ID_VALIDATED;
#endif
//char message[100];
//sprintf(message,"global->g_Volume= %p", global->g_Volume);
//exfat_error(message);
//Pop(LF, message);

	if (global->g_Volume) {
		struct Volume *v = global->g_Volume;

		if (v->v_TotalSectors > DISKSIZE_LIMIT_OS3) {
			id->id_NumBlocks = DISKSIZE_LIMIT_OS3;
			id->id_NumBlocksUsed = DISKSIZE_LIMIT_OS3;
		} else {
			id->id_NumBlocks = (LONG)v->v_TotalSectors;
			id->id_NumBlocksUsed = (LONG)(v->v_TotalSectors - v->v_FreeSectors);
		}
		id->id_BytesPerBlock = (LONG)v->v_SectorSize;

		id->id_DiskType = ID_DOS_DISK;

		id->id_VolumeNode = MKBADDR(v->v_DosList);
		id->id_InUse = (IsListEmpty((struct List *)&v->v_Info->vi_Locks) && IsListEmpty((struct List *)&v->v_Info->vi_Notifies)) ? DOSFALSE : DOSTRUE;
	} else {
		id->id_NumBlocks = (LONG)GetNumBlocks(de);
		id->id_NumBlocksUsed = id->id_NumBlocks;
		id->id_BytesPerBlock = de->de_SizeBlock << 2;
#ifdef __READ_ONLY__
		id->id_DiskState = ID_WRITE_PROTECTED;
#else
		id->id_DiskState = ID_VALIDATED;
#endif
		if (global->g_DiskInhibited)
			id->id_DiskType = ID_BUSY_DISK;
		else if (global->g_DiskInserted)
			id->id_DiskType = 0 ? ID_NOT_REALLY_DOS : ID_UNREADABLE_DISK;
		else
			id->id_DiskType = ID_NO_DISK_PRESENT;

		id->id_VolumeNode = (BPTR)NULL;
		id->id_InUse = DOSFALSE;
	}
}

/*------------------------------------------------------------------------*/

static void SendVolumePacket(struct DosList *vol, LONG action)
{
	struct DosPacket *dp;

	dp = AllocDosObject(DOS_STDPKT, TAG_DONE);
	dp->dp_Type = ACTION_DISK_CHANGE;
	dp->dp_Arg1 = (LONG)ID_EXFAT_DISK;
	dp->dp_Arg2 = (LONG)vol;
	dp->dp_Arg3 = action;
	dp->dp_Port = NULL;

	PutMsg(global->g_OurPort, dp->dp_Link);
}

static struct VolumeInfo *CreateVolumeInfo(APTR pool, ULONG id, struct exFAT_Node *root)
{
	struct VolumeInfo *vi;

	if ((vi = AllocVecPooled(pool, sizeof(struct VolumeInfo)))) {
		memset(vi, 0, sizeof(struct VolumeInfo));
		vi->vi_Pool = pool;
		vi->vi_ID = id;
		NewList((struct List *)&vi->vi_Locks);
		NewList((struct List *)&vi->vi_Notifies);
		{
			struct GlobalLock *gl = &vi->vi_RootLock;

			gl->gl_EN = root;
			gl->gl_Access = SHARED_LOCK;
			gl->gl_IsRoot = TRUE;
			if (!(gl->gl_Path = AllocVecPooled(pool, 1))) {
				FreeVecPooled(pool, vi);
				return NULL;
			}
			gl->gl_Path[0] = '\0';
			NewList((struct List *)&gl->gl_Locks);
		}
	}
	return vi;
}

static struct DosList *CreateVolumeDosList(struct VolumeInfo *vi, struct VolumeMeta *vm)
{
	struct DosList *dol;

	if ((dol = AllocVecPooled(vi->vi_Pool, sizeof(struct DosList)))) {
		memset(dol, 0, sizeof(struct DosList));
		dol->dol_Next = (BPTR)NULL;
		dol->dol_Type = DLT_VOLUME;
		dol->dol_Task = global->g_OurPort;
		dol->dol_Lock = (BPTR)NULL;

		CopyMem(&vm->vm_Created, &dol->dol_misc.dol_volume.dol_VolumeDate, sizeof(struct DateStamp));
		dol->dol_misc.dol_volume.dol_LockList = MKBADDR(vi);
		dol->dol_misc.dol_volume.dol_DiskType = (LONG)ID_EXFAT_DISK;

		if ((dol->dol_Name = MKBADDR(AllocVecPooled(vi->vi_Pool, vm->vm_Label[0] + 2)))) {
			CopyMem(vm->vm_Label, BADDR(dol->dol_Name), vm->vm_Label[0] + 1);
			*((char *)BADDR(dol->dol_Name) + vm->vm_Label[0] + 1) = '\0';
			return dol;
		}
		FreeVecPooled(vi->vi_Pool, dol);
	}
	return NULL;
}

#define LOCKFROMNODE(A) ((struct ExtFileLock *)(((BYTE *)(A)) - (ULONG)&((struct ExtFileLock *)NULL)->fl_Node))

static void PatchVolume(struct Volume *v, struct VolumeInfo *vi)
{
	struct GlobalLock *gl;
	struct ExtFileLock *fl;
	struct MinNode *mn;
	struct NotifyNode *nn;

	ForeachNode(&vi->vi_Locks, gl) {
		ForeachNode(&gl->gl_Locks, mn) {
			fl = LOCKFROMNODE(mn);
			//Log(LD, "Patching adopted lock %p. old port = %p, new port = %p\n", fl, fl->fl_FL.fl_Task, global->g_OurPort);
			fl->fl_FL.fl_Task = global->g_OurPort;
			fl->fl_Volume = v;
		}
	}
	ForeachNode(&vi->vi_RootLock.gl_Locks, mn) {
		fl = LOCKFROMNODE(mn);
		//Log(LD, "Patching adopted ROOT lock %p. old port = %p, new port = %p\n", fl, fl->fl_FL.fl_Task, global->g_OurPort);
		fl->fl_FL.fl_Task = global->g_OurPort;
		fl->fl_Volume = v;
	}
	ForeachNode(&vi->vi_Notifies, nn)
		nn->nn_NR->nr_Handler = global->g_OurPort;
}

void DoDiskInsert(void)
{
	struct Volume *v;
	ULONG err;

	if (global->g_Volume)
		return;

	if ((v = AllocVecPooled(global->g_Pool, sizeof(struct Volume))))
	{
		memset(v, 0, sizeof(struct Volume));

		err = MountVolume(v);
		if (err == 0)
		{
			struct DosList *dl;
			struct VolumeInfo *vi = NULL;
			struct DosList *newvol = NULL;
			char label[AMIGA_LABEL_MAX];

			strncpy(label, &v->v_Meta.vm_Label[1], (int)v->v_Meta.vm_Label[0]);

			dl = LockDosList(LDF_VOLUMES | LDF_WRITE);
			dl = FindDosEntry(dl, label, LDF_VOLUMES | LDF_WRITE);
			UnLockDosList(LDF_VOLUMES | LDF_WRITE);
			if (dl) {
				dl->dol_Task = global->g_OurPort;
				v->v_DosList = dl;

				vi = BADDR(dl->dol_misc.dol_volume.dol_LockList);

				PatchVolume(v, vi);
			} else {
				APTR pool;

				if ((pool = CreatePool(MEMF_PUBLIC, 4096, 4096)))
				{
					vi = CreateVolumeInfo(pool, v->v_ID, v->v_exFAT.ef_Root);
					if (vi)
						newvol = CreateVolumeDosList(vi, &v->v_Meta);

					if (vi && newvol)
						v->v_DosList = newvol;
					else
						DeletePool(pool);
				}
			}

			v->v_Info = vi;
			global->g_Volume = v;

			if (dl)
				SendEvent(IECLASS_DISKINSERTED);
			else
				SendVolumePacket(newvol, ACTION_VOLUME_ADD);

			return;
		}
		FreeVecPooled(global->g_Pool, v);
	}
	SendEvent(IECLASS_DISKINSERTED);
}

BOOL AttemptDestroyVolume(struct Volume *v)
{
	BOOL destroyed = FALSE;

	if (IsListEmpty((struct List *)&v->v_Info->vi_Locks) && IsListEmpty((struct List *)&v->v_Info->vi_Notifies)) {
		if (v == global->g_Volume)
			global->g_Volume = NULL;
		else
			Remove((struct Node *)v);

		SendVolumePacket(v->v_DosList, ACTION_VOLUME_REMOVE);

		UnmountVolume(v);
		FreeVecPooled(global->g_Pool, v);
		destroyed = TRUE;
	}
	return destroyed;
}

void DoDiskRemove(void)
{
	if (global->g_Volume) {
		struct Volume *v = global->g_Volume;

		if (!AttemptDestroyVolume(v)) {
			if (v->v_DosList) v->v_DosList->dol_Task = NULL;
			global->g_Volume = NULL;
			AddTail((struct List *)&global->g_BusyVolumes, (struct Node *)v);
			SendEvent(IECLASS_DISKREMOVED);
		}
	}
}
 
void ProcessDiskChange(void)
{
	if (global->g_DiskInhibited)
		return;

	global->g_DiskIOReq->iotd_Req.io_Command = TD_CHANGESTATE;
	global->g_DiskIOReq->iotd_Req.io_Data = NULL;
	global->g_DiskIOReq->iotd_Req.io_Length = 0;
	global->g_DiskIOReq->iotd_Req.io_Flags = IOF_QUICK;
	DoIO((struct IORequest*) global->g_DiskIOReq);

	if (global->g_DiskIOReq->iotd_Req.io_Error == 0 && global->g_DiskIOReq->iotd_Req.io_Actual == 0) {
		global->g_DiskInserted = TRUE;
		DoDiskInsert();
	} else {
		global->g_DiskInserted = FALSE;
		DoDiskRemove();
	}
}

void UpdateDisk(void)
{
	global->g_DiskIOReq->iotd_Req.io_Command = CMD_UPDATE;
	DoIO((struct IORequest *)global->g_DiskIOReq);

	if (!global->g_TimerRestart) {
		global->g_DiskIOReq->iotd_Req.io_Command = TD_MOTOR;
		global->g_DiskIOReq->iotd_Req.io_Length = 0;
		DoIO((struct IORequest *)global->g_DiskIOReq);
	}
}

