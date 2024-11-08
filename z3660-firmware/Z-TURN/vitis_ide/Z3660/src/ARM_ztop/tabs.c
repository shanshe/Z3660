#include "tabs.h"
#include "../config_file.h"
#include <stdio.h>
#include "button.h"
#include "list_button.h"
#include "list_select.h"
#include "checkbox.h"
#include "slider.h"

#include <stdlib.h>

Tabs selected_tab=TAB_INFO;
extern int16_t mousex,mousey;
extern WIN win;

int8_t is_cursor_at_tab(Tab *t)
{
   if(mousex>=t->x && mousex<=t->x+t->w
   && mousey>=t->y && mousey<=t->y+t->h)
   {
      return(1);
   }
   return(0);
}
void tab_run(Tab *t)
{
   if( is_cursor_at_tab(t) )
      t->is_pressed=1;
   else
      t->is_pressed=0;
}

Tab *t_info;
void paint_tab_info(void);
void t_info_action(void)
{
   paint_tab_info();
}
void init_tab_info(void)
{
   t_info=(Tab *)malloc(sizeof(Tab));
   t_info->w=tab_w;
   t_info->h=tab_h;
   t_info->text="Info";
   t_info->is_pressed=0;
   t_info->t_was_at_cursor=0;
   t_info->action=t_info_action;
   t_info->tab=TAB_INFO;
}

Tab *t_boot;
void paint_tab_boot(void);
void t_boot_action(void)
{
   paint_tab_boot();
}
void init_tab_boot(void)
{
   t_boot=(Tab *)malloc(sizeof(Tab));
   t_boot->w=tab_w;
   t_boot->h=tab_h;
   t_boot->text="Boot";
   t_boot->is_pressed=0;
   t_boot->t_was_at_cursor=0;
   t_boot->action=t_boot_action;
   t_boot->tab=TAB_BOOT;
}

Tab *t_scsi;
void paint_tab_scsi(void);
void t_scsi_action(void)
{
   paint_tab_scsi();
}
void init_tab_scsi(void)
{
   t_scsi=(Tab *)malloc(sizeof(Tab));
   t_scsi->w=tab_w;
   t_scsi->h=tab_h;
   t_scsi->text="SCSI";
   t_scsi->is_pressed=0;
   t_scsi->t_was_at_cursor=0;
   t_scsi->action=t_scsi_action;
   t_scsi->tab=TAB_SCSI;
}

Tab *t_misc;
void paint_tab_misc(void);
void t_misc_action(void)
{
   paint_tab_misc();
}
void init_tab_misc(void)
{
   t_misc=malloc(sizeof(Tab));
   t_misc->w=tab_w;
   t_misc->h=tab_h;
   t_misc->text="Misc";
   t_misc->is_pressed=0;
   t_misc->t_was_at_cursor=0;
   t_misc->action=t_misc_action;
   t_misc->tab=TAB_MISC;
}
Tab *t_preset;
void paint_tab_preset(void);
void t_preset_action(void)
{
   paint_tab_preset();
}
void init_tab_preset(void)
{
   t_preset=malloc(sizeof(Tab));
   t_preset->w=tab_w+12;
   t_preset->h=tab_h;
   t_preset->text="Preset";
   t_preset->is_pressed=0;
   t_preset->t_was_at_cursor=0;
   t_preset->action=t_preset_action;
   t_preset->tab=TAB_PRESET;
}

void init_tabs(void)
{
   init_tab_info();
   init_tab_boot();
   init_tab_scsi();
   init_tab_misc();
   init_tab_preset();

   selected_tab=TAB_INFO;
}

void tab_action(Tab *t)
{
   if(t->is_pressed==0)
      return;
   t->is_pressed=0;
   t->t_was_at_cursor=0;
   if( is_cursor_at_tab(t) )
   {
      TAB(t,1);
      if(t==t_info)
      {
         selected_tab=TAB_INFO;
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_boot)
      {
         selected_tab=TAB_BOOT;
         TAB(t_info,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_scsi)
      {
         selected_tab=TAB_SCSI;
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_misc)
      {
         selected_tab=TAB_MISC;
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_preset ,0);
      }
      else if(t==t_preset)
      {
         selected_tab=TAB_PRESET;
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
      }
      t->action();
   }
}
void tab_repaint(Tab *t)
{
   if(t->is_pressed==0)
      return;
   uint8_t t_is_at_cursor=is_cursor_at_tab(t);
   if(t_is_at_cursor && t->t_was_at_cursor==0)
   {
      TAB(t,1);
      if(t==t_info)
      {
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_boot)
      {
         TAB(t_info,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_scsi)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(t==t_misc)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_preset ,0);
      }
      else if(t==t_preset)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
      }
   }
   else if(t_is_at_cursor==0 && t->t_was_at_cursor)
   {
      if(selected_tab==TAB_INFO)
      {
         TAB(t_info,1);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(selected_tab==TAB_BOOT)
      {
         TAB(t_info,0);
         TAB(t_boot,1);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(selected_tab==TAB_SCSI)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,1);
         TAB(t_misc ,0);
         TAB(t_preset ,0);
      }
      else if(selected_tab==TAB_MISC)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,1);
         TAB(t_preset ,0);
      }
      else if(selected_tab==TAB_PRESET)
      {
         TAB(t_info,0);
         TAB(t_boot,0);
         TAB(t_scsi,0);
         TAB(t_misc ,0);
         TAB(t_preset ,1);
      }
   }
   t->t_was_at_cursor=t_is_at_cursor;
}

