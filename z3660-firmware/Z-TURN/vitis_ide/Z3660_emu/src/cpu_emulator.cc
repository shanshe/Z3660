/*
 * cpu_emulator.c
 *
 *  Created on: 3 dic. 2022
 *      Author: shanshe
 *
 *  Just a simple wrapper for Musashi
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "main.h"
#include "memorymap.h"
#include "uae/types.h"
#include "sleep.h"
#include "musashi/m68K.h"
#include <xgpiops.h>

#include "m68kcpu.h"
#include "m68k.h"
#include "cpu_emulator.h"
#include <xil_mmu.h>
#include "xil_cache.h"
//uint32_t video_formatter_read(uint16_t op);
void write_rtg_register(uint16_t zaddr,uint32_t zdata);
void write_scsi_register(uint16_t zaddr,uint32_t zdata,int type);
uint32_t read_scsi_register(uint16_t zaddr,int type);
void reset_autoconfig(void);
extern "C" void init_ovl_chip_ram_bank(void);
extern "C" void init_z3_ram_bank(void);
extern "C" void init_rtg_bank(void);

extern XGpioPs GpioPs;
int cpu_type=M68K_CPU_TYPE_68030;
char disasm_buf[4096];
uint8_t *ROM=(uint8_t *)0x00F80000;
void load_rom(int load);
volatile uint8_t *RAM=(volatile uint8_t *)0x08000000;
volatile uint8_t *Z3660_RTG_BASE=(volatile uint8_t *)RTG_BASE;
volatile uint8_t *Z3660_Z3RAM_BASE=(volatile uint8_t *)0x20000000;
int disasm_enable=0;
int ovl=1;
extern SHARED *shared;

#define MAP_ROM

void cpu_emulator_reset_core0(void);
void cpu_emulator_reset(void)
{
	cpu_emulator_reset_core0();
	ovl=1;
	m68k_pulse_reset();
	reset_autoconfig();
	*(uint32_t *)0x83c00000=0x80000000;
	dsb();
	*(uint32_t *)0x83c00000=0x00000000;
}
//int irq=0;
//int last_cpu_level=0;
//int last_irq=-1;
//int mask0=0,mask1=0;
void other_tasks(void);
#define BIT0  (1<<0)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT15 (1<<15)

int intlev(void);
extern "C" void z3660_printf(const TCHAR *format, ...);

void cpu_emulator(void)
{
#ifdef MAP_ROM
	load_rom(1); // Load ROM
#else
	load_rom(2); // Do not load ROM
#endif
	z3660_printf("[Core1] Starting emulator\n");
//	m68ki_cpu_core *state= &m68ki_cpu;
	m68k_set_cpu_type(cpu_type);
	m68k_init();
	cpu_emulator_reset();

	while(1)
	{
		int irq=intlev();
		if(irq<0) irq=0;
//		if(irq!=last_irq)
			m68k_set_irq(irq);
//		last_irq=irq;

		if(disasm_enable==0)
			m68k_execute(100);
		else
		{
			m68k_disassemble(disasm_buf, m68k_get_reg(NULL, M68K_REG_PC), cpu_type);
		    	z3660_printf("REGA: 0:$%.8X 1:$%.8X 2:$%.8X 3:$%.8X 4:$%.8X 5:$%.8X 6:$%.8X 7:$%.8X\n", m68k_get_reg(NULL, M68K_REG_A0), m68k_get_reg(NULL, M68K_REG_A1), m68k_get_reg(NULL, M68K_REG_A2), m68k_get_reg(NULL, M68K_REG_A3), \
		            m68k_get_reg(NULL, M68K_REG_A4), m68k_get_reg(NULL, M68K_REG_A5), m68k_get_reg(NULL, M68K_REG_A6), m68k_get_reg(NULL, M68K_REG_A7));
		    	z3660_printf("REGD: 0:$%.8X 1:$%.8X 2:$%.8X 3:$%.8X 4:$%.8X 5:$%.8X 6:$%.8X 7:$%.8X\n", m68k_get_reg(NULL, M68K_REG_D0), m68k_get_reg(NULL, M68K_REG_D1), m68k_get_reg(NULL, M68K_REG_D2), m68k_get_reg(NULL, M68K_REG_D3), \
		            m68k_get_reg(NULL, M68K_REG_D4), m68k_get_reg(NULL, M68K_REG_D5), m68k_get_reg(NULL, M68K_REG_D6), m68k_get_reg(NULL, M68K_REG_D7));
		    	z3660_printf("%.8X (%.8X)]] %s\n", m68k_get_reg(NULL, M68K_REG_PC), (m68k_get_reg(NULL, M68K_REG_PC) & 0xFFFFFF), disasm_buf);
		    	m68k_execute(1);
		}
/*
		if(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0)
		{
			z3660_printf("[Core1] Reset active (DOWN)...\n\r");
			while(XGpioPs_ReadPin(&GpioPs, n040RSTI)==0){}
			z3660_printf("[Core1] Reset inactive (UP)...\n\r");
			cpu_emulator_reset();
		}
*/
//		if( (m68k_get_reg(NULL, M68K_REG_D4)&0xFFFFFFFF)==0x3030302D)
//		if( (m68k_get_reg(NULL, M68K_REG_A5)&0xFFFFFFFF)==0x3030302D)
//		if( (m68k_get_reg(NULL, M68K_REG_PC)&0xFFFF0000)==0x50000000)
//		if( m68k_get_reg(NULL, M68K_REG_PC)==0xF80CDC)
//		if( m68k_get_reg(NULL, M68K_REG_PC)==0x4048)
//		if( m68k_get_reg(NULL, M68K_REG_A4)==0xDE0003)
//			disasm_enable=1;
	}
}
uint32_t autoConfigBaseFastRam=0;
uint32_t autoConfigBaseRTG=0;
int configured=0;
int enabled=1+2; // enabled RAM and RTG autoconfig
int shutup=0;
void reset_autoconfig(void)
{
	configured=0;
	autoConfigBaseFastRam=0;
	autoConfigBaseRTG=0;
	shutup=0;
}
#define MANUF_ID 0x144B // Double H Tech
#define AUTOC_NIBBLE(CODE,NIBBLE) ((~(unsigned long)CODE)<<(28-NIBBLE*4))|0x0FFFFFFFUL

