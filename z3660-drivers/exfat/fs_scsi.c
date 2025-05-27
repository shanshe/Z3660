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

/* commands */
#define SCSI_TEST_UNIT_READY		0x00
#define SCSI_INQUIRY					0x12
#define SCSI_READ_CAPACITY_10		0x25
#define SCSI_SYNCHRONIZE_CACHE	0x35

#define SCSI_READ_6					0x08
#define SCSI_READ_10					0x28
#define SCSI_READ_12					0xa8
#define SCSI_READ_16					0x88

#define SCSI_WRITE_6					0x0a
#define SCSI_WRITE_10				0x2a
#define SCSI_WRITE_12				0xaa
#define SCSI_WRITE_16				0x8a

/* status codes */
#define SC_GOOD 								0x00
#define SC_CHECK_CONDITION 				0x02
#define SC_CONDITION_MET 					0x04
#define SC_BUSY 								0x08
#define SC_INTERMEDIATE 					0x10
#define SC_INTERMEDIATE_CONDITION_MET	0x14
#define SC_RESERVATION_CONFLICT 			0x18
#define SC_COMMAND_TERMINATED 			0x20
#define SC_QUEUE_FULL 						0x28

/* sense keys */
#define SK_NO_SENSE 				0x00
#define SK_RECOVERED_ERROR 	0x01
#define SK_NOT_READY 			0x02
#define SK_MEDIUM_ERROR 		0x03
#define SK_HARDWARE_ERROR 		0x04
#define SK_ILLEGAL_REQUEST		0x05
#define SK_UNIT_ATTENTION 		0x06
#define SK_DATA_PROTECT 		0x07
#define SK_BLANK_CHECK 			0x08
#define SK_COPY_ABORTED 		0x0a
#define SK_ABORTED_COMMAND 	0x0b
#define SK_EQUAL 					0x0c
#define SK_VOLUME_OVERFLOW 	0x0d
#define SK_MISCOMPARE 			0x0e

#define RES 0
#define OBS 0

/*------------------------------------------------------------------------*/

