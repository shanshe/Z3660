/* JTAG low level functions and base class for cables

Copyright (C) 2004 Andrew Rogers
Additions (C) 2005-2011  Uwe Bonnes
                         bon@elektron.ikp.physik.tu-darmstadt.de

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Changes:
Sandro Amato [sdroamt@netscape.net] 26 Jun 2006 [applied 13 Jul 2006]:
   Added a 'dotted' progress bar
Dmitry Teytelman [dimtey@gmail.com] 14 Jun 2006 [applied 13 Aug 2006]:
    Code cleanup for clean -Wall compile.
    Extensive changes to support FT2232 driver.
    Moved progress bar to ioparport.cpp and ioftdi.cpp.
 */



#include "iobase.h"
#include "utilities.h"

//#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define false 0
#define true 1
void setVerbose_io(IOBase *io,bool v) { io->verbose = v; }
void settype(int subtype) {}
IOBase iobase;
void IOBase_init(IOBase *io)
{
   io->verbose = false;
   memset( io->ones,0xff,CHUNK_SIZE);
   memset(io->zeros,   0,CHUNK_SIZE);
   memset(io->tms_buf, 0,CHUNK_SIZE);
   io->tms_len = 0;
}


void flush_tms(IOBase *io)
{
   if (io->tms_len)
      tx_tms(io->tms_buf, io->tms_len);
   memset(io->tms_buf, 0,CHUNK_SIZE);
   io->tms_len = 0;
}

void set_tms(IOBase *io,bool val)
{
   if( io->tms_len + 1 > CHUNK_SIZE*8)
      flush_tms(io);
   if(val)
      io->tms_buf[io->tms_len/8] |= (1 <<(io->tms_len &0x7));
   io->tms_len++;
}

void shiftTDITDO(IOBase *io, unsigned char *tdi, unsigned char *tdo, int length, bool last)
{
   if(length==0) return;
   flush_tms(io);
   txrx_block(tdi, tdo, length,last);
   return;
}

void shiftTDI(IOBase *io, unsigned char *tdi, int length, bool last)
{
   shiftTDITDO(io,tdi, NULL, length,last);
}

// TDI gets a load of zeros, we just record TDO.
void shiftTDO(IOBase *io,unsigned char *tdo, int length, bool last)
{
   shiftTDITDO(io,NULL, tdo, length,last);
}

// TDI gets a load of zeros or ones, and we ignore TDO
void shift(IOBase *io,bool tdi, int length, bool last)
{
   int len = length;
   unsigned char *block = (tdi)?io->ones:io->zeros;
   flush_tms(io);
   while (len > CHUNK_SIZE*8)
   {
      txrx_block(block, NULL, CHUNK_SIZE*8, false);
      len -= (CHUNK_SIZE*8);
   }
   shiftTDITDO(io,block, NULL, len, last);
}

void Usleep(IOBase *io,unsigned int usec)
{
   flush_tms(io);
   usleep(usec);
}

