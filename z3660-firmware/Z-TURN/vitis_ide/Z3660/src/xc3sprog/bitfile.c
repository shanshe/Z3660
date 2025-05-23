/* Xilinx .bit file parser

Copyright (C) 2004 Andrew Rogers

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
*/


#include "bitfile.h"
#if 0
//#include "io_exception.h"

#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "bitrev.h"
#include <stdlib.h>

#define false 0
#define true 1

void BitFile_init(BitFile *bf)
{
   bf->length=0;
   bf->buffer=0;
   bf->Error=false;
   bf->logfile=stderr;
   bf->offset=0;
   bf->rlength=0;
}

int readBitfile(FILE *fp)
{
    { // Skip the header
      char hdr[13];
      fread(hdr, 1, 13, fp); // 13 byte header
    }

    char         key;
    char *field;
    char  dummy[100];

    while(!feof(fp)) {
      fread(&key, 1, 1, fp);
      switch(key) {
      case 'a': field = &ncdFilename; break;
      case 'b': field = &partName;    break;
      case 'c': field = &date;        break;
      case 'd': field = &dtime;        break;
      case 'e':
	processData(fp);
	return 0;
      default:
	fprintf(stderr, "Ignoring unknown field '%c'\n", key);
	field = &dummy;
      }
      readField(*field, fp);
    }
    throw  io_exception("Unexpected end of file");
  }
  catch(...) {
    fprintf(stderr, "Unknown error\n");
    return 2;
  }
  if(!length)
    return 3;
  return 0;
}

// Read in file
int readFile(FILE *fp, FILE_STYLE in_style)
{
   if(!fp)
      return 1;
   return 1;
}

void processData(BitFile *bf,FILE *fp)
{
   byte t[4];
   fread(t,1,4,fp);
   bf->length=(t[0]<<24)+(t[1]<<16)+(t[2]<<8)+t[3];
   if(bf->buffer)
   {
      free(bf->buffer);
      bf->buffer=NULL;
   }
   bf->buffer=malloc(bf->length);
   for(unsigned int i=0; i<bf->length&&!feof(fp); i++){
      byte b;
      fread(&b,1,1,fp);
      bf->buffer[i]=bitRevTable[b]; // Reverse the bit order.
   }
   if(feof(fp))  printf("Unexpected end of file");

   fread(t,1,1,fp);
   if(!feof(fp))  error("Ignoring extra data at end of file");
}

void append(BitFile *bf,uint32_t val, unsigned cnt) {
   size_t const  nlen = bf->length + 4*cnt;
   byte  *const  nbuf = malloc(nlen);

   // copy old part
   for(size_t i = 0; i < bf->length; i++)  nbuf[i] = bf->buffer[i];
   free(bf->buffer);
   bf->buffer = nbuf;

   // append new contents
   for(size_t  i = bf->length; i < nlen; i += 4) {
      bf->buffer[i+0] = bitRevTable[0xFF & (val >> 24)];
      bf->buffer[i+1] = bitRevTable[0xFF & (val >> 16)];
      bf->buffer[i+2] = bitRevTable[0xFF & (val >>  8)];
      bf->buffer[i+3] = bitRevTable[0xFF & (val >>  0)];
   }
   bf->length = nlen;

}

void append_file(BitFile *bf,char const *fname) {
   FILE *const  fp=fopen(fname,"rb");
   if(!fp)  printf("Cannot open file %s\n", fname);

   {
      struct stat  stats;
      stat(fname, &stats);

      size_t nlen = bf->length + stats.st_size;
      byte  *const  nbuf = malloc(nlen);

      // copy old part
      for(size_t i = 0; i < bf->length; i++)  nbuf[i] = bf->buffer[i];
      free(bf->buffer);
      bf->buffer = nbuf;

      // append new contents
      for(size_t i = bf->length; i < nlen; i++) {
         if(feof(fp)) printf("Unexpected end of file\n");

         byte  b;
         fread(&b, 1, 1, fp);
         bf->buffer[i] = bitRevTable[b]; // Reverse the bit order.
      }
      bf->length = nlen;

      fclose(fp);
   }
}

void setLength(BitFile *bf,unsigned int size)
{
  length = size/8 + ((size%8)?1:0);
  if(buffer) delete [] buffer;
  buffer=new byte[length];
  memset(buffer, 0xff, length);
}

