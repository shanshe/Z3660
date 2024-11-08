/* Spartan3 JTAG programmer

Copyright (C) 2004 Andrew Rogers
              2005-2011 Uwe Bonnes bon@elektron.ikp.physik.tu-darmstadt.de

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
Dmitry Teytelman [dimtey@gmail.com] 14 Jun 2006 [applied 13 Aug 2006]:
    Code cleanup for clean -Wall compile.
    Added support for FT2232 driver.
    Verbose support added.
    Installable device database location.
 */

#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
extern char *optarg;
extern int optind, opterr, optopt;
int    getopt(int, char * const [], const char *);
#include <ctype.h>
#include <limits.h>
#include <ff.h>
#include "../defines.h"
void print_hdmi_ln(int xpos, char *message, int line_inc);

#include "ioparport.h"
#include "utilities.h"
#include "jtag.h"
#include "bitfile.h"
#include <errno.h>
#include "progalgxc95x.h"

#define false 0
#define true 1

#define MAXPOSITIONS    8

#define IDCODE_TO_FAMILY(id)        ((id>>21) & 0x7f)
#define IDCODE_TO_MANUFACTURER(id)  ((id>>1) & 0x3ff)

#define MANUFACTURER_ATMEL          0x01f
#define MANUFACTURER_XILINX         0x049

int do_exit = 0;
void ctrl_c(int sig)
{
   do_exit = 1;
}
#define bool int
extern Jtag jtag1;
int programXC95X(Jtag *jtag, IOBase *io, unsigned long id, char *arg,
      bool verbose, bool erase, char *device);

int idToIRLength(DeviceID idcode)
{
   // 0x09608093 8 0xfe XC95144XL
   if(idcode==0x09608093)
      return(8);
   else return(-1);
}
int idToIDCmd(DeviceID idcode)
{
   // 0x09608093 8 0xfe XC95144XL
   if(idcode==0x09608093)
      return(0xfe);
   else return(-1);
}
char *idToDescription(DeviceID idcode)
{
   // 0x09608093 8 0xfe XC95144XL
   if(idcode==0x09608093)
      return("XC95144XL");
   else return("Unknown");
}
/* Excercise the IR Chain for at least 10000 Times
   If we read a different pattern, print the pattern for for optical
   comparision and read for at least 100000 times more

   If we found no chain, simple rerun the chain detection

   This may result in an endless loop to facilitate debugging with a scope etc
 */
