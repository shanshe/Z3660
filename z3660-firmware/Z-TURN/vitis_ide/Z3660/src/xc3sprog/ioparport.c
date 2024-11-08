/* JTAG GNU/Linux parport device io

Copyright (C) 2004 Andrew Rogers
Additions for Byte Blaster Cable (C) 2005-2011  Uwe Bonnes
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
Dmitry Teytelman [dimtey@gmail.com] 14 Jun 2006 [applied 13 Aug 2006]:
    Code cleanup for clean -Wall compile.
    Changes to support new IOBase interface.
    Support for byte counting and progress bar.
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <xgpiops.h>
#include <xiicps.h>
#include <xclk_wiz_hw.h>
#include <xparameters.h>
#include "../main.h"
#include "../mobotest.h"
extern XGpioPs GpioPs;
#define false 0
#define true 1

//#include <unistd.h>

#include "ioparport.h"
IOParport iopp;
//#include "debug.h"
void debug_ioparport(const char *format, ...)
{
   if(iopp.debug==0)
      return;
   va_list args;
   va_start(args, format);
   vprintf(format,args);
   va_end(args);
}


#define NO_CABLE 0
#define IS_PCIII 1
#define IS_BBLST 2

#define BIT_MASK(b)     (1<<(b))

/* Attention: PARPORT_STATUS_BUSY reflects the inverted input */
/* Attention: PARPORT_CONTROL_AUTOFD write zero to output */

/* Altera Byteblaster Definitions */
#define BBLST_DEF_BYTE      0
#define BBLST_ENABLE_N      PARPORT_CONTROL_AUTOFD   /* Base + 2, Inv */
#define BBLST_TCK_VALUE     BIT_MASK(0)              /* Base */
#define BBLST_TMS_VALUE     BIT_MASK(1)              /* Base */
#define BBLST_TDI_VALUE     BIT_MASK(6)              /* Base */
#define BBLST_RESET_VALUE   BIT_MASK(3)              /* Base, Inv by Open
			    			        Collector Transistor */
#define BBLST_TDO_MASK      PARPORT_STATUS_BUSY      /* Base + 1, Input */
#define BBLST_LB_IN_MASK    PARPORT_STATUS_PAPEROUT  /* Base + 1, Input */
#define BBLST_LB_OUT_VALUE  BIT_MASK(7)              /* Base */
#define BBLST_ACK_OUT_VALUE BIT_MASK(5)
#define BBLST_ACK_IN_MASK   PARPORT_STATUS_ACK

/* Xilinx Parallel Cable III Definitions */
#define PCIII_PROG_EN_N     BIT_MASK(4)
#define PCIII_DEF_BYTE      PCIII_PROG_EN_N
#define PCIII_TCK_VALUE     BIT_MASK(1)              /* Base */
#define PCIII_TMS_VALUE     BIT_MASK(2)              /* Base */
#define PCIII_TDI_VALUE     BIT_MASK(0)              /* Base */
#define PCIII_TDO_MASK      PARPORT_STATUS_SELECT
#define PCIII_CHECK_OUT     BIT_MASK(6)
#define PCIII_CHECK_IN1     PARPORT_STATUS_BUSY
#define PCIII_CHECK_IN2     PARPORT_STATUS_PAPEROUT

int  detectcable(void)
{
   return NO_CABLE;
}

int Init(unsigned int freq, IOpp_mode mode)
{
   iopp.debug=0;

   iopp.total=0;
   iopp.def_byte=0x00;
   if(mode==IOPP_MODE_I2CSW)
   {
      iopp.tdi_value=0x80;
      iopp.tms_value=0x10;
      iopp.tck_value=0x20;
      iopp.tdo_mask =0x40;
   }
   else
   {
      iopp.tdi_value=0x10;
      iopp.tms_value=0x40;
      iopp.tck_value=0x80;
      iopp.tdo_mask =0x20;
   }
   iopp.tdo_inv=0;
   iopp.mode=mode;
   return 0;
}
bool txrx(bool tms, bool tdi)
{
   unsigned char ret=0;
   bool retval;
   unsigned char data=iopp.def_byte; // D4 pin5 TDI enable
   if(tdi)data|=iopp.tdi_value; // D0 pin2
   if(tms)data|=iopp.tms_value; // D2 pin4
   write_data(data);
   data|=iopp.tck_value; // clk high D1 pin3
   write_data(data);
   iopp.total++;
   read_status(&ret);
   //data=data^2; // clk low
   //write_data(fd, data);
   //read_status(fd, &ret);
   retval = (ret&iopp.tdo_mask)?!iopp.tdo_inv:iopp.tdo_inv;
   debug_ioparport("txrx tms %s tdi %s tdo %s \n",
         (tms)?"true ":"false", (tdi)?"true ":"false",
               (retval)?"true ":"false");
   return retval;
}

