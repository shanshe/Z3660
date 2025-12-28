#ifndef _BUTTON_H_
#define _BUTTON_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
   int x;
   int y;
   int w;
   int h;
   char *text;
   uint8_t is_pressed;
   uint8_t b_was_at_cursor;
   void (*action)();
   Tabs tab;
   int disabled;
} Button;

extern sFONT *Font;
extern uint32_t* fb;
extern uint32_t fb_pitch;

#define BUTTON(t) do {\
   FILLRECT( t->x+1, t->y+1, t->w-1, t->h-1, 0x00A8A8A8);           \
   FILLRECT(   t->x,   t->y,   t->w,      1, 0x00FFFFFF);           \
   FILLRECT(   t->x, t->y+1,      1, t->h-1, 0x00FFFFFF);           \
   FILLRECT( t->x+1, t->y+1,      1, t->h-2, 0x00FFFFFF);           \
   Font->BackColor=0x00A8A8A8;                                      \
   Font->TextColor=0x00000000;                                      \
   displayStringAt(Font,t->x+8,t->y+2,(uint8_t*)t->text,LEFT_MODE); \
   FILLRECT(   t->x+t->w, t->y+1,    1, t->h-1, 0x00000000);        \
   FILLRECT( t->x+t->w-1, t->y+2,    1, t->h-2, 0x00000000);        \
   FILLRECT(      t->x+1, t->y+t->h, t->w,      1, 0x00000000);     \
   if(t->disabled) {                                                \
      for(int i=0;i<t->w;i++)                                       \
         for(int j=0;j<t->h;j++) {                                  \
            uint16_t* dp =(uint16_t*)( fb + ((t->y+j) * fb_pitch)); \
            if(((i+j)&1)==0)                                        \
               dp[t->x+i]=0x0000;}                                  \
   } }while(0)
#define BUTTON_PRESSED(t) do {\
   FILLRECT( t->x+1, t->y+1, t->w-1, t->h-1, 0x006088BE); \
   FILLRECT(   t->x,   t->y,   t->w,      1, 0x00000000); \
   FILLRECT(   t->x, t->y+1,      1, t->h-1, 0x00000000); \
   FILLRECT( t->x+1, t->y+1,      1, t->h-2, 0x00000000); \
   Font->BackColor=0x006088BE; \
   Font->TextColor=0x00000000; \
   displayStringAt(Font,t->x+8,t->y+2,(uint8_t*)t->text,LEFT_MODE); \
   FILLRECT(   t->x+t->w,    t->y+1,    1, t->h-1, 0x00FFFFFF);     \
   FILLRECT( t->x+t->w-1,    t->y+2,    1, t->h-2, 0x00FFFFFF);     \
   FILLRECT(      t->x+1, t->y+t->h, t->w,      1, 0x00FFFFFF);     \
   Font->BackColor=0x00A8A8A8;                                      \
   }while(0)

void recalculate_coords_buttons(void);
void init_buttons(void);

void paint_b_apply_screen(void);
void paint_b_apply_boot_mode(void);
void paint_b_apply_all_boot(void);
void paint_b_apply_scsi(void);
void paint_b_apply_all_scsi(void);
void paint_b_apply_misc(void);
void paint_b_apply_all_misc(void);
void paint_b_apply_preset(void);
void paint_b_apply_all_preset(void);
void paint_b_delete_preset(void);
void paint_b_apply_timings(void);
void paint_b_apply_all_timings(void);

int8_t is_cursor_at_button(Button *b);
void buttons_run(void);
void buttons_action(void);
void buttons_repaint(void);

#endif // _BUTTON_H_
