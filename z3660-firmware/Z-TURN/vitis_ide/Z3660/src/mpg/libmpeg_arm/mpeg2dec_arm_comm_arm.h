/*
 * mpeg2dec_arm_comm.h - ARM-68k Communication Protocol
 * 
 * This header defines the communication protocol between 68k (AMP) 
 * and ARM (decoder) for Z3660 MPEG video decoding.
 */

#ifndef MPEG2DEC_ARM_COMM_ARM_H
#define MPEG2DEC_ARM_COMM_ARM_H

#include "inttypes_arm.h"

/* Commands sent from 68k to ARM */
#define ARM_CMD_NONE            0
#define ARM_CMD_INIT            1
#define ARM_CMD_PROCESS         2
#define ARM_CMD_RESET           3
#define ARM_CMD_GET_INFO        4
#define ARM_CMD_CLOSE           5
#define ARM_CMD_DISPLAY_FRAME   6

/* Status responses from ARM to 68k */
#define ARM_STATUS_ERROR         0x02
#define ARM_STATUS_SETUP_NEEDED  0x04
#define ARM_STATUS_VIDEO_REFRESH 0x08
#define ARM_STATUS_VIDEO_INIT    0x10
#define ARM_STATUS_FINISHED      0x20

/*
 * Shared memory structure for ARM-68k communication
 * This structure should be placed in shared memory accessible by both processors
 */
typedef struct {
    /* Control block - modified by both processors */
    volatile uint32_t status;           /* Status from ARM to 68k */
    volatile uint32_t status2;          /* Status from ARM to 68k */
    volatile int32_t  frames_decoded;   /* Number of frames decoded */
    volatile uint32_t error_code;       /* Error code if status == ARM_STATUS_ERROR */
    
    /* Input parameters - set by 68k before command */
    volatile uint32_t input_length;              /* Length of MPEG data */
    volatile int32_t gray_mode;                  /* Gray mode flag (for init) */
    double sync_time;                   /* Synchronization time */
    
    uint32_t frame_width;               /* Frame width in pixels */
    uint32_t frame_height;              /* Frame height in pixels */

    float framerate;
    uint32_t frame_coded_width;

    /* Statistics - updated by ARM */
    uint32_t skipped_frames;
    uint32_t decode_time_us;            /* Time taken to decode last frame */
    uint32_t megabytes_remaining;       /* Total megabytes remaining ARM decoder */
    uint32_t bytes_remaining;

    /* Display frame data - for vo_draw communication */
    struct {
        double sync_time;           /* Frame sync time */
        double video_time;          /* Frame video time */
        int16_t drop_flag;          /* Drop flag result from ARM */
        int16_t padding1;
        int16_t padding2;
        int16_t padding3;
        uint32_t framebuffer_addr;  /* Framebuffer address in Amiga memory */
        uint32_t framebuffer_pitch;  /* Pitch/stride of framebuffer */
//        uint32_t frame_base_amiga[3];   /* Y, U, V plane pointers */
//        uint32_t frame_rgb;             /* ARGB frame pointer */
//        uint8_t *frame_base_arm[3];     /* Y, U, V plane pointers */
    } display_frame;
} arm_decoder_shared_arm_t __attribute__((aligned(32)));

#endif // MPEG2DEC_ARM_COMM_ARM_H
