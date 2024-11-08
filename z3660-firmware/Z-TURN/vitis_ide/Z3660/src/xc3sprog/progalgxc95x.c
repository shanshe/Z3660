/* XC95 CPLD JTAG programming algorithms

Copyright (C) 2008-2009 Uwe Bonnes bon@elektron.ikp.physik.tu-darmstadt.de
(C) Copyright 2001 Nahitafu,Naitou Ryuji

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

 */

#include <stdio.h>
#include <sys/time.h>
//#include <unistd.h>
#include "progalgxc95x.h"
#define false 0
#define true 1

byte ISC_NOOP=0xff;
byte ISC_DISABLE=0xf0;
byte ISC_ERASE=0xed;
byte ISC_PROGRAM=0xea;
byte ISC_READ=0xee;
byte ISC_ENABLE=0xe9;
byte ISC_USERCODE=0xfd;

byte XSC_BLANK_CHECK=0xe5;

byte BYPASS=0xff;
int _gettimeofday (struct timeval *tv, void *tzvp)
{

   return(0);
}
#define deltaT(tvp1, tvp2) (((tvp2)->tv_sec-(tvp1)->tv_sec)*1000000 + \
      (tvp2)->tv_usec - (tvp1)->tv_usec)
int alg_blank_check()
{
   flow_enable();
   int ret= flow_blank_check();
   flow_disable();
   return ret;
};
int alg_erase()
{
   flow_enable();
   flow_erase();
   return flow_blank_check();
};
ProgAlgXC95X alg;

void ProgAlgXC95X_init(Jtag *j,IOBase *io, int s)
{
   alg.jtag=j;
   alg.io=io;

   switch (s)
   {
   case  1: alg.DRegLength = 2;
   break;
   case  2: alg.DRegLength = 4;
   break;
   case  4: alg.DRegLength = 8;
   break;
   case 11: alg.DRegLength = 16;
   break;
   default: printf("Unknown device\n");
   }
}

void flow_enable()
{
//   printf("flow_enable()\n");
   byte data[1];
   shiftIR(alg.jtag,alg.io,&ISC_ENABLE,0);
   data[0]=0x15;
   shiftDR(alg.jtag,alg.io,data,0,6,0,true);
   cycleTCK(alg.jtag,alg.io,1,1);
}

void flow_disable()
{
//   printf("flow_disable()\n");
   shiftIR(alg.jtag,alg.io,&ISC_DISABLE,0);
   usleep(100);
   shiftIR(alg.jtag,alg.io,&BYPASS,0);
   cycleTCK(alg.jtag,alg.io,1,1);
}


void flow_error_exit()
{
//   printf("flow_error_exit()\n");
   shiftIR(alg.jtag,alg.io,&ISC_NOOP,0);
   cycleTCK(alg.jtag,alg.io,1,1);
}
void print_hdmi_ln(int xpos, char *message, int line_inc);
int flow_blank_check()
{
//   printf("flow_blank_check()\n");
   byte i_data[3]={0x3,0,0};
   byte o_data[3];
   shiftIR(alg.jtag,alg.io,&XSC_BLANK_CHECK,0);
   shiftDR(alg.jtag,alg.io,i_data, 0,18,0,true);
   cycleTCK(alg.jtag,alg.io,500,1);
   shiftDR(alg.jtag,alg.io,0,o_data,18,0,true);
//   if(getVerbose_jtag(alg.jtag))
   {
      char message[100];
      if ((o_data[0] & 0x03) == 0x01)
         sprintf(message,"Device is blank");
      else
         sprintf(message,"Device is not blank");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

   }
   return ((o_data[0] & 0x03) == 0x01);

}

int flow_erase()
{
//   printf("flow_erase()\n");
   char message[100];
   sprintf(message,"Erasing device");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);

   byte i_data[3]={0x3,0,0};
   byte o_data[3];
   shiftIR(alg.jtag,alg.io,&ISC_ERASE,0);
   shiftDR(alg.jtag,alg.io,i_data,0,18,0,true);
   usleep(600000);
   shiftDR(alg.jtag,alg.io,0,o_data,18,0,true);
   if((o_data[0]& 0x03) != 0x01)
      printf( "Erase still running %02x\n", o_data[0]);
   else
      return(1);
   return(0);
}

