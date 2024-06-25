/*
sc Page.c LINK LIB lib:reaction.lib NOSTACKCHECK
quit
*/

/**
 **  This is a simple example testing some of the capabilities
 **  of the clicktab and page layout gadget class.
 **
 **  Best viewed with TabSize = 2, or = 4.
 **/
#define UAETEST
//#define CPU_FREQ_STEP 5
//#define CPU_FREQ_THRESHOLD 3
#define CPU_FREQ_STEP 50
#define CPU_FREQ_THRESHOLD 25

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <graphics/gfxmacros.h>
#include <utility/tagitem.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <clib/alib_protos.h>
#include <clib/reaction_lib_protos.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/wb_protos.h>
#include <clib/icon_protos.h>
#include <clib/clicktab_protos.h>
#include <clib/label_protos.h>
#include <clib/layout_protos.h>
#include <clib/listbrowser_protos.h>
#include <clib/string_protos.h>
#include <clib/window_protos.h>
#include <clib/expansion_protos.h>

#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>
#include <gadgets/clicktab.h>
#include <images/label.h>
#include <gadgets/layout.h>
#include <gadgets/listbrowser.h>
#include <classes/window.h>

#include <pragmas/clicktab_pragmas.h>
#include <pragmas/label_pragmas.h>
#include <pragmas/layout_pragmas.h>
#include <pragmas/listbrowser_pragmas.h>
#include <pragmas/string_pragmas.h>
#include <pragmas/window_pragmas.h>

#include <gadgets/checkbox.h>
#include <proto/checkbox.h>

#include <gadgets/slider.h>
#include <proto/slider.h>

#include <gadgets/space.h>
#include <proto/space.h>

#include <clib/listview_protos.h>
#include <pragmas/listview_pragmas.h>
#include <gadgets/listview.h>

#include <gadgets/chooser.h>
#include <proto/chooser.h>

#include <stdio.h>
#include <stdlib.h>

#include "z3660_regs.h"
#include <string.h>

#define SPACE LAYOUT_AddChild, SpaceObject, End
#define LABEL_CENTERED(A) LAYOUT_AddChild, VLayoutObject,\
                             LAYOUT_AddChild, SpaceObject, SPACE_MinHeight,2,End,\
                             LAYOUT_AddImage, LabelObject, LABEL_Text,A,LABEL_VerticalSpacing,2, LabelEnd,\
                             LAYOUT_AddChild, SpaceObject, SPACE_MinHeight,0,End,\
                          LayoutEnd

#define ListViewObject NewObject(LISTVIEW_GetClass(), NULL

IMPORT struct Library *ButtonBase,
                      *CheckBoxBase,
                      *SliderBase,
                      *ClickTabBase,
                      *LabelBase,
                      *LayoutBase,
                      *ListBrowserBase,
                      *StringBase,
                      *WindowBase;

struct Library *ListViewBase;
struct Library* ExpansionBase;

struct List dlist;
struct List *kickstarts_list;
struct List *ext_kickstarts_list;

struct ConfigDev* zz_cd;
volatile UBYTE* zz_regs;

char txt_buf[64];

#define INFO_STR_WIDTH 50

enum
{
	GID_MAIN=0,
	GID_CLICKTAB,
	GID_PAGE,

	GID_ALIGN1,
	GID_ALIGN2,

	GID_ALIGNED_LEFT0,
	GID_ALIGNED_LEFT1,
	GID_ALIGNED_LEFT2,
	GID_ALIGNED_LEFT3,
	GID_ALIGNED_LEFT4,
	GID_ALIGNED_LEFT5,
	GID_ALIGNED_LEFT6,
	GID_ALIGNED_LEFT7,
	GID_ALIGNED_LEFT8,
	GID_ALIGNED_LEFT9,

	GID_PAGELAY1,
	GID_PAGELAY2,
	GID_PAGELAY3,

	GID_INFO_CPU_FREQ,
	GID_INFO_FWVER,
	GID_INFO_TEMP,
	GID_INFO_VAUX,
	GID_INFO_VINT,
	GID_INFO_LTC_TEMP,
	GID_INFO_LTC_V1,
	GID_INFO_LTC_V2,
	GID_INFO_LTC_060_TEMP,
	GID_INFO_BTN_TEST,
	GID_INFO_BTN_REFRESH,
	GID_INFO_JIT,
	GID_INFO_LPF,
	
	GID_BOOT_CPU_FREQ,
	GID_BOOT_LIST_BOOTMODE,
	GID_BOOT_SCSIBOOT,
	GID_BOOT_AUTOCONFIG_RAM,
	GID_BOOT_LIST_KICKSTART,
	GID_BOOT_LIST_EXT_KICKSTART,
	GID_BOOT_BTN_APPLY_BOOTMODE,

	GID_SCSI_LABEL,

	GID_QUIT,
	GID_LAST
};
struct Gadget *gadgets[GID_LAST];

enum
{
	WID_MAIN=0,
	WID_LAST
};
struct Window *windows[WID_LAST];

enum
{
	OID_MAIN=0,
	OID_LAST
};

enum {
	CPU,
	MUSASHI,
	UAE,
	UAEJIT,
	NUM_BOOTMODES
};
char bootmode_names[NUM_BOOTMODES][25]={
//	"XXXXXXXXXXXXXXXXXXX",
	"   060 real CPU    ",
	"  030 MUSASHI emu  ",
	"    040 UAE emu    ",
	"  040 UAE JIT emu  ",
};
struct ListLabelNode dnode[NUM_BOOTMODES];
#define KS_CHARS "012345678901234567890123456789"
char *kickstarts[] = {
	"0" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"1" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"2" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"3" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"4" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"5" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"6" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"7" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"8" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"9" KS_CHARS "\0" KS_CHARS KS_CHARS,
	NULL
};
char *ext_kickstarts[] = {
	"00" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"01" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"02" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"03" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"04" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"05" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"06" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"07" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"08" KS_CHARS "\0" KS_CHARS KS_CHARS,
	"09" KS_CHARS "\0" KS_CHARS KS_CHARS,
	NULL
};
/* Try opening the class library from a number of common places */
struct Library *openclass (STRPTR name, ULONG version)
{
 // struct ExecBase *SysBase = (*((struct ExecBase **) 4));
    struct Library *retval;
    UBYTE buffer[256];

