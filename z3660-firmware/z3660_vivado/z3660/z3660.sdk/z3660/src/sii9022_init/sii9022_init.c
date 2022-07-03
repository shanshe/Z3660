/*
 * sii9022_init.c
 *
 *  Created on: 2017Äê8ÔÂ24ÈÕ
 *      Author: pgsimple
 */

#include "xil_types.h"
#include "xiicps.h"

#include "sleep.h"
XIicPs IicInstance;		/* The instance of the IIC device. */

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID

u8 WriteBuffer[1 + 1];

u8 ReadBuffer[1];	/* Read buffer for reading a page. */

struct sensor_register {
	u8 addr;
	u8 value;
};

static const struct sensor_register sii9022_init_regs[] = {

	{0x1e, 0x00},
	{0x08, 0x70},

	{0x09, 0x00},
	{0x0a, 0x00},

	{0x60, 0x04},
	{0x3c, 0x01},

	{0x1a, 0x11},

	{0x00, 0x02},
	{0x01, 0x3a},
	{0x02, 0x70},
	{0x03, 0x17},
	{0x04, 0x98},
	{0x05, 0x08},
	{0x06, 0x65},
	{0x07, 0x04},
	{0x08, 0x70},
	{0x1a, 0x01},

	{0xff, 0xff}, /*over */
};


int iic_write_8(u8 a,u8 data)
{
	int Status;

	WriteBuffer[0]	=	a;
	WriteBuffer[1]	=	data;

	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
					  2, 0x3b);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&IicInstance));

	usleep(2500);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	WriteBuffer[0]	=	a;

	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
					  1, 0x3b);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&IicInstance));

	usleep(2500);

	Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer,
					  1, 0x3b);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	while (XIicPs_BusIsBusy(&IicInstance));
//	xil_printf("0x%02x=0x%02x\r\n",a,ReadBuffer[0]);
	return 0;
}

int iic_master_init(void)
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	ConfigPtr = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&IicInstance, 400000);

	return XST_SUCCESS;
}

int sii9022_init(void)
{
	int i;
	i=0;

	/* ------------------------------------------------------------ */
	/*					sii9022 hardware reset   					*/
	/* ------------------------------------------------------------ */

	iic_master_init();
	iic_write_8(0xc7,0x00);//software reset

	while(1)
	{
		if(sii9022_init_regs[i].addr==0xff)
			break;
		iic_write_8(sii9022_init_regs[i].addr,sii9022_init_regs[i].value);
		i++;
	}

	xil_printf("Init SII9022\n\r");

	return 0;
}