void tx(bool tms, bool tdi)
{
   unsigned char data=iopp.def_byte; // D4 pin5 TDI enable
   debug_ioparport("tx tms %s tdi %s\n",(tms)?"true ":"false",
         (tdi)?"true ":"false");
   if(tdi)data|=iopp.tdi_value; // D0 pin2
   if(tms)data|=iopp.tms_value; // D2 pin4
   write_data(data);
   //delay(2);
   data|=iopp.tck_value; // clk high
   iopp.total++;
   write_data(data);
   //delay(2);
   //data=data^2; // clk low
   //write_data(fd, data);
   //delay(2);
}

void tx_tdi_byte(unsigned char tdi_byte)
{
   int k;

   for (k = 0; k < 8; k++)
      tx(false, (tdi_byte>>k)&1);
}
void txrx_block(unsigned char *tdi, unsigned char *tdo,
      int length, bool last)
{
   int i=0;
   int j=0;
   unsigned char tdo_byte=0;
   unsigned char tdi_byte=0;
   unsigned char data=iopp.def_byte;
   if (tdi)
      tdi_byte = tdi[j];

   while(i<length-1){
      tdo_byte=tdo_byte+(txrx(false, (tdi_byte&1)==1)<<(i%8));
      if (tdi)
         tdi_byte=tdi_byte>>1;
      i++;
      if((i%8)==0){ // Next byte
         if(tdo)
            tdo[j]=tdo_byte; // Save the TDO byte
         tdo_byte=0;
         j++;
         if (tdi)
            tdi_byte=tdi[j]; // Get the next TDI byte
      }
   }
   tdo_byte=tdo_byte+(txrx(last, (tdi_byte&1)==1)<<(i%8));
   if(tdo)
      tdo[j]=tdo_byte;
   write_data(data); /* Make sure, TCK is low */
   return;
}

void tx_tms(unsigned char *pat, int length)
{
   int i;
   unsigned char tms=0;
   unsigned char data=iopp.def_byte;
   for (i = 0; i < length; i++)
   {
      if ((i & 0x7) == 0)
         tms = pat[i>>3];
      tx((tms & 0x01), true);
      tms = tms >> 1;
   }
   write_data(data); /* Make sure, TCK is low */
}


#define XC3S_OK 0
#define XC3S_EIO 1
#define XC3S_ENIMPL 2
#ifdef DIRECT_CPLD_PROGRAMMING
unsigned char tdi_value_last=255;
unsigned char tms_value_last=255;
int write_data(unsigned char data)
{
   //    debug_ioparport("WRITE fd %d, data %02X\n",fd,data);
   int clock_used=0;
   if((data&iopp.tdi_value)!=tdi_value_last)
   {
      tdi_value_last=data&iopp.tdi_value;

      if(data&iopp.tdi_value) //=0x01
      {
         // CPUCLK
         XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( 100 *1000));
      }
      else
      {
         XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( 0 *1000));
      }
      clock_used=1;
   }
   if((data&iopp.tms_value)!=tms_value_last)
   {
      tms_value_last=data&iopp.tms_value;
      if(data&iopp.tms_value) //=0x02
      {
         // CLK90
         XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( 100 *1000));
      }
      else
      {
         XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( 0 *1000));
      }
      clock_used=1;
   }
   if(clock_used)
   {
      XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
//      while(XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x004)==0);
   }
//   else
   {
   }
   if(data&iopp.tck_value) //=0x04
   {
      // JP1
      XGpioPs_WritePin(&GpioPs, JP1_PIN, 1);
//      printf("TMS %d TDI %d\n",!!(data&iopp.tms_value),!!(data&iopp.tdi_value));
   }
   else
   {
      XGpioPs_WritePin(&GpioPs, TCK_PIN, 0);
   }
   usleep(20);

   return 0;
}