extern "C" uint32_t read_autoconfig(uint32_t address)
{
	uint32_t data;
#ifdef AUTOCONFIG_ENABLED
	data=0xFFFFFFFF;
	switch(address&0xFFFF)
	{
		case 0x0000:
			if((configured&1) == 0 && (enabled&1) == 1) data = 0xAFFFFFFF; // 0b1010 zorro 3 (10), pool link (1), autoboot ROM no (0)
			else
			{
				if(shared->boot_rom_loaded)
				{
					if((configured&2) == 0 && (enabled&2) == 2) data = 0x9FFFFFFF; // 0b1001 zorro 3 (10), no pool link (0), autoboot ROM yes (0)
				}
				else
				{
					if((configured&2) == 0 && (enabled&2) == 2) data = 0x8FFFFFFF; // 0b1000 zorro 3 (10), no pool link (0), autoboot ROM no (0)
				}
			}
			break;
		case 0x0100:
			if((configured&1) == 0 && (enabled&1) == 1) data = 0x4FFFFFFF; // 0b0100 next board unrelated (0), 256MB FastRAM
			else
			if((configured&2) == 0 && (enabled&2) == 2) data = 0xBFFFFFFF; // 0b1011 next board unrelated (0), 128MB RTG
			break;
		case 0x0004:
			data = 0xFFFFFFFF; // 0b1111 product number
			break;
		case 0x0104:
			if((configured&1) == 0 && (enabled&1) == 1) data = 0xDFFFFFFF; // 0b1101 2 for the 256MB Z3 Fast
			else
			if((configured&2) == 0 && (enabled&2) == 2) data = 0xEFFFFFFF; // 0b1110 1 for the RTG PIC
			break;
		case 0x0008:
			if((configured&1) == 0 && (enabled&1) == 1) data = 0x8FFFFFFF; // 0b1000 flags inverted 0111 io,shutup,extension,reserved(1)
			else
			if((configured&2) == 0 && (enabled&2) == 2) data = 0x8FFFFFFF; // 0b1000 flags inverted 0111 io,shutup,extension,reserved(1)
			break;
		case 0x0108:
			data = 0xFFFFFFFF; // inverted zero
			break;
		case 0x000C: data = 0xFFFFFFFF; break; // reserved?
		case 0x010C: data = 0xFFFFFFFF; break; // reserved?

		case 0x0010: data = AUTOC_NIBBLE(MANUF_ID,3); break; // manufacturer high byte inverted
		case 0x0110: data = AUTOC_NIBBLE(MANUF_ID,2); break; //
		case 0x0014: data = AUTOC_NIBBLE(MANUF_ID,1); break; // manufacturer low byte
		case 0x0114: data = AUTOC_NIBBLE(MANUF_ID,0); break; //

		case 0x0028: data = shared->boot_rom_loaded?0x9FFFFFFF:0xFFFFFFFF; break; // autoboot rom vector (er_InitDiagVec)
		case 0x0128: data = shared->boot_rom_loaded?0xFFFFFFFF:0xFFFFFFFF; break; // ~0x6000
		case 0x002C: data = shared->boot_rom_loaded?0xFFFFFFFF:0xFFFFFFFF; break; //
		case 0x012C: data = shared->boot_rom_loaded?0xFFFFFFFF:0xFFFFFFFF; break; //

		default: data = 0xFFFFFFFF;
	}
	return(data);
#else
	return(0);
#endif
}
extern u32 MMUTable;
void setMMU(INTPTR Addr, u32 attrib)
{
	u32 *ptr;
	u32 section;

	section = Addr / 0x100000U;
	ptr = &MMUTable;
	ptr += section;
	if(ptr != NULL) {
		*ptr = attrib;
	}
}
void finish_MMU_OP(void)
{
	Xil_DCacheFlush();

	mtcp(XREG_CP15_INVAL_UTLB_UNLOCKED, 0U);
	/* Invalidate all branch predictors */
	mtcp(XREG_CP15_INVAL_BRANCH_ARRAY, 0U);

	dsb(); /* ensure completion of the BP and TLB invalidation */
    isb(); /* synchronize context on this processor */
}
extern "C" void write_autoconfig(uint32_t address, uint32_t data)
{
#ifdef AUTOCONFIG_ENABLED
	switch(address&0xFC)
	{
		case 0x44:

            if ((configured&1) == 0 && (enabled&1) == 1)
            {
                autoConfigBaseFastRam = data&0xFFFF0000;     // FastRAM
                configured|=1;
                unsigned int ini=(autoConfigBaseFastRam>>20)&0xFFF; // should be 0x400
               	z3660_printf("Warning: autoconfig Z3 RAM to 0x%03X (should be 0x400)\n",ini);
                unsigned int end=ini+0x100; // +256 MByte
                // The following MMU operation hangs the access of the core0
                // so we hold here core0
        		shared->core0_hold=1;
                shared->shared_data=1;
                while(shared->core0_hold_ack==0);
                for(unsigned int i=ini,j=0;i<end;i++,j++)
                	setMMU(i*0x100000UL,(0x20000000+j*0x100000UL)|NORM_WB_CACHE|SHAREABLE); // mapped to 0x20000000
                finish_MMU_OP();
                init_z3_ram_bank();
				// core0 continues
                shared->core0_hold=0;
            }
            else
            if ((configured&2) == 0 && (enabled&2) == 2)
            {
                autoConfigBaseRTG = data&0xFFFF0000;         // RTG
                configured|=2;
                unsigned int ini=(autoConfigBaseRTG>>20)&0xFFF; // should be 0x500
               	z3660_printf("Warning: autoconfig RTG RAM to 0x%03X (should be 0x500)\n",ini);
                unsigned int end=ini+0x080; // +128 MByte
                // The following MMU operation hangs the access of the core0
                // so we hold here core0
        		shared->core0_hold=1;
                shared->shared_data=1;
                while(shared->core0_hold_ack==0);
                // +2 -> don't use RTG registers with MMU (write to RTG registers is emulated)
                for(unsigned int i=ini+2,j=2;i<end;i++,j++)
                {
                	uint32_t address=(RTG_BASE+j*0x100000UL);
                	setMMU(address,address|NORM_WB_CACHE);      // mapped to RTG_BASE
                	setMMU(i*0x100000UL,address|NORM_WT_CACHE); // mapped to RTG_BASE
                }
            	setMMU((end-2)*0x100000UL,0x1FE00000|STRONG_ORDERED); // mapped to RTG_BASE
//            	setMMU((end-1)*0x100000UL,0x1FF00000|STRONG_ORDERED); // mapped to RTG_BASE
                finish_MMU_OP();
                init_rtg_bank();
                // core0 continues
                shared->core0_hold=0;

            }
            break;

		case 0x4C:
            if (configured&1) shutup|=1;   // FastRAM
            if (configured&2) shutup|=2;   // RTG
            break;
	}
#endif
}
extern "C" void bus_error(void);
inline int not_decode(uint32_t address)
{
//	return(0);
#if 1
//	if(address>=0xF0000000 && address<0xFF000000)
//		return(1);
	if(0
		||(address>=0x18000000 && address<0x40000000)
		||(address>=0x78000000 && address<0xFF000000)
//		||(address>=RTG_BASE && address<0x40000000)
		||(address>=0x00E00000 && address<0x00E80000)
//		||(address>=0x00DD0000 && address<0x00DE0000) mobo IDE (SCSI control)
		||(address>=0x00C00000 && address<0x00DC0000)
		||(address>=0x00B80000 && address<0x00BF0000)
		||(address>=0x80000000 && address<0xFF000000)
		||(address>=0xFF00F000)
	)
	{
//		z3660_printf("access to address 0x%08X\n",address);
//		bus_error();
		return(1);
	}
	return(0);
#endif
}

