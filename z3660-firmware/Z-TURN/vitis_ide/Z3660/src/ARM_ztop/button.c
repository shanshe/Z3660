#include "button.h"
#include <stdlib.h>
#include <stdio.h>

#include "checkbox.h"
#include "slider.h"
#include "list_button.h"
#include "list_select.h"
#include "textedit.h"
#include "../config_file.h"

extern Tabs selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern ListSelect *ls_screen_res;
extern ListSelect *ls_kickstart;
extern ListSelect *ls_kickstart_ext;
extern ListSelect *ls_scsi[7];
extern ListButton *b_list;
extern CheckBox *cb_scsi_boot;
extern CheckBox *cb_autoc_ram;
extern CheckBox *cb_autoc_rtg;
extern CheckBox *cb_test;
extern CheckBox *cb_cpuram;
extern Slider *slider_cpufreq;
extern int v_major;
extern int v_minor;
extern int beta;
extern int alfa;
extern int preset_selected;

uint32_t read_rtg_register(uint32_t zaddr);
void write_rtg_register(uint32_t zaddr,uint32_t zdata);
void hard_reboot(void);
void reboot(void);
int delete_selected_preset(void);

int8_t is_cursor_at_button(Button *b)
{
   if(mousex>=b->x && mousex<=b->x+b->w
   && mousey>=b->y && mousey<=b->y+b->h)
   {
      return(1);
   }
   return(0);
}
void button_run(Button *b)
{
   if( selected_tab==b->tab && is_cursor_at_button(b) )
      b->is_pressed=1;
   else
      b->is_pressed=0;
}
extern char labels1[5][5];
extern char labels2[4][25];

Button *b_refresh;
void paint_tab_info(void);
void b_refresh_action(void)
{
   int data=read_rtg_register(REG_ZZ_FW_VERSION);
   v_major=(data>>8)&0xFF;
   v_minor=(data)&0xFF;
   int info_cpu_freq=read_rtg_register(REG_ZZ_CPU_FREQ);
   double temp=read_rtg_register(REG_ZZ_TEMPERATURE)*0.100;
   double vaux=read_rtg_register(REG_ZZ_VOLTAGE_AUX)*0.010;
   double vint=read_rtg_register(REG_ZZ_VOLTAGE_INT)*0.010;
   double ltc_temp=read_rtg_register(REG_ZZ_LTC_TEMP)*0.010;
   double ltc_v1=read_rtg_register(REG_ZZ_LTC_V1)*0.010;
   double ltc_v2=read_rtg_register(REG_ZZ_LTC_V2)*0.010;
   double ltc_060_temp=read_rtg_register(REG_ZZ_LTC_060_TEMP)*0.010;

   sprintf(labels1[0],"%01d.%02d",v_major,v_minor);
   sprintf(labels1[1],"%.2f",vaux);
   sprintf(labels1[2],"%.2f",vint);
   sprintf(labels1[3],"%.2f",ltc_v1);
   sprintf(labels1[4],"%.2f",ltc_v2);

   sprintf(labels2[0],"%d",info_cpu_freq);
   sprintf(labels2[1],"%.1f",temp);
   sprintf(labels2[2],"%.1f",ltc_temp);
   sprintf(labels2[3],"%.1f",ltc_060_temp);
   paint_tab_info();
}

