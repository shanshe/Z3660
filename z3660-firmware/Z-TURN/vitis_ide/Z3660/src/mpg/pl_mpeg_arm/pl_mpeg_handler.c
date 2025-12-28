/*
 * pl_mpeg_handler.c - ARM-side PL_MPEG Handler for Z3660
 *
 * Implementation of ARM-side PL_MPEG decoder functions that interface
 * with the 68k processor via shared memory communication.
 */

#include <stdio.h>
#include <string.h>

#include "mpg/pl_mpeg.h"
#include "mpg/libmpeg_arm/mpeg2dec_arm_comm_arm.h"

// Endianness conversion macros for ARM (little-endian) to 68k (big-endian)
#define be16_to_host(x) __builtin_bswap16(x)
#define be32_to_host(x) __builtin_bswap32(x)
#define be64_to_host(x) __builtin_bswap64(x)
#define host_to_be32(x) __builtin_bswap32(x)
#define host_to_be16(x) __builtin_bswap16(x)

// Global PL_MPEG instance
static plm_t *plm_decoder = NULL;
static plm_buffer_t *plm_buffer = NULL;
static arm_decoder_shared_arm_t *plm_shared = NULL; // Use the original structure
static int header_parsed = 0; // Flag to track if video headers have been parsed
static size_t total_data_received = 0; // Track total data accumulated for header parsing

// Get microsecond timestamp for performance measurement
static uint32_t plm_get_microseconds(void) {
    // Simple implementation - should be replaced with actual hardware timer
    static uint32_t micros = 0;
    return micros += 1000; // Increment by 1ms each call as placeholder
}

// Video callback function - called by PL_MPEG when a frame is decoded
static void pl_mpeg_video_callback(plm_t *plm, plm_frame_t *frame, void *user) {
    (void)plm; // Unused parameter
    (void)user; // Unused parameter
    
    if (!plm_shared || !frame) return;
    
    static int first_frame = 1;
    
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
        // Subsequent frames - send refresh signal and copy frame data
        
        // Convert PL_MPEG frame to output format and copy to shared memory
        //if (plm_shared->display_frame.frame_base_arm[0])
        {
            // Convert frame format and copy to display frame buffer
            // PL_MPEG frames might need conversion to the expected YUV format
            // For now, just mark as having valid frame data
            plm_shared->display_frame.drop_flag = host_to_be16(0); // Don't drop frame
            
            // Store timing information
            union {
                double d;
                uint64_t u64;
            } time_converter;
            time_converter.d = plm_shared->sync_time;
            uint64_t be_time = be64_to_host(time_converter.u64);
            memcpy(&plm_shared->display_frame.sync_time, &be_time, sizeof(double));
            
            // TODO: Implement actual frame data conversion from PL_MPEG format
            // to the expected YUV format for the display frame buffers
        }
        
        // Signal 68k to refresh video display
        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_REFRESH);
        
        printf("[PL_MPEG ARM DEBUG] Frame dimensions in callback: %dx%d\n", 
               frame->width, frame->height);
        printf("[PL_MPEG ARM] Frame decoded, signaling VIDEO_REFRESH\n");
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
    total_data_received = 0;
    
    // Initialize video info fields to zero until headers are parsed
    printf("plm_shared=%p\n",plm_shared);
    plm_shared->frame_width = 0;
    printf("after setting plm_shared->frame_width\n");
    plm_shared->frame_height = 0;
    printf("after setting plm_shared->frame_height\n");
    plm_shared->frame_coded_width = 0;
    printf("after setting plm_shared->frame_coded_width\n");
    plm_shared->framerate = 0.0;
    printf("after setting plm_shared->framerate\n");
    plm_shared->skipped_frames = 0;
    printf("after init plm_shared fields\n");
    plm_shared->status = ARM_STATUS_READY;
    plm_shared->error_code = 0;
    
    printf("[PL_MPEG ARM] PL_MPEG ARM handler initialized (buffer and decoder will be created when data arrives)\n");
    return 0;
}