unsigned int read_long(unsigned int address)
{
	uint32_t data;
#ifdef MAP_ROM
	if(ovl==1 && address<0x00080000)
	{
		return(swap32(*(uint32_t*)(ROM+address)));
	}
	if(address>=0x00f80000 && address<0x1000000)
	{
		uint32_t add=address-0x00f80000;
		return(swap32(*(uint32_t*)(ROM+add)));
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		return(ps_read_32(address));
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		return(swap32(*(((uint32_t*)(RAM+add)))));
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return(0);
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		return(swap32(*(((uint32_t*)(Z3660_Z3RAM_BASE+add)))));
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		if(add==0x7c)
		{
//			return(video_formatter_read(0));
#define VIDEO_FORMATTER_BASEADDR XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_VIDEO_VIDEO_FORMATTER_0_BASEADDR
			return(*(uint32_t *)VIDEO_FORMATTER_BASEADDR);
		}
		if(add<0x100000)
		{
			if(add>=0x2000)
				return(read_scsi_register(add-0x2000,2));
		}
		return(swap32(*(uint32_t*)(Z3660_RTG_BASE+add)));
	}
	if(not_decode(address))
	{
//		z3660_printf(" Read Long\n");
		return(0);
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
		z3660_printf("[Core1] Autoconfig: Read LONG 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
		if((configured&enabled)!=enabled)
			return(read_autoconfig(address));
#else
		return(ps_read_32(address));
#endif
	}
	return(ps_read_32(address));
}
unsigned int read_word(unsigned int address)
{
	uint32_t data;
#ifdef MAP_ROM
	if(ovl==1 && address<0x00080000)
	{
		return(swap16(*(uint16_t *)(ROM+address)));
	}
	if(address>=0x00f80000 && address<0x01000000)
	{
		uint32_t add=address-0x00f80000;
		return(swap16(*(uint16_t *)(ROM+add)));
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		return(ps_read_16(address));
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		return(swap16(*(uint16_t *)(RAM+add)));
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return(0);
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		return(swap16(*(uint16_t *)(Z3660_Z3RAM_BASE+add)));
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		if(add<0x100000)
		{
			if(add>=0x2000)
				return(read_scsi_register(add-0x2000,1));
		}
		return(swap16(*(uint16_t *)(Z3660_RTG_BASE+add)));
	}
	if(not_decode(address))
	{
//		z3660_printf(" Read Word\n");
		return(0);
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
		z3660_printf("[Core1] Autoconfig: Read WORD 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
		if((configured&enabled)!=enabled)
			return(read_autoconfig(address)>>16);
#else
		return(ps_read_16(address));
#endif
	}
	return(ps_read_16(address));
}
unsigned int read_byte(unsigned int address)
{
	uint32_t data;
#ifdef MAP_ROM
	if(ovl==1 && address<0x00080000)
	{
		data=ROM[address];
		return(data);
	}
	if(address>=0x00f80000 && address<0x1000000)
	{
		uint32_t add=address-0x00f80000;
		data=ROM[add];
		return(data);
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		return(ps_read_8(address));
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		data=RAM[add];
		return(data);
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return(0);
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		data=Z3660_Z3RAM_BASE[add];
		return(data);
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		if(add<0x100000)
		{
			if(add>=0x2000)
				return(read_scsi_register(add-0x2000,0));
		}
		return(Z3660_RTG_BASE[add]);
	}
	if(not_decode(address))
	{
//		z3660_printf(" Read Byte\n");
		return(0);
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
//		z3660_printf("Autoconfig: Read 0x%08lX\n",address);
#ifdef AUTOCONFIG_ENABLED
		if((configured&enabled)!=enabled)
			return(read_autoconfig(address)>>24);
#else
		return(ps_read_8(address));
#endif
	}
	return(ps_read_8(address));
}
unsigned int  m68k_read_memory_8(unsigned int address)
{
	return(read_byte(address));
}
unsigned int  m68k_read_memory_16(unsigned int address)
{
	return(read_word(address));
}
unsigned int  m68k_read_memory_32(unsigned int address)
{
	return(read_long(address));
}
void m68k_write_memory_8(unsigned int address, unsigned int value)
{
#ifdef MAP_ROM
#define CIAAPRA 0xBFE001
	if(address==CIAAPRA)
	{
		if (ovl != (value & (1 << 0)))
		{
			ovl = (value & (1 << 0));
			z3660_printf("[Core1] OVL:%x\n", ovl);
			if(ovl==0)
			{
				init_ovl_chip_ram_bank();
			}
		}
	}
	if(ovl==1 && address<0x00800000)
	{
		return;
	}
	if(address>=0x00F80000 && address<0x01000000)
	{
		return;
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		ps_write_8(address,value);
		return;
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		RAM[add]=value&0xFF;
		return;
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return;
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		Z3660_Z3RAM_BASE[add]=value&0xFF;
		return;
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		Z3660_RTG_BASE[add]=value&0xFF;
		if(add<0x100000)
		{
			if(add<0x2000)
				write_rtg_register(add,value);
			else
				write_scsi_register(add-0x2000,value,0);
		}
		return;
	}
	if(not_decode(address))
	{
//		z3660_printf(" Write Byte\n");
		return;
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
		z3660_printf("[Core1] Autoconfig: Write 0x%08X 0x%08X\n",address,value);
		if((configured&enabled)!=enabled)
		{
#ifdef AUTOCONFIG_ENABLED
			write_autoconfig(address,value<<24);
#else
			ps_write_8(address,value);
#endif
			return;
		}
	}
	ps_write_8(address,value);
}
//void m68k_write_memory_16(uint32_t address, uint32_t value)
void m68k_write_memory_16(unsigned int address, unsigned int value)
{
#ifdef MAP_ROM
	if(ovl==1 && address<0x00800000)
	{
		return;
	}
	if(address>=0x00F80000 && address<0x01000000)
	{
		return;
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		ps_write_16(address,value);
		return;
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		*(uint16_t*)(RAM+add)=swap16(value);
		return;
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return;
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		*(uint16_t*)(Z3660_Z3RAM_BASE+add)=swap16(value);
		return;
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		*(uint16_t*)(Z3660_RTG_BASE+add)=swap16(value);
		if(add<0x100000)
		{
			if(add<0x2000)
				write_rtg_register(add,value);
			else
				write_scsi_register(add-0x2000,value,1);
		}
		return;
	}
	if(not_decode(address))
	{
//		z3660_printf(" Write Word\n");
		return;
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
		z3660_printf("[Core1] Autoconfig: Write 0x%08X 0x%08X\n",address,value);
		if((configured&enabled)!=enabled)
		{
#ifdef AUTOCONFIG_ENABLED
			write_autoconfig(address,value<<16);
#else
			ps_write_16(address,value);
#endif
			return;
		}
	}
	ps_write_16(address,value);
}
//void m68k_write_memory_32(uint32_t address, uint32_t value)
void m68k_write_memory_32(unsigned int address, unsigned int value)
{
#ifdef MAP_ROM
	if(ovl==1 && address<0x00800000)
	{
		return;
	}
	if(address>=0x00F80000 && address<0x01000000)
	{
		return;
	}
#endif
	if(address<0x00E00000) // CHIP and Amiga Resources
	{
		ps_write_32(address,value);
		return;
	}
#ifdef CPU_RAM
	if(address>=0x08000000 && address<0x18000000)
	{
		uint32_t add=address-0x08000000;
		*(((uint32_t*)(RAM+add)))=swap32(value);
		return;
	}
#else
	if(address>=0x08000000 && address<0x18000000)
		return;
#endif
	if(address>=autoConfigBaseFastRam && address<autoConfigBaseFastRam+0x10000000 && (configured&1))
	{
		uint32_t add=address-autoConfigBaseFastRam;
		*(((uint32_t*)(Z3660_Z3RAM_BASE+add)))=swap32(value);
		return;
	}
	if(address>=autoConfigBaseRTG && address<autoConfigBaseRTG+0x08000000 && (configured&2))
	{
		uint32_t add=address-autoConfigBaseRTG;
		*(((uint32_t*)(Z3660_RTG_BASE+add)))=swap32(value);
		if(add<0x100000)
		{
			if(add<0x2000)
				write_rtg_register(add,value);
			else
				write_scsi_register(add-0x2000,value,2);
		}
		return;
	}
	if(not_decode(address))
	{
//		z3660_printf(" Write Long\n");
		return;
	}
	if(address>=0xFF000000 && address<0xFF010000)
	{
		z3660_printf("[Core1] Autoconfig: Write 0x%08X 0x%08X\n",address,value);
		if((configured&enabled)!=enabled)
		{
#ifdef AUTOCONFIG_ENABLED
			write_autoconfig(address,value);
#else
			ps_write_32(address,value);
#endif
			return;
		}
	}
	ps_write_32(address,value);
}
#define NOPX_WRITE \
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;

uint32_t nop_read_max=24;
uint32_t nop_write_max=0;
// 0 1.61
// 1 1.61
// 2 1.62
// 3 1.61
// 4 1.70
// 5 1.59
// 6 1.51
// 7 1.61
// 8 1.54
// 9 1.54
//10 1.59
//11 1.60
//12 1.66
//13 1.61
//14 1.55
//15 1.55
//16 1.73
//17 1.59

#define NOP asm(" nop")

#define NOPX_READ2 do {for(int i=nop_read_max;i>0;i--) NOP;}while(0)
#define NOPX_WRITE2 do {for(int i=nop_write_max;i>0;i--) NOP;}while(0)
#define NOPX_READ_WORD \
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;
// 969 965 951
#define NOPX_READ_LONG \
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;\
		NOP;NOP;NOP;NOP;NOP;

#define NOPX_READ_BYTE NOPX_READ_WORD
// max 1.70 ???
// # of NOPS in NOPX   Sysinfo Chip Speed vs A600
// 55                   -> 1.66
// 50                   -> 1.70
// 45                   -> 1.66
// 40                   -> 1.66
// 35                   -> 1.64
// 30                   -> 1.61
// 25                   ->
// 20                   ->
// 15                   ->
// 10                   ->
//  5                   ->
//  3                   ->
//  1                   -> 1.69
//  0                   -> 1.70 <---------------
#define check_bus_error(A,B)
/*
void check_bus_error(uint32_t v,uint32_t address)
{
	if(v!=1)
	{
		printf("W %s 0x%08x\n",v==2?"Bus error":"Retry",address);
//		if(v==2) bus_error();
	}
}
*/
#define WRITE_FINISH_DELAYED
//#define WRITE_THROUGH_REGS

int write_pending=0;
uint32_t last_bank=-1;
inline void arm_write_amiga_long(uint32_t address, uint32_t data)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);        // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
#ifdef WRITE_THROUGH_REGS
//	write_reg64(0x08,(((uint64_t)data)<<32)|address);
	write_reg(0x08,address);
	dsb();
	write_reg(0x0C,data);
	dsb();
	write_reg(0x10,0x11|WRITE_|LONG_); // command write
	dsb();
#else
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	write_mem32(address,data);
#endif
	write_pending=1;
#ifndef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);        // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
}
inline void arm_write_amiga_word(uint32_t address, uint32_t data)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);        // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
#ifdef WRITE_THROUGH_REGS
//	write_reg64(0x08,(((uint64_t)data)<<32)|address);
	write_reg(0x08,address);
	dsb();
	write_reg(0x0C,data);
	dsb();
	write_reg(0x10,0x11|WRITE_|WORD_); // command write
	dsb();
