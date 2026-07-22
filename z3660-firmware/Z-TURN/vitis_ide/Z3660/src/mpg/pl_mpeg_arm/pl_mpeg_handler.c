/*
 * pl_mpeg_handler.c - ARM-side PL_MPEG Handler for Z3660
 *
 * Implementation of ARM-side PL_MPEG decoder functions that interface
 * with the 68k processor via shared memory communication.
 */

#define PL_MPEG_IMPLEMENTATION
#define PLM_NO_STDIO

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "../pl_mpeg.h"
#include "../libmpeg_arm/mpeg2dec_arm_comm_arm.h"
#include "../../memorymap.h"
#include "../../rtg/zzregs.h"
#include "xil_cache.h"
#include "xtime_l.h"
#include <math.h>
#include "xscutimer.h"
#include "xscugic.h"
#include "xparameters.h"
#include "xparameters_ps.h"

#define ACCUMULATION_BUFFER_SIZE_IN_SAMPLES (4*1024*1024) // 4 mega samples

// External RTG register functions
extern void write_rtg_register(uint32_t zaddr, uint32_t zdata);
extern uint32_t read_rtg_register(uint32_t zaddr);
extern volatile int audio_local_interrput;
extern uint8_t* audio_tx_buffer;

// Video interrupt variables
volatile int video_local_interrupt = 0;
volatile int interrupt_enabled_video = 0;

// Timer for video interrupt
static XScuTimer VideoTimerInstance;
static int video_timer_initialized = 0;

uint32_t audio_accumulated_samples = 0; // Total samples in buffer
    
// Global variables for audio processing
uint32_t audio_buff_offset = 0;
uint32_t audio_write_index = 0; // Index for writing new samples
uint32_t audio_read_index = 0;  // Index for reading processed samples
int16_t *audio_accumulation_buffer = NULL;

// Function prototypes
int get_current_cpu_frequency();
void enable_video_interrupt_timer(void);
void disable_video_interrupt_timer(void);
static void draw_fps_overlay(uint32_t *framebuffer, int width, int height, int pitch);
static void draw_char(uint32_t *framebuffer, int width, int height, int pitch, 
                      int x, int y, char c, uint32_t color);
extern int16_t overlay_x;
extern int16_t overlay_y;
extern int16_t overlay_w;
extern int16_t overlay_h;
extern int overlay_is_enabled;
void enable_overlay(void);
void disable_overlay(void);
// Anti-aliasing intensity control (1-10, higher = more smoothing)
int dithering_intensity = 0;
// Grayscale mode flag (0 = color, 1 = grayscale)
int grayscale_mode = 0;
int show_fps = 1;

// Endianness conversion macros for ARM (little-endian) to 68k (big-endian)
#define be16_to_host(x) __builtin_bswap16(x)
#define be32_to_host(x) __builtin_bswap32(x)
#define be64_to_host(x) __builtin_bswap64(x)
#define host_to_be16(x) __builtin_bswap16(x)
#define host_to_be32(x) __builtin_bswap32(x)
#define host_to_be64(x) __builtin_bswap64(x)

// Global PL_MPEG instance
plm_t *plm_decoder = NULL;
static plm_buffer_t *plm_buffer = NULL;
static arm_decoder_shared_arm_t *plm_shared = NULL; // Use the original structure
static int header_parsed = 0; // Flag to track if video headers have been parsed

// Frame timing control variables
static uint32_t last_frame_time = 0;
static uint32_t frame_count = 0;
static uint32_t accumulated_delay = 0;
static uint32_t last_data_time = 0;

// FIFO buffer for accumulating MPEG data
#define FIFO_BUFFER_SIZE (1024 * 1024) // 1MB FIFO buffer
static uint8_t fifo_buffer[FIFO_BUFFER_SIZE];
static size_t fifo_buffer_head = 0;
static size_t fifo_buffer_tail = 0;
static size_t fifo_buffer_count = 0;
int arm_freq=667;
// Get microsecond timestamp for performance measurement
static uint32_t plm_get_microseconds(void) {
    // Use Xilinx XTime timer for precise timing
    
    static XTime start_time = 0;
    XTime current_time;
    
    XTime_GetTime(&current_time);
    
    // Initialize start time on first call
    if (start_time == 0) {
        start_time = current_time;
    }
    
    // Convert from timer ticks to microseconds
    // XTime runs at CPU frequency (typically 666MHz for Zynq-7000)
    // ticks / (666,000,000 ticks/sec) * 1,000,000 μs/sec = ticks / 666

    return (uint32_t)((current_time - start_time) / (arm_freq/2) ); // Adjust for actual CPU frequency
}

// Add data to FIFO buffer
static int fifo_buffer_add(const uint8_t *data, size_t length) {
    if (fifo_buffer_count + length > FIFO_BUFFER_SIZE) {
        printf("[PL_MPEG ARM] FIFO buffer overflow, resetting\n");
        // Reset FIFO on overflow
        fifo_buffer_head = 0;
        fifo_buffer_tail = 0;
        fifo_buffer_count = 0;
    }
    
    // Add data to FIFO buffer
    for (size_t i = 0; i < length; i++) {
        fifo_buffer[fifo_buffer_head] = data[i];
        fifo_buffer_head = (fifo_buffer_head + 1) % FIFO_BUFFER_SIZE;
        fifo_buffer_count++;
    }
    
    return 0;
}

// Get data from FIFO buffer
static size_t fifo_buffer_get(uint8_t *data, size_t max_length) {
    size_t bytes_read = 0;
    
    while (bytes_read < max_length && fifo_buffer_count > 0) {
        data[bytes_read] = fifo_buffer[fifo_buffer_tail];
        fifo_buffer_tail = (fifo_buffer_tail + 1) % FIFO_BUFFER_SIZE;
        fifo_buffer_count--;
        bytes_read++;
    }
    
    return bytes_read;
}

// Get current FIFO buffer count
static size_t fifo_buffer_available(void) {
    return fifo_buffer_count;
}

// Reset FIFO buffer
static void fifo_buffer_reset(void) {
    fifo_buffer_head = 0;
    fifo_buffer_tail = 0;
    fifo_buffer_count = 0;
}

