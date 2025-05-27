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

static const char *modes[] = { "DEBUG","WARN","ERROR","FATAL" };
static const char *modes2[] = {
	FULL_NAME" debug",
	FULL_NAME" warning",
	FULL_NAME" error",
	FULL_NAME" fatal error"
};

void Log(short mode, const char *fmt, ...)
{
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	kprintf("exFAT[%s]: %s", modes[mode], buf);
}

void Pop(short mode, const char *fmt, ...)
{
	struct IntuitionBase *IntuitionBase;
	char *buf;
	va_list args;
	ULONG len;

	va_start(args, fmt);
	len = (ULONG)vscountf(fmt, args);
	va_end(args);

	if ((buf = AllocMem(len+1, MEMF_PUBLIC))) {
		va_start(args, fmt);
		vsprintf(buf, fmt, args);
		va_end(args);

		if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0))) {
			struct EasyStruct es;

			es.es_StructSize = sizeof(struct EasyStruct);
			es.es_Flags = 0ul;
			es.es_Title = (UBYTE *)modes2[mode];
			es.es_TextFormat = (UBYTE *)buf;
			es.es_GadgetFormat = mode == LF ? "Damn" : "Ok";

			EasyRequestArgs(NULL, &es, NULL, NULL);
			CloseLibrary((struct Library *)IntuitionBase);
		}
		FreeMem(buf, len+1);
	}
}

/*------------------------------------------------------------------------*/

ULONG GetBlockSize(struct DosEnvec *de) {
	return de->de_SizeBlock << 2;
}

UQUAD GetNumBlocks(struct DosEnvec *de) {
	return (UQUAD)(de->de_HighCyl - de->de_LowCyl + 1) * de->de_Surfaces * de->de_BlocksPerTrack - de->de_Reserved;
}

UQUAD GetLowBlock(struct DosEnvec *de) {
	return (UQUAD)de->de_LowCyl * de->de_Surfaces * de->de_BlocksPerTrack + de->de_Reserved;
}

UQUAD GetHighBlock(struct DosEnvec *de) {
	return (UQUAD)(de->de_HighCyl + 1) * de->de_Surfaces * de->de_BlocksPerTrack - 1;
}

/*------------------------------------------------------------------------*/

void SendEvent(LONG event) {
	struct IOStdReq *InputRequest;
	struct MsgPort *InputPort;
	struct InputEvent *ie;

	if ((InputPort = (struct MsgPort*)CreateMsgPort())) {
		if ((InputRequest = (struct IOStdReq*)CreateIORequest(InputPort, sizeof(struct IOStdReq)))) {
			if (!OpenDevice("input.device", 0, (struct IORequest*)InputRequest, 0)) {
				if ((ie = AllocVec(sizeof(struct InputEvent), MEMF_PUBLIC | MEMF_CLEAR))) {
					ie->ie_Class = event;
					InputRequest->io_Command = IND_WRITEEVENT;
					InputRequest->io_Data = ie;
					InputRequest->io_Length = sizeof(struct InputEvent);

					DoIO((struct IORequest*)InputRequest);
					FreeVec(ie);
				}
				CloseDevice((struct IORequest*)InputRequest);
			}
			DeleteIORequest((struct IORequest *)InputRequest);
		}
		DeleteMsgPort (InputPort);
	}
}

/*------------------------------------------------------------------------*/

void dec2hex(ULONG dec, char *hex)
{
	UBYTE *p = (UBYTE *)(hex + 8);

	*p-- = '\0';
	while (dec != 0) {
		int hv = (int)(dec % 16);
		UBYTE	hc = (UBYTE)((hv >= 0 &&  hv <= 9) ? (hv + '0') : (hv - 10 + 'A'));

		*p-- = hc;
		dec /= 16;
	}
}

void cstr2bstr_buf(const char *c, char *b, int limit)
{
	int l = strlen(c);

	if (l > limit) l = limit;
	if (l > 127) l = 127;

	*b++ = (char)l;
	while (l-- > 0)
		*b++ = *c++;
}