#else
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	write_mem16(address,data);
#endif
	write_pending=1;
#ifndef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);        // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
}
inline void arm_write_amiga_byte(uint32_t address, uint32_t data)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);       // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
#ifdef WRITE_THROUGH_REGS
//	write_reg64(0x08,(((uint64_t)data)<<32)|address);
	write_reg(0x08,address);
	dsb();
	write_reg(0x0C,data);
	dsb();
	write_reg(0x10,0x11|WRITE_|BYTE_); // command write
	dsb();
#else
	uint32_t bank=(address>>24)&0xFF;
	if(bank!=last_bank)
	{
		write_reg(0x18,bank);
		last_bank=bank;
	}
	write_mem8(address,data);
#endif
	write_pending=1;
#ifndef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);        // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
}
inline uint32_t arm_read_amiga_long(uint32_t address)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);       // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
	NOP;
	write_reg(0x08,address);           // address
	NOP;
	write_reg(0x10,0x11|READ_|LONG_);  // command read
	do {
		NOPX_READ_LONG;
	}
	while(read_reg(0x14)==0);          // read ack
	check_bus_error(read_reg(0x14),address);
	uint32_t data_read=read_reg(0x1C); // read data
	return(data_read);
}
inline uint32_t arm_read_amiga_word(uint32_t address)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);       // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
	NOP;
	write_reg(0x08,address);           // address
	NOP;
	write_reg(0x10,0x11|READ_|WORD_);  // command read
	do {
		NOPX_READ_WORD;
	}
	while(read_reg(0x14)==0);          // read ack
	check_bus_error(read_reg(0x14),address);
	uint32_t data_read=read_reg(0x1C); // read data
	return(data_read);
}
inline uint32_t arm_read_amiga_byte(uint32_t address)
{
#ifdef WRITE_FINISH_DELAYED
	if(write_pending)
	{
		write_pending=0;
		do {
			NOPX_WRITE;
		}
		while(read_reg(0x14)==0);       // read ack
		check_bus_error(read_reg(0x14),address);
	}
#endif
	NOP;
	write_reg(0x08,address);           // address
	NOP;
	write_reg(0x10,0x11|READ_|BYTE_);  // command read
	do {
		NOPX_READ_BYTE;
	}
	while(read_reg(0x14)==0);          // read ack
	check_bus_error(read_reg(0x14),address);
	uint32_t data_read=read_reg(0x1C); // read data
	return(data_read);
}