// Simple font data for FPS overlay (8x8 pixel characters)
static const uint8_t font_8x8[16][8] = {
    {0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00000000}, // Space (ASCII 32)
    // Numbers 0-9
    {0b01111100,
     0b11000110,
     0b11001110,
     0b11010110,
     0b11100110,
     0b11000110,
     0b01111100,
     0b00000000}, // 0
    {0b00011000,
     0b00111000,
     0b01011000,
     0b00011000,
     0b00011000,
     0b00011000,
     0b01111110,
     0b00000000}, // 1
    {0b00111100,
     0b01100110,
     0b00000110,
     0b00011100,
     0b00110000,
     0b01100000,
     0b01111110,
     0b00000000}, // 2
    {0b00111100,
     0b01100110,
     0b00000110,
     0b00011100,
     0b00000110,
     0b01100110,
     0b00111100,
     0b00000000}, // 3
    {0b00001110,
     0b00011110,
     0b00110110,
     0b01100110,
     0b01111111,
     0b00000110,
     0b00001111,
     0b00000000}, // 4
    {0b01111110,
     0b01100000,
     0b01111100,
     0b00000110,
     0b00000110,
     0b01100110,
     0b00111100,
     0b00000000}, // 5
    {0b00011100,
     0b00110000,
     0b01100000,
     0b01111100,
     0b01100110,
     0b01100110,
     0b00111100,
     0b00000000}, // 6
    {0b01111110,
     0b01100110,
     0b00000110,
     0b00001100,
     0b00011000,
     0b00011000,
     0b00011000,
     0b00000000}, // 7
    {0b00111100,
     0b01100110,
     0b01100110,
     0b00111100,
     0b01100110,
     0b01100110,
     0b00111100,
     0b00000000}, // 8
    {0b00111100,
     0b01100110,
     0b01100110,
     0b00111110,
     0b00000110,
     0b00001100,
     0b00111000,
     0b00000000}, // 9
    // Dot
    {0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00000000,
     0b00011000,
     0b00011000,
     0b00000000}, // .
    // F, P, S
    {0b01111110,
     0b01100000,
     0b01100000,
     0b01111110,
     0b01100000,
     0b01100000,
     0b01100000,
     0b00000000}, // F
    {0b01111100,
     0b01100110,
     0b01100110,
     0b01111100,
     0b01100000,
     0b01100000,
     0b01100000,
     0b00000000}, // P
    {0b00111100,
     0b01100110,
     0b01110000,
     0b00111000,
     0b00001110,
     0b01100110,
     0b00111100,
     0b00000000}, // S
    // Colon
    {0b00000000,
     0b00011000,
     0b00011000,
     0b00000000,
     0b00000000,
     0b00011000,
     0b00011000,
     0b00000000}, // :
};

// Draw a single character on framebuffer
static void draw_char(uint32_t *framebuffer, int width, int height, int pitch, 
                      int x, int y, char c, uint32_t color) {
    // Only draw characters that we have defined in our font array
    // We only have characters: space, 0-9, ., F, P, S, :
    const char *valid_chars = " 0123456789.FPS:";
    const char *found = strchr(valid_chars, c);
    if (!found) return;
    
    int char_index = found - valid_chars;
    if (char_index >= (int)(sizeof(font_8x8)/sizeof(font_8x8[0]))) return;
    int pitch_4 = pitch / 4;
    
    for (int cy = 0; cy < 8; cy++) {
        for (int cx = 0; cx < 8; cx++) {
            if (font_8x8[char_index][cy] & (128 >> cx)) {
                int px = x + cx;
                int py = y + cy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    framebuffer[py * pitch_4 + px] = color;
                }
            }
        }
    }
}

// Draw FPS overlay on framebuffer
static void draw_fps_overlay(uint32_t *framebuffer, int width, int height, int pitch) {
    static uint32_t last_fps_display_time = 0;
    static double last_displayed_fps = 0.0;
    static uint32_t fps_start_time = 0;
    static uint32_t fps_frame_count = 0;
    uint32_t current_time = plm_get_microseconds();
    
    // Reset FPS calculation if frame_count was reset
    if (frame_count == 0) {
        fps_start_time = 0;
        fps_frame_count = 0;
        last_fps_display_time = 0;
        last_displayed_fps = 0.0;
    }
    
    // Update FPS calculation every 500ms to avoid flickering
    if (last_fps_display_time == 0 || (current_time - last_fps_display_time) > 500000) {
        // Initialize FPS measurement
        if (fps_start_time == 0) {
            fps_start_time = current_time;
            fps_frame_count = frame_count;
        } else {
            // Calculate actual FPS correctly
            uint32_t elapsed_time = current_time - fps_start_time;
            if (elapsed_time > 0 && frame_count > fps_frame_count) {
                last_displayed_fps = ((frame_count - fps_frame_count) * 1000000.0) / elapsed_time;
                
                // Reset for next measurement
                fps_start_time = current_time;
                fps_frame_count = frame_count;
            }
        }
        last_fps_display_time = current_time;
    }
    
    // Get target framerate from decoder
    double target_fps = 25.0; // Default
    if (plm_decoder && plm_get_video_enabled(plm_decoder)) {
        target_fps = plm_get_framerate(plm_decoder);
        if (target_fps <= 0) target_fps = 25.0;
    }
    
    // Draw FPS text overlay (white text on semi-transparent black background)
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f/%.1f", last_displayed_fps, target_fps);
/*    
    // Draw semi-transparent background
    uint32_t bg_color = 0x80000000; // Semi-transparent black
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 120; x++) {
            if (x < width && y < height) {
                framebuffer[(y+overlay_y) * (pitch / 4) + x] = bg_color;
            }
        }
    }
*/    
    // Draw text
    uint32_t text_color = 0xFFFFFFFF; // White
    for (int i = 0; i < (int)strlen(fps_text); i++) {
        draw_char(framebuffer, width, height, pitch, 2 + i * 8, 2+overlay_y, fps_text[i], text_color);
    }
}

