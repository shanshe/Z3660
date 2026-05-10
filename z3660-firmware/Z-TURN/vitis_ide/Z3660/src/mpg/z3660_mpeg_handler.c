/*
 * z3660_mpeg_handler.c - ARM-side MPEG decoder handler for Z3660
 *
 * This handles MPEG decoding commands from the Amiga 68k side using
 * the ARM MPEG2 decoder directly (NO pl_mpeg).
 * 
 * Follows the Z3660_PLUGIN_CALL_SCHEMA.md specification exactly.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "rtg/zzregs.h"
#include "video.h"
#include "memorymap.h"
#include "rtg/zz_video_modes.h"
#include "rtg/gfx.h"
#include "pl_mpeg.h"
#include "xparameters.h"
#include "xil_cache.h"

// Video mode constants
#define ZZVMODE_640x400       16
#define ZZVMODE_640x480       2
#define ZZVMODE_640x512       9
#define ZZVMODE_800x600       3
#define ZZVMODE_1280x720      0
#define ZZVMODE_1920x1080_50  7

#define MNTVA_COLOR_32BIT     2
#define NO_SCALE              0

// External reference to video system state
extern ZZ_VIDEO_STATE vs;
void enable_overlay(void);
void disable_overlay(void);
extern int16_t overlay_x;
extern int16_t overlay_y;
extern int16_t overlay_w;
extern int16_t overlay_h;


// YCbCr to BGRA conversion for PL_MPEG frames
// Clamp function for PL_MPEG
inline static int plm_clamp_custom(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

inline static void plm_put_pixel(int y, int r, int g, int b, uint32_t *dest)
{
   uint32_t bgra = 0;
   bgra  = plm_clamp_custom(y + b);
   bgra |= plm_clamp_custom(y - g) << 8;
   bgra |= plm_clamp_custom(y + r) << 16;
   *dest = bgra;
}

// External function declaration - defined in mobotest.c
extern void plm_frame_to_bgra_custom(plm_frame_t *frame, uint8_t *dest);

// External video functions
extern void video_mode_init(int mode, int scalemode, int colormode);
extern void set_fb(uint32_t* fb_, uint32_t pitch);
#if 0
// Configure video mode based on frame dimensions
void configure_video_mode(int width, int height) {
    int mode;
    int scale = 0;
    
    if (width <= 320 && height <= 200) {
        mode = ZZVMODE_640x400;
        scale = 1;
    } else if (width <= 320 && height <= 240) {
        mode = ZZVMODE_640x480;
        scale = 1;
    } else if (width <= 320 && height <= 256) {
        mode = ZZVMODE_640x512;
        scale = 1;
    } else if (width <= 640 && height <= 400) {
        mode = ZZVMODE_640x400;
    } else if (width <= 640 && height <= 480) {
        mode = ZZVMODE_640x480;
    } else if (width <= 800 && height <= 600) {
        mode = ZZVMODE_800x600;
    } else if (width <= 1280 && height <= 720) {
        mode = ZZVMODE_1280x720;
    } else {
        mode = ZZVMODE_1920x1080_50;
    }
    
    // Initialize video mode
    video_mode_init(mode, scale, MNTVA_COLOR_32BIT);
    
    // Set framebuffer
    set_fb((uint32_t*)(((uint32_t)vs.framebuffer) + 0), vs.vmode_hsize / vs.vmode_hdiv);
    
    // Clear framebuffer
    memset(vs.framebuffer, 0, vs.framebuffer_size);
    
    printf("[Z3660 MPEG ARM] Video mode configured: %dx%d -> mode %d, scale %d\n", 
           width, height, mode, scale);
}
#endif
// External video functions
extern void video_mode_init(int mode, int scalemode, int colormode);
extern void set_fb(uint32_t* fb_, uint32_t pitch);

#include "z3660_mpeg_handler.h"
#include "mpg/libmpeg_arm/mpeg2dec_arm_comm_arm.h"
#include "../main.h"

// Global variables
arm_decoder_shared_arm_t *bridge_state_arm = NULL;
uint32_t mpeg_param_registers[5];

// FIFO state
uint8_t *fifo_buffer = NULL;
uint32_t fifo_size = 0;
uint32_t fifo_read_index = 0;
uint32_t fifo_write_index = 0;

// Function to convert from big-endian to little-endian
static inline uint32_t be32_to_host(uint32_t x) {
    return __builtin_bswap32(x);
}

static inline uint64_t be64_to_host(uint64_t x) {
    return __builtin_bswap64(x);
}

// Maximum frame size
#define MAX_FRAME_WIDTH         1920
#define MAX_FRAME_HEIGHT        1080
#define FRAME_BUFFER_SIZE       (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 4)

// FIFO configuration
#define MPEG_FIFO_SIZE          (1024 * 1024)  // 1MB FIFO for MPEG data

#include "mpg/pl_mpeg_arm/pl_mpeg_handler.h"

// FPGA register base address
#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR

uint32_t base_rtg_amiga = 0;

// Main MPEG command processor
int z3660_mpeg_process_command(uint32_t command) {
    int result = 0;
//    printf("[Z3660 MPEG ARM] Received command 0x%08lx\n", (unsigned long)command);
        
    switch (command) {
        case ARM_CMD_INIT: {
            printf("[Z3660 MPEG ARM] Received INIT command\n");
/*            
            printf("[Z3660 MPEG ARM] Parameter registers:\n");
            for (int i = 0; i < 4; i++) {
                printf("  Param%d: 0x%08lx\n", i, (unsigned long)mpeg_param_registers[i]);
            }
*/
            // Get shared buffer address from registers
            uint32_t shared_mem_offset = mpeg_param_registers[0];  // Offset from Amiga perspective
            base_rtg_amiga = (uint32_t)mpeg_param_registers[1];
            
            // Convert Amiga address to ARM address
            // Shared memory allocated by Amiga needs to be mapped to ARM space
            bridge_state_arm = (arm_decoder_shared_arm_t *)(/*0x10000000 +*/ shared_mem_offset);