extern "C" void ps_write_32(unsigned int address, unsigned int value)
{
	switch(address&3)
	{
		case 0:
			arm_write_amiga_long(address,value);
			break;
		case 1:
			arm_write_amiga_byte(address  ,value>> 8);
			arm_write_amiga_word(address+1,value>> 8);
			arm_write_amiga_byte(address+3,value<<24);
			break;
		case 2:
			arm_write_amiga_word(address  ,value>>16);
			arm_write_amiga_word(address+2,value<<16);
			break;
		case 3:
			arm_write_amiga_byte(address  ,value>>24);
			arm_write_amiga_word(address+1,value<< 8);
			arm_write_amiga_byte(address+3,value<< 8);
			break;
	}
}
extern "C" void ps_write_16(unsigned int address, unsigned int value)
{
	switch(address&3)
	{
		case 0:
			arm_write_amiga_word(address,value<<16);
			break;
		case 1:
			arm_write_amiga_byte(address  ,value<<8);
			arm_write_amiga_byte(address+1,value<<8);
			break;
		case 2:
			arm_write_amiga_word(address,value);
			break;
		case 3:
			arm_write_amiga_byte(address  ,value>> 8);
			arm_write_amiga_byte(address+1,value<<24);
			break;
	}
}
extern "C" void ps_write_8(unsigned int address, unsigned int value)
{
	switch(address&3)
	{
		case 0:
			arm_write_amiga_byte(address,value<<24);
			break;
		case 1:
			arm_write_amiga_byte(address,value<<16);
			break;
		case 2:
			arm_write_amiga_byte(address,value<< 8);
			break;
		case 3:
			arm_write_amiga_byte(address,value    );
			break;
	}
}