    if ((retval = OpenLibrary (name, version)) == NULL)
    {
		sprintf (buffer, ":classes/%s", name);
		if ((retval = OpenLibrary (buffer, version)) == NULL)
		{
		    sprintf (buffer, "classes/%s", name);
		    retval = OpenLibrary (buffer, version);
		}
    }
    return retval;
}
void errorMessage(char* error)
{
	if (error) printf("Error: %s\n", error);
}

uint32_t zz_get_reg(uint32_t offset)
{
	return *((volatile uint32_t*)(zz_regs+offset));
}

void zz_set_reg(uint32_t offset, uint32_t value)
{
	*((volatile uint32_t*)(zz_regs+offset)) = value;
}

double zz_get_temperature(void)
{
	double temp = (double)(zz_get_reg(REG_ZZ_TEMPERATURE));
	return temp*0.100;
}

double zz_get_voltage_aux(void)
{
	double vaux = (double)(zz_get_reg(REG_ZZ_VOLTAGE_AUX));
	return vaux*0.010;
}

double zz_get_voltage_int(void)
{
	double vint = (double)(zz_get_reg(REG_ZZ_VOLTAGE_INT));
	return vint*0.010;
}
double zz_get_ltc_temperature(void)
{
	double ltc_temp = (double)(zz_get_reg(REG_ZZ_LTC_TEMP));
	return ltc_temp*0.010;
}
double zz_get_ltc_v1(void)
{
	double ltc_v1 = (double)(zz_get_reg(REG_ZZ_LTC_V1));
	return ltc_v1*0.010;
}
double zz_get_ltc_v2(void)
{
	double ltc_v2 = (double)(zz_get_reg(REG_ZZ_LTC_V2));
	return ltc_v2*0.010;
}
double zz_get_ltc_060_temperature(void)
{
	double ltc_060_temp = (double)(zz_get_reg(REG_ZZ_LTC_060_TEMP));
	return ltc_060_temp*0.010;
}

uint32_t zz_get_jit_enable(void)
{
	return zz_get_reg(REG_ZZ_JIT_ENABLE);
}

uint32_t zz_get_scsiboot_enable(void)
{
	return zz_get_reg(REG_ZZ_SCSIBOOT_EN);
}

uint32_t zz_get_autoconfig_ram_enable(void)
{
	return zz_get_reg(REG_ZZ_AUTOC_RAM_EN);
}

uint32_t zz_get_emulation_used(void)
{
	return zz_get_reg(REG_ZZ_EMULATION_USED);
}
/* ax is always preset :)
uint32_t zz_get_ax_present(void)
{
	return zz_get_reg(REG_ZZ_AUDIO_CONFIG);
}
*/
uint32_t zz_get_cpu_freq(void)
{
	return 	zz_get_reg(REG_ZZ_CPU_FREQ);
}

uint32_t zz_get_selected_bootmode(void)
{
	return zz_get_reg(REG_ZZ_BOOTMODE);
}
uint32_t zz_get_selected_kickstart(void)
{
	return zz_get_reg(REG_ZZ_KS_SEL);
}
uint32_t zz_get_selected_ext_kickstart(void)
{
	return zz_get_reg(REG_ZZ_EXT_KS_SEL);
}

uint32_t zz_get_usb_status(void)
{
	return zz_get_reg(REG_ZZ_USB_STATUS);
}

uint32_t zz_get_usb_capacity(void)
{
	return zz_get_reg(REG_ZZ_USB_CAPACITY);
}

void zz_set_jit_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_JIT_ENABLE, !!enable);
}

void zz_set_scsiboot_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_SCSIBOOT_EN, !!enable);
}

void zz_set_autoconfig_ram_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_AUTOC_RAM_EN, !!enable);
}

void zz_set_lpf_freq(uint16_t freq)
{
	zz_set_reg(REG_ZZ_AUDIO_PARAM, 9);
	zz_set_reg(REG_ZZ_AUDIO_VAL, freq);
//	zz_set_reg(REG_ZZ_AUDIO_PARAM, 0);
}

void zz_set_cpu_freq(uint16_t freq)
{
	zz_set_reg(REG_ZZ_CPU_FREQ, freq);
}

void zz_set_selected_bootmode(uint16_t bm)
{
	zz_set_reg(REG_ZZ_BOOTMODE, bm);
}

void zz_set_selected_kickstart(uint16_t ks)
{
	zz_set_reg(REG_ZZ_KS_SEL, ks);
}
void zz_set_selected_ext_kickstart(uint16_t ks)
{
	zz_set_reg(REG_ZZ_EXT_KS_SEL, ks);
}
void zz_set_selected_kickstart_txt(uint16_t ks)
{
	zz_set_reg(REG_ZZ_KS_SEL_TXT, ks);
}
void zz_set_selected_ext_kickstart_txt(uint16_t ks)
{
	zz_set_reg(REG_ZZ_EXT_KS_SEL_TXT, ks);
}