Button *b_apply_screen_res;
void b_apply_screen_res_action(void)
{
   if(preset_selected>=0)
   {
      env_file_vars_temp[preset_selected].bootscreen_resolution=ls_screen_res->selected_item;
      if(write_env_files_bootscres(&env_file_vars_temp[preset_selected]))
         hard_reboot();
   }
}
Button *b_apply_boot_mode;
void b_apply_boot_mode_action(void)
{
   load_config_from_ztop();
   write_rtg_register(REG_ZZ_BOOTMODE,b_list->selected_item);
   write_rtg_register(REG_ZZ_SCSIBOOT_EN,cb_scsi_boot->checked);
   write_rtg_register(REG_ZZ_AUTOC_RAM_EN,cb_autoc_ram->checked);
   write_rtg_register(REG_ZZ_AUTOC_RTG_EN,cb_autoc_rtg->checked);
   write_rtg_register(REG_ZZ_TEST_ENABLE,cb_test->checked);
   write_rtg_register(REG_ZZ_CPU_RAM_EN,cb_cpuram->checked);
   write_rtg_register(REG_ZZ_CPU_FREQ,slider_cpufreq->pos);
   write_rtg_register(REG_ZZ_KS_SEL,ls_kickstart->selected_item);
   write_rtg_register(REG_ZZ_EXT_KS_SEL,ls_kickstart_ext->selected_item);
   if(write_env_files_boot(&env_file_vars_temp[preset_selected])==1)
      hard_reboot();
}
Button *b_apply_all_boot;
void b_apply_all_action(void)
{
   load_config_from_ztop();
   write_rtg_register(REG_ZZ_BOOTMODE,b_list->selected_item);
   write_rtg_register(REG_ZZ_SCSIBOOT_EN,cb_scsi_boot->checked);
   write_rtg_register(REG_ZZ_AUTOC_RAM_EN,cb_autoc_ram->checked);
   write_rtg_register(REG_ZZ_AUTOC_RTG_EN,cb_autoc_rtg->checked);
   write_rtg_register(REG_ZZ_TEST_ENABLE,cb_test->checked);
   write_rtg_register(REG_ZZ_CPU_RAM_EN,cb_cpuram->checked);
   write_rtg_register(REG_ZZ_CPU_FREQ,slider_cpufreq->pos);
   write_rtg_register(REG_ZZ_KS_SEL,ls_kickstart->selected_item);
   write_rtg_register(REG_ZZ_EXT_KS_SEL,ls_kickstart_ext->selected_item);
   for(int i=0;i<7;i++)
      write_rtg_register(REG_ZZ_SCSI_SEL_0+4*i,ls_scsi[i]->selected_item);
   write_rtg_register(REG_ZZ_ETH_MAC_HI,(mac_textedit->mac_address[0]<<8)|mac_textedit->mac_address[1]);
   write_rtg_register(REG_ZZ_ETH_MAC_LO,(mac_textedit->mac_address[2]<<24)|(mac_textedit->mac_address[3]<<16)|(mac_textedit->mac_address[4]<<8)|mac_textedit->mac_address[5]);
   for(int i=0;i<6;i++)
      env_file_vars_temp[preset_selected].mac_address[i]=mac_textedit->mac_address[i];
   strcpy(env_file_vars_temp[preset_selected].preset_name,preset_textedit[preset_selected]->text);
printf("preset_selected %d %s\n",preset_selected,preset_textedit[preset_selected]->text);
   env_file_vars_temp[preset_selected].bootscreen_resolution=ls_screen_res->selected_item;
   if(write_env_files(&env_file_vars_temp[preset_selected])==1)
      hard_reboot();
}
Button *b_apply_scsi;
void b_apply_scsi_action(void)
{
   load_config_from_ztop();
   for(int i=0;i<7;i++)
      write_rtg_register(REG_ZZ_SCSI_SEL_0+4*i,ls_scsi[i]->selected_item);
   if(write_env_files_scsi(&env_file_vars_temp[preset_selected])==1)
      hard_reboot();
}
Button *b_apply_all_scsi;
Button *b_apply_misc;
void b_apply_misc_action(void)
{
   if(preset_selected==PRESET_CB_MAX-1)
   {

      hard_reboot();
      return;
   }
   load_config_from_ztop();
   write_rtg_register(REG_ZZ_ETH_MAC_HI,(mac_textedit->mac_address[0]<<8)|mac_textedit->mac_address[1]);
   write_rtg_register(REG_ZZ_ETH_MAC_LO,(mac_textedit->mac_address[2]<<24)|(mac_textedit->mac_address[3]<<16)|(mac_textedit->mac_address[4]<<8)|mac_textedit->mac_address[5]);
   for(int i=0;i<6;i++)
      env_file_vars_temp[preset_selected].mac_address[i]=mac_textedit->mac_address[i];
   if(write_env_files_misc(&env_file_vars_temp[preset_selected]))
      hard_reboot();
}
Button *b_apply_all_misc;
Button *b_apply_preset;
void b_apply_preset_action(void)
{
   strncpy(env_file_vars_temp[preset_selected].preset_name,preset_textedit[preset_selected]->text,50);
   if(write_env_files_preset(&env_file_vars_temp[preset_selected]))
      hard_reboot();
}
Button *b_apply_all_preset;
Button *b_delete_preset;
void b_delete_preset_action(void)
{
   if(preset_selected>=0 && preset_selected<=7)
   {
      if(delete_selected_preset())
      {
         preset_selected=8;
         b_apply_preset_action();
      }
   }
}