int flow_usercode(char *userid)
{
//   printf("flow_usercode()\n");
   char message[100];
   sprintf(message,"Reading CPLD Usercode");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);

   byte i_data[4]={0,0,0,0};
   byte o_data[4];
   shiftIR(alg.jtag,alg.io,&ISC_USERCODE,0);
   shiftDR(alg.jtag,alg.io,i_data,0,32,0,true);
   shiftDR(alg.jtag,alg.io,0,o_data,32,0,true);
   //printf( "Usercode %02x %02x %02x %02x\n", o_data[3],o_data[2],o_data[1],o_data[0]);
   userid[0]=o_data[3];
   userid[1]=o_data[2];
   userid[2]=o_data[1];
   userid[3]=o_data[0];
   userid[4]=0;

   return(0);
}

#define MaxSector 108
#define MaxDRegLength 16
void print_hdmi_ln(int xpos, char *message, int line_inc);

int flow_array_program(JedecFile *file)
{
//   printf("flow_array_program()\n");
   byte preamble[1]= {0x01};
   byte i_data[MaxDRegLength+2];
   byte o_data[MaxDRegLength+3];
   struct timeval tv[2];
   unsigned long Addr=0;
   int bitlen;
   int k, sec,l,m;
   unsigned char data;
   unsigned int idx=0;


   gettimeofday(tv, NULL);
   printf("Programming\n");
   for(sec=0;sec < MaxSector;sec++)
   {
//      if(getVerbose_jtag(alg.jtag))
      {
         char message[100];
         sprintf(message,"Programming Sector %3d/%3d", sec+1,MaxSector);
         print_hdmi_ln(0,message,0);
      }
      preamble[0]= 0x01;
      for(l=0;l<3;l++){
         for(m=0;m<5;m++){
            Addr = sec*0x20 + l*0x08 + m;
            i_data[alg.DRegLength] = (byte) (Addr &0xff);
            i_data[alg.DRegLength+1] = (byte) ((Addr>>8) &0xff);
            if(l*5+m >= 9){
               bitlen=6;
            }
            else{
               bitlen=8;
            }
            for(int j=0;j<alg.DRegLength;j++)
            {
               data = 0;
               for(int i=0;i<bitlen;i++)
               {
                  data |=(get_fuse(idx++)<<i);
               }
               i_data[j] = data;
            }
            if ((l == 2) && (m == 4))
               preamble[0] = 0x03;
            Usleep(alg.io,1000); /*FIXME: IOFTDI Buffer causes abort with high
                                          rate on some board otherwise*/
            shiftIR(alg.jtag,alg.io,&ISC_PROGRAM,0);
            shiftDR(alg.jtag,alg.io,preamble,0,2,0,false);
            shiftDR(alg.jtag,alg.io,i_data,0,(alg.DRegLength+2)*8,0,true);
            if((l == 2) && (m == 4))
               Usleep(alg.io,50000);
            else
               cycleTCK(alg.jtag,alg.io,1,1);
            if ((l == 2) && (m == 4))
            {
               preamble[0]= 0x00;
               for(k=0; k< 32; k++)
               {
                  shiftIR(alg.jtag,alg.io,&ISC_PROGRAM,0);
                  shiftDR(alg.jtag,alg.io,preamble, 0,2,0,false);
                  shiftDR(alg.jtag,alg.io,i_data, 0,(alg.DRegLength+2)*8,0,true);
                  Usleep(alg.io,50000);
                  shiftDR(alg.jtag,alg.io,0,o_data, ((alg.DRegLength+2)*8)+2,0,true);
                  if(getVerbose_jtag(alg.jtag))
                  {
                     printf( ".");
                  }
                  if ((o_data[0] & 0x03) == 0x01)
                     break;
               }
               if (k == 32)
               {
                  printf( "failed\n");
                  return 1;
               }
            }
         }
      }
   }
   gettimeofday(tv+1, NULL);
//   if(getVerbose_jtag(alg.jtag))
   {
      char message[100];
      sprintf(message,"P"); // P of "Programming"
      print_hdmi_ln(0,message,1);
      sprintf(message,"Finished!");
      print_hdmi_ln(0,message,1);
      printf("%s\n", message);

//      printf( "\nProgramming  time %.1f ms\n",
//            (double)deltaT(tv, tv + 1)/1.0e3);
   }
   return 0;
}


