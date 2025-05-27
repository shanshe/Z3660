/*
sc Page.c LINK LIB lib:reaction.lib NOSTACKCHECK
quit
*/

//#define UAETEST
#define CPU_FREQ_STEP 5
#define CPU_FREQ_THRESHOLD 3
//#define CPU_FREQ_STEP 50
//#define CPU_FREQ_THRESHOLD 25

#define Z3660_ZTOP_VERSION_MAJOR "Z3660 ZTop 1.03"
#define Z3660_ZTOP_VERSION_MINOR 18 // BETA number (0 = full version, no beta)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/gadtools.h>
#include <pragmas/gadtools_pragmas.h>
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

#include <gadgets/chooser.h>
#include <proto/chooser.h>

#include <stdio.h>
#include <stdlib.h>

#include "z3660_regs.h"
#include <string.h>

void refresh_zz_info(struct Window *win);

#define SPACE LAYOUT_AddChild, SpaceObject, End

IMPORT struct Library *ButtonBase,
                      *CheckBoxBase,
                      *SliderBase,
                      *ClickTabBase,
                      *LabelBase,
                      *LayoutBase,
                      *ListBrowserBase,
                      *StringBase,
                      *WindowBase,
                      *GadToolsBase;

struct Library* ExpansionBase;

struct List dlist;
struct List *kickstarts_list;
struct List *ext_kickstarts_list;
struct List *scsis_list;
int scsi[7]={0,0,0,0,0,0,0};

struct ConfigDev* zz_cd;
volatile UBYTE* zz_regs;

char txt_buf[64];
char window_title[64]="";

#define INFO_STR_WIDTH 60
#define PRESET_STR_WIDTH 50

enum
{
	GID_MAIN=0,
	GID_CLICKTAB,
	GID_PAGE,

	GID_ALIGN1,
	GID_ALIGN2_1,
	GID_ALIGN2_2,
	GID_ALIGN3_1,
	GID_ALIGN3_2,
	GID_ALIGN3_3,
	GID_ALIGN4_1,
	GID_ALIGN4_2,
	GID_ALIGN4_3,
	GID_ALIGN5,
	GID_ALIGN6_1,
	GID_ALIGN6_2,
	GID_ALIGN6_3,

	GID_PAGELAY1,
	GID_PAGELAY2,
	GID_PAGELAY3,
	GID_PAGELAY4,
	GID_PAGELAY5,

	GID_INFO_CPU_FREQ,
	GID_INFO_FWVER,
	GID_INFO_TEMP,
	GID_INFO_VAUX,
	GID_INFO_VINT,
	GID_INFO_LTC_TEMP,
	GID_INFO_LTC_V1,
	GID_INFO_LTC_V2,
	GID_INFO_LTC_060_TEMP,
	GID_INFO_BTN_HARDREBOOT,
	GID_INFO_BTN_TEST,
	GID_INFO_BTN_REFRESH,
	GID_INFO_JIT,
	GID_INFO_LPF,
	GID_INFO_MON_SWITCH_CTS,
	GID_INFO_MON_SWITCH_SEL,
	GID_INFO_CTS_ACT_LEVEL,
	GID_INFO_SEL_ACT_LEVEL,
	
	GID_BOOT_CPU_FREQ,
	GID_BOOT_LIST_BOOTMODE,
	GID_BOOT_SCSIBOOT,
	GID_BOOT_ENABLETEST,
	GID_CPURAM_ENABLE,
	GID_MOUNT_SD_0x76,
	GID_MOUNT_SD_ROOT,
	GID_BOOT_AUTOCONFIG_RAM,
	GID_BOOT_AUTOCONFIG_RTG,
	GID_BOOT_LIST_KICKSTART,
	GID_BOOT_LIST_EXT_KICKSTART,
	GID_BOOT_BTN_APPLY_BOOTMODE,
	GID_BOOT_BTN_APPLY_ALL,

	GID_SCSI0,
	GID_SCSI1,
	GID_SCSI2,
	GID_SCSI3,
	GID_SCSI4,
	GID_SCSI5,
	GID_SCSI6,
	GID_SCSI_BTN_APPLY_SCSI,
	GID_SCSI_BTN_APPLY_ALL,

	GID_MISC_MAC,
	GID_MISC_BPTON,
	GID_MISC_BPTOFF,
	GID_MISC_BTN_APPLY_MISC,
	GID_MISC_BTN_APPLY_ALL,


	GID_PRESET0,
	GID_PRESET1,
	GID_PRESET2,
	GID_PRESET3,
	GID_PRESET4,
	GID_PRESET5,
	GID_PRESET6,
	GID_PRESET7,
	GID_PRESET8,
	GID_PRESET_CB0,
	GID_PRESET_CB1,
	GID_PRESET_CB2,
	GID_PRESET_CB3,
	GID_PRESET_CB4,
	GID_PRESET_CB5,
	GID_PRESET_CB6,
	GID_PRESET_CB7,
	GID_PRESET_CB8,
	GID_PRESET_BTN_APPLY_PRESET,
	GID_PRESET_BTN_DELETE_PRESET,
	GID_PRESET_BTN_APPLY_ALL,


	GID_LAST
};
#define REFRESH_COUNT 15
int table_refresh[REFRESH_COUNT]={
	GID_INFO_CPU_FREQ,
	GID_INFO_FWVER,
	GID_INFO_TEMP,
	GID_INFO_VAUX,
	GID_INFO_VINT,
	GID_INFO_LTC_TEMP,
	GID_INFO_LTC_V1,
	GID_INFO_LTC_V2,
	GID_INFO_LTC_060_TEMP,
	GID_INFO_JIT,
	GID_INFO_LPF,
	GID_INFO_MON_SWITCH_CTS,
	GID_INFO_MON_SWITCH_SEL,
	GID_INFO_CTS_ACT_LEVEL,
	GID_INFO_SEL_ACT_LEVEL,
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
	UAE_030,
	UAEJIT_030,
	UAE_040,
	UAEJIT_040,
	NUM_BOOTMODES
};
char bootmode_names[NUM_BOOTMODES][25]={
//	"XXXXXXXXXXXXXXXXXXX",
	"060 real CPU   ",
	"030 MUSASHI emu",
	"030 UAE emu    ",
	"030 UAE JIT emu",
	"040 UAE emu    ",
	"040 UAE JIT emu",
};

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
#define SCSI_CHARS "012345678901234567890123456789"
char *scsis[] = {
	"Disabled" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 0" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 1" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 2" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 3" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 4" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 5" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 6" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 7" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 8" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	" 9" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"10" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"11" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"12" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"13" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"14" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"15" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"16" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"17" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"18" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	"19" SCSI_CHARS "\0" SCSI_CHARS SCSI_CHARS,
	NULL
};
#define PRESET_CHARS "012345678901234567890123456789"
char *presets[] = {
	" 0 " PRESET_CHARS,
	" 1 " PRESET_CHARS,
	" 2 " PRESET_CHARS,
	" 3 " PRESET_CHARS,
	" 4 " PRESET_CHARS,
	" 5 " PRESET_CHARS,
	" 6 " PRESET_CHARS,
	" 7 " PRESET_CHARS,
	NULL
};

