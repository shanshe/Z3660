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

LONG InitTimer(void)
{
	LONG err = ERROR_NO_FREE_STORE;

	if ((global->g_TimerPort = CreateMsgPort())) {
		if ((global->g_TimerReq = (struct timerequest *)CreateIORequest(global->g_TimerPort, sizeof(struct timerequest)))) {
			if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)global->g_TimerReq, 0) == 0) {
				global->g_TimerActive = FALSE;
				global->g_TimerRestart = TRUE;
				return 0;
			} else
				err = ERROR_DEVICE_NOT_MOUNTED;

			DeleteIORequest((struct IORequest *)global->g_TimerReq);
		}
		DeleteMsgPort(global->g_TimerPort);
	}
	return err;
}

void CleanupTimer(void)
{
	if (global->g_TimerActive) {
		AbortIO((struct IORequest *)global->g_TimerReq);
		WaitIO((struct IORequest *)global->g_TimerReq);
	}
	CloseDevice((struct IORequest *)global->g_TimerReq);
	DeleteIORequest((struct IORequest *)global->g_TimerReq);
	DeleteMsgPort(global->g_TimerPort);
}

/*------------------------------------------------------------------------*/

void RestartTimer(void)
{
	if (global->g_TimerActive)
		global->g_TimerRestart = TRUE;
	else {
		global->g_TimerReq->tr_node.io_Command = TR_ADDREQUEST;
		global->g_TimerReq->tr_time.tv_secs = 1;
		global->g_TimerReq->tr_time.tv_micro = 0;
		SendIO((struct IORequest *)global->g_TimerReq);
		global->g_TimerActive = TRUE;
	}
}

void HandleTimer(void)
{
	WaitIO((struct IORequest *)global->g_TimerReq);
	global->g_TimerActive = FALSE;
	if (global->g_TimerRestart) {
		global->g_TimerRestart = FALSE;
		RestartTimer();
	} else
		UpdateDisk();
}

