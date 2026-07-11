// This file has two main functions: mother board test, and BOOT.bin update system.
#define PL_MPEG_USE_FILE 1
#define XFILE FIL

#include <inttypes.h>
#include "mobotest.h"
#include "sd_fileops.h"
#include <xparameters.h>
#include "main.h"
#include "lwip.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sys.h"
#include "arch/sys_arch.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "rtg/fonts.h"
#include "config_file.h"
#include "xtime_l.h"
#include "xil_mmu.h"
#include "lwip/tftp_server.h"
#include "lwip/web_utils.h"
#include <stdbool.h>
#include "sii9022_init/sii9022_init.h"

#include "pt/pt.h"
#include "ARM_ztop/tabs.h"
#include "usb.h"

#include "ARM_ztop/textedit.h"
#include "ARM_ztop/tabs.h"
#include "coremark/coremark_port.h"
#include "ff.h"
#include "video.h"
#include "rtg/zz_video_modes.h"

#include "mpg/pl_mpeg_arm/pl_mpeg_handler.h"
#include "mpg/pl_mpeg.h"

// FIFO file wrapper structure
typedef struct {
    FIL *real_file;
    size_t file_size;
    size_t current_pos;
} fifo_file_wrapper_t;

// FIFO buffer implementation
// Feed PL_MPEG buffer from FIFO when needed
void feed_plm_buffer(plm_buffer_t *buffer, FIL *fil, uint32_t position) {
    // Reposition file if needed
    if (f_tell(fil) != position) {
        f_lseek(fil, position);
    }
    
    uint8_t read_buffer[4096]; // Leer 4KB
    UINT bytes_read;
    
    // Read data from file and feed PL_MPEG buffer
    if (f_read(fil, read_buffer, sizeof(read_buffer), &bytes_read) == FR_OK) {
        if (bytes_read > 0) {
            plm_buffer_write(buffer, read_buffer, bytes_read);
//            printf("[PLM BUFFER] Added %u bytes to PL_MPEG buffer\n", bytes_read);
        }
    }
}

// Custom buffer load callback for PL_MPEG
void my_plm_buffer_load_callback(plm_buffer_t *buffer, void *user) {
    fifo_file_wrapper_t *wrapper = (fifo_file_wrapper_t *)user;
    
    // Verificar si hemos llegado al final del archivo
    if (wrapper->current_pos >= wrapper->file_size) {
        static int eof_count = 0;
        eof_count++;
        
        // Solo mostrar el mensaje las primeras veces para evitar spam
        if (eof_count < 5) {
//            printf("[PLM BUFFER] End of file reached, no more data available (%d)\n", eof_count);
        } else if (eof_count % 100 == 0) {
            printf("[PLM BUFFER] Still at EOF, call count: %d\n", eof_count);
        }
        
        // Don't load more data - PL_MPEG should detect EOF
        return;
    }
    
    // Limitar la frecuencia de llamadas para evitar bucle infinito
    static uint32_t call_count = 0;
    call_count++;
    
    if (call_count % 10 != 0) { // Solo procesar cada 10ma llamada
        return;
    }
    
    // Feed buffer with more data
    feed_plm_buffer(buffer, wrapper->real_file, wrapper->current_pos);
    
    // Update current position
    wrapper->current_pos = f_tell(wrapper->real_file);
    
//    printf("[PLM BUFFER] Load callback: position %u/%u\n", wrapper->current_pos, wrapper->file_size);
}

#define FIFO_BUFFER_SIZE (1024 * 1024) // 1MB FIFO buffer

// FIFO file wrapper implementation

#include "mobotest.h"
#include <xparameters.h>
#include "main.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "lwip/sys.h"
#include "arch/sys_arch.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "rtg/fonts.h"
#include "config_file.h"
#include "xtime_l.h"
#include "xil_mmu.h"
#include "lwip/tftp_server.h"
#include "lwip/web_utils.h"
#include <stdbool.h>
#include "sii9022_init/sii9022_init.h"

#include "pt/pt.h"
#include "ARM_ztop/tabs.h"
#include "usb.h"

#include "ARM_ztop/textedit.h"
#include "ARM_ztop/tabs.h"
#include "coremark/coremark_port.h"
#include "video.h"
#include "rtg/zz_video_modes.h"
int init_xc3sprog(void);
int i2c_finish(void);
int main_xc3sprog(unsigned int cpufreq);
int measures_thread(struct pt *pt);
extern uint32_t ticks;
void b_refresh_action(void);
extern int selected_tab;
extern uint32_t fb_pitch;
#include "ARM_ztop/button.h"
extern Button *b_apply_all_timings;
void test_tab_timings(void);
void ns_repaint(void);
extern void *(memcpy_neon)(void * s1, const void * s2, u32 n);
void configure_clk(int clk, int verbose, int emu);

server_t server=GITHUB_SERVER;
unsigned int cpufreq_values[FREQ_NUM]={
   667,
   767,
   867,
   900,
   933,
   967,
   1000,
   1033,
   1067,
   1100,
   1133,
   1167,
   1200,
   1233,
   1267,
   1300,
};

extern ZZ_VIDEO_STATE vs;
extern int original_h;
#define NO_SCALE 0
int line=0;
int screen_width=800;

int timer_thread(struct pt *pt)
{
   PT_BEGIN(pt);
   while(1)
   {
      PT_WAIT_UNTIL(pt,ticks>=1000); // ~1 second
      ticks=0;
//      printf("tick\n");
      if(selected_tab==TAB_INFO)
         b_refresh_action();
   }
   PT_END(pt);
}
int ltc2990_init(void);
extern long int task_counter;

extern struct pt pt_measures;
static struct pt pt_timer;
sFONT *Font=&Font20;
extern SHARED *shared;
#define NOP __asm(" NOP")

void NOPX_WRITE(void)
{
   for(int i=shared->nops_write;i>0;i--)
   {
      NOP;
   }
}
extern void print_hdmi_ln(int xpos, char *message, int line_inc);
extern void print_hdmi(int xpos, char *message);
extern uint8_t read_keyboard(uint8_t *data, int enable_mouse);
extern uint32_t counts_per_second;
extern int core_main(int argc, char *argv[]); // CoreMark main function (renamed from main)

// EEMBC CoreMark standard benchmark for ARM performance measurement
void test_coremark(int freq_code)
{
   char message[100];
   
   sprintf(message,"=== EEMBC CoreMark BENCHMARK ===");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"CoreMark standard for Cortex-A9 @ %s MHz", arm_frequency_names[config.arm_frequency]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"Running CoreMark standard benchmark...");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   // Execute standard CoreMark - it will capture and return the score
   float return_value = 0.0f;
   int coremark_result = coremark_run(&return_value);
   
   if (return_value > 0.0f) {
      sprintf(message,"CoreMark score : %.2f", return_value);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      float freq_table[16]={667.0, 767.0, 867.0, 900.0, 933.0, 967.0, 1000.0, 1033.0, 1067.0, 1100.0, 1133.0, 1167.0, 1200.0, 1233.0, 1267.0, 1300.0};
      if(freq_code<0 || freq_code>=16)
         sprintf(message,"Error, freq_code %d unknown", freq_code);
      else
         sprintf(message,"CoreMark/MHz : %.4f", return_value/freq_table[freq_code]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   } else {
      sprintf(message,"CoreMark score not captured");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   
   sprintf(message,"CoreMark benchmark completed");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"Return code: %d", coremark_result);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}

// YCbCr to BGRA conversion for PL_MPEG frames
// Clamp function for PL_MPEG
inline static int plm_clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

inline static void plm_put_pixel(int y, int r, int g, int b, uint32_t *dest)
{
   uint32_t bgra = 0;
   bgra  = plm_clamp(y + b);
   bgra |= plm_clamp(y - g) << 8;
   bgra |= plm_clamp(y + r) << 16;
   *dest = bgra;
}

void plm_frame_to_bgra_custom(plm_frame_t *frame, uint8_t *dest) {
   int cols = frame->width >> 1;
   int rows = frame->height >> 1;
   int yw = frame->y.width;
   int cw = frame->cb.width;
   for (int row = 0; row < rows; row++) {
      int c_index = row * cw;
      int y_index = row * 2 * yw;
      for (int col = 0; col < cols; col++) {
         int cr = frame->cr.data[c_index] - 128;
         int cb = frame->cb.data[c_index] - 128;
         int r = (cr * 104597) >> 16;
         int g = (cb * 25674 + cr * 53278) >> 16;
         int b = (cb * 132201) >> 16;
         int y1 = ((frame->y.data[y_index + 0] - 16) * 76309) >> 16;
         int y2 = ((frame->y.data[y_index + 1] - 16) * 76309) >> 16;
         int y3 = ((frame->y.data[y_index + yw] - 16) * 76309) >> 16;
         int y4 = ((frame->y.data[y_index + yw + 1] - 16) * 76309) >> 16;
         uint32_t *dest_index = (uint32_t*)dest + y_index;
         plm_put_pixel(y1, r, g, b, dest_index + 0);
         plm_put_pixel(y2, r, g, b, dest_index + 1);
         plm_put_pixel(y3, r, g, b, dest_index + yw);
         plm_put_pixel(y4, r, g, b, dest_index + yw + 1);
         c_index += 1;
         y_index += 2;
      }
   }
}
// Variables for FPS calculation and display
static XTime last_time = 0;
static float current_fps = 30.0f;
static int show_fps = 0; // 0 = no FPS display, 1 = show FPS
static float video_fps = 30.0f; // Default FPS, will be updated with actual video FPS
static float fps_history[10] = {30.0f}; // History of FPS values for averaging
static int fps_history_index = 0; // Current index in FPS history
static int fps_history_count = 0; // Number of FPS values recorded
void set_default_timings(void);
// Test PL_MPEG video playback
void test_video_plmpeg_with_fps(int show_fps_param); // Forward declaration

void test_video_plmpeg(void)
{
   test_video_plmpeg_with_fps(0); // Default: no FPS display
}
void configure_video(int width, int height)
{
   original_h = 256;
   int w=width;
   int h=height;
   int mode=ZZVMODE_640x480;
   int scale=0;
   if(width<=320 && height<=200)
   {
      mode=ZZVMODE_640x400;
      w=640;
      h=400;
      scale=1;
      original_h = 200;
   }
   else if(width<=320 && height<=240)
   {
      mode=ZZVMODE_640x480;
      w=640;
      h=480;
      scale=1;
      original_h = 240;
   }
   else if(width<=320 && height<=256)
   {
      mode=ZZVMODE_640x512;
      w=640;
      h=512;
      scale=1;
      original_h = 256;
   }
   else if(width<=640 && height<=400)
   {
      mode=ZZVMODE_640x400;
      w=640;
      h=400;
   }
   else if(width<=640 && height<=480)
   {
      mode=ZZVMODE_640x480;
      w=640;
      h=480;
   }
   else if(width<=800 && height<=600)
   {
      mode=ZZVMODE_800x600;
      w=800;
      h=600;
   }
   else if(width<=1280 && height<=720)
   {
      mode=ZZVMODE_1280x720;
      w=1280;
      h=720;
   }
   else // 1920x1080
   {
      mode=ZZVMODE_1920x1080_50;
      w=1920;
      h=1080;
   }
   video_mode_init(mode, scale, MNTVA_COLOR_32BIT);
   set_fb((uint32_t*)(((uint32_t)vs.framebuffer) + 0), vs.vmode_hsize / vs.vmode_hdiv);
   memset(vs.framebuffer, 0, vs.framebuffer_size);

   switch(mode)
   {
      case ZZVMODE_1920x1080_50:
         Font=&Font20;
         break;
      default:
         Font=&Font12;
         break;
   }

   Font->TextColor=0x00FFFFFF; // white
   Font->BackColor=0x00000000; // black

   line=(h/Font->Height - 1);
   screen_width=w;
}

// Video decode callback
int fifo_file_eof(void *user) {
    fifo_file_wrapper_t *wrapper = (fifo_file_wrapper_t *)user;
    return (wrapper->current_pos >= wrapper->file_size);
}

