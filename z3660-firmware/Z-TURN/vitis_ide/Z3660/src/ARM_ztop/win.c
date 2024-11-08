#include "win.h"
#include "tabs.h"
#include "list_button.h"
#include "list_select.h"
#include "slider.h"
#include "checkbox.h"
#include "button.h"
#include "button_quit.h"
#include "textedit.h"
#include "../config_file.h"

#include <stdlib.h>
#include <stdio.h>

WIN win;
extern int v_major;
extern int v_minor;
extern int beta;
extern int alfa;
extern char message[300];
extern ZZ_VIDEO_STATE vs;
extern CONFIG config;
extern ENV_FILE_VARS env_file_vars_temp[9]; // really size 8
void load_config_from_ztop(void)
{
   env_file_vars_temp[preset_selected].boot_mode=b_list->selected_item;
   env_file_vars_temp[preset_selected].scsiboot=cb_scsi_boot->checked;
   env_file_vars_temp[preset_selected].autoconfig_ram=cb_autoc_ram->checked;
   env_file_vars_temp[preset_selected].autoconfig_rtg=cb_autoc_rtg->checked;
   env_file_vars_temp[preset_selected].enable_test=cb_test->checked;
   env_file_vars_temp[preset_selected].cpu_ram=cb_cpuram->checked;
   env_file_vars_temp[preset_selected].cpufreq=slider_cpufreq->pos;
   env_file_vars_temp[preset_selected].kickstart=ls_kickstart->selected_item;
   env_file_vars_temp[preset_selected].ext_kickstart=ls_kickstart_ext->selected_item;
   env_file_vars_temp[preset_selected].bootscreen_resolution=ls_screen_res->selected_item;
   for(int i=0;i<7;i++)
      env_file_vars_temp[preset_selected].scsi_num[i]=ls_scsi[i]->selected_item-1;
   for(int i=0;i<6;i++)
      env_file_vars_temp[preset_selected].mac_address[i]=mac_textedit->mac_address[i];
   env_file_vars_temp[preset_selected].bp_ton=((slider_bpton->pos+0.5e-6)*TON_MAX)/slider_bpton->max;
   env_file_vars_temp[preset_selected].bp_toff=((slider_bptoff->pos+0.5e-3)*TOFF_MAX)/slider_bptoff->max;

}
void load_config_to_ztop(void)
{
   b_list->selected_item=config.boot_mode=env_file_vars_temp[preset_selected].boot_mode;
   cb_scsi_boot->checked=config.scsiboot=env_file_vars_temp[preset_selected].scsiboot;
   cb_autoc_ram->checked=config.autoconfig_ram=env_file_vars_temp[preset_selected].autoconfig_ram;
   cb_autoc_rtg->checked=config.autoconfig_rtg=env_file_vars_temp[preset_selected].autoconfig_rtg;
   cb_test->checked=config.enable_test=env_file_vars_temp[preset_selected].enable_test;
   cb_cpuram->checked=config.cpu_ram=env_file_vars_temp[preset_selected].cpu_ram;
   slider_cpufreq->pos=config.cpufreq=env_file_vars_temp[preset_selected].cpufreq;
   ls_kickstart->selected_item=config.kickstart=env_file_vars_temp[preset_selected].kickstart;
   ls_kickstart_ext->selected_item=config.ext_kickstart=env_file_vars_temp[preset_selected].ext_kickstart;
   ls_screen_res->selected_item=config.bootscreen_resolution=env_file_vars_temp[preset_selected].bootscreen_resolution;
   for(int i=0;i<7;i++)
   {
      ls_scsi[i]->selected_item=env_file_vars_temp[preset_selected].scsi_num[i]+1;
      config.scsi_num[i]=env_file_vars_temp[preset_selected].scsi_num[i];
   }
   for(int i=0;i<6;i++)
      mac_textedit->mac_address[i]=config.mac_address[i]=env_file_vars_temp[preset_selected].mac_address[i];

   for(int i=0;i<PRESET_CB_MAX;i++)
      cb_preset[i]->checked=preset_selected==i;
   config.bp_ton=env_file_vars_temp[preset_selected].bp_ton;
   slider_bpton->pos=(config.bp_ton+0.5e-6)*slider_bpton->max/TON_MAX;
   config.bp_toff=env_file_vars_temp[preset_selected].bp_toff;
   slider_bptoff->pos=(config.bp_toff+0.5e-3)*slider_bptoff->max/TOFF_MAX;

}

void recalculate_coords(void)
{
   recalculate_coords_drag();

   recalculate_coords_tabs();

   recalculate_coords_list_button();

   recalculate_coords_list_select();

   recalculate_coords_sliders();

   recalculate_coords_buttons();

   recalculate_coords_button_quit();

   recalculate_coords_checkboxes();

   recalculate_coords_textedits();
}