void setNCDFields(const char * partname)
{
  char outstr[200];
  time_t t;
  struct tm *tmp;

  if (!ncdFilename.size())
    {
      ncdFilename.assign("XC3SPROG");
      ncdFilename.push_back(0);
    }

  if (!partName.size())
    {
      partName.assign(partname);
      partName.push_back(0);
    }

  t = time(NULL);
  tmp = localtime(&t);
  if (tmp != NULL)
    {
      if (!dtime.size())
	{
	  if (strftime(outstr, sizeof(outstr), "%Y/%m/%d", tmp))
	    {
	      date.assign(outstr);
	      date.push_back(0);
	    }
	}
      if (!dtime.size())
	{
	  if (strftime(outstr, sizeof(outstr), "%T", tmp))
	    {
	      dtime.assign(outstr);
	      dtime.push_back(0);
	    }
	}
    }
}

unsigned char checksum(char *buf)
{
  int i;
  unsigned char chksum = 0;
  unsigned char val;
  for (i = 0; buf[i]; i = i +2)
    {
      if (sscanf(buf +i, "%2hhX", &val) == 1)
	chksum += val;
      else break;
    }
  return (chksum ^ 0xff) + 1;
}

uint32_t saveAs(FILE_STYLE style, const char  *device,
			      FILE *fp)
{
  if(length<=0)return length;
  unsigned int clip;
  unsigned int i;

  setNCDFields(device);
  /* Don't store 0xff bytes from the end of the flash */
  for(clip=length-1; (buffer[clip] == 0xff) && clip>0; clip--){};
  clip++; /* clip is corrected length, not index */
  if (rlength) /* Don't clip is explicit length is requested */
      clip = rlength;
  switch (style)
    {
    case STYLE_BIT:
    case STYLE_BIN:
    case STYLE_BPI:
      if(style == STYLE_BIT)
	{
	  uint8_t buffer[256] = {0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0,
			      0x0f, 0xf0, 0x00, 0x00, 0x01};
	  int len;

	  fwrite(buffer, 1, 13, fp);

	  buffer[0] = 'a';
	  len = ncdFilename.size();
	  buffer[1] = len >>8;
	  buffer[2] = len & 0xff;
	  fwrite(buffer, 3, 1, fp);
	  fwrite(ncdFilename.c_str(), len, 1, fp);

	  buffer[0] = 'b';
	  len = partName.size();
	  buffer[1] = len >>8;
	  buffer[2] = len & 0xff;
	  fwrite(buffer, 3, 1, fp);
	  fwrite(partName.c_str(), len, 1, fp);

	  buffer[0] = 'c';
	  len = date.size();
	  buffer[1] = len >>8;
	  buffer[2] = len & 0xff;
	  fwrite(buffer, 3, 1, fp);
	  fwrite(date.c_str(), len, 1, fp);

	  buffer[0] = 'd';
	  len = dtime.size();
	  buffer[1] = len >>8;
	  buffer[2] = len & 0xff;
	  fwrite(buffer, 3, 1, fp);
	  fwrite(dtime.c_str(), len, 1, fp);

	  buffer[0] = 'e';
	  buffer[1] = clip >>24 & 0xff;
	  buffer[2] = clip >>16 & 0xff;
	  buffer[3] = clip >> 8 & 0xff;
	  buffer[4] = clip & 0xff;
	  fwrite(buffer, 5, 1, fp);
	}
      for(i=0; i<clip; i++)
	{
	  byte b;
	  if (style != STYLE_BPI)
		  b = bitRevTable[buffer[i]]; // Reverse bit order
	  else
		  b = buffer[i];
	  fwrite(&b,1,1,fp);
	}
      break;
    case STYLE_HEX:
      for(i=0; i<clip; i++)
	{
	  byte b=bitRevTable[buffer[i]]; // Reverse bit order
	  if ( i%16 ==  0)
	    fprintf(fp,"%7d:  ", i);
	  fprintf(fp,"%02x ", b);
	  if ( i%16 ==  7)
	    fprintf(fp," ");
	  if ( i%16 == 15)
	    fprintf(fp,"\n");
	}
      break;
    case STYLE_HEX_RAW:
      for(i=0; i<clip; i++)
	{
	  byte b=bitRevTable[buffer[i]]; // Reverse bit order
	  fprintf(fp,"%02x", b);
	  if ( i%4 == 3)
	    fprintf(fp,"\n");
	}
      if ( i%4 != 3) /* Terminate semil full lines */
          fprintf(fp,"\n");
      break;
    case STYLE_MCS:
    case STYLE_IHEX:
      {
        unsigned int base = (unsigned int)-1;
        char buf[1024];
        int len = 0;
        for(i=0; i<clip; i++)
          {
            byte b = buffer[i];
            if (style == STYLE_MCS)
              b = bitRevTable[b];
            if (base != i>>16)
              {
                base = i >> 16;
                fprintf(fp,":");
		sprintf(buf, "02000004%04X%c", base, 0);
                fprintf(fp, "%s%02X\r\n", buf, checksum(buf));
              }
	    if ((i & 0xf) == 0)
	      {
                fprintf(fp,":");
		sprintf(buf, "%02X", (i & 0xf) +1 );
		if (clip -i < 0xf)
		  len = sprintf(buf, "%02X%04X00", clip-i, i & 0xffff);
		else
		  len = sprintf(buf, "10%04X00", i & 0xffff);
	      }
	    len += sprintf(buf+len, "%02X", b);
	    if (((i & 0xf) == 0xf) || (i == clip -1))
	      {
		buf[len] = 0;
		len = fprintf(fp, "%s%02X\r\n", buf, checksum(buf));
	      }
	  }
	fprintf(fp,":");
	sprintf(buf, "00000001");
	fprintf(fp, "%s%02X\r\n", buf, checksum(buf));
	break;
      }
     default:
      fprintf(stderr, "Style not yet implemted\n");
    }

  return clip;
}

