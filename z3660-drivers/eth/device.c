/*
 * MNT ZZ9000 Network Driver (ZZ9000Net.device)
 * Copyright (C) 2016-2019, Lukas F. Hartmann <lukas@mntre.com>
 *                          MNT Research GmbH, Berlin
 *                          https://mntre.com
 *
 * Based on code copyright (C) 2018 Henryk Richter <henryk.richter@gmx.net>
 * Released under GPLv3+ with permission.
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

#define DEVICE_MAIN

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/expansion.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>
#include <exec/lists.h>
#include <exec/errors.h>
#include <exec/interrupts.h>
#include <exec/tasks.h>
#include <hardware/intbits.h>
#include <string.h>

#ifdef HAVE_VERSION_H
#include "version.h"
#endif

#include "z3660_regs.h"

/* NSD support is optional */
#ifdef NEWSTYLE
#include <devices/newstyle.h>
#endif /* NEWSTYLE */
#ifdef DEVICES_NEWSTYLE_H

const UWORD dev_supportedcmds[] = {
	NSCMD_DEVICEQUERY,
	CMD_READ,
	CMD_WRITE,
	/* ... add all cmds here that are supported by BeginIO */
	0
};

const struct NSDeviceQueryResult NSDQueryAnswer = {
	0,
	16, /* up to SupportedCommands (inclusive) TODO: correct number */
	NSDEVTYPE_SANA2, /* TODO: proper device type */
	0,  /* subtype */
	(UWORD*)dev_supportedcmds
};
#endif /* DEVICES_NEWSTYLE_H */

#include "device.h"
#include "macros.h"

#ifndef DEBUG
#define KPrintF(...)
#endif

// FIXME get rid of global var!
static ULONG ZZ9K_REGS = 0;

__saveds void frame_proc();
char *frame_proc_name = "Z3660NetFramer";

// Z3660 Interrupt Handler (INT6)
__saveds void dev_isr(__reg("a1") struct devbase* db) {
  ULONG status = *(volatile ULONG*)(ZZ9K_REGS+REG_ZZ_INT_STATUS);

//if(status!=0)
//  D(("Z3660Net: status: 0x%lx\n",status));

  // ethernet interrupt signal set?
  if (status & 1) {
    // ack/clear ethernet interrupt
    *(volatile ULONG*)(ZZ9K_REGS+REG_ZZ_CONFIG) = 8|16;

    // signal main process that a packet is available
    if (db->db_Proc) {
      Signal((struct Task*)db->db_Proc, SIGBREAKF_CTRL_F);
    }
  }
/*
  if (status == 1) {
    return 1;
  } else {
    return 0;
  }
*/
}

static UBYTE HW_MAC[] = {0x00,0x00,0x00,0x00,0x00,0x00};

void set_mac_from_string(UBYTE* buf) {
  int k=0;
  for (int i=0; i<6; i++) {
    int c = buf[k];
    int v = 0;

    if (c>='0' && c<='9') c-='0';
    else if (c>='a' && c<='f') c=c+10-'a';
    else if (c>='A' && c<='F') c=c+10-'A';

    v = c<<4;
    c = buf[k+1];

    if (c>='0' && c<='9') c-='0';
    else if (c>='a' && c<='f') c=c+10-'a';
    else if (c>='A' && c<='F') c=c+10-'A';

    HW_MAC[i] = v+c;

    k+=3;
  }
}

struct ProcInit
{
   struct Message msg;
   struct devbase *db;
   BOOL  error;
   UBYTE pad[2];
};