unsigned char read_status(unsigned char *status)
{
   // read TDO (JP2)
   if(XGpioPs_ReadPin(&GpioPs, JP2_PIN)==0)
      *status=0;
   else
      *status=iopp.tdo_mask;
//   debug_ioparport("READ fd %d, status %02X\n",fd,*status);
//   printf("TDO status %d\n",!!*status);
   return 0;
}

#endif
#ifdef I2C_CPLD_PROGRAMMING
#endif
#ifdef I2CSW_CPLD_PROGRAMMING
typedef struct
{
   uint8_t b0:1; // I2C_MASTER_NACK
   uint8_t b1:1; // I2C_MASTER_STOP
   uint8_t b2:1; // I2C_MASTER_ERROR
} I2C_BITS;
typedef struct
{
   uint8_t config;
   I2C_BITS flags;
   uint8_t data_rd;
   uint8_t i; // aux
} I2C;
I2C i2c;
#define I2C_MASTER_ACK  i2c.flags.b0
#define I2C_MASTER_STOP i2c.flags.b1
#define I2C_SLAVE_ERROR i2c.flags.b2
#define I2C_SDA_DIR_0 do{XGpioPs_SetDirectionPin(&GpioPs, JP2_PIN, 0);\
                      XGpioPs_SetOutputEnablePin(&GpioPs, JP2_PIN, 0);}while(0)
#define I2C_SDA_DIR_1 do{XGpioPs_SetDirectionPin(&GpioPs, JP2_PIN, 1);\
                      XGpioPs_SetOutputEnablePin(&GpioPs, JP2_PIN, 1);}while(0)
#define I2C_SCL_DIR_0 do{XGpioPs_SetDirectionPin(&GpioPs, JP1_PIN, 0);\
                      XGpioPs_SetOutputEnablePin(&GpioPs, JP1_PIN, 0);}while(0)
#define I2C_SCL_DIR_1 do{XGpioPs_SetDirectionPin(&GpioPs, JP1_PIN, 1);\
                      XGpioPs_SetOutputEnablePin(&GpioPs, JP1_PIN, 1);}while(0)

#define I2C_SDA_PORT_0 do{XGpioPs_WritePin(&GpioPs, JP2_PIN, 0);}while(0)
#define I2C_SDA_PORT_1 do{XGpioPs_WritePin(&GpioPs, JP2_PIN, 1);}while(0)
#define I2C_SCL_PORT_0 do{XGpioPs_WritePin(&GpioPs, JP1_PIN, 0);}while(0)
#define I2C_SCL_PORT_1 do{XGpioPs_WritePin(&GpioPs, JP1_PIN, 1);}while(0)

#define I2C_SDA_0() do{I2C_SDA_DIR_1;I2C_SDA_PORT_0;}while(0)
#define I2C_SCL_0() do{I2C_SCL_DIR_1;I2C_SCL_PORT_0;}while(0)
#define I2C_SDA_1() do{I2C_SDA_DIR_0;}while(0)
#define I2C_SCL_1() do{I2C_SCL_DIR_0;while(XGpioPs_ReadPin(&GpioPs, JP1_PIN)==0){}}while(0)
//#define i2c_delay() usleep(1)
#define i2c_delay() do{__asm(" nop");dsb();}while(0)
void i2csw_start()
{
   I2C_MASTER_ACK=0;
   I2C_MASTER_STOP=0;
   I2C_SLAVE_ERROR=0;

   I2C_SDA_1();
   I2C_SCL_1();
   i2c_delay();
   I2C_SDA_0();
   i2c_delay();
   I2C_SCL_0();
   i2c_delay();
}
void i2csw_stop()
{
   I2C_SDA_0();
   I2C_SCL_1();
   i2c_delay();
   I2C_SDA_1();
   i2c_delay();
}
uint8_t i2csw_send(uint8_t data)
{
   I2C_MASTER_ACK=0;
   for(i2c.i=8;i2c.i>0;i2c.i--)
   {
      i2c_delay();
      if((data&(1<<(i2c.i-1)))==0)
      {
         I2C_SDA_0();
      }
      else
      {
         I2C_SDA_1();
      }
      i2c_delay();
      I2C_SCL_1();
      i2c_delay();
      I2C_SCL_0();
   }
   I2C_SDA_1();
   i2c_delay();
   if(XGpioPs_ReadPin(&GpioPs, JP2_PIN)==0)
   {
      I2C_MASTER_ACK=1;
   }
   I2C_SCL_1();
   i2c_delay();
   I2C_SCL_0();
   i2c_delay();
   return(I2C_MASTER_ACK==0);
}
uint8_t i2csw_receive(void)
{
   i2c.data_rd=0;
   I2C_SDA_1();
   i2c_delay();
   i2c_delay();
   for(i2c.i=8;i2c.i>0;i2c.i--)
   {
      I2C_SCL_1();
      i2c.data_rd=i2c.data_rd<<1;
      i2c_delay();
      if(XGpioPs_ReadPin(&GpioPs, JP2_PIN)==0)
      {
         i2c.data_rd=i2c.data_rd&0b11111110;
      }
      else
      {
         i2c.data_rd=i2c.data_rd|0b00000001;
      }
      I2C_SCL_0();
      i2c_delay();
   }
   if(I2C_MASTER_STOP==0)
   {
      I2C_SDA_0();
   }
   I2C_SCL_1();
   i2c_delay();
   I2C_SCL_0();
   i2c_delay();
   return(i2c.data_rd);
}
extern XIicPs IicInstance;
void i2c_send(uint8_t data)
{
   static int i2c_ltc2990=0;
   int Status;
   uint8_t WriteBuffer[1];
   WriteBuffer[0]   =   data;

   Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
                 1, 0x20);
   if (Status != XST_SUCCESS) {
      i2c_ltc2990++;
      printf("[I2C] write1 error %d\n",i2c_ltc2990);
      return;
   }
