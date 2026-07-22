#ifndef _LIST_SELECT_H_
#define _LIST_SELECT_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
	int x;
	int y;
	int w;
	int h;
	char text[21][150];
	uint8_t is_pressed;
	uint8_t b_was_at_cursor;
	void (*action)(void *ls);
	Tabs tab;
	int selected_item;
	int num_items;
} ListSelect;

enum TIMINGS{
   T50,
   T55,
   T60,
   T65,
   T70,
   T75,
   T80,
   T85,
   T90,
   T95,
   T100,
   T105,
   T110,
   T115,
   T120,
   TIMINGS_NUM
};

extern ListSelect *ls_kickstart;
extern ListSelect *ls_kickstart_ext;
extern ListSelect *ls_scsi[7];
extern ListSelect *ls_adf[8];
extern ListSelect *ls_screen_res;
extern ListSelect *ls_timings;
extern ListSelect *ls_arm_frequency;

#define LISTSEL(x,y,w,h) do {\
	FILLRECT(   x+3, y+2, w-6, h-2, 0x00A8A8A8); \
	FILLRECT(     x,   y,   w,   1, 0x00FFFFFF); \
	FILLRECT(     x, y+1,   1, h-1, 0x00FFFFFF); \
	FILLRECT(   x+1, y+1,   1, h-2, 0x00FFFFFF); \
	Font->BackColor=0x00A8A8A8; \
	Font->TextColor=0x00000000; \
	displayStringAt(Font,x+5,y+1,(uint8_t*)"\x80",LEFT_MODE); \
	FILLRECT(  x+14, y+2,   1, h-4, 0x00000000); \
	FILLRECT(  x+15, y+2,   1, h-4, 0x00FFFFFF); \
	FILLRECT(   x+w,   y,   1,   h, 0x00000000); \
	FILLRECT( x+w-1, y+1,   1, h-1, 0x00000000); \
	FILLRECT(   x+1, y+h,   w,   1, 0x00000000);}while(0)

void list_selects_run(void);
void init_listselects(void);
void list_select_repaint(void);
void list_selects_action(void);
void paint_ls_scsi(void);
void paint_ls_adf(void);
void paint_ls_kickstart_ext(void);
void recalculate_coords_list_select(void);
void paint_ls_kickstart(void);
void paint_ls_screen_res(void);
void paint_ls_timings(void);
void paint_ls_arm_frequency(void);

#endif // _LIST_SELECT_H_
