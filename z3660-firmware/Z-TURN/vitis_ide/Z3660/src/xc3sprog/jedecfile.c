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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "jedecfile.h"
//#include "io_exception.h"

static unsigned char*allocate_fusemap(unsigned size)
{
   unsigned char*ptr = (unsigned char*) calloc(size/8 + ((size%8)?1:0), 1);
   return ptr;
}

int jedec_get_fuse(jedec_data_t jed, unsigned idx)
{
   unsigned int bval, bit;
   if(idx >= jed->fuse_count)
      printf("jedec_get_fuse exception\n"); // exception
//   printf("jedec_get_fuse() idx %d jed->fuse_list %08X\n",idx,jed->fuse_list);

   bval = idx / 8;
   bit  = idx % 8;
   return (jed->fuse_list[bval] & (1 << bit))? 1 : 0;
}

void jedec_set_fuse(jedec_data_t jed, unsigned idx, int blow)
{
   unsigned int bval, bit;
   if(idx >= jed->fuse_count)
      printf("jedec_set_fuse exception\n"); // exception

   bval = idx / 8;
   bit  = idx % 8;

   if (blow)
      jed->fuse_list[bval] |=  (1 << bit);
   else
      jed->fuse_list[bval] &= ~(1 << bit);
}

struct state_mach {
   jedec_data_t jed;

   void (*state)(int ch, struct state_mach*m);

   unsigned int checksum;
   union {
      struct {
         unsigned cur_fuse;
      } l;
   } m;
};

static void m_startup(int ch, struct state_mach*m);
static void m_header(int ch, struct state_mach*m);
static void m_base(int ch, struct state_mach*m);
static void m_C(int ch, struct state_mach*m);
static void m_L(int ch, struct state_mach*m);
static void m_Lfuse(int ch, struct state_mach*m);
static void m_Q(int ch, struct state_mach*m);
static void m_QF(int ch, struct state_mach*m);
static void m_QP(int ch, struct state_mach*m);
static void m_skip(int ch, struct state_mach*m);
static void m_N(int ch, struct state_mach*m);

int m_N_item;
int m_N_pos;
int m_H_pos = 0;
char m_H_string[MAX_SIZE];
char m_N_strings[MAX_ITEM][MAX_SIZE];

static void m_startup(int ch, struct state_mach*m)
{
   switch (ch)
   {
   case '\002':
      m->state = m_base;
      break;

   case 'D':
      m->state = m_header;
      break;

   default:
      break;
   }
}

static void m_header(int ch, struct state_mach*m)
{
   switch (ch)
   {
   case '\n':
   case '\r':
      if (m_H_pos)
      {
         char * ptr = strchr( m_H_string, ':');
         if (ptr)
            strncpy(m->jed->date, ptr, MAX_SIZE);
      }
      m->state = m_startup;
      break;
   default:
      m_H_string[m_H_pos] = ch;
      m_H_pos++;
   }
}

static void m_base(int ch, struct state_mach*m)
{
   if (isspace(ch))
      return;

   switch (ch) {
   case 'L':
      m->state = m_L;
      m->m.l.cur_fuse = 0;
      break;
   case 'Q':
      m->state = m_Q;
      break;
   case 'C':
      m->state = m_C;
      m->jed->checksum = 0;
      break;
   case 'N':
      m->state = m_N;
      m_N_item = -1;
      break;
   default:
      m->state = m_skip;
      break;
   }
}

static void m_C(int ch, struct state_mach*m)
{
   if (isspace(ch))
      return;

   if (ch == '*') {
      m->state = m_base;
      return;
   }

   if(ch >='0' && ch <='9')
   {
      m->jed->checksum <<=4;
      m->jed->checksum += ch - '0';
      return;
   }
   ch &= 0x5f;
   if(ch >='A' && ch <= 'F')
   {
      m->jed->checksum <<=4;
      m->jed->checksum += ch - '7';
      return;
   }

   printf( "m_C: Dangling '%c' 0x%02x\n", ch, ch);
   printf("m_C exceptionn"); // exception
}
static void m_L(int ch, struct state_mach*m)
{
   if (isdigit(ch)) {
      m->m.l.cur_fuse *= 10;
      m->m.l.cur_fuse += ch - '0';
      return;
   }

   if (isspace(ch)) {
      m->state = m_Lfuse;
      return;
   }

   if (ch == '*') {
      m->state = m_base;
      return;
   }

   printf( "m_L: Dangling '%c' 0x%02x\n", ch, ch);
   m->state = 0;
}

static void m_Lfuse(int ch, struct state_mach*m)
{
   switch (ch) {

   case '*':
      m->state = m_base;
      break;

   case '0':
      jedec_set_fuse(m->jed, m->m.l.cur_fuse, 0);
      m->m.l.cur_fuse += 1;
      break;

   case '1':
      jedec_set_fuse(m->jed, m->m.l.cur_fuse, 1);
      m->m.l.cur_fuse += 1;
      break;

   case ' ':
   case '\n':
   case '\r':
      break;

   default:
      printf( "m_LFuse: Dangling '%c' 0x%02x\n", ch, ch);
      m->state = 0;
      break;
   }
}

