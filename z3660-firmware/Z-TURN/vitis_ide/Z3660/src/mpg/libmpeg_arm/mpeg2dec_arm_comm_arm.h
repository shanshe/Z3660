/*
 * mpeg2dec_arm_comm.h - ARM-68k Communication Protocol
 * 
 * This header defines the communication protocol between 68k (AMP) 
 * and ARM (decoder) for Z3660 MPEG video decoding.
 */

#ifndef MPEG2DEC_ARM_COMM_ARM_H
#define MPEG2DEC_ARM_COMM_ARM_H

#include "inttypes_arm.h"

/* Maximum input buffer size for MPEG data chunks */
#define ARM_INPUT_BUFFER_SIZE (256 * 1024)

/* Commands sent from 68k to ARM */
#define ARM_CMD_NONE            0
#define ARM_CMD_INIT            1
#define ARM_CMD_PROCESS         2
#define ARM_CMD_RESET           3
#define ARM_CMD_GET_INFO        4
#define ARM_CMD_CLOSE           5
#define ARM_CMD_DISPLAY_FRAME   6

/* Status responses from ARM to 68k */
#define ARM_STATUS_READY         0
#define ARM_STATUS_BUSY          1
#define ARM_STATUS_ERROR         2
#define ARM_STATUS_SETUP_NEEDED  3
#define ARM_STATUS_VIDEO_REFRESH 4
#define ARM_STATUS_VIDEO_INIT    5

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
    
    /* Video information - set by ARM when available */
//    decoder_video_info_arm_t video_info;
    
    /* Statistics - updated by ARM */
//    decoder_stats_arm_t stats;
        
    /* Frame output buffers - decoded frames from ARM to 68k */
    /* Note: These will be ARM memory addresses that 68k can access */
//    uint32_t frame_y_addr;              /* Y plane address in ARM memory */
//    uint32_t frame_u_addr;              /* U plane address in ARM memory */
//    uint32_t frame_v_addr;              /* V plane address in ARM memory */
    uint32_t frame_width;               /* Frame width in pixels */
    uint32_t frame_height;              /* Frame height in pixels */
//    uint32_t frame_stride;              /* Stride for Y plane */
    
    /* Synchronization - for signaling between processors */
//    volatile uint32_t arm_signal;       /* Signal from 68k to ARM */
//    volatile uint32_t m68k_signal;      /* Signal from ARM to 68k */

    float framerate;
    uint32_t frame_coded_width;

    /* Statistics - updated by ARM */
    uint32_t skipped_frames;
    uint32_t decode_time_us;            /* Time taken to decode last frame */

    /* Display frame data - for vo_draw communication */
    struct {
        double sync_time;           /* Frame sync time */
        double video_time;          /* Frame video time */
        int16_t drop_flag;          /* Drop flag result from ARM */
        int16_t padding1;
        int16_t padding2;
        int16_t padding3;
//        uint32_t frame_base_amiga[3];     /* Y, U, V plane pointers */
//        uint32_t padding4;
//        uint8_t *frame_base_arm[3];     /* Y, U, V plane pointers */
    } display_frame;

    /* Input data buffer - MPEG stream data from 68k */
    uint8_t input_buffer[ARM_INPUT_BUFFER_SIZE];

} arm_decoder_shared_arm_t __attribute__((aligned(32)));

#endif // MPEG2DEC_ARM_COMM_ARM_H
