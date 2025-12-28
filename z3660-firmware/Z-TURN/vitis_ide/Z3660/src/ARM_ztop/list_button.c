#include "list_button.h"
#include "tabs.h"
#include <stdlib.h>
#include "../config_file.h"

extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern sFONT *Font;

int8_t is_cursor_at_list(ListButton *b)
{
   if(mousex>=b->x && mousex<=b->x+b->w
         && mousey>=b->y && mousey<=b->y+b->h)
   {
      return(1);
   }
   return(0);
}
void listbutton_run(ListButton *b)
{
   if( selected_tab==b->tab && is_cursor_at_list(b) )
      b->is_pressed=1;
   else
      b->is_pressed=0;
}

ListButton *b_list_emu;
void b_list_emu_action(void)
{
   char boot_mode_labels[NUM_BOOT_MODE_LABELS][25]={
         "060 real CPU    ",
         "030 MUSASHI emu ",
         "030 UAE emu     ",
         "030 UAE JIT emu ",
         "040 UAE emu     ",
         "040 UAE JIT emu ",
   };
   for(int i=0;i<NUM_BOOT_MODE_LABELS;i++)
   {
      if(i==b_list_emu->selected_item)
         Font->BackColor=0x006088BE;
      else
         Font->BackColor=0x00A8A8A8;
      displayStringAt(Font,win.x+14+38,win.y+win.t+TAB_HEIGHT+8+(Font->Height+3)+3+(Font->Height+1)*i,(uint8_t*)boot_mode_labels[i],LEFT_MODE);
   }
}

void list_action(ListButton *b)
{
   if(b->is_pressed==0)
      return;
   b->is_pressed=0;
   b->b_was_at_cursor=0;
   if( selected_tab==b->tab && is_cursor_at_list(b) )
   {
      b->selected_item=(mousey-b->y-3)/(b->h/NUM_BOOT_MODE_LABELS);
      if(b->selected_item>NUM_BOOT_MODE_LABELS-1) b->selected_item=NUM_BOOT_MODE_LABELS-1;
      b->action();
   }
}
void list_repaint(ListButton *b)
{
   if(selected_tab!=b->tab)
      return;
   if(b->is_pressed==0)
      return;
   uint8_t b_is_at_cursor=is_cursor_at_list(b);
   b->b_was_at_cursor=b_is_at_cursor;
}
void listbuttons_run(void)
{
   listbutton_run(b_list_emu);
}
void init_listbuttons(void)
{
   b_list_emu=(ListButton *)malloc(sizeof(ListButton));
   b_list_emu->w=120;
   b_list_emu->h=(Font->Height+1)*NUM_BOOT_MODE_LABELS+3;
   b_list_emu->text="";
   b_list_emu->is_pressed=0;
   b_list_emu->b_was_at_cursor=0;
   b_list_emu->action=b_list_emu_action;
   b_list_emu->tab=TAB_BOOT;
}
void list_button_repaint(void)
{
   list_repaint(b_list_emu);
}
void recalculate_coords_list_button(void)
{
   b_list_emu->x=win.x+10+38;
   b_list_emu->y=win.y+win.t+TAB_HEIGHT+8+(Font->Height+3);
}
void list_buttons_action(void)
{
   list_action(b_list_emu);
}

