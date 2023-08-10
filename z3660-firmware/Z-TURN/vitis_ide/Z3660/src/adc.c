#include <stdio.h>
#include "xadcps.h"

// XADC adc converter instance
XAdcPs Xadc;

int xadc_init() {
	printf("[adc] xadc_init()...\n");
	XAdcPs_Config* cfg;
	cfg = XAdcPs_LookupConfig(XPAR_XADCPS_0_DEVICE_ID);
	XAdcPs_CfgInitialize(&Xadc, cfg, cfg->BaseAddress);
	int status = XAdcPs_SelfTest(&Xadc);
	printf("[adc] xadc_init() done: %d\n", status);
	return(status);
}

float xadc_get_temperature() {
	u16 raw = XAdcPs_GetAdcData(&Xadc, XADCPS_CH_TEMP);
	return XAdcPs_RawToTemperature(raw);
}

float xadc_get_aux_voltage() {
	u16 raw = XAdcPs_GetAdcData(&Xadc, XADCPS_CH_VCCAUX);
	return XAdcPs_RawToVoltage(raw);
}

float xadc_get_int_voltage() {
	u16 raw = XAdcPs_GetAdcData(&Xadc, XADCPS_CH_VCCINT);
	return XAdcPs_RawToVoltage(raw);
}