void zz_set_apply_bootmode(void)
{
	int kickstart=0,ext_kickstart=0;
	GetAttrs((Object *)gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, &kickstart, TAG_END);
	zz_set_selected_kickstart(kickstart);
	GetAttrs((Object *)gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, &ext_kickstart, TAG_END);
	zz_set_selected_ext_kickstart(ext_kickstart);
	zz_set_reg(REG_ZZ_APPLY_BOOTMODE, 0x55AA);
}
typedef struct {
	float m;
	float m_old;
	float m_filt;
} Measure;
Measure t,vaux,vint,ltc_temp,ltc_v1,ltc_v2,ltc_060_temp;
void filter(Measure *measure)
{
	if (measure->m_old==0.)
		measure->m_filt=measure->m;
	else
		measure->m_filt=0.1*measure->m+0.9*measure->m_old;
	measure->m_old=measure->m_filt;
}
void init_measures(void)
{
	t.m_old=0;
	vaux.m_old=0;
	vint.m_old=0;
	ltc_temp.m_old=0;
	ltc_v1.m_old=0;
	ltc_v2.m_old=0;
	ltc_060_temp.m_old=0;

}
unsigned int num_kickstarts=0,num_ext_kickstarts=0;
void refresh_zz_info(void)
{
	int i;
	int emulation_used;
	int jit_enable;
	int cpu_freq;
	int bootmode;
	int scsiboot;
	int autoconfig_ram;
	int kickstart;
	int ext_kickstart;
	
	uint32_t fwrev = zz_get_reg(REG_ZZ_FW_VERSION);

	int fwrev_major = fwrev>>8;
	int fwrev_minor = fwrev&0xff;
	t.m = zz_get_temperature();
	vaux.m = zz_get_voltage_aux();
	vint.m = zz_get_voltage_int();
	ltc_temp.m = zz_get_ltc_temperature();
	ltc_v1.m = zz_get_ltc_v1();
	ltc_v2.m = zz_get_ltc_v2();
	ltc_060_temp.m = zz_get_ltc_060_temperature();
	emulation_used = zz_get_emulation_used();
	jit_enable = zz_get_jit_enable();
	cpu_freq=zz_get_cpu_freq();
	bootmode=zz_get_selected_bootmode();
	scsiboot=zz_get_scsiboot_enable();
	autoconfig_ram=zz_get_autoconfig_ram_enable();
	kickstart=zz_get_selected_kickstart();

	for(i=0;i<10;i++)
	{
		int j=0;
		zz_set_selected_kickstart_txt(i);
		while(1)
		{
			uint32_t data=*((volatile uint32_t*)(zz_regs+REG_ZZ_SEL_KS_TXT+j));
			char hh=data>>24;
			char hl=data>>16;
			char lh=data>>8;
			char ll=data;
			kickstarts[i][j++]=hh;
			if(hh==0)
				break;
			kickstarts[i][j++]=hl;
			if(hl==0)
				break;
			kickstarts[i][j++]=lh;
			if(lh==0)
				break;
			kickstarts[i][j++]=ll;
			if(ll==0)
				break;
		}
		if(j==1)
		{
			num_kickstarts=i;
			kickstarts[i][1]='\0';
			kickstarts[i][2]='\0';
			kickstarts[i][3]='\0';
			i++;
			kickstarts[i][0]='\0';
			kickstarts[i][1]='\0';
			kickstarts[i][2]='\0';
			kickstarts[i][3]='\0';
			break;
		}
	}
	kickstarts[10][0]='\0';
	kickstarts[10][1]='\0';
	kickstarts[10][2]='\0';
	kickstarts[10][3]='\0';

	ext_kickstart=zz_get_selected_ext_kickstart();

	for(i=0;i<10;i++)
	{
		int j=0;
		zz_set_selected_ext_kickstart_txt(i);
		while(1)
		{
			uint32_t data=*((volatile uint32_t*)(zz_regs+REG_ZZ_SEL_KS_TXT+j));
			char hh=data>>24;
			char hl=data>>16;
			char lh=data>>8;
			char ll=data;
			ext_kickstarts[i][j++]=hh;
			if(hh==0)
				break;
			ext_kickstarts[i][j++]=hl;
			if(hl==0)
				break;
			ext_kickstarts[i][j++]=lh;
			if(lh==0)
				break;
			ext_kickstarts[i][j++]=ll;
			if(ll==0)
				break;
		}
		if(j==1)
		{
			num_ext_kickstarts=i;
			ext_kickstarts[i][1]='\0';
			ext_kickstarts[i][2]='\0';
			ext_kickstarts[i][3]='\0';
			i++;
			ext_kickstarts[i][0]='\0';
			ext_kickstarts[i][1]='\0';
			ext_kickstarts[i][2]='\0';
			ext_kickstarts[i][3]='\0';
			break;
		}
	}
	ext_kickstarts[10][0]='\0';
	ext_kickstarts[10][1]='\0';
	ext_kickstarts[10][2]='\0';
	ext_kickstarts[10][3]='\0';

	filter(&t);
	filter(&vaux);
	filter(&vint);
	filter(&ltc_temp);
	filter(&ltc_v1);
	filter(&ltc_v2);
	filter(&ltc_060_temp);

	sprintf(txt_buf, "%d", (int)cpu_freq);
	SetAttrs(gadgets[GID_INFO_CPU_FREQ], STRINGA_TextVal, txt_buf, TAG_END);
	SetAttrs(gadgets[GID_BOOT_CPU_FREQ], SLIDER_Level, cpu_freq, TAG_END);
	
	sprintf(txt_buf, "%d.%02d", fwrev_major, fwrev_minor);
	SetAttrs(gadgets[GID_INFO_FWVER], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.1f", t.m_filt);
	SetAttrs(gadgets[GID_INFO_TEMP], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.2f", vaux.m_filt);
	SetAttrs(gadgets[GID_INFO_VAUX], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.2f", vint.m_filt);
	SetAttrs(gadgets[GID_INFO_VINT], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.1f", ltc_temp.m_filt);
	SetAttrs(gadgets[GID_INFO_LTC_TEMP], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.2f", ltc_v1.m_filt);
	SetAttrs(gadgets[GID_INFO_LTC_V1], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.2f", ltc_v2.m_filt);
	SetAttrs(gadgets[GID_INFO_LTC_V2], STRINGA_TextVal, txt_buf, TAG_END);

	sprintf(txt_buf, "%.1f", ltc_060_temp.m_filt);
	SetAttrs(gadgets[GID_INFO_LTC_060_TEMP], STRINGA_TextVal, txt_buf, TAG_END);

	if (emulation_used) {
		SetAttrs(gadgets[GID_INFO_JIT], CHECKBOX_Checked, jit_enable, TAG_END);
	} else {
		SetAttrs(gadgets[GID_INFO_JIT], CHECKBOX_Checked, FALSE, TAG_END);
	}
	
	if (scsiboot) {
		SetAttrs(gadgets[GID_BOOT_SCSIBOOT], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_BOOT_SCSIBOOT], CHECKBOX_Checked, FALSE, TAG_END);
	}
	
	if (autoconfig_ram) {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RAM], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RAM], CHECKBOX_Checked, FALSE, TAG_END);
	}

	SetAttrs(gadgets[GID_BOOT_LIST_BOOTMODE], LISTBROWSER_Selected, bootmode, TAG_END);
	SetAttrs(gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, kickstart,
	                                           CHOOSER_MaxLabels, num_kickstarts,
	                                           TAG_END);
	SetAttrs(gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, ext_kickstart,
	                                               CHOOSER_MaxLabels, num_ext_kickstarts,
	                                               TAG_END);
}

ULONG zz_perform_memtest(uint32_t offset)
{
	uint32_t errors=0;
	volatile uint32_t* bufferl = (volatile uint32_t*)((uint32_t)zz_cd->cd_BoardAddr+offset);
	volatile uint16_t* bufferw = (volatile uint16_t*)bufferl;
	uint32_t i = 0;
	uint32_t rep=1024*256;

	printf("zz_perform_memtest...\n");

	for (i=0; i<rep; i++) {
		uint32_t v2 = 0;
		uint32_t v = (i%2)?0xaaaa5555:0x33337777;
		uint16_t v4 = 0;
		uint16_t v3 = (i%2)?0xffff:0x0000;

		if ((i % (32*1024)) == 0) {
			printf("`-- Test %lx %6ld/%ld...\n", (uint32_t)zz_cd->cd_BoardAddr+offset, i, rep);
		}

		bufferl[i] = v;
		v2 = bufferl[i];

		if (v!=v2) {
			printf("32-bit mismatch at %lx: 0x%lx should be 0x%lx\n",(uint32_t)&bufferl[i],v2,v);
			errors++;
		}

		bufferw[i] = v3;
		v4 = bufferw[i];

		if (v3!=v4) {
			printf("16-bit mismatch at %lx: 0x%x should be 0x%x\n",(uint32_t)&bufferw[i],v4,v3);
			errors++;
		}
	}
	printf("Done. %ld errors.\n", errors);
	return errors;
}

ULONG zz_perform_memtest_rand(uint32_t offset, int rep)
{
	uint32_t errors=0;
	int i,k;
	uint16_t* tbuf;
	const int sz = 16;
	volatile uint16_t* buffer = (volatile uint16_t*)((uint32_t)zz_cd->cd_BoardAddr+offset);

	printf("zz_perform_memtest_rand...\n");

	tbuf = malloc(2*sz);
	if (!tbuf) {
		printf("Error: Could not allocate memory for test buffer\n");
		return 1;
	}

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test %lx %3d/%d...\n", (uint32_t)zz_cd->cd_BoardAddr+offset, k, rep);
		}
		// step 1: fill buffer with random data
		for (i=0; i<sz; i++) {
			tbuf[sz] = rand();
		}

		buffer[0] = tbuf[0];
		buffer[1] = tbuf[1];
		buffer[2] = tbuf[2];
		buffer[3] = tbuf[3];
		buffer[4] = tbuf[4];
		buffer[5] = tbuf[5];
		buffer[6] = tbuf[6];
		buffer[7] = tbuf[7];
		buffer[8] = tbuf[8];
		buffer[9] = tbuf[9];
		buffer[10] = tbuf[10];
		buffer[11] = tbuf[11];
		buffer[12] = tbuf[12];
		buffer[13] = tbuf[13];
		buffer[14] = tbuf[14];
		buffer[15] = tbuf[15];

		for (i=0; i<sz; i++) {
			uint16_t v = buffer[i];
			if (v != tbuf[i]) {
				if (errors<100) printf("Mismatch at %lx: 0x%x should be 0x%x\n",(uint32_t)&buffer[i],v,tbuf[i]);
				errors++;
			}
		}
	}

	free(tbuf);

	printf("Done. %ld errors.\n", errors);
	return errors;
}

