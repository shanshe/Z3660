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

void SendNotify(struct NotifyRequest *nr)
{
	struct NotifyMessage *nm;

	if (nr->nr_Flags & NRF_SEND_SIGNAL) {
		Signal(nr->nr_stuff.nr_Signal.nr_Task, 1 << nr->nr_stuff.nr_Signal.nr_SignalNum);
		return;
	}
	if (!(nr->nr_Flags & NRF_SEND_MESSAGE))
		return;
	if ((nr->nr_Flags & NRF_WAIT_REPLY) && nr->nr_MsgCount > 0)
		return;

	nr->nr_MsgCount++;

	nm = AllocVec(sizeof(struct NotifyMessage), MEMF_PUBLIC | MEMF_CLEAR);
	nm->nm_ExecMessage.mn_ReplyPort = global->g_NotifyPort;
	nm->nm_ExecMessage.mn_Length = sizeof(struct NotifyMessage);
	nm->nm_Class = NOTIFY_CLASS;
	nm->nm_Code = NOTIFY_CODE;
	nm->nm_NReq = nr;

	PutMsg(nr->nr_stuff.nr_Msg.nr_Port, (struct Message *)nm);
}

void SendNotifyByLock(struct Volume *v, struct GlobalLock *gl)
{
	struct NotifyNode *nn;

	ForeachNode(&v->v_Info->vi_Notifies, nn) {
		if (nn->nn_GL == gl)
			SendNotify(nn->nn_NR);
	}
}

void SendNotifyByNode(struct Volume *v, struct exFAT_Node *node)
{
	struct NotifyNode *nn;

	ForeachNode(&v->v_Info->vi_Notifies, nn) {
		if (nn->nn_GL) {
			if (nn->nn_GL->gl_EN->en_FPtrCluster == node->en_FPtrCluster && nn->nn_GL->gl_EN->en_FPtrIndex == node->en_FPtrIndex)
				SendNotify(nn->nn_NR);
		} else {
			char *ptr, *ptr8;

			if ((ptr = strchr((const char *)nn->nn_NR->nr_FullName, ':'))) {
				if (*++ptr == '\0')
					continue;
			} else
				continue;

			if ((ptr8 = AllocVecPooled(global->g_Pool, strlen(ptr) * 6 + 1))) {
				struct exFAT *ef = &global->g_Volume->v_exFAT;
				struct exFAT_Node *node2;

				iso8859_to_utf8(ptr8, (const char *)ptr, strlen(ptr));

				if (exfat_lookup(ef, &node2, ptr8) == 0) {
					if (node2->en_FPtrCluster == node->en_FPtrCluster && node2->en_FPtrIndex == node->en_FPtrIndex)
				  		SendNotify(nn->nn_NR);

					exfat_put_node(ef, node2);
				}
				FreeVecPooled(global->g_Pool, ptr8);
			}
		}
	}
}

/*------------------------------------------------------------------------*/

void ProcessNotify(void)
{
	struct NotifyMessage *nm;

	while ((nm = (struct NotifyMessage *)GetMsg(global->g_NotifyPort)) != NULL) {
		if (nm->nm_Class == NOTIFY_CLASS && nm->nm_Code == NOTIFY_CODE) {
			nm->nm_NReq->nr_MsgCount--;
			if (nm->nm_NReq->nr_MsgCount < 0)
				nm->nm_NReq->nr_MsgCount = 0;

			FreeVec(nm);
		}
	}
}