void test_video_plmpeg_with_fps(int show_fps_param)
{   
   // Set FPS display flag
   show_fps = show_fps_param;
   
   char message[200];
   uint8_t keybd_data;
   
   sprintf(message,"=== PL_MPEG VIDEO TEST ===");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"Searching for .mpg files in 1:/video...");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   // Mount filesystem
   FATFS fatfs;
   TCHAR *Path = "1:/";
   FRESULT res = f_mount(&fatfs, Path, 1);
   if (res != FR_OK) {
      sprintf(message,"ERROR: Failed to mount filesystem (%d)", res);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return;
   }
   
   // Open video directory
   DIR dir;
   res = f_opendir(&dir, "1:/video");
   if (res != FR_OK) {
      sprintf(message,"ERROR: Failed to open 1:/video directory (%d)", res);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      f_mount(NULL, Path, 0);
      return;
   }
   
   // Find first .mpg file
   FILINFO fno;
   char mpg_filename[256+10] = "";
   while (1) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) break;
      
      if (!(fno.fattrib & AM_DIR)) {
         char *ext = strrchr(fno.fname, '.');
         if (ext && strcasecmp(ext, ".mpg") == 0) {
            sprintf(mpg_filename, "1:/video/%s", fno.fname);
            break;
         }
      }
   }
   
   f_closedir(&dir);
   
   if (strlen(mpg_filename) == 0) {
      sprintf(message,"ERROR: No .mpg files found in 1:/video");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      f_mount(NULL, Path, 0);
      return;
   }
   
   sprintf(message,"Found video file: %s", mpg_filename);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   sprintf(message,"File size: %llu bytes", fno.fsize);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);

   // Open the file
   FIL fil;
   res = f_open(&fil, mpg_filename, FA_READ);
   if (res != FR_OK) {
      sprintf(message,"ERROR: Failed to open file (%d)", res);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      f_mount(NULL, Path, 0);
      return;
   }
   
   sprintf(message,"Starting playback... Press SPACE to stop");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   // Crear wrapper FIFO para PL_MPEG usando buffer
   fifo_file_wrapper_t wrapper;
   wrapper.real_file = &fil;
   wrapper.file_size = f_size(&fil);
   wrapper.current_pos = 0;
   
   // Create PL_MPEG buffer and feed it initially
   plm_buffer_t *buffer = plm_buffer_create_with_capacity(FIFO_BUFFER_SIZE);
   
   // Configure callback to load more data when needed
   plm_buffer_set_load_callback(buffer, my_plm_buffer_load_callback, &wrapper);
   
   // Feed initial data to buffer
   uint8_t initial_data[131072]; // 128KB initial read
   UINT bytes_read;
   if (f_read(&fil, initial_data, sizeof(initial_data), &bytes_read) == FR_OK) {
      plm_buffer_write(buffer, initial_data, bytes_read);
      wrapper.current_pos = bytes_read;
      printf("[PLM BUFFER] Initial buffer loaded: %u bytes\n", bytes_read);
   } else {
      sprintf(message,"ERROR: Failed to read initial data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      f_close(&fil);
      f_mount(NULL, Path, 0);
      return;
   }
   
   // Create PL_MPEG instance with buffer
   plm_t *plm = plm_create_with_buffer(buffer, TRUE);
   
   if (!plm) {
      sprintf(message,"ERROR: Failed to create PL_MPEG instance");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      f_close(&fil);
      f_mount(NULL, Path, 0);
      return;
   }
   
   // Verify that PL_MPEG was initialized correctly
   if (plm_get_width(plm) == 0 || plm_get_height(plm) == 0) {
      sprintf(message,"ERROR: PL_MPEG failed to initialize video stream");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      plm_destroy(plm);
      f_close(&fil);
      f_mount(NULL, Path, 0);
      return;
   }
   
   // Disable audio since we're only interested in video
   plm_set_audio_enabled(plm, FALSE);
   
   // Get video information without displaying frames
   int video_width = plm_get_width(plm);
   int video_height = plm_get_height(plm);
   video_fps = plm_get_framerate(plm);
   
   printf("[MOBOTEST] Video info: %dx%d @ %.2f fps\n", video_width, video_height, video_fps);
   
   sprintf(message,"Video: %dx%d @ %.2f fps, Duration: %.2f sec", 
           video_width, video_height, video_fps, plm_get_duration(plm));
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   
   // Configure video mode BGRA
   configure_video(video_width, video_height);

   // Rewind to beginning for clean playback (this will skip the initial glitchy frames)
   plm_rewind(plm);

   // Playback loop with proper frame rate synchronization
   XTime start_time;
   XTime_GetTime(&start_time);
   uint32_t displayed_frame_count = 0;
   int wants_to_quit = 0;
   
   printf("[MOBOTEST] Starting playback loop\n");
   
   // Reset FPS calculation variables
   XTime_GetTime(&last_time);
   uint32_t frames_in_interval = 0; // Frames displayed in current 500ms interval
   fps_history_index = 0;
   fps_history_count = 0;
   memset(fps_history, 0, sizeof(fps_history));
   
   while (!wants_to_quit && !plm_has_ended(plm)) {
      // Try to decode a video frame
      plm_frame_t *frame = plm_decode_video(plm);
      
      if (frame) {
         // Frame available, display it
         plm_frame_to_bgra_custom(frame, (uint8_t*)vs.framebuffer);
         
         displayed_frame_count++;
         frames_in_interval++;
         
         // Update FPS calculation
         XTime current_time;
         XTime_GetTime(&current_time);
         if (current_time - last_time >= (counts_per_second / 2)) { // 500ms en ticks de XTime
            float instant_fps = (float)frames_in_interval * (float)counts_per_second / (float)(current_time - last_time);
            
            // Add to FPS history
            fps_history[fps_history_index] = instant_fps;
            fps_history_index = (fps_history_index + 1) % 10;
            if (fps_history_count < 10) {
               fps_history_count++;
            }
            
            // Calculate average FPS from history
            float sum = 0.0f;
            for (int i = 0; i < fps_history_count; i++) {
               sum += fps_history[i];
            }
            current_fps = sum / (float)fps_history_count;
            
            // Reset for next interval
            frames_in_interval = 0;
            last_time = current_time;
         }
         // Display FPS if enabled
         if (show_fps) {
            char fps_message[64];
            sprintf(fps_message, "FPS: %.2f / %.2f", current_fps, video_fps);
            
            int x = vs.vmode_hsize - strlen(fps_message) * Font->Width - 10;
            int y = 480-50;//10;
            
            uint32_t old_text_color = Font->TextColor;
            uint32_t old_back_color = Font->BackColor;
            
            Font->TextColor = 0x00FFFFFF;
            Font->BackColor = 0x00000000;
            
            displayStringAt(Font, x, y, (uint8_t*)fps_message, LEFT_MODE);
            
            Font->TextColor = old_text_color;
            Font->BackColor = old_back_color;
         }
         
         // Synchronize frame rate using real elapsed time
         XTime frame_end_time;
         XTime_GetTime(&frame_end_time);
         
         // Calculate target time for next frame
         double target_frame_time = (double)displayed_frame_count / video_fps;
         double elapsed_time = (double)(frame_end_time - start_time) / (double)counts_per_second;
         
         // If we're ahead of schedule, wait until the target time
         if (elapsed_time < target_frame_time) {
            double wait_time_us = (target_frame_time - elapsed_time) * 1000000.0;
            if (wait_time_us > 0) {
               usleep((useconds_t)wait_time_us);
            }
         }
         
         // If we're behind schedule, continue without waiting (catch up)
      } else {
         // No frame available
         static int no_frame_count = 0;
         no_frame_count++;
         
         // If no frames for a long time, check if video really ended
         if (no_frame_count > 200) { // 200ms without frames, check for end of video
//            printf("[MOBOTEST] No frames for %d iterations, checking if video ended\n", no_frame_count);
            
            // Force to state verify
            if (plm_has_ended(plm)) {
               printf("[MOBOTEST] Video has ended, exiting playback loop\n");
               break;
            }
            
            // If we've reached EOF and no frames, exit
            if (wrapper.current_pos >= wrapper.file_size) {
               printf("[MOBOTEST] EOF reached and no frames, exiting playback loop\n");
               break;
            }
            
            // Reset counter to avoid spam
            no_frame_count = 0;
         }
      }
      
      // Check for user input
      if (read_keyboard(&keybd_data, 0) && (keybd_data == ' ' || keybd_data == 27)) {
         sprintf(message,"DEBUG: User quit requested, key=%d", keybd_data);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         wants_to_quit = 1;
      }
   }
   
   plm_destroy(plm);
   f_close(&fil);
   f_mount(NULL, Path, 0);
   
   sprintf(message,"Video playback completed - %ld frames processed", displayed_frame_count);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}

void NOPX_READ(void)
{
   for(int i=shared->nops_read;i>0;i--)
   {
      NOP;
   }
}
#include "ff.h"
#include "video.h"
#include "rtg/zz_video_modes.h"

#define check_bus_error(A,B)

#define REG_BASE_ADDRESS XPAR_Z3660_0_BASEADDR

#define write_reg64(Offset,Data) (*(volatile uint64_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)
#define write_reg(Offset,Data)   (*(volatile uint32_t *)(REG_BASE_ADDRESS+(Offset)))=(Data)

#define read_mem32(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x14000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem16(Offset) (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x18000000+((Offset&0x00FFFFFF)<<2)))=(0)
#define read_mem8(Offset)  (*(volatile uint32_t *)(REG_BASE_ADDRESS+0x1C000000+((Offset&0x00FFFFFF)<<2)))=(0)

uint32_t last_bank=-1;

