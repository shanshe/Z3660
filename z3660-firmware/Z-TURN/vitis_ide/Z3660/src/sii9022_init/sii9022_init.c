/*
 * sii9022_init.c
 *
 *  Created on: 2017Äê8ÔÂ24ÈÕ
 *      Author: pgsimple
 */

#include "xil_types.h"
#include "sii9022_init.h"
#include "xiicps.h"
//#include "xparameters.h"

#include "sleep.h"
XIicPs IicInstance;		/* The instance of the IIC device. */

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID

uint8_t WriteBuffer[1 + 1];

uint8_t ReadBuffer[1];	/* Read buffer for reading a page. */

struct sensor_register {
	uint8_t addr;
	uint8_t value;
};

struct sensor_register sii9022_init_regs[] = {

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
//I2S config

	//0x26
	{SII902X_TPI_AUDIO_CONFIG_BYTE2_REG,SII902X_TPI_AUDIO_MUTE_ENABLE
                                       |SII902X_TPI_AUDIO_CODING_PCM
			                           |SII902X_TPI_AUDIO_INTERFACE_I2S},
	//0x20
	{SII902X_TPI_I2S_INPUT_CONFIG_REG  ,SII902X_TPI_I2S_FIRST_BIT_SHIFT_NO
			                           |SII902X_TPI_I2S_SD_DIRECTION_MSB_FIRST
									   |SII902X_TPI_I2S_SD_JUSTIFY_LEFT
									   |SII902X_TPI_I2S_WS_POLARITY_LOW
			                           |SII902X_TPI_I2S_MCLK_MULTIPLIER_384
	                                   |SII902X_TPI_I2S_SCK_EDGE_RISING},
	//0x21
	{0x21,0x00},
	//0x22
	{0x22,0x00},
	//0x23
	{0x23,0x20},
	//0x24
	{0x24,0x02},
	//0x25
	{0x25,0x02},

	{SII902X_TPI_AUDIO_CONFIG_BYTE2_REG,SII902X_TPI_AUDIO_MUTE_DISABLE
			                           |SII902X_TPI_AUDIO_CODING_PCM
			                           |SII902X_TPI_AUDIO_INTERFACE_I2S},

	{SII902X_TPI_I2S_ENABLE_MAPPING_REG,SII902X_TPI_I2S_CONFIG_FIFO0
                                       |SII902X_TPI_I2S_SELECT_SD0
                                       |SII902X_TPI_I2S_FIFO_ENABLE},

	{SII902X_TPI_AUDIO_CONFIG_BYTE3_REG,SII902X_TPI_AUDIO_FREQ_48KHZ
			                           |SII902X_TPI_AUDIO_SAMPLE_SIZE_16},

	{0xff, 0xff}, /*over */
};


int iic_write_8(uint8_t a,uint8_t data)
{
	int Status;

	WriteBuffer[0]	=	a;
	WriteBuffer[1]	=	data;

	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
					  2, 0x3b);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	while ((Status=XIicPs_BusIsBusy(&IicInstance)));

	usleep(2500);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	WriteBuffer[0]	=	a;

	Status = XIicPs_MasterSendPolled(&IicInstance, WriteBuffer,
					  1, 0x3b);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	while ((Status=XIicPs_BusIsBusy(&IicInstance)));

	usleep(2500);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	Status = XIicPs_MasterRecvPolled(&IicInstance, ReadBuffer,
					  1, 0x3b);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}
	while (XIicPs_BusIsBusy(&IicInstance));
//	xil_printf("0x%02x=0x%02x\r\n",a,ReadBuffer[0]);
	return(XST_SUCCESS);
}

int iic_master_init(void)
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	ConfigPtr = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return(XST_FAILURE);
	}

	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	XIicPs_SetSClk(&IicInstance, 400000);

	return(XST_SUCCESS);
}
void set_hdmi_video_mode(uint16_t htotal, uint16_t vtotal, uint32_t pixelclock_hz, uint16_t vhz, uint8_t hdmi)
{
	sii9022_init_regs[ 7].value=(uint8_t)(pixelclock_hz/10000);
	sii9022_init_regs[ 8].value=(uint8_t)((pixelclock_hz/10000)>>8);
	sii9022_init_regs[ 9].value=(uint8_t)(vhz*100);
	sii9022_init_regs[10].value=(uint8_t)((vhz*100)>>8);
	sii9022_init_regs[11].value=(uint8_t)(htotal);
	sii9022_init_regs[12].value=(uint8_t)(htotal>>8);
	sii9022_init_regs[13].value=(uint8_t)(vtotal);
	sii9022_init_regs[14].value=(uint8_t)(vtotal>>8);
	sii9022_init_regs[16].value=hdmi;
}
int sii9022_init(zz_video_mode *vmode)
{
	int i;
	i=0;

	/* ------------------------------------------------------------ */
	/*					sii9022 hardware reset   					*/
	/* ------------------------------------------------------------ */

	set_hdmi_video_mode(vmode->hres,vmode->vres,vmode->phz, 60, 1);

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

	return(0);
}
