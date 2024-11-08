/* JTAG routines

Copyright (C) 2004 Andrew Rogers
Copyright (C) 2005-2009 Uwe Bonnes

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

#include <stdio.h>
#include <stdlib.h>

#include "jtag.h"

//#include <unistd.h>
#define false 0
#define true 1
Jtag jtag;
void setVerbose_jtag(Jtag *j,bool v) { j->verbose = v; }
bool getVerbose_jtag(Jtag *j) { return j->verbose; }
DeviceID getDeviceID(Jtag *j,unsigned int dev){
   //  if(dev>=sizeof(j->devices))return 0;
   return 0x09608093;
}

void Jtag_init(Jtag *j,IOBase *iob)
{
//   j->MAXNUMDEVICES=1;

   j->verbose = false;
//   j->io=iob;
//   j->current_state = UNKNOWN;
   j->postDRState = RUN_TEST_IDLE;
   j->postIRState = RUN_TEST_IDLE;
//   j->deviceIndex = -1;
//   j->numDevices  = -1;
   j->shiftDRincomplete=false;
   j->debug = false;
   iob->verbose=false;
}

void Jtag_end(Jtag *j)
{
}

/* Detect chain length on first start, return chain length else*/
int getChain(Jtag *j,IOBase *io, bool detect)
{
   if(j->numDevices  == -1 || detect)
   {
      tapTestLogicReset(j,io);
      setTapState(j,io,SHIFT_DR,0);
      byte idx[4];
      byte zero[4];
      j->numDevices=0;
      for(int i=0; i<4; i++)zero[i]=0;
      do{
         shiftTDITDO(io,zero,idx,32,false);
         unsigned long id=byteArrayToLong(idx);
         if(id!=0 && id !=0xffffffff){
            j->numDevices++;
            //            chainParam_t dev;
            //            dev.idcode=id;
            //            insert(j->devices,dev);
         }
         else {
            if (id == 0xffffffff && j->numDevices >0)
            {
               printf("Probably a broken Atmel device in your chain!\n");
               printf("No succeeding device can be identified\n");
            }
            break;
         }
      }while(j->numDevices<j->MAXNUMDEVICES);
      setTapState(j,io,TEST_LOGIC_RESET,0);
   }
   if(j->debug)
      printf("getChain found %d devices\n",j->numDevices);
   return j->numDevices;
}

const char* getStateName(tapState_t s)
{
   switch(s)
   {
   case TEST_LOGIC_RESET: return "TEST_LOGIC_RESET";
   case RUN_TEST_IDLE: return "RUN_TEST_IDLE";
   case SELECT_DR_SCAN: return "SELECT_DR_SCAN";
   case CAPTURE_DR: return "CAPTURE_DR";
   case SHIFT_DR: return "SHIFT_DR";
   case EXIT1_DR: return "EXIT1_DR";
   case PAUSE_DR: return "PAUSE_DR";
   case EXIT2_DR: return "EXIT2_DR";
   case UPDATE_DR: return "UPDATE_DR";
   case SELECT_IR_SCAN: return "SELECT_IR_SCAN";
   case CAPTURE_IR: return "CAPTURE_IR";
   case SHIFT_IR: return "SHIFT_IR";
   case EXIT1_IR: return "EXIT1_IR";
   case PAUSE_IR: return "PAUSE_IR";
   case EXIT2_IR: return "EXIT2_IR";
   case UPDATE_IR: return "UPDATE_IR";
   default:
      return "Unknown";
   }
}

int selectDevice(Jtag *j,int dev)
{
   if(dev>=j->numDevices) j->deviceIndex=-1;
   else j->deviceIndex=dev;
   if(j->debug)
      printf("selectDevices %d\n", j->deviceIndex);
   return j->deviceIndex;
}

void cycleTCK(Jtag *j,IOBase *io,int n, bool tdi)
{
   if(j->current_state==TEST_LOGIC_RESET)
      printf( "cycleTCK in TEST_LOGIC_RESET\n");
   if(j->debug)
      printf( "cycleTCK %d TDI %s\n", n, (tdi)?"TRUE":"FALSE");
   shift(io,tdi, n, false);
}

int setDeviceIRLength(Jtag *j,int dev, int len)
{
   if(dev>=j->numDevices||dev<0)return -1;
   j->devices.irlen=len;
   return dev;
}