static BYTE DoSCSI(struct IOStdReq *io, APTR cdb, UWORD cdblen, APTR data, ULONG length)
{
	struct SCSICmd scsi;
	UBYTE sensedata[256];

	switch (*((UBYTE *)cdb)) {
		case SCSI_WRITE_6:
		case SCSI_WRITE_10:
		case SCSI_WRITE_12:
			//scsi.scsi_Flags = SCSIF_WRITE | SCSIF_NOSENSE;
			scsi.scsi_Flags = SCSIF_WRITE | SCSIF_AUTOSENSE;
			break;
		default:
			//scsi.scsi_Flags = SCSIF_READ | SCSIF_NOSENSE;
			scsi.scsi_Flags = SCSIF_READ | SCSIF_AUTOSENSE;
	}
	scsi.scsi_Data = (UWORD *)data;
	scsi.scsi_Length = length;
	scsi.scsi_Actual = 0ul;
	scsi.scsi_Command = cdb;
	scsi.scsi_CmdLength = cdblen;
	scsi.scsi_CmdActual = 0;
	scsi.scsi_Status = 0;
	scsi.scsi_SenseData = sensedata;
	scsi.scsi_SenseLength = sizeof(sensedata);
	scsi.scsi_SenseActual = 0;

	io->io_Command = HD_SCSICMD;
	io->io_Length = sizeof(struct SCSICmd);
	io->io_Data = &scsi;

	DoIO((struct IORequest *)io);
	/*if (io->io_Error || scsi.scsi_Status)
		Log(LD, "error %d, status 0x%02x", io->io_Error, scsi.scsi_Status);

	if (io->io_Error == HFERR_BadStatus) {
		//UBYTE status = ((scsi.scsi_Status & 0x3E) >> 1);
		//status &= 0xfe; //bit0 is listed as reserved in SCSI-2, but is significant in SCSI-3

		if (scsi.scsi_Status == SC_CHECK_CONDITION) {
			Log(LD, "SK %02lx, ASC[Q] %02lx %02lx, AS %u\n", sensedata[2] & 0x0f, sensedata[12], sensedata[13], scsi.scsi_SenseActual);
		}
	}*/
	return io->io_Error;
}

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct CDB_TestUnitReady
{
	UBYTE opcode;
	UBYTE _res1;
	UBYTE _res2;
	UBYTE _res3;
	UBYTE _res4;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

static inline void Set_TestUnitReady(struct CDB_TestUnitReady *cdb, UBYTE ctrl)
{
	cdb->opcode = SCSI_TEST_UNIT_READY;
	cdb->_res1 = RES;
	cdb->_res2 = RES;
	cdb->_res3 = RES;
	cdb->_res4 = RES;
	cdb->ctrl = ctrl;
}

static BYTE SCSI_TestUnitReady(struct IOStdReq *io)
{
	struct CDB_TestUnitReady cdb;

	Set_TestUnitReady(&cdb, 0);

	return (DoSCSI(io, &cdb, sizeof(cdb), NULL, 0ul));
}

/*----------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct CDB_ReadCapacity10
{
	UBYTE opcode;
	UBYTE _res1:6,longlba:1,reladdr:1;
	ULONG lba;
	UBYTE _res2;
	UBYTE _res3;
	UBYTE _res4:7,pmi:1;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct DAT_ReadCapacity10_short
{
	ULONG lba;
	ULONG blocksize;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct DAT_ReadCapacity10_long
{
	UQUAD lba;
	ULONG blocksize;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

static inline void Set_ReadCapacity10(struct CDB_ReadCapacity10 *cdb, BOOL longlba, BOOL reladdr, ULONG lba, BOOL pmi, UBYTE ctrl)
{
	cdb->opcode = SCSI_READ_CAPACITY_10;
	cdb->_res1 = RES; cdb->longlba = longlba?1:0; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->_res2 = RES;
	cdb->_res3 = RES;
	cdb->_res4 = RES; cdb->pmi = pmi?1:0;
	cdb->ctrl = ctrl;
}

static BYTE SCSI_ReadCapacity10(struct IOStdReq *io)
{
	struct CDB_ReadCapacity10 cdb;
	struct DAT_ReadCapacity10_short dat;
	BYTE err;

	Set_ReadCapacity10(&cdb, FALSE, FALSE, 0ul, FALSE, 0);
	memset(&dat, 0, sizeof(dat));

	if ((err = DoSCSI(io, &cdb, sizeof(cdb), &dat, sizeof(dat))) == 0) {
		if (dat.lba < 0xffffffff) {
			global->g_DiskSize = (UQUAD)(dat.lba + 1);
			//Log(LD, "%lu blocks, blocksize %lu\n", dat.lba + 1, dat.blocksize);
		} else {
			struct DAT_ReadCapacity10_long dat;

			Set_ReadCapacity10(&cdb, TRUE, FALSE, 0ul, FALSE, 0);
			memset(&dat, 0, sizeof(dat));

			if ((err = DoSCSI(io, &cdb, sizeof(cdb), &dat, sizeof(dat))) == 0) {
				global->g_DiskSize = dat.lba + 1;
				//Log(LD, "%llu blocks, blocksize %lu\n", dat.lba + 1, dat.blocksize);
			}
		}
	}
	return err;
}

/*----------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct CDB_Inquiry
{
	UBYTE opcode;
	UBYTE _res1:6, cmddt:1, evpd:1;
	UBYTE pagecode;
	UBYTE _res2;
	UBYTE al;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct DAT_Inquiry
{
	UBYTE pq:3, pdt:5;
	UBYTE rmb:1, _res1:7;
	UBYTE version;
	UBYTE aerc:1, _obs1:1, normaca:1, hisup:1, rdf:4;
	UBYTE al;
	UBYTE sccs:1, _res2:7;
	UBYTE bque:1, encserv:1, vs1:1, multip:1, mchngr:1, _obs2:1, _obs3:1, addr16:1;
	UBYTE reladr:1, _obs4:1, wbus16:1, sync:1, linked:1, _obs5:1, cmdque:1, vs2:1;
	UBYTE vendor[8];
	UBYTE product[16];
	UBYTE revison[4];
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

static inline void Set_Inquiry(struct CDB_Inquiry *cdb, BOOL cmddt, BOOL evpd, UBYTE pagecode, UBYTE al, UBYTE ctrl)
{
	cdb->opcode = SCSI_INQUIRY;
	cdb->_res1 = RES; cdb->cmddt = cmddt?1:0; cdb->evpd = evpd?1:0;
	cdb->pagecode = pagecode;
	cdb->_res2 = RES;
	cdb->al = al;
	cdb->ctrl = ctrl;
}

static void strcut(char *to, const char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
		to[i] = str[i];
	to[len] = '\0';

	while (--len >= 0) {
		if (to[len] == ' ')
			to[len] = '\0';
		else
			break;
	}
}

static BYTE SCSI_Inquiry(struct IOStdReq *io)
{
	struct CDB_Inquiry cdb;
	struct DAT_Inquiry dat;
	BOOL evpd = FALSE;
	UBYTE	pagecode = 0;
	BYTE err;

	Set_Inquiry(&cdb, FALSE, evpd, pagecode, sizeof(dat), 0);
	memset(&dat, 0, sizeof(dat));

	if ((err = DoSCSI(io, &cdb, sizeof(cdb), &dat, sizeof(dat))) == 0) {
		global->g_DiskRemovable = dat.rmb == 1;

		if (dat.al + 4 + 1 == sizeof(dat)) {
			strcut(global->g_DiskVendor, (const char *)dat.vendor, 8);
			strcut(global->g_DiskProduct, (const char *)dat.product, 16);
			strcut(global->g_DiskRevision, (const char *)dat.revison, 4);
		} else {
			strcpy(global->g_DiskVendor, "Unknown");
			strcpy(global->g_DiskProduct, "Unknown");
			strcpy(global->g_DiskRevision, "");
		}

		/*Log(LD, "ver %u\n", dat.version);
		Log(LD, "rmb %u\n", global->g_DiskRemovable);
		Log(LD, "ven '%s' (%d)\n", global->g_DiskVendor, strlen(global->g_DiskVendor));
		Log(LD, "pro '%s' (%d)\n", global->g_DiskProduct, strlen(global->g_DiskProduct));
		Log(LD, "rev '%s' (%d)\n", global->g_DiskRevision, strlen(global->g_DiskRevision));*/
	}
	return err;
}