#if 0
void test_IRChain(Jtag *jtag, IOBase *io, int test_count)
{
   int num=getChain(jtag,io,true);
   int failed = 0;
   int len = 0;
   int i, j, k;
   unsigned char ir_in[256];
   unsigned char ir_out[256];
   unsigned char din[256];
   unsigned char dout[256];
   unsigned char dcmp[256];

   memset(ir_in, 0x00, 256);
   memset(ir_out, 0x00, 256);
   memset(din, 0xaa, 256);
   memset(dout, 0x00, 256);
   memset(dcmp, 0x00, 256);

   int run_irtest = 0;

   if(num == 0)
      /* the chain is not functional and we have no idea
       * what parts to look for
       */
   {
      printf( "Running getChain %d times\n", test_count);
      k=0;
      for(i=0; i<test_count; i++)
      {
         if (getChain(jtag,io,true)> 0)
         {
            if(k%1000 == 1)
            {
               printf(".");
            }
            k++;
         }
      }
      return;
   }
   if(num >8)
      printf( "Found %d devices\n", num);

   /* Read the IDCODE via the IDCODE command */

   for(i=0; i<num; i++)
   {
      setTapState(jtag,io,TEST_LOGIC_RESET,0);
      selectDevice(jtag,i);
      DeviceID id = getDeviceID(jtag,i);
      int irlen = idToIRLength(id);
      if (irlen == 0)
      {
         run_irtest++;
         break;
      }
      uint32_t idcmd = idToIDCmd(id);
      for (j = 0; j < irlen; j = j+8)
         ir_in[j>>3] = (idcmd>>j) & 0xff;
      shiftIR(jtag,io,ir_in, ir_out);
      cycleTCK(jtag,io,1,1);
      shiftDR(jtag,io,NULL, &dout[i*4], 32, 0, true);
      if ((byteArrayToLong(dout+i*4)&0x0FFFFFFF) != id)
      {
         printf( "IDCODE mismatch pos %d Read 0x%08lx vs 0x%08lx\n",
               i, byteArrayToLong(dout+i*4), (unsigned long)id);
         run_irtest++;
      }
   }

   if(run_irtest)
   { /* ID Code did fail, to simple shift the IR chain */
      printf( "Running IR_TEST %d  times\n", test_count);
      /* exercise the chain */
      for(i=0; i<num; i++)
         len += idToIRLength(getDeviceID(jtag,i));
      printf( "IR len = %d\n", len);
      setTapState(jtag,io,TEST_LOGIC_RESET,0);
      setTapState(jtag,io,SHIFT_IR,0);
      shiftTDITDO(io,din,dout,len,true);
      nextTapState(jtag,io,true);
      printf( "0x");
      for(i=0; i <len>>3;  i++)
         printf( "%02x", dout[i]);
      printf( " binary ");
      k=len-1;
      for(i = 0; i<num; i++)
      {
         int irlen = idToIRLength(getDeviceID(jtag,i));
         for(j=0; j<irlen; j++)
         {
            printf( "%c",
                  (((dout[k>>3]>>(k&0x7)) &0x01) == 0x01)?'1':'0');
            k--;
         }
         printf( " ");
      }
      for(i=0; i<test_count&& !do_exit; i++)
      {
         setTapState(jtag,io,SELECT_DR_SCAN,0);
         setTapState(jtag,io,SHIFT_IR,0);
         shiftTDITDO(io,din,dcmp,len,true);
         nextTapState(jtag,io,true);
         if (memcmp(dout, dcmp, (len+1)>>3) !=0)
         {
            printf( "mismatch run %d\n", i);
            failed++;
            for(j=0; j <len>>3;  j++)
               printf( "%02x", dcmp[j]);
            printf( " ");         k=len-1;
            for(i = 0; i<num; i++)
            {
               int irlen = idToIRLength(getDeviceID(jtag,i));
               for(j=0; j<irlen; j++)
               {
                  printf( "%c",
                        (((dcmp[k>>3]>>(k&0x7)) &0x01) == 0x01)?'1':'0');
                  k--;
               }
               printf( " ");
            }
         }
         if(i%1000 == 999)
         {
            printf( ".");
         }
      }
      printf( "\n");
   }
   else
   {
      printf( "Reading ID_CODE %d  times\n", test_count);
      memset(ir_in, 0, 256);
      /* exercise the chain */
      for(i=num-1; i>=0 && !do_exit; i--)
      {
         int irlen = idToIRLength(getDeviceID(jtag,i));
         uint32_t idcmd = idToIDCmd(getDeviceID(jtag,i));
         for(j=0; j<irlen; j++)
         {
            char l = (idcmd & (1<<j))?1:0;
            ir_in[len>>3] |= ((l)?(1<<(len & 0x7)):0);
            len++;
            longToByteArray(getDeviceID(jtag,i), dcmp+((num -1 -i)*4));
         }
      }
      printf( "Sending %d bits IDCODE Commands: 0x", len);
      for(i=0; i <len;  i+=8)
         printf( "%02x", ir_in[i>>3]);
      printf( "\n");
      printf( "Expecting %d IDCODES  :", num);
      for(i=num-1; i >= 0;  i--)
         printf( " 0x%08lx", (unsigned long) getDeviceID(jtag,i));

      tapTestLogicReset(jtag,io);
      for(i=0; i<test_count&& !do_exit; i++)
      {
         setTapState(jtag,io,SHIFT_IR,0);
         shiftTDI(io,ir_in,len,true);
         nextTapState(jtag,io,true);
         setTapState(jtag,io,SHIFT_DR,0);
         shiftTDITDO(io,NULL,dout,num*32,true);
         nextTapState(jtag,io,true);
         setTapState(jtag,io,TEST_LOGIC_RESET,0);
         dout[3]&=0x0F; // weird hack
         if(memcmp(dout, dcmp, num*4) !=0)
         {
            printf( "\nMismatch run %8d:", i+1);
            failed++;
            for(j=num-1; j>=0; j--)
               printf(" 0x%08lx", byteArrayToLong(dout+j*4));
         }
         if(i%1000 == 999)
         {
            printf( ".");
         }
      }
      printf( "\n");
   }
   if (failed)
      printf( "Failed %8d/%8d\n", failed, i);

}
#endif
extern IOParport iopp;
int init_chain(Jtag *jtag,IOBase *io,int *num_out)
{
   unsigned int use_freq = 0;
   iopp.mode=IOPP_MODE_I2CSW;
   int num=0;
   do{
      Init(use_freq,iopp.mode);
      num = getChain(jtag,io,true);
      if (num == 0)
      {
         iopp.mode++;
         if(iopp.mode==IOPP_MODE_MAX)
         {
            char message[100];
            sprintf(message,"No JTAG Chain found");
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            return 0;
         }
      }
   }while(num==0);
   *num_out=num;
   // Synchronise database with chain of devices.
   for (int i=0; i<num; i++){
      unsigned long id = getDeviceID(jtag,i);
      int length = idToIRLength(id);
      if (length > 0)
         setDeviceIRLength(jtag,i,length);
      else
      {
         printf("Cannot find device having IDCODE=%07lx Revision %c\n",
               id & 0x0fffffff,  (int)(id  >>28) + 'A');
         return 0;
      }
   }
   return num;
}

