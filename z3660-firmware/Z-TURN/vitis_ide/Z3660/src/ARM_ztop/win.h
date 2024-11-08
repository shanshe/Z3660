#ifndef _WIN_H_
#define _WIN_H_
#include <inttypes.h>
#include "ztop.h"

typedef struct ZTOP_WIN{
	int x;
	int y;
	int w;
	int h;
	int t;
} WIN;

void clear_screen(void);
void show_ztop(void);
void show_ztop(void);
void init_win(void);
void calculate_drag(int delta_x, int delta_y);
void win_run(void);
void win_actions(void);
void win_repaint(void);
void load_config_to_ztop(void);
void load_config_from_ztop(void);

#endif // _WIN_H_
