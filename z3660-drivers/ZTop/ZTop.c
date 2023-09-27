/*
 * Z3660 ZTop application based on MNT ZZ9000 ZZTop
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 */

/*
 * MNT ZZ9000 Amiga Graphics Card Diagnostics (ZZTop)
 * Copyright (C) 2016-2022, Lukas F. Hartmann <lukas@mntre.com>
 *                                       MNT Research GmbH, Berlin
 *                                       https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 */

#include <exec/types.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/expansion_protos.h>

#include <clib/timer_protos.h>
#include <clib/alib_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "z3660_regs.h"

enum {
MYGAD_CPU_FREQ,           //  0
MYGAD_FWVER,              //  1
MYGAD_TEMP,               //  2
MYGAD_VAUX,               //  3
MYGAD_VINT,               //  4
MYGAD_BTN_TEST,           //  5
MYGAD_BTN_REFRESH,        //  6
MYGAD_Z9AX,               //  7
MYGAD_JIT,                //  8
MYGAD_LPF,                //  9
MYGAD_LIST_BOOTMODE,      // 10
MYGAD_BTN_APPLY_BOOTMODE, // 11
MYGAD_SCSIBOOT,           // 12
MYGAD_LTC_TEMP,           // 13
MYGAD_LTC_V1,             // 14
MYGAD_LTC_V2,             // 15
MYGAD_LTC_060_TEMP,       // 16
MYGAD_AUTOCONFIG_RAM,     // 17
NUM_GADGETS
};
struct Gadget *gads[NUM_GADGETS];

typedef struct Cpulist{
    struct Node node;
} cpulist;

enum {
   CPU,
   MUSASHI,
   UAE,
   UAEJIT,
   NUM_BOOTMODES
};
char bootmode_names[NUM_BOOTMODES][20]={
//   "XXXXXXXXXXXXXXXXXXX"
   "   060 real CPU    ",
   "  030 MUSASHI emu  ",
   "    040 UAE emu    ",
   "  040 UAE JIT emu  ",
};
cpulist dnode[NUM_BOOTMODES];

struct TextAttr Topaz80 = { (STRPTR)"topaz.font", 8, 0, 0, };

struct Library* IntuitionBase;
struct Library* GfxBase;
struct Library* GadToolsBase;
struct Library* ExpansionBase;

struct ConfigDev* zz_cd;
volatile UBYTE* zz_regs;

char txt_buf[64];

struct timerequest * timerio;
struct MsgPort *timerport;
struct Library *TimerBase;

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

uint32_t zz_get_ax_present(void)
{
   return zz_get_reg(REG_ZZ_AUDIO_CONFIG);
}

uint32_t zz_get_cpu_freq(void)
{
   return    zz_get_reg(REG_ZZ_CPU_FREQ);
}

uint32_t zz_get_selected_bootmode(struct Window* win)
{
   return zz_get_reg(REG_ZZ_BOOTMODE);
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
//   zz_set_reg(REG_ZZ_AUDIO_PARAM, 0);
}

void zz_set_cpu_freq(uint16_t freq)
{
   zz_set_reg(REG_ZZ_CPU_FREQ, freq);
}

void zz_set_selected_bootmode(struct Window* win,uint16_t bm)
{
   zz_set_reg(REG_ZZ_BOOTMODE, bm);
}

