#include "list_select.h"
#include "tabs.h"
#include <stdlib.h>
#include "../config_file.h"

extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern const char *resolution_names[RES_NUM];
extern sFONT *Font;
void textedits_timings_reload(void);
extern int timing_selected;

ListSelect *ls_kickstart;
ListSelect *ls_kickstart_ext;
ListSelect *ls_scsi[7];
ListSelect *ls_adf[8];
ListSelect *ls_screen_res;
ListSelect *ls_timings;
ListSelect *ls_arm_frequency;

const char *timings_names[TIMINGS_NUM] = {
      " 50 MHz",
      " 55 MHz",
      " 60 MHz",
      " 65 MHz",
      " 70 MHz",
      " 75 MHz",
      " 80 MHz",
      " 85 MHz",
      " 90 MHz",
      " 95 MHz",
      "100 MHz",
      "105 MHz",
      "110 MHz",
      "115 MHz",
      "120 MHz",
};


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
   for(int i=0;i<8;i++)
      ls_run(ls_adf[i]);
   ls_run(ls_timings);
   ls_run(ls_arm_frequency);
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
void listselect_timings_action(void *ls_)
{
   ListSelect *ls=(ListSelect *)ls_;
   if(ls->selected_item<0)
      ls->selected_item=0;
   ls->selected_item++;
   if(ls->selected_item>=ls->num_items)
      ls->selected_item=0;
   displayStringAt(Font,ls->x+18,ls->y+2,(uint8_t *)ls->text[ls->selected_item],LEFT_MODE);
   timing_selected=ls->selected_item;
   textedits_timings_reload();
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
   for(int i=0;i<21;i++)
      strcpy(ls_kickstart->text[i],"");
   strcpy(ls_kickstart->text[0],"Amiga mother board kickstart");
   if(config.kickstart1[0]!=0) strcpy(ls_kickstart->text[1],config.kickstart1+3); // +3 skips "1:/"
   if(config.kickstart2[0]!=0) strcpy(ls_kickstart->text[2],config.kickstart2+3);
   if(config.kickstart3[0]!=0) strcpy(ls_kickstart->text[3],config.kickstart3+3);
   if(config.kickstart4[0]!=0) strcpy(ls_kickstart->text[4],config.kickstart4+3);
   if(config.kickstart5[0]!=0) strcpy(ls_kickstart->text[5],config.kickstart5+3);
   if(config.kickstart6[0]!=0) strcpy(ls_kickstart->text[6],config.kickstart6+3);
   if(config.kickstart7[0]!=0) strcpy(ls_kickstart->text[7],config.kickstart7+3);
   if(config.kickstart8[0]!=0) strcpy(ls_kickstart->text[8],config.kickstart8+3);
   if(config.kickstart9[0]!=0) strcpy(ls_kickstart->text[9],config.kickstart9+3);
   // cut the text to 43 chars, because the listselect is not wide enough to display more than 43 chars
   for(int i=0;i<21;i++)
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
   for(int i=0;i<21;i++)
      strcpy(ls_kickstart_ext->text[i],"");
   strcpy(ls_kickstart_ext->text[0],"Amiga mother board ext kickstart");
   if(config.ext_kickstart1[0]!=0) strcpy(ls_kickstart_ext->text[1],config.ext_kickstart1+3); // +3 skips "1:/"
   if(config.ext_kickstart2[0]!=0) strcpy(ls_kickstart_ext->text[2],config.ext_kickstart2+3);
   if(config.ext_kickstart3[0]!=0) strcpy(ls_kickstart_ext->text[3],config.ext_kickstart3+3);
   if(config.ext_kickstart4[0]!=0) strcpy(ls_kickstart_ext->text[4],config.ext_kickstart4+3);
   if(config.ext_kickstart5[0]!=0) strcpy(ls_kickstart_ext->text[5],config.ext_kickstart5+3);
   if(config.ext_kickstart6[0]!=0) strcpy(ls_kickstart_ext->text[6],config.ext_kickstart6+3);
   if(config.ext_kickstart7[0]!=0) strcpy(ls_kickstart_ext->text[7],config.ext_kickstart7+3);
   if(config.ext_kickstart8[0]!=0) strcpy(ls_kickstart_ext->text[8],config.ext_kickstart8+3);
   if(config.ext_kickstart9[0]!=0) strcpy(ls_kickstart_ext->text[9],config.ext_kickstart9+3);
   // cut the text to 43 chars, because the listselect is not wide enough to display more than 43 chars
   for(int i=0;i<21;i++)
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

   for(int i=0;i<8;i++)
   {
      ls_adf[i]=(ListSelect *)malloc(sizeof(ListSelect));
      ls_adf[i]->w=win.w-68;
      ls_adf[i]->h=Font->Height+2;
      ls_adf[i]->is_pressed=0;
      ls_adf[i]->b_was_at_cursor=0;
      ls_adf[i]->action=listselect_action;
      ls_adf[i]->tab=TAB_ADF;
      strcpy(ls_adf[i]->text[0],"Disabled");
      for(int j=1;j<21;j++)
         strcpy(ls_adf[i]->text[j],config.adf[j-1]+3); // +3 skips "1:/"
      for(int j=0;j<21;j++)
         ls_adf[i]->text[j][48]=0;
      ls_adf[i]->num_items=0;
      while((ls_adf[i]->text[ls_adf[i]->num_items])[0]!=0)
      {
         ls_adf[i]->num_items++;
      }
   }

   ls_timings=(ListSelect *)malloc(sizeof(ListSelect));
   ls_timings->w=70; // 120 MHz 7*8=56 + 14
   ls_timings->h=Font->Height+2;
   ls_timings->is_pressed=0;
   ls_timings->b_was_at_cursor=0;
   ls_timings->action=listselect_timings_action;
   ls_timings->tab=TAB_TIMINGS;
   for(int i=0;i<TIMINGS_NUM;i++)
      strcpy(ls_timings->text[i],timings_names[i]);
   ls_timings->num_items=TIMINGS_NUM;

   ls_arm_frequency=(ListSelect *)malloc(sizeof(ListSelect));
   ls_arm_frequency->w=50; // 667 4*8=32 + 18
   ls_arm_frequency->h=Font->Height+2;
   ls_arm_frequency->is_pressed=0;
   ls_arm_frequency->b_was_at_cursor=0;
   ls_arm_frequency->action=listselect_action;
   ls_arm_frequency->tab=TAB_INFO;
   for(int i=0;i<FREQ_NUM;i++)
      strcpy(ls_arm_frequency->text[i],arm_frequency_names[i]);
   ls_arm_frequency->num_items=FREQ_NUM;

}
void list_select_repaint(void)
{
   ls_repaint(ls_screen_res);
   ls_repaint(ls_kickstart);
   ls_repaint(ls_kickstart_ext);
   for(int i=0;i<7;i++)
      ls_repaint(ls_scsi[i]);
   for(int i=0;i<8;i++)
      ls_repaint(ls_adf[i]);
   ls_repaint(ls_timings);
   ls_repaint(ls_arm_frequency);
}
void paint_ls_scsi(void)
{
   char temp[10]="SCSIX";

   for(int i=0;i<7;i++)
   {
      temp[4]=i+'0';
      displayStringAt(Font,win.x+12,win.y+win.t+11+TAB_HEIGHT+25+17*i,(uint8_t*)temp,LEFT_MODE);
      LISTSEL(ls_scsi[i]->x,ls_scsi[i]->y,ls_scsi[i]->w,ls_scsi[i]->h);
      displayStringAt(Font,ls_scsi[i]->x+18,ls_scsi[i]->y+2,(uint8_t *)ls_scsi[i]->text[ls_scsi[i]->selected_item],LEFT_MODE);
   }

}
void paint_ls_adf(void)
{
   char temp[10]="ADFX";

   for(int i=0;i<8;i++)
   {
      temp[3]=i+'0';
      displayStringAt(Font,win.x+12,win.y+win.t+11+TAB_HEIGHT+25+17*i,(uint8_t*)temp,LEFT_MODE);
      LISTSEL(ls_adf[i]->x,ls_adf[i]->y,ls_adf[i]->w,ls_adf[i]->h);
      displayStringAt(Font,ls_adf[i]->x+18,ls_adf[i]->y+2,(uint8_t *)ls_adf[i]->text[ls_adf[i]->selected_item],LEFT_MODE);
   }

}
void recalculate_coords_list_select(void)
{
   ls_screen_res->x=win.x+124;
   ls_screen_res->y=win.y+win.t+TAB_HEIGHT+10+12+5+20+50+10;

   ls_kickstart->x=win.x+84;
   ls_kickstart->y=win.y+win.t+TAB_HEIGHT+10+12+5+20+50+16+16+16+16;

   ls_kickstart_ext->x=win.x+84;
   ls_kickstart_ext->y=win.y+win.t+TAB_HEIGHT+10+12+5+20+50+17+16+16+16+16;

   for(int i=0;i<7;i++)
   {
      ls_scsi[i]->x=win.x+50;
      ls_scsi[i]->y=win.y+win.t+8+TAB_HEIGHT+25+17*i;
   }

   for(int i=0;i<8;i++)
   {
      ls_adf[i]->x=win.x+50;
      ls_adf[i]->y=win.y+win.t+8+TAB_HEIGHT+25+17*i;
   }

   ls_timings->x=win.x+128;
   ls_timings->y=win.y+win.t+TAB_HEIGHT+10;

   ls_arm_frequency->x=win.x+124+224-8;
   ls_arm_frequency->y=win.y+win.t+TAB_HEIGHT+10+12+5+20+50+10;
}
void paint_ls_kickstart(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+TAB_HEIGHT+10+12+2+25+50+16+16+16+16   ,(uint8_t*)"Kickstart ",LEFT_MODE);
   LISTSEL(ls_kickstart->x,ls_kickstart->y,ls_kickstart->w,ls_kickstart->h);
   displayStringAt(Font,ls_kickstart->x+18,ls_kickstart->y+2,(uint8_t *)ls_kickstart->text[ls_kickstart->selected_item],LEFT_MODE);
}
void paint_ls_kickstart_ext(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+TAB_HEIGHT+10+12+2+25+50+16+16+16+16+17,(uint8_t*)"Ext Kicks.",LEFT_MODE);
   LISTSEL(ls_kickstart_ext->x,ls_kickstart_ext->y,ls_kickstart_ext->w,ls_kickstart_ext->h);
   displayStringAt(Font,ls_kickstart_ext->x+18,ls_kickstart_ext->y+2,(uint8_t *)ls_kickstart_ext->text[ls_kickstart_ext->selected_item],LEFT_MODE);
}
void paint_ls_screen_res(void)
{
   displayStringAt(Font,win.x+12,win.y+win.t+TAB_HEIGHT+10+12+2+25+50+10,(uint8_t*)"Boot Screen Res",LEFT_MODE);
   LISTSEL(ls_screen_res->x,ls_screen_res->y,ls_screen_res->w,ls_screen_res->h);
   displayStringAt(Font,ls_screen_res->x+18,ls_screen_res->y+2,(uint8_t *)ls_screen_res->text[ls_screen_res->selected_item],LEFT_MODE);
}
void paint_ls_timings(void)
{
   displayStringAt(Font,win.x+22,win.y+win.t+TAB_HEIGHT+10,(uint8_t*)"Timing Freq.",LEFT_MODE);
   LISTSEL(ls_timings->x,ls_timings->y,ls_timings->w,ls_timings->h);
   displayStringAt(Font,ls_timings->x+18,ls_timings->y+2,(uint8_t *)ls_timings->text[ls_timings->selected_item],LEFT_MODE);
}
void paint_ls_arm_frequency(void)
{
   displayStringAt(Font,win.x+12+224,win.y+win.t+TAB_HEIGHT+10+12+2+25+50+10,(uint8_t*)"ARM freq (MHz)",LEFT_MODE);
   LISTSEL(ls_arm_frequency->x,ls_arm_frequency->y,ls_arm_frequency->w,ls_arm_frequency->h);
   displayStringAt(Font,ls_arm_frequency->x+18,ls_arm_frequency->y+2,(uint8_t *)ls_arm_frequency->text[ls_arm_frequency->selected_item],LEFT_MODE);
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
   for(int i=0;i<8;i++)
      list_select_action(ls_adf[i]);
   list_select_action(ls_timings);
   list_select_action(ls_arm_frequency);
}