void tabs_paint_init(void)
{
   TAB(t_info,1);
   TAB(t_boot,0);
   TAB(t_scsi,0);
   TAB(t_misc ,0);
   TAB(t_preset ,0);
}
void tabs_run(void)
{
   tab_run(t_info);
   tab_run(t_boot);
   tab_run(t_scsi);
   tab_run(t_misc);
   tab_run(t_preset);
}
void tabs_action(void)
{
   tab_action(t_info);
   tab_action(t_boot);
   tab_action(t_scsi);
   tab_action(t_misc);
   tab_action(t_preset);
}
void tabs_repaint(void)
{
   tab_repaint(t_info);
   tab_repaint(t_boot);
   tab_repaint(t_scsi);
   tab_repaint(t_misc);
   tab_repaint(t_preset);
}
void recalculate_coords_tabs(void)
{
   t_info->x=win.x+7;
   t_info->y=win.y+win.t+2;

   t_boot->x=win.x+7+52;
   t_boot->y=win.y+win.t+2;

   t_scsi->x=win.x+7+52+52;
   t_scsi->y=win.y+win.t+2;

   t_misc->x=win.x+7+52+52+52;
   t_misc->y=win.y+win.t+2;

   t_preset->x=win.x+7+52+52+52+52;
   t_preset->y=win.y+win.t+2;
}
void clear_tab(void)
{
   FILLRECT(  win.x+8, win.y+win.t+tab_h+4, win.w-16, win.h-win.t-tab_h-11, 0x00A8A8A8);
}
char labels1[5][5]={
      "",
      "",
      "",
      "",
      "",
};
char labels2[4][25]={
      "",
      "",
      "",
      "",
};

void paint_tab_info(void)
{
   clear_tab();
   Font->BackColor=0x00A8A8A8;
   Font->TextColor=0x00000000;

   char text_labels1[5][25]={
         " Firmware Version",
         "  Aux Voltage (V)",
         " Core Voltage (V)",
         "LTC (3V3) Vdd (V)",
         " LTC (5V) Vcc (V)",
   };
   char text_labels2[4][25]={
         "CPU Frequency (MHz)",
         " FPGA Core Temp (C)",
         "       LTC Temp (C)",
         "LTC (060 THERM) (C)",
   };
   for(int j=0;j<5;j++)
   {
      displayStringAt(Font,win.x+12,win.y+win.t+tab_h+10+j*(Font->Height+5),(uint8_t*)text_labels1[j],LEFT_MODE);
      TEXTFIELD(win.x+10+125,win.y+win.t+tab_h+8+j*(Font->Height+5),50,15,(uint8_t*) labels1[j]);
   }
   for(int j=0;j<4;j++)
   {
      displayStringAt(Font,win.x+200,win.y+win.t+tab_h+10+j*(Font->Height+5),(uint8_t*)text_labels2[j],LEFT_MODE);
      TEXTFIELD(win.x+10+200+125,win.y+win.t+tab_h+8+j*(Font->Height+5),50,15,(uint8_t*) labels2[j]);
   }
//   if(b_refresh.tab==1) // hide this button as it is now auto refreshing
//      BUTTON(b_refresh.x,b_refresh.y,b_refresh.w,b_refresh.h,b_refresh.text);

   paint_b_apply_screen();

   paint_ls_screen();

}
void paint_tab_boot(void)
{
   clear_tab();
   paint_slider_cpufreq();

   TEXTFIELD(win.x+10+38,win.y+win.t+tab_h+8+(Font->Height+3),120,(Font->Height+1)*4+3,"");
   b_list_action();
   paint_b_apply_boot_mode();
   paint_b_apply_all_boot();

   displayStringAt(Font,win.x+224,win.y+win.t+tab_h+10+12+2+25   ,(uint8_t*)"SCSI BOOT enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+tab_h+10+12+2+25+16,(uint8_t*)"AUTOC RAM enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+tab_h+10+12+2+25+32,(uint8_t*)"AUTOC RTG enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+tab_h+10+12+2+25+48,(uint8_t*)"     TEST enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+tab_h+10+12+2+25+64,(uint8_t*)"  CPU RAM enabled",LEFT_MODE);

   paint_checkboxes();

   paint_ls_kickstart();
   paint_ls_kickstart_ext();
}
void paint_tab_scsi(void)
{
   clear_tab();

   paint_ls_scsi();
   paint_b_apply_scsi();
   paint_b_apply_all_scsi();
}
extern char message[300];

void paint_tab_misc(void)
{
   clear_tab();
   paint_mac_textedit();
   paint_slider_bpton();
   paint_slider_bptoff();
   paint_b_apply_misc();
   paint_b_apply_all_misc();
}
void paint_tab_preset(void)
{
   clear_tab();
   paint_checkboxes();
   paint_preset_textedit();
   paint_b_apply_preset();
   paint_b_apply_all_preset();
   paint_b_delete_preset();
}