__saveds struct Device *DevInit( ASMR(d0) DEVBASEP                  ASMREG(d0),
                                   ASMR(a0) BPTR seglist              ASMREG(a0),
				   ASMR(a6) struct Library *_SysBase  ASMREG(a6) )
{
	UBYTE*p;
	ULONG i;
	LONG  ok;

	p = ((UBYTE*)db) + sizeof(struct Library);
	i = sizeof(DEVBASETYPE)-sizeof(struct Library);
	while( i-- )
		*p++ = 0;

	db->db_SysBase = _SysBase;
	db->db_SegList = seglist;
	db->db_Flags   = 0;

	ok = 0;
	if( (DOSBase = OpenLibrary("dos.library", 36)) ) {
		if( (UtilityBase = OpenLibrary("utility.library", 37)) ) {
			ok = 0;

      struct ConfigDev* cd = NULL;
      USHORT fwrev = 0;

      if ((ExpansionBase = OpenLibrary("expansion.library", 0)) ) {
        // Find Z2 or Z3 model of Z3660
        if ((cd = (struct ConfigDev*)FindConfigDev(cd,0x144B,0x1)) ) {
          BPTR fh;

          D(("Z3660Net: Z3660 found.\n"));
          ZZ9K_REGS = (ULONG)cd->cd_BoardAddr;

          // Thanks to https://grandcentrix.team
          HW_MAC[0]=0x68;
          HW_MAC[1]=0x82;
          HW_MAC[2]=0xF2;
          HW_MAC[3]=0x00;
          HW_MAC[4]=0x01;
          HW_MAC[5]=0x00;

          if ((fh=Open("ENV:ZZ9K_MAC",MODE_OLDFILE))) {
            UBYTE char_buf[32];
            char* res = FGets(fh,char_buf,18);
            if (!res || strlen(char_buf)<17) {
              D(("Z3660Net: MAC address in ENV:ZZ9K_MAC has invalid syntax.\n"));
            } else {
              D(("Z3660Net: Setting MAC address from ENV:ZZ9K_MAC.\n"));
              set_mac_from_string(char_buf);
            }
            Close(fh);
          }

          // FIXME
          *(ULONG*)(ZZ9K_REGS+REG_ZZ_ETH_MAC_HI) = (HW_MAC[0]<<8)|HW_MAC[1];
          *(ULONG*)(ZZ9K_REGS+REG_ZZ_ETH_MAC_LO) = (HW_MAC[2]<<24)|(HW_MAC[3]<<16)|(HW_MAC[4]<<8)|HW_MAC[5];

          ok = 1;

        } else {
          D(("Z3660Net: Z3660 not found!\n"));
        }
				CloseLibrary(ExpansionBase);
      } else {
        D(("Z3660Net: failed to open expansion.library!\n"));
      }

			if (!ok) {
				CloseLibrary(DOSBase);
				CloseLibrary(UtilityBase);
			}
		}
		else {
			D(("Z3660Net: Could not open utility.library.\n"));
			CloseLibrary(DOSBase);
		}
	}
	else {
		D(("Z3660Net: Could not open dos.library.\n"));
	}

	{
		BPTR fh;
		if ((fh=Open("ENV:ZZ9K_INT2",MODE_OLDFILE))) {
			D(("Z3660Net: Using INT2 mode.\n"));
			Close(fh);
			db->db_Flags |= DEVF_INT2MODE;
		} else {
			D(("Z3660Net: Using INT6 mode (default).\n"));
		}
	}

	/* no hardware found, reject init */
	return (ok > 0) ? (struct Device*)db : (0);
}