static int last_pos = -1;

unsigned long get_id(Jtag *jtag,IOBase *io, int chainpos, int num)
{
   bool verbose = getVerbose_jtag(jtag);
//   int num = getChain(jtag,io,true);
   if (selectDevice(jtag,chainpos)<0)
   {
      printf( "Invalid chain position %d, must be >= 0 and < %d\n",
            chainpos, num);
      return 0;
   }
   unsigned long id = getDeviceID(jtag,chainpos);
   if (verbose && (last_pos != chainpos))
   {
      printf( "JTAG chainpos: %d Device IDCODE = 0x%08lx\tDesc: %s\n",
            chainpos, id, idToDescription(id));
      last_pos = chainpos;
   }
   return id;
}

FIL jedecfile_fp;

/* Parse a filename in the form
 *           aaaa.bb:action:0x10000|section:0x10000:rawhex:0x1000
 * for name, action, offset|area, style, length
 *
 * return the Open File
 *
 * possible action
 * w: erase whole area, write and verify
 * W: Write with auto-sector erase and verify
 * v: Verify device against filename
 * r: Read from device and write to file, don't overwrite exixting file
 * R: Read from device and write to file, overwrite exixting file
 *
 * possible sections:
 * f: Flash
 * a:
 *
 */
int getFile_and_Attribute_from_name(
      char *name1, char * action, char * section,
      unsigned int *offset, FILE_STYLE *style, unsigned int *length)
{
   char name[100]="z3660.jed:w:0:JEDEC";
   char filename[256];
   char *p = name, *q;
   int len;
   char localaction = 'w';
   char localsection = 'a';
   unsigned int localoffset = 0;
   FILE_STYLE localstyle=STYLE_BIT;
   unsigned int locallength = 0;

   if(!p)
      return 0;
   else
   {
      char message[100];
      sprintf(message,"command %s",name);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      q = strchr(p,':');
#if defined(__WIN32__)
      if (p[1]  == ':') {
         /* Assume we have a DOS path.
          * Look for next colon or end-of-string.
          */
         q = strchr(p + 2, ':');
      }
#endif
      if (q)
         len = q-p;
      else
         len = strlen(p);
      if (len>0)
      {
         char temp[256-4];
         int num = (len>232)?232:len;
         strncpy(temp, p, num);
         temp[num] = 0;
         sprintf(filename,"1:/%s",temp);
      }
      else
         return 0;
      p = q;
      if(p)
         p ++;
   }
   /* Action*/
   if(p)
   {
      q = strchr(p,':');

      if (q)
         len = q-p;
      else
         len = strlen(p);
      if (len == 1)
         localaction = *p;
      else
         localaction = 'w';
      if (action)
      {
         if(localaction == 'W')
            *action =  localaction;
         else
            *action =  tolower(localaction);
      }
      p = q;
      if(p)
         p ++;
   }
   /*Offset/Area*/
   if(p)
   {
      q = strchr(p,':');
      if (q)
         len = q-p;
      else
         len = strlen(p);
      if (!isdigit(*p))
      {
         localsection = *p;
         if (section)
            *section = localsection;
         p++;
      }
      localoffset = strtol(p, NULL, 0);
      if (offset)
         *offset = localoffset;
      p = q;
      if(p)
         p ++;
   }
   /*Style*/
   if(p )
   {
      int res = 0;
      q = strchr(p,':');

      if (q)
         len = q-p;
      else
         len = strlen(p);
      if (len)
         res = styleFromString(p, &localstyle);
      if(res)
      {
         printf( "\nUnknown format \"%*s\"\n", len, p);
         return 0;
      }
      if (len && style)
         *style = localstyle;
      p = q;
      if(p)
         p ++;
   }
   /*Length*/

   if(p)
   {
      locallength = strtol(p, NULL, 0);
      p = strchr(p,':');
      if (length)
         *length = locallength;
      if(p)
         p ++;
   }
/*
   if  (tolower(localaction) == 'r')
   {
      if (!(strcmp(filename,"stdout")))
         ret= stdout;
      else
      {
           int res;
           struct stat  stats;
           res = stat(filename, &stats);
           if ((res == 0) && (localaction == 'r') && stats.st_size !=0)
           {
              printf( "File %s already exists. Aborting\n", filename);
              return NULL;
           }
           ret=fopen(filename,"wb");
           if(!ret)
              printf( "Unable to open File %s. Aborting\n", filename);

      }
   }
   else*/
   {
#if 0
      if (!(strcasecmp(filename,"stdin")))
         ret = stdin;
      else
#endif
      {
         FRESULT res=f_open(&jedecfile_fp,filename, FA_READ);
         if(res!=FR_OK)
         {
            printf( "Can't open datafile %s: %s\n", filename,
                  strerror(errno));
            return 0;
         }
      }
   }
   return(1);
}

