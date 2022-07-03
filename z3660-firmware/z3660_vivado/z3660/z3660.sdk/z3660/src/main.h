/*
 * main.h
 *
 *  Created on: 8 jun. 2022
 *      Author: shanshe
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

void reset_video(int reset_frame_buffer);
void loop2(void);
void Display2(void);
void rtg_loop(void);
void rtg_init(void);
int sii9022_init(void);
void init_vdma(int hsize, int vsize, int hdiv, int vdiv, u32 buspos);
void cache_flush(void);


#define NO_RESET_FRAMEBUFFER 0
#define    RESET_FRAMEBUFFER 1
//#define DEMO_MAX_FRAME (1920*1080*2)
//#define DEMO_STRIDE (1920 * 4)

#endif /* SRC_MAIN_H_ */
