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
extern XIicPs IicInstance;      /* The instance of the IIC device. */

#define IIC_DEVICE_ID   XPAR_XIICPS_0_DEVICE_ID

uint8_t WriteBuffer_ltc2990[2];

uint8_t ReadBuffer_ltc2990[2];   /* Read buffer for reading a page. */

int iic_write_ltc2990(uint8_t command,uint8_t data)
{
   int Status;

   WriteBuffer_ltc2990[0]   =   command;
   WriteBuffer_ltc2990[1]   =   data;

   Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer_ltc2990,
                 2, LTC_I2C_ADDRESS);
   if (Status != XST_SUCCESS) {
      xil_printf("[I2C] write error\r\n");
      return(XST_FAILURE);
   }

   while (XIicPs_BusIsBusy(&IicInstance));

   return(XST_SUCCESS);
}
int iic_read_ltc2990(uint8_t command)
{
   int Status;

   WriteBuffer_ltc2990[0]   =   command;


   Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer_ltc2990,
                 1, LTC_I2C_ADDRESS);
   if (Status != XST_SUCCESS) {
      xil_printf("[I2C] write2 error\r\n");
      return(XST_FAILURE);
   }

   while ((Status=XIicPs_BusIsBusy(&IicInstance)));

   usleep(2500);
   if (Status != XST_SUCCESS) {
      xil_printf("[I2C] bus busy error\r\n");
      return(XST_FAILURE);
   }

   ReadBuffer_ltc2990[0]   =   0;
   ReadBuffer_ltc2990[1]   =   0;

   Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer_ltc2990,
                 2, LTC_I2C_ADDRESS);
   if (Status != XST_SUCCESS) {
      xil_printf("[I2C] read error\r\n");
      return(XST_FAILURE);
   }
   while (XIicPs_BusIsBusy(&IicInstance));
//   xil_printf("0x%02x=0x%02x\r\n",a,ReadBuffer[0]);
   return(XST_SUCCESS);
}

int ltc2990_init(void)
{
   // 0 celsuis, 1 single, 0 reserved, 11 all mode, 111 V1, V2, V3, V4
   usleep(2500);
   iic_write_ltc2990(LTC_CONTROL_REG,0b01011111);
   usleep(25000);

   xil_printf("Init LTC2990\n\r");
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
      xil_printf("TINT_REG:= %d.%02d C\r\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V1_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      xil_printf("V1_REG:= %d.%02d V\r\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V2_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      xil_printf("V2_REG:= %d.%02d V\r\n",(int)value,(int)dvalue);

      iic_read_ltc2990(LTC_V3_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6);
      dvalue=100*(value-(int)value)+0.5;
      xil_printf("V3_REG:= %d.%02d V\r\n",(int)value,(int)dvalue);
      value3=value;

      iic_read_ltc2990(LTC_V4_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=data*(305.8e-6)*10.;
      dvalue=100*(value-(int)value)+0.5;
      xil_printf("V4_REG:= %d.%02d V\r\n",(int)value,(int)dvalue);
      value4=value;

      // resistor calculation

      value=value3/(value4-value3)*9000.;
      xil_printf("RESISTOR:= %d ohm\r\n",(int)value);

      iic_read_ltc2990(LTC_VCC_MSB);
      data = ((ReadBuffer_ltc2990[0]&0x7F)<<8) | ReadBuffer_ltc2990[1];
      value=(2.5+data*305.18e-6);
      dvalue=100*(value-(int)value)+0.5;
      xil_printf("VCC_REG:= %d.%02d\r\n",(int)value,(int)dvalue);
   }
*/
/*   // Test read all slave addresses
   for (int i=0;i<0x7F;i++)
   {
	   int Status;

	   Status = XIicPs_MasterRecvPolled(&IicInstance2, ReadBuffer_ltc2990, 1, i);
	   if (Status != XST_SUCCESS)
	   {
	      xil_printf("[I2C] 0x%02X read error\r\n",i);
	      return(XST_FAILURE);
	   }
	   else
	   {
		   while (XIicPs_BusIsBusy(&IicInstance2));
		   xil_printf("[I2C] 0x%02X read OK!!!!!!!!!!!!!!!!\r\n",i);
	   }
	   usleep(25000);
   }
*/
   return(0);
}