void recalculate_coords_buttons(void)
{
   b_refresh->x=win.x+263;
   b_refresh->y=win.y+153;

   b_apply_screen_res->x=win.x+23;
   b_apply_screen_res->y=win.y+164+16+16;

   b_apply_boot_mode->x=win.x+23;
   b_apply_boot_mode->y=win.y+164+16+16;

   b_apply_scsi->x=win.x+23;
   b_apply_scsi->y=win.y+164+16+16;

   b_apply_misc->x=win.x+23;
   b_apply_misc->y=win.y+164+16+16;

   b_apply_preset->x=win.x+23-5;
   b_apply_preset->y=win.y+164+16+16;

   b_delete_preset->x=win.x+23+150;
   b_delete_preset->y=win.y+164+16+16;

   b_apply_all_boot->x=win.x+win.w-80-23;
   b_apply_all_boot->y=win.y+164+16+16;

   b_apply_all_scsi->x=win.x+win.w-80-23;
   b_apply_all_scsi->y=win.y+164+16+16;

   b_apply_all_misc->x=win.x+win.w-80-23;
   b_apply_all_misc->y=win.y+164+16+16;

   b_apply_all_preset->x=win.x+win.w-80-(23-5);
   b_apply_all_preset->y=win.y+164+16+16;
}
void init_buttons(void)
{
   b_refresh=(Button *)malloc(sizeof(Button));
   b_refresh->w=63;
   b_refresh->h=14;
   b_refresh->text="Refresh";
   b_refresh->is_pressed=0;
   b_refresh->b_was_at_cursor=0;
   b_refresh->action=b_refresh_action;
   b_refresh->tab=-1; // hide this button

   b_apply_screen_res=(Button *)malloc(sizeof(Button));
   b_apply_screen_res->w=112;
   b_apply_screen_res->h=14;
   b_apply_screen_res->text="Apply Boot Res";
   b_apply_screen_res->is_pressed=0;
   b_apply_screen_res->b_was_at_cursor=0;
   b_apply_screen_res->action=b_apply_screen_res_action;
   b_apply_screen_res->tab=TAB_INFO;

   b_apply_boot_mode=(Button *)malloc(sizeof(Button));
   b_apply_boot_mode->w=120;
   b_apply_boot_mode->h=14;
   b_apply_boot_mode->text="Apply Boot Mode";
   b_apply_boot_mode->is_pressed=0;
   b_apply_boot_mode->b_was_at_cursor=0;
   b_apply_boot_mode->action=b_apply_boot_mode_action;
   b_apply_boot_mode->tab=TAB_BOOT;

   b_apply_scsi=(Button *)malloc(sizeof(Button));
   b_apply_scsi->w=86;
   b_apply_scsi->h=14;
   b_apply_scsi->text="Apply SCSI";
   b_apply_scsi->is_pressed=0;
   b_apply_scsi->b_was_at_cursor=0;
   b_apply_scsi->action=b_apply_scsi_action;
   b_apply_scsi->tab=TAB_SCSI;

   b_apply_misc=(Button *)malloc(sizeof(Button));
   b_apply_misc->w=86-8+8*2;
   b_apply_misc->h=14;
   b_apply_misc->text="Apply Misc";
   b_apply_misc->is_pressed=0;
   b_apply_misc->b_was_at_cursor=0;
   b_apply_misc->action=b_apply_misc_action;
   b_apply_misc->tab=TAB_MISC;

   b_apply_preset=(Button *)malloc(sizeof(Button));
   b_apply_preset->w=86-8+8*3;
   b_apply_preset->h=14;
   b_apply_preset->text="Apply Preset";
   b_apply_preset->is_pressed=0;
   b_apply_preset->b_was_at_cursor=0;
   b_apply_preset->action=b_apply_preset_action;
   b_apply_preset->tab=TAB_PRESET;

   b_delete_preset=(Button *)malloc(sizeof(Button));
   b_delete_preset->w=86-8+8*4;
   b_delete_preset->h=14;
   b_delete_preset->text="Delete Preset";
   b_delete_preset->is_pressed=0;
   b_delete_preset->b_was_at_cursor=0;
   b_delete_preset->action=b_delete_preset_action;
   b_delete_preset->tab=TAB_PRESET;

   b_apply_all_boot=(Button *)malloc(sizeof(Button));
   b_apply_all_boot->w=80;
   b_apply_all_boot->h=14;
   b_apply_all_boot->text="Apply All";
   b_apply_all_boot->is_pressed=0;
   b_apply_all_boot->b_was_at_cursor=0;
   b_apply_all_boot->action=b_apply_all_action;
   b_apply_all_boot->tab=TAB_BOOT;

   b_apply_all_scsi=(Button *)malloc(sizeof(Button));
   b_apply_all_scsi->w=80;
   b_apply_all_scsi->h=14;
   b_apply_all_scsi->text="Apply All";
   b_apply_all_scsi->is_pressed=0;
   b_apply_all_scsi->b_was_at_cursor=0;
   b_apply_all_scsi->action=b_apply_all_action;
   b_apply_all_scsi->tab=TAB_SCSI;

   b_apply_all_misc=(Button *)malloc(sizeof(Button));
   b_apply_all_misc->w=80;
   b_apply_all_misc->h=14;
   b_apply_all_misc->text="Apply All";
   b_apply_all_misc->is_pressed=0;
   b_apply_all_misc->b_was_at_cursor=0;
   b_apply_all_misc->action=b_apply_all_action;
   b_apply_all_misc->tab=TAB_MISC;

   b_apply_all_preset=(Button *)malloc(sizeof(Button));
   b_apply_all_preset->w=80;
   b_apply_all_preset->h=14;
   b_apply_all_preset->text="Apply All";
   b_apply_all_preset->is_pressed=0;
   b_apply_all_preset->b_was_at_cursor=0;
   b_apply_all_preset->action=b_apply_all_action;
   b_apply_all_preset->tab=TAB_PRESET;
}