/*
            printf("[Z3660 MPEG ARM] Memory mapping debug:\n");
            printf("  Shared memory offset (Amiga): %p\n", (uint8_t *)shared_mem_offset);
            printf("  Shared memory address (ARM): %p\n", bridge_state_arm);
            printf("  base_rtg_amiga: 0x%08lx\n", (unsigned long)base_rtg_amiga);
*/
            // Initialize FIFO from MPEG parameters (priority over FPGA registers)
            fifo_size = mpeg_param_registers[2];  // FIFO size from parameter
            uint32_t fifo_offset = mpeg_param_registers[3];  // FIFO offset from Amiga perspective
            
            // Convert Amiga offset to ARM address
            // Amiga sees RTG RAM at 0x10000000 (ARM is at 0x18000000) and FIFO at offset 0x07A00000
            // ARM needs to map this to its address space
            fifo_buffer = (uint8_t*)((uint32_t)RTG_BASE + fifo_offset);  // Convert to ARM address
/*
            printf("[Z3660 MPEG ARM] FIFO configuration:\n");
            printf("  FIFO offset (Amiga): 0x%08lx\n", (unsigned long)fifo_offset);
            printf("  FIFO buffer (ARM): %p\n", fifo_buffer);
            printf("  FIFO size: %lu bytes\n", (unsigned long)fifo_size);
            printf("  Write index: %lu\n", (unsigned long)fifo_write_index);
            printf("  Read index: %lu\n", (unsigned long)fifo_read_index);
            printf("  Shared memory: %p\n", bridge_state_arm);
*/
            // Invalidate cache to ensure we see fresh data from Amiga
            Xil_DCacheInvalidateRange((uintptr_t)fifo_buffer, fifo_size);
            Xil_DCacheInvalidateRange((uintptr_t)bridge_state_arm, sizeof(arm_decoder_shared_arm_t));
/*
            // Debug: Check first few bytes of FIFO buffer
            printf("[Z3660 MPEG ARM] First 16 bytes of FIFO buffer: ");
            for (uint32_t i = 0; i < 16 && i < fifo_size; i++) {
                printf("%02x ", fifo_buffer[i]);
            }
            printf("\n");
*/
            // Initialize PL_MPEG
            pl_mpeg_set_shared_memory(bridge_state_arm);
            result = pl_mpeg_arm_init();
            mpeg_param_registers[4] = result;
            