struct NewMenu mainmenu[] =
{
   { NM_TITLE, "Project " , 0  , 0, 0, 0},
   { NM_ITEM , "About..." , "?", 0, 0, 0},
   { NM_ITEM , NM_BARLABEL, 0  , 0, 0, 0},
   { NM_ITEM , "Quit..."  , "Q", 0, 0, 0},
   { NM_END  , ""         , 0  , 0, 0, 0}
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
void Pop(char *fmt)
{
	struct IntuitionBase *IntuitionBase;
	if ((IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0))) {
		struct EasyStruct es;

		es.es_StructSize = sizeof(struct EasyStruct);
		es.es_Flags = 0ul;
		es.es_Title = "Z3660 ZTop";
		es.es_TextFormat = (UBYTE *)fmt;
		es.es_GadgetFormat = "Ok";

		EasyRequestArgs(NULL, &es, NULL, NULL);
		CloseLibrary((struct Library *)IntuitionBase);
	}
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

uint32_t zz_get_test_enable(void)
{
	return zz_get_reg(REG_ZZ_TEST_ENABLE);
}

uint32_t zz_get_cpuram_enable(void)
{
	return zz_get_reg(REG_ZZ_CPU_RAM_EN);
}

uint32_t zz_get_mount_sd_0x76(void)
{
	return zz_get_reg(REG_ZZ_MOUNT_SD_0x76);
}

uint32_t zz_get_mount_sd_root(void)
{
	return zz_get_reg(REG_ZZ_MOUNT_SD_ROOT);
}
uint32_t get_monitor_switch(void)
{
	uint32_t monitor_switch=0;
	int temp;
	GetAttrs((Object *)gadgets[GID_INFO_MON_SWITCH_CTS], CHECKBOX_Checked, &temp, TAG_END);
	monitor_switch|=temp?0x01:0;
	GetAttrs((Object *)gadgets[GID_INFO_MON_SWITCH_SEL], CHECKBOX_Checked, &temp, TAG_END);
	monitor_switch|=temp?0x02:0;
	GetAttrs((Object *)gadgets[GID_INFO_CTS_ACT_LEVEL], CHECKBOX_Checked, &temp, TAG_END);
	monitor_switch|=temp?0x10:0;
	GetAttrs((Object *)gadgets[GID_INFO_SEL_ACT_LEVEL], CHECKBOX_Checked, &temp, TAG_END);
	monitor_switch|=temp?0x20:0;
	return(monitor_switch);
}
uint32_t zz_get_monitor_switch(void)
{
	return zz_get_reg(REG_ZZ_MONITOR_SWITCH);
}

uint32_t zz_get_autoconfig_ram_enable(void)
{
	return zz_get_reg(REG_ZZ_AUTOC_RAM_EN);
}

uint32_t zz_get_autoconfig_rtg_enable(void)
{
	return zz_get_reg(REG_ZZ_AUTOC_RTG_EN);
}

uint32_t zz_get_emulation_used(void)
{
	return zz_get_reg(REG_ZZ_EMULATION_USED);
}
/* ax is always present :)
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
#ifndef UAETEST
	return zz_get_reg(REG_ZZ_KS_SEL);
#else
	return(1);
#endif
}
uint32_t zz_get_selected_ext_kickstart(void)
{
#ifndef UAETEST
	return zz_get_reg(REG_ZZ_EXT_KS_SEL);
#else
	return(2);
#endif
}
uint32_t zz_get_selected_scsi(int scsi)
{
#ifndef UAETEST
	return zz_get_reg(REG_ZZ_SCSI_SEL_0+scsi*4);
#else
	return(scsi*2UL);
#endif
}
uint32_t zz_get_bpton(void)
{
#ifndef UAETEST
	return zz_get_reg(REG_ZZ_BPTON);
#else
	return(1);
#endif
}

uint32_t zz_get_bptoff(void)
{
#ifndef UAETEST
	return zz_get_reg(REG_ZZ_BPTOFF);
#else
	return(1);
#endif
}

void zz_get_mac(char *mac)
{
	uint32_t data_hi=zz_get_reg(REG_ZZ_ETH_MAC_HI);
	uint32_t data_lo=zz_get_reg(REG_ZZ_ETH_MAC_LO);
	sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",(data_hi>>8)&0xFF,(data_hi)&0xFF,(data_lo>>24)&0xFF,(data_lo>>16)&0xFF,(data_lo>>8)&0xFF,(data_lo)&0xFF);
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

void zz_set_test_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_TEST_ENABLE, !!enable);
}

void zz_set_cpuram_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_CPU_RAM_EN, !!enable);
}

void zz_set_mount_sd_0x76(uint16_t enable)
{
	zz_set_reg(REG_ZZ_MOUNT_SD_0x76, !!enable);
}

void zz_set_mount_sd_root(uint16_t enable)
{
	zz_set_reg(REG_ZZ_MOUNT_SD_ROOT, !!enable);
}

void zz_set_monitor_switch(uint16_t value)
{
	zz_set_reg(REG_ZZ_MONITOR_SWITCH, value);
}

void zz_set_autoconfig_ram_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_AUTOC_RAM_EN, !!enable);
}

void zz_set_autoconfig_rtg_enabled(uint16_t enable)
{
	zz_set_reg(REG_ZZ_AUTOC_RTG_EN, !!enable);
}

void zz_set_lpf_freq(uint16_t freq)
{
	zz_set_reg(REG_ZZ_AUDIO_PARAM, 9);
	zz_set_reg(REG_ZZ_AUDIO_VAL, freq);
//	zz_set_reg(REG_ZZ_AUDIO_PARAM, 0);
}
void zz_set_bpton(uint16_t value)
{
	zz_set_reg(REG_ZZ_BPTON, value);
}

void zz_set_bptoff(uint16_t value)
{
	zz_set_reg(REG_ZZ_BPTOFF, value);
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
void zz_set_selected_scsi(uint32_t index,uint16_t scsi)
{
	zz_set_reg(REG_ZZ_SCSI_SEL_0+(index<<2), scsi);
}
void zz_set_selected_preset(struct Window *win,int index)
{
	zz_set_reg(REG_ZZ_PRESET_SEL,index);
}

void zz_set_selected_kickstart_txt(uint16_t ks)
{
	zz_set_reg(REG_ZZ_KS_SEL_TXT, ks);
}
void zz_set_selected_ext_kickstart_txt(uint16_t ks)
{
	zz_set_reg(REG_ZZ_EXT_KS_SEL_TXT, ks);
}
void zz_set_selected_scsi_txt(uint16_t scsi)
{
	zz_set_reg(REG_ZZ_SCSI_SEL_TXT, scsi);
}
void zz_set_selected_preset_txt(int index)
{
	zz_set_reg(REG_ZZ_PRESET_SEL_TXT,index);
}

void zz_set_apply_bootmode(void)
{
	int bootmode;
	int cpufreq;
	int kickstart,ext_kickstart;
	int scsiboot;
	int test_enable;
	int cpuram_enable;
	int mount_sd_0x76;
	int mount_sd_root;
	int monitor_switch;
	int autoconfig_ram;
	int autoconfig_rtg;

	GetAttrs((Object *)gadgets[GID_BOOT_LIST_BOOTMODE], LISTBROWSER_Selected, &bootmode, TAG_END);
	zz_set_selected_bootmode(bootmode);
	GetAttrs((Object *)gadgets[GID_BOOT_CPU_FREQ], SLIDER_Level, &cpufreq, TAG_DONE);
	zz_set_cpu_freq(cpufreq);
	GetAttrs((Object *)gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, &kickstart, TAG_END);
	zz_set_selected_kickstart(kickstart);
	GetAttrs((Object *)gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, &ext_kickstart, TAG_END);
	zz_set_selected_ext_kickstart(ext_kickstart);

	GetAttrs((Object *)gadgets[GID_BOOT_SCSIBOOT], CHECKBOX_Checked, &scsiboot, TAG_END);
	zz_set_scsiboot_enabled(scsiboot);
	GetAttrs((Object *)gadgets[GID_BOOT_AUTOCONFIG_RAM], CHECKBOX_Checked, &autoconfig_ram, TAG_END);
	zz_set_autoconfig_ram_enabled(autoconfig_ram);
	GetAttrs((Object *)gadgets[GID_BOOT_AUTOCONFIG_RTG], CHECKBOX_Checked, &autoconfig_rtg, TAG_END);
	zz_set_autoconfig_rtg_enabled(autoconfig_rtg);
	GetAttrs((Object *)gadgets[GID_BOOT_ENABLETEST], CHECKBOX_Checked, &test_enable, TAG_END);
	zz_set_test_enabled(test_enable);
	GetAttrs((Object *)gadgets[GID_CPURAM_ENABLE], CHECKBOX_Checked, &cpuram_enable, TAG_END);
	zz_set_cpuram_enabled(cpuram_enable);
	GetAttrs((Object *)gadgets[GID_MOUNT_SD_0x76], CHECKBOX_Checked, &mount_sd_0x76, TAG_END);
	zz_set_mount_sd_0x76(mount_sd_0x76);
	GetAttrs((Object *)gadgets[GID_MOUNT_SD_ROOT], CHECKBOX_Checked, &mount_sd_root, TAG_END);
	zz_set_mount_sd_root(mount_sd_root);

	monitor_switch=get_monitor_switch();
	zz_set_monitor_switch(monitor_switch);
}
void zz_set_hardreboot(void)
{
	zz_set_reg(REG_ZZ_APPLY_BOOTMODE, 0x5A5A); // ARM hard reboot
}
void zz_set_apply_scsi(void)
{
	int scsi[7];
	int i;
	for(i=0;i<7;i++)
	{
		GetAttrs((Object *)gadgets[GID_SCSI0+i], CHOOSER_Selected, &scsi[i], TAG_END);
		zz_set_selected_scsi(i,scsi[i]);
	}
}
uint8_t hex2int(char c1, char c2)
{
	uint8_t data=0;
	if(c1>='a' && c1<'f') c1=c1-'a'+'A';
	if(c2>='a' && c2<'f') c2=c2-'a'+'A';

	if(c1>='0' && c1<='9')
		data|=(c1-'0')<<4;
	else if(c1>='A' && c1<='F')
		data|=(c1-'A'+10)<<4;

	if(c2>='0' && c2<='9')
		data|=c2-'0';
	else if(c2>='A' && c2<='F')
		data|=c2-'A'+10;
	return data;
}
void zz_set_apply_misc(void)
{
	int bpton,bptoff;
	char mac[18];
	uint32_t data;
	GetAttrs((Object *)gadgets[GID_MISC_MAC], SLIDER_Level, mac, TAG_END);
	data =((uint32_t)hex2int(mac[ 0],mac[ 1]))<<8;
	data|=((uint32_t)hex2int(mac[ 3],mac[ 4]));
	zz_set_reg(REG_ZZ_ETH_MAC_HI,data);
	data =((uint32_t)hex2int(mac[ 6],mac[ 7]))<<24;
	data|=((uint32_t)hex2int(mac[ 9],mac[10]))<<16;
	data|=((uint32_t)hex2int(mac[12],mac[13]))<<8;
	data|=((uint32_t)hex2int(mac[15],mac[16]));
	zz_set_reg(REG_ZZ_ETH_MAC_LO,data);

	GetAttrs((Object *)gadgets[GID_MISC_BPTON], SLIDER_Level, &bpton, TAG_END);
	zz_set_reg(REG_ZZ_BPTON,bpton);
	GetAttrs((Object *)gadgets[GID_MISC_BPTOFF], SLIDER_Level, &bptoff, TAG_END);
	zz_set_reg(REG_ZZ_BPTOFF,bptoff);
}
uint32_t zz_get_selected_preset(void)
{
	return zz_get_reg(REG_ZZ_PRESET_SEL);
}

void 
zz_set_apply_preset(void)
{
	STRPTR name=NULL;
	int preset=zz_get_selected_preset();
	switch(preset)
	{
		case 0:
			GetAttrs((Object *)gadgets[GID_PRESET0], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 1:
			GetAttrs((Object *)gadgets[GID_PRESET1], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 2:
			GetAttrs((Object *)gadgets[GID_PRESET2], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 3:
			GetAttrs((Object *)gadgets[GID_PRESET3], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 4:
			GetAttrs((Object *)gadgets[GID_PRESET4], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 5:
			GetAttrs((Object *)gadgets[GID_PRESET5], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 6:
			GetAttrs((Object *)gadgets[GID_PRESET6], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		case 7:
			GetAttrs((Object *)gadgets[GID_PRESET7], STRINGA_TextVal, (ULONG*)&name, TAG_END);
			break;
		default:
		case 8:
			name=NULL;
	}

	if(name!=NULL)
	{
//		printf("%s\n",name);
		strcpy((char *)(zz_regs+REG_ZZ_SEL_PRESET_TXT),name);
	}
}

void zz_set_apply_all(void)
{
	zz_set_apply_bootmode();
	zz_set_apply_scsi();
	zz_set_apply_misc();
	zz_set_apply_preset();

	zz_set_reg(REG_ZZ_APPLY_ALL, 0x55AA);
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
unsigned int num_scsis=0;
void refresh_zz_info(struct Window *win)
{
	int i;
	int emulation_used;
	int jit_enable;
	int cpu_freq;
	int bootmode;
	int scsiboot;
	int test_enable;
	int cpuram_enable;
	int mount_sd_0x76;
	int mount_sd_root;
	int monitor_switch;
	int autoconfig_ram;
	int autoconfig_rtg;
	int kickstart;
	int ext_kickstart;
	uint32_t beta,alfa;
	int bpton;
	int bptoff;
	char mac[18];
	int selected_preset;
	
	uint32_t fwrev = zz_get_reg(REG_ZZ_FW_VERSION);

	int fwrev_major = fwrev>>8;
	int fwrev_minor = fwrev&0xff;
#ifdef UAETEST
	SetWindowTitles(win,Z3660_ZTOP_VERSION_MAJOR " UAETEST",(CONST_STRPTR)-1);
#else
	beta = zz_get_reg(REG_ZZ_FW_BETA);
	if(beta)
	{
		alfa = zz_get_reg(REG_ZZ_FW_ALFA);
		if(alfa)
		{
			sprintf(window_title,Z3660_ZTOP_VERSION_MAJOR " (BETA %d ALFA %d)", beta, alfa);
		}
		else
		{
			sprintf(window_title,Z3660_ZTOP_VERSION_MAJOR " (BETA %d FIRMWARE DETECTED)", beta);
		}
		SetWindowTitles(win,window_title,(CONST_STRPTR)-1);
		if(beta>Z3660_ZTOP_VERSION_MINOR)
		{
			static BOOL advice_shown=FALSE;
			if(advice_shown==FALSE)
			{
				Pop("You should update this ZTop app from the adf supplied with the BOOT.BIN (FPGA firmware)");
				advice_shown=TRUE;
			}
		}
		else if(beta<Z3660_ZTOP_VERSION_MINOR)
		{
			static BOOL advice_shown=FALSE;
			if(advice_shown==FALSE)
			{
				Pop("You should update the BOOT.BIN (FPGA firmware) before using this ZTop app");
				advice_shown=TRUE;
			}
		}
	}
#endif
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
	test_enable=zz_get_test_enable();
	cpuram_enable=zz_get_cpuram_enable();
	mount_sd_0x76=zz_get_mount_sd_0x76();
	mount_sd_root=zz_get_mount_sd_root();
	monitor_switch=zz_get_monitor_switch();
	autoconfig_ram=zz_get_autoconfig_ram_enable();
	autoconfig_rtg=zz_get_autoconfig_rtg_enable();
	kickstart=zz_get_selected_kickstart();
	ext_kickstart=zz_get_selected_ext_kickstart();
	for(i=0;i<7;i++)
		scsi[i]=zz_get_selected_scsi(i)+1;
	bpton=zz_get_bpton();
	bptoff=zz_get_bptoff();
	zz_get_mac(mac);
	selected_preset=(int)zz_get_selected_preset();

#ifndef UAETEST
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
//	kickstarts[10][0]='\0';
//	kickstarts[10][1]='\0';
//	kickstarts[10][2]='\0';
//	kickstarts[10][3]='\0';
#else
	num_kickstarts=8;
//	kickstarts[8][0]='\0';
//	kickstarts[8][1]='\0';
//	kickstarts[8][2]='\0';
//	kickstarts[8][3]='\0';
#endif

#ifndef UAETEST
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
//	ext_kickstarts[10][0]='\0';
//	ext_kickstarts[10][1]='\0';
//	ext_kickstarts[10][2]='\0';
//	ext_kickstarts[10][3]='\0';
#else
	num_ext_kickstarts=5;
//	ext_kickstarts[5][0]='\0';
//	ext_kickstarts[5][1]='\0';
//	ext_kickstarts[5][2]='\0';
//	ext_kickstarts[5][3]='\0';
#endif
	scsis[0][0]='D';
	scsis[0][1]='i';
	scsis[0][2]='s';
	scsis[0][3]='a';
	scsis[0][4]='b';
	scsis[0][5]='l';
	scsis[0][6]='e';
	scsis[0][7]='d';
	scsis[0][8]='\0';
	scsis[0][9]='\0';
	scsis[0][10]='\0';
	scsis[0][11]='\0';
#ifndef UAETEST
	for(i=1;i<21;i++)
	{
		int j=0;
		zz_set_selected_scsi_txt(i-1);
		while(1)
		{
			uint32_t data=*((volatile uint32_t*)(zz_regs+REG_ZZ_SEL_SCSI_TXT+j));
			char hh=data>>24;
			char hl=data>>16;
			char lh=data>>8;
			char ll=data;
			scsis[i][j++]=hh;
			if(hh==0)
				break;
			scsis[i][j++]=hl;
			if(hl==0)
				break;
			scsis[i][j++]=lh;
			if(lh==0)
				break;
			scsis[i][j++]=ll;
			if(ll==0)
				break;
		}
		if(j==1)
		{
			num_scsis=i;
			scsis[i][1]='\0';
			scsis[i][2]='\0';
			scsis[i][3]='\0';
			i++;
			scsis[i][0]='\0';
			scsis[i][1]='\0';
			scsis[i][2]='\0';
			scsis[i][3]='\0';
			break;
		}
	}
//	scsis[21][0]='\0';
//	scsis[21][1]='\0';
//	scsis[21][2]='\0';
//	scsis[21][3]='\0';
#else
	num_scsis=20;
//	scsis[21][0]='\0';
//	scsis[21][1]='\0';
//	scsis[21][2]='\0';
//	scsis[21][3]='\0';
#endif

#ifndef UAETEST
	for(i=0;i<8;i++)
	{
		int j=0;
		int k;
		zz_set_selected_preset_txt(i);
		while(1)
		{
			uint32_t data=*((volatile uint32_t*)(zz_regs+REG_ZZ_SEL_PRESET_TXT+j));
			char hh=data>>24;
			char hl=data>>16;
			char lh=data>>8;
			char ll=data;
			presets[i][j++]=hh;
			if(hh==0)
				break;
			presets[i][j++]=hl;
			if(hl==0)
				break;
			presets[i][j++]=lh;
			if(lh==0)
				break;
			presets[i][j++]=ll;
			if(ll==0)
				break;
			if(j>48)
			break;
		}
		for(k=j;k<4;k++)
			presets[i][k]='\0';
	}
//	presets[8][0]='\0';
//	presets[8][1]='\0';
//	presets[8][2]='\0';
//	presets[8][3]='\0';
#else
//	presets[8][0]='\0';
//	presets[8][1]='\0';
//	presets[8][2]='\0';
//	presets[8][3]='\0';
#endif



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
	
	if (test_enable) {
		SetAttrs(gadgets[GID_BOOT_ENABLETEST], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_BOOT_ENABLETEST], CHECKBOX_Checked, FALSE, TAG_END);
	}
	
	if (cpuram_enable) {
		SetAttrs(gadgets[GID_CPURAM_ENABLE], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_CPURAM_ENABLE], CHECKBOX_Checked, FALSE, TAG_END);
	}
	
	if (mount_sd_0x76) {
		SetAttrs(gadgets[GID_MOUNT_SD_0x76], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_MOUNT_SD_0x76], CHECKBOX_Checked, FALSE, TAG_END);
	}

	if (mount_sd_root) {
		SetAttrs(gadgets[GID_MOUNT_SD_ROOT], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_MOUNT_SD_ROOT], CHECKBOX_Checked, FALSE, TAG_END);
	}

	if (monitor_switch&0x01) {
		SetAttrs(gadgets[GID_INFO_MON_SWITCH_CTS], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_INFO_MON_SWITCH_CTS], CHECKBOX_Checked, FALSE, TAG_END);
	}
	if (monitor_switch&0x02) {
		SetAttrs(gadgets[GID_INFO_MON_SWITCH_SEL], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_INFO_MON_SWITCH_SEL], CHECKBOX_Checked, FALSE, TAG_END);
	}
	if (monitor_switch&0x10) {
		SetAttrs(gadgets[GID_INFO_CTS_ACT_LEVEL], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_INFO_CTS_ACT_LEVEL], CHECKBOX_Checked, FALSE, TAG_END);
	}
	if (monitor_switch&0x20) {
		SetAttrs(gadgets[GID_INFO_SEL_ACT_LEVEL], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_INFO_SEL_ACT_LEVEL], CHECKBOX_Checked, FALSE, TAG_END);
	}

	if (autoconfig_ram) {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RAM], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RAM], CHECKBOX_Checked, FALSE, TAG_END);
	}

	if (autoconfig_rtg) {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RTG], CHECKBOX_Checked, TRUE, TAG_END);
	} else {
		SetAttrs(gadgets[GID_BOOT_AUTOCONFIG_RTG], CHECKBOX_Checked, FALSE, TAG_END);
	}

	SetAttrs(gadgets[GID_BOOT_LIST_BOOTMODE], LISTBROWSER_Selected, bootmode, TAG_END);
	SetAttrs(gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, kickstart,
	                                           CHOOSER_MaxLabels, num_kickstarts,
	                                           TAG_END);
	SetAttrs(gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, ext_kickstart,
	                                               CHOOSER_MaxLabels, num_ext_kickstarts,
	                                               TAG_END);

	SetAttrs(gadgets[GID_MISC_BPTON], SLIDER_Level, bpton, TAG_END);
	SetAttrs(gadgets[GID_MISC_BPTOFF], SLIDER_Level, bptoff, TAG_END);
	SetAttrs(gadgets[GID_MISC_MAC], STRINGA_TextVal, mac, TAG_END);

#define SCSI_LABELS(A) SetAttrs(gadgets[GID_SCSI ## A], CHOOSER_Selected, scsi[A],\
	                             CHOOSER_MaxLabels, num_scsis,\
	                             TAG_END);
	SCSI_LABELS(0);
	SCSI_LABELS(1);
	SCSI_LABELS(2);
	SCSI_LABELS(3);
	SCSI_LABELS(4);
	SCSI_LABELS(5);
	SCSI_LABELS(6);

	switch(selected_preset)
	{
#define PRESET(A)   case A: \
						SetAttrs(gadgets[GID_PRESET_CB ## A], CHECKBOX_Checked, TRUE, TAG_END); \
						break
		PRESET(0);
		PRESET(1);
		PRESET(2);
		PRESET(3);
		PRESET(4);
		PRESET(5);
		PRESET(6);
		PRESET(7);
		default:
		PRESET(8);
	}
	SetAttrs(gadgets[GID_PRESET0], STRINGA_TextVal, presets[0], TAG_END);
	SetAttrs(gadgets[GID_PRESET1], STRINGA_TextVal, presets[1], TAG_END);
	SetAttrs(gadgets[GID_PRESET2], STRINGA_TextVal, presets[2], TAG_END);
	SetAttrs(gadgets[GID_PRESET3], STRINGA_TextVal, presets[3], TAG_END);
	SetAttrs(gadgets[GID_PRESET4], STRINGA_TextVal, presets[4], TAG_END);
	SetAttrs(gadgets[GID_PRESET5], STRINGA_TextVal, presets[5], TAG_END);
	SetAttrs(gadgets[GID_PRESET6], STRINGA_TextVal, presets[6], TAG_END);
	SetAttrs(gadgets[GID_PRESET7], STRINGA_TextVal, presets[7], TAG_END);

	if(win!=NULL)
	{
		for(i=0;i<REFRESH_COUNT;i++)
			RefreshGadgets(gadgets[table_refresh[i]], win, NULL);
	}
}

ULONG zz_perform_memtest(uint32_t offset)
{
	uint32_t errors=0;
	volatile uint32_t* bufferl = (volatile uint32_t*)((uint32_t)zz_cd->cd_BoardAddr+offset);
	volatile uint16_t* bufferw = (volatile uint16_t*)bufferl;
	uint32_t i;
	uint32_t rep=1024*256;

	printf("zz_perform_memtest...\n");

	for (i=0; i<rep; i++) {
		uint32_t v2;
		uint32_t v = (i%2)?0xaaaa5555:0x33337777;
		uint16_t v4;
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
VOID handleGadgetEvent(struct Window *win,int gad, UWORD code)
{
	switch (gad)
	{
		case GID_INFO_BTN_REFRESH: {
			refresh_zz_info(win);
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
				SetGadgetAttrs(gadgets[GID_INFO_JIT],windows[WID_MAIN], NULL,
				                                     CHECKBOX_Checked, FALSE,
				                                     TAG_END);
			refresh_zz_info(win);
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
                                                      SLIDER_Level, code,
                                                      TAG_DONE);
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
			zz_set_apply_preset();
			zz_set_reg(REG_ZZ_APPLY_BOOTMODE, 0x55AA);
			break;
		}
		case GID_INFO_BTN_HARDREBOOT: {
			zz_set_hardreboot();
			break;
		}
		case GID_SCSI_BTN_APPLY_SCSI: {
			zz_set_apply_scsi();
			zz_set_apply_preset();
			zz_set_reg(REG_ZZ_APPLY_SCSI, 0x55AA);
			break;
		}
		case GID_MISC_BTN_APPLY_MISC: {
			zz_set_apply_misc();
			zz_set_apply_preset();
			zz_set_reg(REG_ZZ_APPLY_MISC, 0x55AA);
			break;
		}
		case GID_PRESET_BTN_APPLY_PRESET: {
			zz_set_apply_preset();
			zz_set_reg(REG_ZZ_APPLY_PRESET, 0x55AA);
			break;
		}
		case GID_PRESET_BTN_DELETE_PRESET: {
			zz_set_reg(REG_ZZ_DELETE_PRESET, 0x55AA);
			break;
		}
		case GID_BOOT_BTN_APPLY_ALL:
		case GID_SCSI_BTN_APPLY_ALL:
		case GID_MISC_BTN_APPLY_ALL:
		case GID_PRESET_BTN_APPLY_ALL: {
			zz_set_apply_all();
			break;
		}
		case GID_BOOT_SCSIBOOT: {
			zz_set_scsiboot_enabled(code);
			break;
		}
		case GID_BOOT_ENABLETEST: {
			zz_set_test_enabled(code);
			break;
		}
		case GID_CPURAM_ENABLE: {
			zz_set_cpuram_enabled(code);
			break;
		}
		case GID_MOUNT_SD_0x76: {
			zz_set_mount_sd_0x76(code);
			break;
		}
		case GID_MOUNT_SD_ROOT: {
			zz_set_mount_sd_root(code);
			break;
		}
		
		case GID_INFO_MON_SWITCH_CTS: {
			zz_set_monitor_switch((get_monitor_switch()&(~0x01))|(code?0x01:0));
			break;
		}
		case GID_INFO_MON_SWITCH_SEL: {
			zz_set_monitor_switch((get_monitor_switch()&(~0x02))|(code?0x02:0));
			break;
		}
		case GID_INFO_CTS_ACT_LEVEL: {
			zz_set_monitor_switch((get_monitor_switch()&(~0x10))|(code?0x10:0));
			break;
		}
		case GID_INFO_SEL_ACT_LEVEL: {
			zz_set_monitor_switch((get_monitor_switch()&(~0x20))|(code?0x20:0));
			break;
		}
		case GID_BOOT_AUTOCONFIG_RAM: {
			zz_set_autoconfig_ram_enabled(code);
			break;
		}
		case GID_BOOT_AUTOCONFIG_RTG: {
			zz_set_autoconfig_rtg_enabled(code);
			break;
		}
		case GID_MISC_MAC: {
//			zz_set_lpf_freq(code);
			break;
		}
		case GID_MISC_BPTON: {
//			code=code-(code%100);
//			SetAttrs( gadgets[GID_MISC_BPTON], SLIDER_Level,  code, TAG_DONE);
			if(code==0)
				SetAttrs( gadgets[GID_MISC_BPTON], SLIDER_LevelFormat,  " Disabled", TAG_DONE);
			else
				SetAttrs( gadgets[GID_MISC_BPTON], SLIDER_LevelFormat,  "%4ld.0 us", TAG_DONE);
			RefreshGadgets(gadgets[GID_MISC_BPTON], win, NULL);
			zz_set_bpton(code);
			break;
		}
		case GID_MISC_BPTOFF: {
			zz_set_bptoff(code);
			break;
		}
		case GID_PRESET_CB0:
		case GID_PRESET_CB1:
		case GID_PRESET_CB2:
		case GID_PRESET_CB3:
		case GID_PRESET_CB4:
		case GID_PRESET_CB5:
		case GID_PRESET_CB6:
		case GID_PRESET_CB7:
		case GID_PRESET_CB8: {
			int i;
			for(i=0;i<9;i++)
				SetAttrs( gadgets[i+GID_PRESET_CB0], CHECKBOX_Checked, FALSE, TAG_DONE);
			SetAttrs( gadgets[gad], CHECKBOX_Checked, TRUE, TAG_DONE);
			for(i=0;i<9;i++)
				RefreshGadgets(gadgets[i+GID_PRESET_CB0], win, NULL);
			zz_set_selected_preset(win, gad-GID_PRESET_CB0);
			refresh_zz_info(win);
			break;
		}
	}
}
VOID free_listview_list(struct List *list)
{
	struct Node *node, *nextnode;

	node = list->lh_Head;
	while (nextnode = node->ln_Succ)
	{
		FreeVec(node);
		node = nextnode;
	}
	NewList(list);
}

#include <intuition/intuition.h>
#include <intuition/screens.h>
 
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

BOOL processMenus(UWORD menuNumber)
{
	UWORD menuNum;
	UWORD itemNum;
	UWORD subNum;

	menuNum = MENUNUM(menuNumber);
	itemNum = ITEMNUM(menuNumber);
	subNum  = SUBNUM(menuNumber);

	if(menuNum==0)
	{
		if(itemNum==0) // About
		{
			char message[100];
			if(Z3660_ZTOP_VERSION_MINOR>0)
				sprintf(message,Z3660_ZTOP_VERSION_MAJOR " BETA %d", Z3660_ZTOP_VERSION_MINOR );
			else
				strcpy(message,Z3660_ZTOP_VERSION_MAJOR);
			Pop(message);
		}
		else if (itemNum==2) // Quit
		{
			return(TRUE);
		}
	}
	return(FALSE);
}
int main(void)
{
	int i;
	int font_height,font_width;
	struct MsgPort *AppPort;
	struct Screen *my_screen;
	struct DrawInfo *drinfo;
//#define NEWSCREEN__
#ifdef NEWSCREEN__
	uint16_t pens[] = { 0xFFFF };
	struct IntuitionIFace *IIntuition;
	struct Screen *new_screen;
#endif
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
	my_screen = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;
	drinfo = GetScreenDrawInfo(my_screen);

	font_height=drinfo->dri_Font->tf_YSize;
	font_width=drinfo->dri_Font->tf_XSize;
#ifdef NEWSCREEN__
	new_screen=NULL;
	
	if(my_screen->Width<500 || my_screen->Height<=200)
	{
		new_screen = OpenScreenTags(NULL,
			SA_Pens, pens,
			SA_Depth, 2,
			SA_Width, 640,
			SA_Height,480,
			SA_AutoScroll, TRUE,
			SA_OffScreenDragging, TRUE,
			TAG_END);
		if (new_screen != NULL)
    	{ 
			/* screen successfully opened */
		}
	}
 #endif

	NewList(&dlist);
	
	kickstarts_list = ChooserLabelsA(kickstarts);
	ext_kickstarts_list = ChooserLabelsA(ext_kickstarts);
	scsis_list = ChooserLabelsA(scsis);

	if ( !ButtonBase      || 
		 !CheckBoxBase    ||
		 !SliderBase      ||
		 !ClickTabBase    ||
		 !LayoutBase      ||
		 !ListBrowserBase ||
		 !StringBase        )
	{
		if(!ButtonBase)
			errorMessage("gadget/button.gadget not found.\n");
		if(!CheckBoxBase)
			errorMessage("gadget/checkbox.gadget not found.\n");
		if(!SliderBase)
			errorMessage("gadget/slider.gadget not found.\n");
		if(!ClickTabBase)
			errorMessage("gadget/clicktab.gadget not found.\n");
		if(!LayoutBase)
			errorMessage("gadget/layout.gadget not found.\n");
		if(!ListBrowserBase)
			errorMessage("gadget/listbrowser.gadget not found.\n");
		if(!StringBase)
			errorMessage("gadget/string.gadget not found.\n");
		return(30);
	}
	else if ( AppPort = CreateMsgPort() )
	{
		struct List *tablabels = ClickTabs("Info","Boot","SCSI","Misc","Preset", NULL);
		if (tablabels)
		{
			/* Create the window object.
			 */
			objects[OID_MAIN] = WindowObject,
//				WA_ScreenTitle, "Ztop 1.03",
				WA_Title, Z3660_ZTOP_VERSION_MAJOR,
				WA_Activate, TRUE,
				WA_DepthGadget, TRUE,
				WA_DragBar, TRUE,
				WA_CloseGadget, TRUE,
				WA_SizeGadget, FALSE,
#ifdef NEWSCREEN__
				WA_CustomScreen,	new_screen,
#endif
				WA_NewLookMenus, TRUE,
				WINDOW_NewMenu, mainmenu,
				WA_IDCMP, IDCMP_GADGETUP | IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
				          IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
				          BUTTONIDCMP,
				WA_Flags, WFLG_SIMPLE_REFRESH,
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

								LAYOUT_AddChild, HLayoutObject,
									LAYOUT_AddChild, gadgets[GID_ALIGN2_1] = VLayoutObject,

										LAYOUT_AddChild, gadgets[GID_INFO_FWVER] = StringObject,
											GA_ID, GID_INFO_FWVER,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"Firmware Version", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_VAUX] = StringObject,
											GA_ID, GID_INFO_VAUX,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"Aux Voltage (V)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_VINT] = StringObject,
											GA_ID, GID_INFO_VINT,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"Core Voltage (V)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_LTC_V1] = StringObject,
											GA_ID, GID_INFO_LTC_V1,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"LTC (3V3) Vdd (V)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_LTC_V2] = StringObject,
											GA_ID, GID_INFO_LTC_V2,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"LTC (5V) Vcc (V)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

									LayoutEnd,
									
									SPACE,

									LAYOUT_AddChild, gadgets[GID_ALIGN2_2] = VLayoutObject,

										LAYOUT_AddChild, gadgets[GID_INFO_CPU_FREQ] = StringObject,
											GA_ID, GID_INFO_CPU_FREQ,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"CPU Frequency (MHz)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_TEMP] = StringObject,
											GA_ID, GID_INFO_TEMP,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"FPGA Core Temp (C)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_LTC_TEMP] = StringObject,
											GA_ID, GID_INFO_LTC_TEMP,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"LTC Temp (C)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, gadgets[GID_INFO_LTC_060_TEMP] = StringObject,
											GA_ID, GID_INFO_LTC_060_TEMP,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 48,
											STRINGA_TextVal, "",
											STRINGA_Justification, GACT_STRINGCENTER,
											StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"LTC (060 THERM) (C)", LabelEnd,
										CHILD_MinWidth, INFO_STR_WIDTH,
										CHILD_MaxWidth, INFO_STR_WIDTH+20,

										LAYOUT_AddChild, HLayoutObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_INFO_JIT] = CheckBoxObject,
												GA_ID, GID_INFO_JIT,
												GA_RelVerify, TRUE,
												GA_Text, "JIT Enabled",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											SPACE,
										LayoutEnd,

										
									LayoutEnd,


								LayoutEnd,

								LAYOUT_AddChild, HLayoutObject,

									LAYOUT_AddChild, gadgets[GID_ALIGN3_1] = VLayoutObject,
										SPACE,

										LAYOUT_AddChild, gadgets[GID_ALIGN6_1] = HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_INFO_MON_SWITCH_CTS] = CheckBoxObject,
												GA_ID, GID_INFO_MON_SWITCH_CTS,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "Mon. Switch CTS",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, gadgets[GID_ALIGN6_2] = HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_INFO_MON_SWITCH_SEL] = CheckBoxObject,
												GA_ID, GID_INFO_MON_SWITCH_SEL,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "Mon. Switch SEL",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,
									LayoutEnd,

									LAYOUT_AddChild, gadgets[GID_ALIGN3_2] = VLayoutObject,
										LAYOUT_AddChild, HGroupObject,
											SPACE,
										LayoutEnd,
										LAYOUT_AddChild, HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_INFO_CTS_ACT_LEVEL] = CheckBoxObject,
												GA_ID, GID_INFO_CTS_ACT_LEVEL,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "CTS act. level",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,
										LAYOUT_AddChild, HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_INFO_SEL_ACT_LEVEL] = CheckBoxObject,
												GA_ID, GID_INFO_SEL_ACT_LEVEL,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "SEL act. level",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
											LayoutEnd,
											CHILD_WeightedHeight, 0,

									LayoutEnd,
									LAYOUT_AddChild, gadgets[GID_ALIGN3_3] = VLayoutObject,
										SPACE,
									LayoutEnd,
								LayoutEnd,

								LAYOUT_AddChild, HGroupObject,

									LAYOUT_AddChild, gadgets[GID_INFO_LPF] = SliderObject,
										GA_ID, GID_INFO_LPF,
										GA_RelVerify, TRUE,
										SLIDER_Min,          0,
										SLIDER_Max,      23900,
										SLIDER_Level,    23900,
										SLIDER_Orientation,  SLIDER_HORIZONTAL,
										SLIDER_LevelPlace,   PLACETEXT_ABOVE,
										SLIDER_LevelMaxLen,  8,
										SLIDER_LevelFormat,  "%ld Hz",
										SLIDER_Ticks,        25,
										SLIDER_ShortTicks,   TRUE,
									SliderEnd,
									Label("Audio Lowpass Filter"),
									CHILD_WeightedHeight, 0,
								LayoutEnd,

								SPACE,

								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_BTN_HARDREBOOT] = ButtonObject,
										GA_ID, GID_INFO_BTN_HARDREBOOT,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Hard Reboot",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_BTN_TEST] = ButtonObject,
										GA_ID, GID_INFO_BTN_TEST,
										GA_RelVerify, TRUE,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Bus Test",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_INFO_BTN_REFRESH] = ButtonObject,
										GA_ID, GID_INFO_BTN_REFRESH,
										GA_RelVerify, TRUE,
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

								LAYOUT_AddChild,  HGroupObject,
									LAYOUT_AddChild, VGroupObject,
										LAYOUT_AddChild,  HGroupObject,
											SPACE,
											LAYOUT_AddImage, LabelObject,
												LABEL_Text,  "Boot Mode Selection",
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
												LISTBROWSER_AutoFit, TRUE,
											LayoutEnd,
											CHILD_MinWidth,  (1+font_width)*15,
											CHILD_MaxWidth,  (1+font_width)*15,
											CHILD_MinHeight, (1+font_height)*NUM_BOOTMODES,
											CHILD_MaxHeight, (1+font_height)*NUM_BOOTMODES,
											SPACE,

										LayoutEnd,

										SPACE,
										SPACE,
										SPACE,

										LAYOUT_AddChild, gadgets[GID_ALIGN4_1] = HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_MOUNT_SD_0x76] = CheckBoxObject,
												GA_ID, GID_MOUNT_SD_0x76,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "  MOUNT SD 0x76",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, gadgets[GID_ALIGN4_2] = HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_MOUNT_SD_ROOT] = CheckBoxObject,
												GA_ID, GID_MOUNT_SD_ROOT,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "  MOUNT SD ROOT",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,
									LayoutEnd,
									CHILD_WeightedHeight, 0,

									LAYOUT_AddChild, VGroupObject,

										LAYOUT_AddChild, gadgets[GID_BOOT_CPU_FREQ] = SliderObject,
											GA_ID, GID_BOOT_CPU_FREQ,
											GA_RelVerify, TRUE,
											SLIDER_Min,       50,
											SLIDER_Max,      100,
											SLIDER_Level,    100,
											SLIDER_Orientation,  SLIDER_HORIZONTAL,
											SLIDER_LevelPlace,   PLACETEXT_ABOVE,
											SLIDER_LevelMaxLen,  7,
											SLIDER_KnobDelta,     CPU_FREQ_STEP,
											SLIDER_LevelFormat,  "%ld MHz",
											SLIDER_Ticks,        11,
											SLIDER_ShortTicks,   TRUE,
										SliderEnd,
										Label("CPU Frequency"),
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, HGroupObject,
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

										LAYOUT_AddChild, HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_BOOT_AUTOCONFIG_RAM] = CheckBoxObject,
												GA_ID, GID_BOOT_AUTOCONFIG_RAM,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "AUTOC RAM enabled",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_BOOT_AUTOCONFIG_RTG] = CheckBoxObject,
												GA_ID, GID_BOOT_AUTOCONFIG_RTG,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "AUTOC RTG enabled",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_BOOT_ENABLETEST] = CheckBoxObject,
												GA_ID, GID_BOOT_ENABLETEST,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "     TEST enabled",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

										LAYOUT_AddChild, gadgets[GID_ALIGN5] = HGroupObject,
											SPACE,
											LAYOUT_AddChild, gadgets[GID_CPURAM_ENABLE] = CheckBoxObject,
												GA_ID, GID_CPURAM_ENABLE,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 24,
												GA_Text, "  CPU RAM enabled",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
											SPACE,
										LayoutEnd,
										CHILD_WeightedHeight, 0,

									LayoutEnd,
									CHILD_WeightedHeight, 0,
								LayoutEnd,
								CHILD_WeightedHeight, 0,
	
								SPACE,

								LAYOUT_AddChild, HGroupObject,
									LAYOUT_SpaceInner, TRUE,
									LAYOUT_AddChild, gadgets[GID_BOOT_LIST_KICKSTART] = ChooserObject,
										GA_ID, GID_BOOT_LIST_KICKSTART,
										GA_RelVerify, TRUE,
										CHOOSER_Labels, kickstarts_list,
										CHOOSER_Selected, 0,
										CHOOSER_PopUp, TRUE,
										CHOOSER_DropDown, FALSE,
										CHOOSER_AutoFit, FALSE,
									ChooserEnd,
									CHILD_NominalSize, TRUE,
									CHILD_Label, LabelObject, LABEL_Text, "Kickstart ", LabelEnd,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								LAYOUT_AddChild, HGroupObject,
									LAYOUT_SpaceInner, TRUE,
									LAYOUT_AddChild, gadgets[GID_BOOT_LIST_EXT_KICKSTART] = ChooserObject,
										GA_ID, GID_BOOT_LIST_EXT_KICKSTART,
										GA_RelVerify, TRUE,
										CHOOSER_Labels, ext_kickstarts_list,
										CHOOSER_Selected, 0,
										CHOOSER_PopUp, TRUE,
										CHOOSER_DropDown, FALSE,
										CHOOSER_AutoFit, FALSE,
									ChooserEnd,
									CHILD_NominalSize, TRUE,
									CHILD_Label, LabelObject, LABEL_Text, "Ext Kicks.", LabelEnd,
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
									LAYOUT_AddChild, gadgets[GID_BOOT_BTN_APPLY_ALL] = ButtonObject,
										GA_ID, GID_BOOT_BTN_APPLY_ALL,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply ALL",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

							PageEnd,

							PAGE_Add, gadgets[GID_PAGELAY3] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