void shiftDR(Jtag *j,IOBase *io, byte *tdi, byte *tdo, int length, int align, bool exit)
{
   if(j->deviceIndex<0)return;
   int post=j->deviceIndex;
   if(!j->shiftDRincomplete){
      int pre=j->numDevices-j->deviceIndex-1;
      if(align){
         pre=-post;
         while(pre<=0)pre+=align;
      }
      /* We can combine the pre bits to reach the target device with
     the TMS bits to reach the SHIFT-DR state, as the pre bit can be '0'*/
      setTapState(j,io,SHIFT_DR,pre);
   }
   if(j->debug)
   {
      printf( "shiftDR len %d\n", length);
      if (tdi)
      {
         int i;
         printf( "In:\n" );
         for (i=0; i< (length+7)>>3; i++)
         {
            printf( " %02x", tdi[i]);
            if (i % 26== 25)
               printf( "\n");
         }
         printf( "\n");
      }
   }
   if(tdi!=0&&tdo!=0) shiftTDITDO(io,tdi,tdo,length,post==0&&exit);
   else if(tdi!=0&&tdo==0) shiftTDI(io,tdi,length,post==0&&exit);
   else if(tdi==0&&tdo!=0) shiftTDO(io,tdo,length,post==0&&exit);
   else  shift(io,false,length,post==0&&exit);
   if(j->debug)
   {
      if (tdo)
      {
         int i;
         printf( "Out:\n" );
         for (i=0; i< (length+7)>>3; i++)
         {
            printf( " %02x", tdo[i]);
            if (i % 26 == 25)
               printf( "\n");
         }
         printf( "\n");
      }
   }
   nextTapState(j,io,post==0&&exit); // If TMS is set the the state of the tap changes
   if(exit){
      shift(io,false,post,true);
      if (!(post==0&&exit))
         nextTapState(j,io,true);
      setTapState(j,io,j->postDRState,0);
      j->shiftDRincomplete=false;
      flush_tms(io);
   }
   else j->shiftDRincomplete=true;
}

void shiftIR(Jtag *j,IOBase *io, byte *tdi, byte *tdo)
{
   if(j->deviceIndex<0)return;
   setTapState(j,io,SHIFT_IR,0);
   if(j->debug)
   {
      printf( "shiftIR ");
      if (tdi)
         printf( "In: %02x", *tdi );
   }
   int pre=0;
   for(int dev=j->deviceIndex+1; dev<j->numDevices; dev++)
      pre+=j->devices.irlen; // Calculate number of pre BYPASS bits.
   int post=0;
   for(int dev=0; dev<j->deviceIndex; dev++)
      post+=j->devices.irlen; // Calculate number of post BYPASS bits.
   shift(io,true,pre,false);
   if(tdo!=0) shiftTDITDO(io,tdi,tdo,j->devices.irlen,post==0);
   else if(tdo==0) shiftTDI(io,tdi,j->devices.irlen,post==0);
   shift(io,true,post,true);
   if(j->debug)
   {
      if (tdo)
         printf( "Out: %02x", *tdo);
      printf( "\n");
   }
   nextTapState(j,io,true);
   setTapState(j,io,j->postIRState,0);
}

