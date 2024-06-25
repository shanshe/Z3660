/*
 * main.h
 *
 *  Created on: 8 jun. 2022
 *      Author: shanshe
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#define REVISION_MAJOR 1
#define REVISION_MINOR 03
//#define REVISION_BETA 0
#define REVISION_BETA 1

#define INT_IPL_ON_THIS_CORE 0

//#define NO_ARM_RESET_ON_AMIGA_RESET
//#define CPU_FIXED_FREQUENCY

#define M_PI 3.14159265358979323846

#include "platform.h"
#include "xil_printf.h"
#include "xgpiops.h"
#include "xil_io.h"
#include "xscugic.h"

#include "xil_cache.h"
#include "xil_cache_l.h"
#include "xil_exception.h"
#include "xclk_wiz.h"
#include "xadcps.h"
#include "xuartps.h"

//#include "mpg/ff.h"
#include <ff.h>

#include "xaxivdma.h"
#include "sii9022_init/sii9022_init.h"
#include "xil_types.h"
#include "rtg/zz_video_modes.h"

#include "rtg/gfx.h"
#include "video.h"
#include "ethernet.h"
#include "interrupt.h"
#include "adc.h"
#include "ax.h"
#include "xpseudo_asm.h"
#include <xparameters.h>

// comment out if you don't want to compile CPU emulators (Musashi and UAE)
#define CPU_EMULATOR

#define OP_PALETTE 3
#define OP_PALETTE_HI 19

#define PS_MIO_0      0 // MIO  0
#define PS_MIO_8      8 // MIO  8
#define PS_MIO_9      9 // MIO  9
#define PS_MIO_10    10 // MIO 10
#define PS_MIO_11    11 // MIO 11
#define PS_MIO_12    12 // MIO 12
#define PS_MIO_13    13 // MIO 13
#define PS_MIO_15    15 // MIO 15
#define PS_MIO_50    50 // MIO 50
#define PS_MIO_51    51 // MIO 51

typedef struct {
	volatile uint32_t shared_data;         // 0xFFFF0000
	volatile uint32_t write_rtg;           // 0xFFFF0004
	volatile uint32_t write_rtg_addr;      // 0xFFFF0008
	volatile uint32_t write_rtg_data;      // 0xFFFF000C
	volatile uint32_t core0_hold;          // 0xFFFF0010
	volatile uint32_t core0_hold_ack;      // 0xFFFF0014
	volatile uint32_t irq;                 // 0xFFFF0018
	volatile uint32_t uart_semaphore;      // 0xFFFF001C
	volatile uint32_t jit_enabled;         // 0xFFFF0020
	volatile uint32_t reset_emulator;      // 0xFFFF0024
	volatile uint32_t load_rom_emu;        // 0xFFFF0028
	volatile uint32_t load_rom_addr;       // 0xFFFF002C
	volatile uint32_t int_available;       // 0xFFFF0030
	volatile uint32_t cfg_emu;             // 0xFFFF0034
	volatile uint32_t write_scsi;          // 0xFFFF0038
	volatile uint32_t write_scsi_addr;     // 0xFFFF003C
	volatile uint32_t write_scsi_data;     // 0xFFFF0040
	volatile uint32_t write_scsi_type;     // 0xFFFF0044
	volatile uint32_t read_scsi;           // 0xFFFF0048
	volatile uint32_t read_scsi_addr;      // 0xFFFF004C
	volatile uint32_t read_scsi_data;      // 0xFFFF0050
	volatile uint32_t read_scsi_type;      // 0xFFFF0054
	volatile uint32_t scsiboot_rom_loaded; // 0xFFFF0058
	volatile uint32_t write_scsi_in_progress; // 0xFFFF005C
	volatile uint32_t read_rtg;            // 0xFFFF0060
	volatile uint32_t read_rtg_addr;       // 0xFFFF0064
	volatile uint32_t read_rtg_data;       // 0xFFFF0068
	volatile uint32_t mmu_core1_add;       // 0xFFFF006C
	volatile uint32_t z3_enabled;          // 0xFFFF0070
	volatile uint32_t load_ext_rom_addr;   // 0xFFFF0074
	volatile uint32_t load_romext_emu;     // 0xFFFF0078
	volatile uint32_t nops_write;          // 0xFFFF007C
	volatile uint32_t nops_read;           // 0xFFFF0080
	volatile uint32_t disassemble;         // 0xFFFF0088
	volatile uint32_t musashi_step;        // 0xFFFF008C
	volatile uint32_t reset_emulator_dis;  // 0xFFFF0090
} SHARED;
extern SHARED *shared;
#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR
#define write_reg(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define read_reg(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))

#define BIT0  (1<<0)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT15 (1<<15)

//#define NBR_ARM(X)        *(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR)=~((BIT15)<<16) & (0xFFFF0000U | ((X)*(BIT15)));
//#define CPLD_RESET_ARM(X) *(volatile uint32_t*)(XPAR_PS7_GPIO_0_BASEADDR)=~((BIT13)<<16) & (0xFFFF0000U | ((X)*(BIT13)));
#define NBR_ARM(X)        XGpioPs_WritePin(&GpioPs, PS_MIO_8, X);
#define CPLD_RESET_ARM(X) do{ XGpioPs_WritePin(&GpioPs, PS_MIO_13, X);} while(0)

#define n040RSTI     PS_MIO_10 // MIO 10
#define LED1         PS_MIO_11 // MIO 11 (Z3660's green led)

//#define CPLD_RAM_EN  14 // MIO 14 used by CAN RX !!!!
#define USER_SW1     PS_MIO_50 // MIO 50
#define PS_MIO51_501 PS_MIO_51 // MIO 51 (ethernet reset)

#define REG0 0x00
#define REG1 0x04
#define REG2 0x08
//#define REG3 0x0C
//#define REG4 0x10
#define REG5 0x14

#define FPGA_RAM_EN                   (1L<< 0)  // SAXI REG0  0
#define FPGA_RAM_BURST_READ_EN        (1L<< 1)  // SAXI REG0  1
#define FPGA_RAM_BURST_WRITE_EN       (1L<< 2)  // SAXI REG0  2
#define FPGA_MAPROM_EN                (1L<< 3)  // SAXI REG0  3
#define FPGA_TSCONDITION0             (0L<< 4)  // SAXI REG0  6..4
#define FPGA_TSCONDITION1             (1L<< 4)  // SAXI REG0  6..4
#define FPGA_TSCONDITION2             (2L<< 4)  // SAXI REG0  6..4
#define FPGA_TSCONDITION3             (3L<< 4)  // SAXI REG0  6..4
#define FPGA_TSCONDITION4             (4L<< 4)  // SAXI REG0  6..4
#define FPGA_TSCONDITION5             (5L<< 4)  // SAXI REG0  6..4
#define FPGA_MAPROMEXT_EN             (1L<< 7)  // SAXI REG0  7
#define FPGA_ENCONDITION_PCLK0_CLKEN0 (0L<< 8)  // SAXI REG0  9..8
#define FPGA_ENCONDITION_PCLK1_CLKEN0 (1L<< 8)  // SAXI REG0  9..8
#define FPGA_ENCONDITION_PCLK0        (2L<< 8)  // SAXI REG0  9..8
#define FPGA_ENCONDITION_PCLK1        (3L<< 8)  // SAXI REG0  9..8
#define FPGA_256MB_AUTOCONFIG_EN      (1L<<12)  // SAXI REG0 12
#define FPGA_RTG_AUTOCONFIG_EN        (1L<<13)  // SAXI REG0 13
#define FPGA_AUTOCONFIG_BOOT_EN       (1L<<27)  // SAXI REG0 27
#define FPGA_BP                       (1L<<28)  // SAXI REG0 28
#define FPGA_INT6                     (1L<<29)  // SAXI REG0 29
#define READ_WRITE_ACK                (1L<<30)  // SAXI REG0 30
#define FPGA_RESET                    (1L<<31)  // SAXI REG0 31

#ifdef XPARAMETERS_H // FIXME: this is not needed, but eclipse complaints if not defined here
//#define XPAR_Z3660_0_BASEADDR 0x83C00000
#define XPAR_XI2STX_0_DEVICE_ID XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_I2S_TRANSMITTER_0_DEVICE_ID
#define XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_I2S_TRANSMITTER_0_DEVICE_ID 0
#define XPAR_XAUDIOFORMATTER_0_DEVICE_ID XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_AUDIO_FORMATTER_0_DEVICE_ID
#define XPAR_PROCESSING_AV_SYSTEM_AUDIO_VIDEO_ENGINE_AUDIO_FORMATTER_0_DEVICE_ID 0
//#define XPAR_XAUDIOFORMATTER_0_BASEADDR 0x78C40000
#endif

#define REG_BASE_ADDRESS_S01 0x7FC70000
#define REG_BASE_ADDRESS_S00 0x80000000

#define read_reg_s01(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S01+(Offset)))
#define write_reg_s01(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S01+(Offset)))=(Data)
//#define write_reg64_s01(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS_S01+(Offset)))=(Data)

#define write_reg_s00(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S00+(Offset)))=(Data)
#define read_reg_s00(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S00+(Offset)))
#define write_reg64_s00(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS_S00+(Offset)))=(Data)
#define write_mem32(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S00+0x04000000+((Offset&0x00FFFFFF)<<2)))=(Data)
#define write_mem16(Offset,Data) (*(volatile uint32_t *)(REG_BASE_ADDRESS_S00+0x08000000+((Offset&0x00FFFFFF)<<2)))=(Data)
#define write_mem8(Offset,Data)  (*(volatile uint32_t *)(REG_BASE_ADDRESS_S00+0x0C000000+((Offset&0x00FFFFFF)<<2)))=(Data)

#define DiscreteSet(Offset,Mask) write_reg_s01(Offset,read_reg_s01(Offset)|(Mask))
#define DiscreteClear(Offset,Mask) write_reg_s01(Offset,read_reg_s01(Offset)&(~(Mask)))

void reset_video(int reset_frame_buffer);
void loop2(void);
void Display2(void);
void rtg_loop(void);
void rtg_init(void);

void video_mode_init(int mode, int scalemode, int colormode);
int init_vdma(int hsize, int vsize, int hdiv, int vdiv, uint32_t buspos);
uint32_t arm_read_amiga(uint32_t address, uint32_t size);
void arm_write_amiga(uint32_t address, uint32_t data, uint32_t size);
void arm_write_nowait(uint32_t address, uint32_t data);

#define WRITE_ (0<<1)
#define READ_  (1<<1)
#define LONG_  (0<<2)
#define BYTE_  (1<<2)
#define WORD_  (2<<2)
#define LINE_  (3<<2)

#define NO_RESET_FRAMEBUFFER 0
#define    RESET_FRAMEBUFFER 1

#define M68K_RUNNING 0
#define M68K_RESET   1
#include "defines.h"
#endif /* SRC_MAIN_H_ */