#if defined(__unix__) || defined(__MACH__)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

static void m_N(int ch, struct state_mach*m)
{
   switch (ch) {

   case '*':
      if ((strcmp(m_N_strings[0], "DEVICE")) == 0)
      {
         m_N_strings[m_N_item][m_N_pos] = 0;
         strncpy(m->jed->device, m_N_strings[1], MAX_SIZE);
      }
      if ((strcmp(m_N_strings[0], "VERSION")) == 0)
      {
         m_N_strings[m_N_item][m_N_pos] = 0;
         strncpy(m->jed->version, m_N_strings[1], MAX_SIZE);
      }
      m->state = m_base;
      m_N_item= -1;
      break;
   case ' ':
      if(m_N_item >=0)
         m_N_strings[m_N_item][m_N_pos] = 0;
      if (m_N_item < MAX_ITEM)
      {
         /* Don't stumble on too many items like in ISE XC2C Jedecfiles */
         m_N_item++;
      }
      m_N_pos = 0;
   case '\n':
   case '\r':
      break;

   default:
      if((m_N_item >=0) && (m_N_item <MAX_ITEM) && (m_N_pos < MAX_SIZE-1))
         m_N_strings[m_N_item][m_N_pos] = ch;
      m_N_pos++;
      break;
   }
}

static void m_Q(int ch, struct state_mach*m)
{
   switch (ch) {
   case 'F':
      if (m->jed->fuse_count != 0) {
         m->state = 0;
         return;
      }
      m->state = m_QF;
      m->jed->fuse_count = 0;
      break;
   case 'P':
      if (m->jed->pin_count != 0) {
         m->state = 0;
         return;
      }
      m->state = m_QP;
      m->jed->pin_count = 0;
      break;
   default:
      m->state = m_skip;
      break;
   }
}

static void m_QF(int ch, struct state_mach*m)
{
   if (isspace(ch))
      return;

   if (isdigit(ch)) {
      m->jed->fuse_count *= 10;
      m->jed->fuse_count += ch - '0';
      return;
   }

   if (ch == '*') {
      m->state = m_base;
      m->jed->fuse_list = allocate_fusemap(m->jed->fuse_count);
      return;
   }

   printf("m_QF exception\n"); // exception
}

static void m_QP(int ch, struct state_mach*m)
{
   if (isspace(ch))
      return;

   if (isdigit(ch)) {
      m->jed->pin_count *= 10;
      m->jed->pin_count += ch - '0';
      return;
   }

   if (ch == '*') {
      m->state = m_base;
      return;
   }

   printf("m_QP exception\n"); // exception
}

static void m_skip(int ch, struct state_mach*m)
{
   switch (ch) {
   case '*':
      m->state = m_base;
      break;

   default:
      break;
   }
}
jedec_data_t jed;
void JedecFile_init(void)
{
   jed->checksum = 0;
   jed->fuse_count = 0;
   jed->vector_count = 0;
   jed->pin_count = 0;
   jed->fuse_list = 0;
   jed->device[0] = 0;
   jed->version[0] = 0;
   jed->date[0] = 0;
}


int readFile_jedec(FIL *fp)
{
   unsigned char ch;
   struct state_mach m;
//   printf("readFile_jedec()\n");
   if(!fp)
      return 1;

   jed = (jedec_data_t)malloc(sizeof(struct jedec_data));
   JedecFile_init();
   m.jed = jed;
   m.state = m_startup;
   unsigned int br;
   while (!f_eof(fp)) {
      f_read(fp,&ch,1,&br);
      m.state(ch, &m);
      if (m.state == 0) {
         /* Some sort of error happened. */
         return 2;
      }
   }
   if (!jed->fuse_count)
      return 3;
   return 0;
}

void setLength(unsigned int f_count)
{
   if(f_count > jed->fuse_count)
   {
      if (jed->fuse_list)
         free(jed->fuse_list);
      jed->fuse_list = malloc(f_count/8 + ((f_count%8)?1:0));
      memset(jed->fuse_list, 0xff, f_count/8 + ((f_count%8)?1:0));
   }
   else
   {
      for (unsigned int i = f_count; i < jed->fuse_count; i++)
         set_fuse(i, 0);
   }
   jed->fuse_count = f_count;
}

void set_fuse(unsigned int idx, int blow)
{
   jedec_set_fuse(jed, idx,blow);
}

int get_fuse(unsigned int idx)
{
   return jedec_get_fuse(jed, idx);
}

uint16_t calcChecksum()
{
   unsigned int i;
   uint16_t cc=0;

   for(i=0; i<(jed->fuse_count/8 + ((jed->fuse_count%8)?1:0)); i++)
      cc += jed->fuse_list[i];
   return cc;
}
JedecFile jedecfile;
char *getDevice(){return jedecfile.jed.device;}
char *getVersion(){return jedecfile.jed.version;}
char *getDate_jedec(){return jedecfile.jed.date;}