__saveds LONG DevOpen( ASMR(a1) struct IOSana2Req *ioreq           ASMREG(a1),
                         ASMR(d0) ULONG unit                         ASMREG(d0),
                         ASMR(d1) ULONG flags                        ASMREG(d1),
                         ASMR(a6) DEVBASEP                           ASMREG(a6) )
{
	LONG ok = 0,ret = IOERR_OPENFAIL;
  struct BufferManagement *bm;

	D(("Z3660Net: DevOpen for %ld\n",unit));

	db->db_Lib.lib_OpenCnt++; /* avoid Expunge, see below for separate "unit" open count */

  if (unit==0 && db->db_Lib.lib_OpenCnt==1) {
    if ((bm = (struct BufferManagement*)AllocVec(sizeof(struct BufferManagement), MEMF_CLEAR|MEMF_PUBLIC))) {
      bm->bm_CopyToBuffer = (BMFunc)GetTagData(S2_CopyToBuff, 0, (struct TagItem *)ioreq->ios2_BufferManagement);
      bm->bm_CopyFromBuffer = (BMFunc)GetTagData(S2_CopyFromBuff, 0, (struct TagItem *)ioreq->ios2_BufferManagement);

      ioreq->ios2_BufferManagement = (VOID *)bm;
      ioreq->ios2_Req.io_Error = 0;
      ioreq->ios2_Req.io_Unit = (struct Unit *)unit; // not a real pointer, but id integer
      ioreq->ios2_Req.io_Device = (struct Device *)db;

      NewList(&db->db_ReadList);
      InitSemaphore(&db->db_ReadListSem);

      struct ProcInit init;
      struct MsgPort *port;

      if ((port = CreateMsgPort())) {
        D(("Z3660Net: Starting Process\n"));
        if ((db->db_Proc = CreateNewProcTags(NP_Entry, frame_proc, NP_Name,
                                            frame_proc_name, NP_Priority, 0, TAG_DONE))) {
          InitSemaphore(&db->db_ProcExitSem);

          init.error = 1;
          init.db = db;
          init.msg.mn_Length = sizeof(init);
          init.msg.mn_ReplyPort = port;

          D(("Z3660Net: handover db: %lx\n",init.db));

          PutMsg(&db->db_Proc->pr_MsgPort, (struct Message*)&init);
          WaitPort(port);

          if (!init.error) {
            ok = 1;

            // Register Interrupt server
            if ((db->db_interrupt = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR))) {
              db->db_interrupt->is_Node.ln_Type = NT_INTERRUPT;
              db->db_interrupt->is_Node.ln_Pri = -60;
              db->db_interrupt->is_Node.ln_Name = "Z3660Net";
              db->db_interrupt->is_Data = (APTR)db;
              db->db_interrupt->is_Code = dev_isr;

              Disable();
              AddIntServer((db->db_Flags & DEVF_INT2MODE) ? INTB_PORTS : INTB_EXTER, db->db_interrupt);
              Enable();

              D(("Z3660Net: Interrupt server registered, using INT%ld\n",(db->db_Flags & DEVF_INT2MODE) ? 2L : 6L));
              ret = 0;
              ok = 1;

              // enable HW interrupt
              //ULONG hw_config = *(volatile ULONG*)(ZZ9K_REGS+REG_ZZ_INT_STATUS);
              //hw_config |= 1;
              *(volatile ULONG*)(ZZ9K_REGS+REG_ZZ_CONFIG) = 1;

              D(("Z3660Net: ZZ interrupt enabled\n"));
            } else {
              D(("Z3660Net: failed to alloc struct Interrupt\n"));
              ret = IOERR_OPENFAIL;
              ok = 0;

              Signal((struct Task*)db->db_Proc, SIGBREAKF_CTRL_C);
              // this will block until the process has really quit and released the semaphore
              ObtainSemaphore(&db->db_ProcExitSem);
              ReleaseSemaphore(&db->db_ProcExitSem);
            }
          } else {
            D(("Z3660Net:process startup error\n"));
            ret = IOERR_OPENFAIL;
            ok = 0;
          }
        } else {
          D(("Z3660Net:couldn't create process\n"));
          ret = IOERR_OPENFAIL;
          ok = 0;
        }
        DeleteMsgPort(port);
      }
    }
  } else {
    ret = IOERR_OPENFAIL;
    ok = 0;
  }

	if (ok) {
		ret = 0;
    db->db_Lib.lib_Flags &= ~LIBF_DELEXP;
	}

	if (ret == IOERR_OPENFAIL) {
		ioreq->ios2_Req.io_Unit   = (0);
		ioreq->ios2_Req.io_Device = (0);
		ioreq->ios2_Req.io_Error  = ret;
		db->db_Lib.lib_OpenCnt--;
	}
	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;

	D(("Z3660Net: DevOpen return code %ld\n",ret));

	return ret;
}