//   DEBUG_I2C("[I2C] write1 ok\n");
   while(XIicPs_BusIsBusy(&IicInstance)!=XST_SUCCESS) {}
//   DEBUG_I2C("[I2C] write slave:%02x data:%02x ok\n", 0x20, data);
}
uint8_t i2c_receive(void)
{
   static int i2c_ltc2990=0;
   int Status;
   uint8_t ReadBuffer[1];
   Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer,
                 1, 0x20);
   if (Status != XST_SUCCESS) {
      i2c_ltc2990++;
      printf("[I2C] read error %d\n",i2c_ltc2990);
      return(0);
   }
   while(XIicPs_BusIsBusy(&IicInstance)!=XST_SUCCESS) {}
//   DEBUG_I2C("[I2C] write slave:%02x data:%02x ok\n", 0x20, ReadBuffer[0]);
   return(ReadBuffer[0]);
}

int write_data(unsigned char data)
{
   //    debug_ioparport("WRITE fd %d, data %02X\n",fd,data);
   uint8_t data_to_send=iopp.tdo_mask|0x0F;
   data_to_send|=data&iopp.tdi_value;
   data_to_send|=data&iopp.tms_value;
   data_to_send|=data&iopp.tck_value;
   if(iopp.mode==IOPP_MODE_I2CSW)
   {
      i2csw_start();
      i2csw_send(0x40);
      i2csw_send(data_to_send);
      i2csw_stop();
   }
   else
   {
      i2c_send(data_to_send);
   }
   return 0;
}
unsigned char read_status(unsigned char *status)
{
   if(iopp.mode==IOPP_MODE_I2CSW)
   {
      // read TDO (JP2)
      i2csw_start();
      i2csw_send(0x41);
      I2C_MASTER_STOP=1;
      uint8_t data=i2csw_receive();
      *status=data&iopp.tdo_mask;
      I2C_MASTER_STOP=0;
      i2csw_stop();
   }
   else
   {
      uint8_t data=i2c_receive();
      *status=data&iopp.tdo_mask;
   }
//   debug_ioparport("READ fd %d, status %02X\n",fd,*status);
//   printf("TDO status %d\n",!!*status);
   return 0;
}
void i2c_finish(void)
{
   uint8_t data_to_send=0x00;
   if(iopp.mode==IOPP_MODE_I2CSW)
   {
      i2csw_start();
      i2csw_send(0x40);
      i2csw_send(data_to_send);
      i2csw_stop();

      i2csw_start();
      i2csw_send(0x41);
      I2C_MASTER_STOP=1;
      //uint8_t data=
      i2csw_receive();
      I2C_MASTER_STOP=0;
      i2csw_stop();
   }
   else
   {

   }
}

#endif