// Audio callback function - sends decoded audio to Z3660 audio buffer
static void pl_mpeg_audio_callback(plm_t *plm, plm_samples_t *samples, void *user) {
    (void)plm;
    (void)user;
    
    static int audio_ready = 0; // Track if audio system is ready
    static int audio_callback_count = 0;
    
    audio_callback_count++;
    
    if (!samples || samples->count == 0) {
        printf("[PL_MPEG ARM AUDIO] No samples available\n");
        return;
    }
    
    // Activate audio system immediately (like AHI driver does when interrupt is detected)
    if (!audio_ready) {
        audio_ready = 1;
        extern int interrupt_enabled_audio_fake;
        interrupt_enabled_audio_fake = 1; // Enable fake interrupt for audio processing
        extern int audio_request_init;
        audio_set_tx_buffer((uint8_t *)(RTG_BASE+0x07000000));
        audio_adau_set_lpf_params(23900);
        audio_adau_set_mixer_vol(50, 50);
        audio_adau_set_prefactor(50);
        for(int i=0;i<10;i++)
            audio_adau_set_eq_gain(i, 50);
        audio_adau_set_vol_pan(50, 50);
        audio_request_init=1;
        printf("[PL_MPEG ARM AUDIO] Audio system activated, fake interrupt enabled\n");
    }

    // Initialize accumulation buffer if needed
    if (!audio_accumulation_buffer) {
        // Use a larger buffer to handle timing irregularities
        audio_accumulation_buffer = malloc(ACCUMULATION_BUFFER_SIZE_IN_SAMPLES*4);
        if (!audio_accumulation_buffer) {
            printf("[PL_MPEG ARM AUDIO] ERROR: Failed to allocate accumulation buffer\n");
            return;
        }
    }
    
    // Copy incoming samples to accumulation buffer using circular buffer
    for (unsigned int i = 0; i < samples->count; i++) {        
        #ifdef PLM_AUDIO_SEPARATE_CHANNELS
            // Separate channels
            int16_t left_sample = (int16_t)(samples->left[i] * 32767.0f);
            int16_t right_sample = (int16_t)(samples->right[i] * 32767.0f);
            audio_accumulation_buffer[audio_write_index*2    ] = left_sample;
            audio_accumulation_buffer[audio_write_index*2 + 1] = right_sample;
        #else
            // Interleaved channels
            int16_t left_sample  = (int16_t)(samples->interleaved[i * 2    ] * 32767.0f);
            int16_t right_sample = (int16_t)(samples->interleaved[i * 2 + 1] * 32767.0f);
            audio_accumulation_buffer[audio_write_index * 2    ] = left_sample;
            audio_accumulation_buffer[audio_write_index * 2 + 1] = right_sample;
        #endif
        // Update write index
        audio_write_index = (audio_write_index + 1) % (ACCUMULATION_BUFFER_SIZE_IN_SAMPLES); // Wrap around
    }
//    printf("w_idx %ld, r_idx %ld\n", audio_write_index, audio_read_index);
    audio_accumulated_samples += samples->count;
    // Debug: log accumulation
/*
    if (callback_count % 50 == 0) {
        printf("[PL_MPEG ARM AUDIO] Accumulated %lu samples from %u incoming samples\n", 
               audio_accumulated_samples, samples->count);
    }
*/
}

static int first_frame = 1;

