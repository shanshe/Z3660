#include "checkbox.h"
#include "textedit.h"
#include <stdlib.h>
#include "../config_file.h"

extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern sFONT *Font;

CheckBox *cb_scsi_boot;
void cb_scsi_boot_action(void *cb)
{
   cb_scsi_boot->checked=!cb_scsi_boot->checked;
}
CheckBox *cb_autoc_ram;
void cb_autoc_ram_action(void *cb)
{
   cb_autoc_ram->checked=!cb_autoc_ram->checked;
}
CheckBox *cb_autoc_rtg;
void cb_autoc_rtg_action(void *cb)
{
   cb_autoc_rtg->checked=!cb_autoc_rtg->checked;
}
CheckBox *cb_test;
void cb_test_action(void *cb)
{
   cb_test->checked=!cb_test->checked;
}
CheckBox *cb_cpuram;
void cb_cpuram_action(void *cb)
{
   cb_cpuram->checked=!cb_cpuram->checked;
}
CheckBox *cb_preset[PRESET_CB_MAX]; // 8 preset + 1 use no preset
void cb_preset_action(void *cb)
{
   // Before changing the preset, store the current settings
   if(preset_selected<PRESET_CB_MAX-1)
   {
      load_config_from_ztop();
   }
   // then change the preset
   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      cb_preset[i]->checked=0;
      if(cb_preset[i]==cb)
      {
         preset_selected=i;
         cb_preset[i]->checked=1;
      }
      CHECKBOX(cb_preset[i],cb_preset[i]->checked);
      if(i<PRESET_CB_MAX-1)
         preset_textedit[i]->cursor_pos=-1;
   }
   if(preset_selected<PRESET_CB_MAX-1)
   {
      preset_textedit[preset_selected]->cursor_pos=1;
      // and finally, load the settings from the preset
      load_config_to_ztop();
   }
   else
   {

   }
}

int8_t is_cursor_at_checkbox(CheckBox *cb)
{
   if(mousex>=cb->x && mousex<=cb->x+cb->w
   && mousey>=cb->y && mousey<=cb->y+cb->h)
   {
      return(1);
   }
   return(0);
}