void paint_b_apply_screen(void)
{
   BUTTON(b_apply_screen_res);
}

void paint_b_apply_boot_mode(void)
{
   BUTTON(b_apply_boot_mode);
}
void paint_b_apply_all_boot(void)
{
   BUTTON(b_apply_all_boot);
}
void paint_b_apply_scsi(void)
{
   BUTTON(b_apply_scsi);
}
void paint_b_apply_all_scsi(void)
{
   BUTTON(b_apply_all_scsi);
}
void paint_b_apply_misc(void)
{
   BUTTON(b_apply_misc);
}
void paint_b_apply_preset(void)
{
   BUTTON(b_apply_preset);
}
void paint_b_delete_preset(void)
{
   BUTTON(b_delete_preset);
}
void paint_b_apply_all_misc(void)
{
   BUTTON(b_apply_all_misc);
}
void paint_b_apply_all_preset(void)
{
   BUTTON(b_apply_all_preset);
}
void buttons_run(void)
{
   button_run(b_refresh);
   button_run(b_apply_screen_res);
   button_run(b_apply_boot_mode);
   button_run(b_apply_all_boot);
   button_run(b_apply_scsi);
   button_run(b_apply_all_scsi);
   button_run(b_apply_misc);
   button_run(b_apply_all_misc);
   button_run(b_apply_preset);
   button_run(b_delete_preset);
   button_run(b_apply_all_preset);
}
void button_action(Button *b)
{
   if(b->is_pressed==0)
      return;
   b->is_pressed=0;
   b->b_was_at_cursor=0;
   if( selected_tab==b->tab && is_cursor_at_button(b) )
   {
      BUTTON(b);
      b->action();
   }
}
void button_repaint(Button *b)
{
   if(selected_tab!=b->tab)
      return;
   if(b->is_pressed==0)
      return;
   uint8_t b_is_at_cursor=is_cursor_at_button(b);
   if(b_is_at_cursor && b->b_was_at_cursor==0)
      BUTTON_PRESSED(b);
   else if(b_is_at_cursor==0 && b->b_was_at_cursor)
      BUTTON(b);
   b->b_was_at_cursor=b_is_at_cursor;
}

void buttons_action(void)
{
   button_action(b_refresh);
   button_action(b_apply_screen_res);
   button_action(b_apply_boot_mode);
   button_action(b_apply_all_boot);
   button_action(b_apply_scsi);
   button_action(b_apply_all_scsi);
   button_action(b_apply_misc);
   button_action(b_apply_all_misc);
   button_action(b_apply_preset);
   button_action(b_delete_preset);
   button_action(b_apply_all_preset);
}
void buttons_repaint(void)
{
   button_repaint(b_refresh);
   button_repaint(b_apply_screen_res);
   button_repaint(b_apply_boot_mode);
   button_repaint(b_apply_all_boot);
   button_repaint(b_apply_scsi);
   button_repaint(b_apply_all_scsi);
   button_repaint(b_apply_misc);
   button_repaint(b_apply_all_misc);
   button_repaint(b_apply_preset);
   button_repaint(b_delete_preset);
   button_repaint(b_apply_all_preset);
}