int flow_usercode(char *userid);
unsigned int family = 0;
unsigned int manufacturer = 0;
unsigned long id = 0;
int isnumber(char p)
{
   if(p>='0' && p<='9')
      return 1;
   return 0;
}
int init_xc3sprog(void)
{
   char message[100];
   bool    verbose   = false;
   unsigned int jtag_freq= 0;
   int      chainpos     = 0;
//   int      nchainpos    = 1;
   char const *serial  = 0;

   int res = getIO( &iobase, serial, verbose, jtag_freq);
   if (res) /* some error happend*/
   {
      return 1;
   }

   //  Jtag jtag = Jtag(io.get());
   Jtag_init(&jtag,&iobase);
   setVerbose_jtag(&jtag,verbose);
   int num=-1;
   if (init_chain(&jtag, &iobase,&num))
      id = get_id (&jtag, &iobase, chainpos,num);
   else
      id = 0;
   if (id == 0)
      return 2;

   family = IDCODE_TO_FAMILY(id);
   manufacturer = IDCODE_TO_MANUFACTURER(id);

   if (manufacturer != MANUFACTURER_XILINX || family != 0x4b)
   {
      sprintf(message,"No JTAG found");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return 3;
   }

   int size = (id & 0x000ff000)>>13;
   ProgAlgXC95X_init(&jtag, &iobase, size);

   char userid[10];
   flow_enable();
   flow_usercode(userid);
   flow_disable();
   if(!isnumber(userid[2]) || !isnumber(userid[3]))
   {
      sprintf(message,"Read a Bad CPLD firmware version. CPLD erased? JTAG error?");
   }
   else
   {
      if(userid[1]=='0')
         sprintf(message,"CPLD firmware 1.03 BETA 8 or lower");
      if(userid[1]=='B')
         sprintf(message,"CPLD firmware 1.0%c BETA %c%c",userid[0],userid[2],userid[3]);
      else if(userid[1]=='A')
         sprintf(message,"CPLD firmware 1.0%c ALFA %c%c",userid[0],userid[2],userid[3]);
      else
         sprintf(message,"CPLD firmware 1.0%c rev %c%c",userid[0],userid[2],userid[3]);
   }
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   return 0;
}
int main_xc3sprog(void)
{
   char arg[100]="z3660.jed:w:0:JEDEC";
//   char args[100]="z3660.jed:v:0:JEDEC";
   bool    verbose   = false;
   bool     erase    = false;
   int      nchainpos    = 1;

   char message[100];
   sprintf(message," ");
   print_hdmi_ln(0,message,1);
   sprintf(message,"CPLD programming based on XC3SPROG (c) 2004-2011 xc3sprog project $Rev: 795");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);

