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

struct DosLibrary *DOSBase = NULL;
UTILITYBASE *UtilityBase = NULL;
#ifndef __MORPHOS__
UTILITYBASE *__UtilityBase = NULL;
#ifndef __68881__
struct MathIeeeDoubBasBase *MathIeeeDoubBasBase = NULL, *__MathIeeeDoubBasBase = NULL;
struct MathIeeeDoubTransBase *MathIeeeDoubTransBase = NULL, *__MathIeeeDoubTransBase = NULL;
#endif
#endif

struct Global *global = NULL;

const char *version = "$VER:"FULL_NAME" "VERSION"."REVISION" ["ARCH"] ("__AMIGADATE__") 2014-"__YEAR__" Rupert Hausberger";

#ifdef __MORPHOS__
#define LIB_VERSION 50
#else
#define LIB_VERSION 37
#endif

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
	#pragma pack(2)
#endif

static struct {
	APTR 	*base;
	UBYTE *name;
	ULONG  minver;
} libs[] = {
	{ (APTR *)&DOSBase, "dos.library", LIB_VERSION },
	{ (APTR *)&UtilityBase, "utility.library", LIB_VERSION },
#ifndef __MORPHOS__
#ifndef __68881__
	{ (APTR *)&MathIeeeDoubBasBase, "mathieeedoubbas.library", LIB_VERSION },
	{ (APTR *)&MathIeeeDoubTransBase, "mathieeedoubtrans.library", LIB_VERSION },
#endif
#endif
	{ NULL, NULL }
};

#ifdef __MORPHOS__
	#pragma pack()
#endif

static void ExitLibraries(void)
{
	int i;

	for (i = 0; libs[i].base; i++) {
		if (*libs[i].base)
			CloseLibrary(*libs[i].base);
	}
}

static BOOL InitLibraries(LONG version)
{
	int i;

	for (i = 0; libs[i].base; i++) {
		if (!(*(libs[i].base) = (APTR)OpenLibrary(libs[i].name, libs[i].minver))) {
			Log(LF, "Can't open %s v%lu", libs[i].name, libs[i].minver);
			Pop(LF, "Can't open %s v%lu", libs[i].name, libs[i].minver);
			ExitLibraries();
			return FALSE;
		}
	}
#ifndef __MORPHOS__
	__UtilityBase = UtilityBase;
#ifndef __68881__
	__MathIeeeDoubBasBase = MathIeeeDoubBasBase;
	__MathIeeeDoubTransBase = MathIeeeDoubTransBase;
#endif
#endif
	return TRUE;
}

/*------------------------------------------------------------------------*/

