#include "button.h"
#include <stdlib.h>
#include <stdio.h>

#include "checkbox.h"
#include "slider.h"
#include "list_button.h"
#include "list_select.h"
#include "textedit.h"
#include "../config_file.h"

#include "config_clk.h"
extern clock_data cd[];

extern Tabs selected_tab;
extern int16_t mousex,mousey;
extern int timing_selected;
extern WIN win;
extern ListSelect *ls_screen_res;
extern ListSelect *ls_kickstart;
extern ListSelect *ls_kickstart_ext;
extern ListSelect *ls_scsi[7];
extern ListSelect *ls_adf[8];
extern ListSelect *ls_arm_frequency;
extern ListButton *b_list_emu;
extern CheckBox *cb_scsi_boot;
extern CheckBox *cb_autoc_ram;
extern CheckBox *cb_autoc_rtg;
extern CheckBox *cb_test;
extern CheckBox *cb_cpuram;
extern CheckBox *cb_mount_sd_0x76;
extern CheckBox *cb_mount_sd_root;
extern CheckBox *cb_doubled_cursor;
extern CheckBox *cb_monitor_switch_CTS;
extern CheckBox *cb_monitor_switch_SEL;
extern CheckBox *cb_monitor_switch_CTS_level;
extern CheckBox *cb_monitor_switch_SEL_level;
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
extern int i2c_ltc2990;
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
   if(i2c_ltc2990<100)
   {
      sprintf(labels1[3],"%.2f",ltc_v1);
      sprintf(labels1[4],"%.2f",ltc_v2);
   }
   else
   {
      sprintf(labels1[3],"----");
      sprintf(labels1[4],"----");
   }
   sprintf(labels2[0],"%d",info_cpu_freq);
   sprintf(labels2[1],"%.1f",temp);
   if(i2c_ltc2990<100)
   {
      sprintf(labels2[2],"%.1f",ltc_temp);
      sprintf(labels2[3],"%.1f",ltc_060_temp);
   }
   else
   {
      sprintf(labels2[2],"----");
      sprintf(labels2[3],"----");
   }
   paint_tab_info();
}

