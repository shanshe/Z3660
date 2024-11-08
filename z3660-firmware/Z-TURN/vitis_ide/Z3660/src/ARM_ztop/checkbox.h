#ifndef _CHECKBOX_H_
#define _CHECKBOX_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
	int x;
	int y;
	int w;
	int h;
	int checked;
	uint8_t is_pressed;
	uint8_t b_was_at_cursor;
	void (*action)(void *cb);
	Tabs tab;
} CheckBox;

#define PRESET_CB_MAX 9

extern CheckBox *cb_scsi_boot;
extern CheckBox *cb_autoc_ram;
extern CheckBox *cb_autoc_rtg;
extern CheckBox *cb_test;
extern CheckBox *cb_cpuram;
extern CheckBox *cb_preset[PRESET_CB_MAX];

#define CHECKBOX(t,checked) do {\
	FILLRECT( t->x+3, t->y+2, t->w-6, t->h-2, 0x00A8A8A8); \
	FILLRECT(   t->x,   t->y,   t->w,      1, 0x00FFFFFF); \
	FILLRECT(   t->x, t->y+1,      1, t->h-1, 0x00FFFFFF); \
	FILLRECT( t->x+1, t->y+1,      1, t->h-2, 0x00FFFFFF); \
	Font->BackColor=0x00A8A8A8; \
	Font->TextColor=0x00000000; \
	if(checked) \
	displayStringAt(Font,t->x+4,t->y+1,(uint8_t*)"\x7F",LEFT_MODE); \
	FILLRECT(   t->x+t->w,      t->y,    1,   t->h, 0x00000000); \
	FILLRECT( t->x+t->w-1,    t->y+1,    1, t->h-1, 0x00000000); \
	FILLRECT(      t->x+1, t->y+t->h, t->w,      1, 0x00000000);}while(0)

void checkboxes_run(void);
void init_checkboxes(void);
void checkboxes_repaint(void);
void paint_checkboxes(void);
void recalculate_coords_checkboxes(void);
void checkboxes_action(void);


#endif // _CHECKBOX_H_
