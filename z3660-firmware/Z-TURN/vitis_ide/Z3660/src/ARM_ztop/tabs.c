#include "tabs.h"
#include "../config_file.h"
#include <stdio.h>
#include "button.h"
#include "list_button.h"
#include "list_select.h"
#include "checkbox.h"
#include "slider.h"

#include <stdlib.h>

Tab *t_info;
Tab *t_boot;
Tab *t_scsi;
Tab *t_adf;
Tab *t_misc;
Tab *t_preset;
Tab *t_timings;

Tab **tabs[NUM_TABS] = {
      &t_info,
      &t_boot,
      &t_scsi,
      &t_adf,
      &t_misc,
      &t_preset,
      &t_timings,
};

Tabs selected_tab=TAB_INFO;
extern int16_t mousex,mousey;
extern WIN win;
extern Button *b_apply_timings;
extern Button *b_apply_all_timings;

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

void paint_tab_info(void);
void t_info_action(void)
{
   paint_tab_info();
}
void init_tab_info(void)
{
   t_info=(Tab *)malloc(sizeof(Tab));
   t_info->w=TAB_MIN_WIDTH;
   t_info->h=TAB_HEIGHT;
   t_info->text="Info";
   t_info->is_pressed=0;
   t_info->t_was_at_cursor=0;
   t_info->action=t_info_action;
   t_info->tab=TAB_INFO;
   t_info->is_painted=0;
}

void paint_tab_boot(void);
void t_boot_action(void)
{
   paint_tab_boot();
}
void init_tab_boot(void)
{
   t_boot=(Tab *)malloc(sizeof(Tab));
   t_boot->w=TAB_MIN_WIDTH;
   t_boot->h=TAB_HEIGHT;
   t_boot->text="Boot";
   t_boot->is_pressed=0;
   t_boot->t_was_at_cursor=0;
   t_boot->action=t_boot_action;
   t_boot->tab=TAB_BOOT;
   t_boot->is_painted=0;
}

void paint_tab_scsi(void);
void t_scsi_action(void)
{
   paint_tab_scsi();
}
void init_tab_scsi(void)
{
   t_scsi=(Tab *)malloc(sizeof(Tab));
   t_scsi->w=TAB_MIN_WIDTH;
   t_scsi->h=TAB_HEIGHT;
   t_scsi->text="SCSI";
   t_scsi->is_pressed=0;
   t_scsi->t_was_at_cursor=0;
   t_scsi->action=t_scsi_action;
   t_scsi->tab=TAB_SCSI;
   t_scsi->is_painted=0;
}

void paint_tab_adf(void);
void t_adf_action(void)
{
   paint_tab_adf();
}
void init_tab_adf(void)
{
   t_adf=(Tab *)malloc(sizeof(Tab));
   t_adf->w=TAB_MIN_WIDTH;
   t_adf->h=TAB_HEIGHT;
   t_adf->text="ADF";
   t_adf->is_pressed=0;
   t_adf->t_was_at_cursor=0;
   t_adf->action=t_adf_action;
   t_adf->tab=TAB_ADF;
   t_adf->is_painted=0;
}

void paint_tab_misc(void);
void t_misc_action(void)
{
   paint_tab_misc();
}
void init_tab_misc(void)
{
   t_misc=malloc(sizeof(Tab));
   t_misc->w=TAB_MIN_WIDTH;
   t_misc->h=TAB_HEIGHT;
   t_misc->text="Misc";
   t_misc->is_pressed=0;
   t_misc->t_was_at_cursor=0;
   t_misc->action=t_misc_action;
   t_misc->tab=TAB_MISC;
   t_misc->is_painted=0;
}

void paint_tab_preset(void);
void t_preset_action(void)
{
   paint_tab_preset();
}
void init_tab_preset(void)
{
   t_preset=malloc(sizeof(Tab));
   t_preset->w=TAB_MIN_WIDTH+12;
   t_preset->h=TAB_HEIGHT;
   t_preset->text="Preset";
   t_preset->is_pressed=0;
   t_preset->t_was_at_cursor=0;
   t_preset->action=t_preset_action;
   t_preset->tab=TAB_PRESET;
   t_preset->is_painted=0;
}

void paint_tab_timings(void);
void t_timings_action(void)
{
   paint_tab_timings();
}
void init_tab_timings(void)
{
   t_timings=malloc(sizeof(Tab));
   t_timings->w=TAB_MIN_WIDTH+20;
   t_timings->h=TAB_HEIGHT;
   t_timings->text="Timings";
   t_timings->is_pressed=0;
   t_timings->t_was_at_cursor=0;
   t_timings->action=t_timings_action;
   t_timings->tab=TAB_TIMINGS;
   t_timings->is_painted=0;
}

