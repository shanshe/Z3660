#include "textedit.h"
#include "tabs.h"
#include <stdlib.h>
#include <stdio.h>
#include "../config_file.h"
extern Tabs selected_tab;
extern int16_t mousex,mousey;
extern WIN win;

uint32_t read_rtg_register(uint32_t zaddr);
void write_rtg_register(uint32_t zaddr,uint32_t zdata);
void hard_reboot(void);
void reboot(void);

int8_t is_cursor_at_textedit(TextEdit *b)
{
   if(mousex>=b->x && mousex<=b->x+b->w
         && mousey>=b->y && mousey<=b->y+b->h)
   {
      return(1);
   }
   return(0);
}
void textedit_run(TextEdit *b)
{
   if( selected_tab==b->tab && is_cursor_at_textedit(b) )
      b->is_pressed=1;
   else
      b->is_pressed=0;
}

TextEdit *mac_textedit;
extern char message[300];
void paint_mac_textedit(void)
{
   sprintf(message,"%02X:%02X:%02X:%02X:%02X:%02X",mac_textedit->mac_address[0],
         mac_textedit->mac_address[1],mac_textedit->mac_address[2],
         mac_textedit->mac_address[3],mac_textedit->mac_address[4],mac_textedit->mac_address[5]);
   displayStringAt(Font,win.x+10+8,win.y+win.t+tab_h+13,(uint8_t*)"MAC Address",LEFT_MODE);
   TEXTFIELD(mac_textedit->x,mac_textedit->y,mac_textedit->w,(Font->Height+1)+3,message);
}
void mac_textedit_action(void)
{
   TextEdit *b=mac_textedit;
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/19);
   if(b->cursor_pos<=0) b->cursor_pos=-1;
   if(b->cursor_pos>17) b->cursor_pos=-1;
   if((b->cursor_pos%3)==0) b->cursor_pos++; // skip the :

   paint_mac_textedit();
}
TextEdit *preset_textedit[PRESET_CB_MAX];
void paint_preset_textedit(void)
{
   char temp_str[60];
   for(int i=0;i<PRESET_CB_MAX-1;i++)
   {
      strcpy(message,preset_textedit[i]->text);
      sprintf(temp_str,"Preset %d Name",i);
      displayStringAt(Font,win.x+10+8,win.y+win.t+tab_h+10+(Font->Height+5)*(i),(uint8_t*)temp_str,LEFT_MODE);
      TEXTFIELD(preset_textedit[i]->x,preset_textedit[i]->y,preset_textedit[i]->w,(Font->Height+1)+3,message);
   }
   sprintf(temp_str,"No Preset (will use z3660cfg.txt file as default)");
   displayStringAt(Font,win.x+10+8,win.y+win.t+tab_h+10+(Font->Height+5)*(8),(uint8_t*)temp_str,LEFT_MODE);
}
void preset_textedit_action(void)
{
   TextEdit *b=preset_textedit[preset_selected];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(PRESET_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>PRESET_MAX_LENGTH+1) b->cursor_pos=PRESET_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_preset_textedit();
}

void recalculate_coords_textedits(void)
{
   mac_textedit->x=win.x+10+94;
   mac_textedit->y=win.y+win.t+tab_h+10;

   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      preset_textedit[i]->x=win.x+20+94;
      preset_textedit[i]->y=win.y+win.t+tab_h+7+(Font->Height+5)*(i);
   }
}
void init_textedits(void)
{
   mac_textedit=(TextEdit *)malloc(sizeof(TextEdit));
   mac_textedit->w=144;
   mac_textedit->h=(Font->Height+1)+3;
   mac_textedit->is_pressed=0;
   mac_textedit->b_was_at_cursor=0;
   mac_textedit->action=mac_textedit_action;
   mac_textedit->tab=TAB_MISC;
   mac_textedit->cursor_pos=-1;
   mac_textedit->inverted=0;

   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      preset_textedit[i]=(TextEdit *)malloc(sizeof(TextEdit));
      preset_textedit[i]->w=244;
      preset_textedit[i]->h=(Font->Height+1)+3;
      strcpy(preset_textedit[i]->text,env_file_vars_temp[i].preset_name);
      for(int j=strlen(preset_textedit[i]->text);j<50;j++)
         preset_textedit[i]->text[j]=0;
      preset_textedit[i]->is_pressed=0;
      preset_textedit[i]->b_was_at_cursor=0;
      preset_textedit[i]->action=preset_textedit_action;
      preset_textedit[i]->tab=TAB_PRESET;
      preset_textedit[i]->cursor_pos=-1;
      preset_textedit[i]->inverted=0;
   }
}

void textedits_run(void)
{
   textedit_run(mac_textedit);
   for(int i=0;i<PRESET_CB_MAX;i++)
      textedit_run(preset_textedit[i]);
}
void textedit_action(TextEdit *b)
{
   if(b->is_pressed==0)
      return;
   b->is_pressed=0;
   b->b_was_at_cursor=0;
   if( selected_tab==b->tab && is_cursor_at_textedit(b) )
   {
      //		BUTTON(b);
      b->action();
   }
}
void textedit_repaint(TextEdit *b)
{
   if(selected_tab!=b->tab)
      return;
   if(b->cursor_pos>=0)
   {
      static long int time=25000;
      time--;
      if(time<0)
      {
         time=25000;
         INVERT_RECT(b->x+5+b->cursor_pos*Font->Width,b->y+2,Font->Width,Font->Height);
         b->inverted=!b->inverted;
         if(b->inverted)
            b->last_cursor_pos=b->cursor_pos;
      }
   }
   else
   {
      if(b->inverted)
      {
         INVERT_RECT(b->x+5+b->last_cursor_pos*Font->Width,b->y+2,Font->Width,Font->Height);
         b->inverted=0;
      }
   }
   uint8_t b_is_at_cursor=is_cursor_at_textedit(b);
   if(b_is_at_cursor && b->b_was_at_cursor==0)
   {
      //		BUTTON_PRESSED(b);
   }
   else if(b_is_at_cursor==0 && b->b_was_at_cursor)
   {
      //		BUTTON(b);
   }
   b->b_was_at_cursor=b_is_at_cursor;
}

void textedits_action(void)
{
   textedit_action(mac_textedit);
   textedit_action(preset_textedit[preset_selected]);
}
void textedits_repaint(void)
{
   textedit_repaint(mac_textedit);
   for(int i=0;i<PRESET_CB_MAX-1;i++)
      textedit_repaint(preset_textedit[i]);
}