__saveds BPTR DevClose(   ASMR(a1) struct IORequest *ioreq        ASMREG(a1),
                            ASMR(a6) DEVBASEP                       ASMREG(a6) )
{
	/* ULONG unit; */
	BPTR  ret = (0);

	D(("Z3660Net: DevClose open count %ld\n",db->db_Lib.lib_OpenCnt));

	if( !ioreq )
		return ret;

	db->db_Lib.lib_OpenCnt--;

  if (db->db_Lib.lib_OpenCnt == 0) {
    // disable HW interrupt
//    ULONG hw_config = *(ULONG*)(ZZ9K_REGS+REG_ZZ_INT_STATUS);
//    hw_config &= 0xfffffffe;
    *(volatile ULONG*)(ZZ9K_REGS+REG_ZZ_CONFIG) = 0;

    D(("Z3660Net: ZZ interrupt disabled\n"));

    if (db->db_interrupt) {
      D(("Z3660Net: Remove IntServer...\n"));
      Forbid();
      RemIntServer((db->db_Flags & DEVF_INT2MODE) ? INTB_PORTS : INTB_EXTER, db->db_interrupt);
      db->db_interrupt = 0;
      Permit();
    }
    if (db->db_Proc) {
      D(("Z3660Net: End Proc...\n"));
      Signal((struct Task*)db->db_Proc, SIGBREAKF_CTRL_C);
      db->db_Proc = 0;

      ObtainSemaphore(&db->db_ProcExitSem);
      ReleaseSemaphore(&db->db_ProcExitSem);
    }
  }

	ioreq->io_Device = (0);
	ioreq->io_Unit   = (struct Unit *)(-1);

	if (db->db_Lib.lib_Flags & LIBF_DELEXP)
		ret = DevExpunge(db);

	return ret;
}

__saveds BPTR DevExpunge( ASMR(a6) DEVBASEP                        ASMREG(a6) )
{
	BPTR seglist = db->db_SegList;

	if( db->db_Lib.lib_OpenCnt )
	{
		db->db_Lib.lib_Flags |= LIBF_DELEXP;
		return (0);
	}

  D(("Z3660Net: Remove Device Node...\n"));
  Remove((struct Node*)db);

	CloseLibrary(DOSBase);
	CloseLibrary(UtilityBase);
	FreeMem( ((BYTE*)db)-db->db_Lib.lib_NegSize,(ULONG)(db->db_Lib.lib_PosSize + db->db_Lib.lib_NegSize));

	return seglist;
}

ULONG read_frame(struct IOSana2Req *req, volatile UBYTE *frame);
ULONG write_frame(struct IOSana2Req *req, UBYTE *frame);

