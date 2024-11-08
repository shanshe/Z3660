/*
 * sii9022_init.c
 *
 *  Created on: 2017Äê8ÔÂ24ÈÕ
 *      Author: pgsimple
 */

#include "xil_types.h"
#include "sii9022_init.h"
#include "xiicps.h"
#include <stdio.h>
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

	{SII9022_POWER_STATE_CTRL_REG, 0x00},
	{SII9022_PIXEL_REPETITION_REG, 0x70},

	{SII9022_AVI_IN_FORMAT_REG,    0x00},
	{SII9022_AVI_OUT_FORMAT_REG,   0x00},

	{0x60, 0x04},
	{SII9022_IRQ_ENABLE_REG,       0x01},

	{SII9022_SYS_CTRL_DATA_REG,    0x11},

	{SII9022_PIXEL_CLK_LSB_REG,    0x02},
	{SII9022_PIXEL_CLK_MSB_REG,    0x3a},
	{SII9022_VFREQ_LSB_REG,        0x70},
	{SII9022_VFREQ_MSB_REG,        0x17},
	{SII9022_PIXELS_LSB_REG,       0x98},
	{SII9022_PIXELS_MSB_REG,       0x08},
	{SII9022_LINES_LSB_REG,        0x65},
	{SII9022_LINES_MSB_REG,        0x04},
	{SII9022_PIXEL_REPETITION_REG, 0x70},

	{SII9022_SYS_CTRL_DATA_REG,    0x01},
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

	{SII902X_TPI_I2S_STRM_HDR_0_REG, 0x04},
	{SII902X_TPI_I2S_STRM_HDR_1_REG, 0x00},
	{SII902X_TPI_I2S_STRM_HDR_2_REG, 0x20},
	{SII902X_TPI_I2S_STRM_HDR_3_REG, 0x22},
	{SII902X_TPI_I2S_STRM_HDR_4_REG, 0x02},

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
//	printf("0x%02x=0x%02x\n",a,ReadBuffer[0]);
	return(XST_SUCCESS);
}

#define I2C_FREQ 400000

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

	XIicPs_SetSClk(&IicInstance, I2C_FREQ);

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

	set_hdmi_video_mode(vmode->hres,vmode->vres,vmode->phz, vmode->vhz, vmode->hdmi);

	iic_master_init();
	iic_write_8(0xc7,0x00);//software reset

	while(1)
	{
		if(sii9022_init_regs[i].addr==0xff)
			break;
		iic_write_8(sii9022_init_regs[i].addr,sii9022_init_regs[i].value);
		i++;
	}

	printf("Init SII9022\n");

	return(0);
}
