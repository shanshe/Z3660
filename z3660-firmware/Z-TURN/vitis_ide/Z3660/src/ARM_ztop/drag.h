#ifndef _DRAG_H_
#define _DRAG_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct {
	int x;
	int y;
	int w;
	int h;
	uint8_t is_pressed;
	uint8_t b_was_at_cursor;
	int mousex_pre;
	int mousey_pre;
	int win_x_pre;
	int win_y_pre;
	uint8_t is_dragging;
} Drag;

int8_t is_cursor_at_drag(Drag *d);
void init_drag(void);
void drag_run(void);
void drag_action(void);
void drag_repaint(void);
uint8_t is_dragging(void);
int get_drag_mousex_pre(void);
int get_drag_mousey_pre(void);
int get_drag_win_x_pre(void);
int get_drag_win_y_pre(void);
void recalculate_coords_drag(void);

#endif // _DRAG_H_