__saveds VOID DevBeginIO( ASMR(a1) struct IOSana2Req *ioreq       ASMREG(a1),
                            ASMR(a6) DEVBASEP                       ASMREG(a6) )
{
	ULONG unit = (ULONG)ioreq->ios2_Req.io_Unit;
  int mtu;

	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
  ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
  ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;

	//D(("BeginIO command %ld unit %ld\n",(LONG)ioreq->ios2_Req.io_Command,unit));

	switch( ioreq->ios2_Req.io_Command ) {
  case CMD_READ:
    if (ioreq->ios2_BufferManagement == NULL) {
      ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
      ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
    }
    else {
      // not quick, add request to reader list
      // will be handled on interrupts by frame_proc
      ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
      ObtainSemaphore(&db->db_ReadListSem);
      AddHead((struct List*)&db->db_ReadList, (struct Node*)ioreq);
      ReleaseSemaphore(&db->db_ReadListSem);
      ioreq = NULL;
    }
    break;
  case S2_BROADCAST:
    /* set broadcast addr: ff:ff:ff:ff:ff:ff */
    if (ioreq->ios2_DstAddr) {
      memset(ioreq->ios2_DstAddr, 0xff, HW_ADDRFIELDSIZE);
    } else {
      D(("bcast: invalid dst addr\n"));
    }
    /* fall through */
  case CMD_WRITE: {
//    ULONG res = write_frame(ioreq, (UBYTE*)(ZZ9K_REGS+REG_ZZ_TX_BUFF));
D(("write_frame: ZZ9K_REGS+TX_FRAME_ADDRESS = 0x%lx\n",ZZ9K_REGS+TX_FRAME_ADDRESS));
    ULONG res = write_frame(ioreq, (UBYTE*)(ZZ9K_REGS+TX_FRAME_ADDRESS));
    if (res!=0) {
      ioreq->ios2_Req.io_Error = S2ERR_NO_RESOURCES;
      ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
    } else {
      ioreq->ios2_Req.io_Error = 0;
    }
    break;
  }

  case S2_READORPHAN:
    if( !ioreq->ios2_BufferManagement )
			{
				ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
				ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
			}
    else
			{
        ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
        // FIXME do we need this list?
        //ObtainSemaphore(&db->db_Units[unit].du_Sem);
        //AddHead((struct List*)&db->db_ReadOrphans,(struct Node*)ioreq);
        //ReleaseSemaphore(&db->db_Units[unit].du_Sem);
        ioreq = NULL;
			}
    break;
  case S2_ONLINE:
  case S2_OFFLINE:
  case S2_CONFIGINTERFACE:   /* forward request */
    break;

  case S2_GETSTATIONADDRESS:
    memcpy(ioreq->ios2_SrcAddr, HW_MAC, HW_ADDRFIELDSIZE); /* current */
    memcpy(ioreq->ios2_DstAddr, HW_MAC, HW_ADDRFIELDSIZE); /* default */
    break;
  case S2_DEVICEQUERY:
    {
      struct Sana2DeviceQuery *devquery;

      devquery = ioreq->ios2_StatData;
      devquery->DevQueryFormat = 0;        /* "this is format 0" */
      devquery->DeviceLevel = 0;           /* "this spec defines level 0" */

      if (devquery->SizeAvailable >= 18) devquery->AddrFieldSize = HW_ADDRFIELDSIZE * 8; /* Bits! */
      if (devquery->SizeAvailable >= 22) devquery->MTU           = 1500;
      if (devquery->SizeAvailable >= 26) devquery->BPS           = 1000*1000*100;
      if (devquery->SizeAvailable >= 30) devquery->HardwareType  = S2WireType_Ethernet;

      devquery->SizeSupplied = (devquery->SizeAvailable<30?devquery->SizeAvailable:30);
    }
    break;
  case S2_GETSPECIALSTATS:
    {
      struct Sana2SpecialStatHeader *s2ssh = (struct Sana2SpecialStatHeader *)ioreq->ios2_StatData;
      s2ssh->RecordCountSupplied = 0;
    }
    break;
  default:
    {
      ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
      ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
      break;
    }
	}

	if (ioreq) {
		DevTermIO(db, (struct IORequest*)ioreq);
  }
}

__saveds LONG DevAbortIO( ASMR(a1) struct IORequest *ioreq        ASMREG(a1),
                            ASMR(a6) DEVBASEP                       ASMREG(a6) )
{
	LONG   ret = 0;
  struct IOSana2Req* ios2 = (struct IOSana2Req*)ioreq;

	D(("Z3660Net: AbortIO on %lx\n",(ULONG)ioreq));

  Remove((struct Node*)ioreq);

	ioreq->io_Error = IOERR_ABORTED;
  ios2->ios2_WireError = 0;

	ReplyMsg((struct Message*)ioreq);
	return ret;
}

void DevTermIO( DEVBASEP, struct IORequest *ioreq )
{
  struct IOSana2Req* ios2 = (struct IOSana2Req*)ioreq;

  if (!(ios2->ios2_Req.io_Flags & SANA2IOF_QUICK)) {
    ReplyMsg((struct Message *)ioreq);
  } else {
    ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
  }
}

ULONG get_frame_serial(volatile UBYTE* frame) {
  volatile UBYTE* frm = (volatile UBYTE*)frame;
  ULONG ser  = ((ULONG)frm[2]<<8)|((ULONG)frm[3]);
  return ser;
}

