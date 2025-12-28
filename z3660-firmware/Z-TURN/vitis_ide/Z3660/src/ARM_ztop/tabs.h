#ifndef _TABS_H_
#define _TABS_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
   int x;
   int y;
   int w;
   int h;
   char *text;
   uint8_t is_pressed;
   uint8_t t_was_at_cursor;
   void (*action)();
   Tabs tab;
   uint8_t is_painted;
} Tab;

extern sFONT *Font;
extern Tab **tabs[NUM_TABS];
#define TAB_MIN_WIDTH 51
#define TAB_HEIGHT 14

#define TAB(t,selected) do{\
   FILLRECT(     t->x+ 5,      t->y+ 0, t->w-9,      1, 0x00FFFFFF); \
   FILLRECT(     t->x+ 3,      t->y+ 1,      2,      1, 0x00FFFFFF); \
   FILLRECT(     t->x+ 2,      t->y+ 2,      1,      2, 0x00FFFFFF); \
   FILLRECT(     t->x+ 1,      t->y+ 4,      2, t->h-5, 0x00FFFFFF); \
   FILLRECT(     t->x+ 0, t->y+t->h- 1,      1,      1, 0x00FFFFFF); \
   FILLRECT(     t->x+ 1, t->y+t->h- 1,      1,      1, 0x00FFFFFF); \
   FILLRECT( t->x+t->w- 4,     t->y+ 1,      2,      1, 0x00FFFFFF); \
   Font->BackColor=0x00A8A8A8;    \
   if(selected)                   \
      Font->TextColor=0x00FFFFFF; \
   else                           \
      Font->TextColor=0x00000000; \
   displayStringAt(Font,t->x+12,t->y+2,(uint8_t*)t->text,LEFT_MODE); \
   Font->TextColor=0x00000000; \
   FILLRECT( t->x+t->w- 2,      t->y+ 2,    1,      2, 0x00000000); \
   FILLRECT( t->x+t->w- 2,      t->y+ 4,    2, t->h-5, 0x00000000); \
   FILLRECT( t->x+t->w+ 0, t->y+t->h- 1,    1,      1, 0x00000000); \
   FILLRECT( t->x+t->w- 1, t->y+t->h- 1,    1,      1, 0x00000000); \
   if(selected) FILLRECT( t->x+ 1, t->y+t->h+ 0, t->w-1,   1, 0x00A8A8A8); \
   else FILLRECT( t->x+ 1, t->y+t->h+ 0,  t->w-1,   1, 0x00FFFFFF);}while(0)

void paint_tab_info(void);
void t_info_action(void);

void paint_tab_boot(void);
void t_boot_action(void);

void init_tabs(void);

int8_t is_cursor_at_tab(Tab *t);

void tabs_run(void);
void tabs_action(void);
void tabs_repaint(void);
void tabs_paint_init(void);
void paint_mac_textedit(void);
void paint_preset_textedit(void);
uint32_t paint_timings_phase_pclk_textedit(void);
uint32_t paint_timings_phase_clken_textedit(void);
uint32_t paint_timings_phase_bclk_textedit(void);
uint32_t paint_timings_phase_cpuclk_textedit(void);
uint32_t paint_timings_phase_clk90_textedit(void);
uint32_t paint_timings_emu_extra_phase_textedit(void);
uint32_t paint_timings_divider_pclk_textedit(void);
uint32_t paint_timings_divider_clken_textedit(void);
uint32_t paint_timings_divider_bclk_textedit(void);
uint32_t paint_timings_divider_cpuclk_textedit(void);
uint32_t paint_timings_divider_clk90_textedit(void);
uint32_t paint_timings_multiplier_textedit(void);
uint32_t paint_timings_divider_textedit(void);

void tab_change(Tab *t, uint8_t select);
void tab_repaint(Tab *t);

void recalculate_coords_tabs(void);
#endif // _TABS_H_
