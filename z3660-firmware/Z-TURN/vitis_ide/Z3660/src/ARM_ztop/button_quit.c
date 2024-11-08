#include "button_quit.h"
#include <stdlib.h>
#include <stdio.h>
extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern int v_major;
extern int v_minor;
extern int beta;
extern int alfa;

int8_t is_cursor_at_button_quit(ButtonQuit *b)
{
	if(mousex>=b->x && mousex<=b->x+b->w
	&& mousey>=b->y && mousey<=b->y+b->h)
	{
		return(1);
	}
	return(0);
}
ButtonQuit *b_quit;

void recalculate_coords_button_quit(void)
{
	b_quit->x=win.x;
	b_quit->y=win.y;
}
void hard_reboot(void);
void reboot(void);
void b_quit_action(void)
{
	reboot();
}

void init_button_quit(void)
{
	b_quit=(ButtonQuit *)malloc(sizeof(ButtonQuit));
	b_quit->w=18;
	b_quit->h=win.t;
	b_quit->text="";
	b_quit->is_pressed=0;
	b_quit->b_was_at_cursor=0;
	b_quit->action=b_quit_action;
}
void button_quit_paint_init(void)
{
	BUTTON_QUIT(b_quit->x,b_quit->y,b_quit->w,b_quit->h);
}

void button_quit_run(void)
{
	if( is_cursor_at_button_quit(b_quit) )
		b_quit->is_pressed=1;
	else
		b_quit->is_pressed=0;
}

void button_quit_action(void)
{
	if(b_quit->is_pressed==0)
		return;
	b_quit->is_pressed=0;
	b_quit->b_was_at_cursor=0;
	if( is_cursor_at_button_quit(b_quit) )
	{
		BUTTON_QUIT(b_quit->x,b_quit->y,b_quit->w,b_quit->h);
		b_quit->action();
	}
}
void button_quit_repaint(void)
{
	if(b_quit->is_pressed==0)
		return;
	uint8_t b_is_at_cursor=is_cursor_at_button_quit(b_quit);
	if(b_is_at_cursor && b_quit->b_was_at_cursor==0)
		BUTTON_QUIT_PRESSED(b_quit->x,b_quit->y,b_quit->w,b_quit->h);
	else if(b_is_at_cursor==0 && b_quit->b_was_at_cursor)
		BUTTON_QUIT(b_quit->x,b_quit->y,b_quit->w,b_quit->h);
	b_quit->b_was_at_cursor=b_is_at_cursor;
}