// Video callback function - called by PL_MPEG when a frame is decoded
static void pl_mpeg_video_callback(plm_t *plm, plm_frame_t *frame, void *user) {
    (void)plm; // Unused parameter
    (void)user; // Unused parameter
    
//    printf("[PL_MPEG ARM] Video callback called\n");
    
    if (!plm_shared) {
        printf("[PL_MPEG ARM] ERROR: plm_shared is NULL\n");
        return;
    }
    if (!frame) {
        printf("[PL_MPEG ARM] ERROR: frame is NULL\n");
        return;
    }
    
    if (first_frame) {
        // First frame - send video init signal
        
        // Get video dimensions from PL_MPEG decoder (not from frame structure)
        int width = plm_get_width(plm);
        int height = plm_get_height(plm);
        double framerate = plm_get_framerate(plm);
        
        printf("[PL_MPEG ARM DEBUG] Video dimensions: width=%d, height=%d, fps=%.2f\n", width, height, framerate);
        printf("[PL_MPEG ARM DEBUG] Frame info: width=%d, height=%d\n", frame->width, frame->height);
        
        // Store framerate with endianness conversion
        union {
            double d;
            uint64_t u64;
        } converter;
        converter.d = framerate;
        uint64_t be_val = be64_to_host(converter.u64);
        memcpy(&plm_shared->framerate, &be_val, sizeof(double));
        
        // Store video format information for initialization
        plm_shared->frame_width = host_to_be32(width);
        plm_shared->frame_height = host_to_be32(height);
//        plm_shared->frame_stride = host_to_be32(width); // Assumes no padding
        plm_shared->frame_coded_width = host_to_be32(width);
        
        // Signal 68k to initialize video
        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
        
        first_frame = 0;
        printf("[PL_MPEG ARM] First frame dimensions set: %dx%d, signaling VIDEO_INIT\n", width, height);
    } else {
        // Subsequent frames - write directly to overlay framebuffer
        
// Not used because we write to the overlay framebuffer at a fixed address that is 32bit per pixel and has the same dimensions as the video,
// so we can write directly to it without worrying about the pitch or offsets.
//            uint32_t *framebuffer = (uint32_t *)be32_to_host(plm_shared->display_frame.framebuffer_addr);
//            uint32_t pitch = be32_to_host(plm_shared->display_frame.framebuffer_pitch);
        uint32_t *ovl_framebuffer = (uint32_t *)(RTG_BASE+0x06000000);//(uint32_t *)be32_to_host(plm_shared->display_frame.framebuffer_addr);
        uint32_t ovl_pitch = vs.vmode_hsize * 4; // Calculate pitch based on video mode width and 4 bytes per pixel
            
        if (ovl_framebuffer && ovl_pitch > 0) {
            // Convert PL_MPEG YUV frame to ARGB and write to framebuffer
            int width = frame->width;
            int height = frame->height;
#ifndef __ARM_NEON
            // Improved YUV to ARGB conversion for MPEG
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int y_idx = y * width + x;
                        
                    // Calculate UV indices correctly for 4:2:0 subsampling
                    int uv_width = width / 2;
//                    int uv_height = height / 2;
                    int uv_x = x / 2;
                    int uv_y = y / 2;
                    int uv_idx = uv_y * uv_width + uv_x;
                    
                    // Get YUV components
                    uint8_t y_val = frame->y.data[y_idx];
                    uint8_t u_val = frame->cb.data[uv_idx];
                    uint8_t v_val = frame->cr.data[uv_idx];
                    
                    if(dithering_intensity>0) {
                        // Apply fast anti-aliasing filter to reduce aliasing artifacts
                        // Ultra-fast anti-aliasing using selective sampling
                        // Much lighter than 3x3 kernel - only samples 4 pixels
                        int sum_y = y_val;
                        int count = 1;
                        
                        // UNROLLED: Sample only 4 diagonal pixels (much faster than loop)
                        // Check bounds once for all pixels to avoid repeated comparisons
                        int x_plus_ok  = (x + 1) < width;
                        int x_minus_ok = (x - 1) >= 0;
                        int y_plus_ok  = (y + 1) < height;
                        int y_minus_ok = (y - 1) >= 0;
                        
                        // Sample pixel (x+1, y+1)
                        if (x_plus_ok && y_plus_ok) {
                            int sample_idx = (y + 1) * width + (x + 1);
                            sum_y += frame->y.data[sample_idx];
                            count++;
                        }
                        
                        // Sample pixel (x-1, y+1)
                        if (x_minus_ok && y_plus_ok) {
                            int sample_idx = (y + 1) * width + (x - 1);
                            sum_y += frame->y.data[sample_idx];
                            count++;
                        }
                        
                        // Sample pixel (x+1, y-1)
                        if (x_plus_ok && y_minus_ok) {
                            int sample_idx = (y - 1) * width + (x + 1);
                            sum_y += frame->y.data[sample_idx];
                            count++;
                        }
                        
                        // Sample pixel (x-1, y-1)
                        if (x_minus_ok && y_minus_ok) {
                            int sample_idx = (y - 1) * width + (x - 1);
                            sum_y += frame->y.data[sample_idx];
                            count++;
                        }
                        
                        // Apply fast anti-aliasing with configurable intensity
                        if (count > 1) {
                            int avg_y = sum_y / count;
                            // Simple linear blend - higher intensity = more smoothing
                            // Use bit shifting for faster division
                            y_val = (y_val * (10 - dithering_intensity) + avg_y * dithering_intensity) / 10;
                        }
                    }
                    
                    // YUV to RGB conversion
                    float y_f = (y_val - 16) * 1.164f;
                    float u_f = u_val - 128;
                    float v_f = v_val - 128;
                    
                    int r, g, b;
                    
                    if(grayscale_mode) {
                        // Grayscale mode: use only Y component
                        r = g = b = (int)y_f;
                    }
                    else {
                        // Color mode: YUV to RGB conversion
                        r = (int)(y_f + 1.596f * v_f);
                        g = (int)(y_f - 0.391f * u_f - 0.813f * v_f);
                        b = (int)(y_f + 2.018f * u_f);
                    }
                    
                    // Clamp values
                    r = (r < 0) ? 0 : (r > 255) ? 255 : r;
                    g = (g < 0) ? 0 : (g > 255) ? 255 : g;
                    b = (b < 0) ? 0 : (b > 255) ? 255 : b;
                    
                    // Create ARGB pixel (0xFFRRGGBB)
                    uint32_t argb = 0xFF000000 | (r << 16) | (g << 8) | b;
                    
                    // Write to ovl_framebuffer
                    if(overlay_x>=0 || (overlay_x<0 && x+overlay_x>=0)) {
                        ovl_framebuffer[(y+overlay_y) * (ovl_pitch / 4) + x] = argb;
                    }
                }
            }
#else
            // Optimized YUV to ARGB conversion using ARM NEON SIMD intrinsics
            // Processes 8 pixels at a time for maximum throughput
            {
                uint8_t *y_plane = frame->y.data;
                uint8_t *u_plane = frame->cb.data;
                uint8_t *v_plane = frame->cr.data;
                int uv_width = width / 2;

                // NEON vector constants for YUV->RGB conversion
                // BT.601: R = 1.164*(Y-16) + 1.596*(V-128)
                //         G = 1.164*(Y-16) - 0.391*(U-128) - 0.813*(V-128)
                //         B = 1.164*(Y-16) + 2.018*(U-128)
                const int16x8_t v_y_offset  = vdupq_n_s16(16);
                const int16x8_t v_uv_offset = vdupq_n_s16(128);
                const float32x4_t v_y_scale = vdupq_n_f32(1.164f);
                const float32x4_t v_rv      = vdupq_n_f32(1.596f);
                const float32x4_t v_gu      = vdupq_n_f32(-0.391f);
                const float32x4_t v_gv      = vdupq_n_f32(-0.813f);
                const float32x4_t v_bu      = vdupq_n_f32(2.018f);
                const int32x4_t v_zero      = vdupq_n_s32(0);
                const int32x4_t v_255       = vdupq_n_s32(255);

                for (int y = 0; y < height; y++) {
                    int uv_y = y / 2;
                    uint8_t *y_row = y_plane + y * width;
                    uint8_t *u_row = u_plane + uv_y * uv_width;
                    uint8_t *v_row = v_plane + uv_y * uv_width;
                    uint32_t *fb_row = ovl_framebuffer + (y + overlay_y) * (ovl_pitch / 4);
                    int x = 0;

                    // Process 8 pixels at a time with NEON
                    for (; x <= width - 8; x += 8) {
                        int uv_x = x / 2;

                        // Load 8 Y values
                        uint8x8_t y_u8 = vld1_u8(y_row + x);

                        // Load only 4 U and 4 V values (4:2:0 subsampling)
                        // Each U/V covers a 2x2 block, so for 8 Y pixels we need 4 U and 4 V
                        // Then duplicate each to match 8 Y pixels
                        uint8x8_t u_u8 = vld1_u8(u_row + uv_x); // loads 8 bytes but only first 4 are valid
                        uint8x8_t v_u8 = vld1_u8(v_row + uv_x);

                        // Duplicate U and V: each of the 4 UV values covers 2 Y pixels
                        // vzip_u8 interleaves: [u0,u1,u2,u3,...] -> [u0,u0,u1,u1,u2,u2,u3,u3]
                        uint8x8x2_t u_zipped = vzip_u8(u_u8, u_u8);
                        uint8x8x2_t v_zipped = vzip_u8(v_u8, v_u8);
                        u_u8 = u_zipped.val[0]; // [u0,u0,u1,u1,u2,u2,u3,u3]
                        v_u8 = v_zipped.val[0]; // [v0,v0,v1,v1,v2,v2,v3,v3]

                        // Widen Y to 16-bit and subtract offset (Y-16)
                        int16x8_t y_s16 = vreinterpretq_s16_u16(vmovl_u8(y_u8));
                        y_s16 = vsubq_s16(y_s16, v_y_offset);

                        // Widen U and V to 16-bit and subtract offset (U-128, V-128)
                        int16x8_t u_s16 = vreinterpretq_s16_u16(vmovl_u8(u_u8));
                        int16x8_t v_s16 = vreinterpretq_s16_u16(vmovl_u8(v_u8));
                        u_s16 = vsubq_s16(u_s16, v_uv_offset);
                        v_s16 = vsubq_s16(v_s16, v_uv_offset);

                        // Widen to 32-bit for float multiplication
                        int32x4_t y_lo = vmovl_s16(vget_low_s16(y_s16));
                        int32x4_t y_hi = vmovl_s16(vget_high_s16(y_s16));
                        int32x4_t u_lo = vmovl_s16(vget_low_s16(u_s16));
                        int32x4_t u_hi = vmovl_s16(vget_high_s16(u_s16));
                        int32x4_t v_lo = vmovl_s16(vget_low_s16(v_s16));
                        int32x4_t v_hi = vmovl_s16(vget_high_s16(v_s16));

                        // Convert to float
                        float32x4_t yf_lo = vcvtq_f32_s32(y_lo);
                        float32x4_t yf_hi = vcvtq_f32_s32(y_hi);
                        float32x4_t uf_lo = vcvtq_f32_s32(u_lo);
                        float32x4_t uf_hi = vcvtq_f32_s32(u_hi);
                        float32x4_t vf_lo = vcvtq_f32_s32(v_lo);
                        float32x4_t vf_hi = vcvtq_f32_s32(v_hi);

                        // Scale Y: Y' = 1.164 * (Y-16)
                        yf_lo = vmulq_f32(yf_lo, v_y_scale);
                        yf_hi = vmulq_f32(yf_hi, v_y_scale);

                        // Calculate RGB components
                        float32x4_t r_lo, r_hi, g_lo, g_hi, b_lo, b_hi;

                        if(grayscale_mode)
                        {
                            // Grayscale: R = G = B = Y'
                            r_lo = yf_lo; r_hi = yf_hi;
                            g_lo = yf_lo; g_hi = yf_hi;
                            b_lo = yf_lo; b_hi = yf_hi;
                        }
                        else
                        {
                            // R = Y' + 1.596*V
                            r_lo = vmlaq_f32(yf_lo, vf_lo, v_rv);
                            r_hi = vmlaq_f32(yf_hi, vf_hi, v_rv);

                            // G = Y' - 0.391*U - 0.813*V
                            g_lo = vmlaq_f32(yf_lo, uf_lo, v_gu);
                            g_lo = vmlaq_f32(g_lo, vf_lo, v_gv);
                            g_hi = vmlaq_f32(yf_hi, uf_hi, v_gu);
                            g_hi = vmlaq_f32(g_hi, vf_hi, v_gv);

                            // B = Y' + 2.018*U
                            b_lo = vmlaq_f32(yf_lo, uf_lo, v_bu);
                            b_hi = vmlaq_f32(yf_hi, uf_hi, v_bu);
                        }
                        // Convert back to int32
                        int32x4_t r_i_lo = vcvtq_s32_f32(r_lo);
                        int32x4_t r_i_hi = vcvtq_s32_f32(r_hi);
                        int32x4_t g_i_lo = vcvtq_s32_f32(g_lo);
                        int32x4_t g_i_hi = vcvtq_s32_f32(g_hi);
                        int32x4_t b_i_lo = vcvtq_s32_f32(b_lo);
                        int32x4_t b_i_hi = vcvtq_s32_f32(b_hi);

                        // Clamp to 0-255
                        r_i_lo = vmaxq_s32(vminq_s32(r_i_lo, v_255), v_zero);
                        r_i_hi = vmaxq_s32(vminq_s32(r_i_hi, v_255), v_zero);
                        g_i_lo = vmaxq_s32(vminq_s32(g_i_lo, v_255), v_zero);
                        g_i_hi = vmaxq_s32(vminq_s32(g_i_hi, v_255), v_zero);
                        b_i_lo = vmaxq_s32(vminq_s32(b_i_lo, v_255), v_zero);
                        b_i_hi = vmaxq_s32(vminq_s32(b_i_hi, v_255), v_zero);

                        // Pack: saturate int32 to unsigned uint8
                        // vqmovun_s32: saturates signed int32 to unsigned uint16 (0-65535)
                        // Then vmovn_u16: narrows uint16 to uint8 (values already 0-255 after clamp)
                        uint16x4_t r_u16_lo = vqmovun_s32(r_i_lo);
                        uint16x4_t r_u16_hi = vqmovun_s32(r_i_hi);
                        uint16x4_t g_u16_lo = vqmovun_s32(g_i_lo);
                        uint16x4_t g_u16_hi = vqmovun_s32(g_i_hi);
                        uint16x4_t b_u16_lo = vqmovun_s32(b_i_lo);
                        uint16x4_t b_u16_hi = vqmovun_s32(b_i_hi);

                        uint8x8_t r_u8 = vmovn_u16(vcombine_u16(r_u16_lo, r_u16_hi));
                        uint8x8_t g_u8 = vmovn_u16(vcombine_u16(g_u16_lo, g_u16_hi));
                        uint8x8_t b_u8 = vmovn_u16(vcombine_u16(b_u16_lo, b_u16_hi));

                        // Build ARGB pixels as uint32: 0xAARRGGBB
                        // On ARM little-endian, uint32 0xFFRRGGBB is stored as bytes [BB,GG,RR,FF]
                        // So we interleave as [B,G,R,A] for vst4_u8 to get correct memory layout
                        uint8x8x4_t argb;
                        argb.val[0] = b_u8;  // Blue  (byte 0 = least significant)
                        argb.val[1] = g_u8;  // Green (byte 1)
                        argb.val[2] = r_u8;  // Red   (byte 2)
                        argb.val[3] = vdup_n_u8(0xFF); // Alpha (byte 3 = most significant)

                        // Store 8 ARGB pixels directly to framebuffer
                        vst4_u8((uint8_t*)(fb_row + x), argb);
                    }

                    // Scalar fallback for remaining pixels
                    for (; x < width; x++) {
                        int uv_x = x / 2;
                        uint8_t y_val = y_row[x];
                        uint8_t u_val = u_row[uv_x];
                        uint8_t v_val = v_row[uv_x];

                        float y_f = (y_val - 16) * 1.164f;
                        float u_f = u_val - 128;
                        float v_f = v_val - 128;

                        int r = (int)(y_f + 1.596f * v_f);
                        int g = (int)(y_f - 0.391f * u_f - 0.813f * v_f);
                        int b = (int)(y_f + 2.018f * u_f);

                        r = (r < 0) ? 0 : (r > 255) ? 255 : r;
                        g = (g < 0) ? 0 : (g > 255) ? 255 : g;
                        b = (b < 0) ? 0 : (b > 255) ? 255 : b;

                        uint32_t argb = 0xFF000000 | (r << 16) | (g << 8) | b;
                        if(overlay_x>=0 || (overlay_x<0 && x+overlay_x>=0)) {
                            fb_row[x] = argb;
                        }
                    }
                }
            }
#endif
            // Show FPS overlay on framebuffer for debugging
            if(show_fps)
                draw_fps_overlay(ovl_framebuffer, width, height, ovl_pitch);
            Xil_L2CacheFlushRange((uintptr_t)ovl_framebuffer, height * ovl_pitch);
        } else {
            printf("[PL_MPEG ARM] WARNING: Invalid framebuffer address or pitch\n");
        }
        
        // Signal 68k to refresh video display
