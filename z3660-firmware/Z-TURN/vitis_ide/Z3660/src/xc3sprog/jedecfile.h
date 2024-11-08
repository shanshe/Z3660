/* Jedec .jed file parser

Copyright (C) Uwe Bonnes 2009 bon@elektron.ikp.physik.tu-darmstadt.de

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
 * Using a slightly corrected version from IPAL libjedec
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 */

#ifndef JEDECFILE_H
#define JEDECFILE_H

#include <string.h>
#include <ff.h>

#define MAX_ITEM   8
#define MAX_SIZE 256
typedef unsigned char byte;

#define bool int

struct jedec_data {
      char device[MAX_SIZE];
      char version[MAX_SIZE];
      char date[MAX_SIZE];

      unsigned fuse_count;
      unsigned pin_count;
      unsigned vector_count;
      unsigned checksum;
      unsigned char fuse_default;

      unsigned char*fuse_list;
};
typedef struct jedec_data *jedec_data_t;
extern jedec_data_t jed;
#define JED_XC95X 0
#define JED_XC2C  1
#define JED_XC95  2

typedef struct
{
  struct jedec_data jed;
  bool Error;
  char errorStr[100];
  FILE *logfile;

}  JedecFile;
extern JedecFile jedecfile;

  int readFile_jedec(FIL *fp);
  inline unsigned int getLength_jedec(){return jedecfile.jed.fuse_count;}
  inline unsigned short getChecksum(){return jedecfile.jed.checksum;}
  char *getDevice();
  char *getVersion();
  char *getDate_jedec();
  unsigned short calcChecksum();
  void setLength(unsigned int fuse_count);
  int get_fuse(unsigned int idx);
  void set_fuse(unsigned int idx, int blow);

#endif //JEDECFILE_H
