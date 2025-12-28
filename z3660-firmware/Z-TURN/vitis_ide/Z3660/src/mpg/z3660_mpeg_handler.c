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
#include "xil_cache.h"       // For Xil_DCacheInvalidateRange and Xil_DCacheFlushRange
#include "rtg/zzregs.h"
#include "video.h"
#include "memorymap.h"

// External reference to video system state
extern ZZ_VIDEO_STATE vs;

#include "z3660_mpeg_handler.h"
#include "../main.h"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"


// MPEG operation codes (compatible with Z3660 layered plugin)
#define ARM_CMD_NONE            0
#define ARM_CMD_INIT            1
#define ARM_CMD_PROCESS         2
#define ARM_CMD_RESET           3
#define ARM_CMD_GET_INFO        4
#define ARM_CMD_CLOSE           5
#define ARM_CMD_DISPLAY_FRAME   6
#define ARM_CMD_GET_YUV_FRAME   8
/*
// Legacy command codes (for backward compatibility)
#define MPEG_OP_INIT            ARM_CMD_INIT
#define MPEG_OP_DECODE_FRAME    ARM_CMD_PROCESS
#define MPEG_OP_GET_FRAME       0x2003
#define MPEG_OP_CLOSE           ARM_CMD_CLOSE
#define MPEG_OP_SET_FRAMEBUFFER 0x2005
#define MPEG_OP_GET_FRAMEBUFFER 0x2006
#define MPEG_OP_INIT_STREAM     0x2008
#define MPEG_OP_SET_FB_OFFSET   0x2007  // Set framebuffer pan offset
#define MPEG_OP_TEST_FIFO       0x2009
*/
// Status flags (compatible with Z3660 layered plugin)
#define ARM_STATUS_READY        0
#define ARM_STATUS_BUSY         1
#define ARM_STATUS_ERROR        2
#define ARM_STATUS_SETUP_NEEDED 3

// Maximum frame buffer size
#define MAX_FRAME_WIDTH         1920
#define MAX_FRAME_HEIGHT        1080
#define FRAME_BUFFER_SIZE       (MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 4)

// MPEG FIFO circular buffer settings (similar to MHI)
#define MPEG_FIFO_SIZE          (1024 * 1024)  // 1MB circular buffer for MPEG data
#define MPEG_FIFO_BASE_ADDR     (RTG_BASE + 0x07A00000)  // Within RTG space, before framebuffer

#include "mpg/pl_mpeg_arm/pl_mpeg_handler.h"

arm_decoder_shared_arm_t *bridge_state_arm = NULL;
uint32_t mpeg_param_registers[4];
uint32_t base_rtg_amiga = 0;

