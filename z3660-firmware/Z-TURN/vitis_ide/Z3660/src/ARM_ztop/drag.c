#include "drag.h"
#include <stdlib.h>
extern int16_t mousex,mousey;
extern WIN win;

Drag drag;
void recalculate_coords_drag(void)
{
	drag.x=win.x+19;
	drag.y=win.y+1;
}
int8_t is_cursor_at_drag(Drag *d)
{
	if(mousex>=d->x && mousex<=d->x+d->w
	&& mousey>=d->y && mousey<=d->y+d->h)
	{
		return(1);
	}
	return(0);
}
void init_drag(void)
{
	drag.w=win.w-2-19;
	drag.h=win.t-1;
}
void drag_run(void)
{
	if( is_cursor_at_drag(&drag) )
	{
		drag.is_pressed=1;
		drag.mousex_pre=mousex;
		drag.mousey_pre=mousey;
		drag.win_x_pre=win.x;
		drag.win_y_pre=win.y;
		drag.is_dragging=1;
	}
	else
	{
		drag.is_pressed=0;
		drag.is_dragging=0;
	}
}
void drag_action(void)
{
	if(drag.is_pressed==0)
		return;
	drag.is_pressed=0;
	drag.b_was_at_cursor=0;
	if( is_cursor_at_drag(&drag) )
	{
//		BUTTON(drag.x,drag.y,drag.w,drag.h,drag.text);
	}
}
void drag_repaint(void)
{
	if(drag.is_pressed==0)
		return;
	uint8_t b_is_at_cursor=is_cursor_at_drag(&drag);
	if(b_is_at_cursor && drag.b_was_at_cursor==0)
	{
//		BUTTON_PRESSED(b->x,b->y,b->w,b->h,b->text);
	}
	else if(b_is_at_cursor==0 && drag.b_was_at_cursor)
	{
//		BUTTON(b->x,b->y,b->w,b->h,b->text);
	}
	drag.b_was_at_cursor=b_is_at_cursor;
}
uint8_t is_dragging(void)
{
	return(drag.is_dragging);
}
int get_drag_mousex_pre(void)
{
	return(drag.mousex_pre);
}
int get_drag_mousey_pre(void)
{
	return(drag.mousey_pre);
}
int get_drag_win_x_pre(void)
{
	return(drag.win_x_pre);
}
int get_drag_win_y_pre(void)
{
	return(drag.win_y_pre);
}