void zz_set_apply_bootmode(void)
{
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
void refresh_zz_info(struct Window* win)
{
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
   int z9ax_present = zz_get_ax_present();
   int emulation_used = zz_get_emulation_used();
   int jit_enable = zz_get_jit_enable();
   int cpu_freq=zz_get_cpu_freq();
   int bootmode=zz_get_selected_bootmode(win);
   int scsiboot=zz_get_scsiboot_enable();
   int autoconfig_ram=zz_get_autoconfig_ram_enable();

   filter(&t);
   filter(&vaux);
   filter(&vint);
   filter(&ltc_temp);
   filter(&ltc_v1);
   filter(&ltc_v2);
   filter(&ltc_060_temp);

   GT_SetGadgetAttrs(gads[MYGAD_CPU_FREQ], win, NULL, GTSL_Level, cpu_freq, TAG_END);

   snprintf(txt_buf, 20, "Z3660 %d.%02d", fwrev_major, fwrev_minor);
   GT_SetGadgetAttrs(gads[MYGAD_FWVER], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.1f", t.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_TEMP], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.2f", vaux.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_VAUX], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.2f", vint.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_VINT], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.1f", ltc_temp.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_LTC_TEMP], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.2f", ltc_v1.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_LTC_V1], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.2f", ltc_v2.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_LTC_V2], win, NULL, GTST_String, txt_buf, TAG_END);

   snprintf(txt_buf, 20, "%.1f", ltc_060_temp.m_filt);
   GT_SetGadgetAttrs(gads[MYGAD_LTC_060_TEMP], win, NULL, GTST_String, txt_buf, TAG_END);

   if (emulation_used) {
      GT_SetGadgetAttrs(gads[MYGAD_JIT], win, NULL, GTCB_Checked, jit_enable, TAG_END);
   } else {
      GT_SetGadgetAttrs(gads[MYGAD_JIT], win, NULL, GTCB_Checked, FALSE, TAG_END);
   }
   
   if (scsiboot) {
      GT_SetGadgetAttrs(gads[MYGAD_SCSIBOOT], win, NULL, GTCB_Checked, TRUE, TAG_END);
   } else {
      GT_SetGadgetAttrs(gads[MYGAD_SCSIBOOT], win, NULL, GTCB_Checked, FALSE, TAG_END);
   }
   
   if (autoconfig_ram) {
      GT_SetGadgetAttrs(gads[MYGAD_AUTOCONFIG_RAM], win, NULL, GTCB_Checked, TRUE, TAG_END);
   } else {
      GT_SetGadgetAttrs(gads[MYGAD_AUTOCONFIG_RAM], win, NULL, GTCB_Checked, FALSE, TAG_END);
   }

   if (z9ax_present) {
      GT_SetGadgetAttrs(gads[MYGAD_Z9AX], win, NULL, GTST_String, (STRPTR)"Present", TAG_END);
   } else {
      GT_SetGadgetAttrs(gads[MYGAD_Z9AX], win, NULL, GTST_String, (STRPTR)"Not present", TAG_END);
   }

   GT_SetGadgetAttrs(gads[MYGAD_LIST_BOOTMODE], win, NULL, GTLV_Selected, bootmode, TAG_END);
}

ULONG zz_perform_memtest(uint32_t offset)
{
   volatile uint32_t* bufferl = (volatile uint32_t*)(zz_cd->cd_BoardAddr+offset);
   volatile uint16_t* bufferw = (volatile uint16_t*)bufferl;
   uint32_t i = 0;
   uint32_t errors = 0;
   uint32_t rep=1024*256;

   printf("zz_perform_memtest...\n");

   for (i=0; i<rep; i++) {
      uint32_t v2 = 0;
      uint32_t v = (i%2)?0xaaaa5555:0x33337777;
      uint16_t v4 = 0;
      uint16_t v3 = (i%2)?0xffff:0x0000;

      if ((i % (32*1024)) == 0) {
         printf("`-- Test %p %6ld/%ld...\n", zz_cd->cd_BoardAddr+offset, i, rep);
      }

      bufferl[i] = v;
      v2 = bufferl[i];

      if (v!=v2) {
         printf("32-bit mismatch at %p: 0x%lx should be 0x%lx\n",&bufferl[i],v2,v);
         errors++;
      }

      bufferw[i] = v3;
      v4 = bufferw[i];

      if (v3!=v4) {
         printf("16-bit mismatch at %p: 0x%x should be 0x%x\n",&bufferw[i],v4,v3);
         errors++;
      }
   }
   printf("Done. %ld errors.\n", errors);
   return errors;
}