void arm_write_amiga_long(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem32(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}
void arm_write_long(uint32_t address, uint32_t data, int *timeout)
{
   if(address< 0x08000000)
      arm_write_amiga_long(address, data, timeout);
   else if(address<0x10000000)
      *(uint32_t*)address=data;
}
void arm_write_amiga_word(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem16(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}
void arm_write_amiga_byte(uint32_t address, uint32_t data, int *timeout)
{
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   write_mem8(address,data);
   NOP;
   do {
      NOPX_WRITE();
      (*timeout)--;
   }
   while((read_reg(REG5)&0x3)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
}

uint32_t arm_read_amiga_long(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem32(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   uint32_t data_read=read_reg(REG7); // read data
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   return(data_read);
}
uint32_t arm_read_amiga_word(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem16(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(REG7); // read data
   return(data_read);
}
uint32_t arm_read_long(uint32_t address, int *timeout)
{
   if(address< 0x08000000)
      return arm_read_amiga_long(address, timeout);
   else if(address<0x10000000)
      return *(uint32_t*)address;
   else return 0xFFFFFFFF;
}
#define ps_read_byte(A,T) ((arm_read_amiga_byte(A,T)>>((3-(A&3))*8))&0xFF)
#define ps_write_byte(A,D,T) arm_write_amiga_byte(A,(D&0xFF)<<((3-(A&3))*8),T)

uint32_t arm_read_amiga_byte(uint32_t address, int *timeout)
{
//   while(read_reg(REG5)&0x80000000);        // previous write ack
   *timeout=100000;
   while(READ_NBG_ARM()!=0);
   uint32_t bank=(address>>24)&0xFF;
//   if(bank!=last_bank)
   {
      write_reg(REG6,bank);
      last_bank=bank;
   }
   NOP;
   read_mem8(address);
   NOP;
   do {
      NOPX_READ();
      (*timeout)--;
   }
   while(read_reg(REG5)==0 && *timeout>0);        // read ack
   check_bus_error(read_reg(REG5),address);
   write_reg(REG4,0x0); // ARM Bus Hi-Z
   uint32_t data_read=read_reg(REG7); // read data
   return(data_read);
}
char message[300]={0};
void handle_cache_flush(uint32_t address,uint32_t size);
extern ZZ_VIDEO_STATE vs;
void copy_rect_16_mobotest(int line)
{
#define LINE_MAX_FONT ((int)((vs.vmode_vsize/Font->Height) - 1))
   int delta=(line-LINE_MAX_FONT)*Font->Height;
   uint32_t h = vs.vmode_vsize - delta;
   uint32_t w = vs.vmode_hsize;
   uint16_t *dp=(uint16_t *)(FRAMEBUFFER_ADDRESS);
   uint16_t *sp=(uint16_t *)(FRAMEBUFFER_ADDRESS+delta*w*2);
   for (uint16_t y_line = 0; y_line < h; y_line++,dp += w,sp += w)
      memcpy_neon(dp, sp, w * 2);
   for (uint16_t y_line = h; y_line < vs.vmode_vsize; y_line++,dp += w)
      memset(dp, 0, w * 2);
}
void copy_rect_32_mobotest(int line)
{
#define LINE_MAX_FONT ((int)((vs.vmode_vsize/Font->Height) - 1))
   int delta=(line-LINE_MAX_FONT)*Font->Height;
   uint32_t h = vs.vmode_vsize - delta;
   uint32_t w = vs.vmode_hsize;
   uint32_t *dp=(uint32_t *)(FRAMEBUFFER_ADDRESS);
   uint32_t *sp=(uint32_t *)(FRAMEBUFFER_ADDRESS+delta*w*4);
   for (uint32_t y_line = 0; y_line < h; y_line++,dp += w,sp += w)
      memcpy_neon(dp, sp, w * 4);
   for (uint32_t y_line = h; y_line < vs.vmode_vsize; y_line++,dp += w)
      memset(dp, 0, w * 4);
}
inline void copy_rect_mobotest(int line)
{
   if(vs.video_mode==MNTVA_COLOR_16BIT565)
      copy_rect_16_mobotest(line);
   else
      copy_rect_32_mobotest(line);
}

void print_hdmi(int xpos, char *message)
{
   displayStringAt(Font,xpos*Font->Width,line*Font->Height,(uint8_t*)message,LEFT_MODE);
//   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
//   while(video_formatter_read(0)==1) //wait vblank
//   {}
//   while(video_formatter_read(0)==0)
//   {}
}
void print_hdmi_ln(int xpos, char *message, int line_inc)
{
   print_hdmi(xpos,message);
   line+=line_inc;
   if(line>LINE_MAX_FONT)
   {
      copy_rect_mobotest(line);
      line=LINE_MAX_FONT;
   }
//   handle_cache_flush(((uint32_t)vs.framebuffer) + vs.framebuffer_pan_offset,vs.framebuffer_size);
//   while(video_formatter_read(0)==1) //wait vblank
//   {}
//   while(video_formatter_read(0)==0)
//   {}
}
void print_hdmi_ln_centered(char *message, int line_inc)
{
   int x=(screen_width/Font->Width-strlen(message))/2;
   print_hdmi_ln(x, message, line_inc);
}
char kb_tbl[256]=
   "`1234567890-=\\\x00" "0"
   "qwertyuiop[]" "\x00" "123"
   "asdfghjkl;'\x00\x00" "456"
   "\x00zxcvbnm,./\x00.789"
   " \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
   "\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
   "\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
   "\x80\x81\x82\x83\x84\x85\x86\x87"
   "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
   "~!@#$%^&*()_+|\x00" "0"
   "QWERTYUIOP{}\x00" "123"
   "ASDFGHJKL:\x22\x00\x00" "456"
   "\x00ZXCVBNM<>?\x00.789"
   " \x08\x09\x0d\x0d\x1b\x7f\x00\x00\x00-\x00\x1f\x1e\x1d\x1c" // 28-31 Cursor Keys
   "\xf9\xf8\xf7\xf6\xf5\xf4\xf3\xf2\xf1\xf0()/*+\xfa"          // F-Keys, Help
   "\xfe\xfe\xfb\xff\xfd\xfd\xfc\xfc"                           // Shift, Ctrl, Alt, Amiga
   "\x80\x81\x82\x83\x84\x85\x86\x87"
   "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97"
   ;
void keyboard_handshake(void)
{
   int timeout=0;
   ps_write_byte(0xbfee01,0x50,&timeout);
   usleep(100);
   ps_write_byte(0xbfee01,0x10,&timeout);
   usleep(1000);
}
#define L_SHIFT   0x60
#define R_SHIFT   0x61
#define CAPS_LOCK 0x62
#define L_AMIGA   0x66
#define R_AMIGA   0x67
uint8_t shift_status=0;
uint8_t amiga_status=0;
int16_t mousex=0,mousey=0;
uint8_t counterx=0,countery=0;
uint8_t mouse_pressed=0,mouse_pressed_old=-1;
void do_clip_hw_sprite(int16_t offset_x, int16_t offset_y);
void do_update_hw_sprite_pos(int16_t x, int16_t y);
void update_mouse_position(void)
{
   if(mousex<0) mousex=0;
   if(mousex>(int16_t)(vs.vmode_hsize-1)) mousex=vs.vmode_hsize-1;
   if(mousey<0) mousey=0;
   if(mousey>(int16_t)(vs.vmode_vsize-1)) mousey=vs.vmode_vsize-1;
   vs.sprite_x_base=mousex;
   vs.sprite_y_base=mousey;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);

   update_hw_sprite_pos();
}
void calc_mouseposition(uint16_t joy0dat)
{
   uint8_t counter_new;
   counter_new=(joy0dat>>8)&0xFF;
   int16_t diff=counter_new-countery;
   if(diff > 127)
      mousey += diff - 256;
   else if(diff < -128)
      mousey += diff + 256;
   else
      mousey += diff;
   countery=counter_new;

   counter_new=(joy0dat   )&0xFF;
   diff=counter_new-counterx;
   if(diff > 127)
      mousex += diff - 256;
   else if(diff < -128)
      mousex += diff + 256;
   else
      mousex += diff;
   counterx=counter_new;

   update_mouse_position();
//   char message2[20];
//   sprintf(message2,"%04d,%04d",mousex,mousey);
//   displayStringAt(Font,0,0,(uint8_t*)message2,LEFT_MODE);
}
bool enable_amiga_keyboard_read=true;
uint8_t read_keyboard(uint8_t *data, int enable_mouse)
{
   if(XUartPs_IsReceiveData(STDIN_BASEADDRESS))
   {
      *data=XUartPs_ReadReg(STDIN_BASEADDRESS, XUARTPS_FIFO_OFFSET);
      return(1);
   }
   if(enable_amiga_keyboard_read)
   {
      if(enable_mouse)
      {
         int timeout;
         uint16_t joyy0dat=arm_read_amiga_word(0xdff00a,&timeout)&0xFFFF;
         calc_mouseposition(joyy0dat);
         uint8_t mousedat=ps_read_byte(0xbfe001,&timeout);
         mouse_pressed=mousedat&(1<<6)?0:1;
      }
      int timeout;
      uint8_t kb_int=ps_read_byte(0xbfed01,&timeout);
      if(timeout==0)
      {
         static int retry=0;
         if(retry==10)
         {
            enable_amiga_keyboard_read=false;
            sprintf(message,"Can't access to the Amiga keyboard -> Serial Debug used as keyboard");
            printf("%s\n",message);
            Font->TextColor=0x00FF0000; // red
            int line_old=line;
            line = 1; // show this at top
            print_hdmi_ln_centered(message,0);
            line=line_old;
            Font->TextColor=0x00FFFFFF; // white
         }
         else
         {
            retry++;
         }
      }
      if((kb_int&(1<<3))==0)
         return(0);
      ps_write_byte(0xbfed01,0x00,&timeout);

      uint8_t kb_data=ps_read_byte(0xbfec01,&timeout);
      uint8_t data1=(~(kb_data>>1))&0x7F;
      keyboard_handshake();
//      sprintf(message,"kb_data %02X  data1  %02X      ",kb_data,data1);
//      line++;
//      print_hdmi(0,message);
//      line--;
      uint8_t down=kb_data&0x01;
      if(data1==L_SHIFT || data1==R_SHIFT || data1==CAPS_LOCK)
      {
         shift_status=down?1:0;
//         sprintf(message,"changed shift_status %d      ",shift_status);
//         line+=2;
//         print_hdmi(0,message);
//         line-=2;
         return(0);
      }
      if(data1==L_AMIGA || data1==R_AMIGA)
      {
         amiga_status=down?1:0;
         return(0);
      }
      if(shift_status)
         data1=data1+128;
      *data=kb_tbl[data1];
      return(down);
   }
   return(0);
}
extern char version_string_export[];
extern char version_scsirom_string_export[];
extern char version_jed_string_export[];
uint32_t read_rtg_register(uint32_t zaddr);
extern DOWNLOAD_DATA download_data;

void hard_reboot(void);
void reboot(void)
{
   sprintf(message,"Reboot in 1 second...");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   usleep(1000000);
   hard_reboot();
}
int v_major=1;
int v_minor=3;
int beta=0;
int alfa=0;
int mio14=0;
extern uint32_t counts_per_second;
void test_nops(void)
{
   XTime total_time_start = 0;
   XTime total_time_stop;
   XTime debug_time_start = 0;
   XTime debug_time_stop;
   float read_mbs[10];
   float write_mbs[10];
#define isr_usb NULL
   fpga_interrupt_connect(isr_video, isr_audio_tx, isr_usb, INT_IPL_ON_THIS_CORE);
#define TOTAL_MEM_TESTED 0x40000
#define TESTED_TIMES 8
   sprintf(message,"NOPS ticks (READ)");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   XTime_GetTime(&total_time_start);
   for(int r=0;r<10;r++)
   {
      shared->nops_read=r;
      XTime_GetTime(&debug_time_start);
      int timeout;
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_read_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i,&timeout);
      XTime_GetTime(&debug_time_stop);
      read_mbs[r]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(counts_per_second))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",r,read_mbs[r]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   float max=0;
   int opt_r=0;
   for(int r=0;r<10;r++)
   {
      if(read_mbs[r]>max)
      {
         max=read_mbs[r];
         opt_r=r;
      }
   }
   sprintf(message,"Optimal NOPS for READ: %d (%6.4f MB/s)",opt_r,read_mbs[opt_r]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   shared->nops_read=opt_r;

   sprintf(message,"NOPS ticks (WRITE)");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   for(int w=0;w<10;w++)
   {
      shared->nops_write=w;
      XTime_GetTime(&debug_time_start);
      int timeout;
      for(int j=0;j<TESTED_TIMES;j++)
         for(uint32_t i=0;i<TOTAL_MEM_TESTED;i+=4)
            arm_write_amiga_long((0x200000-TOTAL_MEM_TESTED-0x10000)+i,i,&timeout);
      XTime_GetTime(&debug_time_stop);
      write_mbs[w]=(TESTED_TIMES*((float)TOTAL_MEM_TESTED)/1024./1024.*(counts_per_second))/((1.0 * (debug_time_stop-debug_time_start)));
      sprintf(message," %2d %6.4f MB/s",w,write_mbs[w]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
   max=0;
   int opt_w=0;
   for(int w=0;w<10;w++)
   {
      if(write_mbs[w]>max)
      {
         max=write_mbs[w];
         opt_w=w;
      }
   }
   XTime_GetTime(&total_time_stop);
   sprintf(message,"Optimal NOPS for WRITE: %d (%6.4f MB/s)",opt_w,write_mbs[opt_w]);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   shared->nops_read=opt_w;
   float total_time=(1.0 * (total_time_stop-total_time_start)) / (counts_per_second);
   sprintf(message,"Total time: %4.1f s (%3.1f MB)",total_time,(20.*TOTAL_MEM_TESTED*TESTED_TIMES)/1024./1024.);
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
}
uint8_t keybd_data=0;

void show_options(void)
{
#define MSG_LINE(X) sprintf(message,X);print_hdmi_ln(0,message,1);printf("%s\n",message)
   MSG_LINE("Update Options:");
   MSG_LINE("'H' - Show this help menu");
   MSG_LINE("'I' - Connect to the Internet and check for updates");
   MSG_LINE("'S' - Change the update server");
   MSG_LINE("'O' - Download the latest z3660_scsi.rom (overwrites z3660scsi.rom on exFAT)");
   MSG_LINE("'T' - Test download of latest BOOT.bin (does NOT write to storage)");
   MSG_LINE("'U' - Download and overwrite Z3660.bin with latest BOOT.bin");
   MSG_LINE("'F' - Download and overwrite FAILSAFE.bin with latest BOOT.bin");
   MSG_LINE("'D' - Download and flash the latest CPLD firmware");
   MSG_LINE("      (if not connected to the Internet, flashes the previously downloaded file)");
   MSG_LINE("'G' - Apply default timings");
   MSG_LINE("'P' - Start a TFTP server connected to your network");
   MSG_LINE("Test Options:");
   MSG_LINE("'X'  - Test read/write and determine optimal NOPS values for EMU (only for developers)");
   MSG_LINE("'Q'  - CoreMark performance test for ARM CPU");
//   MSG_LINE("'V'  - Test PL_MPEG video playback");
//   MSG_LINE("'W'  - Test PL_MPEG video playback with FPS display");
   MSG_LINE("File Management Options:");
   MSG_LINE("'B' - Copy BOOT.bin to FAILSAFE.bin");
   MSG_LINE("'N' - Copy BOOT.bin to Z3660.bin");
   MSG_LINE("'M' - Copy Z3660.bin to BOOT.bin");
   MSG_LINE("'J' - Copy Z3660.bin to FAILSAFE.bin");
   MSG_LINE("'K' - Copy FAILSAFE.bin to BOOT.bin");
   MSG_LINE("'L' - Copy FAILSAFE.bin to Z3660.bin");
   MSG_LINE("'E' - SD card file manager (DIR/COPY/REN/DEL/MKDIR/CRC/FREE)");
   MSG_LINE(" ");
   MSG_LINE("'R' to reboot");
   MSG_LINE(" ");
   MSG_LINE("Note: BOOT.bin initializes the system and then attempts to load Z3660.bin to continue booting.");
   MSG_LINE("      If Z3660.bin is invalid or missing, it will attempt to boot from FAILSAFE.bin.");
   MSG_LINE("      If both fail, BOOT.bin will continue booting on its own.");
   MSG_LINE(" ");
}
void mobo_change_jumpers(int reg6)
{
   for(int i=0x180;i<0x200;i++) // RTG force no cache
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);

   vs.sprite_x_base = 3000;
   vs.sprite_y_base = 3000;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
   switch(config.bootscreen_resolution)
   {
   case RES_1920x1080:
      Font=&Font20;
      break;
   case RES_1280x720:
      Font=&Font12;
      break;
   case RES_800x600:
   default:
      Font=&Font12;
      break;
   }

   Font->TextColor=0x00FFFFFF; // white
   Font->BackColor=0x00000000; // black

   int w=1920;
   int h=1080;
//   if(config.enable_test!=_YES)
   {
      int offset=8;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
         w=1920;
         h=1080;
         offset=11;
         break;
      case RES_1280x720:
         w=1280;
         h=720;
         offset=12;
         break;
      case RES_800x600:
      default:
         w=800;
         h=600;
         offset=12+5;
         break;
      }
      line=(h/Font->Height - 1)- offset; // 43 @ 1080p
   }
   screen_width=w;
   if((reg6&(FPGA_CLK90_DETECTED|FPGA_CPUCLK_DETECTED))==FPGA_CPUCLK_DETECTED)
      sprintf(message,"FPGA detected CPUCLK!!! For the love of your Amiga, change it now to EXT position !!!");
   else if((reg6&(FPGA_CLK90_DETECTED|FPGA_CPUCLK_DETECTED))==FPGA_CLK90_DETECTED)
      sprintf(message,"FPGA detected CLK90!!! For the love of your Amiga, change it now to EXT position !!!");
   else
      sprintf(message,"FPGA detected CPUCLK and CLK90!!! For the love of your Amiga, change them now to EXT position !!!");

   int x=(w/Font->Width-strlen(message))/2;
   Font->TextColor=0x00FFFFFF;
   Font->BackColor=0x00303030;
   print_hdmi_ln(x,message,2);
   printf("%s\n",message);
}
void mobotest(int sw1_is_down)
{
   for(int i=0x180;i<0x200;i++) // RTG force no cache
      Xil_SetTlbAttributes(i*0x100000UL,NORM_NONCACHE);

   vs.sprite_x_base = 3000;
   vs.sprite_y_base = 3000;
   do_update_hw_sprite_pos(vs.sprite_x_base, vs.sprite_y_base);
   video_formatter_write((vs.sprite_y_adj << 16) | vs.sprite_x_adj, MNTVF_OP_SPRITE_XY);
   switch(config.bootscreen_resolution)
   {
   case RES_1920x1080:
      Font=&Font20;
      break;
   case RES_1280x720:
      Font=&Font12;
      break;
   case RES_800x600:
   default:
      Font=&Font12;
      break;
   }

   Font->TextColor=0x00FFFFFF; // white
   Font->BackColor=0x00000000; // black

   NBR_ARM(0);        // bus request
   usleep(100000);
   CPLD_RESET_ARM(1); // CPLD RUN
   int timeout1=0;
   printf("READ_NBG_ARM()\n");
   while(READ_NBG_ARM()!=0) // make sure that we have the bus control
   {
      usleep(1000);
      timeout1++;
      if(timeout1==1000)
         break;
   }

#ifdef CPLD_PROGRAMMING
   if(timeout1==1000
      || XGpioPs_ReadPin(&GpioPs, n040RSTI)==0
      || sw1_is_down)
   {
      if(sw1_is_down==1)
         sprintf(message,"SW1 pressed -> CPLD programming...");
      else if(sw1_is_down==2)
         sprintf(message,"CPLD programming (reset pin held low)");
      else
         sprintf(message,"Timeout waiting NBG_ARM (CPLD erased?)");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sw1_is_down=0;
      CPLD_RESET_ARM(0);
      Xil_ExceptionDisable();
      int err=init_xc3sprog();
      if(err==0)
      {
         main_xc3sprog(config.arm_frequency);
      }
      else
      {
         //We will not have access to the Amiga keyboard, so enable serial debug as keyboard
         enable_amiga_keyboard_read=false;
         printf("We will not have access to the Amiga keyboard, so enable serial debug as keyboard\n");
      }
      i2c_finish();
      Xil_ExceptionEnable();
      CPLD_RESET_ARM(1);
   }
#endif
   usleep(1000);

   int failed=0;
   int ovl_failed=0;
   int timeout;
   if(config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Simple Amiga bus test...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      sprintf(message,"[TEST] 0xbfe201 %02lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %02lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      sprintf(message,"[TEST] Setting new values");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
//#define FORCE_OVL_FAIL
#ifndef FORCE_OVL_FAIL
      ps_write_byte(0xbfe201,0x01,&timeout); // OVL as OUTPUT
#endif
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 1 to 0xbfe201");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      ps_write_byte(0xbfe001,0x00,&timeout); // OVL LOW to see the CHIP RAM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe001");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      sprintf(message,"[TEST] 0xbfe201 %0lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %0lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      arm_write_amiga_long(0x00000000,0xAA55AA55,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0xAA55AA55 to 0x00000000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      arm_write_amiga_long(0x00100000,0x55AA55AA,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0x55AA55AA to 0x00100000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data1=arm_read_amiga_long(0x00000000,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00000000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      uint32_t data2=arm_read_amiga_long(0x00100000,&timeout);
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when reading from 0x00100000");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      if(data1==0xAA55AA55 && data2==0x55AA55AA)
      {
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM ok");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
      }
      else
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Test basic Amiga CHIP RAM FAILED!!!");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
         if(data1==0xAA55AA55)
         {
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0xAA55AA55  OK!!!",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else if(data1==0x11144EF9)
         {
            ovl_failed=1;
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX == 0x11144EF9 This is the kickstart!!!",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00000000) -> 0x%08lX instead of 0xAA55AA55",data1);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
         if(data2==0x55AA55AA)
         {
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX == 0x55AA55AA  OK!!!",data2);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else
         {
            Font->TextColor=0x00FF0000; // red
            sprintf(message,"[TEST] (0x00100000) -> 0x%08lX instead of 0x55AA55AA",data2);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            Font->TextColor=0x00FFFFFF; // white
         }
      }
   }
   if(failed || config.enable_test==_YES)
   {
      if(failed)
         sprintf(message,"[TEST] Something failed -> exhaustive test");
      else
         sprintf(message,"[TEST] Test Enabled in z3660cfg.txt file -> exhaustive test");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);

      usleep(1000000); // wait time to allow monitor to show the screen
      failed=0;
#define NUM_MEM_REGIONS 8
      struct mem_region {
         uint8_t *test_mem1;
         uint8_t *test_mem2;
         uint32_t start;
         uint32_t len;
         char name[50];
         int failed;
      };
      struct mem_region mem_regions[NUM_MEM_REGIONS];
      int max_str_length=0;
      for(int i=0;i<NUM_MEM_REGIONS;i++)
      {
         mem_regions[i].start=config.test_range[i].start;
         mem_regions[i].len=config.test_range[i].length;
         mem_regions[i].failed=0;
         int len=strlen(config.test_range[i].name);
         if(len>max_str_length) max_str_length=len;
      }
      max_str_length+=17; // "Test Region x () "
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         uint32_t length=mem_regions[j].len;
         if(length>2*1024*1024) length=2*1024*1024; // 2MB max
         mem_regions[j].test_mem1=malloc(length);
         mem_regions[j].test_mem2=malloc(length);
         for(uint32_t i=0;i<length;i++)
            mem_regions[j].test_mem1[i]=(uint8_t)(rand()&0xFF);
      }
      sprintf(message,"[TEST] Writing random data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      int x;
      int steps=4*1;
      int disp=5;
      int xx_init=75;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
      case RES_1280x720:
         disp=5;
         xx_init=75;
         break;
      case RES_800x600:
      default:
         disp=4;
         xx_init=75-10;
         break;
      }

      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            arm_write_long(i,((uint32_t *)mem_regions[j].test_mem1)[k>>2],&timeout);
            k+=steps;
            if(k>=2*1024*1024) k=0;
            if(timeout==0)
            {
               if(mem_regions[j].failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Timeout when writing 0x%08lX to 0x%08lX",((uint32_t *)mem_regions[j].test_mem1)[i>>2],i);
               printf("%s\n",message);
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Timeout when writing 0x%08lX to 0x%08lX      ",((uint32_t *)mem_regions[j].test_mem1)[i>>2],i);
               int line_old=line;
               line=0;
               print_hdmi(60,message);
               line=line_old;
               Font->TextColor=0x00FFFFFF; // white
               mem_regions[j].failed=1;
            }
            if(mem_regions[j].failed==0 && (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               printf("%s",message);
               line--;
               print_hdmi(x++,message);
               line++;
            }
         }
         if(mem_regions[j].failed==0)
         {
            sprintf(message," done");
            printf("%s\n",message);
            line --;
            print_hdmi(x,message);
            line++;
         }
      }
      sprintf(message,"[TEST] Reading data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         mem_regions[j].failed=0;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            ((uint32_t *)mem_regions[j].test_mem2)[k>>2]=arm_read_long(i,&timeout);
            k+=steps;
            if(k>=2*1024*1024) k=0;
            if(timeout==0)
            {
               if(mem_regions[j].failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Timeout when reading from 0x%08lX",i);
               printf("%s\n",message);
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Timeout when reading from 0x%08lX               ",i);
               int line_old=line;
               line=0;
               print_hdmi(60,message);
               line=line_old;
               Font->TextColor=0x00FFFFFF; // white
               mem_regions[j].failed=1;
            }
            if(mem_regions[j].failed==0 && (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               printf("%s",message);
               line--;
               print_hdmi(x++,message);
               line++;
            }
         }
         if(mem_regions[j].failed==0)
         {
            sprintf(message," done");
            printf("%s\n",message);
            line--;
            print_hdmi(x++,message);
            line++;
         }
      }
      sprintf(message,"Press '1' -> 1k jump, '2' -> 2k jump, '3' -> 8k jump, ...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message," ..., '9' -> 256k jump, '0' -> 512 k jump, 'P' pause/resume, ESC exit");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] Comparing data");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         if(mem_regions[j].len>=128*1024*1024) steps=4*256;
         else if(mem_regions[j].len>=64*1024*1024) steps=4*128;
         else if(mem_regions[j].len>=32*1024*1024) steps=4*64;
         else if(mem_regions[j].len>=16*1024*1024) steps=4*32;
         else if(mem_regions[j].len>=8*1024*1024) steps=4*16;
         else if(mem_regions[j].len>=4*1024*1024) steps=4*8;
         mem_regions[j].failed=0;
         uint32_t fail_mask=0;
         x=max_str_length;
         sprintf(message,"Test Region %d (%s)",j,config.test_range[j].name);
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         for(uint32_t i=mem_regions[j].start,k=0;i<mem_regions[j].start+mem_regions[j].len;i+=steps)
         {
            if(read_keyboard(&keybd_data,0))
            {
               if(keybd_data>='0' && keybd_data <='9')
               {
                  uint32_t jump_table[10]={512,1,2,4,8,16,32,64,128,256};
                  uint32_t jump=jump_table[keybd_data-'0']*1024;
                  i=(i+jump)&(~(jump-1));
                  if(i>=mem_regions[j].start+mem_regions[j].len)
                     break;
               }
               if(keybd_data=='\x1b') // ESC
               {
                  goto end_test;
               }
               if(keybd_data=='p' || keybd_data=='P')
               {
//                  while(!read_keyboard(&keybd_data,0));
                  while(1) // wait until resume with 'p'
                  {
                     if(read_keyboard(&keybd_data,0))
                     {
                        if(keybd_data=='p' || keybd_data=='P')
                           break;
                     }
                  }
               }
            }
            uint32_t data1=((uint32_t *)mem_regions[j].test_mem1)[k>>2];
            uint32_t data2=((uint32_t *)mem_regions[j].test_mem2)[k>>2];
            k+=steps;
            if(k>=2*1024*1024) k=0;

            if(data1!=data2)
//            if((data1&0xFFFFFFF5)!=(data2&0xFFFFFFF5))
            {
               if(failed==0)
                  printf("\n");
               sprintf(message,"[TEST] Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
               printf("%s",message);
               mem_regions[j].failed=1;
               Font->TextColor=0x00FF0000; // red
               sprintf(message,"Address 0x%08lX failed: W/R %08lX/%08lX",i,data1,data2);
            }
            else
            {
               Font->TextColor=0x0000FF00; // green
               sprintf(message,"Address 0x%08lX OK:     W/R %08lX/%08lX",i,data1,data2);
            }
            {
               int line_old=line;
               line=0+3*j;
               print_hdmi(xx_init-1,message);
               Font->TextColor=0x00FFFFFF; // white
               sprintf(message,"31     24   23     16   15      8   7       0");
               line=1+3*j;
               print_hdmi(xx_init,message);
               int xx=xx_init;
               line=2+3*j;
               for(int l=31;l>=0;l--)
               {
                  int bit=1L<<l;
                  int data=0;
                  if(fail_mask&bit)
                  {
                     Font->TextColor=0x00FF0000; // red
                     data=1;
                  }
                  else
                  {
                     if((data1&bit)!=(data2&bit))
                     {
                        fail_mask|=bit;
                        Font->TextColor=0x00FF0000; // red
                        data=1;
                     }
                     else
                     {
                        Font->TextColor=0x0000FF00; // green
                     }
                  }
                  int len1=0;
                  if((l%8)==0 && l>0)
                  {
                     sprintf(message,"%s   ",data==1?"X":"O");
                     len1=4;
                  }
                  else if((l%4)==0 && l>0)
                  {
                     sprintf(message,"%s ",data==1?"X":"O");
                     len1=2;
                  }
                  else
                  {
                     sprintf(message,"%s",data==1?"X":"O");
                     len1=1;
                  }
                  print_hdmi(xx,message);
                  xx+=len1;
               }

               line=line_old;
               Font->TextColor=0x00FFFFFF; // white

            }
            if(//mem_regions[j].failed==0 &&
               (i%(mem_regions[j].len>>disp))==0)
            {
               sprintf(message,".");
               if(mem_regions[j].failed==0)
               {
                  printf("%s",message);
               }
               line--;
               print_hdmi(x++,message);
               line++;
            }
            if(ovl_failed)
            {
               if(i<0x80000)
               {
                  i+=0x100-4; // if ovl is not working this space will fail completely, so speed up
               }
            }
         }
         sprintf(message," done");
         printf("%s\n",message);
         if(mem_regions[j].failed==0) sprintf(message," done");
         else sprintf(message," failed");
         line--;
         print_hdmi_ln(x++,message,1);
         sprintf(message,"Press 'C' to continue, 'R' to retry or ESC to exit");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
//         if(mem_regions[j].failed)
         if(mem_regions[j].len>0)
         {
            int exit=0;
            do
            {
               if(read_keyboard(&keybd_data,0))
               {
                  if(keybd_data=='c' || keybd_data=='C')
                     exit=1;
                  if(keybd_data=='r' || keybd_data=='R')
                  {
                     j--;
                     exit=1;
                     fail_mask=0;
                  }
                  if(keybd_data=='\x1b') // ESC
                  {
                     j=NUM_MEM_REGIONS;
                     exit=1;
                  }
               }
            }
            while(exit==0);
         }
      }
end_test:
      for(int j=0;j<NUM_MEM_REGIONS;j++)
      {
         free(mem_regions[j].test_mem1);
         free(mem_regions[j].test_mem2);
      }
      for(int j=0;j<NUM_MEM_REGIONS;j++)
         failed|=mem_regions[j].failed;
   }

   if(failed || config.enable_test==_YES || config.enable_test==_MIN)
   {
      sprintf(message,"[TEST] Restoring...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      ps_write_byte(0xbfe001,0xFF,&timeout); // OVL HIGH to see the ROM
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0Xff to 0xbfe001");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }
      ps_write_byte(0xbfe201,0x00,&timeout); // OVL as INPUT
      if(timeout==0)
      {
         failed=1;
         Font->TextColor=0x00FF0000; // red
         sprintf(message,"[TEST] Timeout when writing 0 to 0xbfe201");
         print_hdmi_ln(0,message,1);
         printf("%s\n",message);
         Font->TextColor=0x00FFFFFF; // white
      }

      sprintf(message,"[TEST] 0xbfe201 %02lX",ps_read_byte(0xbfe201,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      sprintf(message,"[TEST] 0xbfe001 %02lX",ps_read_byte(0xbfe001,&timeout));
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
}
extern const char *bootmode_names[];
uint8_t cursor_data[32*48]={
      1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      3, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 3, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 3, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 3, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 3, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 3, 1, 1, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 3, 1, 0, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 3, 1, 0, 0, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
void monitor_switch(int RTG);
void hdmi_tick(int clean);
void look_for_ver(char *filename)
{
   FIL fil;
   FATFS fatfs;
   TCHAR *Path = "0:/";
   f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
   char filepath[20];
   sprintf(filepath,"0:/%s",filename);
   FRESULT res=f_open(&fil,filepath, FA_READ);
   if(res!=FR_OK)
   {
      sprintf(message,"Can't find the %s file on FAT partition",filename);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      return;
   }
   UINT NumBytesRead;
   FSIZE_t filesize=fil.obj.objsize;
   DATA=malloc(filesize+100);
   ACTIVITY_LED_ON; // ON
   f_read(&fil, DATA, filesize,&NumBytesRead);
   f_close(&fil);
   f_umount(Path);
   ACTIVITY_LED_OFF; // OFF
   uint32_t pos=0;
   while(!(DATA[pos]=='$' && DATA[pos+1]=='V' && DATA[pos+2]=='E'  && DATA[pos+3]=='R'  && DATA[pos+4]==':'))
   {
      if((pos&0xFFFFFL)==0)
         hdmi_tick(0); // roll the bar
      pos++;
      if(pos>=filesize)
         break;
   }
   hdmi_tick(1); // clean the bar
   if(pos>=filesize)
   {
      sprintf(message,"Can't find the $%s: in the %s file on FAT partition","VER",filename); // "VER" hides $VER:
   }
   else
   {
      pos+=5;
      sprintf(message,"%s v%s",filename,&DATA[pos]);
   }
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   free(DATA);
}
int main_thread();
void start_ztop(void);
int sw1_is_down_mobotest=0;

void update_sd(void)
{
   uint8_t keybd_data=0;
   int data=read_rtg_register(REG_ZZ_FW_VERSION);
   v_major=(data>>8)&0xFF;
   v_minor=(data)&0xFF;
   beta=read_rtg_register(REG_ZZ_FW_BETA);
   alfa=read_rtg_register(REG_ZZ_FW_ALFA);
   int w=1920;
   int h=1080;
//   if(config.enable_test!=_YES)
   {
      int offset=8;
      switch(config.bootscreen_resolution)
      {
      case RES_1920x1080:
         w=1920;
         h=1080;
         offset=11;
         break;
      case RES_1280x720:
         w=1280;
         h=720;
         offset=12;
         break;
      case RES_800x600:
      default:
         w=800;
         h=600;
         offset=12+5;
         break;
      }
      line=(h/Font->Height - 1)- offset; // 43 @ 1080p
   }
   screen_width=w;
   if(beta==0)
      sprintf(message,"Z3660 Firmware %d.%02d (%s)",v_major,v_minor,__DATE__);
   else
   {
      if(alfa==0)
         sprintf(message,"Z3660 Firmware %d.%02d BETA %d (%s)",v_major,v_minor,beta,__DATE__);
      else
         sprintf(message,"Z3660 Firmware %d.%02d BETA %d ALFA %d (%s)",v_major,v_minor,beta,alfa,__DATE__);
   }

//   Font=&Font20;
   int x=(w/Font->Width-strlen(message))/2;
   Font->TextColor=0x00FFFFFF;
   Font->BackColor=0x00303030;
   print_hdmi_ln(x,message,2);
//   Font=&Font12;
   Font->BackColor=0x00404040;
   sprintf(message,"After Power ON, 'C' opens the SD update tool, 'Z' opens ZTop ...");
//   if(config.enable_test==_YES)
//      x=0;
//   else
      x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);

   keybd_data=0;
   int time=config.boot_delay;
   if(config.enable_test==_YES)
      time=config.boot_delay+5;
   printf("  Continuing in %d seconds...  \n",time);
   for(int i=time*1000;i>0;i--)
   {
      if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
      {
         sw1_is_down_mobotest=1;
         goto start_console;
      }
      if(read_keyboard(&keybd_data,0))
      {
         if(keybd_data=='c' || keybd_data=='C')
         {
            Font->TextColor=0x00FFFFFF; // white
            Font->BackColor=0x00000000;
            memset(vs.framebuffer,0,w*h*4);
            line=0;
            sprintf(message,"Starting console...                 ");
            printf("Starting console... Use the Amiga keyboard\n");
            print_hdmi_ln(0,message,1);
            goto start_console;
         }
         else if(keybd_data=='z' || keybd_data=='Z')
         {
            Font->TextColor=0x00FFFFFF; // white
            Font->BackColor=0x00000000;
            memset(vs.framebuffer,0,w*h*4);
            line=0;
            sprintf(message,"Starting ZTop...                 ");
            printf("Starting ZTop... Use the Amiga keyboard and mouse\n");
            print_hdmi_ln(0,message,1);
            start_ztop();
         }
      }
      if((i%1000)==0)
      {
         sprintf(message,"   Continuing in %i %s   ",i/1000,i/1000==1?"second":"seconds");
//         if(config.enable_test==_YES)
//            x=0;
//         else
            x=(w/Font->Width-strlen(message))/2;
         print_hdmi(x,message);
      }
      usleep(1000);
   }
   sprintf(message,"          Booting...          ");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   if(preset_selected>=0 && preset_selected<PRESET_CB_MAX-1)
      sprintf(message,"Selected preset %d \"%s\"",preset_selected,env_file_vars_temp[preset_selected].preset_name);
   else
      sprintf(message,"Default preset (z3360cfg.txt file)");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   if(config.boot_mode==MOBOCPU)
      sprintf(message,"Mother Board CPU");
   else if(config.boot_mode==CPU)
      sprintf(message,"060 CPU");
   else
      sprintf(message,"%s CPU EMULATOR",bootmode_names[config.boot_mode]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"BUS Frequency %d MHz",config.cpufreq);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"Z3 RAM %s",config.autoconfig_ram?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"Z3 RTG AUTOCONFIG %s",config.autoconfig_rtg?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"SCSI BOOT %s",config.scsiboot?"Enabled":"Disabled");
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

   sprintf(message,"MAC ADDRESS %02X:%02X:%02X:%02X:%02X:%02X",config.mac_address[0],
         config.mac_address[1],config.mac_address[2],
         config.mac_address[3],config.mac_address[4],config.mac_address[5]);
   x=(w/Font->Width-strlen(message))/2;
   print_hdmi_ln(x,message,1);
   printf("%s\n",message);

//   NBR_ARM(1);
   usleep(1000);
   CPLD_RESET_ARM(0); // CPLD RESET
   return;

start_console:
#ifndef USE_RTOS
   main_thread();
#else
#define DEFAULT_THREAD_PRIO 2
#define THREAD_STACKSIZE 1024*4

   sys_thread_new("main_thrd", (void(*)(void*))main_thread, 0,
                THREAD_STACKSIZE,
                DEFAULT_THREAD_PRIO);
   vTaskStartScheduler();
   while(1);
#endif
}
int main_thread()
{
#ifdef USE_RTOS
   printf("FreeRTOS main_thread\n");
#endif
   char commands[20]="";
   hw_sprite_show(0);
   vs.sprite_showing=0;
   if(!sw1_is_down_mobotest)
   {
      show_options();
   }
   else //if(sw1_is_down)
   {
      sw1_is_down_mobotest=0;
      sprintf(message,"SW1 pressed -> download and CPLD programming...");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
      usleep(1000000);
      for(int i=0;i<100;i++)
      {
         if(XGpioPs_ReadPin(&GpioPs, USER_SW1)==0)
         {
            sw1_is_down_mobotest=1;
            sprintf(message,"SW1 pressed again -> download ALFA and CPLD programming...");
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            i=100;
         }
         usleep(10000);
      }
      if(sw1_is_down_mobotest==0)
         sprintf(commands,"IDR");
      else
         sprintf(commands,"ADR");
      sprintf(message,"Sending commands '%c' '%c' '%c'",commands[0],commands[1],commands[2]);
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }

#ifdef CPLD_PROGRAMMING
   // Init CPLD programming (and read firmware version)
   CPLD_RESET_ARM(0);
   Xil_ExceptionDisable();
   init_xc3sprog();
   Xil_ExceptionEnable();
   CPLD_RESET_ARM(1);
   monitor_switch(1); // 1=RTG
#endif
#include "alfa.txt"
#if REVISION_ALFA == 0
#define READ_FILE_VERSIONS
#endif
#ifdef READ_FILE_VERSIONS
   // here we look for the versions of the BOOT.BIN, FAILSAFE.BIN and Z3660.BIN versions
   sprintf(message,"Reading file versions");
   print_hdmi_ln(0,message,1);
   printf("%s\n",message);
   look_for_ver("BOOT.bin");
   look_for_ver("FAILSAFE.bin");
   look_for_ver("Z3660.bin");
#endif
   int connected=0;
   alfa=0;
   while(1)
   {
      int commands_len=strlen(commands);
      if(read_keyboard(&keybd_data,0) || commands_len>0)
      {
         if(commands_len>0)
         {
            keybd_data=commands[0];
            commands[0]=commands[1];
            commands[1]=commands[2];
            commands[2]=0;
         }
         if(keybd_data=='i' || keybd_data=='I')
         {
            alfa=0;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               int ok=lwip_get_update_version("version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  int data=read_rtg_register(REG_ZZ_FW_VERSION);
                  int v_major=(data>>8)&0xFF;
                  int v_minor=(data)&0xFF;
                  int beta=read_rtg_register(REG_ZZ_FW_BETA);
                  int alfa=read_rtg_register(REG_ZZ_FW_ALFA);
                  if(beta==0)
                     sprintf(message,"The loaded BOOT.bin version is %d.%02d",v_major,v_minor);
                  else
                  {
                     if(alfa==0)
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d",v_major,v_minor,beta);
                     else
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d ALFA %d",v_major,v_minor,beta,alfa);
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='$')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660_scsi.rom file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='V' && DATA[pos+31]=='I')
                     {
//                        DATA[pos+37]=0;
                        sprintf(message,"You have z3660_scsi.rom version  v%s",&DATA[pos+34]);
                     }
                     else
                        sprintf(message,"Can't read the version number of the z3660_scsi.rom file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",0); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='D')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660.jed file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='a' && DATA[pos+14]==' ')
                     {
                        DATA[pos+39]=0;
                        sprintf(message,"You have z3660.jed date  %s",&DATA[pos+15]);
                     }
                     else
                        sprintf(message,"Can't read the date of the z3660.jed file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#endif
            }
         }
         if(keybd_data=='p' || keybd_data=='P')
         {
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               char path[500]="0:";
               printAppHeader(TFTP_PORT);
tftp_restart:
               sprintf(message,"Current path %s (Press SPACE to change the path, R to reboot)",path);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);

               initFileSystem(path,0);
               startApplication();
               listDirectory(path);
               createIndexFileTree(path);
               while(1)
               {
#ifndef USE_RTOS
                  lwip_run();
#endif
                  if(read_keyboard(&keybd_data,0))
                  {
                     if(keybd_data==' ')
                     {
                        strcpy(path,"1:");
                        goto tftp_restart;
                     }
                     else if(keybd_data=='r' || keybd_data=='R')
                     {
                        reboot();
                     }
                  }
               }
            }
         }
         else if(keybd_data=='s' || keybd_data=='S')
         {
            if(server==SHANSHE_SERVER)
            {
               server=GITHUB_SERVER;
               sprintf(message,"github server selected");
            }
            else //if(server==GITHUB_SERVER)
            {
               server=SHANSHE_SERVER;
               sprintf(message,"shanshe server selected");
            }
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else if(keybd_data=='a' || keybd_data=='A')
         {
            alfa=1;
            if(connected==0)
            {
               sprintf(message,"Ethernet PHY auto-negotiation");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               connected=z3660_lwip_connect();
               if(connected)
               {
                  sprintf(message,"Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NOT Connected");
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            if(connected)
            {
               sprintf(message,"Getting version info");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               int ok=lwip_get_update_version("version.txt",1); // 1 = alfa
               if(ok)
               {
                  sprintf(message,"Latest BOOT.bin version is     %s",version_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  int data=read_rtg_register(REG_ZZ_FW_VERSION);
                  int v_major=(data>>8)&0xFF;
                  int v_minor=(data)&0xFF;
                  int beta=read_rtg_register(REG_ZZ_FW_BETA);
                  int alfa=read_rtg_register(REG_ZZ_FW_ALFA);
                  if(beta==0)
                     sprintf(message,"The loaded BOOT.bin version is %d.%02d",v_major,v_minor);
                  else
                  {
                     if(alfa==0)
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d",v_major,v_minor,beta);
                     else
                        sprintf(message,"The loaded BOOT.bin version is %d.%02d BETA %d ALFA %d",v_major,v_minor,beta,alfa);
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               ok=lwip_get_update_version_scsirom("scsirom_version.txt",1); // 1 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660_scsi.rom version is v%s",version_scsirom_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660_scsi.rom", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='$')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660_scsi.rom file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='V' && DATA[pos+31]=='I')
                     {
//                        DATA[pos+37]=0;
                        sprintf(message,"You have z3660_scsi.rom version  v%s",&DATA[pos+34]);
                     }
                     else
                        sprintf(message,"Can't read the version number of the z3660_scsi.rom file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#ifdef CPLD_PROGRAMMING
               ok=lwip_get_update_version_jed("jed_version.txt",1); // 0 = no alfa
               if(ok)
               {
                  sprintf(message,"Latest z3660.jed date is %s",version_jed_string_export);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  FIL fil;
                  FATFS fatfs;
                  TCHAR *Path = "1:/";
                  f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                  f_open(&fil,"1:/z3660.jed", FA_READ);
                  UINT NumBytesRead;
                  FSIZE_t filesize=fil.obj.objsize;
                  DATA=malloc(filesize+100);
                  f_read(&fil, DATA, filesize,&NumBytesRead);
                  f_close(&fil);
                  f_umount(Path);
                  FSIZE_t pos=0;
                  while(DATA[pos]!='D')
                  {
                     pos++;
                     if(pos>=filesize)
                        break;
                  }
                  if(pos>=filesize)
                     sprintf(message,"Can't read the z3660.jed file on exFAT partition");
                  else
                  {
                     pos++;
                     if(DATA[pos]=='a' && DATA[pos+14]==' ')
                     {
                        DATA[pos+39]=0;
                        sprintf(message,"You have z3660.jed date  %s",&DATA[pos+15]);
                     }
                     else
                        sprintf(message,"Can't read the date of the z3660.jed file");
                  }
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  free(DATA);
               }
#endif
            }
         }
         else if(keybd_data=='h' || keybd_data=='H')
         {
            show_options();
         }
         else if(keybd_data=='x' || keybd_data=='X')
         {
            test_nops();
         }
         else if(keybd_data=='q' || keybd_data=='Q')
         {
            test_coremark(config.arm_frequency);
         }
         else if(keybd_data=='v' || keybd_data=='V')
         {
            test_video_plmpeg();
         }
         else if(keybd_data=='g' || keybd_data=='G')
         {
            sprintf(message,"Saving default timings");
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
            set_default_timings();
            sprintf(message,"Default timings saved");
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);
         }
         else if(keybd_data=='w' || keybd_data=='W')
         {
            test_video_plmpeg_with_fps(1);
         }
#ifdef CPLD_PROGRAMMING
         else if(keybd_data=='d' || keybd_data=='D')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               char filename[]="z3660.jed";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(1*1024*1024);
               memset(DATA,0,1*1024*1024);
               int ok=lwip_get_update_jed(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize_jed/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_jed,download_data.checksum32_jed);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  if(checksum32==download_data.checksum32_jed)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_jed,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_jed);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     f_umount(Path);
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }


            CPLD_RESET_ARM(0); // Reset when CPLD programming

#ifdef DIRECT_CPLD_PROGRAMMING
            printf("")
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x24C,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x240,(uint32_t)( 100 *1000)); // TDI CLK90
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x23C,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x238,(uint32_t)( 8 ));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x248,(uint32_t)( 0 *1000));
            XClk_Wiz_WriteReg(XPAR_CLK_WIZ_0_BASEADDR, 0x25C, 0x00000003);
            while(XClk_Wiz_ReadReg(XPAR_CLK_WIZ_0_BASEADDR, 0x004)==0);
#endif
            Xil_DCacheEnable();
            Xil_ExceptionDisable();
            XIicPs_SetSClk(&IicInstance, I2C_FREQ_SII9022*4); // 400 kHz
            main_xc3sprog(cpufreq_values[config.arm_frequency]);
            i2c_finish();
            XIicPs_SetSClk(&IicInstance, I2C_FREQ_SII9022);
            Xil_ExceptionEnable();

            //Restore pins
#ifdef DIRECT_CPLD_PROGRAMMING
            configure_clk(config.cpufreq,0,1);
#endif
            CPLD_RESET_ARM(1);

         }
#endif // CPLD_PROGRAMMIG
         else if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F' || keybd_data=='t' || keybd_data=='T')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               sprintf(message,"Downloading the BOOT.bin file");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(32*1024*1024);
               memset(DATA,0,32*1024*1024);
               int ok=lwip_get_update("BOOT.BIN",alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32,download_data.checksum32);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  char filename[20];
                  if(keybd_data=='u' || keybd_data=='U' || keybd_data=='f' || keybd_data=='F')
                  {
                     if(keybd_data=='u' || keybd_data=='U')
                        strcpy(filename,"Z3660.bin");
                     else
                        strcpy(filename,"FAILSAFE.bin");
                     if(checksum32==download_data.checksum32)
                     {
                        sprintf(message,"Writing the %s file...",filename);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                        ACTIVITY_LED_ON; // ON
                        FIL fil;
                        FATFS fatfs;
                        TCHAR *Path = "0:/";
                        f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                        f_open(&fil,filename, FA_CREATE_ALWAYS | FA_WRITE);
                        UINT NumBytesWritten,NumBytesWritten_total=0;
                        int num=download_data.filesize/(1L<<20);
                        print_hdmi_ln(0,"[",0);
                        printf("[");
                        fflush(stdout);
                        for(int i=0;i<num;i++)
                        {
                           print_hdmi_ln(i+1,".",0);
                           printf(".");
                           fflush(stdout);
                        }
                        print_hdmi_ln(num+1,"]",0);
                        printf("]\r[");
                        fflush(stdout);
                        for(int i=0;i<num;i++)
                        {
                           f_write(&fil, DATA+(i*(1L<<20)), 1L<<20,&NumBytesWritten);
                           print_hdmi_ln(i+1,"=",0);
                           printf("=");
                           fflush(stdout);
                           NumBytesWritten_total+=NumBytesWritten;
                        }
                        f_write(&fil, DATA+(num*(1L<<20)), download_data.filesize-((1L<<20)*num),&NumBytesWritten);
                        print_hdmi_ln(num+1,"]",1);
                        printf("]\n");
                        fflush(stdout);
                        NumBytesWritten_total+=NumBytesWritten;
                        f_close(&fil);
                        ACTIVITY_LED_OFF; // OFF
                        sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten_total,download_data.filesize);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                        f_umount(Path);
                     }
                     else
                     {
                        sprintf(message,"File %s NOT written (bad checksum)",filename);
                        print_hdmi_ln(0,message,1);
                        printf("%s\n",message);
                     }
                  }
                  else
                  {
                     if(checksum32==download_data.checksum32)
                        sprintf(message,"Download test OK");
                     else
                        sprintf(message,"Download test NOT OK !!!");
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }
         }
         else if(keybd_data=='o' || keybd_data=='O')
         {
            if(connected==0)
            {
               sprintf(message,"NOT Connected");
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            else
            {
               char filename[]="z3660_scsi.rom";
               sprintf(message,"Downloading the %s file",filename);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(1*1024*1024);
               memset(DATA,0,1*1024*1024);
               int ok=lwip_get_update_scsirom(filename,alfa);
               if(ok)
               {
                  uint32_t checksum32=0;
                  uint32_t *data_u32=(uint32_t *)DATA;
                  for(uint32_t i=0;i<download_data.filesize_scsirom/4;i++)
                     checksum32+=__builtin_bswap32(data_u32[i]);
                  sprintf(message,"Checksum-32 %lu (%08lX) %lu (%08lX)",checksum32,checksum32,download_data.checksum32_scsirom,download_data.checksum32_scsirom);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  if(checksum32==download_data.checksum32_scsirom)
                  {
                     sprintf(message,"Writing the %s file...",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     ACTIVITY_LED_ON; // ON
                     FIL fil;
                     FATFS fatfs;
                     TCHAR *Path = "1:/";
                     f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
                     char filename_ex[20];
                     sprintf(filename_ex,"%s%s",Path,filename);
                     f_open(&fil,filename_ex, FA_CREATE_ALWAYS | FA_WRITE);
                     UINT NumBytesWritten;
                     f_write(&fil, DATA, download_data.filesize_scsirom,&NumBytesWritten);
                     f_close(&fil);
                     ACTIVITY_LED_OFF; // OFF

                     sprintf(message,"File %s written (%d bytes of %ld)",filename,NumBytesWritten,download_data.filesize_scsirom);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                     f_umount(Path);
                  }
                  else
                  {
                     sprintf(message,"File %s NOT written (bad checksum)",filename);
                     print_hdmi_ln(0,message,1);
                     printf("%s\n",message);
                  }
               }
               free(DATA);
            }
         }
         else if(keybd_data=='b' || keybd_data=='B' || keybd_data=='n' || keybd_data=='N' || keybd_data=='m' || keybd_data=='M' ||
               keybd_data=='j' || keybd_data=='J' || keybd_data=='k' || keybd_data=='K' || keybd_data=='l' || keybd_data=='L')
         {
            char src[20];
            char dst[20];
            if(keybd_data=='b' || keybd_data=='B')
            {
               strcpy(src,"BOOT.bin");
               strcpy(dst,"FAILSAFE.bin");
            }
            else if(keybd_data=='n' || keybd_data=='N')
            {
               strcpy(src,"BOOT.bin");
               strcpy(dst,"Z3660.bin");
            }
            else if(keybd_data=='m' || keybd_data=='M')
            {
               strcpy(src,"Z3660.bin");
               strcpy(dst,"BOOT.bin");
            }
            else if(keybd_data=='j' || keybd_data=='J')
            {
               strcpy(src,"Z3660.bin");
               strcpy(dst,"FAILSAFE.bin");
            }
            else if(keybd_data=='k' || keybd_data=='K')
            {
               strcpy(src,"FAILSAFE.bin");
               strcpy(dst,"BOOT.bin");
            }
            else //if(keybd_data=='l' || keybd_data=='L')
            {
               strcpy(src,"FAILSAFE.bin");
               strcpy(dst,"Z3660.bin");
            }
            sprintf(message,"Copying file %s to %s",src,dst);
            print_hdmi_ln(0,message,1);
            printf("%s\n",message);

            ACTIVITY_LED_ON; // ON
            FIL fil_src,fil_dst;
            FATFS fatfs;
            TCHAR *Path = "0:/";
            f_clk_mount(&fatfs, Path, 1); // 1 mount immediately
            FRESULT res=f_open(&fil_src,src, FA_READ);
            if(res==FR_OK)
            {
               UINT NumBytesRead,NumBytesRead_total=0;
               FSIZE_t filesize=fil_src.obj.objsize;
               sprintf(message,"Filesize to copy %lld bytes",filesize);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
               DATA=malloc(filesize+100);
               int num=filesize/(1L<<20);
               print_hdmi_ln(0,"[",0);
               printf("[");
               fflush(stdout);
               for(int i=0;i<num;i++)
               {
                  f_read(&fil_src, DATA+(i*(1L<<20)), 1L<<20,&NumBytesRead);
                  NumBytesRead_total+=NumBytesRead;
                  print_hdmi_ln(i+1,".",0);
                  printf(".");
                  fflush(stdout);
               }
               f_read(&fil_src, DATA+(num*(1L<<20)), filesize-(1L<<20)*num,&NumBytesRead);
               NumBytesRead_total+=NumBytesRead;
               print_hdmi_ln(num+1,"]",0);
               printf("]\r[");
               fflush(stdout);
               f_close(&fil_src);

               f_open(&fil_dst,dst, FA_CREATE_ALWAYS | FA_WRITE);
               UINT NumBytesWritten,NumBytesWritten_total=0;
               for(int i=0;i<num;i++)
               {
                  f_write(&fil_dst, DATA+(i*(1L<<20)), 1L<<20,&NumBytesWritten);
                  NumBytesWritten_total+=NumBytesWritten;
                  print_hdmi_ln(i+1,"=",0);
                  printf("=");
                  fflush(stdout);
               }
               f_write(&fil_dst, DATA+(num*(1L<<20)), filesize-(1L<<20)*num,&NumBytesWritten);
               NumBytesWritten_total+=NumBytesWritten;
               print_hdmi_ln(num+1,"]",1);
               printf("]\n");
               fflush(stdout);
               f_close(&fil_dst);

               if(NumBytesRead_total==NumBytesWritten_total)
               {
                  sprintf(message,"Copied file %s to %s",src,dst);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
               else
               {
                  sprintf(message,"NumBytesRead %d != NumBytesWritten %d",NumBytesRead_total,NumBytesWritten_total);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
                  sprintf(message,"Error when copying file %s to %s",src,dst);
                  print_hdmi_ln(0,message,1);
                  printf("%s\n",message);
               }
            }
            else
            {
               sprintf(message,"Can't read file %s",src);
               print_hdmi_ln(0,message,1);
               printf("%s\n",message);
            }
            f_umount(Path);
            ACTIVITY_LED_OFF; // OFF
            free(DATA);
         }
         else if(keybd_data=='e' || keybd_data=='E')
         {
            sd_fileops_menu();
         }
         else if(keybd_data=='r' || keybd_data=='R')
         {
            reboot();
         }
      }
#ifndef USE_RTOS
      if(connected)
         lwip_run();
#endif
   }
}
void start_ztop(void)
{

   init_win();

   uint8_t* bmp_data = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2));
   uint8_t* bmp_colors = (uint8_t*) (((uint32_t) vs.framebuffer) + (1920*1080*2+32*48));

   bmp_colors[ 0]=0xFF;bmp_colors[ 1]=0xFF;bmp_colors[ 2]=0xFF; // yeah 24 bit per color
   bmp_colors[ 3]=0xF3;bmp_colors[ 4]=0x30;bmp_colors[ 5]=0x30;
   bmp_colors[ 6]=0xF0;bmp_colors[ 7]=0xEC;bmp_colors[ 8]=0xDC;
   bmp_colors[ 9]=0x00;bmp_colors[10]=0x00;bmp_colors[11]=0x00;

   for(int i=0;i<32*48;i++)
      bmp_data[i]=cursor_data[i];
   update_hw_sprite_clut(bmp_data,bmp_colors,32,48,0,config.doubled_cursor,0);
   update_hw_sprite_pos();
   hw_sprite_show(1);
   vs.sprite_showing=1;
   do_clip_hw_sprite(0, 0);

   xadc_init();
   ltc2990_init();
   PT_INIT(&pt_measures);
   PT_INIT(&pt_timer);
   fpga_interrupt_connect(isr_video, isr_audio_tx, isr_usb, INT_IPL_ON_THIS_CORE);
   for(int i=0;i<20;i++)
   {
      usleep(1000);
      task_counter=10000000;
      measures_thread(&pt_measures);
   }

   show_ztop();

   monitor_switch(1); // 1=RTG

   while(1)
   {
      task_counter+=100;
      measures_thread(&pt_measures);
      timer_thread(&pt_timer);
      if(read_keyboard(&keybd_data,1))
      {
//         if(keybd_data=='\x1b') // ESC
//         {
//            reboot();
//         }
//         printf("key = 0x%02X\n",keybd_data);
         if(amiga_status) // amiga cursors + Amiga key
         {
            if(keybd_data=='\x1f') // up
            {
               mousey-=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1e') // down
            {
               mousey+=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1d') // right
            {
               mousex+=10;
               update_mouse_position();
            }
            if(keybd_data=='\x1c') // left
            {
               mousex-=10;
               update_mouse_position();
            }
            if(keybd_data=='\xfd') // left alt + Amiga key = mouse click
            {
               mouse_pressed=1;
            }
         }
         if(keybd_data=='\x1b') // serial debug cursors
         {
            int timeout_key=100000;
            while(timeout_key>0 && !read_keyboard(&keybd_data,1))
            {
               timeout_key--;
            }
            if(timeout_key!=0)
            {
               if(keybd_data=='\x5b')
               {
                  timeout_key=100000;
                  while(timeout_key>0 && !read_keyboard(&keybd_data,1))
                  {
                     timeout_key--;
                  }
                  if(timeout_key!=0)
                  {
                     if(keybd_data=='\x41') // up
                     {
                        mousey-=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x42') // down
                     {
                        mousey+=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x43') // right
                     {
                        mousex+=10;
                        update_mouse_position();
                     }
                     if(keybd_data=='\x44') // left
                     {
                        mousex-=10;
                        update_mouse_position();
                     }
                  }
               }
            }
         }
         if(keybd_data==' ') // space = mouse click
         {
            mouse_pressed=1;
         }
         if(keybd_data=='\x09') // TAB
         {
            selected_tab++;
            if(selected_tab==NUM_TABS+1)
            {
               selected_tab=1;
            }
            tab_change(*tabs[selected_tab-1],1);
            (*tabs[selected_tab-1])->action();
         }
         if(selected_tab==TAB_MISC && mac_textedit->cursor_pos>0)
         {
            if(keybd_data=='\x1c')
            {
               mac_textedit->cursor_pos--;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos--; // skip the :
               if(mac_textedit->cursor_pos<=0)
                  mac_textedit->cursor_pos=17;
               paint_mac_textedit();
            }
            if(keybd_data=='\x1d')
            {
               mac_textedit->cursor_pos++;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos++; // skip the :
               if(mac_textedit->cursor_pos>=18)
                  mac_textedit->cursor_pos=1;
               paint_mac_textedit();
            }
            if((keybd_data>='0' && keybd_data<='9') ||
               (keybd_data>='a' && keybd_data<='f') ||
               (keybd_data>='A' && keybd_data<='F') )
            {
               int idx[19]={0,0,1,1,
                              2,3,3,
                              4,5,5,
                              6,7,7,
                              8,9,9,
                              10,11,11,
                              };
               if(keybd_data>='a' && keybd_data<='f')
                  keybd_data+='A'-'a';
               if(keybd_data>='0' && keybd_data<='9')
                  keybd_data-='0';
               else
                  keybd_data-='A'-10;
               int index=idx[mac_textedit->cursor_pos];
               if(index&1)
                  mac_textedit->mac_address[index>>1]=(mac_textedit->mac_address[index>>1]&0xF0)|keybd_data;
               else
                  mac_textedit->mac_address[index>>1]=(mac_textedit->mac_address[index>>1]&0x0F)|(keybd_data<<4);
               mac_textedit->cursor_pos++;
               if((mac_textedit->cursor_pos%3)==0) mac_textedit->cursor_pos++; // skip the :
               if(mac_textedit->cursor_pos>=18)
                  mac_textedit->cursor_pos=1;
               paint_mac_textedit();
            }
         }
         if(selected_tab==TAB_PRESET && preset_textedit[preset_selected]->cursor_pos>0)
         {
            if(keybd_data=='\x1c')
            {
//#define debug_pos printf("pos %d len %d\n",preset_textedit[preset_selected]->cursor_pos,strlen(preset_textedit[preset_selected]->text))
#define debug_pos

               preset_textedit[preset_selected]->cursor_pos--;
               if(preset_textedit[preset_selected]->cursor_pos<=1)
                  preset_textedit[preset_selected]->cursor_pos=1;
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x1d')
            {
               preset_textedit[preset_selected]->cursor_pos++;
               int len=strlen(preset_textedit[preset_selected]->text);
               if(preset_textedit[preset_selected]->cursor_pos>=len+1)
                  preset_textedit[preset_selected]->cursor_pos=len+1;
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x08')
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               if(preset_textedit[preset_selected]->cursor_pos>1 && preset_textedit[preset_selected]->cursor_pos-1<=len)
               {
                  for(int i=preset_textedit[preset_selected]->cursor_pos-1;i<=len;i++)
                     preset_textedit[preset_selected]->text[i-1]=preset_textedit[preset_selected]->text[i];
                  preset_textedit[preset_selected]->cursor_pos--;
               }
               paint_preset_textedit();

               debug_pos;
            }
            else if(keybd_data=='\x7f')
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               for(int i=preset_textedit[preset_selected]->cursor_pos;i<=len+1;i++)
                  preset_textedit[preset_selected]->text[i-1]=preset_textedit[preset_selected]->text[i];
               paint_preset_textedit();

               debug_pos;
            }
            else if((keybd_data>='0' && keybd_data<='9') ||
                    (keybd_data>='a' && keybd_data<='z') ||
                    (keybd_data>='A' && keybd_data<='Z') ||
                     keybd_data=='_' || keybd_data=='-' || keybd_data==' ' )
            {
               int len=strlen(preset_textedit[preset_selected]->text);
               if(len<PRESET_MAX_LENGTH)
               {
                  int index=preset_textedit[preset_selected]->cursor_pos-1;
                  if(index<len)
                  {
                     for(int i=len;i>=index;i--)
                        preset_textedit[preset_selected]->text[i+1]=preset_textedit[preset_selected]->text[i];
                  }
                  preset_textedit[preset_selected]->text[index]=keybd_data;
                  preset_textedit[preset_selected]->cursor_pos++;
                  if(preset_textedit[preset_selected]->cursor_pos>=PRESET_MAX_LENGTH+1)
                     preset_textedit[preset_selected]->cursor_pos=PRESET_MAX_LENGTH+1;
                  paint_preset_textedit();
               }
               debug_pos;
            }
         }
         if(selected_tab==TAB_TIMINGS && (
               (timings_phase_textedit[0]->cursor_pos>0) ||
               (timings_phase_textedit[1]->cursor_pos>0) ||
               (timings_phase_textedit[2]->cursor_pos>0) ||
               (timings_phase_textedit[3]->cursor_pos>0) ||
               (timings_phase_textedit[4]->cursor_pos>0) ||
               (timings_phase_textedit[5]->cursor_pos>0) ||
               (timings_divider_textedit[0]->cursor_pos>0) ||
               (timings_divider_textedit[1]->cursor_pos>0) ||
               (timings_divider_textedit[2]->cursor_pos>0) ||
               (timings_divider_textedit[3]->cursor_pos>0) ||
               (timings_divider_textedit[4]->cursor_pos>0) ||
               (timings_muldiv_textedit[0]->cursor_pos>0) ||
               (timings_muldiv_textedit[1]->cursor_pos>0)
               ) )
         {
            if(keybd_data=='\x1c')
            {
//#define debug_pos printf("pos %d len %d\n",preset_textedit[preset_selected]->cursor_pos,strlen(preset_textedit[preset_selected]->text))
#define debug_pos
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               b->cursor_pos--;
               if(b->cursor_pos<=1)
                  b->cursor_pos=1;
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x1d')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               b->cursor_pos++;
               int len=strlen(b->text);
               if(b->cursor_pos>=len+1)
                  b->cursor_pos=len+1;
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x08')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               int len=strlen(b->text);
               if(b->cursor_pos>1 && b->cursor_pos-1<=len)
               {
                  for(int i=b->cursor_pos-1;i<=len;i++)
                     b->text[i-1]=b->text[i];
                  b->cursor_pos--;
               }
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if(keybd_data=='\x7f')
            {
               TextEdit *b;
               if(timings_phase_textedit[0]->cursor_pos>0)
                  b=timings_phase_textedit[0];
               else if(timings_phase_textedit[1]->cursor_pos>0)
                  b=timings_phase_textedit[1];
               else if(timings_phase_textedit[2]->cursor_pos>0)
                  b=timings_phase_textedit[2];
               else if(timings_phase_textedit[3]->cursor_pos>0)
                  b=timings_phase_textedit[3];
               else if(timings_phase_textedit[4]->cursor_pos>0)
                  b=timings_phase_textedit[4];
               else if(timings_phase_textedit[5]->cursor_pos>0)
                  b=timings_phase_textedit[5];
               else if(timings_divider_textedit[0]->cursor_pos>0)
                  b=timings_divider_textedit[0];
               else if(timings_divider_textedit[1]->cursor_pos>0)
                  b=timings_divider_textedit[1];
               else if(timings_divider_textedit[2]->cursor_pos>0)
                  b=timings_divider_textedit[2];
               else if(timings_divider_textedit[3]->cursor_pos>0)
                  b=timings_divider_textedit[3];
               else if(timings_divider_textedit[4]->cursor_pos>0)
                  b=timings_divider_textedit[4];
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
                  b=timings_muldiv_textedit[0];
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
                  b=timings_muldiv_textedit[1];
               else
                  b=timings_phase_textedit[0];
               int len=strlen(b->text);
               for(int i=b->cursor_pos;i<=len+1;i++)
                  b->text[i-1]=b->text[i];
               test_tab_timings();
               ns_repaint();
               paint_b_apply_timings();
               paint_b_apply_all_timings();

               debug_pos;
            }
            else if((keybd_data>='0' && keybd_data<='9') ||
                     keybd_data=='-' )
            {
               TextEdit *b;
               int max_length;
               if(timings_phase_textedit[0]->cursor_pos>0)
               {
                  b=timings_phase_textedit[0];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[1]->cursor_pos>0)
               {
                  b=timings_phase_textedit[1];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[2]->cursor_pos>0)
               {
                  b=timings_phase_textedit[2];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[3]->cursor_pos>0)
               {
                  b=timings_phase_textedit[3];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[4]->cursor_pos>0)
               {
                  b=timings_phase_textedit[4];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_phase_textedit[5]->cursor_pos>0)
               {
                  b=timings_phase_textedit[5];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               else if(timings_divider_textedit[0]->cursor_pos>0)
               {
                  b=timings_divider_textedit[0];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[1]->cursor_pos>0)
               {
                  b=timings_divider_textedit[1];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[2]->cursor_pos>0)
               {
                  b=timings_divider_textedit[2];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[3]->cursor_pos>0)
               {
                  b=timings_divider_textedit[3];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_divider_textedit[4]->cursor_pos>0)
               {
                  b=timings_divider_textedit[4];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_muldiv_textedit[0]->cursor_pos>0)
               {
                  b=timings_muldiv_textedit[0];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else if(timings_muldiv_textedit[1]->cursor_pos>0)
               {
                  b=timings_muldiv_textedit[1];
                  max_length=TIMINGS_DIVIDER_MAX_LENGTH;
               }
               else
               {
                  b=timings_phase_textedit[0];
                  max_length=TIMINGS_PHASE_MAX_LENGTH;
               }
               int len=strlen(b->text);
               if(len<max_length)
               {
                  int index=b->cursor_pos-1;
                  if(keybd_data!='-' || index==0) // do nothing when '-' and index!=0
                  {
                     if(index<len)
                     {
                        for(int i=len;i>=index;i--)
                           b->text[i+1]=b->text[i];
                     }
                     b->text[index]=keybd_data;
                     b->cursor_pos++;
                     if(b->cursor_pos>max_length+1)
                        b->cursor_pos=max_length+1;
                     test_tab_timings();
                     ns_repaint();
                     paint_b_apply_timings();
                     paint_b_apply_all_timings();
                  }
               }
               debug_pos;
            }
         }
      }

      if(mouse_pressed==1 && mouse_pressed_old==0)
      {
         win_run();
      }
      else if(mouse_pressed==0 && mouse_pressed_old==1)
      {
         win_actions();
      }
      else if(mouse_pressed && is_dragging())
      {
         int drag_x=get_drag_mousex_pre();
         int drag_y=get_drag_mousey_pre();
         int delta_x=mousex-drag_x;
         int delta_y=mousey-drag_y;
         if(delta_x!=0 || delta_y!=0)
         {
            calculate_drag(delta_x,delta_y);
         }
      }
      win_repaint();
      mouse_pressed_old=mouse_pressed;
      mouse_pressed=0;
   }
}
void hdmi_tick(int clean)
{
   char chars[]="\\|/-";
   static int i=0;
   if(clean==0)
   {
      sprintf(message,"%c",chars[i++]);
      i=i&3;
      print_hdmi(0,message);
   }
   else if(clean==1)
   {
      sprintf(message," ");
      print_hdmi(0,message);
   }
   else //if(clean==2)
   {
      sprintf(message,"Timeout");
      print_hdmi_ln(0,message,1);
      printf("%s\n",message);
   }
}