#define SCSI_CHOOSER(A,B)		LAYOUT_AddChild, HGroupObject,\
									LAYOUT_SpaceInner, TRUE,\
									LAYOUT_AddChild, gadgets[GID_SCSI ## A] = ChooserObject,\
										GA_ID, GID_SCSI0,\
										GA_RelVerify, TRUE,\
										CHOOSER_Labels, scsis_list,\
										CHOOSER_Active, scsi[A],\
										/*CHOOSER_Selected, scsis[A],*/\
										CHOOSER_PopUp, TRUE,\
										CHOOSER_DropDown, FALSE,\
										CHOOSER_AutoFit, FALSE,\
									ChooserEnd,\
									CHILD_NominalSize, TRUE,\
									CHILD_Label, LabelObject, LABEL_Text, B, LabelEnd,\
								LayoutEnd,\
								CHILD_WeightedHeight, 0

								SCSI_CHOOSER(0,"SCSI0"),
								SCSI_CHOOSER(1,"SCSI1"),
								SCSI_CHOOSER(2,"SCSI2"),
								SCSI_CHOOSER(3,"SCSI3"),
								SCSI_CHOOSER(4,"SCSI4"),
								SCSI_CHOOSER(5,"SCSI5"),
								SCSI_CHOOSER(6,"SCSI6"),

								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_SCSI_BTN_APPLY_SCSI] = ButtonObject,
										GA_ID, GID_SCSI_BTN_APPLY_SCSI,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply SCSI",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_SCSI_BTN_APPLY_ALL] = ButtonObject,
										GA_ID, GID_SCSI_BTN_APPLY_ALL,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply ALL",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,
							PageEnd,

							PAGE_Add, gadgets[GID_PAGELAY4] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

								LAYOUT_AddChild, HLayoutObject,
									LAYOUT_AddChild, VLayoutObject,

										LAYOUT_AddChild, gadgets[GID_MISC_MAC] = StringObject,
											GA_ID, GID_INFO_FWVER,
											GA_RelVerify, TRUE,
											STRINGA_MaxChars, 18,
											STRINGA_TextVal, "00:00:00:00:00:00",
											STRINGA_Justification, GACT_STRINGCENTER,
										StringEnd,
										CHILD_Label, LabelObject, LABEL_Text,"MAC Address", LabelEnd,
										CHILD_MaxWidth, 150+16,

										SPACE,

										LAYOUT_AddChild, gadgets[GID_MISC_BPTON] = SliderObject,
											GA_ID, GID_MISC_BPTON,
											GA_RelVerify, TRUE,
											SLIDER_Min,          0,
											SLIDER_Max,       2000,
											SLIDER_Level,        0,
											SLIDER_Orientation,  SLIDER_HORIZONTAL,
											SLIDER_LevelPlace,   PLACETEXT_RIGHT,
											SLIDER_KnobDelta,  100,
											SLIDER_LevelMaxLen,  9,
											SLIDER_LevelFormat,  "%4ld.0 us",
											SLIDER_Ticks,       21,
											SLIDER_ShortTicks,   TRUE,
										SliderEnd,
										Label("Beeper Ton "),
										CHILD_WeightedHeight, 0,
										CHILD_MaxWidth, 280,

										SPACE,

										LAYOUT_AddChild, gadgets[GID_MISC_BPTOFF] = SliderObject,
											GA_ID, GID_MISC_BPTOFF,
											GA_RelVerify, TRUE,
											SLIDER_Min,          0,
											SLIDER_Max,         20,
											SLIDER_Level,        0,
											SLIDER_Orientation,  SLIDER_HORIZONTAL,
											SLIDER_LevelPlace,   PLACETEXT_RIGHT,
											SLIDER_KnobDelta,    1,
											SLIDER_LevelMaxLen,  9,
											SLIDER_LevelFormat,  "%4ld.0 ms",
											SLIDER_Ticks,       21,
											SLIDER_ShortTicks,   TRUE,
										SliderEnd,
										Label("Bepper Toff"),
										CHILD_WeightedHeight, 0,
										CHILD_MaxWidth, 280,

									LayoutEnd,
									CHILD_WeightedHeight, 0,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_MISC_BTN_APPLY_MISC] = ButtonObject,
										GA_ID, GID_MISC_BTN_APPLY_MISC,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply Misc",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_MISC_BTN_APPLY_ALL] = ButtonObject,
										GA_ID, GID_MISC_BTN_APPLY_ALL,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply ALL",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

							PageEnd,

							PAGE_Add, gadgets[GID_PAGELAY5] = VGroupObject,
								LAYOUT_SpaceOuter, TRUE,
								LAYOUT_SpaceInner, TRUE,

								LAYOUT_AddChild, HLayoutObject,
									LAYOUT_AddChild, VLayoutObject,
