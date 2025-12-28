/*
 * pl_mpeg_arm_main.c - Main ARM-side entry point for PL_MPEG decoder on Z3660
 *
 * This file integrates PL_MPEG into the Z3660 ARM firmware to handle
 * MPEG video decoding via shared memory communication with 68k processor.
 */

#include "platform.h"
#include "xil_cache.h"
#include "memorymap.h"
#include <stdio.h>

// Existing ARM communication system
#include "mpg/libmpeg_arm/mpeg2dec_arm_comm_arm.h"

// Our PL_MPEG handler
#include "mpg/pl_mpeg_arm/pl_mpeg_handler.h"

// Endianness conversion macros for ARM (little-endian) to 68k (big-endian)
#define be16_to_host(x) __builtin_bswap16(x)
#define be32_to_host(x) __builtin_bswap32(x)
#define be64_to_host(x) __builtin_bswap64(x)
#define host_to_be32(x) __builtin_bswap32(x)

// Shared memory base address for ARM-68k communication
#define ARM_DECODER_SHARED_BASE (RTG_BASE + 0x01000000)

// Global shared memory reference for PL_MPEG
static arm_decoder_shared_arm_t *arm_shared_68k = NULL;
static int plmpeg_active = 0;

// Command handler for PL_MPEG integration
void pl_mpeg_arm_command_handler(uint32_t command) {
    if (!arm_shared_68k) return;
    
    switch (command) {
        case ARM_CMD_INIT:
            printf("[PL_MPEG ARM] CMD: INIT\n");
            if (!plmpeg_active) {
                int result = pl_mpeg_arm_init();
                if (result == 0) {
                    plmpeg_active = 1;
                    arm_shared_68k->status = ARM_STATUS_READY;
                } else {
                    arm_shared_68k->status = ARM_STATUS_ERROR;
                }
                printf("[PL_MPEG ARM] INIT result: %d\n", result);
            }
            break;
            
        case ARM_CMD_PROCESS:
            printf("[PL_MPEG ARM] CMD: PROCESS\n");
            if (plmpeg_active && arm_shared_68k->input_length > 0) {
                pl_mpeg_arm_process(arm_shared_68k->input_buffer, 
                                  arm_shared_68k->input_length,
                                 arm_shared_68k->sync_time);
            }
            break;
            
        case ARM_CMD_RESET:
            printf("[PL_MPEG ARM] CMD: RESET\n");
            if (plmpeg_active) {
                pl_mpeg_arm_reset();
                arm_shared_68k->status = ARM_STATUS_READY;
            }
            break;
            
        case ARM_CMD_CLOSE:
            printf("[PL_MPEG ARM] CMD: CLOSE\n");
            if (plmpeg_active) {
                pl_mpeg_arm_close();
                plmpeg_active = 0;
                arm_shared_68k->status = ARM_STATUS_READY;
            }
            break;
            
        default:
            // Unsupported command
            printf("[PL_MPEG ARM] CMD: Unknown (%lu)\n", (unsigned long)command);
            break;
    }
}

// Initialize PL_MPEG ARM system
void pl_mpeg_arm_system_init(void) {
    printf("[PL_MPEG ARM] Initializing PL_MPEG ARM system\n");
    
    // Set up shared memory reference
    arm_shared_68k = (arm_decoder_shared_arm_t*)ARM_DECODER_SHARED_BASE;
    
    // Initialize shared memory structure
    if (arm_shared_68k) {
        arm_shared_68k->status = ARM_STATUS_READY;
        arm_shared_68k->status2 = 0;
        arm_shared_68k->frames_decoded = 0;
        arm_shared_68k->error_code = 0;
        arm_shared_68k->input_length = 0;
        arm_shared_68k->gray_mode = 0;
        arm_shared_68k->sync_time = 0.0;
        
        // Initialize video info struct
        arm_shared_68k->framerate = 0.0;
        arm_shared_68k->frame_coded_width = 0;
        arm_shared_68k->skipped_frames = 0;
        
        // Initialize stats struct (if needed)
        // These fields don't seem to exist in the actual structure
    }
    
    printf("[PL_MPEG ARM] System initialized\n");
}