void init_tabs(void)
{
   init_tab_info();
   init_tab_boot();
   init_tab_scsi();
   init_tab_adf();
   init_tab_misc();
   init_tab_preset();
   init_tab_timings();

   selected_tab=TAB_INFO;
}
void tab_change(Tab *t, uint8_t select)
{
   t->is_painted=0;
   TAB(t,1);
   if(t==t_info)
   {
      if(select)
         selected_tab=TAB_INFO;
      TAB(t_boot,0);
      TAB(t_scsi,0);
      TAB(t_adf,0);
      TAB(t_misc,0);
      TAB(t_preset,0);
      TAB(t_timings,0);
   }
   else if(t==t_boot)
   {
      if(select)
         selected_tab=TAB_BOOT;
      TAB(t_info,0);
      TAB(t_scsi,0);
      TAB(t_adf,0);
      TAB(t_misc,0);
      TAB(t_preset,0);
      TAB(t_timings,0);
   }
   else if(t==t_scsi)
   {
      if(select)
         selected_tab=TAB_SCSI;
      TAB(t_info,0);
      TAB(t_boot,0);
      TAB(t_adf,0);
      TAB(t_misc,0);
      TAB(t_preset,0);
      TAB(t_timings,0);
   }
   else if(t==t_adf)
   {
      if(select)
         selected_tab=TAB_ADF;
      TAB(t_info,0);
      TAB(t_boot,0);
      TAB(t_scsi,0);
      TAB(t_misc,0);
      TAB(t_preset,0);
      TAB(t_timings,0);
   }
   else if(t==t_misc)
   {
      if(select)
         selected_tab=TAB_MISC;
      TAB(t_info,0);
      TAB(t_boot,0);
      TAB(t_scsi,0);
      TAB(t_adf,0);
      TAB(t_preset,0);
      TAB(t_timings,0);
   }
   else if(t==t_preset)
   {
      if(select)
         selected_tab=TAB_PRESET;
      TAB(t_info,0);
      TAB(t_boot,0);
      TAB(t_scsi,0);
      TAB(t_adf,0);
      TAB(t_misc,0);
      TAB(t_timings,0);
   }
   else if(t==t_timings)
   {
      if(select)
         selected_tab=TAB_TIMINGS;
      TAB(t_info,0);
      TAB(t_boot,0);
      TAB(t_scsi,0);
      TAB(t_adf,0);
      TAB(t_misc,0);
      TAB(t_preset,0);
   }
}
void tab_action(Tab *t)
{
   if(t->is_pressed==0)
      return;
   t->is_pressed=0;
   t->t_was_at_cursor=0;
   if( is_cursor_at_tab(t) )
   {
      tab_change(t,1);
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
      tab_change(t,0);
   }
   else if(t_is_at_cursor==0 && t->t_was_at_cursor)
   {
      TAB(t_info  ,selected_tab==TAB_INFO);
      TAB(t_boot  ,selected_tab==TAB_BOOT);
      TAB(t_scsi  ,selected_tab==TAB_SCSI);
      TAB(t_adf   ,selected_tab==TAB_ADF);
      TAB(t_misc  ,selected_tab==TAB_MISC);
      TAB(t_preset,selected_tab==TAB_PRESET);
      TAB(t_timings,selected_tab==TAB_TIMINGS);
   }
   t->t_was_at_cursor=t_is_at_cursor;
}

