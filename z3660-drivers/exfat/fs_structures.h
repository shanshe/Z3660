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

#ifndef _STRUCTURES_H
#define _STRUCTURES_H 1

/*-------------------------------------------------------------------------*/

#define ID_EXFAT_DISK 0x45584641ul /* EXFA */

#ifndef ID_BUSY_DISK
#define ID_BUSY_DISK 0x42555359 /* BUSY */
#endif

#define AMIGA_NAME_MAX			108
#define AMIGA_COMMENT_MAX		80
#define AMIGA_LABEL_MAX			32

#define FILESIZE_LIMIT_OS3 	0x7fffffff /* bytes */
#define DISKSIZE_LIMIT_OS3 	0x7fffffff /* blocks */

#define ACTION_VOLUME_ADD		16000
#define ACTION_VOLUME_REMOVE	16001

/*-------------------------------------------------------------------------*/

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

struct GlobalLock
{
	struct MinNode			 gl_Node;

	struct exFAT_Node		*gl_EN;

	LONG						 gl_Access;
	BOOL						 gl_IsRoot;
	BOOL						 gl_Is64Bit;
	char						*gl_Path;

	struct MinList			 gl_Locks;
};

struct ExtFileLock
{
	struct FileLock		 fl_FL;

	ULONG						 fl_Magic;
	struct MinNode	       fl_Node;

	struct Volume			*fl_Volume;
	struct GlobalLock	   *fl_GL;
	BOOL						 fl_DoNotify;

	char						*fl_Path;
	QUAD						 fl_Pos;
	struct exFAT_Iterator fl_EI;
};

struct NotifyNode {
	struct MinNode			 nn_Node;
	struct GlobalLock		*nn_GL;
	struct NotifyRequest	*nn_NR;
};

struct VolumeInfo
{
	APTR						 vi_Pool;
	ULONG						 vi_ID;
	struct MinList			 vi_Locks;
	struct MinList			 vi_Notifies;
	struct GlobalLock	    vi_RootLock;
};

struct VolumeMeta
{
	UBYTE						 vm_Label[AMIGA_LABEL_MAX];
	struct DateStamp		 vm_Created;
};

struct Volume
{
	struct Node				 v_Node;
	struct DosList			*v_DosList;
	struct VolumeInfo		*v_Info;
	struct VolumeMeta		 v_Meta;

	struct exFAT			 v_exFAT;
	ULONG						 v_ID;
	ULONG						 v_SectorSize;
	UQUAD						 v_TotalSectors;
	UQUAD						 v_FreeSectors;
};

struct IntData {
	struct Interrupt		 id_Interrupt;
	struct ExecBase		*id_SysBase;
	struct Task				*id_Task;
	ULONG						 id_Signal;
	ULONG						 id_Count;
};

struct Global
{
	/* mem/task */
	struct Task						*g_OurTask;
	struct MsgPort					*g_OurPort;
	APTR								 g_Pool;

	struct MsgPort					*g_NotifyPort;

	/* fs */
	struct DosList					*g_DevNode;
	struct FileSysStartupMsg	*g_FSSM;
	LONG								 g_Quit;
	struct DosPacket				*g_DeathPacket;

	/* timer */
	struct timerequest			*g_TimerReq;
	struct MsgPort					*g_TimerPort;
	BOOL								 g_TimerActive;
	BOOL								 g_TimerRestart;

	/* io */
	struct IOExtTD					*g_DiskIOReq;
	struct IOExtTD					*g_DiskChgReq;
	struct MsgPort					*g_DiskPort;
	LONG								 g_DiskChgSig;
	struct IntData					 g_DiskChangeIntData;

	/* scsi capacity */
	UQUAD							    g_DiskSize;
	/* scsi inquiry */
	BOOL 								 g_DiskRemovable;
	char 								 g_DiskVendor[8 + 1];
	char 								 g_DiskProduct[16 + 1];
	char 								 g_DiskRevision[4 + 1];
	char 								 g_pad;

	/* volumes */
	struct Volume					*g_Volume;
	struct MinList					 g_BusyVolumes;

	/* disk status */
	BOOL								 g_DiskInserted;
	WORD								 g_DiskInhibited;
};

#ifdef __MORPHOS__
	#pragma pack()
#endif

/*-------------------------------------------------------------------------*/

#endif

