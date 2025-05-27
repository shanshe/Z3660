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

#ifndef _INCLUDE_H
#define _INCLUDE_H 1

#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/scsidisk.h>
#include <devices/timer.h>
#include <devices/trackdisk.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/filehandler.h>
#include <dos/notify.h>
#include <exec/errors.h>
#include <exec/execbase.h>
#include <exec/initializers.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/types.h>
#ifdef __MORPHOS__
#include <hardware/byteswap.h>
#endif
#include <hardware/intbits.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#include <clib/debug_protos.h>
#include <clib/alib_protos.h>

#define __NOLIBBASE__

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/utility.h>

#include <string.h>
#ifndef __MORPHOS__
#include <stdarg.h> /* va_args */
#endif

#ifndef __MORPHOS__
#include "fs_os3.h"
#endif

#include "ef_exfat.h"

#include "fs_rev.h"
#include "fs_define.h"
#include "fs_structures.h"

#include "fs_disk.h"
#include "fs_lock.h"
#include "fs_main.h"
#include "fs_notify.h"
#include "fs_ops.h"
#include "fs_packet.h"
#include "fs_scsi.h"
#include "fs_string.h"
#include "fs_support.h"
#include "fs_timer.h"

#ifdef __NOLIBBASE__
extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;
extern UTILITYBASE *UtilityBase;
#endif

#endif