//        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_REFRESH);

    }
}

// Initialize PL_MPEG decoder on ARM side
int pl_mpeg_arm_init(void) {
    if (!plm_shared) {
        printf("[PL_MPEG ARM] ERROR: Shared memory not set\n");
        return -1;
    }
    printf("[PL_MPEG ARM] Initializing PL_MPEG ARM handler...\n");
    // Ignore gray mode for PL_MPEG (parameter is unused)
//    (void)plm_shared->gray_mode;
    
    // Reset flags before initialization
    header_parsed = 0;
    
    // Reset FIFO buffer
    fifo_buffer_reset();
    
    // Initialize video info fields to zero until headers are parsed
    printf("plm_shared=%p\n",plm_shared);
    plm_shared->frame_width = 0;
    plm_shared->frame_height = 0;
    plm_shared->frame_coded_width = 0;
    plm_shared->framerate = 0.0;
    plm_shared->skipped_frames = 0;
    plm_shared->status = 0;
    plm_shared->error_code = 0;
    
    printf("[PL_MPEG ARM] PL_MPEG ARM handler initialized (buffer and decoder will be created when data arrives)\n");
    if(dithering_intensity>0)
        printf("[PL_MPEG ARM] Fast anti-aliasing ENABLED with intensity: %d\n", dithering_intensity);
    else
        printf("[PL_MPEG ARM] Anti-aliasing DISABLED\n");
    if(grayscale_mode)
        printf("[PL_MPEG ARM] Grayscale mode ENABLED\n");
    else
        printf("[PL_MPEG ARM] Color mode ENABLED\n");
    return 0;
}

