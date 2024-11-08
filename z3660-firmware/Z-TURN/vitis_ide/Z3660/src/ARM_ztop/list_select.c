#include "list_select.h"
#include "tabs.h"
#include <stdlib.h>
#include "../config_file.h"

extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern const char *resolution_names[RES_NUM];
extern sFONT *Font;

ListSelect *ls_kickstart;
ListSelect *ls_kickstart_ext;
ListSelect *ls_scsi[7];
ListSelect *ls_screen_res;

int8_t is_cursor_at_listselect(ListSelect *ls)
{
   if(mousex>=ls->x && mousex<=ls->x+ls->w
   && mousey>=ls->y && mousey<=ls->y+ls->h)
   {
      return(1);
   }
   return(0);
}
void ls_run(ListSelect *ls)
{
   if( selected_tab==ls->tab && is_cursor_at_listselect(ls) )
      ls->is_pressed=1;
   else
      ls->is_pressed=0;
}
void ls_action(void *ls_)
{
   ListSelect *ls=(ListSelect *)ls_;
   if(ls->is_pressed==0)
      return;
   ls->is_pressed=0;
   ls->b_was_at_cursor=0;
   if( selected_tab==ls->tab && is_cursor_at_listselect(ls) )
   {
      LISTSEL(ls->x,ls->y,ls->w,ls->h);
      ls->action(ls);
   }
}
void ls_repaint(ListSelect *ls)
{
   if(selected_tab!=ls->tab)
      return;
   if(ls->is_pressed==0)
      return;
   uint8_t b_is_at_cursor=is_cursor_at_listselect(ls);
   ls->b_was_at_cursor=b_is_at_cursor;
}

void list_selects_run(void)
{
   ls_run(ls_screen_res);
   ls_run(ls_kickstart);
   ls_run(ls_kickstart_ext);
   for(int i=0;i<7;i++)
      ls_run(ls_scsi[i]);
}
void listselect_action(void *ls_)
{
   ListSelect *ls=(ListSelect *)ls_;
   if(ls->selected_item<0)
      ls->selected_item=0;
   ls->selected_item++;
   if(ls->selected_item>=ls->num_items)
      ls->selected_item=0;
   displayStringAt(Font,ls->x+18,ls->y+2,(uint8_t *)ls->text[ls->selected_item],LEFT_MODE);
}