Button *b_apply_screen_res;
void b_apply_screen_res_action(void)
{
   if(preset_selected>=0)
   {
      env_file_vars_temp[preset_selected].bootscreen_resolution=ls_screen_res->selected_item;
      env_file_vars_temp[preset_selected].doubled_cursor=cb_doubled_cursor->checked;
      int monswitch=0;
      if(cb_monitor_switch_CTS->checked)
         monswitch|=1;
      if(cb_monitor_switch_SEL->checked)
         monswitch|=2;
      if(cb_monitor_switch_CTS_level->checked)
         monswitch|=0x10;
      if(cb_monitor_switch_SEL_level->checked)
         monswitch|=0x20;
      env_file_vars_temp[preset_selected].monitor_switch=monswitch;

      env_file_vars_temp[preset_selected].arm_frequency=ls_arm_frequency->selected_item;

      if(write_env_files_bootscres(&env_file_vars_temp[preset_selected]))
         hard_reboot();
   }
}
Button *b_apply_boot_mode;
void b_apply_boot_mode_action(void)
{
   load_config_from_ztop();
   write_rtg_register(REG_ZZ_BOOTMODE,b_list_emu->selected_item);
   write_rtg_register(REG_ZZ_SCSIBOOT_EN,cb_scsi_boot->checked);
   write_rtg_register(REG_ZZ_AUTOC_RAM_EN,cb_autoc_ram->checked);
   write_rtg_register(REG_ZZ_AUTOC_RTG_EN,cb_autoc_rtg->checked);
   write_rtg_register(REG_ZZ_TEST_ENABLE,cb_test->checked);
   write_rtg_register(REG_ZZ_CPU_RAM_EN,cb_cpuram->checked);
   write_rtg_register(REG_ZZ_MOUNT_SD_0x76,cb_mount_sd_0x76->checked);
   write_rtg_register(REG_ZZ_MOUNT_SD_ROOT,cb_mount_sd_root->checked);
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
   write_rtg_register(REG_ZZ_BOOTMODE,b_list_emu->selected_item);
   write_rtg_register(REG_ZZ_SCSIBOOT_EN,cb_scsi_boot->checked);
   write_rtg_register(REG_ZZ_AUTOC_RAM_EN,cb_autoc_ram->checked);
   write_rtg_register(REG_ZZ_AUTOC_RTG_EN,cb_autoc_rtg->checked);
   write_rtg_register(REG_ZZ_TEST_ENABLE,cb_test->checked);
   write_rtg_register(REG_ZZ_CPU_RAM_EN,cb_cpuram->checked);
   write_rtg_register(REG_ZZ_MOUNT_SD_0x76,cb_mount_sd_0x76->checked);
   write_rtg_register(REG_ZZ_MOUNT_SD_ROOT,cb_mount_sd_root->checked);
   write_rtg_register(REG_ZZ_CPU_FREQ,slider_cpufreq->pos);
   write_rtg_register(REG_ZZ_KS_SEL,ls_kickstart->selected_item);
   write_rtg_register(REG_ZZ_EXT_KS_SEL,ls_kickstart_ext->selected_item);
   for(int i=0;i<7;i++)
      write_rtg_register(REG_ZZ_SCSI_SEL_0+4*i,ls_scsi[i]->selected_item);
   for(int i=0;i<8;i++)
      write_rtg_register(REG_ZZ_ADF_SEL_0+4*i,ls_adf[i]->selected_item);
   write_rtg_register(REG_ZZ_ETH_MAC_HI,(mac_textedit->mac_address[0]<<8)|mac_textedit->mac_address[1]);
   write_rtg_register(REG_ZZ_ETH_MAC_LO,(mac_textedit->mac_address[2]<<24)|(mac_textedit->mac_address[3]<<16)|(mac_textedit->mac_address[4]<<8)|mac_textedit->mac_address[5]);
   for(int i=0;i<6;i++)
      env_file_vars_temp[preset_selected].mac_address[i]=mac_textedit->mac_address[i];
   strcpy(env_file_vars_temp[preset_selected].preset_name,preset_textedit[preset_selected]->text);
printf("preset_selected %d %s\n",preset_selected,preset_textedit[preset_selected]->text);
   env_file_vars_temp[preset_selected].bootscreen_resolution=ls_screen_res->selected_item;
   env_file_vars_temp[preset_selected].doubled_cursor=cb_doubled_cursor->checked;
   int monswitch=0;
   if(cb_monitor_switch_CTS->checked)
      monswitch|=1;
   if(cb_monitor_switch_SEL->checked)
      monswitch|=2;
   if(cb_monitor_switch_CTS_level->checked)
      monswitch|=0x10;
   if(cb_monitor_switch_SEL_level->checked)
      monswitch|=0x20;
   env_file_vars_temp[preset_selected].monitor_switch=monswitch;

   env_file_vars_temp[preset_selected].arm_frequency=ls_arm_frequency->selected_item;

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

Button *b_apply_adf;
void b_apply_adf_action(void)
{
   load_config_from_ztop();
   for(int i=0;i<7;i++)
      write_rtg_register(REG_ZZ_ADF_SEL_0+4*i,ls_adf[i]->selected_item);
   if(write_env_files_adf(&env_file_vars_temp[preset_selected])==1)
      hard_reboot();
}
Button *b_apply_all_adf;

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
int write_timings(void);
Button *b_apply_all_timings;
Button *b_apply_timings;
void b_apply_timings_action(void)
{
   if(b_apply_timings->disabled==0)
   {
      cd[timing_selected].M  =atoi(timings_muldiv_textedit[0]->text);
      cd[timing_selected].D  =atoi(timings_muldiv_textedit[1]->text);
      cd[timing_selected].pclk.phase  =atoi(timings_phase_textedit[0]->text);
      cd[timing_selected].clken.phase =atoi(timings_phase_textedit[1]->text);
      cd[timing_selected].bclk.phase  =atoi(timings_phase_textedit[2]->text);
      cd[timing_selected].cpuclk.phase=atoi(timings_phase_textedit[3]->text);
      cd[timing_selected].clk90.phase =atoi(timings_phase_textedit[4]->text);
      cd[timing_selected].emu_extra_phase=atoi(timings_phase_textedit[5]->text);
      cd[timing_selected].pclk.divider  =atoi(timings_divider_textedit[0]->text);
      cd[timing_selected].clken.divider =atoi(timings_divider_textedit[1]->text);
      cd[timing_selected].bclk.divider  =atoi(timings_divider_textedit[2]->text);
      cd[timing_selected].cpuclk.divider=atoi(timings_divider_textedit[3]->text);
      cd[timing_selected].clk90.divider =atoi(timings_divider_textedit[4]->text);
   }
   write_timings();
//   if(write_env_files_preset(&env_file_vars_temp[preset_selected]))
//      hard_reboot();
}

void recalculate_coords_buttons(void)
{
   b_refresh->x=win.x+263;
   b_refresh->y=win.y+153;

   b_apply_screen_res->x=win.x+23;
   b_apply_screen_res->y=win.y+164+16+16+16+16;

   b_apply_boot_mode->x=win.x+23;
   b_apply_boot_mode->y=win.y+164+16+16+16+16;

   b_apply_scsi->x=win.x+23;
   b_apply_scsi->y=win.y+164+16+16+16+16;

   b_apply_adf->x=win.x+23;
   b_apply_adf->y=win.y+164+16+16+16+16;

   b_apply_misc->x=win.x+23;
   b_apply_misc->y=win.y+164+16+16+16+16;

   b_apply_preset->x=win.x+23;
   b_apply_preset->y=win.y+164+16+16+16+16;

   b_delete_preset->x=win.x+23+150;
   b_delete_preset->y=win.y+164+16+16+16+16;

   b_apply_timings->x=win.x+23;
   b_apply_timings->y=win.y+164+16+16+16+16;

   b_apply_all_boot->x=win.x+win.w-80-23;
   b_apply_all_boot->y=win.y+164+16+16+16+16;

   b_apply_all_scsi->x=win.x+win.w-80-23;
   b_apply_all_scsi->y=win.y+164+16+16+16+16;

   b_apply_all_adf->x=win.x+win.w-80-23;
   b_apply_all_adf->y=win.y+164+16+16+16+16;

   b_apply_all_misc->x=win.x+win.w-80-23;
   b_apply_all_misc->y=win.y+164+16+16+16+16;

   b_apply_all_preset->x=win.x+win.w-80-23;
   b_apply_all_preset->y=win.y+164+16+16+16+16;

   b_apply_all_timings->x=win.x+win.w-80-23;
   b_apply_all_timings->y=win.y+164+16+16+16+16;
}
void init_buttons(void)
{
#define SET_BUTTON_DEFAULTS(X,T) X->text=T;                      \
                                 X->w=(sizeof(T)+1)*Font->Width; \
                                 X->h=Font->Height+2;            \
                                 X->is_pressed=0;                \
                                 X->b_was_at_cursor=0;           \
                                 X->disabled=0;

   b_refresh=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_refresh,"Refresh");
   b_refresh->action=b_refresh_action;
   b_refresh->tab=-1; // hide this button

   b_apply_screen_res=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_screen_res,"Apply Boot Res");
   b_apply_screen_res->action=b_apply_screen_res_action;
   b_apply_screen_res->tab=TAB_INFO;

   b_apply_boot_mode=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_boot_mode,"Apply Boot Mode");
   b_apply_boot_mode->action=b_apply_boot_mode_action;
   b_apply_boot_mode->tab=TAB_BOOT;

   b_apply_scsi=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_scsi,"Apply SCSI");
   b_apply_scsi->action=b_apply_scsi_action;
   b_apply_scsi->tab=TAB_SCSI;

   b_apply_adf=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_adf,"Apply ADF");
   b_apply_adf->action=b_apply_adf_action;
   b_apply_adf->tab=TAB_ADF;

   b_apply_misc=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_misc,"Apply Misc");
   b_apply_misc->action=b_apply_misc_action;
   b_apply_misc->tab=TAB_MISC;

   b_apply_preset=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_preset,"Apply Preset");
   b_apply_preset->action=b_apply_preset_action;
   b_apply_preset->tab=TAB_PRESET;

   b_delete_preset=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_delete_preset,"Delete Preset");
   b_delete_preset->action=b_delete_preset_action;
   b_delete_preset->tab=TAB_PRESET;

   b_apply_timings=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_timings,"Apply Timings");
   b_apply_timings->action=b_apply_timings_action;
   b_apply_timings->tab=TAB_TIMINGS;

   b_apply_all_boot=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_boot,"Apply All");
   b_apply_all_boot->action=b_apply_all_action;
   b_apply_all_boot->tab=TAB_BOOT;

   b_apply_all_scsi=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_scsi,"Apply All");
   b_apply_all_scsi->action=b_apply_all_action;
   b_apply_all_scsi->tab=TAB_SCSI;

   b_apply_all_adf=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_adf,"Apply All");
   b_apply_all_adf->action=b_apply_all_action;
   b_apply_all_adf->tab=TAB_ADF;

   b_apply_all_misc=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_misc,"Apply All");
   b_apply_all_misc->action=b_apply_all_action;
   b_apply_all_misc->tab=TAB_MISC;

   b_apply_all_preset=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_preset,"Apply All");
   b_apply_all_preset->action=b_apply_all_action;
   b_apply_all_preset->tab=TAB_PRESET;

   b_apply_all_timings=(Button *)malloc(sizeof(Button));
   SET_BUTTON_DEFAULTS(b_apply_all_timings,"Apply All");
   b_apply_all_timings->action=b_apply_all_action;
   b_apply_all_timings->tab=TAB_TIMINGS;
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
void paint_b_apply_adf(void)
{
   BUTTON(b_apply_adf);
}
void paint_b_apply_all_adf(void)
{
   BUTTON(b_apply_all_adf);
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
void paint_b_apply_timings(void)
{
   BUTTON(b_apply_timings);
}
void paint_b_apply_all_timings(void)
{
   BUTTON(b_apply_all_timings);
}
void buttons_run(void)
{
   button_run(b_refresh);
   button_run(b_apply_screen_res);
   button_run(b_apply_boot_mode);
   button_run(b_apply_all_boot);
   button_run(b_apply_scsi);
   button_run(b_apply_all_scsi);
   button_run(b_apply_adf);
   button_run(b_apply_all_adf);
   button_run(b_apply_misc);
   button_run(b_apply_all_misc);
   button_run(b_apply_preset);
   button_run(b_delete_preset);
   button_run(b_apply_all_preset);
   button_run(b_apply_timings);
   button_run(b_apply_all_timings);
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
   button_action(b_apply_adf);
   button_action(b_apply_all_adf);
   button_action(b_apply_misc);
   button_action(b_apply_all_misc);
   button_action(b_apply_preset);
   button_action(b_delete_preset);
   button_action(b_apply_all_preset);
   button_action(b_apply_timings);
   button_action(b_apply_all_timings);
}
void buttons_repaint(void)
{
   button_repaint(b_refresh);
   button_repaint(b_apply_screen_res);
   button_repaint(b_apply_boot_mode);
   button_repaint(b_apply_all_boot);
   button_repaint(b_apply_scsi);
   button_repaint(b_apply_all_scsi);
   button_repaint(b_apply_adf);
   button_repaint(b_apply_all_adf);
   button_repaint(b_apply_misc);
   button_repaint(b_apply_all_misc);
   button_repaint(b_apply_preset);
   button_repaint(b_delete_preset);
   button_repaint(b_apply_all_preset);
   button_repaint(b_apply_timings);
   button_repaint(b_apply_all_timings);
}