#ifdef __MORPHOS__
static int DiskChangeInt(void)
{
	struct IntData *id = (APTR)REG_A1;
#else
static int DiskChangeInt(struct IntData *id __asm("a1"))
{
#endif
	struct ExecBase *SysBase = id->id_SysBase;

	Signal(id->id_Task, id->id_Signal);
	return 0;
}
#ifdef __MORPHOS__
static const struct EmulLibEntry gate_DiskChangeInt = {
	TRAP_LIB, 0, (void (*)(void))DiskChangeInt
};
#endif

static void SetupDiskChangeInt(void)
{
	struct IntData *id = &global->g_DiskChangeIntData;

	id->id_Interrupt.is_Node.ln_Type = NT_INTERRUPT;
	id->id_Interrupt.is_Node.ln_Pri = 0;
	id->id_Interrupt.is_Node.ln_Name = FULL_NAME" [diskchange]";
	id->id_Interrupt.is_Data = id;
#ifdef __MORPHOS__
	id->id_Interrupt.is_Code = (void (*)(void))&gate_DiskChangeInt;
#else
	id->id_Interrupt.is_Code = (void (*)(void))&DiskChangeInt;
#endif
	id->id_SysBase = SysBase;
	id->id_Task = global->g_OurTask;
	id->id_Signal = 1ul << global->g_DiskChgSig;
	id->id_Count = 0ul;
}

static void AddDiskChangeInt(void)
{
	struct IntData *id = &global->g_DiskChangeIntData;

	global->g_DiskChgReq->iotd_Req.io_Command = TD_ADDCHANGEINT;
	global->g_DiskChgReq->iotd_Req.io_Data = &id->id_Interrupt;
	global->g_DiskChgReq->iotd_Req.io_Length = sizeof(struct Interrupt);
	global->g_DiskChgReq->iotd_Req.io_Flags = 0;
	SendIO((struct IORequest*)global->g_DiskChgReq);
}

static void RemDiskChangeInt(void)
{
	struct IntData *id = &global->g_DiskChangeIntData;

	global->g_DiskChgReq->iotd_Req.io_Command = TD_REMCHANGEINT;
	global->g_DiskChgReq->iotd_Req.io_Data = &id->id_Interrupt;
	global->g_DiskChgReq->iotd_Req.io_Length = sizeof(struct Interrupt);
	global->g_DiskChgReq->iotd_Req.io_Flags = 0;
	DoIO((struct IORequest*)global->g_DiskChgReq);
}

static LONG InitDiskHandler(struct FileSysStartupMsg *fssm)
{
	LONG err;

	if ((global->g_DiskChgSig = (LONG)AllocSignal(-1)) >= 0)
	{
		if ((global->g_DiskPort = CreateMsgPort()))
		{
			if ((global->g_DiskIOReq = CreateIORequest(global->g_DiskPort, sizeof(struct IOExtTD))))
			{
				char device[64];

				bstr2cstr_buf(BADDR(fssm->fssm_Device), device, sizeof(device));

				if (OpenDevice((CONST_STRPTR)device, fssm->fssm_Unit, (struct IORequest *)global->g_DiskIOReq, fssm->fssm_Flags) == 0)
				{
					if ((global->g_DiskChgReq = AllocVec(sizeof(struct IOExtTD), MEMF_PUBLIC)))
					{
						CopyMem(global->g_DiskIOReq, global->g_DiskChgReq, sizeof(struct IOExtTD));

						SetupDiskChangeInt();
						AddDiskChangeInt();
						return 0;
					} else
						err = ERROR_NO_FREE_STORE;

					CloseDevice((struct IORequest *)global->g_DiskIOReq);
				} else
					err = ERROR_DEVICE_NOT_MOUNTED;

				DeleteIORequest(global->g_DiskIOReq);
				global->g_DiskIOReq = NULL;
			} else
				err = ERROR_NO_FREE_STORE;

			DeleteMsgPort(global->g_DiskPort);
			global->g_DiskPort = NULL;
		} else
			err = ERROR_NO_FREE_STORE;

		FreeSignal(global->g_DiskChgSig);
		global->g_DiskChgSig = 0;
	} else
		err = ERROR_NO_FREE_STORE;

	return err;
}

static void ExitDiskHandler(void)
{
	RemDiskChangeInt();

	CloseDevice((struct IORequest *)global->g_DiskIOReq);
	DeleteIORequest(global->g_DiskIOReq);
	FreeVec(global->g_DiskChgReq);
	DeleteMsgPort(global->g_DiskPort);

	global->g_DiskIOReq = NULL;
	global->g_DiskChgReq = NULL;
	global->g_DiskPort = NULL;

	FreeSignal(global->g_DiskChgSig);
}

/*------------------------------------------------------------------------*/

LONG handler(void)
{
	LONG error;

	if ((global = AllocVec(sizeof(struct Global), MEMF_PUBLIC | MEMF_CLEAR)))
	{
		struct Message *msg;
		struct DosPacket *startupPacket;

		NewList((struct List *)&global->g_BusyVolumes);
		global->g_OurTask = FindTask(NULL);
		global->g_OurPort = &((struct Process *)global->g_OurTask)->pr_MsgPort;
		WaitPort(global->g_OurPort);

		msg = GetMsg(global->g_OurPort);
		startupPacket = (struct DosPacket *)msg->mn_Node.ln_Name;
		global->g_FSSM = BADDR(startupPacket->dp_Arg2);
		global->g_DevNode = BADDR(startupPacket->dp_Arg3);

		if (InitLibraries(LIB_VERSION))
		{
			if ((global->g_NotifyPort = CreateMsgPort()))
			{
				if ((global->g_Pool = CreatePool(MEMF_PUBLIC, 8192, 8192)))
				{
					if ((error = InitTimer()) == 0)
					{
						if ((error = InitDiskHandler(global->g_FSSM)) == 0)
						{
							ULONG pktsig = 1ul << global->g_OurPort->mp_SigBit;
							ULONG diskchgsig = 1ul << global->g_DiskChgSig;
							ULONG notifysig = 1ul << global->g_NotifyPort->mp_SigBit;
							ULONG timersig = 1ul << global->g_TimerPort->mp_SigBit;
							ULONG mask = pktsig | diskchgsig | notifysig | timersig;
							ULONG sigs;

							global->g_DevNode->dol_Task = global->g_OurPort;

							startupPacket->dp_Res1 = DOSTRUE;
							startupPacket->dp_Res2 = 0;
							ReplyPacket(startupPacket);

							ProcessDiskChange(); /* Insert disk */

							while (!global->g_Quit) {
								sigs = Wait(mask);
								if (sigs & diskchgsig)
									ProcessDiskChange();
								if (sigs & pktsig)
									ProcessPackets();
								if (sigs & notifysig)
									ProcessNotify();
								if (sigs & timersig)
									HandleTimer();
							}

							error = 0;
							startupPacket = NULL;

							ExitDiskHandler();
						}
						CleanupTimer();
					}
					DeletePool(global->g_Pool);
				} else
					error = ERROR_NO_FREE_STORE;

				DeleteMsgPort(global->g_NotifyPort);
			} else
				error = ERROR_NO_FREE_STORE;

			ExitLibraries();
		} else
			error = ERROR_INVALID_RESIDENT_LIBRARY;

		if (global->g_DeathPacket)
			ReplyPacket(global->g_DeathPacket);

		if (startupPacket) {
			startupPacket->dp_Res1 = error == 0 ? DOSTRUE : DOSFALSE;
			startupPacket->dp_Res2 = error;
			ReplyPacket(startupPacket);
		}

		FreeVec(global);
	} else
		error = ERROR_NO_FREE_STORE;

	return error;
}