// Process MPEG data using PL_MPEG
// Simplified function: Use only plm_video_t for ES streams
int pl_mpeg_arm_process(uint8_t *input_data, uint32_t input_length, double sync_time) {
    if (!plm_shared) {
        printf("[PL_MPEG ARM] ERROR: Shared memory not set\n");
        return -1;
    }
    // Convert sync_time from big-endian to host order
    union {
        double d;
        uint64_t u64;
    } sync_converter;
    memcpy(&sync_converter.u64, &sync_time, sizeof(uint64_t));
    sync_converter.u64 = be64_to_host(sync_converter.u64);
    (void)sync_converter.d; // Currently unused, but converted for future use
    
    // Store values back to shared memory in big-endian format
    plm_shared->input_length = host_to_be32(input_length);
    memcpy(&plm_shared->sync_time, &sync_time, sizeof(double)); // Already big-endian from 68k
    
    // Add data to FIFO buffer for accumulation
    fifo_buffer_add(input_data, input_length);
    // Only process if we have accumulated enough data
    if (fifo_buffer_available() >= 512*1024) {
        // Create buffer with accumulated data if needed
        if (!plm_buffer) {
            plm_buffer = plm_buffer_create_with_capacity(4 * 1024 * 1024);
            if (!plm_buffer) {
                printf("[PL_MPEG ARM] ERROR: Failed to create buffer\n");
                plm_shared->status = ARM_STATUS_ERROR;
                return -1;
            }
        }
        
        // Fill buffer with data from FIFO
        size_t fifo_data_size = fifo_buffer_available();
        if (fifo_data_size > 0) {
            uint8_t *temp_buffer = malloc(fifo_data_size);
            if (temp_buffer) {
                size_t bytes_read = fifo_buffer_get(temp_buffer, fifo_data_size);
                
                {
                    // Feed data directly to PL_MPEG buffer without ES->PS conversion
//                    size_t bytes_written = 
                    plm_buffer_write(plm_buffer, temp_buffer, bytes_read);
//                    printf("[PL_MPEG ARM] Wrote %zu bytes directly to PL_MPEG buffer (total: %zu)\n", 
//                           bytes_written, plm_buffer_get_size(plm_buffer));
                }                
                free(temp_buffer);
            }
        }
        
        // Try to create decoder if we don't have one and have substantial accumulated data
        if (!plm_decoder && plm_buffer_get_remaining(plm_buffer) > 524288) {  // Need substantial data for headers
            printf("[PL_MPEG ARM] Creating PL_MPEG decoder with %zu bytes of accumulated data\n", 
                   plm_buffer_get_remaining(plm_buffer));
            
            // Try to create decoder - PL_MPEG can auto-detect format
            plm_decoder = plm_create_with_buffer(plm_buffer, 1);
            arm_freq=get_current_cpu_frequency()/1000000;
            if (plm_decoder) {
                printf("[PL_MPEG ARM] PL_MPEG decoder created successfully\n");
                
                // Check if audio is available
                int has_audio = plm_get_num_audio_streams(plm_decoder);
                int samplerate = plm_get_samplerate(plm_decoder);
                if(samplerate==0)
                {
                    printf("samplerate_index %d\n",plm_decoder->audio_decoder->samplerate_index);
                    samplerate=44100;
                }
                int audio_enabled = plm_get_audio_enabled(plm_decoder);
                printf("[PL_MPEG ARM AUDIO] Audio streams: %d, Sample rate: %d Hz, Audio enabled: %d\n", 
                       has_audio, samplerate, audio_enabled);
                
                // Configure audio system
                write_rtg_register(REG_ZZ_AUDIO_PARAM, 0);
                write_rtg_register(REG_ZZ_AUDIO_VAL, (uint32_t)0x07DE0000); // DECODED buffer
                
                // Set audio scale like AHI driver does
                write_rtg_register(REG_ZZ_AUDIO_SCALE, samplerate / 50); // BuffSamples = MixFreq/50
                
                // Set sample rate for audio formatter
                write_rtg_register(REG_ZZ_AUDIO_PARAM, 9); // Sample rate parameter
                write_rtg_register(REG_ZZ_AUDIO_VAL, samplerate); // Actual sample rate

                plm_set_video_decode_callback(plm_decoder, pl_mpeg_video_callback, NULL);
                plm_set_audio_decode_callback(plm_decoder, pl_mpeg_audio_callback, NULL);
                
                printf("[PL_MPEG ARM] Video and audio callbacks registered\n");
                plm_set_audio_enabled(plm_decoder, 1);
                // Enable fake interrupt for audio processing
                extern int interrupt_enabled_audio_fake;
                interrupt_enabled_audio_fake = 1;
                                
                // Configure decoder
                plm_set_loop(plm_decoder, 0);
                first_frame = 1; // Reset first frame flag
                header_parsed = 1;
                
                // Enable video interrupt timer now that decoder is ready
                enable_video_interrupt_timer();
            } else {
                printf("[PL_MPEG ARM] WARNING: Could not create decoder yet, need more data\n");
                printf("[PL_MPEG ARM DEBUG] Buffer state:\n");
                printf("[PL_MPEG ARM DEBUG]   - Buffer size: %zu bytes\n", plm_buffer_get_size(plm_buffer));
                printf("[PL_MPEG ARM DEBUG]   - Buffer remaining: %zu bytes\n", plm_buffer_get_remaining(plm_buffer));
            }
        }
    }
    return 0;
}

