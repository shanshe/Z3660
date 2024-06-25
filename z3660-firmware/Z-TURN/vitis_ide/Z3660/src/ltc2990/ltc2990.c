/*
 * ltc2990_init.c
 *
 *  Created on: 08/13/2023
 *      Author: sHaNsHe
 */

#include "ltc2990.h"

#include "xil_types.h"
#include "xiicps.h"
//#include "xparameters.h"
#include "../sii9022_init/sii9022_init.h"

#include "sleep.h"
#include <stdio.h>

#include "../debug_console.h"

extern DEBUG_CONSOLE debug_console;

extern XIicPs IicInstance;      /* The instance of the IIC device. */

#define IIC_DEVICE_ID   XPAR_XIICPS_0_DEVICE_ID

volatile uint8_t WriteBuffer_ltc2990[2];

volatile uint8_t ReadBuffer_ltc2990[2];   /* Read buffer for reading a page. */

void DEBUG_I2C(const char *format, ...)
{
	if(debug_console.debug_i2c==0)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format,args);
	va_end(args);
}

int i2c_ltc2990=0;
int iic_write_ltc2990(uint8_t command,uint8_t data)
{
   static int state=0;
   int Status;
   if(i2c_ltc2990==100 || debug_console.stop_i2c)
   {
      return(1);
   }
   else if(i2c_ltc2990==99)
   {
      printf("[I2C] LTC2990 DISABLED!!!!!!\n");
      i2c_ltc2990++;
      return(1);
   }
   switch(state)
   {
   case 0:
      WriteBuffer_ltc2990[0]   =   command;
      WriteBuffer_ltc2990[1]   =   data;

      Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer_ltc2990,
                 2, LTC_I2C_ADDRESS);
      if (Status != XST_SUCCESS) {
         i2c_ltc2990++;
         DEBUG_I2C("[I2C] write1 error %d\n",i2c_ltc2990);
         return(1);
      }
      state=1;
      DEBUG_I2C("[I2C] write1 ok\n");
      break;
   case 1:
      Status=XIicPs_BusIsBusy(&IicInstance);
      if(Status==XST_SUCCESS) {
    	  state=0;
          DEBUG_I2C("[I2C] write slave:%02x cmd:%02x data:%02x ok\n", LTC_I2C_ADDRESS, command, data);
    	  return(1);
      }
      DEBUG_I2C("[I2C] waiting bus busy (write)\n");
      break;
   default:
      DEBUG_I2C("[I2C] default1\n");
   }
   return(0);
}
int iic_read_ltc2990(uint8_t command)
{
   static int state=0;
   int Status;
   if(i2c_ltc2990==100 || debug_console.stop_i2c)
   {
      return(1);
   }
   else if(i2c_ltc2990==99)
   {
      printf("[I2C] LTC2990 DISABLED!!!!!!\n");
      i2c_ltc2990++;
      return(1);
   }
   switch(state)
   {
   case 0:
      WriteBuffer_ltc2990[0]   =   command;
      Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer_ltc2990,
                 1, LTC_I2C_ADDRESS);
      if (Status != XST_SUCCESS) {
         i2c_ltc2990++;
         DEBUG_I2C("[I2C] write2 error %d\n",i2c_ltc2990);
         return(1);
      }
      state=1;
      DEBUG_I2C("[I2C] write2 ok\n");
      break;
   case 1:
      Status=XIicPs_BusIsBusy(&IicInstance);
      if(Status==XST_SUCCESS) {
    	  state=2;
          DEBUG_I2C("[I2C] write slave:%02x cmd:%02x ok\n", LTC_I2C_ADDRESS, command);
      }
      DEBUG_I2C("[I2C] waiting bus busy (write (read command))\n");
      break;
   case 2:
      ReadBuffer_ltc2990[0]   =   0;
      ReadBuffer_ltc2990[1]   =   0;

      Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer_ltc2990,
                 2, LTC_I2C_ADDRESS);
      if (Status != XST_SUCCESS) {
         i2c_ltc2990++;
         printf("[I2C] read error %d\n",i2c_ltc2990);
         state=0;
         return(1);
      }
      state=3;
      DEBUG_I2C("[I2C] read ok\n");
      break;
   case 3:
      Status=XIicPs_BusIsBusy(&IicInstance);
      if(Status==XST_SUCCESS) {
         state=0;
         DEBUG_I2C("[I2C] write slave:%02x cmd:%02x data:%02x%02x ok\n", LTC_I2C_ADDRESS, command, ReadBuffer_ltc2990[0],ReadBuffer_ltc2990[1]);
         return(1);
      }
      DEBUG_I2C("[I2C] waiting bus busy (read)\n");
      break;
   default:
      DEBUG_I2C("[I2C] default2\n");
   }
   return(0);
}
void test_i2c(void)
{
   // Test read all slave addresses
   for (int i=0;i<0x7F;i++)
   {
      int Status;
      Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer_ltc2990, 1, i);
      if (Status != XST_SUCCESS)
      {
         printf("[I2C] 0x%02X read error\n",i);
      }
      else
      {
         while (XIicPs_BusIsBusy(&IicInstance));
         printf("[I2C] 0x%02X read OK!!!!!!!!!!!!!!!!\n",i);
      }
      usleep(25000);
   }
}

int ltc2990_init(void)
{
   // 0 celsuis, 1 single, 0 reserved, 11 all mode, 111 V1, V2, V3, V4
   usleep(2500);
   iic_write_ltc2990(LTC_CONTROL_REG,0b01011111); // V1, V2, V3, V4
   usleep(25000);

   printf("Init LTC2990\n");
/*
   // test LTC2990
   while(1)
   {
      int data;
      float value,dvalue,value3,value4;
      iic_write_ltc2990(LTC_CONTROL_REG,0b01011111); // V1, V2, V3, V4
      usleep(2500);
      iic_write_ltc2990(LTC_TRIGGER_REG,0); // any value trigger a conversion
      sleep(1);

      iic_read_ltc2990(LTC_TINT_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x1F)<<8) | ReadBuffer_ltc2990[1];
      // xli_printf does not support float...
      value=data*0.0625;
      dvalue=100*(value-(int)value)+0.5;
      printf("TINT_REG:= %d.%02d C\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V1_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      printf("V1_REG:= %d.%02d V\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V2_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      printf("V2_REG:= %d.%02d V\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V3_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6);
      dvalue=100*(value-(int)value)+0.5;
      printf("V3_REG:= %d.%02d V\n",(int)value,(int)dvalue);
      value3=value;

      iic_read_ltc2990(LTC_V4_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      printf("V4_REG:= %d.%02d V\n",(int)value,(int)dvalue);
      value4=value;

      // resistor calculation

      value=value3/(value4-value3)*9000.;
      printf("RESISTOR:= %d ohm\n",(int)value);

      iic_read_ltc2990(LTC_VCC_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=(2.5+data*305.18e-6);
      dvalue=100*(value-(int)value)+0.5;
      printf("VCC_REG:= %d.%02d\n",(int)value,(int)dvalue);
   }
*/
//   test_i2c();
   return(0);
}