// Process MPEG data using PL_MPEG
// Simplified function: Use only plm_video_t for ES streams
int pl_mpeg_arm_process(uint8_t *input_data, uint32_t input_length, double sync_time) {
    if (!plm_shared) {
        printf("[PL_MPEG ARM] ERROR: Shared memory not set\n");
        return -1;
    }
    
    // CRITICAL FIX: Check endianness - input_length might already be in host order
    uint32_t converted_length;
    
    // Debug raw values
    printf("[PL_MPEG ARM DEBUG] RAW: input_length=0x%08lx\n", input_length);
    
    // Check if this is big-endian (68k) by testing the high bytes
    if (input_length > 0x00FFFFFF) {
        // This looks like big-endian 68k data, needs conversion
        converted_length = be32_to_host(input_length);
        printf("[PL_MPEG ARM DEBUG] Big-endian detected: 0x%08lx -> %lu\n", input_length, converted_length);
    } else {
        // This might already be in host order
        converted_length = input_length;
        printf("[PL_MPEG ARM DEBUG] Host order detected: %lu bytes\n", converted_length);
    }
    
    // Convert sync_time from big-endian to host order
    union {
        double d;
        uint64_t u64;
    } sync_converter;
    memcpy(&sync_converter.u64, &sync_time, sizeof(uint64_t));
    sync_converter.u64 = be64_to_host(sync_converter.u64);
    (void)sync_converter.d; // Currently unused, but converted for future use
    
    (void)plm_get_microseconds(); // Measure time placeholder
    plm_shared->status = ARM_STATUS_BUSY;
    
    // Store values back to shared memory in big-endian format
    plm_shared->input_length = host_to_be32(converted_length);
    memcpy(&plm_shared->sync_time, &sync_time, sizeof(double)); // Already big-endian from 68k
    
    // CRITICAL FIX: Completely bypass PL_MPEG for ES streams
    if (!plm_buffer) {
        plm_buffer = plm_buffer_create_with_capacity(1024 * 1024);
        if (!plm_buffer) {
            printf("[PL_MPEG ARM] ERROR: Failed to create buffer\n");
            return -1;
        }
    }
    
    // Debug: Check first few bytes of MPEG data for validity
    printf("[PL_MPEG ARM DEBUG] buf: 0x%08lx, length: %lu\n", (unsigned long)input_data, converted_length);
    printf("[PL_MPEG ARM DEBUG] First 16 bytes: ");
    for (uint32_t i = 0; i < 16 && i < converted_length; i++) {
        printf("%02x ", input_data[i]);
    }
    printf("\n");
    
    // Add data to buffer
    size_t bytes_written = plm_buffer_write(plm_buffer, input_data, converted_length);
    if (bytes_written != converted_length) {
        printf("[PL_MPEG ARM] WARNING: Only wrote %zu/%lu bytes\n", 
               bytes_written, converted_length);
    }
    
    total_data_received += converted_length;
    
    // Direct video decoder approach for ES streams
    if (!header_parsed && total_data_received > 4096) {
        printf("[PL_MPEG ARM] Attempting direct ES stream decoding with %zu bytes\n", total_data_received);
        
        // Create video decoder directly (bypasses PL_MPEG's PS requirements)
        plm_video_t *video_decoder = plm_video_create_with_buffer(plm_buffer, 0);
        if (video_decoder) {
            plm_video_rewind(video_decoder);
            
            // Try to get dimensions
            int width = plm_video_get_width(video_decoder);
            int height = plm_video_get_height(video_decoder);
            printf("[PL_MPEG ARM] Direct video decoder reports: %dx%d\n", width, height);
            
            if (width == 0 || height == 0) {
                // Try decoding a frame to force header parsing
                printf("[PL_MPEG ARM] Forcing header detection by decoding a frame...\n");
                plm_frame_t *frame = plm_video_decode(video_decoder);
                if (frame) {
                    width = frame->width;
                    height = frame->height;
                    printf("[PL_MPEG ARM] Frame decoded: %dx%d\n", width, height);
                }
            }
            
            if (width > 0 && height > 0) {
                double framerate = plm_video_get_framerate(video_decoder);
                
                // Update shared memory
                plm_shared->frame_width = host_to_be32(width);
                plm_shared->frame_height = host_to_be32(height);
                plm_shared->frame_coded_width = host_to_be32(width);
                
                union {
                    double d;
                    uint64_t u64;
                } fr_converter;
                fr_converter.d = framerate;
                uint64_t be_fr = be64_to_host(fr_converter.u64);
                memcpy(&plm_shared->framerate, &be_fr, sizeof(double));
                
                header_parsed = 1;
                plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
                printf("[PL_MPEG ARM] ES stream dimensions detected: %dx%d @ %.2f fps\n", width, height, framerate);
                plm_shared->status = ARM_STATUS_VIDEO_REFRESH;
            }
            
            plm_video_destroy(video_decoder);
        }
    }
    
    plm_shared->status = ARM_STATUS_READY;
    return 0;
    
    // First, try to parse headers if not done yet
    if (!header_parsed) {
        total_data_received += converted_length;
        
        printf("[PL_MPEG ARM DEBUG] Total accumulated data: %zu bytes\n", total_data_received);
        
        // DEBUG: Check what's at the VERY beginning of our accumulated data
        if (total_data_received >= 100) {
            printf("[PL_MPEG ARM DEBUG] ES stream detected: PL_MPEG can't parse without pack headers\n");
            printf("[PL_MPEG ARM DEBUG] Fixing by completely bypassing PL_MPEG's standard initialization\n");
        }
        
        // For MPEG video, we need significantly more data to parse headers correctly
        if (total_data_received >= 4096) { 
            printf("[PL_MPEG ARM DEBUG] Have %zu bytes, attempting to force header parsing\n", total_data_received);
            
            // CRITICAL: Force PL_MPEG to reinitialize decoders with current data
            printf("[PL_MPEG ARM DEBUG] Forcing decoder reinitialization...\n");
            
            // Rewind to the beginning to ensure we're reading from the start
            plm_rewind(plm_decoder);
            
            // ES stream: Dimensions will be detected by direct video decoder
            int width = 0;
            int height = 0;
            double framerate = 0.0;
            
            printf("[PL_MPEG ARM DEBUG] ES stream - direct video decoder approach\n");
            
            // DEBUG: Check if we're getting any dimensions at all
            if (width == 0 && height == 0) {
                printf("[PL_MPEG ARM DEBUG] No dimensions detected - forcing multiple rewind attempts\n");
                
                // Try multiple rewinds to force header detection
                for (int rewind_attempt = 0; rewind_attempt < 3; rewind_attempt++) {
                    printf("[PL_MPEG ARM DEBUG] Rewind attempt %d\n", rewind_attempt + 1);
                    if (plm_buffer) {
                        plm_buffer_rewind(plm_buffer);
                    }
                    
                    // Small delay
                    for (volatile int i = 0; i < 1000; i++);
                    
                    width = 0;  // Reset since we're not using PL_MPEG decoder
                    height = 0;
                    printf("[PL_MPEG ARM DEBUG] After rewind %d: ES stream, dimensions will be detected by direct video decoder\n", 
                           rewind_attempt + 1);
                    
                    if (width > 0 && height > 0) break;
                }
            }
            
            if (width > 0 && height > 0) {
                // Success! Headers are detected
                printf("[PL_MPEG ARM DEBUG]  Headers parsed successfully: %dx%d @ %.2f fps\n", 
                       width, height, framerate);
                
                // Update shared memory with video info
                plm_shared->frame_width = host_to_be32(width);
                plm_shared->frame_height = host_to_be32(height);
                plm_shared->frame_coded_width = host_to_be32(width);
                
                // Store framerate
                union {
                    double d;
                    uint64_t u64;
                } frame_converter;
                frame_converter.d = framerate;
                uint64_t be_val = be64_to_host(frame_converter.u64);
                memcpy(&plm_shared->framerate, &be_val, sizeof(double));
                
                header_parsed = 1;
                // Send video init signal to 68k
                plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
                printf("[PL_MPEG ARM]  Video headers parsed, signaling VIDEO_INIT to 68k\n");
            } else {
                // Headers not detected - try to decode frames to force header parsing
                printf("[PL_MPEG ARM DEBUG]  Direct dimension check failed, trying decode approach\n");
                
                int max_decode_attempts = 5;
                int frames_decoded = 0;
                
                for (int attempt = 0; attempt < max_decode_attempts; attempt++) {
                    printf("[PL_MPEG ARM DEBUG]  Decode attempt %d/%d\n", attempt + 1, max_decode_attempts);
                    
                    plm_frame_t *frame = plm_decode_video(plm_decoder);
                    
                    if (frame) {
                        frames_decoded++;
                        printf("[PL_MPEG ARM DEBUG]  Frame %d decoded: %dx%d\n", 
                               frames_decoded, frame->width, frame->height);
                        
                        // Update dimensions from the decoded frame
                        width = frame->width;
                        height = frame->height;
                        framerate = plm_get_framerate(plm_decoder);
                        
                        printf("[PL_MPEG ARM DEBUG]  Using frame dimensions: %dx%d @ %.2f fps\n", 
                               width, height, framerate);
                        
                        // Update shared memory
                        plm_shared->frame_width = host_to_be32(width);
                        plm_shared->frame_height = host_to_be32(height);
                        plm_shared->frame_coded_width = host_to_be32(width);
                        
                        union {
                            double d;
                            uint64_t u64;
                        } frame_converter;
                        frame_converter.d = framerate;
                        uint64_t be_val = be64_to_host(frame_converter.u64);
                        memcpy(&plm_shared->framerate, &be_val, sizeof(double));
                        
                        header_parsed = 1;
                        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
                        printf("[PL_MPEG ARM]  Frame-based dimensions obtained, signaling VIDEO_INIT\n");
                        break;
                    } else {
                        printf("[PL_MPEG ARM DEBUG]  Frame decode attempt %d failed\n", attempt + 1);
                        
                        // Check dimensions after failed decode
                    width = 0; // Will be set by direct video decoder
                    height = 0;
                        printf("[PL_MPEG ARM DEBUG]  After failed decode: width=%d, height=%d\n", 
                               width, height);
                        
                        if (width > 0 && height > 0) {
                            // Dimensions might have been set by the decode attempt
                            header_parsed = 1;
                            break;
                        }
                        
                        // Try seeking back if we're not at the beginning
                        size_t current_buffer_size = plm_buffer_get_size(plm_buffer);
                        if (plm_buffer_get_remaining(plm_buffer) < current_buffer_size) {
                            if (plm_buffer) {
                                plm_buffer_rewind(plm_buffer);
                            }
                            printf("[PL_MPEG ARM DEBUG]  Rewinding buffer for next attempt\n");
                        }
                    }
                }
                
                if (!header_parsed && total_data_received >= 16384) {
                    printf("[PL_MPEG ARM DEBUG]  Still no headers after %zu bytes and %d decode attempts\n", 
                           total_data_received, max_decode_attempts);
                    printf("[PL_MPEG ARM DEBUG]  Stream might be incompatible with PL_MPEG\n");
                }
            }
        } else {
            printf("[PL_MPEG ARM DEBUG]  Need more data for header parsing (%zu/%d bytes)\n", 
                   total_data_received, 8192);
        }
    }
    
    // Only attempt to decode frames if headers are parsed
    if (header_parsed) {
        // Attempt to decode one frame
        plm_frame_t *frame = plm_decode_video(plm_decoder);
        
        if (frame) {
            // Frame decoded successfully - callback will update shared memory
            plm_shared->status = ARM_STATUS_VIDEO_REFRESH;
            printf("[PL_MPEG ARM] Frame decoded successfully (%dx%d)\n", frame->width, frame->height);
        } else {
            // No frame available - this might be normal if we're still accumulating data
            // or if the frame requires more data than we have
            printf("[PL_MPEG ARM] No frame available with current data\n");
            plm_shared->status = ARM_STATUS_READY; // Not an error, just waiting for more data
        }
    } else {
        // Headers not parsed yet - try alternative approach for ES streams
        printf("[PL_MPEG ARM] Waiting for video headers (accumulated %zu bytes)\n", total_data_received);
        
    // CRITICAL FIX: For Elementary Streams (ES), create video decoder directly and force detection
    if (total_data_received >= 4096 && !header_parsed) {
        printf("[PL_MPEG ARM] Attempting direct video decoder creation for ES stream...\n");
        
        // Create a separate video decoder directly (bypassing PL_MPEG demuxer)
        plm_video_t *video_decoder = plm_video_create_with_buffer(plm_buffer, 0); // Don't destroy buffer
        if (video_decoder) {
            printf("[PL_MPEG ARM] Direct video decoder created successfully\n");
            
            // CRITICAL: Force video decoder to parse headers by seeking and checking
            plm_video_rewind(video_decoder);
            
            // Try multiple attempts to force header detection
            int video_width = 0, video_height = 0;
            int decode_attempts = 3;
            
            for (int attempt = 0; attempt < decode_attempts; attempt++) {
                printf("[PL_MPEG ARM] Direct video decoder attempt %d/%d\n", attempt + 1, decode_attempts);
                
                // Try to decode a frame to force header parsing
                plm_frame_t *frame = plm_video_decode(video_decoder);
                if (frame) {
                    video_width = frame->width;
                    video_height = frame->height;
                    printf("[PL_MPEG ARM] Frame decoded: %dx%d\n", video_width, video_height);
                    break;
                } else {
                    // Frame decode failed, try rewinding and forcing header detection
                    plm_video_rewind(video_decoder);
                    
                    // Force header parsing by checking dimensions multiple times
                    for (int i = 0; i < 5; i++) {
                        video_width = plm_video_get_width(video_decoder);
                        video_height = plm_video_get_height(video_decoder);
                        if (video_width > 0 && video_height > 0) break;
                        
                        // Small delay to let decoder process
                        for (volatile int j = 0; j < 1000; j++);
                    }
                    
                    if (video_width > 0 && video_height > 0) {
                        printf("[PL_MPEG ARM] Dimensions detected after forced parsing: %dx%d\n", video_width, video_height);
                        break;
                    }
                }
            }
            
            double video_framerate = plm_video_get_framerate(video_decoder);
            printf("[PL_MPEG ARM] Direct video decoder final check: %dx%d @ %.2f fps\n", 
                   video_width, video_height, video_framerate);
            
            if (video_width > 0 && video_height > 0) {
                // Update shared memory with video info
                plm_shared->frame_width = host_to_be32(video_width);
                plm_shared->frame_height = host_to_be32(video_height);
                plm_shared->frame_coded_width = host_to_be32(video_width);
                
                // Store framerate
                union {
                    double d;
                    uint64_t u64;
                } frame_converter;
                frame_converter.d = video_framerate;
                uint64_t be_val = be64_to_host(frame_converter.u64);
                memcpy(&plm_shared->framerate, &be_val, sizeof(double));
                
                header_parsed = 1;
                plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
                printf("[PL_MPEG ARM] Video headers parsed via direct video decoder\n");
                
                // Clean up the temporary decoder
                plm_video_destroy(video_decoder);
            } else {
                printf("[PL_MPEG ARM] Direct video decoder failed to detect dimensions after %d attempts\n", decode_attempts);
                plm_video_destroy(video_decoder);
                plm_shared->status = ARM_STATUS_READY;
            }
        } else {
            printf("[PL_MPEG ARM] Failed to create direct video decoder\n");
            plm_shared->status = ARM_STATUS_READY;
        }
    } else {
        plm_shared->status = ARM_STATUS_READY;
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
    total_data_received = 0;
    
    if (plm_shared) {
        plm_shared->status = ARM_STATUS_READY;
        plm_shared->error_code = 0;
        // Signal initialization on reset
        plm_shared->status2 = host_to_be32(ARM_STATUS_VIDEO_INIT);
    }
    
    printf("[PL_MPEG ARM] PL_MPEG decoder reset (headers, frames, and data counter reset)\n");
}

// Close and clean up PL_MPEG decoder resources
void pl_mpeg_arm_close(void) {
    if (plm_decoder) {
        plm_destroy(plm_decoder);
        plm_decoder = NULL;
    }
    
    if (plm_buffer) {
        plm_buffer_destroy(plm_buffer);
        plm_buffer = NULL;
    }
    
    if (plm_shared) {
        plm_shared->status = ARM_STATUS_READY;
    }
    
    printf("[PL_MPEG ARM] PL_MPEG decoder closed\n");
}

// Set shared memory pointer for ARM-68k communication
void pl_mpeg_set_shared_memory(arm_decoder_shared_arm_t *shared) {
    plm_shared = shared;
}

// Get PL_MPEG library version information
const char* pl_mpeg_arm_get_version(void) {
    return "PL_MPEG Z3660 Integration v1.0";
}
