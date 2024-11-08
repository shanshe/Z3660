#ifndef _LIST_BUTTON_H_
#define _LIST_BUTTON_H_
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
	int selected_item;
} ListButton;

void listbuttons_run(void);
void init_listbuttons(void);
void b_list_action(void);
void list_button_repaint(void);
void recalculate_coords_list_button(void);
void list_buttons_action(void);
extern ListButton *b_list;

#endif // _LIST_BUTTON_H_
