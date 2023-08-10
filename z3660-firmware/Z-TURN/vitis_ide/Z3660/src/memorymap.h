/*
 * memorymap.h
 *
 *  Created on: 15 ago. 2022
 *      Author: shanshe
 */

#ifndef MEMORYMAP_H_
#define MEMORYMAP_H_

// FIXME allocate this memory properly

#define AUDIO_NUM_PERIODS           8
#define AUDIO_BYTES_PER_PERIOD      3840

#define RTG_BASE                    0x18000000

#define FRAMEBUFFER_ADDRESS         (RTG_BASE+0x00200000)
#define AUDIO_TX_BUFFER_SIZE        (AUDIO_BYTES_PER_PERIOD * AUDIO_NUM_PERIODS)

#define Z3_SCRATCH_ADDR             (RTG_BASE+0x03200000) // FIXME @ _Bnu
#define ADDR_ADJ                    0x001F0000 // FIXME @ _Bnu

#define AUDIO_TX_BUFFER_ADDRESS     0x07CE0000 // default, changed by driver
#define AUDIO_RX_BUFFER_ADDRESS     0x07D00000 // default, changed by driver
#define TX_BD_LIST_START_ADDRESS    0x07E00000 //---------------------------------
#define RX_BD_LIST_START_ADDRESS    0x07E80000 //                                 | <- 1 MB STRONG_ORDERED
#define RX_BACKLOG_ADDRESS          0x07EF0000 // 32 * 2048 space (64 kB) --------
#define TX_FRAME_ADDRESS            0x07F00000
#define RX_FRAME_ADDRESS            0x07F10000
#define USB_BLOCK_STORAGE_ADDRESS   0x3FE10000 // FIXME move all of these to a memory table header file
#define BOOT_ROM_ADDRESS            (RTG_BASE+0x6000)
#define BOOT_ROM_SIZE               0x20000
#define RX_FRAME_PAD 4
#define FRAME_SIZE 2048

// Our address space is relative to the autoconfig base address (for example, it could be 0x600000)
//#define MNT_REG_BASE    			0x00000000

// 0x2000 - 0x7fff   ETH RX
// 0x8000 - 0x9fff   ETH TX
// 0xa000 - 0xffff   USB BLOCK

// Frame buffer/graphics memory starts at 64KB (relative to card address), leaving ample space for general purpose registers.
//#define MNT_FB_BASE     			0x00010000


#endif /* MEMORYMAP_H_ */
