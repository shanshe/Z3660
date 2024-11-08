#ifndef _BUTTON_QUIT_H_
#define _BUTTON_QUIT_H_
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
} ButtonQuit;

extern sFONT *Font;

#define BUTTON_QUIT(x,y,w,h) do {\
	FILLRECT(   x+1, y+1, w-2, h-2, 0x006088BE); \
	FILLRECT(     x,   y,   w,   1, 0x00FFFFFF); \
	FILLRECT(     x, y+1,   1, h-1, 0x00FFFFFF); \
	FILLRECT( x+(w-4)/2, y+(h-4)/2, 5, 5, 0x00000000); \
	FILLRECT( x+(w-4)/2+1, y+(h-4)/2+1, 3, 3, 0x00FFFFFF); \
	FILLRECT(     x, y+h,  w,   1, 0x00000000); \
	FILLRECT( x+w-1, y+1,  1, h-1, 0x00000000); \
	FILLRECT(   x+w, y+1,  1, h-1, 0x00FFFFFF); \
	FILLRECT(   x+1, y+h,  w,   1, 0x00000000);}while(0)
#define BUTTON_QUIT_PRESSED(x,y,w,h) do {\
	FILLRECT(  x+1, y+1, w-2, h-2, 0x006088BE); \
	FILLRECT(     x,   y,   w,   1, 0x00000000); \
	FILLRECT(     x, y+1,   1, h-1, 0x00000000); \
	FILLRECT( x+(w-4)/2, y+(h-4)/2, 5, 5, 0x00000000); \
	FILLRECT( x+(w-4)/2+1, y+(h-4)/2+1, 3, 3, 0x00A8A8A8); \
	FILLRECT(     x, y+h,  w,   1, 0x00FFFFFF); \
	FILLRECT( x+w-1, y+1,  1, h-1, 0x00FFFFFF); \
	FILLRECT(   x+w, y+1,  1, h-1, 0x00FFFFFF); \
	FILLRECT(   x+1, y+h,  w,   1, 0x00FFFFFF);}while(0)

void recalculate_coords_button_quit(void);
void init_button_quit(void);

void paint_b_apply_screen(void);
void paint_b_apply_boot_mode(void);
void paint_b_apply_all_boot(void);
void paint_b_apply_scsi(void);
void paint_b_apply_all_scsi(void);

int8_t is_cursor_at_button_quit(ButtonQuit *b);
void button_quit_run(void);
void button_quit_action(void);
void button_quit_repaint(void);
void button_quit_paint_init(void);

#endif // _BUTTON_QUIT_H_
