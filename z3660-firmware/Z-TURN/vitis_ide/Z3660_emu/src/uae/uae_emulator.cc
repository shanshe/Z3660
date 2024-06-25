/*
 * uae_emulator.cc
 *
 *  Created on: 2 ene. 2023
 *      Author: shanshe
 */
#include "uae/types.h"
#include "../main.h"
#include "sysconfig.h"
#include "xgpiops.h"
#include "maccess.h"
#include "memory.h"
#include "../memorymap.h"

LOCAL local;
extern int ovl;
#ifdef __cplusplus
extern "C" {
#endif
void cpu_emulator_reset_core0(void);
void cpu_set_fc(int fc);
void load_rom(void);
void load_romext(void);
void reset_autoconfig(void);
#ifdef __cplusplus
}
#endif

//cycle exact 68000
extern "C" void m68k_write_memory_32(unsigned int address, unsigned int value);
extern "C" void write_rtg_register(uint16_t zaddr,uint32_t zdata);
extern "C" void write_scsi_register(uint16_t zaddr,uint32_t zdata,int type);
extern "C" uint32_t read_scsi_register(uint16_t zaddr,int type);
extern "C" uint32_t read_rtg_register(uint16_t zaddr);
extern SHARED *shared;

void put_long_ce000 (uaecptr addr, uae_u32 v)
{
   m68k_write_memory_32((unsigned int)addr,(unsigned int)v);
}
void put_word_ce000 (uaecptr addr, uae_u32 v)
{
   m68k_write_memory_16(addr,v);
}
void put_byte_ce000 (uaecptr addr, uae_u32 v)
{
   m68k_write_memory_8(addr,v);
}
uae_u32 do_get_mem_long(uae_u32* a)
{
//    uint8_t* b = (uint8_t*)a;

//    return (*b << 24) | (*(b + 1) << 16) | (*(b + 2) << 8) | (*(b + 3));
    return((uae_u32)memory_get_long((uint32_t)a));
}

uint16_t do_get_mem_word(uint16_t* a)
{
//    uint8_t* b = (uint8_t*)a;

//    return (*b << 8) | (*(b + 1));
   return((uint16_t)memory_get_word((uint32_t)a));
}

uint8_t do_get_mem_byte(uint8_t* a)
{
//    return *a;
    return((uint8_t)memory_get_byte((uint32_t)a));
}

void do_put_mem_long(uae_u32* a, uae_u32 v)
{
//    uint8_t* b = (uint8_t*)a;

//    *b = v >> 24;
//    *(b + 1) = v >> 16;
//    *(b + 2) = v >> 8;
//    *(b + 3) = v;
    memory_put_long((unsigned int)a, (unsigned int)v);
}

void do_put_mem_word(uint16_t* a, uint16_t v)
{
//    uint8_t* b = (uint8_t*)a;

//    *b = v >> 8;
//    *(b + 1) = v;
   memory_put_word((unsigned int)a, (unsigned int)v);
}

void do_put_mem_byte(uint8_t* a, uint8_t v)
{
//    *a = v;
   memory_put_byte((unsigned int)a, (unsigned int)v);
}

#include "options.h"
#include "custom.h"
struct uae_prefs currprefs,changed_prefs;

#define write_log z3660_printf
//void write_log(const TCHAR *format, ...)
//{
//}

#include "uae_emulator.h"
#include "fpp.h"
#include "../main.h"
#include "newcpu.h"

void fill_prefetch_quick(void);
void m68k_reset_newcpu(bool hardreset);
void build_cpufunctbl (void);

bool is_cycle_ce(uaecptr address)
{
   return(false);
}
unsigned int direct_read_32(uaecptr add)
{
   return(swap32(*(uint32_t *)add));
}
unsigned int direct_read_16(uaecptr add)
{
   return(swap16(*(uint16_t *)add));
}
unsigned int direct_read_8(uaecptr add)
{
   return(*(uint8_t *)add);
}
void direct_write_32(uaecptr add, unsigned int data)
{
   *(uint32_t *)add=swap32(data);
}
void direct_write_16(uaecptr add, unsigned int data)
{
   *(uint16_t *)add=swap16(data);
}
void direct_write_8(uaecptr add, unsigned int data)
{
   *(uint8_t *)add=data&0xFF;
}
extern uint32_t autoConfigBaseFastRam;
extern uint32_t autoConfigBaseRTG;
extern volatile uint8_t *Z3660_RTG_BASE;
extern volatile uint8_t *Z3660_Z3RAM_BASE;