//   if(1 < 1 && !reconfigure && !erase && !rUsercode) detectchain = true;

//   if(chaintest && !spiflash)
//      test_IRChain(&jtag, &iobase, test_count);

//   if (detectchain && !spiflash)
//   {
//      detect_chain(&jtag, &iobase);
//      return 0;
//   }



   if (nchainpos != 1 &&
         (manufacturer != MANUFACTURER_XILINX))
   {
      sprintf(message,"Multiple positions only supported in case of XCF");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return 1;
   }

   if (manufacturer != MANUFACTURER_XILINX || family != 0x4b) /* XC95XL XC95XV*/
   {
      sprintf(message,"No JTAG found");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return 1;
   }
   return programXC95X(&jtag, &iobase, id, arg, verbose, erase,
         idToDescription(id));
}


int programXC95X(Jtag *jtag, IOBase *io, unsigned long id, char *arg,
      bool verbose, bool erase, char *device)
{
   int ret = 0;
   int size = (id & 0x000ff000)>>13;
   ProgAlgXC95X_init(jtag, io, size);
   if (erase)
   {
      ret=alg_erase();
      if(ret==0) return 0;

   }

   unsigned int jedecfile_offset = 0;
   unsigned int jedecfile_rlength = 0;
   char action = 'w';
   JedecFile  jedecfile;
   FILE_STYLE  jedecfile_style= STYLE_JEDEC;

   static FATFS fatfs;
   TCHAR *Path = DEFAULT_ROOT;

   f_mount(&fatfs, Path, 1); // 1 mount immediately

   int ret1= getFile_and_Attribute_from_name
         (arg, &action, NULL, &jedecfile_offset,
               &jedecfile_style, &jedecfile_rlength);
   if(ret1==0)
   {
      return 0;
   }
   if (jedecfile_offset != 0)
   {
      printf( "Offset %d not supported, Using 0\n",
            jedecfile_offset);
      jedecfile_offset = 0;
   }
   if (jedecfile_rlength != 0)
   {
      printf( "Readlength %d not supported, Using 0\n",
            jedecfile_rlength);
      jedecfile_rlength = 0;
   }
   if(jedecfile_style != STYLE_JEDEC)
   {
      printf( "Style %s not supported, skipping\n",
            styleToString(jedecfile_style));
   }
   if (action == 'v' || tolower(action) == 'w')
   {
      readFile_jedec(&jedecfile_fp);
      uint16_t checksum=calcChecksum();
      char message[100];
      sprintf(message,"calculated checksum = %04X, jed checksum = %04X",checksum,jed->checksum);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      if (action == 'w')
      {
         if (!erase)
         {
            ret = alg_erase();
            if(ret==0) return 0;
         }
      }
      if(tolower(action) == 'w')
         alg_array_program(&jedecfile);
      ret = alg_array_verify(&jedecfile);
   }
   if(ret)
   {
      return ret;
   }
   return 0;
}