ULONG zz_perform_memtest_cross(uint32_t offset, int rep)
{
	uint32_t errors=0;
	int i,k;
	const int sz = 16;
	volatile uint32_t * buffer32 = (volatile uint32_t *)(offset);
	volatile uint16_t * buffer16 = (volatile uint16_t *)(offset);
	volatile uint8_t  * buffer8  = (volatile uint8_t  *)(offset);

	printf("zz_perform_memtest... address 0x%lx write32 read32\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer32)+i;
			uint32_t value = rand();
			uint32_t v;
			// write32
			*((uint32_t *)dir)=value;
			//read32
			v = *((uint32_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%08lx should be 0x%08lx\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write32 read16\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer32)+i;
			uint32_t value = rand();
			uint16_t vlow;
			uint16_t vhigh;
			uint32_t v;
			// write32
			*((uint32_t *)dir)=value;
			//read16
			vlow  = *((uint16_t *)dir+1);
			vhigh = *((uint16_t *)dir);
			v=(vhigh<<16)|vlow;
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%08lx should be 0x%08lx\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write32 read8\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer32)+i;
			uint32_t value = rand();
			uint32_t vhh,vlh,vhl,vll,v;
			// write32
			*((uint32_t *)dir)=value;
			//read8
			vhh = *((uint8_t *)dir);
			vlh = *((uint8_t *)dir+2);
			vhl = *((uint8_t *)dir+1);
			vll = *((uint8_t *)dir+3);
			v=(vhh<<24)|(vhl<<16)|(vlh<<8)|vll;
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%08lx should be 0x%08lx\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write16 read32\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer16)+i;
			uint16_t value1 = rand();
			uint16_t value2 = rand();
			uint32_t value=(value1<<16)|value2;
			uint32_t v;
			// write16
			*((uint16_t *)dir+1)=value2;
			*((uint16_t *)dir)=value1;
			//read32
			v = *((uint32_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%08lx should be 0x%08lx\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write16 read16\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer16)+i;
			uint16_t value = rand();
			uint16_t v;
			// write16
			*((uint16_t *)dir)=value;
			//read16
			v = *((uint16_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%04x should be 0x%04x\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write16 read8\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer16)+i;
			uint16_t value = rand();
			uint8_t vl,vh;
			uint16_t v;
			// write16
			*((uint16_t *)dir)=value;
			//read8
			vl = *((uint8_t *)dir+1);
			vh = *((uint8_t *)dir);
			v=(vh<<8)|vl;
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%04x should be 0x%04x\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write8 read32\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer8)+i;
			uint8_t value1 = rand();
			uint8_t value2 = rand();
			uint8_t value3 = rand();
			uint8_t value4 = rand();
			uint32_t value=(value1<<24)|(value2<<16)|(value3<<8)|value4;
			uint32_t v;
			// write8
			*((uint8_t *)dir)=value1;
			*((uint8_t *)dir+2)=value3;
			*((uint8_t *)dir+1)=value2;
			*((uint8_t *)dir+3)=value4;
			//read32
			v = *((uint32_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%08lx should be 0x%08lx\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write8 read16\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer8)+i;
			uint8_t value1 = rand();
			uint8_t value2 = rand();
			uint16_t value=(value1<<8)|value2;
			uint16_t v;
			// write8
			*((uint8_t *)dir+1)=value2;
			*((uint8_t *)dir)=value1;
			//read16
			v = *((uint16_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%04x should be 0x%04x\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("zz_perform_memtest... address 0x%lx write8 read8\n",offset);

	for (k = 0; k < rep; k++) {
		if ((k % 128) == 0) {
			printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
		}

		for (i=0; i<sz; i++) {
			uint32_t dir=(uint32_t)(buffer8)+i;
			uint8_t value = rand();
			uint8_t v;
			// write8
			*((uint8_t *)dir)=value;
			//read16
			v = *((uint8_t *)dir);
			if (v != value) {
				if (errors<100) printf("Mismatch at %lx: 0x%02x should be 0x%02x\n",(uint32_t)dir,v,value);
				errors++;
			}
		}
	}

	printf("Done. %ld errors.\n", errors);
	return errors;
}

ULONG zz_perform_memtest_fpgareg(void) {
	int i;
	volatile uint16_t* d1 = (volatile uint16_t*)((uint32_t)zz_cd->cd_BoardAddr+0x1030);
	volatile uint16_t* d2 = (volatile uint16_t*)((uint32_t)zz_cd->cd_BoardAddr+0x1034);
	volatile uint16_t* dr = (volatile uint16_t*)((uint32_t)zz_cd->cd_BoardAddr+0x1030);

	printf("zz_perform_memtest_fpgareg...\n");

	*d2 = 1;
	for (i = 0; i < 0x100000*2; i++) {
		*d1 = i;
	}

	printf("Done. Result: %x\n", *dr);

	return 0;
}

ULONG zz_perform_memtest_multi(void) {
	uint32_t offset = 0x07100000;
	uint32_t errors=0;
	errors+=zz_perform_memtest(offset);
	errors+=zz_perform_memtest_rand(offset, 1024);
	printf("Testing CPU access to Z3660 Memory...\n");
	errors+=zz_perform_memtest_cross((uint32_t)zz_cd->cd_BoardAddr+offset, 1024);
	offset = 0x00100000;
	printf("Testing CPU access to CHIP...\n");
	errors+=zz_perform_memtest_cross(offset, 1024);
	//zz_perform_memtest_fpgareg();
	printf("Bus Test finished with %ld total errors\n",errors);

	return 0;
}
VOID handleGadgetEvent(int gad, UWORD code)
{
	switch (gad)
	{
		case GID_INFO_BTN_REFRESH: {
			refresh_zz_info();
			break;
		}
		case GID_INFO_BTN_TEST: {
			zz_perform_memtest_multi();
			break;
		}
		case GID_INFO_JIT: {
			if(zz_get_emulation_used())
				zz_set_jit_enabled(code);
			else
				SetAttrs(gadgets[GID_INFO_JIT], CHECKBOX_Checked, FALSE, TAG_END);
//				refresh_zz_info();
			break;
		}
		case GID_INFO_LPF: {
			zz_set_lpf_freq(code);
			break;
		}
		case GID_BOOT_CPU_FREQ: {
			int inc=(code%CPU_FREQ_STEP)>=CPU_FREQ_THRESHOLD?CPU_FREQ_STEP:0;
			code=(code/CPU_FREQ_STEP)*CPU_FREQ_STEP;
			code+=inc;
//			sprintf(txt_buf, "%d", (int)code);
//			SetAttrs(gadgets[GID_INFO_CPU_FREQ], STRINGA_TextVal, txt_buf, TAG_END);

			SetGadgetAttrs(gadgets[GID_BOOT_CPU_FREQ],windows[WID_MAIN], NULL,
                                                      SLIDER_Level, code, TAG_DONE);
			zz_set_cpu_freq(code);
			break;
		}
		case GID_BOOT_LIST_BOOTMODE: {
			zz_set_selected_bootmode(code);
			break;
		}
		case GID_BOOT_LIST_KICKSTART: {
			if(code>=num_kickstarts)
				SetAttrs(gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, 0,
				                                           CHOOSER_MaxLabels, num_kickstarts,
				                                           TAG_END);
			break;
		}
		case GID_BOOT_LIST_EXT_KICKSTART: {
			if(code>=num_ext_kickstarts)
				SetAttrs(gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, 0,
				                                               CHOOSER_MaxLabels, num_ext_kickstarts,
				                                               TAG_END);
			break;
		}
		case GID_BOOT_BTN_APPLY_BOOTMODE: {
			zz_set_apply_bootmode();
			break;
		}
		case GID_BOOT_SCSIBOOT: {
			zz_set_scsiboot_enabled(code);
			break;
		}
		case GID_BOOT_AUTOCONFIG_RAM: {
			zz_set_autoconfig_ram_enabled(code);
			break;
		}
	}
}

int main(void)
{
	int i;
	int bootmode=0;
	int kickstart=0;
	int ext_kickstart=0;
	struct MsgPort *AppPort;

	Object *objects[OID_LAST];

	if (!(ExpansionBase = (struct Library*)OpenLibrary((CONST_STRPTR)"expansion.library",0L))) {
		errorMessage("Requires expansion.library");
		return 0;
	}

	zz_cd = (struct ConfigDev*)FindConfigDev(zz_cd,0x144B,0x1);
	if (!zz_cd) {
		CloseLibrary(ExpansionBase);
#ifndef UAETEST
		errorMessage("Z3660 not found.\n");
		return 0;
#endif
	}

	zz_regs = (UBYTE*)zz_cd->cd_BoardAddr;
	CloseLibrary(ExpansionBase);

	NewList(&dlist);
	
	kickstarts_list = ChooserLabelsA(kickstarts);
	ext_kickstarts_list = ChooserLabelsA(ext_kickstarts);

	ListViewBase = openclass ("gadgets/listview.gadget", 0L);
	/* special case - reference buttonbase to make sure it autoinit!
	 */
	if ( !ButtonBase || !ListViewBase)
		return(30);
	else if ( AppPort = CreateMsgPort() )
	{
		struct List *tablabels = ClickTabs("Info","Boot","SCSI", NULL);

		if (tablabels)
		{
			struct List CustomersList, OrdersList, DetailsList;

			NewList(&CustomersList);
			NewList(&OrdersList);
			NewList(&DetailsList);

			/* Create the window object.
			 */
			objects[OID_MAIN] = WindowObject,
//				WA_ScreenTitle, "Ztop 1.03",
				WA_Title, "Z3660 ZTop 1.03",
				WA_Activate, TRUE,
				WA_DepthGadget, TRUE,
				WA_DragBar, TRUE,
				WA_CloseGadget, TRUE,
				WA_SizeGadget, FALSE,
				WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
				          IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
				          BUTTONIDCMP,
				WINDOW_IconifyGadget, TRUE,
				WINDOW_IconTitle, "Z3660 Ztop",
				WINDOW_AppPort, AppPort,
				WINDOW_Position, WPOS_CENTERSCREEN,
				WINDOW_ParentGroup, gadgets[GID_MAIN] = VGroupObject,
					LAYOUT_DeferLayout, TRUE,
					LAYOUT_SpaceOuter, TRUE,
					LAYOUT_SpaceInner, TRUE,

					LAYOUT_AddChild,  gadgets[GID_CLICKTAB] = ClickTabObject,
						GA_ID, GID_CLICKTAB,
						GA_RelVerify, TRUE,
						CLICKTAB_Labels, tablabels,

						/* Embed the PageObject "inside" the Clicktab
						 * the clicktab's beveling will surround the page.
						 */
						CLICKTAB_PageGroup, gadgets[GID_PAGE] = PageObject,
							/* We will defer layout/render changing pages! */
							LAYOUT_DeferLayout, TRUE,

							PAGE_Add, gadgets[GID_PAGELAY1] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT0] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_CPU_FREQ] = StringObject,
										GA_ID, GID_INFO_CPU_FREQ,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"CPU Frequency", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("MHz"),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT1] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_FWVER] = StringObject,
										GA_ID, GID_INFO_FWVER,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"Firmware Version", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("   "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT2] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_TEMP] = StringObject,
										GA_ID, GID_INFO_TEMP,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"FPGA Core Temp", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("C  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT3] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_VAUX] = StringObject,
										GA_ID, GID_INFO_VAUX,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"Aux Voltage", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("V  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT4] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_VINT] = StringObject,
										GA_ID, GID_INFO_VINT,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"Core Voltage", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("V  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT5] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_LTC_TEMP] = StringObject,
										GA_ID, GID_INFO_LTC_TEMP,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"LTC Temp", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("C  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT6] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_LTC_V1] = StringObject,
										GA_ID, GID_INFO_LTC_V1,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"LTC (3V3) Vdd", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("V  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT7] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_LTC_V2] = StringObject,
										GA_ID, GID_INFO_LTC_V2,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"LTC (5V) Vcc", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("V  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT8] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_LTC_060_TEMP] = StringObject,
										GA_ID, GID_INFO_LTC_060_TEMP,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, "",
										STRINGA_Justification, GACT_STRINGCENTER,
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"LTC (060 THERM)", LabelEnd,
									CHILD_MinWidth, INFO_STR_WIDTH,
									LABEL_CENTERED("C  "),
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_ALIGNED_LEFT9] = HLayoutObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_JIT] = CheckBoxObject,
										GA_ID, GID_INFO_JIT,
										GA_RelVerify, TRUE,
										GA_Text, "JIT Enabled",
										CHECKBOX_TextPlace, PLACETEXT_LEFT,
									CheckBoxEnd,
									CHILD_WeightedHeight, 0,
									SPACE,
								LayoutEnd,

								LAYOUT_AddChild, gadgets[GID_INFO_LPF] = SliderObject,
									GA_ID, GID_INFO_LPF,
									GA_RelVerify, TRUE,
                    				SLIDER_Min,          0,
                    				SLIDER_Max,      23900,
                    				SLIDER_Level,    23900,
                    				SLIDER_Orientation,  SLIDER_HORIZONTAL,
                    				SLIDER_LevelPlace,   PLACETEXT_ABOVE,
                    				SLIDER_LevelMaxLen,  10,
//                    SLIDER_LevelJustify, SLJ_CENTER,
                    				SLIDER_LevelFormat,  "%ld Hz",
                    				SLIDER_Ticks,        25,
				                    SLIDER_ShortTicks,   TRUE,
								SliderEnd,
				                Label("Audio Lowpass Filter"),
								CHILD_WeightedHeight, 0,

								SPACE,

								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_BTN_TEST] = ButtonObject,
										GA_ID, GID_INFO_BTN_TEST,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Bus Test",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_BTN_REFRESH] = ButtonObject,
										GA_ID, GID_INFO_BTN_REFRESH,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Refresh",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

							PageEnd,

							PAGE_Add, gadgets[GID_PAGELAY2] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

								LAYOUT_AddChild, gadgets[GID_BOOT_CPU_FREQ] = SliderObject,
									GA_ID, GID_BOOT_CPU_FREQ,
									GA_RelVerify, TRUE,
                    				SLIDER_Min,       50,
                    				SLIDER_Max,      100,
                    				SLIDER_Level,    100,
                    				SLIDER_Orientation,  SLIDER_HORIZONTAL,
                    				SLIDER_LevelPlace,   PLACETEXT_ABOVE,
                    				SLIDER_LevelMaxLen,  10,
									SLIDER_KnobDelta,     CPU_FREQ_STEP,
//                    SLIDER_LevelJustify, SLJ_CENTER,
                    				SLIDER_LevelFormat,  "%ld MHz",
                    				SLIDER_Ticks,        11,
				                    SLIDER_ShortTicks,   TRUE,
								SliderEnd,
				                Label("CPU Frequency"),
								CHILD_WeightedHeight, 0,

								SPACE,

								LAYOUT_AddChild, VGroupObject,
									LAYOUT_AddChild,  HGroupObject,
										SPACE,
										LAYOUT_AddImage, LabelObject,
							            	LABEL_Text,  "Boot Mode",
	            						LabelEnd,  // Label
										SPACE,
									LayoutEnd,
									LAYOUT_AddChild,  HGroupObject,
										SPACE,
            							LAYOUT_AddChild, gadgets[GID_BOOT_LIST_BOOTMODE] = ListBrowserObject,
											GA_ID, GID_BOOT_LIST_BOOTMODE,
											GA_RelVerify, TRUE,
											LISTBROWSER_Labels, &dlist,
											LISTBROWSER_ShowSelected, TRUE,
											LISTBROWSER_VerticalProp, FALSE,
											LISTBROWSER_AutoFit,		TRUE,
										LayoutEnd,
										CHILD_MinWidth, 20*8,
										CHILD_MaxWidth, 20*8,
										CHILD_MinHeight, 9*NUM_BOOTMODES,
										CHILD_MaxHeight, 9*NUM_BOOTMODES,
										SPACE,
									LayoutEnd,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								LAYOUT_AddChild, gadgets[GID_ALIGN1] = HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_BOOT_SCSIBOOT] = CheckBoxObject,
										GA_ID, GID_BOOT_SCSIBOOT,
										GA_RelVerify, TRUE,
										GA_TabCycle, TRUE,
										STRINGA_MaxChars, 24,
										GA_Text, "SCSI BOOT enabled",
										CHECKBOX_TextPlace, PLACETEXT_LEFT,
									CheckBoxEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								LAYOUT_AddChild, gadgets[GID_ALIGN2] = HGroupObject,
									SPACE,
//									LAYOUT_BevelStyle, BVS_SBAR_VERT,
//									LAYOUT_TopSpacing, 2,

									LAYOUT_AddChild, gadgets[GID_BOOT_AUTOCONFIG_RAM] = CheckBoxObject,
										GA_ID, GID_BOOT_AUTOCONFIG_RAM,
										GA_RelVerify, TRUE,
										GA_TabCycle, TRUE,
										STRINGA_MaxChars, 48,
										GA_Text, "AUTOC RAM enabled",
										CHECKBOX_TextPlace, PLACETEXT_LEFT,
									CheckBoxEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddImage, LabelObject,
									LABEL_Text,"Kickstart",
									LabelEnd,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								LAYOUT_AddChild, HGroupObject,
									LAYOUT_SpaceInner, TRUE,
									LAYOUT_AddChild, gadgets[GID_BOOT_LIST_KICKSTART] = ChooserObject,
										GA_ID, GID_BOOT_LIST_KICKSTART,
										GA_RelVerify, TRUE,
										CHOOSER_Labels, kickstarts_list,
										CHOOSER_Selected, kickstart,
										CHOOSER_PopUp, TRUE,
										CHOOSER_DropDown, FALSE,
										CHOOSER_AutoFit, FALSE,
									ChooserEnd,
								LayoutEnd,
								CHILD_WeightedHeight, 0,
								
								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddImage, LabelObject,
									LABEL_Text,"Ext Kickstart",
									LabelEnd,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								LAYOUT_AddChild, HGroupObject,
									LAYOUT_SpaceInner, TRUE,
									LAYOUT_AddChild, gadgets[GID_BOOT_LIST_EXT_KICKSTART] = ChooserObject,
										GA_ID, GID_BOOT_LIST_EXT_KICKSTART,
										GA_RelVerify, TRUE,
										CHOOSER_Labels, ext_kickstarts_list,
										CHOOSER_Selected, ext_kickstart,
										CHOOSER_PopUp, TRUE,
										CHOOSER_DropDown, FALSE,
										CHOOSER_AutoFit, FALSE,
									ChooserEnd,
								LayoutEnd,
								CHILD_WeightedHeight, 0,
								
								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_BOOT_BTN_APPLY_BOOTMODE] = ButtonObject,
										GA_ID, GID_BOOT_BTN_APPLY_BOOTMODE,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply Boot Mode",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

							PageEnd,


							PAGE_Add, gadgets[GID_PAGELAY3] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

								LAYOUT_AddChild, HLayoutObject,
									LAYOUT_AddChild, gadgets[GID_SCSI_LABEL] = StringObject,
										GA_ID, GID_SCSI_LABEL,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										STRINGA_TextVal, " Under Construction... ",
									StringEnd,
									CHILD_Label, LabelObject, LABEL_Text,"SCSI", LabelEnd,
									CHILD_MinWidth, 200,
									SPACE,
								LayoutEnd,
							PageEnd,


						ClickTabEnd,
					LayoutEnd,

					SPACE,
					LAYOUT_AddChild, HGroupObject,

						SPACE,
						LAYOUT_AddChild, ButtonObject,
							GA_ID, GID_QUIT,
							GA_RelVerify, TRUE,
							GA_Text,"_Quit",
						ButtonEnd,
						CHILD_WeightedHeight, 0,
						CHILD_WeightedWidth, 0,
						SPACE,
					LayoutEnd,

				EndGroup,
			EndWindow;

	 	 	/*  Object creation sucessful?
	 	 	 */
			if (objects[OID_MAIN])
			{
				/* Set up inter-group label pagement.
				 */
				SetAttrs( gadgets[GID_ALIGN1], LAYOUT_AlignLabels, gadgets[GID_ALIGN2], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGN2], LAYOUT_AlignLabels, gadgets[GID_ALIGN1], TAG_DONE);

				SetAttrs( gadgets[GID_ALIGNED_LEFT0], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT1], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT1], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT2], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT2], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT3], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT3], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT4], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT4], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT5], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT5], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT6], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT6], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT7], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT7], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT8], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT8], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT9], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGNED_LEFT9], LAYOUT_AlignLabels, gadgets[GID_ALIGNED_LEFT0], TAG_DONE);

				for(i=0;i<NUM_BOOTMODES;i++)
				{
					LBAddNode(gadgets[GID_BOOT_LIST_BOOTMODE], NULL, NULL, (struct Node *)~0,
								LBNCA_CopyText, TRUE,
								LBNCA_Text, bootmode_names[i],
								TAG_DONE);
				}
				SetAttrs(gadgets[GID_BOOT_LIST_BOOTMODE], LISTBROWSER_Selected, bootmode, TAG_END);
				SetAttrs(gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, kickstart, TAG_END);
				SetAttrs(gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, ext_kickstart, TAG_END);

				init_measures();
				refresh_zz_info();