//            printf("[Z3660 MPEG ARM] ARM_CMD_INIT: gray_mode %ld\n", be32_to_host(bridge_state_arm->gray_mode));
            enable_overlay();

            break;
        }
        
        case ARM_CMD_PROCESS: {
//            printf("[Z3660 MPEG ARM DEBUG] FIFO state:\n");
//            printf("  Current write index: %lu (0x%08lx)\n", 
//                   (unsigned long)fifo_write_index, (unsigned long)fifo_write_index);
//            printf("  Read index: %lu\n", (unsigned long)fifo_read_index);
            
            // Check if there's new data to process
            if (fifo_write_index == fifo_read_index) {
                printf("[Z3660 MPEG ARM] No new data in FIFO w=%ld r=%ld\n",fifo_write_index,fifo_read_index);
                result = 0;
                break;
            }
            
            // Calculate amount of available data
            uint32_t available_data;
            if (fifo_write_index > fifo_read_index) {
                available_data = fifo_write_index - fifo_read_index;
            } else {
                available_data = (fifo_size - fifo_read_index) + fifo_write_index;
            }
            
//            printf("[Z3660 MPEG ARM] Available data: %lu bytes\n", (unsigned long)available_data);
            
            // Process all available data
            if (available_data > 0) {
                // Allocate temporary buffer for data
                uint8_t *temp_buffer2 = (uint8_t *)malloc(available_data);
                if (!temp_buffer2) {
                    printf("[Z3660 MPEG ARM] ERROR: Could not allocate memory for temporary buffer\n");
                    result = -1;
                    break;
                }
                
                // Copy data from FIFO to temporary buffer
                // Invalidate cache before reading from shared memory
                Xil_DCacheInvalidateRange((uintptr_t)fifo_buffer, fifo_size);
                
                if (fifo_read_index + available_data <= fifo_size) {
                    // Direct copy without wrap-around
                    memcpy(temp_buffer2, fifo_buffer + fifo_read_index, available_data);
                } else {
                    // Copy with wrap-around
                    uint32_t first_part = fifo_size - fifo_read_index;
                    memcpy(temp_buffer2, fifo_buffer + fifo_read_index, first_part);
                    memcpy(temp_buffer2 + first_part, fifo_buffer, available_data - first_part);
                }
                
                // Process data with PL_MPEG
//                printf("[Z3660 MPEG ARM] Processing %lu bytes from FIFO\n", (unsigned long)available_data);
                
                // Get sync_time from shared buffer - invalidate cache first
//                Xil_DCacheInvalidateRange((uintptr_t)bridge_state_arm, sizeof(arm_decoder_shared_arm_t));
                union {
                    uint64_t u64;
                    double d;
                } time_converter;
                time_converter.u64 = be64_to_host(bridge_state_arm->sync_time);
                
                result = pl_mpeg_arm_process(temp_buffer2, available_data, time_converter.d);
                mpeg_param_registers[4] = result;
                
                if (result >= 0) {
                    bridge_state_arm->frames_decoded = be32_to_host(result);
                    // Flush cache to ensure Amiga sees the update
                    Xil_DCacheFlushRange((uintptr_t)&bridge_state_arm->frames_decoded, sizeof(uint32_t));
                }
                
                // Free temporary buffer
                free(temp_buffer2);
                
                // Update read index
                fifo_read_index = (fifo_read_index + available_data) % fifo_size;
                // Flush cache to ensure Amiga sees the updated read index
                Xil_DCacheFlushRange((uintptr_t)&fifo_read_index, sizeof(uint32_t));
            }
            break;
        }
        
        case ARM_CMD_RESET: {
            printf("[Z3660 MPEG ARM] Received RESET command\n");
            // Reset FIFO state
            fifo_read_index = 0;
            fifo_write_index = 0;
            
            // Reset PL_MPEG
            pl_mpeg_arm_reset();
            printf("[Z3660 MPEG ARM] MPEG decoder reset completed\n");
            break;
        }
        
        case ARM_CMD_CLOSE: {
            // Reset FIFO state
            fifo_read_index = 0;
            fifo_write_index = 0;
            
            // Close PL_MPEG
            pl_mpeg_arm_close();
            // disable overlay window
            disable_overlay();
            printf("[Z3660 MPEG ARM] MPEG decoder closed\n");
            break;
        }
        
        default:
            printf("[Z3660 MPEG ARM] ERROR: Unknown command 0x%08lx\n", (unsigned long)command);
            result = -1;
            break;
    }
    
    // Update status
    if (result < 0) {
        bridge_state_arm->status = be32_to_host(ARM_STATUS_ERROR);
        bridge_state_arm->error_code = be32_to_host(-result);
        // Update status register for 68k side
        *(volatile uint32_t *)(REG_BASE_ADDRESS + REG_ZZ_MPEG_STATUS) = ARM_STATUS_ERROR;
    } else {
        bridge_state_arm->status = be32_to_host(ARM_STATUS_READY);
        // Update status register for 68k side
        *(volatile uint32_t *)(REG_BASE_ADDRESS + REG_ZZ_MPEG_STATUS) = ARM_STATUS_READY;
    }
    
    return (result);
}

// Initialize MPEG handler system
int z3660_mpeg_handler_init(void) {
    printf("[Z3660 MPEG ARM] MPEG handler system initializing...\n");
    
    // Ensure video system is available
    if (!vs.framebuffer) {
        printf("[Z3660 MPEG ARM] WARNING: Video system not yet initialized\n");
        printf("[Z3660 MPEG ARM] Video initialization will be done when needed\n");
    } else {
        printf("[Z3660 MPEG ARM] Video system ready at 0x%08lx\n", (unsigned long)vs.framebuffer);
    }
    
    printf("[Z3660 MPEG ARM] MPEG handler system initialized\n");
    return 0;
}

// Synchronize FIFO registers with 68k side
void z3660_mpeg_sync_fifo_registers(void) {
    // Get FIFO configuration from 68k side
    fifo_size = mpeg_param_registers[2]; // FIFO size
    uint32_t fifo_addr = mpeg_param_registers[3]; // FIFO address offset
    
    if (fifo_size > 0 && fifo_addr != 0) {
        fifo_buffer = (uint8_t*)((uint32_t)RTG_BASE + fifo_addr);
        printf("[Z3660 MPEG ARM] FIFO configured: size=%lu, addr=0x%08lx\n", 
               (unsigned long)fifo_size, (unsigned long)fifo_addr);
    } else {
        fifo_buffer = NULL;
        fifo_size = 0;
        printf("[Z3660 MPEG ARM] FIFO not configured\n");
    }
    
    // Reset FIFO indices
    fifo_read_index = 0;
    fifo_write_index = 0;
}