ULONG read_frame(struct IOSana2Req *req, volatile UBYTE *frame)
{
  ULONG datasize;
  BYTE *frame_ptr;
  BOOL broadcast;
  ULONG err = 0;
  struct BufferManagement *bm;

  UBYTE* frm = (UBYTE*)frame;
  ULONG sz   = ((ULONG)frm[0]<<8)|((ULONG)frm[1]);
  ULONG ser  = ((ULONG)frm[2]<<8)|((ULONG)frm[3]);
  USHORT tp  = ((USHORT)frm[16]<<8)|((USHORT)frm[17]);

  if (req->ios2_Req.io_Flags & SANA2IOF_RAW) {
    frame_ptr = frm+4;
    datasize = sz;
    req->ios2_Req.io_Flags = SANA2IOF_RAW;
  }
  else {
    frame_ptr = frm+4+HW_ETH_HDR_SIZE;
    datasize = sz-HW_ETH_HDR_SIZE;
    req->ios2_Req.io_Flags = 0;
  }

  req->ios2_DataLength = datasize;

  //D(("datasize: %lx\n",datasize));
  //D(("frame_ptr: %lx\n",frame_ptr));
  //D(("ios2_Data: %lx\n",req->ios2_Data));
  //D(("bufmgmt: %lx\n",req->ios2_BufferManagement));

  // copy frame to device user (probably tcp/ip system)
  bm = (struct BufferManagement *)req->ios2_BufferManagement;
  if (!(*bm->bm_CopyToBuffer)(req->ios2_Data, frame_ptr, datasize)) {
    //D(("rx copybuf error\n"));
    req->ios2_Req.io_Error = S2ERR_SOFTWARE;
    req->ios2_WireError = S2WERR_BUFF_ERROR;
    err = 1;
  }
  else {
    req->ios2_Req.io_Error = req->ios2_WireError = 0;
    err = 0;
  }

  memcpy(req->ios2_SrcAddr, (UBYTE *)frame+4+6, HW_ADDRFIELDSIZE);
  memcpy(req->ios2_DstAddr, (UBYTE *)frame+4, HW_ADDRFIELDSIZE);

  //D(("RXSZ %ld\n",(LONG)sz));
  //D(("RXPT %ld\n",(LONG)tp));

  //D(("RXSER %ld\n",(LONG)ser));
  //D(("RXDST %lx...\n",*((ULONG*)(req->ios2_DstAddr))));
  //D(("RXSRC %lx\n",*((ULONG*)(req->ios2_SrcAddr))));
  //D(("RXSRC %lx\n",*((ULONG*)(frame_ptr))));

  broadcast = TRUE;
  for (int i=0; i<HW_ADDRFIELDSIZE; i++) {
    if (frame[i+4] != 0xff) {
      broadcast = FALSE;
      break;
    }
  }
  if (broadcast) {
    req->ios2_Req.io_Flags |= SANA2IOF_BCAST;
  }

  req->ios2_PacketType = tp;

  return err;
}