void init_listselects(void)
{
   ls_screen_res=(ListSelect *)malloc(sizeof(ListSelect));
   ls_screen_res->w=84; // 1920x1080 9*8=72 + 12
   ls_screen_res->h=Font->Height+2;
   ls_screen_res->is_pressed=0;
   ls_screen_res->b_was_at_cursor=0;
   ls_screen_res->action=listselect_action;
   ls_screen_res->tab=TAB_INFO;
   for(int i=0;i<RES_NUM;i++)
      strcpy(ls_screen_res->text[i],resolution_names[i]);
   ls_screen_res->num_items=RES_NUM;

   ls_kickstart=(ListSelect *)malloc(sizeof(ListSelect));
   ls_kickstart->w=win.w-100;
   ls_kickstart->h=Font->Height+2;
   ls_kickstart->is_pressed=0;
   ls_kickstart->b_was_at_cursor=0;
   ls_kickstart->action=listselect_action;
   ls_kickstart->tab=TAB_BOOT;
   strcpy(ls_kickstart->text[0],"Amiga mother board kickstart");
   strcpy(ls_kickstart->text[1],config.kickstart1+3); // +3 skips "1:/"
   strcpy(ls_kickstart->text[2],config.kickstart2+3);
   strcpy(ls_kickstart->text[3],config.kickstart3+3);
   strcpy(ls_kickstart->text[4],config.kickstart4+3);
   strcpy(ls_kickstart->text[5],config.kickstart5+3);
   strcpy(ls_kickstart->text[6],config.kickstart6+3);
   strcpy(ls_kickstart->text[7],config.kickstart7+3);
   strcpy(ls_kickstart->text[8],config.kickstart8+3);
   strcpy(ls_kickstart->text[9],config.kickstart9+3);
   for(int i=0;i<10;i++)
      ls_kickstart->text[i][43]=0;
   ls_kickstart->num_items=0;
   while((ls_kickstart->text[ls_kickstart->num_items])[0]!=0)
   {
      ls_kickstart->num_items++;
   }

   ls_kickstart_ext=(ListSelect *)malloc(sizeof(ListSelect));
   ls_kickstart_ext->w=win.w-100;
   ls_kickstart_ext->h=Font->Height+2;
   ls_kickstart_ext->is_pressed=0;
   ls_kickstart_ext->b_was_at_cursor=0;
   ls_kickstart_ext->action=listselect_action;
   ls_kickstart_ext->tab=TAB_BOOT;
   strcpy(ls_kickstart_ext->text[0],"Amiga mother board ext kickstart");
   strcpy(ls_kickstart_ext->text[1],config.ext_kickstart1+3); // +3 skips "1:/"
   strcpy(ls_kickstart_ext->text[2],config.ext_kickstart2+3);
   strcpy(ls_kickstart_ext->text[3],config.ext_kickstart3+3);
   strcpy(ls_kickstart_ext->text[4],config.ext_kickstart4+3);
   strcpy(ls_kickstart_ext->text[5],config.ext_kickstart5+3);
   strcpy(ls_kickstart_ext->text[6],config.ext_kickstart6+3);
   strcpy(ls_kickstart_ext->text[7],config.ext_kickstart7+3);
   strcpy(ls_kickstart_ext->text[8],config.ext_kickstart8+3);
   strcpy(ls_kickstart_ext->text[9],config.ext_kickstart9+3);
   for(int i=0;i<10;i++)
      ls_kickstart_ext->text[i][43]=0; // limit to 43 chars
   ls_kickstart_ext->num_items=0;
   while(ls_kickstart_ext->text[ls_kickstart_ext->num_items][0]!=0)
   {
      ls_kickstart_ext->num_items++;
   }

   for(int i=0;i<7;i++)
   {
      ls_scsi[i]=(ListSelect *)malloc(sizeof(ListSelect));
      ls_scsi[i]->w=win.w-68;
      ls_scsi[i]->h=Font->Height+2;
      ls_scsi[i]->is_pressed=0;
      ls_scsi[i]->b_was_at_cursor=0;
      ls_scsi[i]->action=listselect_action;
      ls_scsi[i]->tab=TAB_SCSI;
      strcpy(ls_scsi[i]->text[0],"Disabled");
      for(int j=1;j<21;j++)
         strcpy(ls_scsi[i]->text[j],config.hdf[j-1]+3); // +3 skips "1:/"
      for(int j=0;j<21;j++)
         ls_scsi[i]->text[j][48]=0;
      ls_scsi[i]->num_items=0;
      while((ls_scsi[i]->text[ls_scsi[i]->num_items])[0]!=0)
      {
         ls_scsi[i]->num_items++;
      }
   }
}
void list_select_repaint(void)
{
   ls_repaint(ls_screen_res);
   ls_repaint(ls_kickstart);
   ls_repaint(ls_kickstart_ext);
   for(int i=0;i<7;i++)
      ls_repaint(ls_scsi[i]);
}
void paint_ls_scsi(void)
{
   char temp[10]="SCSIX";

   for(int i=0;i<7;i++)
   {
      temp[4]=i+'0';
      displayStringAt(Font,win.x+12,win.y+win.t+11+tab_h+17*i,(uint8_t*)temp,LEFT_MODE);
      LISTSEL(ls_scsi[i]->x,ls_scsi[i]->y,ls_scsi[i]->w,ls_scsi[i]->h);
      displayStringAt(Font,ls_scsi[i]->x+18,ls_scsi[i]->y+2,(uint8_t *)ls_scsi[i]->text[ls_scsi[i]->selected_item],LEFT_MODE);
   }

}
void recalculate_coords_list_select(void)
{
   ls_screen_res->x=win.x+124;
   ls_screen_res->y=win.y+win.t+tab_h+10+12+5+20+50+10;

   ls_kickstart->x=win.x+84;
   ls_kickstart->y=win.y+win.t+tab_h+10+12+5+20+50+16+16;

   ls_kickstart_ext->x=win.x+84;
   ls_kickstart_ext->y=win.y+win.t+tab_h+10+12+5+20+50+17+16+16;

   for(int i=0;i<7;i++)
   {
      ls_scsi[i]->x=win.x+50;
      ls_scsi[i]->y=win.y+win.t+8+tab_h+17*i;
   }
}
void paint_ls_kickstart(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+tab_h+10+12+2+25+50+16+16   ,(uint8_t*)"Kickstart ",LEFT_MODE);
   LISTSEL(ls_kickstart->x,ls_kickstart->y,ls_kickstart->w,ls_kickstart->h);
   displayStringAt(Font,ls_kickstart->x+18,ls_kickstart->y+2,(uint8_t *)ls_kickstart->text[ls_kickstart->selected_item],LEFT_MODE);
}
void paint_ls_kickstart_ext(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+tab_h+10+12+2+25+50+16+16+17,(uint8_t*)"Ext Kicks.",LEFT_MODE);
   LISTSEL(ls_kickstart_ext->x,ls_kickstart_ext->y,ls_kickstart_ext->w,ls_kickstart_ext->h);
   displayStringAt(Font,ls_kickstart_ext->x+18,ls_kickstart_ext->y+2,(uint8_t *)ls_kickstart_ext->text[ls_kickstart_ext->selected_item],LEFT_MODE);
}
void paint_ls_screen(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+tab_h+10+12+2+25+50+10,(uint8_t*)"Boot Screen Res",LEFT_MODE);
   LISTSEL(ls_screen_res->x,ls_screen_res->y,ls_screen_res->w,ls_screen_res->h);
   displayStringAt(Font,ls_screen_res->x+18,ls_screen_res->y+2,(uint8_t *)ls_screen_res->text[ls_screen_res->selected_item],LEFT_MODE);
}

void list_select_action(ListSelect *b)
{
   if(b->is_pressed==0)
      return;
   b->is_pressed=0;
   b->b_was_at_cursor=0;
   if( selected_tab==b->tab && is_cursor_at_listselect(b) )
   {
      LISTSEL(b->x,b->y,b->w,b->h);
      b->action(b);
   }
}

void list_selects_action(void)
{
   list_select_action(ls_screen_res);
   list_select_action(ls_kickstart);
   list_select_action(ls_kickstart_ext);
   for(int i=0;i<7;i++)
      list_select_action(ls_scsi[i]);
}
