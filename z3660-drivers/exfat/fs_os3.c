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

APTR AllocVecPooled(APTR pool, ULONG size)
{
	ULONG *mem = NULL;

	if (pool) {
		size += sizeof(ULONG);
		if ((mem = (ULONG *)AllocPooled(pool, size)))
			*mem++ = size;
	}
	return mem;
}

void FreeVecPooled(APTR pool, APTR mem)
{
	if (mem) {
		ULONG *addr = (ULONG *)mem;
		ULONG size  = *--addr;

		FreePooled(pool, (APTR)addr, size);
	}
}

/*------------------------------------------------------------------------*/

LONG DoTimer(struct timeval *tv, LONG unit, LONG cmd)
{
	struct MsgPort *mp;
	struct timerequest *tr;
	LONG error;

	if ((mp = CreateMsgPort()))
	{
		if ((tr = (struct timerequest *)CreateIORequest(mp, sizeof(struct timerequest))))
		{
			if ((error = (LONG)OpenDevice(TIMERNAME, (ULONG)unit, (struct IORequest *)tr, 0)) == 0)
			{
				tr->tr_node.io_Command = cmd;
				tr->tr_time.tv_secs = tv->tv_secs;
				tr->tr_time.tv_micro = tv->tv_micro;

#if 0
				error = (LONG)DoIO((struct IORequest *)tr);
#else
				{
					ULONG sigs, breakf = (SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E);

					SetSignal(0, 1ul << mp->mp_SigBit);

					BeginIO((struct IORequest *)tr);

					sigs = Wait((1ul << mp->mp_SigBit) | breakf);
					if (sigs & breakf)
						error = IOERR_ABORTED;
					else
						error = (LONG)tr->tr_node.io_Error;

					if (!CheckIO((struct IORequest *)tr))
						AbortIO((struct IORequest *)tr);

					WaitIO((struct IORequest *)tr);
				}
#endif
				tv->tv_secs = tr->tr_time.tv_secs;
				tv->tv_micro = tr->tr_time.tv_micro;

				CloseDevice((struct IORequest *)tr);
			}
			DeleteIORequest((struct IORequest *)tr);
		} else
			error = IOERR_OPENFAIL;

		DeleteMsgPort(mp);
	} else
		error = IOERR_OPENFAIL;

	return error;
}

LONG TimeDelay(LONG unit, ULONG secs, ULONG micro)
{
	struct timeval tv = { secs, micro };

	return DoTimer(&tv, unit, TR_ADDREQUEST);
}

/*------------------------------------------------------------------------*/

VOID kputs(CONST_STRPTR string) {};
VOID kprintf(CONST_STRPTR formatString, ...) {};