ULONG write_frame(struct IOSana2Req *req, UBYTE *frame)
{
  ULONG rc=0;
  struct BufferManagement *bm;
  USHORT sz=0;
  UBYTE *fram=frame;

  if (req->ios2_Req.io_Flags & SANA2IOF_RAW) {
    sz = req->ios2_DataLength;
  } else {
    sz = req->ios2_DataLength + HW_ETH_HDR_SIZE;
    *((USHORT*)(frame+6+6)) = (USHORT)req->ios2_PacketType;
    memcpy(frame, req->ios2_DstAddr, HW_ADDRFIELDSIZE);
    memcpy(frame+6, HW_MAC, HW_ADDRFIELDSIZE);
    frame+=HW_ETH_HDR_SIZE;
  }
  char hex[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  for(int i=0;i<sz;i++)
  {
    KPrintF("%c%c",hex[(fram[i]>>4)&0xF]); // seems there are some bugs in KPrintF...
    KPrintF("%c%c",hex[fram[i]&0xF]);
    if(i%4==3) {
      KPrintF(" ");
    }
    if(i%32==31) {
      KPrintF("\n");
    }
  }
  KPrintF("\n===============================\n");

  if (sz>0) {
    bm = (struct BufferManagement *)req->ios2_BufferManagement;

    if (!(*bm->bm_CopyFromBuffer)(frame, req->ios2_Data, req->ios2_DataLength)) {
      rc = 1; // FIXME error code
      //D(("tx copybuf err\n"));
    }
    else {
      // buffer was copied to Z3660, send it
      volatile ULONG* reg = (volatile ULONG*)(ZZ9K_REGS+REG_ZZ_ETH_TX);
      *reg = (ULONG)sz;

      // get feedback
      rc = *reg;
      if (rc!=0) {
        D(("tx err: %ld\n",rc));
      }
    }
  }

  return rc;
}

__saveds void frame_proc() {
  ULONG wmask;

  D(("Z3660Net: frame_proc()\n"));

  struct ProcInit* init;

  {
    struct { void *db_SysBase; } *db = (void*)0x4;
    struct Process* proc;

    proc = (struct Process*)FindTask(NULL);
  D(("Z3660Net: WaitPort()\n"));
    WaitPort(&proc->pr_MsgPort);
    init = (struct ProcInit*)GetMsg(&proc->pr_MsgPort);
  }

  struct devbase* db = init->db;

  init->error = 0;
  db = init->db;
  ObtainSemaphore(&db->db_ProcExitSem);
  ReplyMsg((struct Message*)init);

  wmask = SIGBREAKF_CTRL_F | SIGBREAKF_CTRL_C;

  ULONG old_serial = 0;
  ULONG recv = 0;
D(("Z3660Net: wait for the first packet\n"));
  // wait for the first packet
  recv = Wait(wmask);

  while (1) {

    struct IOSana2Req *ior;
    BOOL receiver_found = 0;

    // wait for signal from our interrupt handler
    // remove this to use polled-IO

    if (recv & SIGBREAKF_CTRL_C) {
      D(("Z3660Net: process end\n"));
      break;
    }
// read first the address of frame_received_from_backlog
    uint32_t address_of_rx_buff = *((volatile ULONG*)(ZZ9K_REGS+REG_ZZ_ETH_RX_ADDRESS));

    D(("address_of_rx_buff: 0x%lx\n",ZZ9K_REGS+address_of_rx_buff));

    volatile UBYTE* frm = (volatile UBYTE*)(ZZ9K_REGS+address_of_rx_buff);
    ULONG serial = get_frame_serial(frm);

    //D(("FTI %ld\n", serial));
    if (serial != old_serial) {
      int processed = 0;
      USHORT packet_type = ((USHORT)frm[16]<<8)|((USHORT)frm[17]);
      old_serial = serial;

      ObtainSemaphore(&db->db_ReadListSem);
      for (ior = (struct IOSana2Req *)db->db_ReadList.lh_Head;
           ior->ios2_Req.io_Message.mn_Node.ln_Succ;
           ior = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) {
        if (ior->ios2_PacketType == packet_type) {
          ULONG res = read_frame(ior, frm);
          if (res==0) {
            Remove((struct Node*)ior);
            ReplyMsg((struct Message *)ior);
            processed = 1;
          } else {
            D(("RERR %ld\n",res));
          }
          break;
        }
      }
      ReleaseSemaphore(&db->db_ReadListSem);

      if (!processed) {
        //D(("UNPR %ld\n",(LONG)packet_type));
      }

      // mark this frame as accepted
      volatile ULONG* reg = (volatile ULONG*)(ZZ9K_REGS+REG_ZZ_ETH_RX);
      *reg = 1L;
    } else {
      // if there are no more new packets, idle until the next interrupt
      recv = Wait(wmask);
    }
  }

  Forbid();
  ReleaseSemaphore(&db->db_ProcExitSem);
}