ULONG zz_perform_memtest_rand(uint32_t offset, int rep)
{
   uint32_t errors = 0;
   const int sz = 16;
   volatile uint16_t* buffer = (volatile uint16_t*)(zz_cd->cd_BoardAddr+offset);

   printf("zz_perform_memtest_rand...\n");

   uint16_t* tbuf = malloc(2*sz);
   if (!tbuf) {
      printf("Error: Could not allocate memory for test buffer\n");
      return 1;
   }

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test %p %3d/%d...\n", zz_cd->cd_BoardAddr+offset, k, rep);
      }
      // step 1: fill buffer with random data
      for (int i=0; i<sz; i++) {
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

      for (int i=0; i<sz; i++) {
         uint16_t v = buffer[i];
         if (v != tbuf[i]) {
            if (errors<100) printf("Mismatch at %p: 0x%x should be 0x%x\n",&buffer[i],v,tbuf[i]);
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
   unsigned long errors = 0;
   const int sz = 16;
   volatile uint32_t * buffer32 = (volatile uint32_t *)(offset);
   volatile uint16_t * buffer16 = (volatile uint16_t *)(offset);
   volatile uint8_t  * buffer8  = (volatile uint8_t  *)(offset);

   printf("zz_perform_memtest... address 0x%lx write32 read32\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer32)+i;
         uint32_t value = rand();
         // write32
         *((uint32_t *)dir)=value;
         //read32
         uint32_t v = *((uint32_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%08lx should be 0x%08lx\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write32 read16\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer32)+i;
         uint32_t value = rand();
         // write32
         *((uint32_t *)dir)=value;
         //read16
         uint16_t vlow  = *((uint16_t *)dir+1);
         uint16_t vhigh = *((uint16_t *)dir);
         uint32_t v=(vhigh<<16)|vlow;
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%08lx should be 0x%08lx\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write32 read8\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer32)+i;
         uint32_t value = rand();
         // write32
         *((uint32_t *)dir)=value;
         //read8
         uint32_t vhh = *((uint8_t *)dir);
         uint32_t vlh = *((uint8_t *)dir+2);
         uint32_t vhl = *((uint8_t *)dir+1);
         uint32_t vll = *((uint8_t *)dir+3);
         uint32_t v=(vhh<<24)|(vhl<<16)|(vlh<<8)|vll;
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%08lx should be 0x%08lx\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write16 read32\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer16)+i;
         uint16_t value1 = rand();
         uint16_t value2 = rand();
         uint32_t value=(value1<<16)|value2;
         // write16
         *((uint16_t *)dir+1)=value2;
         *((uint16_t *)dir)=value1;
         //read32
         uint32_t v = *((uint32_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%08lx should be 0x%08lx\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write16 read16\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer16)+i;
         uint16_t value = rand();
         // write16
         *((uint16_t *)dir)=value;
         //read16
         uint16_t v = *((uint16_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%04x should be 0x%04x\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write16 read8\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer16)+i;
         uint16_t value = rand();
         // write16
         *((uint16_t *)dir)=value;
         //read8
         uint8_t vl = *((uint8_t *)dir+1);
         uint8_t vh = *((uint8_t *)dir);
         uint16_t v=(vh<<8)|vl;
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%04x should be 0x%04x\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write8 read32\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer8)+i;
         uint8_t value1 = rand();
         uint8_t value2 = rand();
         uint8_t value3 = rand();
         uint8_t value4 = rand();
         uint32_t value=(value1<<24)|(value2<<16)|(value3<<8)|value4;
         // write8
         *((uint8_t *)dir)=value1;
         *((uint8_t *)dir+2)=value3;
         *((uint8_t *)dir+1)=value2;
         *((uint8_t *)dir+3)=value4;
         //read32
         uint32_t v = *((uint32_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%08lx should be 0x%08lx\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write8 read16\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer8)+i;
         uint8_t value1 = rand();
         uint8_t value2 = rand();
         uint16_t value=(value1<<8)|value2;
         // write8
         *((uint8_t *)dir+1)=value2;
         *((uint8_t *)dir)=value1;
         //read16
         uint16_t v = *((uint16_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%04x should be 0x%04x\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("zz_perform_memtest... address 0x%lx write8 read8\n",offset);

   for (int k = 0; k < rep; k++) {
      if ((k % 128) == 0) {
         printf("`-- Test 0x%lx %3d/%d...\n", offset, k, rep);
      }

      for (int i=0; i<sz; i++) {
         uint32_t dir=(uint32_t)(buffer8)+i;
         uint8_t value = rand();
         // write8
         *((uint8_t *)dir)=value;
         //read16
         uint8_t v = *((uint8_t *)dir);
         if (v != value) {
            if (errors<100) printf("Mismatch at %p: 0x%02x should be 0x%02x\n",(uint32_t *)dir,v,value);
            errors++;
         }
      }
   }

   printf("Done. %ld errors.\n", errors);
   return errors;
}

ULONG zz_perform_memtest_fpgareg() {
   volatile uint16_t* d1 = (volatile uint16_t*)(zz_cd->cd_BoardAddr+0x1030);
   volatile uint16_t* d2 = (volatile uint16_t*)(zz_cd->cd_BoardAddr+0x1034);
   volatile uint16_t* dr = (volatile uint16_t*)(zz_cd->cd_BoardAddr+0x1030);

   printf("zz_perform_memtest_fpgareg...\n");

   *d2 = 1;
   for (int i = 0; i < 0x100000*2; i++) {
      *d1 = i;
   }

   printf("Done. Result: %x\n", *dr);

   return 0;
}

ULONG zz_perform_memtest_multi() {
   uint32_t offset = 0x100000;
   zz_perform_memtest(offset);
   zz_perform_memtest_rand(offset, 1024);
   printf("Testing CPU access to Z3660 Memory...\n");
   zz_perform_memtest_cross((unsigned long)zz_cd->cd_BoardAddr+offset, 1024);
   printf("Testing CPU access to CHIP...\n");
   zz_perform_memtest_cross(offset, 1024);
   //zz_perform_memtest_fpgareg();

   return 0;
}

VOID handleGadgetEvent(struct Window *win, struct Gadget *gad, ULONG code)
{
   switch (gad->GadgetID)
   {
      case MYGAD_BTN_REFRESH: {
         refresh_zz_info(win);
         break;
      }
      case MYGAD_BTN_TEST: {
         zz_perform_memtest_multi();
         break;
      }
      case MYGAD_JIT: {
         if(zz_get_emulation_used())
            zz_set_jit_enabled(code);
         else
            GT_SetGadgetAttrs(gads[MYGAD_JIT], win, NULL, GTCB_Checked, FALSE, TAG_END);
//            refresh_zz_info(win);
         break;
      }
      case MYGAD_LPF: {
         zz_set_lpf_freq(code);
         break;
      }
      case MYGAD_CPU_FREQ: {
// It doesn't work well...
//         zz_set_cpu_freq(code);
         break;
      }
      case MYGAD_LIST_BOOTMODE: {
         zz_set_selected_bootmode(win,code);
         break;
      }
      case MYGAD_BTN_APPLY_BOOTMODE: {
         zz_set_apply_bootmode();
         break;
      }
      case MYGAD_SCSIBOOT: {
         zz_set_scsiboot_enabled(code);
         break;
      }
      case MYGAD_AUTOCONFIG_RAM: {
         zz_set_autoconfig_ram_enabled(code);
         break;
      }
   }
}

struct Gadget *createAllGadgets(struct Gadget **glistptr, void *vi, UWORD topborder)
{
   struct NewGadget ng;
   struct Gadget *gad;

   gad = CreateContext(glistptr);

   ng.ng_LeftEdge   = 20;
   ng.ng_TopEdge    = 270+topborder;
   ng.ng_Width      = 100;
   ng.ng_Height     = 14;
   ng.ng_GadgetText = (STRPTR)"Bus Test";
   ng.ng_TextAttr   = &Topaz80;
   ng.ng_VisualInfo = vi;
   ng.ng_GadgetID   = MYGAD_BTN_TEST;
   ng.ng_Flags      = 0;
 
   gads[MYGAD_BTN_REFRESH] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
                              TAG_END);

   ng.ng_LeftEdge   = 160;
   ng.ng_GadgetID   = MYGAD_BTN_REFRESH;
   ng.ng_GadgetText = (STRPTR)"Refresh";

   gads[MYGAD_BTN_TEST] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
                              TAG_END);

   ng.ng_LeftEdge   = 160;
   ng.ng_TopEdge    = 20+topborder;
   ng.ng_GadgetID   = MYGAD_CPU_FREQ;
   ng.ng_GadgetText = (STRPTR)"CPU Frequency";

   gads[MYGAD_CPU_FREQ] = gad = CreateGadget(SLIDER_KIND, gad, &ng,
                              GTSL_Min, 50,
                              GTSL_Max, 100,
                              GTSL_Level, 100,
                              GTSL_LevelFormat, "%ld MHz",
                              GTSL_MaxLevelLen, 10,
                              GTSL_LevelPlace, PLACETEXT_ABOVE,
                              TAG_END);

   ng.ng_TopEdge    = 40+topborder;
   ng.ng_GadgetID   = MYGAD_FWVER;
   ng.ng_GadgetText = (STRPTR)"Firmware Version";

   gads[MYGAD_FWVER] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 60+topborder;
   ng.ng_GadgetID   = MYGAD_TEMP;
   ng.ng_GadgetText = (STRPTR)"Core Temperature C";

   gads[MYGAD_TEMP] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 80+topborder;
   ng.ng_GadgetID   = MYGAD_VAUX;
   ng.ng_GadgetText = (STRPTR)"Aux Voltage V";

   gads[MYGAD_VAUX] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 100+topborder;
   ng.ng_GadgetID   = MYGAD_VINT;
   ng.ng_GadgetText = (STRPTR)"Core Voltage V";

   gads[MYGAD_VINT] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 120+topborder;
   ng.ng_GadgetID   = MYGAD_LTC_TEMP;
   ng.ng_GadgetText = (STRPTR)"LTC Temperature C";

   gads[MYGAD_LTC_TEMP] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 140+topborder;
   ng.ng_GadgetID   = MYGAD_LTC_V1;
   ng.ng_GadgetText = (STRPTR)"LTC (3V3) Vdd V";

   gads[MYGAD_LTC_V1] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 160+topborder;
   ng.ng_GadgetID   = MYGAD_LTC_V2;
   ng.ng_GadgetText = (STRPTR)"LTC (5V) Vcc V";

   gads[MYGAD_LTC_V2] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 180+topborder;
   ng.ng_GadgetID   = MYGAD_LTC_060_TEMP;
   ng.ng_GadgetText = (STRPTR)"LTC (060 THERM) C";

   gads[MYGAD_LTC_060_TEMP] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 200+topborder;
   ng.ng_GadgetID   = MYGAD_Z9AX;
   ng.ng_GadgetText = (STRPTR)"Z3660 AHI";

   gads[MYGAD_Z9AX] = gad = CreateGadget(STRING_KIND, gad, &ng,
                              GTST_String, "",
                              TAG_END);

   ng.ng_TopEdge    = 220+topborder;
   ng.ng_GadgetID   = MYGAD_JIT;
   ng.ng_GadgetText = (STRPTR)"JIT enabled";

   gads[MYGAD_JIT]  = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
                                      GTCB_Scaled, FALSE, TAG_END);

   ng.ng_TopEdge    = 240+topborder;
   ng.ng_GadgetID   = MYGAD_LPF;
   ng.ng_GadgetText = (STRPTR)"Audio Lowpass";

   gads[MYGAD_LPF]  = gad = CreateGadget(SLIDER_KIND, gad, &ng,
                              GTSL_Min, 0,
                              GTSL_Max, 23900,
                              GTSL_Level, 23900,
                              GTSL_LevelFormat, "%ld Hz",
                              GTSL_MaxLevelLen, 10,
                              GTSL_LevelPlace, PLACETEXT_BELOW,
                              TAG_END);

   ng.ng_LeftEdge   = 280;
   ng.ng_TopEdge    = 20+topborder;
   ng.ng_Width      = 20*8;
   ng.ng_Height     = 10*NUM_BOOTMODES+2;
   ng.ng_GadgetID   = MYGAD_LIST_BOOTMODE;
   ng.ng_GadgetText = (STRPTR)"Boot Mode";
   struct List *dlist;
   dlist = AllocMem(sizeof(struct List), MEMF_CLEAR);
   NewList(dlist);
   for(int i=0;i<NUM_BOOTMODES;i++)
   {
        dnode[i].node.ln_Name = AllocMem(strlen(bootmode_names[i]) + 1, MEMF_CLEAR);
        strcpy(dnode[i].node.ln_Name, bootmode_names[i]);
      AddTail(dlist, (struct Node *) &dnode[i]);
   }
   int selected_option=0;
   gads[MYGAD_LIST_BOOTMODE] = gad = CreateGadget(LISTVIEW_KIND, gad, &ng,
                              GTLV_Labels, dlist,
                              GTLV_Selected, selected_option,
                              GTLV_ScrollWidth, 0,
                              GTLV_ShowSelected, NULL,
                              TAG_END);

   ng.ng_TopEdge    = 20+topborder+10*NUM_BOOTMODES+2;
   ng.ng_LeftEdge   = 280+14*8+(20*8-(100))/2;
   ng.ng_Height     = 14;
   ng.ng_Width      = 100;
   ng.ng_GadgetID   = MYGAD_SCSIBOOT;
   ng.ng_GadgetText = (STRPTR)"SCSI BOOT enabled";

   gads[MYGAD_SCSIBOOT] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
                                      GTCB_Scaled, FALSE, TAG_END);

   ng.ng_TopEdge    = 40+topborder+10*NUM_BOOTMODES+2;
   ng.ng_LeftEdge   = 280+14*8+(20*8-(100))/2;
   ng.ng_Height     = 14;
   ng.ng_Width      = 100;
   ng.ng_GadgetID   = MYGAD_AUTOCONFIG_RAM;
   ng.ng_GadgetText = (STRPTR)"AUTOC RAM enabled";

   gads[MYGAD_AUTOCONFIG_RAM] = gad = CreateGadget(CHECKBOX_KIND, gad, &ng,
                                      GTCB_Scaled, FALSE, TAG_END);

   ng.ng_TopEdge    = 60+topborder+10*NUM_BOOTMODES+2;
   ng.ng_LeftEdge   = 280+(20*8-(15*8+10))/2;
   ng.ng_Height     = 14;
   ng.ng_Width      = 15*8+10;
   ng.ng_GadgetID   = MYGAD_BTN_APPLY_BOOTMODE;
   ng.ng_GadgetText = (STRPTR)"Apply Boot Mode";

   gads[MYGAD_BTN_TEST] = gad = CreateGadget(BUTTON_KIND, gad, &ng,
                              TAG_END);

   return(gad);
}

VOID process_window_events(struct Window *mywin)
{
   struct IntuiMessage *imsg;
   ULONG imsgClass;
   UWORD imsgCode;
   struct Gadget *gad;
   BOOL terminated = FALSE;

   /*if((timerport = CreateMsgPort())) {
      if((timerio=(struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest)))) {
         if(OpenDevice((STRPTR) TIMERNAME, UNIT_MICROHZ, (struct IORequest *) timerio,0) == 0) {
            TimerBase = (struct Library *)timerio->tr_node.io_Device;
         }
         else {
            DeleteIORequest((struct IORequest *)timerio);
            DeleteMsgPort(timerport);
         }
      }
      else {
         DeleteMsgPort(timerport);
      }
   }

   if(!TimerBase) {
      errorMessage("Can't open timer.device");
      return;
   }

   timerio->tr_node.io_Command = TR_ADDREQUEST;
   timerio->tr_time.tv_secs = 1;
   timerio->tr_time.tv_micro = 0;
   SendIO((struct IORequest *) timerio);*/

   while (!terminated) {
      Wait ((1U << mywin->UserPort->mp_SigBit)); // | (1U << timerport->mp_SigBit) );

      /*if ((!terminated) && (1U << timerport->mp_SigBit)) {
         refresh_zz_info(mywin);
      }*/

      while ((!terminated) && (imsg = GT_GetIMsg(mywin->UserPort))) {
         gad = (struct Gadget *)imsg->IAddress;

         imsgClass = imsg->Class;
         imsgCode = imsg->Code;

         GT_ReplyIMsg(imsg);

         switch (imsgClass) {
            /* GadTools puts the gadget address into IAddress of IDCMP_MOUSEMOVE
            ** messages.   This is NOT true for standard Intuition messages,
            ** but is an added feature of GadTools.
            */
            case IDCMP_GADGETDOWN:
            case IDCMP_MOUSEMOVE:
            case IDCMP_GADGETUP:
               handleGadgetEvent(mywin, gad, imsgCode);
               break;
            case IDCMP_VANILLAKEY:
               //handleVanillaKey(mywin, imsgCode, slider_level);
               break;
            case IDCMP_CLOSEWINDOW:
               terminated = TRUE;
               break;
            case IDCMP_REFRESHWINDOW:
               /* With GadTools, the application must use GT_BeginRefresh()
               ** where it would normally have used BeginRefresh()
               */
               GT_BeginRefresh(mywin);
               GT_EndRefresh(mywin, TRUE);
               break;
         }
      }

      /*timerio->tr_node.io_Command = TR_ADDREQUEST;
      timerio->tr_time.tv_secs = 1;
      timerio->tr_time.tv_micro = 0;
      SendIO((struct IORequest *) timerio);*/
   }

   /*if(TimerBase) {
      WaitIO((struct IORequest *) timerio);
      CloseDevice((struct IORequest *) timerio);
      DeleteIORequest((struct IORequest *) timerio);
      DeleteMsgPort(timerport);
      TimerBase = NULL;
   }*/
}

VOID gadtoolsWindow(VOID) {
   struct TextFont *font;
   struct Screen      *mysc;
   struct Window      *mywin;
   struct Gadget      *glist;
   void                  *vi;
   UWORD                  topborder;

   if (NULL == (font = OpenFont(&Topaz80)))
      errorMessage("Failed to open Topaz 80");
   else {
      if (NULL == (mysc = LockPubScreen(NULL)))
         errorMessage("Couldn't lock default public screen");
      else {
         if (NULL == (vi = (void *)GetVisualInfo(mysc, TAG_END)))
            errorMessage("GetVisualInfo() failed");
         else {
            topborder = mysc->WBorTop + (mysc->Font->ta_YSize + 1);

            if (NULL == createAllGadgets(&glist, vi, topborder))
               errorMessage("createAllGadgets() failed");
            else {
               if (NULL == (mywin = OpenWindowTags(NULL,
                     WA_Title,              "Z3660 ZTop 1.01",
                     WA_Gadgets,    glist,   WA_AutoAdjust,     TRUE,
                     WA_Width,        460,   WA_MinWidth,        460,
                     WA_InnerHeight,  300,   WA_MinHeight,       220,
                     WA_DragBar,     TRUE,   WA_DepthGadget,    TRUE,
                     WA_Activate,    TRUE,   WA_CloseGadget,    TRUE,
                     WA_SizeGadget, FALSE,   WA_SimpleRefresh,  TRUE,
                     WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                     IDCMP_VANILLAKEY | SLIDERIDCMP | STRINGIDCMP |
                     BUTTONIDCMP,
                     WA_PubScreen, mysc,
                     TAG_END))) {
                  errorMessage("OpenWindow() failed");
               } else {
                  init_measures();
                  refresh_zz_info(mywin);
                  GT_RefreshWindow(mywin, NULL);
                  process_window_events(mywin);
                  CloseWindow(mywin);
               }
            }

            FreeGadgets(glist);
            FreeVisualInfo(vi);
         }
         UnlockPubScreen(NULL, mysc);
      }
      CloseFont(font);
   }
}

int main(void) {
   if (!(ExpansionBase = (struct Library*)OpenLibrary((CONST_STRPTR)"expansion.library",0L))) {
      errorMessage("Requires expansion.library");
      return 0;
   }

   zz_cd = (struct ConfigDev*)FindConfigDev(zz_cd,0x144B,0x1);
   if (!zz_cd) {
      CloseLibrary(ExpansionBase);
      errorMessage("Z3660 not found.\n");
      return 0;
   }

   zz_regs = (UBYTE*)zz_cd->cd_BoardAddr;
   CloseLibrary(ExpansionBase);

   if (NULL == (IntuitionBase = OpenLibrary((CONST_STRPTR)"intuition.library", 37)))
      errorMessage("Requires V37 intuition.library");
   else {
      if (NULL == (GfxBase = OpenLibrary((CONST_STRPTR)"graphics.library", 37)))
         errorMessage("Requires V37 graphics.library");
      else {
         if (NULL == (GadToolsBase = OpenLibrary((CONST_STRPTR)"gadtools.library", 37)))
            errorMessage("Requires V37 gadtools.library");
         else {
            gadtoolsWindow();
            CloseLibrary(GadToolsBase);
         }
         CloseLibrary(GfxBase);
      }
      CloseLibrary(IntuitionBase);
   }

   return 0;
}