/*----------------------------------*/

BYTE SCSI_Query(struct IOStdReq *io)
{
	BYTE err;

	if ((err = SCSI_TestUnitReady(io)))
		return err;

	if ((err = SCSI_ReadCapacity10(io)))
		return err;

	if ((err = SCSI_Inquiry(io)))
		return err;

	return 0;
}

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct CDB_Read10
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, _res2:2, reladdr:1;
	ULONG lba;
	UBYTE _res3;
	UWORD tl;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct CDB_Read12
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, _res2:2, reladdr:1;
	ULONG lba;
	ULONG tl;
	UBYTE _res3;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct CDB_Read16
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, _res2:2, reladdr:1;
	UQUAD lba;
	ULONG tl;
	UBYTE _res3;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

static inline void Set_Read10(struct CDB_Read10 *cdb, BOOL dpo, BOOL fua, BOOL reladdr, ULONG lba, UWORD tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_READ_10;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->_res3 = RES;
	cdb->tl = tl;
	cdb->ctrl = ctrl;
}

static inline void Set_Read12(struct CDB_Read12 *cdb, BOOL dpo, BOOL fua, BOOL reladdr, ULONG lba, ULONG tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_READ_12;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->tl = tl;
	cdb->_res3 = RES;
	cdb->ctrl = ctrl;
}

static inline void Set_Read16(struct CDB_Read16 *cdb, BOOL dpo, BOOL fua, BOOL reladdr, ULONG lba, ULONG tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_READ_16;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->tl = tl;
	cdb->_res3 = RES;
	cdb->ctrl = ctrl;
}

static BYTE SCSI_Read10(struct IOStdReq *io, ULONG lba, UWORD tl, APTR buffer)
{
	struct CDB_Read10 cdb;

	Set_Read10(&cdb, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, (ULONG)tl << BLOCKSIZE_SHIFT);
}

static BYTE SCSI_Read12(struct IOStdReq *io, ULONG lba, ULONG tl, APTR buffer)
{
	struct CDB_Read12 cdb;

	Set_Read12(&cdb, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, tl << BLOCKSIZE_SHIFT);
}