// Reset PL_MPEG decoder to initial state
void pl_mpeg_arm_reset(void) {
    if (plm_buffer) {
        plm_buffer_rewind(plm_buffer);
    }
    
    // Reset flags and counters for next video stream
    header_parsed = 0;
    
    // Reset frame timing variables
    last_frame_time = 0;
    frame_count = 0;
    accumulated_delay = 0;
    last_data_time = 0;
    first_frame = 1;
    
    // Reset FIFO buffer
    fifo_buffer_reset();
    
    if (plm_shared) {
        plm_shared->error_code = 0;
        // Signal initialization on reset
        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
    }
    
    printf("[PL_MPEG ARM] PL_MPEG decoder reset (headers, frames, timing, and data counter reset)\n");
}

// Close and clean up PL_MPEG decoder resources
void pl_mpeg_arm_close(void) {
    printf("[PL_MPEG ARM] Closing PL_MPEG decoder resources...\n");
    
    // Reset headers flag for next session
//    headers_added = 0;
    
    // Disable video interrupt timer
    disable_video_interrupt_timer();
    
    // Disable fake interrupt when stopping audio
    extern int interrupt_enabled_audio_fake;
    interrupt_enabled_audio_fake = 0;
    audio_buff_offset = 0;
    audio_accumulated_samples=0;
    audio_read_index = 0;
    audio_write_index = 0;
//    if(audio_accumulation_buffer!=NULL)
//    {
//        free(audio_accumulation_buffer);
//        audio_accumulation_buffer=NULL;
//    }

    if (plm_decoder) {
        printf("[PL_MPEG ARM] Destroying PL_MPEG decoder...\n");
        plm_destroy(plm_decoder);
        plm_decoder = NULL;
    }
    
    if (plm_buffer) {
//        printf("[PL_MPEG ARM] Destroying PL_MPEG buffer...\n");
//        plm_buffer_destroy(plm_buffer); This hangs the ARM...
        plm_buffer = NULL;
    }
    
    // Reset FIFO buffer
    printf("[PL_MPEG ARM] Resetting FIFO buffer...\n");
    fifo_buffer_reset();
    
    if (plm_shared) {
        printf("[PL_MPEG ARM] Resetting shared memory status...\n");
    }
    // Audio system cleanup - reset audio hardware state
    audio_silence();
    plm_shared->status2 = host_to_be32(ARM_STATUS_FINISHED);
    printf("[PL_MPEG ARM] PL_MPEG decoder closed\n");
}

// Set shared memory pointer for ARM-68k communication
void pl_mpeg_set_shared_memory(arm_decoder_shared_arm_t *shared) {
    plm_shared = shared;
}

// Get PL_MPEG library version information
const char* pl_mpeg_arm_get_version(void) {
    return "PL_MPEG Z3660 Integration v1.1";
}