void checkbox_run(CheckBox *cb)
{
   if( selected_tab==cb->tab && is_cursor_at_checkbox(cb) )
      cb->is_pressed=1;
   else
      cb->is_pressed=0;
}
void checkbox_action(CheckBox *cb)
{
   if(cb->is_pressed==0)
      return;
   cb->is_pressed=0;
   cb->b_was_at_cursor=0;
   if( selected_tab==cb->tab && is_cursor_at_checkbox(cb) )
   {
      cb->action(cb);
      CHECKBOX(cb,cb->checked);
   }
}
void checkbox_repaint(CheckBox *cb)
{
   if(selected_tab!=cb->tab)
      return;
   if(cb->is_pressed==0)
      return;
   uint8_t cb_is_at_cursor=is_cursor_at_checkbox(cb);
   if(cb_is_at_cursor && cb->b_was_at_cursor==0)
   {
      CHECKBOX(cb,!cb->checked);
   }
   else if(cb_is_at_cursor==0 && cb->b_was_at_cursor)
   {
      CHECKBOX(cb,cb->checked);
   }

   cb->b_was_at_cursor=cb_is_at_cursor;
}
void checkboxes_run(void)
{
   checkbox_run(cb_scsi_boot);
   checkbox_run(cb_autoc_ram);
   checkbox_run(cb_autoc_rtg);
   checkbox_run(cb_test);
   checkbox_run(cb_cpuram);
   for(int i=0;i<PRESET_CB_MAX;i++)
      checkbox_run(cb_preset[i]);
}
void init_checkboxes(void)
{
   cb_scsi_boot=(CheckBox *)malloc(sizeof(CheckBox));
   cb_scsi_boot->w=14;
   cb_scsi_boot->h=14;
   cb_scsi_boot->is_pressed=0;
   cb_scsi_boot->b_was_at_cursor=0;
   cb_scsi_boot->action=cb_scsi_boot_action;
   cb_scsi_boot->tab=TAB_BOOT;

   cb_autoc_ram=(CheckBox *)malloc(sizeof(CheckBox));
   cb_autoc_ram->w=14;
   cb_autoc_ram->h=14;
   cb_autoc_ram->is_pressed=0;
   cb_autoc_ram->b_was_at_cursor=0;
   cb_autoc_ram->action=cb_autoc_ram_action;
   cb_autoc_ram->tab=TAB_BOOT;

   cb_autoc_rtg=(CheckBox *)malloc(sizeof(CheckBox));
   cb_autoc_rtg->w=14;
   cb_autoc_rtg->h=14;
   cb_autoc_rtg->is_pressed=0;
   cb_autoc_rtg->b_was_at_cursor=0;
   cb_autoc_rtg->action=cb_autoc_rtg_action;
   cb_autoc_rtg->tab=TAB_BOOT;

   cb_test=(CheckBox *)malloc(sizeof(CheckBox));
   cb_test->w=14;
   cb_test->h=14;
   cb_test->is_pressed=0;
   cb_test->b_was_at_cursor=0;
   cb_test->action=cb_test_action;
   cb_test->tab=TAB_BOOT;

   cb_cpuram=(CheckBox *)malloc(sizeof(CheckBox));
   cb_cpuram->w=14;
   cb_cpuram->h=14;
   cb_cpuram->is_pressed=0;
   cb_cpuram->b_was_at_cursor=0;
   cb_cpuram->action=cb_cpuram_action;
   cb_cpuram->tab=TAB_BOOT;

   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      cb_preset[i]=(CheckBox *)malloc(sizeof(CheckBox));
      cb_preset[i]->w=14;
      cb_preset[i]->h=14;
      cb_preset[i]->is_pressed=0;
      cb_preset[i]->b_was_at_cursor=0;
      cb_preset[i]->action=cb_preset_action;
      cb_preset[i]->tab=TAB_PRESET;
   }
}
void checkboxes_repaint(void)
{
   checkbox_repaint(cb_scsi_boot);
   checkbox_repaint(cb_autoc_ram);
   checkbox_repaint(cb_autoc_rtg);
   checkbox_repaint(cb_test);
   checkbox_repaint(cb_cpuram);
   for(int i=0;i<PRESET_CB_MAX;i++)
      checkbox_repaint(cb_preset[i]);
}
void paint_checkboxes(void)
{
   if(cb_scsi_boot->tab==selected_tab)
      CHECKBOX(cb_scsi_boot,cb_scsi_boot->checked);
   if(cb_autoc_ram->tab==selected_tab)
      CHECKBOX(cb_autoc_ram,cb_autoc_ram->checked);
   if(cb_autoc_rtg->tab==selected_tab)
      CHECKBOX(cb_autoc_rtg,cb_autoc_rtg->checked);
   if(cb_test->tab==selected_tab)
      CHECKBOX(cb_test,cb_test->checked);
   if(cb_cpuram->tab==selected_tab)
      CHECKBOX(cb_cpuram,cb_cpuram->checked);
   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      if(cb_preset[i]->tab==selected_tab)
         CHECKBOX(cb_preset[i],cb_preset[i]->checked);
   }
}
void recalculate_coords_checkboxes(void)
{
   cb_scsi_boot->x=win.x+270+80;
   cb_scsi_boot->y=win.y+74;

   cb_autoc_ram->x=win.x+270+80;
   cb_autoc_ram->y=win.y+74+16;

   cb_autoc_rtg->x=win.x+270+80;
   cb_autoc_rtg->y=win.y+74+16+16;

   cb_test->x=win.x+270+80;
   cb_test->y=win.y+74+16+16+16;

   cb_cpuram->x=win.x+270+80;
   cb_cpuram->y=win.y+74+16+16+16+16;

   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      cb_preset[i]->x=win.x+270+92;
      cb_preset[i]->y=win.y+37+(Font->Height+5)*i;
   }
}
void checkboxes_action(void)
{
   checkbox_action(cb_scsi_boot);
   checkbox_action(cb_autoc_ram);
   checkbox_action(cb_autoc_rtg);
   checkbox_action(cb_test);
   checkbox_action(cb_cpuram);
   for(int i=0;i<PRESET_CB_MAX;i++)
      checkbox_action(cb_preset[i]);
}