void tabs_paint_init(void)
{
   TAB(t_info,1);
   TAB(t_boot,0);
   TAB(t_scsi,0);
   TAB(t_adf,0);
   TAB(t_misc,0);
   TAB(t_preset,0);
   TAB(t_timings,0);
}
void tabs_run(void)
{
   tab_run(t_info);
   tab_run(t_boot);
   tab_run(t_scsi);
   tab_run(t_adf);
   tab_run(t_misc);
   tab_run(t_preset);
   tab_run(t_timings);
}
void tabs_action(void)
{
   tab_action(t_info);
   tab_action(t_boot);
   tab_action(t_scsi);
   tab_action(t_adf);
   tab_action(t_misc);
   tab_action(t_preset);
   tab_action(t_timings);
}
void tabs_repaint(void)
{
   tab_repaint(t_info);
   tab_repaint(t_boot);
   tab_repaint(t_scsi);
   tab_repaint(t_adf);
   tab_repaint(t_misc);
   tab_repaint(t_preset);
   tab_repaint(t_timings);
}
void recalculate_coords_tabs(void)
{
   int xpos=7;
#define T_TAB(T) T->x=win.x+xpos; xpos+=1+T->w; \
                 T->y=win.y+win.t+2
   T_TAB(t_info);
   T_TAB(t_boot);
   T_TAB(t_scsi);
   T_TAB(t_adf);
   T_TAB(t_misc);
   T_TAB(t_preset);
   T_TAB(t_timings);
}
void clear_tab(void)
{
   FILLRECT(  win.x+8, win.y+win.t+TAB_HEIGHT+4, win.w-16, win.h-win.t-TAB_HEIGHT-11, 0x00A8A8A8);
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
   if(t_info->is_painted==0)
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
      for(unsigned int j=0;j<sizeof(labels1)/sizeof(labels1[0]);j++)
         displayStringAt(Font,win.x+ 12,win.y+win.t+TAB_HEIGHT+10+j*(Font->Height+5),(uint8_t*)text_labels1[j],LEFT_MODE);
      for(unsigned int j=0;j<sizeof(labels2)/sizeof(labels2[0]);j++)
         displayStringAt(Font,win.x+200,win.y+win.t+TAB_HEIGHT+10+j*(Font->Height+5),(uint8_t*)text_labels2[j],LEFT_MODE);
   }
   for(unsigned int j=0;j<sizeof(labels1)/sizeof(labels1[0]);j++)
      TEXTFIELD(win.x+10+    125,win.y+win.t+TAB_HEIGHT+8+j*(Font->Height+5),50,15,(uint8_t*)labels1[j],0x00000000);
   for(unsigned int j=0;j<sizeof(labels2)/sizeof(labels2[0]);j++)
      TEXTFIELD(win.x+10+200+125,win.y+win.t+TAB_HEIGHT+8+j*(Font->Height+5),50,15,(uint8_t*)labels2[j],0x00000000);
   if(t_info->is_painted==0)
   {
//   if(b_refresh.tab==1) // hide this button as it is now auto refreshing
//      BUTTON(b_refresh.x,b_refresh.y,b_refresh.w,b_refresh.h,b_refresh.text);
      displayStringAt(Font,win.x+12 ,win.y+win.t+TAB_HEIGHT+22+25+80   ,(uint8_t*)" Doubled Cursor",LEFT_MODE);
      displayStringAt(Font,win.x+12 ,win.y+win.t+TAB_HEIGHT+22+25+80+16,(uint8_t*)"Mon. Switch CTS",LEFT_MODE);
      displayStringAt(Font,win.x+12 ,win.y+win.t+TAB_HEIGHT+22+25+80+32,(uint8_t*)"Mon. Switch SEL",LEFT_MODE);
      displayStringAt(Font,win.x+152,win.y+win.t+TAB_HEIGHT+22+25+80+16,(uint8_t*)" CTS act. level",LEFT_MODE);
      displayStringAt(Font,win.x+152,win.y+win.t+TAB_HEIGHT+22+25+80+32,(uint8_t*)" SEL act. level",LEFT_MODE);
      paint_checkboxes();

      paint_b_apply_screen();

      paint_ls_screen_res();
      paint_ls_arm_frequency();
   }
   t_info->is_painted=1;
}
void paint_tab_boot(void)
{
   clear_tab();
   paint_slider_cpufreq();

   TEXTFIELD(win.x+10+38,win.y+win.t+TAB_HEIGHT+8+(Font->Height+3),120,(Font->Height+1)*NUM_BOOT_MODE_LABELS+3,"",0x00000000);
   b_list_emu_action();
   paint_b_apply_boot_mode();
   paint_b_apply_all_boot();

   displayStringAt(Font,win.x+224,win.y+win.t+TAB_HEIGHT+10+12+2+25   ,(uint8_t*)"SCSI BOOT enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+TAB_HEIGHT+10+12+2+25+16,(uint8_t*)"AUTOC RAM enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+TAB_HEIGHT+10+12+2+25+32,(uint8_t*)"AUTOC RTG enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+TAB_HEIGHT+10+12+2+25+48,(uint8_t*)"     TEST enabled",LEFT_MODE);
   displayStringAt(Font,win.x+224,win.y+win.t+TAB_HEIGHT+10+12+2+25+64,(uint8_t*)"  CPU RAM enabled",LEFT_MODE);
   displayStringAt(Font,win.x+ 24,win.y+win.t+TAB_HEIGHT+10+12+2+25+80,(uint8_t*)"    MOUNT SD 0x76",LEFT_MODE);
   displayStringAt(Font,win.x+ 24,win.y+win.t+TAB_HEIGHT+10+12+2+25+96,(uint8_t*)"    MOUNT SD ROOT",LEFT_MODE);

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
void paint_tab_adf(void)
{
   clear_tab();

   paint_ls_adf();
   paint_b_apply_adf();
   paint_b_apply_all_adf();
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
void test_tab_timings(void)
{
   uint32_t color=0;
   color|=paint_timings_phase_pclk_textedit();
   color|=paint_timings_phase_clken_textedit();
   color|=paint_timings_phase_bclk_textedit();
   color|=paint_timings_phase_cpuclk_textedit();
   color|=paint_timings_phase_clk90_textedit();
   color|=paint_timings_emu_extra_phase_textedit();
   color|=paint_timings_divider_pclk_textedit();
   color|=paint_timings_divider_clken_textedit();
   color|=paint_timings_divider_bclk_textedit();
   color|=paint_timings_divider_cpuclk_textedit();
   color|=paint_timings_divider_clk90_textedit();
   color|=paint_timings_multiplier_textedit();
   color|=paint_timings_divider_textedit();
   b_apply_timings->disabled=
   b_apply_all_timings->disabled=color!=0x00000000;
}
void paint_tab_timings(void)
{
   clear_tab();
//   paint_checkboxes();
   test_tab_timings();
   paint_b_apply_timings();
   paint_b_apply_all_timings();
   paint_ls_timings();
}