static BYTE SCSI_Read16(struct IOStdReq *io, UQUAD lba, ULONG tl, APTR buffer)
{
	struct CDB_Read16 cdb;

	Set_Read16(&cdb, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, tl << BLOCKSIZE_SHIFT);
}

BYTE SCSI_Read(struct IOStdReq *io, UQUAD lba, ULONG tl, APTR buffer)
{
	if (lba <= 0xffffffffull) {
		if (tl <= 0xffff)
			return SCSI_Read10(io, (ULONG)lba, (UWORD)tl, buffer);
		else
			return SCSI_Read12(io, (ULONG)lba, tl, buffer);
	} else
		return SCSI_Read16(io, lba, tl, buffer);
}

/*----------------------------------*/

#ifdef __MORPHOS__
#pragma pack(1)
#endif

struct CDB_Write10
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, ebp:1, _res2:1, reladdr:1;
	ULONG lba;
	UBYTE _res3;
	UWORD tl;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct CDB_Write12
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, _res2:2, reladdr:1;
	ULONG lba;
	ULONG tl;
	UBYTE _res3;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

struct CDB_Write16
{
	UBYTE opcode;
	UBYTE _res1:3, dpo:1, fua:1, _res2:2, reladdr:1;
	UQUAD lba;
	ULONG tl;
	UBYTE _res3;
	UBYTE ctrl;
}
#ifndef __MORPHOS__
__attribute__((packed))
#endif
;

#ifdef __MORPHOS__
#pragma pack()
#endif

#ifndef __READ_ONLY__

static inline void Set_Write10(struct CDB_Write10 *cdb, BOOL dpo, BOOL fua, BOOL ebp, BOOL reladdr, ULONG lba, UWORD tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_WRITE_10;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->ebp = ebp?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->_res3 = RES;
	cdb->tl = tl;
	cdb->ctrl = ctrl;
}

static inline void Set_Write12(struct CDB_Write12 *cdb, BOOL dpo, BOOL fua, BOOL reladdr, ULONG lba, ULONG tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_WRITE_12;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->_res3 = RES;
	cdb->tl = tl;
	cdb->ctrl = ctrl;
}

static inline void Set_Write16(struct CDB_Write16 *cdb, BOOL dpo, BOOL fua, BOOL reladdr, UQUAD lba, ULONG tl, UBYTE ctrl)
{
	cdb->opcode = SCSI_WRITE_16;
	cdb->_res1 = RES; cdb->dpo = dpo?1:0; cdb->fua = fua?1:0; cdb->_res2 = RES; cdb->reladdr = reladdr?1:0;
	cdb->lba = lba;
	cdb->_res3 = RES;
	cdb->tl = tl;
	cdb->ctrl = ctrl;
}

static BYTE SCSI_Write10(struct IOStdReq *io, ULONG lba, UWORD tl, APTR buffer)
{
	struct CDB_Write10 cdb;

	Set_Write10(&cdb, FALSE, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, (ULONG)tl << BLOCKSIZE_SHIFT);
}

static BYTE SCSI_Write12(struct IOStdReq *io, ULONG lba, ULONG tl, APTR buffer)
{
	struct CDB_Write12 cdb;

	Set_Write12(&cdb, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, tl << BLOCKSIZE_SHIFT);
}

static BYTE SCSI_Write16(struct IOStdReq *io, UQUAD lba, ULONG tl, APTR buffer)
{
	struct CDB_Write16 cdb;

	Set_Write16(&cdb, FALSE, FALSE, FALSE, lba, tl, 0);
	return DoSCSI(io, &cdb, sizeof(cdb), buffer, tl << BLOCKSIZE_SHIFT);
}

#endif

BYTE SCSI_Write(struct IOStdReq *io, UQUAD lba, ULONG tl, APTR buffer)
{
#ifndef __READ_ONLY__
	if (lba <= 0xffffffffull) {
		if (tl <= 0xffff)
			return SCSI_Write10(io, (ULONG)lba, (UWORD)tl, buffer);
		else
			return SCSI_Write12(io, (ULONG)lba, tl, buffer);
	} else
		return SCSI_Write16(io, lba, tl, buffer);
#else
	Log(LF, "write access requested, check driver!\n");
	return IOERR_NOCMD;
#endif
}