unsigned int z3ram_read_32(uaecptr address)
{
   uint32_t add=address-autoConfigBaseFastRam;
   return(swap32(*(uint32_t *)(Z3660_Z3RAM_BASE+add)));
}
unsigned int z3ram_read_16(uaecptr address)
{
   uint32_t add=address-autoConfigBaseFastRam;
   return(swap16(*(uint16_t *)(Z3660_Z3RAM_BASE+add)));
}
unsigned int z3ram_read_8(uaecptr address)
{
   uint32_t add=address-autoConfigBaseFastRam;
   return(*(uint8_t *)(Z3660_Z3RAM_BASE+add));
}
void z3ram_write_32(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseFastRam;
   *(uint32_t *)(Z3660_Z3RAM_BASE+add)=swap32(data);
}
void z3ram_write_16(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseFastRam;
   *(uint16_t *)(Z3660_Z3RAM_BASE+add)=swap16(data);
}
void z3ram_write_8(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseFastRam;
   *(uint8_t *)(Z3660_Z3RAM_BASE+add)=data&0xFF;
}
extern "C" uint32_t read_autoconfig(uint32_t address);
unsigned int auto_read_32(uaecptr address)
{
   z3660_printf("[Core1] Autoconfig BANK: Read LONG 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
   if((configured&local.z3_enabled)!=local.z3_enabled)
      return(read_autoconfig(address));
   else
      return(ps_read_32(address));
#else
   return(ps_read_32(address));
#endif
}
unsigned int auto_read_16(uaecptr address)
{
   z3660_printf("[Core1] Autoconfig BANK: Read WORD 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
   if((configured&local.z3_enabled)!=local.z3_enabled)
      return(read_autoconfig(address)>>16);
   else
      return(ps_read_16(address));
#else
   return(ps_read_16(address));
#endif
}
unsigned int auto_read_8(uaecptr address)
{
//   z3660_printf("Autoconfig BANK: Read 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
   if((configured&local.z3_enabled)!=local.z3_enabled)
      return(read_autoconfig(address)>>24);
   else
      return(ps_read_8(address));
#else
   return(ps_read_8(address));
#endif
}
extern "C" void write_autoconfig(uint32_t address, uint32_t data);
void auto_write_32(uaecptr address, unsigned int data)
{
   z3660_printf("[Core1] Autoconfig BANK: Write 0x%08X 0x%08X\n",address,data);
#ifdef AUTOCONFIG_ENABLED
   if((configured&local.z3_enabled)!=local.z3_enabled)
      write_autoconfig(address,data);
   else
      ps_write_32(address,data);
#else
      ps_write_32(address,data);
#endif
}
void auto_write_16(uaecptr address, unsigned int data)
{
   z3660_printf("[Core1] Autoconfig BANK: Write 0x%08X 0x%08X\n",address,data);
#ifdef AUTOCONFIG_ENABLED
   if((configured&local.z3_enabled)!=local.z3_enabled)
      write_autoconfig(address,data<<16);
   else
      ps_write_16(address,data);
#else
      ps_write_16(address,data);
#endif
}
void auto_write_8(uaecptr address, unsigned int data)
{
#ifdef AUTOCONFIG_ENABLED
   z3660_printf("[Core1] Autoconfig BANK: Write 0x%08X 0x%08X\n",address,data);
   if((configured&local.z3_enabled)!=local.z3_enabled)
      write_autoconfig(address,data<<24);
   else
      ps_write_8(address,data);
#else
      ps_write_8(address,data);
#endif
}
unsigned int rtg_regs_read_32(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
#define REG_ZZ_VBLANK_STATUS 0x17C
   if(add==REG_ZZ_VBLANK_STATUS)
   {
//         return(video_formatter_read(0));
#define VIDEO_FORMATTER_BASEADDR XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR
      return(*(uint32_t *)VIDEO_FORMATTER_BASEADDR);
   }
   if(add<0x6000)
   {
      if(add>=0x2000)
         return(read_scsi_register(add-0x2000,2));
      else
         return(read_rtg_register(add));
   }
   if(add>=0x80000)
      return(swap32(*(uint32_t*)(Z3660_RTG_BASE+add)));
   else
      return(read_scsi_register(add-0x2000,2));
}
unsigned int rtg_regs_read_16(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
   if(add<0x6000)
   {
      if(add>=0x2000)
         return(read_scsi_register(add-0x2000,1));
      else
         return(read_rtg_register(add));
   }
   if(add>=0x80000)
      return(swap16(*(uint16_t*)(Z3660_RTG_BASE+add)));
   else
      return(read_scsi_register(add-0x2000,1));
}
unsigned int rtg_regs_read_8(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
   if(add<0x6000)
   {
      if(add>=0x2000)
	     return(read_scsi_register(add-0x2000,0));
      else
      {
         uint32_t data=read_rtg_register(add&0x1FFFFC);
         switch(add&0x3)
         {
            case 0:
               return((data>>24)&0xFF);
            case 1:
            	return((data>>16)&0xFF);
            case 2:
            	return((data>>8 )&0xFF);
            case 3:
            	return((data    )&0xFF);
	     }
      }
   }
   if(add>=0x80000)
	   return(Z3660_RTG_BASE[add]);
   else
	   return(read_scsi_register(add-0x2000,0));
}
void rtg_regs_write_32(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
   *(uint32_t *)(Z3660_RTG_BASE+add)=swap32(data);
   if(add<0x6000)
   {
	   if(add<0x2000)
		  write_rtg_register(add,data);
	   else
		  write_scsi_register(add-0x2000,data,2);
   }
}
void rtg_regs_write_16(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
   *(uint16_t *)(Z3660_RTG_BASE+add)=swap16(data);
   if(add<0x6000)
   {
	   if(add<0x2000)
		  write_rtg_register(add,data);
	   else
		  write_scsi_register(add-0x2000,data,1);
   }
}
void rtg_regs_write_8(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
   Z3660_RTG_BASE[add]=data;
   if(add<0x6000)
   {
	   if(add<0x2000)
		   write_rtg_register(add,data);
	   else
		   write_scsi_register(add-0x2000,data,0);
   }
}
unsigned int rtg_read_32(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
   uint32_t data=swap32(*(uint32_t *)(Z3660_RTG_BASE+add));
//   printf("read32 rtg (%08lX) = %08lX\n",add,data);
//   if(add&3)
//      printf("rtg_read32 unaligned to 0x%08X\n",add);
   return(data);
}
unsigned int rtg_read_16(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
//   if(add&1)
//      printf("rtg_read16 unaligned to 0x%08X\n",add);
   uint32_t data=swap16(*(uint16_t *)(Z3660_RTG_BASE+add));
//   printf("read16 rtg (%08lX) = %08lX\n",add,data);
   return(data);
}
unsigned int rtg_read_8(uaecptr address)
{
   uint32_t add=address-autoConfigBaseRTG;
   uint32_t data=Z3660_RTG_BASE[add];
//   printf("read rtg (%08lX) = %08lX\n",add,data);
   return(data);
}
void rtg_write_32(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
//   printf("write32 rtg (%08lX) = %08lX\n",add,data);
//   if(add&3)
//      printf("rtg_write32 unaligned to 0x%08X\n",add);
   switch(add&3)
   {
      case 0:
         *(uint32_t *)(Z3660_RTG_BASE+add)=swap32(data);
         break;
      case 1:
         Z3660_RTG_BASE[add]=data>>24;
         *(uint16_t *)(Z3660_RTG_BASE+add+1)=swap16(data>>8);
         Z3660_RTG_BASE[add+3]=data;
         break;
      case 2:
         *(uint16_t *)(Z3660_RTG_BASE+add)=swap16(data>>16);
         *(uint16_t *)(Z3660_RTG_BASE+add+2)=swap16(data);
         break;
      case 3:
         Z3660_RTG_BASE[add]=data>>24;
         *(uint16_t *)(Z3660_RTG_BASE+add+1)=swap16(data>>8);
         Z3660_RTG_BASE[add+3]=data;
         break;
   }
}
void rtg_write_16(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
//   printf("write16 rtg (%08lX) = %08lX\n",add,data);
//   if(add&1)
//      printf("rtg_write16 unaligned to 0x%08X\n",add);
   *(uint16_t *)(Z3660_RTG_BASE+add)=swap16(data);
}
void rtg_write_8(uaecptr address, unsigned int data)
{
   uint32_t add=address-autoConfigBaseRTG;
//   printf("write8 rtg (%08lX) = %08lX\n",add,data);
   Z3660_RTG_BASE[add]=data;
}
unsigned int dummy_read(uaecptr add)
{
   return(0xFFFFFFFF);
}
void dummy_write(uaecptr add, unsigned int data)
{
}
uae_u8 *dummy_xlate(uaecptr add)
{
   return((uae_u8 *)add);
}
int dummy_check(uaecptr add, uae_u32)
{
   return(0);
}
int slow_check(uaecptr add, uae_u32)
{
   return(1);
}
int dflt_check(uaecptr add, uae_u32)
{
   return(1);
}
int auto_check(uaecptr add, uae_u32)
{
   return(1);
}
int mobo_check(uaecptr add, uae_u32)
{
   return(1);
}
int drct_check(uaecptr add, uae_u32)
{
   return(1);
}
int romd_check(uaecptr add, uae_u32)
{
   return(1);
}
int z3ram_check(uaecptr add, uae_u32)
{
   return(1);
}
int rtg_check(uaecptr add, uae_u32)
{
   return(1);
}
#define USE_MEM_BANKS
#ifdef USE_MEM_BANKS
#define MB_READ S_READ
#define MB_WRITE S_WRITE
#else
#define MB_READ 0
#define MB_WRITE 0
#endif
addrbank slow_bank = {
      read_long, read_word, read_byte,
      m68k_write_memory_32, m68k_write_memory_16, m68k_write_memory_8,
      dummy_xlate, slow_check, NULL, NULL, NULL,
      read_long, read_word,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
/*
addrbank prom_bank = {
      prom_read_long, prom_read_word, prom_read_byte,
      prom_m68k_write_memory_32, prom_m68k_write_memory_16, prom_m68k_write_memory_8,
      dummy_xlate, dummy_check, NULL, NULL, NULL,
      prom_read_long, prom_read_word,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
*/
addrbank auto_bank = {
      auto_read_32, auto_read_16, auto_read_8,
      auto_write_32, auto_write_16, auto_write_8,
      dummy_xlate, auto_check, NULL, NULL, NULL,
      auto_read_32, auto_read_16,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
addrbank mobo_bank = {
      ps_read_32, ps_read_16, ps_read_8,
      ps_write_32, ps_write_16, ps_write_8,
      dummy_xlate, mobo_check, NULL, NULL, NULL,
      ps_read_32, ps_read_16,
      ABFLAG_IO, MB_READ, MB_WRITE
};
addrbank chpr_bank = {
      ps_read_32, ps_read_16, ps_read_8,
      ps_write_32, ps_write_16, ps_write_8,
      dummy_xlate, mobo_check, NULL, NULL, NULL,
      ps_read_32, ps_read_16,
      ABFLAG_RAM | ABFLAG_CHIPRAM, MB_READ, MB_WRITE
};
addrbank mbrm_bank = {
      ps_read_32, ps_read_16, ps_read_8,
      ps_write_32, ps_write_16, ps_write_8,
      dummy_xlate, mobo_check, NULL, NULL, NULL,
      ps_read_32, ps_read_16,
      ABFLAG_RAM, MB_READ, MB_WRITE
};
addrbank drct_bank = {
      direct_read_32, direct_read_16, direct_read_8,
      direct_write_32, direct_write_16, direct_write_8,
      dummy_xlate, drct_check, NULL, NULL, NULL,
      direct_read_32, direct_read_16,
      ABFLAG_RAM | ABFLAG_DIRECTACCESS, 0, 0, // <--- Direct memory is faster if it's NOT "special_mem"
      NULL, // sub_banks
      0xFFFFFFFF, //mask
      0, // startmask
      0, // start
      0x08000000, // allocated_size 256 MByte
      0x08000000, // reserved_size 256 MByte
      (uae_u8*)0x08000000, // baseaddr_direct_r
      (uae_u8*)0x08000000, // baseaddr_direct_w
      0x08000000, // startaccessmask
};
addrbank romd_bank = {
      direct_read_32, direct_read_16, direct_read_8,
      dummy_write, dummy_write, dummy_write,
      dummy_xlate, romd_check, NULL, NULL, NULL,
      direct_read_32, direct_read_16,
      ABFLAG_ROM | ABFLAG_DIRECTACCESS, 0, 0, // <--- Direct memory is faster if it's NOT "special_mem"
      NULL, // sub_banks
      0xFFFFFFFF, //mask
      0, // startmask
      0, // start
      0x00080000, // allocated_size 512 KByte
      0x00080000, // reserved_size 512 KByte
      (uae_u8*)0x00F80000, // baseaddr_direct_r
      (uae_u8*)0, // baseaddr_direct_w
      0x00F80000, // startaccessmask
};
addrbank rome_bank = {
      direct_read_32, direct_read_16, direct_read_8,
      dummy_write, dummy_write, dummy_write,
      dummy_xlate, romd_check, NULL, NULL, NULL,
      direct_read_32, direct_read_16,
      ABFLAG_ROM | ABFLAG_DIRECTACCESS, 0, 0, // <--- Direct memory is faster if it's NOT "special_mem"
      NULL, // sub_banks
      0xFFFFFFFF, //mask
      0, // startmask
      0, // start
      0x00080000, // allocated_size 512 KByte
      0x00080000, // reserved_size 512 KByte
      (uae_u8*)0x00F00000, // baseaddr_direct_r
      (uae_u8*)0, // baseaddr_direct_w
      0x00F00000, // startaccessmask
};
addrbank z3ram_bank = {
      z3ram_read_32, z3ram_read_16, z3ram_read_8,
      z3ram_write_32, z3ram_write_16, z3ram_write_8,
      dummy_xlate, z3ram_check, NULL, NULL, NULL,
      z3ram_read_32, z3ram_read_16,
      ABFLAG_RAM | ABFLAG_DIRECTACCESS, 0, 0, // <--- Direct memory is faster if it's NOT "special_mem"
      NULL, // sub_banks
      0xFFFFFFFF, //mask
      0, // startmask
      0, // start
      0x10000000, // allocated_size 256 MByte
      0x10000000, // reserved_size 256 MByte
      (uae_u8*)0x20000000, // baseaddr_direct_r
      (uae_u8*)0x20000000, // baseaddr_direct_w
      0x20000000, // startaccessmask
};
addrbank rtg_regs_bank = {
      rtg_regs_read_32, rtg_regs_read_16, rtg_regs_read_8,
      rtg_regs_write_32, rtg_regs_write_16, rtg_regs_write_8,
      dummy_xlate, rtg_check, NULL, NULL, NULL,
      rtg_regs_read_32, rtg_regs_read_16,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
addrbank rtg_bank = {
      rtg_read_32, rtg_read_16, rtg_read_8,
      rtg_write_32, rtg_write_16, rtg_write_8,
      dummy_xlate, rtg_check, NULL, NULL, NULL,
      rtg_read_32, rtg_read_16,
      ABFLAG_NONE, MB_READ, MB_WRITE
/*      ABFLAG_RAM | ABFLAG_DIRECTACCESS, 0, 0, // <--- Direct memory is faster if it's NOT "special_mem"
      NULL, // sub_banks
      0xFFFFFFFF, //mask
      0, // startmask
      0, // start
      0x08000000, // allocated_size 128 MByte
      0x08000000, // reserved_size 128 MByte
      (uae_u8*)RTG_BASE, // baseaddr_direct_r
      (uae_u8*)RTG_BASE, // baseaddr_direct_w
      RTG_BASE, // startaccessmask
      */
};
addrbank dmmy_bank = {
      dummy_read, dummy_read, dummy_read,
      dummy_write, dummy_write, dummy_write,
      dummy_xlate, dummy_check, NULL, NULL, NULL,
      dummy_read, dummy_read,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
addrbank dflt_bank = {
      read_long, read_word, read_byte,
      m68k_write_memory_32, m68k_write_memory_16, m68k_write_memory_8,
      dummy_xlate, dflt_check, NULL, NULL, NULL,
      read_long, read_word,
      ABFLAG_NONE, MB_READ, MB_WRITE
};
#define RANGE_MAP(A,B,bank) do{for(unsigned int i=(A);i<(B);i++)\
      mem_banks[bankindex(i << 16)]=&bank;}while(0)
extern "C" void init_ovl_chip_ram_bank(void)
{
   RANGE_MAP(0x0000,0x0008,chpr_bank);
}
extern "C" void init_z3_ram_bank(unsigned int ini)
{
   uint32_t dir=ini<<4;
   RANGE_MAP(dir,dir+0x1000,z3ram_bank); // 256 MB Z3 RAM
}
extern "C" void init_rtg_bank(unsigned int ini)
{
   uint32_t dir=ini<<4;
   RANGE_MAP(dir+0x0000,dir+0x0020,rtg_regs_bank); // RTG Registers
   RANGE_MAP(dir+0x0020,dir+0x0800,rtg_bank); // RTG RAM
}
int maxcycles = 64*512; //256*512
extern void init_mem_banks (void);
extern void finish_Attributes(void);
extern uint32_t MMUTable;
extern uint32_t MMUL2Table[];
extern "C" void make_dummy_address_bank(uint32_t address)
{
   int add=address>>16;
   RANGE_MAP(add,add,dmmy_bank); // dummy
}
void uae_emulator(int enable_jit)
{
   z3660_printf("[Core1] Starting UAE%s emulator\n",enable_jit?"JIT":"");
   currprefs.cpu_model              = changed_prefs.cpu_model=68040;
   currprefs.fpu_model              = changed_prefs.fpu_model=68040;
   currprefs.mmu_model              = changed_prefs.mmu_model=68040;
   currprefs.cpu_compatible         = changed_prefs.cpu_compatible=false;
   currprefs.address_space_24       = changed_prefs.address_space_24=false;
   currprefs.cpu_cycle_exact        = changed_prefs.cpu_cycle_exact=false;
   currprefs.cpu_memory_cycle_exact = changed_prefs.cpu_memory_cycle_exact=false;
   currprefs.int_no_unimplemented   = changed_prefs.int_no_unimplemented=false;
   currprefs.fpu_no_unimplemented   = changed_prefs.fpu_no_unimplemented=false;
//   currprefs.blitter_cycle_exact    = changed_prefs.blitter_cycle_exact=false;
   currprefs.m68k_speed             = changed_prefs.m68k_speed=-1;//M68K_SPEED_25MHZ_CYCLES;
   currprefs.comptrustbyte          = changed_prefs.comptrustbyte=true;
   if(enable_jit)
   {
      currprefs.cachesize              = changed_prefs.cachesize=32*1024;
      currprefs.compfpu                = changed_prefs.compfpu=true;
   }
   else
   {
      currprefs.cachesize              = changed_prefs.cachesize=0;
      currprefs.compfpu                = changed_prefs.compfpu=false;
   }
   currprefs.fpu_strict             = changed_prefs.fpu_strict=true;

//   regs.natmem_offset=(uae_u8*)0x08000000;
   for(int i=0;i<MEMORY_BANKS;i++) // zero all memory banks vars
   {
      memset(&mem_banks[i],0,sizeof(addrbank));
   }
   RANGE_MAP(0x0000,MEMORY_BANKS,dflt_bank); // default all memory space
   if(local.load_rom_emu==1)
   {
      RANGE_MAP(0x0000,0x0008,slow_bank); // Slow bank ( ovl=1 -> ROM )
   }
   else
   {
      RANGE_MAP(0x0000,0x0008,chpr_bank); // Mother Board bank ( ovl=1 -> ROM )
   }
   RANGE_MAP(0x0008,0x00B8,chpr_bank); // Mother Board bank ( Chip RAM and Zorro II Expansion Space )
   RANGE_MAP(0x00BF,0x00C0,slow_bank); // Slow bank ( CIA ports & Timers ) <----- Amiga crashes with mobo_bank
   RANGE_MAP(0x00DC,0x00DD,mobo_bank); // Mother Board bank ( RTC, SCSI, Mobo Resources & Custom chips)
   RANGE_MAP(0x00DD,0x00DE,slow_bank); // Mother Board bank ( RTC, SCSI, Mobo Resources & Custom chips)
   RANGE_MAP(0x00DE,0x00E0,mobo_bank); // Mother Board bank ( RTC, SCSI, Mobo Resources & Custom chips)
   RANGE_MAP(0x00E8,0x00F0,mobo_bank); // Mother Board bank ( Zorro II Autoconfig )
   if(local.load_rom_emu==1)
   {
      for(int i=128;i<256;i++)
         MMUL2Table[i]=(0x00F00000+(i<<12))|0x5BA;
      RANGE_MAP(0x00F8,0x0100,romd_bank); // ROM direct bank ( ROM mapped )
   }
   else
   {
      for(int i=128;i<256;i++)
         MMUL2Table[i]=0;
      RANGE_MAP(0x00F8,0x0100,slow_bank);//mobo_bank); // Mother Board bank ( Mobo ROM )
   }
   if(local.load_romext_emu==1)
   {
      for(int i=0;i<128;i++)
         MMUL2Table[i]=(0x00F00000+(i<<12))|0x5BA;
      RANGE_MAP(0x00F0,0x00F8,rome_bank); // ROM direct bank ( ROM mapped )
   }
   else
   {
      for(int i=0;i<128;i++)
         MMUL2Table[i]=0;
      RANGE_MAP(0x00F0,0x00F8,slow_bank);//mobo_bank); // Mother Board bank ( Mobo ROM )
   }
   RANGE_MAP(0x0100,0x0800,dmmy_bank);//mbrm_bank); // Mother Board bank ( Mother board RAM )
   RANGE_MAP(0x0800,0x1000,drct_bank); // Direct bank ( CPU RAM )
   RANGE_MAP(0x1000,0x1800,dflt_bank); // Direct bank ( extended CPU RAM )
   RANGE_MAP(0x1800,0x4000,dmmy_bank); // Slow bank ( Z3 Expansion space )
   RANGE_MAP(0x4000,0x8000,slow_bank); // Slow bank ( Z3 Expansion space )
   RANGE_MAP(0xFF00,0xFF01,auto_bank); // Autoconfig bank ( Z3660 and Z3 AutoConfig )
   RANGE_MAP(0xFFFF,0xFFFF,dmmy_bank); // dummy
   uint32_t *ptr;
       ptr = &MMUTable;
   ptr[0x00F]=((uint32_t)&MMUL2Table)|0x1E1;

   finish_Attributes();
#ifdef SHOW_MMU_TABLES
   {
   	printf("Core 1 MMUTable 0x%08lX\n",(uint32_t)(&MMUTable));
   	for(uint32_t i=0;i<0x100;i++)
   	{
   		printf("0x%03lX ",i*8);
   		for(uint32_t j=0;j<8;j++)
   		{
   			uint32_t data=*((uint32_t *)(&MMUTable+(i*8+j)));
   			printf("%08lX ",data);
   		}
   		printf("\n");
   	}
   	printf("\n");
   	printf("Core 1 MMUL2Table 0x%08lX\n",(uint32_t)(&MMUL2Table));
   	for(uint32_t i=0;i<0x20;i++)
   	{
   		printf("0x%03lX ",i*8);
   		for(uint32_t j=0;j<8;j++)
   		{
   			uint32_t data=*((uint32_t *)((&MMUL2Table[0])+(i*8+j)));
   			printf("%08lX ",data);
   		}
   		printf("\n");
   	}
   }
#endif

   m68k_reset_newcpu(1);
   reset_autoconfig();

   init_m68k();
   build_cpufunctbl();
   m68k_setpc_normal (regs.pc);
   doint();
   fill_prefetch_quick();
   set_cycles (start_cycles);
   regs.stopped=false;
   set_special(0); //

   while(1)
   {
      m68k_go(1);
   }

}
extern XGpioPs GpioPs;
extern volatile uae_atomic uae_int_requested;
extern SHARED *shared;
int jit_enabled_last=-1;
void z3660_tasks(void)
{
   static long int count=1000000;
   if(--count>0) return;
   count=1000000;
   int jit_enabled=shared->jit_enabled;
   if(jit_enabled!=jit_enabled_last)
   {
      jit_enabled_last=jit_enabled;
      if(jit_enabled)
      {
         currprefs.cachesize = changed_prefs.cachesize=32*1024;
         currprefs.compfpu   = changed_prefs.compfpu=true;
      }
      else
      {
         currprefs.cachesize = changed_prefs.cachesize=0;
         currprefs.compfpu   = changed_prefs.compfpu=false;
      }
      set_special(SPCFLAG_MODE_CHANGE);
   }
#if 0
   if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
   {

      //      z3660_printf("Reset active (DOWN)...\n\r");
      while(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0){}
//      z3660_printf("Reset inactive (UP)...\n\r");
      cpu_emulator_reset_core0();
      ovl=1;
      m68k_reset_newcpu(1);
      reset_autoconfig();
      init_m68k();
      build_cpufunctbl();
      m68k_setpc_normal (regs.pc);
//      doint();
      fill_prefetch_quick ();
      set_cycles (start_cycles);
      regs.stopped=false;
      set_special(0);
      write_reg(0x10,0x80000000);
      dmb();
      write_reg(0x10,0x00000000);
   }
#endif
}