void show_ztop(void)
{
   Font = &Font12;

   clear_screen();

   // Winodw background
   FILLRECT(        win.x,       win.y  ,    win.w,           win.h, 0x00A8A8A8); // 431 279 426 177

   FILLRECT(        win.x,        win.y  ,    win.w,               1, 0x00FFFFFF); // top edge white
   FILLRECT(        win.x,        win.y+1,        1,         win.h-1, 0x00FFFFFF); // left edge white
   FILLRECT(win.x+win.w-1,        win.y+1,        1,         win.h-2, 0x00000000); // bottom edge black
   FILLRECT(      win.x+1,  win.y+win.h-1,  win.w-1,               1, 0x00000000); // right edge black

   FILLRECT(      win.x+3,    win.y+win.t,  win.w-6,               1, 0x00000000); // inner top edge black
   FILLRECT(      win.x+3,  win.y+win.t+1,        1,   win.h-win.t-4, 0x00000000); // inner left edge black
   FILLRECT(win.x+win.w-4,  win.y+win.t+1,        1,   win.h-win.t-5, 0x00FFFFFF); // inner bottom edge white
   FILLRECT(      win.x+4,  win.y+win.h-4,  win.w-7,               1, 0x00FFFFFF); // inner right edge white

   FILLRECT(      win.x+1,    win.y+win.t,        2,   win.h-win.t-3, 0x006088BE); // left blue fill
   FILLRECT(win.x+win.w-3,    win.y+win.t,        2,   win.h-win.t-3, 0x006088BE); // right blue fill
   FILLRECT(      win.x+1,  win.y+win.h-3,  win.w-2,               2, 0x006088BE); // bottom blue fill
   FILLRECT(      win.x+1,        win.y+1,  win.w-2,         win.t-1, 0x006088BE); // top blue fill
   if(beta==0)
      sprintf(message,"Z3660 ZTop %d.%02d",v_major,v_minor);
   else
   {
      if(alfa==0)
         sprintf(message,"Z3660 ZTop %d.%02d (BETA %d FIRMWARE DETECTED)",v_major,v_minor,beta);
      else
         sprintf(message,"Z3660 ZTop %d.%02d (BETA %d ALFA %d)",v_major,v_minor,beta,alfa);
   }
   Font->BackColor=0x006088BE;
   Font->TextColor=0x00000000;
   displayStringAt(Font,win.x+16+8,win.y+2,(uint8_t*)message,LEFT_MODE);

   FILLRECT(      win.x+6, win.y+win.t+tab_h+2, win.w-12,                   1, 0x00FFFFFF); // frame top edge
   FILLRECT(      win.x+6, win.y+win.t+tab_h+2,        2, win.h-win.t-tab_h-9, 0x00FFFFFF); // frame left edge
   FILLRECT(      win.x+7,       win.y+win.h-6, win.w-13,                   1, 0x00000000); // frame bottom edge
   FILLRECT(win.x+win.w-8, win.y+win.t+tab_h+3,        2, win.h-win.t-tab_h-8, 0x00000000); // frame right edge

   init_tabs();

   init_buttons();
   init_listbuttons();
   init_listselects();
   init_sliders();
   init_checkboxes();
   init_button_quit();
   init_textedits();

   load_config_to_ztop();

   recalculate_coords();

   init_drag(); // after recalculate_coords

   tabs_paint_init();
   button_quit_paint_init();
   paint_tab_info();


//   b_refresh_action();
}
void clear_screen(void)
{
   FILLRECT(0,0,vs.vmode_hsize,vs.vmode_vsize,0x00C8C8C8); // backgorund
}
void init_win(void)
{
   win.w=426;
   win.h=200+8+16;
   win.x=(vs.vmode_hsize-win.w)/2;
   win.y=(vs.vmode_vsize-win.h)/2;
   win.t=14; // window title height
}
void calculate_drag(int delta_x,int delta_y)
{
   int drag_win_x=get_drag_win_x_pre();
   int drag_win_y=get_drag_win_y_pre();
   int temp_x=delta_x+drag_win_x;
   int temp_y=delta_y+drag_win_y;
   if(temp_x<0) temp_x=0;
   if(temp_y<0) temp_y=0;
   if(temp_x>vs.vmode_hsize-win.w) temp_x=vs.vmode_hsize-win.w;
   if(temp_y>vs.vmode_vsize-win.h) temp_y=vs.vmode_vsize-win.h;
   if(win.x!=temp_x || win.y!=temp_y)
   {
      copy_rect_nomask( temp_x, temp_y, win.w, win.h,
            win.x, win.y, MNTVA_COLOR_16BIT565,
            vs.framebuffer,
            vs.vmode_hsize/vs.vmode_hdiv, MINTERM_SRC);
      if(temp_x < win.x)
      {
         FILLRECT(temp_x+win.w,win.y,win.x-temp_x,win.h, 0x00C8C8C8);
         if(temp_y < win.y)
         {
            FILLRECT(win.x,temp_y+win.h,win.w,win.y-temp_y, 0x00C8C8C8);
         }
         else
         {
            FILLRECT(win.x,win.y,win.w,temp_y-win.y, 0x00C8C8C8);
         }
      }
      else
      {
         FILLRECT(win.x,win.y,temp_x-win.x,win.h, 0x00C8C8C8);
         if(temp_y < win.y)
         {
            FILLRECT(win.x,temp_y+win.h,win.w,win.y-temp_y, 0x00C8C8C8);
         }
         else
         {
            FILLRECT(win.x,win.y,win.w,temp_y-win.y, 0x00C8C8C8);
         }
      }
      win.x=temp_x;
      win.y=temp_y;
      recalculate_coords();
   }

}
void win_run(void)
{
   tabs_run();
   list_selects_run();
   sliders_run();
   drag_run();
   buttons_run();
   button_quit_run();
   listbuttons_run();
   checkboxes_run();
   textedits_run();
}
void win_actions(void)
{
   tabs_action();
   list_selects_action();
   sliders_action();
   drag_action();
   buttons_action();
   button_quit_action();
   list_buttons_action();
   checkboxes_action();
   textedits_action();
}
void win_repaint(void)
{
   tabs_repaint();
   list_select_repaint();
   sliders_repaint();
   drag_repaint();
   buttons_repaint();
   button_quit_repaint();
   list_button_repaint();
   checkboxes_repaint();
   textedits_repaint();
}