extern "C" unsigned int ps_read_8(unsigned int address)
{
	switch(address&3)
	{
		case 0:
			return((arm_read_amiga_byte(address)>>24)&0xFF);
		case 1:
			return((arm_read_amiga_byte(address)>>16)&0xFF);
		case 2:
			return((arm_read_amiga_byte(address)>>8 )&0xFF);
		case 3:
		default:
			return((arm_read_amiga_byte(address)    )&0xFF);
	}
}
extern "C" unsigned int ps_read_16(unsigned int address)
{
	switch(address&3)
	{
		case 0:
			return((arm_read_amiga_word(address)>>16)&0xFFFF);
		case 1:
			return(
					 ((arm_read_amiga_byte(address  )>>8)&0xFF00)
					|((arm_read_amiga_byte(address+1)>>8)&0x00FF)
					);
		case 2:
			return((arm_read_amiga_word(address))&0xFFFF);
		case 3:
		default:
			return(
					 ((arm_read_amiga_byte(address  )<<8 )&0xFF00)
					|((arm_read_amiga_byte(address+1)>>24)&0x00FF)
					);
	}
}
extern "C" unsigned int ps_read_32(unsigned int address)
{
	switch(address&3)
	{
		case 0:
			return(arm_read_amiga_long(address));
		case 1:
			return(
					 ((arm_read_amiga_byte(address  )<<8 )&0xFF000000)
					|((arm_read_amiga_word(address+1)<<8 )&0x00FFFF00)
					|((arm_read_amiga_byte(address+3)>>24)&0x000000FF)
					);
		case 2:
			return(
					 ((arm_read_amiga_word(address  )<<16)&0xFFFF0000)
					|((arm_read_amiga_word(address+2)>>16)&0x0000FFFF)
					);
		case 3:
		default:
			return(
					 ((arm_read_amiga_byte(address  )<<24)&0xFF000000)
					|((arm_read_amiga_word(address+1)>>8 )&0x00FFFF00)
					|((arm_read_amiga_byte(address+3)>>8 )&0x000000FF)
					);
	}
}

unsigned int  m68k_read_disassembler_8(unsigned int address)
{
	return(read_byte(address));
}
unsigned int  m68k_read_disassembler_16(unsigned int address)
{
	return(read_word(address));
}
unsigned int  m68k_read_disassembler_32(unsigned int address)
{
	return(read_long(address));
}
#ifdef __cplusplus
}
#endif
