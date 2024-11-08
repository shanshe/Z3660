#ifndef _TEXTEDIT_H_
#define _TEXTEDIT_H_
#include <inttypes.h>
#include "ztop.h"
#include "checkbox.h"

typedef struct {
	int x;
	int y;
	int w;
	int h;
	char text[50];
	uint8_t is_pressed;
	uint8_t b_was_at_cursor;
	void (*action)();
	Tabs tab;
	int cursor_pos;
	int inverted;
	int last_cursor_pos;
	uint8_t mac_address[6];
} TextEdit;

#define PRESET_MAX_LENGTH 31

extern TextEdit *mac_textedit;
extern TextEdit *preset_textedit[PRESET_CB_MAX];

extern sFONT *Font;

void recalculate_coords_textedits(void);
void init_textedits(void);

int8_t is_cursor_at_textedit(TextEdit *b);
void textedits_run(void);
void textedits_action(void);
void textedits_repaint(void);

#endif // _TEXTEDIT_H_