#define PRESET_TEXT(A,B)                LAYOUT_AddChild, HLayoutObject,                                    \
											LAYOUT_AddChild, gadgets[GID_PRESET ## A] = StringObject,      \
												GA_ID, GID_INFO_FWVER,                                     \
												GA_RelVerify, TRUE,                                        \
												STRINGA_MaxChars, 48,                                      \
												STRINGA_TextVal, "",                                       \
												STRINGA_Justification, GACT_STRINGLEFT,                    \
											StringEnd,                                                     \
											CHILD_Label, LabelObject, LABEL_Text, B, LabelEnd,             \
											CHILD_MinWidth, 280,                                           \
											/*SPACE,*/                                                     \
											LAYOUT_AddChild, gadgets[GID_PRESET_CB ## A] = CheckBoxObject, \
												GA_ID, GID_PRESET_CB ## A,                                 \
												GA_RelVerify, TRUE,                                        \
												GA_TabCycle, TRUE,                                         \
												STRINGA_MaxChars, 0,                                       \
												GA_Text, "",                                               \
												CHECKBOX_TextPlace, PLACETEXT_LEFT,                        \
											CheckBoxEnd,                                                   \
											CHILD_WeightedWidth, 0,                                        \
										LayoutEnd

											PRESET_TEXT(0,"Preset 0 Name"),
											PRESET_TEXT(1,"Preset 1 Name"),
											PRESET_TEXT(2,"Preset 2 Name"),
											PRESET_TEXT(3,"Preset 3 Name"),
											PRESET_TEXT(4,"Preset 4 Name"),
											PRESET_TEXT(5,"Preset 5 Name"),
											PRESET_TEXT(6,"Preset 6 Name"),
											PRESET_TEXT(7,"Preset 7 Name"),
										
										LAYOUT_AddChild, HLayoutObject,
											LAYOUT_AddChild, gadgets[GID_PRESET_CB8] = CheckBoxObject,
												GA_ID, GID_PRESET_CB8,
												GA_RelVerify, TRUE,
												GA_TabCycle, TRUE,
												STRINGA_MaxChars, 0,
												GA_Text, " No Preset (will use z3660cfg.txt file as default)",
												CHECKBOX_TextPlace, PLACETEXT_LEFT,
											CheckBoxEnd,
											CHILD_WeightedWidth, 0,
										LayoutEnd,
									LayoutEnd,
									CHILD_WeightedHeight, 0,

								LayoutEnd,
								CHILD_WeightedHeight, 0,

								SPACE,
								LAYOUT_AddChild, HGroupObject,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_PRESET_BTN_APPLY_PRESET] = ButtonObject,
										GA_ID, GID_PRESET_BTN_APPLY_PRESET,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply Preset",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_PRESET_BTN_DELETE_PRESET] = ButtonObject,
										GA_ID, GID_PRESET_BTN_DELETE_PRESET,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Delete Preset",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
									LAYOUT_AddChild, gadgets[GID_PRESET_BTN_APPLY_ALL] = ButtonObject,
										GA_ID, GID_PRESET_BTN_APPLY_ALL,
										GA_RelVerify, TRUE,
										STRINGA_MaxChars, 48,
										GA_Width,  40,
										GA_Height, 14,
										GA_Text, "Apply ALL",
									ButtonEnd,
									CHILD_WeightedWidth, 0,
									SPACE,
								LayoutEnd,
								CHILD_WeightedHeight, 0,

							PageEnd,
						ClickTabEnd,
					LayoutEnd,

				EndGroup,
			EndWindow;

	 	 	/*  Object creation sucessful?
	 	 	 */
			if (objects[OID_MAIN])
			{
				/* Set up inter-group label pagement.
				 */
				SetAttrs( gadgets[GID_ALIGN2_1], LAYOUT_AlignLabels, gadgets[GID_ALIGN2_2], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGN3_1], LAYOUT_AlignLabels, gadgets[GID_ALIGN3_2], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGN3_2], LAYOUT_AlignLabels, gadgets[GID_ALIGN3_3], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGN4_1], LAYOUT_AlignLabels, gadgets[GID_ALIGN4_2], TAG_DONE);
				SetAttrs( gadgets[GID_ALIGN6_1], LAYOUT_AlignLabels, gadgets[GID_ALIGN6_2], TAG_DONE);

				for(i=0;i<NUM_BOOTMODES;i++)
				{
					LBAddNode(gadgets[GID_BOOT_LIST_BOOTMODE], NULL, NULL, (struct Node *)~0,
								LBNCA_CopyText, TRUE,
								LBNCA_Text, bootmode_names[i],
								TAG_DONE);
				}