void fill_audio_buffer(void)
{
    if(audio_accumulated_samples<=1920 || audio_accumulation_buffer==NULL)
        return;

    int samplerate = plm_get_samplerate(plm_decoder);
    uint32_t num_samples_to_process = samplerate / 50; // Number of samples for 20ms at current sample rate
    int16_t* sdata = (int16_t*)(audio_tx_buffer + audio_buff_offset);
    for(uint32_t i=0; i < num_samples_to_process; i++) {
        // Copy samples from accumulation buffer to audio formatter buffer
        sdata[i * 2    ] = audio_accumulation_buffer[audio_read_index * 2    ]; // Left channel
        sdata[i * 2 + 1] = audio_accumulation_buffer[audio_read_index * 2 + 1]; // Right channel
        // Update read index and wrap around if needed
        audio_read_index = (audio_read_index + 1) % (ACCUMULATION_BUFFER_SIZE_IN_SAMPLES); // Wrap around
    }
    // Update accumulated samples count after processing
    audio_accumulated_samples-=num_samples_to_process;
    resample_s16(sdata,         (int16_t*)((uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2),
                 samplerate/50, 48000/50);
    memcpy(audio_tx_buffer + audio_buff_offset, (uint8_t*)audio_tx_buffer+AUDIO_TX_BUFFER_SIZE*2, AUDIO_BYTES_PER_PERIOD);

    audio_buff_offset += AUDIO_BYTES_PER_PERIOD;
    if (audio_buff_offset >= AUDIO_TX_BUFFER_SIZE) {
        audio_buff_offset = 0;
    }
}
// Progressive decoding with frame timing control
int pl_mpeg_arm_decode_progressive(void) {

    static int run_counter=0;
    run_counter++;
    if(run_counter==2)
    {
        run_counter=0;
        return 0;
    }
    if (!plm_shared || !plm_decoder) {
        plm_shared->megabytes_remaining = host_to_be32(0>>20); // Convert to MB
        plm_shared->bytes_remaining = host_to_be32(0&((1L<<20)-1)); // Convert to MB
        return 0;
    }
    
    // Get current time for timing control
    uint32_t current_time = plm_get_microseconds();
    
    // Get target frame interval based on framerate
    double framerate = plm_get_framerate(plm_decoder);
    if (framerate <= 0) framerate = 25.0; // Default to 25 fps
    
    // Audio-video synchronization: Use audio_samples->time for precise synchronization
    if (plm_get_audio_enabled(plm_decoder)) {
        plm_samples_t *audio_samples = plm_decode_audio(plm_decoder);
        if (audio_samples && audio_samples->count > 0) {
            pl_mpeg_audio_callback(plm_decoder, audio_samples, NULL);
        }
    }
    // Check if it's time to decode the next frame
    // Also check if we have a pending interrupt
    if (last_frame_time == 0 ||
        video_local_interrupt) {
        int number_of_frames_to_decode=video_local_interrupt;
        video_local_interrupt=0;
        for(int i=0;i<number_of_frames_to_decode;i++) {
            plm_frame_t *frame = NULL;
            if (plm_get_video_enabled(plm_decoder)) {
                frame = plm_decode_video(plm_decoder);
            }
            
            if (frame) {
                // Call video callback only for the first frame (skip other frames)
                if(i==0)
                    pl_mpeg_video_callback(plm_decoder, frame, NULL);
                
                // Update frame timing
                last_frame_time = current_time;
                frame_count++;
                
                // Estimate bytes processed based on buffer consumption
                
                // Update shared memory with processed bytes count
//                size_t total_data_remaining = plm_buffer_get_remaining(plm_buffer);
//                plm_shared->megabytes_remaining = host_to_be32(total_data_remaining>>20); // Convert to MB
//                plm_shared->bytes_remaining = host_to_be32(total_data_remaining&((1L<<20)-1)); // Convert to MB
                plm_shared->megabytes_remaining = host_to_be32(fifo_buffer_count>>20); // Convert to MB
                plm_shared->bytes_remaining = host_to_be32(fifo_buffer_count&((1L<<20)-1)); // Convert to MB
                
                // Update shared memory with synchronization information
                double video_time = plm_get_time(plm_decoder);
                union {
                    double d;
                    uint64_t u64;
                } time_converter;
                time_converter.d = video_time;
                uint64_t be_time = host_to_be64(time_converter.u64);
                memcpy(&plm_shared->sync_time, &be_time, sizeof(double));
                
                // Signal video refresh
                plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
                
                // Debug output every 100 frames
                if (frame_count % 100 == 0) {
                    printf("[PL_MPEG ARM] Progressive decode: Frame %lu @ %.2f fps, Time: %.3fs\n", 
                        (unsigned long)frame_count, framerate, video_time);
                }
            } else {
                plm_shared->megabytes_remaining = host_to_be32(0>>20); // Convert to MB
                plm_shared->bytes_remaining = host_to_be32(0&((1L<<20)-1)); // Convert to MB
                // No frame available - check if we need more data
                size_t remaining = plm_buffer_get_remaining(plm_buffer);
                if (remaining <= 100) {
                    plm_shared->status2 = host_to_be32(ARM_STATUS_FINISHED);
                    return 0; // No data available
                }
                printf("[PL_MPEG ARM] Frame not available\n");
                return 0; // Frame not available
            }
        }
        return 1; // Frame decoded successfully
    } else {
        // Not time to decode yet - simply return and wait for next call
        // The RTG main loop will call this function periodically
        return 0; // Waiting for next frame interval
    }
}
/*
#include "../../pt/pt.h"
int mpg_thread(struct pt *pt)
{

    PT_BEGIN(pt);
    while(1)
    {
//        PT_WAIT_UNTIL(pt,!plm_shared || !plm_decoder);
        pl_mpeg_arm_decode_progressive();
    }
    PT_END(pt);
}
*/
// Video timer interrupt handler
void isr_video_timer(void *dummy) {
    (void)dummy;
   
    if (interrupt_enabled_video && plm_decoder)
        video_local_interrupt++;

    // Clear interrupt status
    XScuTimer_ClearInterruptStatus(&VideoTimerInstance);
}

// Enable video interrupt timer
void enable_video_interrupt_timer(void) {
   printf("[PL_MPEG ARM] Enabling video interrupt timer...\n");
   
   // Initialize timer if not already done
   if (!video_timer_initialized) {
      XScuTimer_Config *ConfigPtr;
      int Status;
      
      // Lookup timer configuration
      ConfigPtr = XScuTimer_LookupConfig(XPAR_SCUTIMER_DEVICE_ID);
      if (!ConfigPtr) {
         printf("[PL_MPEG ARM] ERROR: Failed to find timer configuration\n");
         return;
      }
      
      // Initialize timer
      Status = XScuTimer_CfgInitialize(&VideoTimerInstance, ConfigPtr, ConfigPtr->BaseAddr);
      if (Status != XST_SUCCESS) {
         printf("[PL_MPEG ARM] ERROR: Failed to initialize timer\n");
         return;
      }
      
      // Self-test timer
      Status = XScuTimer_SelfTest(&VideoTimerInstance);
      if (Status != XST_SUCCESS) {
         printf("[PL_MPEG ARM] ERROR: Timer self-test failed\n");
         return;
      }
      
      video_timer_initialized = 1;
      printf("[PL_MPEG ARM] Video timer initialized successfully\n");
   }
   
   // Configure timer for video frame rate
   double framerate = 25.0; // Default framerate
   if (plm_decoder) {
       framerate = plm_get_framerate(plm_decoder);
       if (framerate <= 0) framerate = 25.0;
    }
   
    // Calculate timer interval for desired frame rate (microseconds per frame)
    uint32_t timer_interval_us = (uint32_t)(1000000.0 / framerate);

    // Convert to timer ticks using 64-bit math
    // Timer typically runs at half CPU frequency (550 MHz for 1100 MHz CPU)
    // arm_freq is in MHz, so timer_freq = (arm_freq / 2) MHz
    // ticks = timer_interval_us × timer_freq
    uint32_t timer_load_value = (uint32_t)((uint64_t)timer_interval_us * (uint64_t)(arm_freq / 2));
   
   printf("[PL_MPEG ARM] Setting video timer interval: %lu us (%lu ticks) for %.2f fps, arm_freq=%lu MHz, timer_freq=%lu MHz\n", 
          timer_interval_us, timer_load_value, framerate, (unsigned long)arm_freq, (unsigned long)(arm_freq / 2));
   printf("[PL_MPEG ARM DEBUG] Timer config: XPAR_SCUTIMER_DEVICE_ID=%d, XPAR_SCUTIMER_INTR=%d\n",
          XPAR_SCUTIMER_DEVICE_ID, XPAR_SCUTIMER_INTR);
   
   // Configure timer
   XScuTimer_LoadTimer(&VideoTimerInstance, timer_load_value);
   XScuTimer_EnableAutoReload(&VideoTimerInstance);
   XScuTimer_EnableInterrupt(&VideoTimerInstance);
   
   // Connect timer interrupt to GIC
   extern XScuGic int_handler;
   XScuGic_Connect(&int_handler, XPAR_SCUTIMER_INTR, 
                   (Xil_ExceptionHandler)isr_video_timer, &VideoTimerInstance);
   
   // Enable interrupt at GIC
   XScuGic_Enable(&int_handler, XPAR_SCUTIMER_INTR);
   
   // Start timer
   XScuTimer_Start(&VideoTimerInstance);
   
   interrupt_enabled_video = 1;
   video_local_interrupt = 0;
   printf("[PL_MPEG ARM] Video interrupt timer enabled\n");
}

// Disable video interrupt timer
void disable_video_interrupt_timer(void) {
   if (video_timer_initialized) {
      XScuTimer_Stop(&VideoTimerInstance);
      XScuTimer_DisableInterrupt(&VideoTimerInstance);
      
      // Disable interrupt at GIC
      extern XScuGic int_handler;
      XScuGic_Disable(&int_handler, XPAR_SCUTIMER_INTR);
   }
   
   interrupt_enabled_video = 0;
   video_local_interrupt = 0;
   printf("[PL_MPEG ARM] Video interrupt timer disabled\n");
}