void setTapState(Jtag *j,IOBase *io,tapState_t state, int pre)
{
   bool tms;
   while(j->current_state!=state){
      switch(j->current_state){

      case TEST_LOGIC_RESET:
         switch(state){
         case TEST_LOGIC_RESET:
            tms=true;
            break;
         default:
            tms=false;
            j->current_state=RUN_TEST_IDLE;
         };
         break;

      case RUN_TEST_IDLE:
         switch(state){
         case RUN_TEST_IDLE:
            tms=false;
            break;
         default:
            tms=true;
            j->current_state=SELECT_DR_SCAN;
         };
         break;

      case SELECT_DR_SCAN:
         switch(state){
         case CAPTURE_DR:
         case SHIFT_DR:
         case EXIT1_DR:
         case PAUSE_DR:
         case EXIT2_DR:
         case UPDATE_DR:
            tms=false;
            j->current_state=CAPTURE_DR;
            break;
         default:
            tms=true;
            j->current_state=SELECT_IR_SCAN;
         };
         break;

      case CAPTURE_DR:
         switch(state){
         case SHIFT_DR:
            tms=false;
            j->current_state=SHIFT_DR;
            break;
         default:
            tms=true;
            j->current_state=EXIT1_DR;
         };
         break;

      case SHIFT_DR:
         switch(state){
         case SHIFT_DR:
            tms=false;
            break;
         default:
            tms=true;
            j->current_state=EXIT1_DR;
         };
         break;

      case EXIT1_DR:
         switch(state){
         case PAUSE_DR:
         case EXIT2_DR:
         case SHIFT_DR:
         case EXIT1_DR:
            tms=false;
            j->current_state=PAUSE_DR;
            break;
         default:
            tms=true;
            j->current_state=UPDATE_DR;
         };
         break;

      case PAUSE_DR:
         switch(state){
         case PAUSE_DR:
            tms=false;
            break;
         default:
            tms=true;
            j->current_state=EXIT2_DR;
         };
         break;

      case EXIT2_DR:
         switch(state){
         case SHIFT_DR:
         case EXIT1_DR:
         case PAUSE_DR:
            tms=false;
            j->current_state=SHIFT_DR;
            break;
         default:
            tms=true;
            j->current_state=UPDATE_DR;
         };
         break;

      case UPDATE_DR:
         switch(state){
         case RUN_TEST_IDLE:
            tms=false;
            j->current_state=RUN_TEST_IDLE;
            break;
         default:
            tms=true;
            j->current_state=SELECT_DR_SCAN;
         };
         break;

      case SELECT_IR_SCAN:
         switch(state){
         case CAPTURE_IR:
         case SHIFT_IR:
         case EXIT1_IR:
         case PAUSE_IR:
         case EXIT2_IR:
         case UPDATE_IR:
            tms=false;
            j->current_state=CAPTURE_IR;
            break;
         default:
            tms=true;
            j->current_state=TEST_LOGIC_RESET;
         };
         break;

      case CAPTURE_IR:
         switch(state){
         case SHIFT_IR:
            tms=false;
            j->current_state=SHIFT_IR;
            break;
         default:
            tms=true;
            j->current_state=EXIT1_IR;
         };
         break;

      case SHIFT_IR:
         switch(state){
         case SHIFT_IR:
            tms=false;
            break;
         default:
            tms=true;
            j->current_state=EXIT1_IR;
         };
         break;

      case EXIT1_IR:
         switch(state){
         case PAUSE_IR:
         case EXIT2_IR:
         case SHIFT_IR:
         case EXIT1_IR:
            tms=false;
            j->current_state=PAUSE_IR;
            break;
         default:
            tms=true;
            j->current_state=UPDATE_IR;
         };
         break;

      case PAUSE_IR:
         switch(state){
         case PAUSE_IR:
            tms=false;
            break;
         default:
            tms=true;
            j->current_state=EXIT2_IR;
         };
         break;

      case EXIT2_IR:
         switch(state){
         case SHIFT_IR:
         case EXIT1_IR:
         case PAUSE_IR:
            tms=false;
            j->current_state=SHIFT_IR;
            break;
         default:
            tms=true;
            j->current_state=UPDATE_IR;
         };
         break;

      case UPDATE_IR:
         switch(state){
         case RUN_TEST_IDLE:
            tms=false;
            j->current_state=RUN_TEST_IDLE;
            break;
         default:
            tms=true;
            j->current_state=SELECT_DR_SCAN;
         };
         break;

      default:
         tapTestLogicReset(j,io);
         tms=true;
      };
      if(j->debug)
         printf("TMS %d: %s\n", tms, getStateName(j->current_state));
      set_tms(io,tms);
   }
   for(int i=0; i<pre; i++)
      set_tms(io,false);
}

// After shift data into the DR or IR we goto the next state
// This function should only be called from the end of a shift function
void nextTapState(Jtag *j,IOBase *io,bool tms)
{
   if(j->current_state==SHIFT_DR){
      if(tms)j->current_state=EXIT1_DR; // If TMS was set then goto next state
   }
   else if(j->current_state==SHIFT_IR){
      if(tms)j->current_state=EXIT1_IR; // If TMS was set then goto next state
   }
   else
   {
      printf("Unexpected state %d\n",j->current_state);
      tapTestLogicReset(j,io); // We were in an unexpected state
   }
}

void tapTestLogicReset(Jtag *j,IOBase *io)
{
   int i;
   for(i=0; i<5; i++)
      set_tms(io,true);
   j->current_state=TEST_LOGIC_RESET;
   flush_tms(io);
}
