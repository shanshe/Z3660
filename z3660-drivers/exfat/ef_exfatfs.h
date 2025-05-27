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

#ifndef _EXFATFS_H
#define _EXFATFS_H

#if 1
typedef UWORD le16_t;
typedef ULONG le32_t;
typedef UQUAD le64_t;
#else
typedef struct { UWORD _u16; } le16_t;
typedef struct { ULONG _u32; } le32_t;
typedef struct { UQUAD _u64; } le64_t;
#endif
 
typedef ULONG cluster_t; /* cluster number */

#define BLOCKSIZE       512
#define BLOCKSIZE_SHIFT 9

#define EXFAT_FIRST_DATA_CLUSTER 2
#define EXFAT_LAST_DATA_CLUSTER  0xfffffff6

#define EXFAT_CLUSTER_FREE         0 /* free cluster */
#define EXFAT_CLUSTER_BAD 0xfffffff7 /* cluster contains bad sector */
#define EXFAT_CLUSTER_END 0xffffffff /* final cluster of file or directory */

#define EXFAT_STATE_MOUNTED 2

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct exFAT_Super
{
	UBYTE		es_Jump[3];					/* 0x00 jmp and nop instructions */
	UBYTE		es_OemName[8];				/* 0x03 "EXFAT   " */
	UBYTE		___unused1[53];			/* 0x0B always 0 */
	le64_t	es_SectorStart;			/* 0x40 partition first sector */
	le64_t	es_SectorCount;			/* 0x48 partition sectors count */
	le32_t	es_FatSectorStart;		/* 0x50 FAT first sector */
	le32_t	es_FatSectorCount;		/* 0x54 FAT sectors count */
	le32_t	es_ClusterSectorStart;	/* 0x58 first cluster sector */
	le32_t	es_ClusterCount;			/* 0x5C total clusters count */
	le32_t	es_RootdirCluster;		/* 0x60 first cluster of the root dir */
	le32_t	es_VolumeSerial;			/* 0x64 volume serial number */
	struct {									/* 0x68 FS version */
		UBYTE es_Minor;
		UBYTE es_Major;
	}			es_Version;
	le16_t	es_VolumeState;			/* 0x6A volume state flags */
	UBYTE		es_SectorBits;				/* 0x6C sector size as (1 << n) */
	UBYTE		es_SpcBits;					/* 0x6D sectors per cluster as (1 << n) */
	UBYTE		es_FatCount;				/* 0x6E always 1 */
	UBYTE		es_DriveNo;					/* 0x6F always 0x80 */
	UBYTE		es_AllocatedPercent;		/* 0x70 percentage of allocated space */
	UBYTE		___unused2[397];			/* 0x71 always 0 */
	le16_t	es_BootSignature;			/* the value of 0xAA55 */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#define EXFAT_ENTRY_VALID     0x80
#define EXFAT_ENTRY_CONTINUED 0x40

#define EXFAT_ENTRY_BITMAP    (0x01 | EXFAT_ENTRY_VALID)
#define EXFAT_ENTRY_UPCASE    (0x02 | EXFAT_ENTRY_VALID)
#define EXFAT_ENTRY_LABEL     (0x03 | EXFAT_ENTRY_VALID)
#define EXFAT_ENTRY_FILE      (0x05 | EXFAT_ENTRY_VALID)
#define EXFAT_ENTRY_FILE_INFO (0x00 | EXFAT_ENTRY_VALID | EXFAT_ENTRY_CONTINUED)
#define EXFAT_ENTRY_FILE_NAME (0x01 | EXFAT_ENTRY_VALID | EXFAT_ENTRY_CONTINUED)

struct exFAT_Entry /* common container for all entries */
{
	UBYTE		ee_Type;							/* any of EXFAT_ENTRY_xxx */
	UBYTE		ee_Data[31];
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#define EXFAT_ENAME_MAX 15

struct exFAT_EntryBitmap /* allocated clusters bitmap */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_BITMAP */
	UBYTE		___unknown1[19];
	le32_t	ee_StartCluster;
	le64_t	ee_Size;							/* in bytes */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct exFAT_EntryUpperCase /* upper case translation table */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_UPCASE */
	UBYTE		___unknown1[3];
	le32_t	ee_Checksum;
	UBYTE		___unknown2[12];
	le32_t	ee_StartCluster;
	le64_t	ee_Size;							/* in bytes */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct exFAT_EntryLabel /* volume label */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_LABEL */
	UBYTE		ee_Length;						/* number of characters */
	le16_t	ee_Name[EXFAT_ENAME_MAX];	/* in UTF-16LE */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct exFAT_EntryName /* file or directory name */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_FILE_NAME */
	UBYTE		___unknown;
	le16_t	ee_Name[EXFAT_ENAME_MAX];	/* in UTF-16LE */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#define EXFAT_ATTRIB_RO     0x01
#define EXFAT_ATTRIB_HIDDEN 0x02
#define EXFAT_ATTRIB_SYSTEM 0x04
#define EXFAT_ATTRIB_VOLUME 0x08
#define EXFAT_ATTRIB_DIR    0x10
#define EXFAT_ATTRIB_ARCH   0x20

struct exFAT_EntryMeta1 /* file or directory info (part 1) */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_FILE */
	UBYTE		ee_Continuations;
	le16_t	ee_Checksum;
	le16_t	ee_Attrib;						/* combination of EXFAT_ATTRIB_xxx */
	le16_t	___unknown1;
	le16_t	ee_CrTime;						/* creation date and time */
	le16_t	ee_CrDate;						/* creation date and time */
	le16_t	ee_MTime;						/* latest modification date and time */
	le16_t	ee_MDate;						/* latest modification date and time */
	le16_t	ee_ATime;						/* latest access date and time */
	le16_t	ee_ADate;						/* latest access date and time */
	UBYTE		ee_CrTimeCS;					/* creation time in cs (centiseconds) */
	UBYTE		ee_MTimeCS;						/* latest modification time in cs */
	UBYTE		___unknown2[10];
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#define EXFAT_FLAG_ALWAYS1		(1u << 0)
#define EXFAT_FLAG_CONTIGUOUS	(1u << 1)

struct exFAT_EntryMeta2 /* file or directory info (part 2) */
{
	UBYTE		ee_Type;							/* EXFAT_ENTRY_FILE_INFO */
	UBYTE		ee_Flags;						/* combination of EXFAT_FLAG_xxx */
	UBYTE		___unknown1;
	UBYTE		ee_NameLength;
	le16_t	ee_NameHash;
	le16_t	___unknown2;
	le64_t	ee_RealSize;					/* in bytes, equals to size */
	UBYTE		___unknown3[4];
	le32_t	ee_StartCluster;
	le64_t	ee_Size;							/* in bytes, equals to real_size */
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

#endif /* _EXFATFS_H */