//				SetAttrs(gadgets[GID_BOOT_LIST_BOOTMODE], LISTBROWSER_Selected, bootmode, TAG_END);
//				SetAttrs(gadgets[GID_BOOT_LIST_KICKSTART], CHOOSER_Selected, kickstart, TAG_END);
//				SetAttrs(gadgets[GID_BOOT_LIST_EXT_KICKSTART], CHOOSER_Selected, ext_kickstart, TAG_END);

				init_measures();
//refresh_zz_info(windows[WID_MAIN]);

				/*  Open the window.
				 */
				if (windows[WID_MAIN] = (struct Window *) RA_OpenWindow(objects[OID_MAIN]))
				{
					ULONG wait, signal, app = (1L << AppPort->mp_SigBit);
					ULONG done = FALSE;
					ULONG result;
					UWORD code;

refresh_zz_info(windows[WID_MAIN]);
				 	/* Obtain the window wait signal mask.
					 */
					GetAttr(WINDOW_SigMask, objects[OID_MAIN], &signal);

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

//									case WMHI_MOUSEMOVE:
									case WMHI_GADGETUP:
										handleGadgetEvent(windows[WID_MAIN],result & WMHI_GADGETMASK, code);
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
									case WMHI_MENUPICK: { /* Test to see if a menu is selected */
										struct Menu *menuStrip = windows[WID_MAIN]->MenuStrip;    /* Get address of which menu selected */
										UWORD selection = code;
										while ((selection != MENUNULL) && (done == FALSE))
										{
											struct MenuItem *item = ItemAddress(menuStrip, selection); /* Get the menu item from the menu */
											done = processMenus(selection); /* Call a subroutine to process the item */
											if(done)
											{
												windows[WID_MAIN] = NULL;
												break;
											}
											selection = item->NextSelect;
										}
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
	free_listview_list(&dlist);

#ifdef NEWSCREEN__
	if(new_screen!=NULL)
		CloseScreen(new_screen);
#endif
	return(0);
}
