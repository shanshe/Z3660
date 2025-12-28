/*
 * z3660_mpeg_handler.h - Header for Z3660 MPEG decoder handler
 */

#ifndef Z3660_MPEG_HANDLER_H
#define Z3660_MPEG_HANDLER_H

#include <stdint.h>

// Function prototypes
int z3660_mpeg_process_command(uint32_t command);
int z3660_mpeg_handler_init(void);
void z3660_mpeg_sync_fifo_registers(void);

// MPEG operation codes

#define MPEG_OP_INIT            0x2001
#define MPEG_OP_DECODE_FRAME    0x2002
#define MPEG_OP_GET_FRAME       0x2003
#define MPEG_OP_CLOSE           0x2004
#define MPEG_OP_SET_FRAMEBUFFER 0x2005
#define MPEG_OP_GET_FRAMEBUFFER 0x2006
#define MPEG_OP_INIT_STREAM     0x2008
#define MPEG_OP_TEST_FIFO       0x2009

// Status flags
#define MPEG_STATUS_READY       0x0001
#define MPEG_STATUS_BUSY        0x0002
#define MPEG_STATUS_ERROR       0x0004
#define MPEG_STATUS_COMPLETE    0x0010
#define MPEG_STATUS_FRAME_READY 0x0020


/*
 * Video information structure
 * Contains video parameters determined by the decoder
 */
typedef struct {
    int width;              // Picture width
    int height;             // Picture height  
    int coded_width;        // Coded picture width (may be larger than display width)
    double framerate;       // Frame rate in fps
    double bitrate;         // Bitrate
} decoder_video_info_t;

/*
 * Decoder statistics structure
 * Contains decoder timing and state information
 */
typedef struct {
    double current_sync_time;   // Current synchronization time
    double current_video_time;  // Current video time
    double frame_time;          // Duration of one frame
    int searching_iframe;       // 1 if searching for I-frame, 0 otherwise
} decoder_stats_t;

/*
 * Decoder Layer API
 */

/* Initialize decoder with gray mode setting */
int decoder_init(int gray_mode);

/* Setup video parameters (called after sequence header parsed) */
int decoder_setup_video(void);

/* Send MPEG data to ARM decoder, returns number of frames decoded or -1 on error */
int decoder_process_data(unsigned char *buf, unsigned long length, double sync_time);

/* Reset stream - search for next I-frame */
void decoder_reset_stream(void);

/* Check if decoder setup is needed (after sequence header is processed) */
int decoder_needs_setup(void);

/* Mark that decoder setup has been completed */
void decoder_setup_completed(void);

/* Get current video information from decoder */
int decoder_get_video_info(decoder_video_info_t *info);

/* Get decoder statistics */
int decoder_get_stats(decoder_stats_t *stats);

/* Get YUV frame data from ARM for display (returns 0 on success, -1 if no frame ready) */
int decoder_get_yuv_frame(unsigned char **yuv_planes);

/* Close decoder and free resources */
void decoder_close(void);

#endif /* Z3660_MPEG_HANDLER_H */