void error(const string &str)
{
  errorStr=str;
  Error=true;
  fprintf(logfile,"%s\n",str.c_str());
}

void readField(string &field, FILE *fp)
{
  byte t[2];
  fread(t,1,2,fp);
  unsigned short len=(t[0]<<8)+t[1];
  for(int i=0; i<len; i++){
    byte b;
    fread(&b,1,1,fp);
    field+=(char)b;
  }
}

void set_bit(unsigned int idx, int blow)
{
  unsigned int bval, bit;
  bval = idx / 8;
  if(bval >= length)
    {
      fprintf(stderr,"set_bit invalid index %d length %d\n", idx, length*8);
      throw  io_exception(std::string("bit_set_fuse"));
    }
  bit  = idx % 8;

  if (blow)
    buffer[bval] |=  (1 << bit);
  else
    buffer[bval] &= ~(1 << bit);
}

int get_bit(unsigned int idx)
{
     unsigned int bval, bit;
      bval = idx / 8;
      /* Because of the clipping
       we assume bit 1 if idx is beyond the file length. */
      if(bval >= length) return 1;
      bit  = idx % 8;
      return (buffer[bval] & (1 << bit))? 1 : 0;
}
#endif


const char * styleToString(FILE_STYLE style)
{
  switch (style)
    {
      case STYLE_BIT: return "BIT";
      case STYLE_BIN: return "BIN";
      case STYLE_HEX: return "HEX";
      case STYLE_HEX_RAW: return "HEXRAW";
      case STYLE_MCS: return "MCS";
      case STYLE_IHEX: return "IHEX";
      case STYLE_JEDEC: return "JEDEC";
      case STYLE_AUTO: return "AUTO";
      default: return 0;
    }
}
int styleFromString(const char *stylestr, FILE_STYLE *style)
{
    char * q = strchr((char*)stylestr,':');
    int len;

    if (q)
	len = q-stylestr;
    else
	len = strlen(stylestr);

    if (!strncmp(stylestr, "BIT", len))
	*style = STYLE_BIT;
    else if (!strncmp(stylestr, "BIN", len))
	*style = STYLE_BIN;
    else if (!strncmp(stylestr, "BPI", len))
	*style = STYLE_BPI;
    else if (!strncmp(stylestr, "HEX", len))
	*style = STYLE_HEX;
    else if (!strncmp(stylestr, "HEXRAW", len))
	*style = STYLE_HEX_RAW;
    else if (!strncmp(stylestr, "MCS", len))
	*style = STYLE_MCS;
    else if (!strncmp(stylestr, "IHEX", len))
	*style = STYLE_IHEX;
    else if (!strncmp(stylestr, "JEDEC", len))
	*style = STYLE_JEDEC;
    else if (!strncmp(stylestr, "AUTO", len))
	*style = STYLE_AUTO;
    else
	return 1;
    return 0;
}