// Main MPEG command processor following Z3660_PLUGIN_CALL_SCHEMA.md
int z3660_mpeg_process_command(uint32_t command) {
    int result = 0;
    printf("[Z3660 MPEG ARM] Received command 0x%08lx\n", (unsigned long)command);
    
//    printf("[Z3660 MPEG ARM] *** COMMAND RECEIVED *** Processing command 0x%08lx\n", (unsigned long)command);
#define be32_to_host(x) __builtin_bswap32(x)
#define be64_to_host(x) __builtin_bswap64(x)
    switch (command) {
        case ARM_CMD_INIT: {
            bridge_state_arm = (arm_decoder_shared_arm_t *)(mpeg_param_registers[0]);
            base_rtg_amiga = (uint32_t)mpeg_param_registers[1];
            printf("[Z3660 MPEG ARM] base_rtg_amiga %p\n",(uint8_t *)base_rtg_amiga);
            printf("bridge_state_arm=%p\n",bridge_state_arm);
            // Use PL_MPEG instead of MPEG2DEC
            pl_mpeg_set_shared_memory(bridge_state_arm);
            result = pl_mpeg_arm_init();
            mpeg_param_registers[0] = be32_to_host(result);
            printf("[Z3660 MPEG ARM] ARM_CMD_INIT: gray_mode %ld\n",be32_to_host(bridge_state_arm->gray_mode));
            break;
        }
        
        case ARM_CMD_PROCESS: {
            Xil_DCacheInvalidateRange((INTPTR)bridge_state_arm,sizeof(bridge_state_arm));
            union {
                uint64_t u64;
                double d;
            } converter;
            memcpy(&converter.u64, &bridge_state_arm->sync_time, sizeof(uint64_t));
            converter.u64 = be64_to_host(converter.u64);
            printf("received bridge_state_arm at %p\n",bridge_state_arm);
            printf("received input_buffer at %p\n",bridge_state_arm->input_buffer);
//            printf("received frame_base[0] at %p\n",xlate_CPURAM(bridge_state_arm->display_frame.frame_base[0]));
            printf("received sync_time %lf\n",converter.d);
            printf("received input_length %lu\n",be32_to_host(bridge_state_arm->input_length));
            // Process with PL_MPEG instead of MPEG2DEC
            result = pl_mpeg_arm_process(bridge_state_arm->input_buffer,
                                         be32_to_host(bridge_state_arm->input_length),
                                         converter.d);
            mpeg_param_registers[0] = be32_to_host(result);
            if (result >= 0) {
                bridge_state_arm->frames_decoded = be32_to_host(result);
            }
            break;
        }
        
        case ARM_CMD_GET_INFO: {
            result = 0;//local_decoder_get_info();
            mpeg_param_registers[0] = be32_to_host(result);
            printf("[Z3660 MPEG ARM] ARM_CMD_GET_INFO\n");
            break;
        }
#if 0
        case ARM_CMD_GET_YUV_FRAME: {
            //  ARM_CMD_GET_YUV_FRAME implementation (already correct)
            printf("[Z3660 MPEG ARM] ARM_CMD_GET_YUV_FRAME\n");
            
            if (mpeg_ctx.arm_wrapper && mpeg_ctx.framebuffer) {
                // Get current YUV frame pointers from ARM decoder
                uint32_t y_addr, u_addr, v_addr;
                if (mpeg2dec_get_yuv_buffers_arm(mpeg_ctx.arm_wrapper, &y_addr, &u_addr, &v_addr) == 0) {
                    // Calculate YUV buffer sizes
                    int width = mpeg_ctx.width;
                    int height = mpeg_ctx.height;
                    int y_size = width * height;
                    int uv_size = (width / 2) * (height / 2);  // YUV420
                    
                    // Use dedicated YUV buffer area
                    uint32_t yuv_base_arm = RTG_BASE + 0x05000000;  // ARM address for YUV buffer
                    uint8_t *dst_base = (uint8_t*)yuv_base_arm;
#define PRE_TEST_YUV
#ifdef PRE_TEST_YUV
                    printf("[Z3660 MPEG ARM] ARM pre test pattern ENABLED\n");
                    // Fill Y plane with vertical gradient (dark to bright)
                    int height2 = 40;
                    int uv_size2 = (width / 2) * (height2 / 2);  // YUV420
                    uint8_t* y_ptr = (uint8_t*)y_addr;
                    for (int y = height-height2; y < height; y++) {
                        unsigned char luma = (y * 255) / height2;
                        for (int x = 0; x < width; x++) {
                            y_ptr[y * width + x] = luma;
                        }
                    }
                    
                    // Fill U plane (blue-ish)
                    uint8_t* u_ptr = (uint8_t*)u_addr;
                    for (int i = uv_size-uv_size2; i < uv_size; i++) {
                        u_ptr[i] = 200;
                    }
                    
                    // Fill V plane (red-ish)
                    uint8_t* v_ptr = (uint8_t*)v_addr;
                    for (int i = uv_size-uv_size2; i < uv_size; i++) {
                        v_ptr[i] = 100;
                    }
#endif
                    // Copy YUV planes to dedicated YUV memory area
                    printf("Copiando de %p a %p\n", (uint8_t*)y_addr, dst_base);
                    memcpy(dst_base, (uint8_t*)y_addr, y_size);
                    printf("Copiando de %p a %p\n", (uint8_t*)u_addr, dst_base+y_size);
                    memcpy(dst_base + y_size, (uint8_t*)u_addr, uv_size);
                    printf("Copiando de %p a %p\n", (uint8_t*)v_addr, dst_base+y_size+uv_size);
                    memcpy(dst_base + y_size + uv_size, (uint8_t*)v_addr, uv_size);
                    
//#define TEST_YUV
#ifdef TEST_YUV
                    printf("[Z3660 MPEG ARM] ARM test pattern ENABLED\n");
                    // Fill Y plane with vertical gradient (dark to bright)
                    uint8_t* y_ptr = dst_base;
                    for (int y = 0; y < height; y++) {
                        unsigned char luma = (y * 255) / height;
                        for (int x = 0; x < width; x++) {
                            y_ptr[y * width + x] = luma;
                        }
                    }
                    
                    // Fill U plane (blue-ish)
                    uint8_t* u_ptr = dst_base + y_size;
                    for (int i = 0; i < uv_size; i++) {
                        u_ptr[i] = 200;
                    }
                    
                    // Fill V plane (red-ish)
                    uint8_t* v_ptr = dst_base + y_size + uv_size;
                    for (int i = 0; i < uv_size; i++) {
                        v_ptr[i] = 100;
                    }
#else
                    printf("[Z3660 MPEG ARM] ARM test pattern DISABLED\n");
#endif                    
                    // Return offsets relative to RTG_BASE for 68k mapping
                    uint32_t yuv_offset = yuv_base_arm - RTG_BASE;
                    mpeg_param_registers[0] = yuv_offset;              // Y plane offset
                    mpeg_param_registers[1] = yuv_offset + y_size;     // U plane offset
                    mpeg_param_registers[2] = yuv_offset + y_size + uv_size; // V plane offset
                    mpeg_param_registers[3] = mpeg_ctx.frames_decoded; // Frame count
                    
                    printf("[Z3660 MPEG ARM] YUV offsets: Y=0x%08lx, U=0x%08lx, V=0x%08lx\n",
                           (unsigned long)mpeg_param_registers[0], 
                           (unsigned long)mpeg_param_registers[1],
                           (unsigned long)mpeg_param_registers[2]);
                    
                    mpeg_status_register = MPEG_STATUS_READY | MPEG_STATUS_COMPLETE;
                } else {
                    printf("[Z3660 MPEG ARM] No YUV frame ready\n");
                    mpeg_param_registers[0] = 0;
                    mpeg_param_registers[1] = 0;
                    mpeg_param_registers[2] = 0;
                    mpeg_param_registers[3] = 0;
                    mpeg_status_register = MPEG_STATUS_READY;
                }
            } else {
                printf("[Z3660 MPEG ARM] ARM decoder not ready\n");
                mpeg_param_registers[0] = 0;
                mpeg_param_registers[1] = 0;
                mpeg_param_registers[2] = 0;
                mpeg_param_registers[3] = 0;
                mpeg_status_register = MPEG_STATUS_READY;
            }
            break;
        }
#endif
        case ARM_CMD_RESET: {
            // Reset PL_MPEG instead of MPEG2DEC
            pl_mpeg_arm_reset();
            break;
        }
        
        case ARM_CMD_CLOSE: {
            // Close PL_MPEG instead of MPEG2DEC
            pl_mpeg_arm_close();
            break;
        }
        
        default:
            printf("[Z3660 MPEG ARM] ERROR: Unknown command 0x%08lx\n", (unsigned long)command);
            result = -1;
            break;
    }
    // Set final status
    if (result < 0) {
        bridge_state_arm->status = be32_to_host(ARM_STATUS_ERROR);
        bridge_state_arm->error_code = be32_to_host(-result);
    } else if (bridge_state_arm->status != be32_to_host(ARM_STATUS_SETUP_NEEDED)) {
        bridge_state_arm->status = be32_to_host(ARM_STATUS_READY);
    }
    return result;
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