void bstr2cstr_buf(const char *b, char *c, int limit)
{
	int l = (int)*b++;

	if (l > limit-1) l = limit-1;

	while (l-- > 0)
		*c++ = *b++;
	*c = '\0';
}

char *bstr2cstr(const char *b)
{
	int l = (int)*b++;
	char *cstr;

	if ((cstr = AllocVecPooled(global->g_Pool, (ULONG)l + 1))) {
		char *c = cstr;

		while (l-- > 0)
			*c++ = *b++;

		*c = '\0';
	}
	return cstr;
}

/*------------------------------------------------------------------------*/

char *ParentPath(const char *s)
{
	int l = strlen(s);
	char *d;

	while (--l > 0) {
		if (s[l] == '/')
			break;
	}
	if (l == 0)
		return NULL;

	if ((d = AllocVecPooled(global->g_Pool, (ULONG)l + 1)))
		strncpy(d, s, l);
	d[l] = '\0';

	return d;
}

/*------------------------------------------------------------------------*/

static BOOL	FixPath(char *path)
{
	char *ptr = path;

	while (*ptr) {
		if (*ptr == '/') {
			if (*(ptr+1) == '/') {
				char *tmp = ptr - 1;

				if (tmp > path) {
					while (tmp > path) {
						if (*tmp == '/') break;
						tmp--;
					}

					{
						/* move */
						char *s = ptr + 1;
						char *d = tmp;

						while (*s) *d++ = *s++;
						*d = '\0';

						/* restart */
						ptr = path;
						continue;
					}
				} else
					return FALSE; /* behind root */
			}
		}
		ptr++;
	}

	ptr--;
	if (*ptr == '/')
		*ptr = '\0';

	return TRUE;
}

static char *BulidPath(char *dir, char *name, LONG *error)
{
	char *path;

	if (dir == NULL) {
		char *ptr;

		/* absolute dir */
		if ((ptr = strchr(name, ':'))) {
			ptr++;
			if (!(path = AllocVecPooled(global->g_Pool, strlen(ptr) + 1))) {
				*error = ERROR_NO_FREE_STORE;
				return NULL;
			}
			strcpy(path, ptr);
			*error = 0l;
			return path;
		}
		/* current dir */
		else {
			if (!(path = AllocVecPooled(global->g_Pool, strlen(name) + 1))) {
				*error = ERROR_NO_FREE_STORE;
				return NULL;
			}
			strcpy(path, name);
		}
	}
	/* dir relative to another dir */
	else {
		if (!(path = AllocVecPooled(global->g_Pool, strlen(dir) + 1 + strlen(name) + 1))) {
			*error = ERROR_NO_FREE_STORE;
			return NULL;
		}
		strcpy(path, dir);
		strcat(path, "/");
		strcat(path, name);
	}

	/* special case */
	if (*path == '/' && *(path+1) == '\0') {
		FreeVecPooled(global->g_Pool, path);
		*error = ERROR_OBJECT_NOT_FOUND;
		return NULL;
	}

	/* handle multi-parent */
	if (FixPath(path))
		return path;
	else {
		FreeVecPooled(global->g_Pool, path);
		*error = ERROR_OBJECT_NOT_FOUND;
		return NULL;
	}
}

char *MakePath(struct ExtFileLock *fl, const char *name, LONG *error)
{
	char *path;
	char *name8;

	if ((name8 = AllocVecPooled(global->g_Pool, strlen(name) * 6 + 1)))
	{
		if ((*error = (LONG)iso8859_to_utf8(name8, name, strlen(name))) == 0)
		{
			if ((path = BulidPath(fl ? fl->fl_Path : NULL, name8, error)))
			{
				FreeVecPooled(global->g_Pool, name8);
				return path;
			} 
		} else
			*error = -(*error);

		FreeVecPooled(global->g_Pool, name8);
	} else
		*error = ERROR_NO_FREE_STORE;

	return NULL;
}