//				RefreshWindow(mywin, NULL);

				/*  Open the window.
				 */
				if (windows[WID_MAIN] = (struct Window *) RA_OpenWindow(objects[OID_MAIN]))
				{
					ULONG wait, signal, app = (1L << AppPort->mp_SigBit);
					ULONG done = FALSE;
					ULONG result;
					UWORD code;

				 	/* Obtain the window wait signal mask.
					 */
					GetAttr(WINDOW_SigMask, objects[OID_MAIN], &signal);

					/* Activate the first string gadget!
					 */
//					ActivateLayoutGadget( gadgets[GID_MAIN], windows[WID_MAIN], NULL, gadgets[GID_COMPANY] );

					/* Input Event Loop
					 */
					while (!done)
					{
						wait = Wait( signal | SIGBREAKF_CTRL_C | app );

						if ( wait & SIGBREAKF_CTRL_C )
						{
							done = TRUE;
						}
						else
						{
							while ( (result = RA_HandleInput(objects[OID_MAIN], &code) ) != WMHI_LASTMSG )
							{
								switch (result & WMHI_CLASSMASK)
								{
									case WMHI_CLOSEWINDOW:
										windows[WID_MAIN] = NULL;
										done = TRUE;
										break;

									case WMHI_MOUSEMOVE:
									case WMHI_GADGETUP:
										switch (result & WMHI_GADGETMASK)
										{
/*											case GID_COMPANY:
											//	printf( "Company: %s\n", ((struct StringInfo *)(gadgets[GID_COMPANY]->SpecialInfo))->Buffer);
												break;
*/
											case GID_QUIT:
												done = TRUE;
												break;
											default:
												handleGadgetEvent(result & WMHI_GADGETMASK, code);
										}
										break;

									case WMHI_ICONIFY:
										RA_Iconify(objects[OID_MAIN]);
										windows[WID_MAIN] = NULL;
										break;

									case WMHI_UNICONIFY:
										windows[WID_MAIN] = (struct Window *) RA_OpenWindow(objects[OID_MAIN]);

										if (windows[WID_MAIN])
										{
											GetAttr(WINDOW_SigMask, objects[OID_MAIN], &signal);
										}
										else
										{
											done = TRUE;	// error re-opening window!
										}
									 	break;
								}
							}
						}
					}
				}

				/* Disposing of the window object will also close the window if it is
				 * already opened, and it will dispose of the layout object attached to it.
				 */
				DisposeObject(objects[OID_MAIN]);
			}

			/* Free the click tab label list.
			 */ 
			FreeClickTabs(tablabels);
			FreeChooserLabels(kickstarts_list);
			FreeChooserLabels(ext_kickstarts_list);
		}

		/* close/free the application port.
		 */
		DeleteMsgPort(AppPort);
	}
//			free_listview_list(dlist);
	CloseLibrary ((struct Library *) ListViewBase);

	return(0);
}
