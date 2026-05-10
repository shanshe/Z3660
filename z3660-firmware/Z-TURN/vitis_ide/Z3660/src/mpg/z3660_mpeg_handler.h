/*
 * z3660_mpeg_handler.h - Header for Z3660 MPEG decoder handler
 * 
 * Definiciones para la comunicación entre el Amiga (68k) y el ARM
 * para la decodificación de video MPEG usando FIFO.
 */

#ifndef Z3660_MPEG_HANDLER_H
#define Z3660_MPEG_HANDLER_H

#include <stdint.h>

// Function prototypes
int z3660_mpeg_process_command(uint32_t command);
int z3660_mpeg_handler_init(void);
void z3660_mpeg_sync_fifo_registers(void);

// Comandos para el ARM (deben coincidir con el lado 68k)
#define ARM_CMD_NONE            0
#define ARM_CMD_INIT            1
#define ARM_CMD_PROCESS         2
#define ARM_CMD_RESET           3
#define ARM_CMD_CLOSE           5

// Estados del decodificador
#define ARM_STATUS_READY        0x00
#define ARM_STATUS_BUSY         0x01
#define ARM_STATUS_ERROR        0x02
#define ARM_STATUS_SETUP_NEEDED 0x04

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

#endif /* Z3660_MPEG_HANDLER_H */
