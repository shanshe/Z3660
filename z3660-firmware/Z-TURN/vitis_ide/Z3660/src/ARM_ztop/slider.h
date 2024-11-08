#ifndef _SLIDER_H_
#define _SLIDER_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
   int x;
   int y;
   int w;
   int h;
   uint8_t is_pressed;
   uint8_t b_was_at_cursor;
   void (*action)();
   void (*strpos)(char *);
   Tabs tab;
   int pos;
   int max;
   int min;
   int step;
   int text_pos;
} Slider;

#define TON_MAX 2000e-6
#define TOFF_MAX 20e-3

#define SLIDER_BPTON_MAX 2000
#define SLIDER_BPTON_STEP 100
#define SLIDER_BPTOFF_MAX 20
#define SLIDER_BPTOFF_STEP 1

extern Slider *slider_cpufreq;
extern Slider *slider_bpton;
extern Slider *slider_bptoff;

#define SLIDER(t) do {\
   int steps=t->w/(((t->max-t->min)/(t->step*2)));\
   FILLRECT(   t->x,   t->y,   t->w,   t->h, 0x00A8A8A8);     \
   for(int i=0;i<((t->max-t->min)/(t->step*2)+1);i++) {      \
      FILLRECT( t->x+i*steps  , t->y, 1, t->h, 0x00000000); \
      FILLRECT( t->x+i*steps+1, t->y, 1, t->h, 0x00FFFFFF); \
   }\
   for(int i=0;i<((t->max-t->min)/(t->step*2));i++) {                    \
      FILLRECT( t->x+i*steps+steps/2  , t->y+2, 1, t->h-4, 0x00000000); \
      FILLRECT( t->x+i*steps+steps/2+1, t->y+2, 1, t->h-4, 0x00FFFFFF); \
   }\
   int y1=t->y+t->h/2-3;\
   FILLRECT(    t->x+1,y1+1, t->w-2, 4-1, 0x00A8A8A8); \
   FILLRECT(      t->x,   y1,  t->w,   1, 0x00000000); \
   FILLRECT(      t->x, y1+1,     1, 4-1, 0x00000000); \
   FILLRECT( t->x+t->w,   y1,     1,   4, 0x00FFFFFF); \
   FILLRECT(    t->x+1, y1+4,  t->w,   1, 0x00FFFFFF); \
   Font->BackColor=0x00A8A8A8; \
   Font->TextColor=0x00000000; \
   t->strpos(message);\
   if(t->text_pos==0)\
      displayStringAt(Font,t->x+(t->w-Font->Width*7)/2,t->y-15,(uint8_t*)message,LEFT_MODE); \
   else\
      displayStringAt(Font,t->x+t->w+10,t->y+1,(uint8_t*)message,LEFT_MODE); \
   int x1=((t->pos-t->min)*(t->w-5)/(t->max-t->min));                      \
   FILLRECT(    t->x+1,   t->y+5, x1-2, 4-1, 0x006088BE);                                 \
   x1=t->x+((t->pos-t->min)*(t->w-5))/(t->max-t->min); int w1=5;           \
   FILLRECT(    x1+1,      t->y+1, w1-2, t->h-1, 0x00A8A8A8); \
   FILLRECT(      x1,        t->y,   w1,      1, 0x00FFFFFF); \
   FILLRECT(      x1,      t->y+1,    1, t->h-2, 0x00FFFFFF); \
   FILLRECT(    x1+1,      t->y+1,    1, t->h-3, 0x00FFFFFF); \
   FILLRECT(   x1+w1,        t->y,    1, t->h-1, 0x00000000); \
   FILLRECT( x1+w1-1,      t->y+1,    1, t->h-2, 0x00000000); \
   FILLRECT(    x1+1, t->y+t->h-1,   w1,      1, 0x00000000);}while(0)

void sliders_run(void);
void init_sliders(void);
void slider_action(Slider *s);
void sliders_repaint(void);
void sliders_action(void);
void recalculate_coords_sliders(void);
void paint_slider_cpufreq(void);
void paint_slider_bpton(void);
void paint_slider_bptoff(void);

void test_beeper(uint32_t ton, uint32_t toff);

#define FREQ_OSC 18432000
#define INT_TON ((uint32_t)(config.bp_ton*FREQ_OSC))
#define INT_TOFF ((uint32_t)((config.bp_ton + config.bp_toff)*FREQ_OSC))
#define ENV_INT_TON ((uint32_t)(env_file_vars_temp[preset_selected].bp_ton*FREQ_OSC))
#define ENV_INT_TOFF ((uint32_t)((env_file_vars_temp[preset_selected].bp_ton + env_file_vars_temp[preset_selected].bp_toff)*FREQ_OSC))


#endif // _SLIDER_H_