int flow_array_verify(JedecFile *file)
{
//   printf("flow_array_verify()\n");
   byte preamble[1]= {0x03};
   byte i_data[MaxDRegLength+2];
   byte o_data[MaxDRegLength+2];
   struct timeval tv[2];
   for(int i=0;i<MaxDRegLength+2;i++)
   {
      i_data[i]=0;
      o_data[i]=0;
   }
   unsigned long Addr=0;
   int bitlen=0;
   int sec,l,m;
   unsigned char data;
   unsigned int idx=0;

   gettimeofday(tv, NULL);
   printf("Verifying\n");
   for(sec=0;sec < MaxSector;sec++)
   {
//      if(getVerbose_jtag(alg.jtag))
      {
         char message[100];
         sprintf(message,"Verifying Sector %3d/%3d", sec+1,MaxSector);
         print_hdmi_ln(0,message,0);
      }
      for(l=0;l<3;l++){
         for(m=0;m<5;m++){
            Addr = sec*0x20 + l*0x08 + m;
            i_data[alg.DRegLength] = (byte) (Addr &0xff);
            i_data[alg.DRegLength+1] = (byte) ((Addr>>8) &0xff);
            shiftIR(alg.jtag,alg.io,&ISC_READ,0);
            shiftDR(alg.jtag,alg.io,preamble,0,2,0,false);
            shiftDR(alg.jtag,alg.io,i_data,o_data,(alg.DRegLength+2)*8,0,true);
            cycleTCK(alg.jtag,alg.io,1,1);
            if(sec | l | m )
            {
               for(int j=0;j<alg.DRegLength;j++)
               {
                  data = o_data[j];
                  for(int i=0;i<bitlen;i++)
                  {
                     if ((data& 0x01) != get_fuse(idx))
                     {
                        printf( "\nMismatch at fuse %6d: %d vs %d\n",
                              idx, data& 0x01, get_fuse(idx));
                        return 1;
                     }
                     data = data >> 1;
                     idx++;
                  }
               }
            }
            if(l*5+m >= 9){
               bitlen=6;
            }
            else{
               bitlen=8;
            }
         }
      }
   }
   /* Now read the security fuses*/
   shiftIR(alg.jtag,alg.io,&ISC_READ,0);
   shiftDR(alg.jtag,alg.io,preamble,0,2,0,false);
   shiftDR(alg.jtag,alg.io,i_data,o_data,(alg.DRegLength+2)*8,0,true);
   for(int j=0;j<alg.DRegLength;j++)
   {
      data = o_data[j];
      for(int i=0;i<bitlen;i++){
         if ((data& 0x01) != get_fuse(idx++))
         {
            idx--;
            printf( "\nMismatch at security fuse %6d: %d vs %d\n",
                  idx, data& 0x01, get_fuse(idx));
            return 1;
         }
         data = data >> 1;
      }
   }

   gettimeofday(tv+1, NULL);
//   if(getVerbose_jtag(alg.jtag))
   {
      char message[100];
      sprintf(message,"V"); // V for "Verifying"
      print_hdmi_ln(0,message,1);
      sprintf(message,"Success!");
      print_hdmi_ln(0,message,1);
      printf("%s\n", message);

//      printf( "\nSuccess! Verify time %.1f ms\n",
//            (double)deltaT(tv, tv + 1)/1.0e3);
   }
   return 0;
}

void alg_array_program(JedecFile *file)
{
   flow_array_program(file);
}

int alg_array_verify(JedecFile *file)
{
   int ret;
//   printf("alg_array_verify()\n");
   flow_enable();
   ret = flow_array_verify(file);
   flow_disable();
   return ret;
}
