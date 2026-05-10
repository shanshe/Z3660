#include "textedit.h"
#include "button.h"
#include "tabs.h"
#include <stdlib.h>
#include <stdio.h>
#include "../config_file.h"
extern Tabs selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
#include "config_clk.h"
extern clock_data cd[];
extern int timing_selected;

uint32_t read_rtg_register(uint32_t zaddr);
void write_rtg_register(uint32_t zaddr,uint32_t zdata);
void textedits_timings_reload(void);
void hard_reboot(void);
void reboot(void);
extern Button *b_apply_timings;
extern Button *b_apply_all_timings;

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
   displayStringAt(Font,win.x+10+8,win.y+win.t+TAB_HEIGHT+13,(uint8_t*)"MAC Address",LEFT_MODE);
   TEXTFIELD(mac_textedit->x,mac_textedit->y,mac_textedit->w,(Font->Height+1)+3,message,0x00000000);
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
      displayStringAt(Font,win.x+10+8,win.y+win.t+TAB_HEIGHT+10+16+(Font->Height+5)*(i),(uint8_t*)temp_str,LEFT_MODE);
      TEXTFIELD(preset_textedit[i]->x,preset_textedit[i]->y,preset_textedit[i]->w,(Font->Height+1)+3,message,0x00000000);
   }
   sprintf(temp_str,"No Preset (will use z3660cfg.txt file as default)");
   displayStringAt(Font,win.x+10+8,win.y+win.t+TAB_HEIGHT+10+16+(Font->Height+5)*(8),(uint8_t*)temp_str,LEFT_MODE);
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
uint32_t timings_phase_text_color_int(char *message)
{
   uint32_t color=0x00000000;
   int value=atoi(message);
   if(value==0)
   {
      if(message[0]=='0' && message[1]==0)
         color=0x00000000;
      else
         color=0x00FF0000; // red
   }
   if(value<-360 || value>360)
      color=0x00FF0000; // red
   return color;
}
uint32_t timings_divider_text_color_int(char *message)
{
   uint32_t color=0x00000000;
   int value=atoi(message);
   if(value==0)
   {
      if(message[0]=='0' && message[1]==0)
         color=0x00000000;
      else
         color=0x00FF0000; // red
   }
   if(value<=0 || value>64)
      color=0x00FF0000; // red
   return color;
}

uint32_t timings_muldiv_text_color_int(char *message)
{
   uint32_t color=0x00000000;
   int value=atoi(message);
   if(value==0)
   {
      if(message[0]=='0' && message[1]==0)
         color=0x00000000;
      else
         color=0x00FF0000; // red
   }
   if(value<=0 || value>63)
      color=0x00FF0000; // red
   return color;
}


TextEdit *timings_phase_textedit[TIMINGS_TE_PHASE_MAX+1]; // +1 emu_extra
TextEdit *timings_divider_textedit[TIMINGS_TE_DIVIDER_MAX];
TextEdit *timings_muldiv_textedit[TIMINGS_DM_MAX];
char timings_ns[TIMINGS_TE_DIVIDER_MAX][20];
char timings_MHz[TIMINGS_TE_DIVIDER_MAX][20];
char timings_DM[1][20];
uint32_t paint_timings_phase_pclk_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[0]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"200 MHz *");
   displayStringAt(Font,win.x+40+35,win.y+win.t+TAB_HEIGHT+53-15,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"/");
   displayStringAt(Font,win.x+40+172,win.y+win.t+TAB_HEIGHT+53-15,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"pclk");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"phase");
   displayStringAt(Font,win.x+40+110,win.y+40+win.t+TAB_HEIGHT+20,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"ns");
   displayStringAt(Font,win.x+40+172,win.y+40+win.t+TAB_HEIGHT+20,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"divider");
   displayStringAt(Font,win.x+40+204,win.y+40+win.t+TAB_HEIGHT+20,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"MHz");
   displayStringAt(Font,win.x+40+270,win.y+40+win.t+TAB_HEIGHT+20,(uint8_t*)temp_str,LEFT_MODE);
   sprintf(temp_str,"Clock Base");
   displayStringAt(Font,win.x+40+270-8,win.y+40+win.t+TAB_HEIGHT-15,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[0]->x,timings_phase_textedit[0]->y,timings_phase_textedit[0]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_phase_clken_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[1]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"clken");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*1,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[1]->x,timings_phase_textedit[1]->y,timings_phase_textedit[1]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_phase_bclk_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[2]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"bclk");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*2,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[2]->x,timings_phase_textedit[2]->y,timings_phase_textedit[2]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_phase_cpuclk_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[3]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"cpuclk");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*3,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[3]->x,timings_phase_textedit[3]->y,timings_phase_textedit[3]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_phase_clk90_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[4]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"clk90");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*4,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[4]->x,timings_phase_textedit[4]->y,timings_phase_textedit[4]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_emu_extra_phase_textedit(void)
{
   char temp_str[60];
   strcpy(message,timings_phase_textedit[5]->text);
   uint32_t color=timings_phase_text_color_int(message);
   sprintf(temp_str,"emu_extra");
   displayStringAt(Font,win.x+40+35,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*5,(uint8_t*)temp_str,LEFT_MODE);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_phase_textedit[5]->x,timings_phase_textedit[5]->y,timings_phase_textedit[5]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_pclk_textedit(void)
{
   strcpy(message,timings_divider_textedit[0]->text);
   uint32_t color=timings_divider_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_divider_textedit[0]->x,timings_divider_textedit[0]->y,timings_divider_textedit[0]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_clken_textedit(void)
{
   strcpy(message,timings_divider_textedit[1]->text);
   uint32_t color=timings_divider_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_divider_textedit[1]->x,timings_divider_textedit[1]->y,timings_divider_textedit[1]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_bclk_textedit(void)
{
   strcpy(message,timings_divider_textedit[2]->text);
   uint32_t color=timings_divider_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_divider_textedit[2]->x,timings_divider_textedit[2]->y,timings_divider_textedit[2]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_cpuclk_textedit(void)
{
   strcpy(message,timings_divider_textedit[3]->text);
   uint32_t color=timings_divider_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_divider_textedit[3]->x,timings_divider_textedit[3]->y,timings_divider_textedit[3]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_clk90_textedit(void)
{
   strcpy(message,timings_divider_textedit[4]->text);
   uint32_t color=timings_divider_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_divider_textedit[4]->x,timings_divider_textedit[4]->y,timings_divider_textedit[4]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_multiplier_textedit(void)
{
   strcpy(message,timings_muldiv_textedit[0]->text);
   uint32_t color=timings_muldiv_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_muldiv_textedit[0]->x,timings_muldiv_textedit[0]->y,timings_muldiv_textedit[0]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}
uint32_t paint_timings_divider_textedit(void)
{
   strcpy(message,timings_muldiv_textedit[1]->text);
   uint32_t color=timings_muldiv_text_color_int(message);
   if(selected_tab==TAB_TIMINGS)
   {
      TEXTFIELD(timings_muldiv_textedit[1]->x,timings_muldiv_textedit[1]->y,timings_muldiv_textedit[1]->w,(Font->Height+1)+3,message,color);
   }
   Font->TextColor=0x00000000; // black

   return color;
}

void clear_timings_cursorpos(void)
{
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
      timings_phase_textedit[i]->cursor_pos=-1;
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      timings_divider_textedit[i]->cursor_pos=-1;
   for(int i=0;i<TIMINGS_DM_MAX;i++)
      timings_muldiv_textedit[i]->cursor_pos=-1;
}
void timings_pclk_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[0];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_phase_pclk_textedit();
}
void timings_clken_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[1];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_phase_clken_textedit();
}
void timings_bclk_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[2];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_phase_bclk_textedit();
}
void timings_cpuclk_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[3];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_phase_cpuclk_textedit();
}
void timings_clk90_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[4];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_phase_clk90_textedit();
}
void timings_emu_extra_phase_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_phase_textedit[5];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_PHASE_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_PHASE_MAX_LENGTH+1) b->cursor_pos=TIMINGS_PHASE_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_emu_extra_phase_textedit();
}

void timings_pclk_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_divider_textedit[0];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_pclk_textedit();
}
void timings_clken_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_divider_textedit[1];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_clken_textedit();
}
void timings_bclk_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_divider_textedit[2];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_bclk_textedit();
}
void timings_cpuclk_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_divider_textedit[3];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_cpuclk_textedit();
}
void timings_clk90_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_divider_textedit[4];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_clk90_textedit();
}
void timings_multiplier_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_muldiv_textedit[0];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_multiplier_textedit();
}
void timings_divider_textedit_action(void)
{
   clear_timings_cursorpos();
   TextEdit *b=timings_muldiv_textedit[1];
   b->cursor_pos=(mousex-b->x-4)/((b->w-4)/(TIMINGS_DIVIDER_MAX_LENGTH+3));
   if(b->cursor_pos<=1) b->cursor_pos=1;
   if(b->cursor_pos>TIMINGS_DIVIDER_MAX_LENGTH+1) b->cursor_pos=TIMINGS_DIVIDER_MAX_LENGTH+1;
   int len=strlen(b->text);
   if(b->cursor_pos>len+1)
      b->cursor_pos=len+1;
   paint_timings_divider_textedit();
}
void recalculate_coords_textedits(void)
{
   mac_textedit->x=win.x+10+94;
   mac_textedit->y=win.y+win.t+TAB_HEIGHT+10;

   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      preset_textedit[i]->x=win.x+20+94;
      preset_textedit[i]->y=win.y+win.t+TAB_HEIGHT+7+16+(Font->Height+5)*(i);
   }
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
   {
      timings_phase_textedit[i]->x=win.x+40+100;
      timings_phase_textedit[i]->y=win.y+40+win.t+TAB_HEIGHT+35+(Font->Height+5)*i;
   }
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
   {
      timings_divider_textedit[i]->x=win.x+40+208;
      timings_divider_textedit[i]->y=win.y+40+win.t+TAB_HEIGHT+35+(Font->Height+5)*i;
   }
   for(int i=0;i<TIMINGS_DM_MAX;i++)
   {
      timings_muldiv_textedit[i]->x=win.x+140+108*i;
      timings_muldiv_textedit[i]->y=win.y+win.t+TAB_HEIGHT+35;
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
   mac_textedit->timeout=25000;
   printf("timing_selected %d\n",timing_selected);
   for(int i=0;i<PRESET_CB_MAX;i++)
   {
      preset_textedit[i]=(TextEdit *)malloc(sizeof(TextEdit));
      preset_textedit[i]->w=((PRESET_MAX_LENGTH+4)*Font->Width)-1;
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
      preset_textedit[i]->timeout=25000;
   }

   sprintf(timings_phase_textedit[0]->text,"%d",cd[timing_selected].pclk.phase);
   sprintf(timings_phase_textedit[1]->text,"%d",cd[timing_selected].clken.phase);
   sprintf(timings_phase_textedit[2]->text,"%d",cd[timing_selected].bclk.phase);
   sprintf(timings_phase_textedit[3]->text,"%d",cd[timing_selected].cpuclk.phase);
   sprintf(timings_phase_textedit[4]->text,"%d",cd[timing_selected].clk90.phase);
   sprintf(timings_phase_textedit[5]->text,"%d",cd[timing_selected].emu_extra_phase);
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
   {
      timings_phase_textedit[i]=(TextEdit *)malloc(sizeof(TextEdit));
      timings_phase_textedit[i]->w=((TIMINGS_PHASE_MAX_LENGTH+4)*Font->Width)-1;
      timings_phase_textedit[i]->h=(Font->Height+1)+3;
      for(int j=strlen(timings_phase_textedit[i]->text);j<50;j++)
         timings_phase_textedit[i]->text[j]=0;
      timings_phase_textedit[i]->is_pressed=0;
      timings_phase_textedit[i]->b_was_at_cursor=0;
      timings_phase_textedit[i]->tab=TAB_TIMINGS;
      timings_phase_textedit[i]->cursor_pos=-1;
      timings_phase_textedit[i]->inverted=0;
      timings_phase_textedit[i]->timeout=250;
   }
   timings_phase_textedit[0]->action=timings_pclk_phase_textedit_action;
   timings_phase_textedit[1]->action=timings_clken_phase_textedit_action;
   timings_phase_textedit[2]->action=timings_bclk_phase_textedit_action;
   timings_phase_textedit[3]->action=timings_cpuclk_phase_textedit_action;
   timings_phase_textedit[4]->action=timings_clk90_phase_textedit_action;
   timings_phase_textedit[5]->action=timings_emu_extra_phase_textedit_action;

   sprintf(timings_divider_textedit[0]->text,"%d",cd[timing_selected].pclk.divider);
   sprintf(timings_divider_textedit[1]->text,"%d",cd[timing_selected].clken.divider);
   sprintf(timings_divider_textedit[2]->text,"%d",cd[timing_selected].bclk.divider);
   sprintf(timings_divider_textedit[3]->text,"%d",cd[timing_selected].cpuclk.divider);
   sprintf(timings_divider_textedit[4]->text,"%d",cd[timing_selected].clk90.divider);
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
   {
      timings_divider_textedit[i]=(TextEdit *)malloc(sizeof(TextEdit));
      timings_divider_textedit[i]->w=((TIMINGS_DIVIDER_MAX_LENGTH+4)*Font->Width)-1;
      timings_divider_textedit[i]->h=(Font->Height+1)+3;
      for(int j=strlen(timings_divider_textedit[i]->text);j<50;j++)
         timings_divider_textedit[i]->text[j]=0;
      timings_divider_textedit[i]->is_pressed=0;
      timings_divider_textedit[i]->b_was_at_cursor=0;
      timings_divider_textedit[i]->tab=TAB_TIMINGS;
      timings_divider_textedit[i]->cursor_pos=-1;
      timings_divider_textedit[i]->inverted=0;
      timings_divider_textedit[i]->timeout=250;
   }
   timings_divider_textedit[0]->action=timings_pclk_divider_textedit_action;
   timings_divider_textedit[1]->action=timings_clken_divider_textedit_action;
   timings_divider_textedit[2]->action=timings_bclk_divider_textedit_action;
   timings_divider_textedit[3]->action=timings_cpuclk_divider_textedit_action;
   timings_divider_textedit[4]->action=timings_clk90_divider_textedit_action;

   sprintf(timings_muldiv_textedit[0]->text,"%d",cd[timing_selected].M);
   sprintf(timings_muldiv_textedit[1]->text,"%d",cd[timing_selected].D);
   for(int i=0;i<TIMINGS_DM_MAX;i++)
   {
      timings_muldiv_textedit[i]=(TextEdit *)malloc(sizeof(TextEdit));
      timings_muldiv_textedit[i]->w=((TIMINGS_DIVIDER_MAX_LENGTH+4)*Font->Width)-1;
      timings_muldiv_textedit[i]->h=(Font->Height+1)+3;
      for(int j=strlen(timings_muldiv_textedit[i]->text);j<50;j++)
         timings_muldiv_textedit[i]->text[j]=0;
      timings_muldiv_textedit[i]->is_pressed=0;
      timings_muldiv_textedit[i]->b_was_at_cursor=0;
      timings_muldiv_textedit[i]->tab=TAB_TIMINGS;
      timings_muldiv_textedit[i]->cursor_pos=-1;
      timings_muldiv_textedit[i]->inverted=0;
      timings_muldiv_textedit[i]->timeout=250;
   }
   timings_muldiv_textedit[0]->action=timings_multiplier_textedit_action;
   timings_muldiv_textedit[1]->action=timings_divider_textedit_action;
   textedits_timings_reload();

}
void textedits_run(void)
{
   textedit_run(mac_textedit);
   for(int i=0;i<PRESET_CB_MAX;i++)
      textedit_run(preset_textedit[i]);
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
      textedit_run(timings_phase_textedit[i]);
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      textedit_run(timings_divider_textedit[i]);
   for(int i=0;i<TIMINGS_DM_MAX;i++)
      textedit_run(timings_muldiv_textedit[i]);
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
      b->time--;
      if(b->time<0)
      {
         b->time=b->timeout;
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
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
      textedit_action(timings_phase_textedit[i]);
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      textedit_action(timings_divider_textedit[i]);
   for(int i=0;i<TIMINGS_DM_MAX;i++)
      textedit_action(timings_muldiv_textedit[i]);
}
void ns_repaint(void)
{
   if(selected_tab!=TAB_TIMINGS)
      return;
   float phase,phase_int,divider,multiplier,base_MHz;
   float phase_f;
   multiplier=atoi(timings_muldiv_textedit[0]->text);
   divider=atoi(timings_muldiv_textedit[1]->text);
   int apply_timings_button_enabled=1;
   if(divider<=0)
   {
      for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      {
         sprintf(timings_ns[i],"----");
         sprintf(timings_MHz[i],"---");
      }
      apply_timings_button_enabled=0;
   }
   else
   {
      base_MHz=200*multiplier/divider;
      if(base_MHz>=800 && base_MHz<=1600)
         sprintf(timings_DM[0],"= %5.2f MHz  ",base_MHz);
      else
      {
         sprintf(timings_DM[0],"= ----- MHz  ");
         base_MHz=0;
         apply_timings_button_enabled=0;
      }

      divider=atoi(timings_divider_textedit[0]->text);
      if(divider>0 && base_MHz>0)
      {
         if(base_MHz/divider<200)
            sprintf(timings_MHz[0],"%5.2f  ",base_MHz/divider);
         else
         {
            sprintf(timings_MHz[0],"-----  ");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_MHz[0],"-----  ");
         apply_timings_button_enabled=0;
      }
      phase_int=atoi(timings_phase_textedit[0]->text);
      phase=phase_int;
      if(base_MHz>0)
         phase_f=1000.*divider/base_MHz/360.*phase;
      else
         phase_f=1000;
      if(phase_f<100)
      {
         if(base_MHz/divider<200)
            sprintf(timings_ns[0],"%+5.1f",phase_f);
         else
         {
            sprintf(timings_ns[0],"----");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_ns[0],"----");
         apply_timings_button_enabled=0;
      }

      divider=atoi(timings_divider_textedit[1]->text);
       if(divider>0 && base_MHz>0)
      {
         if(base_MHz/divider<200)
            sprintf(timings_MHz[1],"%5.2f  ",base_MHz/divider);
         else
         {
            sprintf(timings_MHz[1],"-----  ");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_MHz[1],"----  -");
         apply_timings_button_enabled=0;
      }
      phase_int=atoi(timings_phase_textedit[1]->text);
      phase=phase_int;
      if(base_MHz>0)
         phase_f=1000.*divider/base_MHz/360.*phase;
      else
         phase_f=1000;
      if(phase_f<100)
         sprintf(timings_ns[1],"%+5.1f",phase_f);
      else
      {
         sprintf(timings_ns[1],"----");
         apply_timings_button_enabled=0;
      }

      divider=atoi(timings_divider_textedit[2]->text);
       if(divider>0 && base_MHz>0)
      {
         if(base_MHz/divider<200)
            sprintf(timings_MHz[2],"%5.2f  ",base_MHz/divider);
         else
         {
            sprintf(timings_MHz[2],"-----  ");
            apply_timings_button_enabled=0;
         }
      }
      else
         sprintf(timings_MHz[2],"-----  ");
      phase_int=atoi(timings_phase_textedit[2]->text);
      phase=phase_int;
      if(base_MHz>0)
         phase_f=1000.*divider/base_MHz/360.*phase;
      else
         phase_f=1000;
      if(phase_f<100)
      {
         if(base_MHz/divider<200)
            sprintf(timings_ns[2],"%+5.1f",phase_f);
         else
         {
            sprintf(timings_ns[2],"----");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_ns[2],"----");
         apply_timings_button_enabled=0;
      }

      divider=atoi(timings_divider_textedit[3]->text);
      if(divider>0 && base_MHz>0)
      {
         if(base_MHz/divider<200)
            sprintf(timings_MHz[3],"%5.2f  ",base_MHz/divider);
         else
         {
            sprintf(timings_MHz[3],"-----  ");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_MHz[3],"-----  ");
         apply_timings_button_enabled=0;
      }

      phase_int=atoi(timings_phase_textedit[3]->text);
      phase=phase_int;
      if(base_MHz>0)
         phase_f=1000.*divider/base_MHz/360.*phase;
      else
         phase_f=1000;
      if(phase_f<100)
         sprintf(timings_ns[3],"%+5.1f",phase_f);
      else
      {
         sprintf(timings_ns[3],"----");
         apply_timings_button_enabled=0;
      }

      divider=atoi(timings_divider_textedit[4]->text);
      if(divider>0 && base_MHz>0)
      {
         if(base_MHz/divider<200)
            sprintf(timings_MHz[4],"%5.2f  ",base_MHz/divider);
         else
         {
            sprintf(timings_MHz[4],"-----  ");
            apply_timings_button_enabled=0;
         }
      }
      else
      {
         sprintf(timings_MHz[4],"-----  ");
         apply_timings_button_enabled=0;
      }
      phase_int=atoi(timings_phase_textedit[4]->text);
      phase=phase_int;
      if(base_MHz>0)
         phase_f=1000.*divider/base_MHz/360.*phase;
      else
         phase_f=1000;
      if(phase_f<100)
         sprintf(timings_ns[4],"%+5.1f",phase_f);
      else
      {
         sprintf(timings_ns[4],"----");
         apply_timings_button_enabled=0;
      }
   }

   for(int i=0;i<TIMINGS_TE_PHASE_MAX;i++) // WITHOUT +1 emu_extra
      displayStringAt(Font,win.x+40+160,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*i,(uint8_t*)timings_ns[i],LEFT_MODE);
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      displayStringAt(Font,win.x+40+270,win.y+40+win.t+TAB_HEIGHT+53-15+(Font->Height+5)*i,(uint8_t*)timings_MHz[i],LEFT_MODE);

   displayStringAt(Font,win.x+40+270-16,win.y+40+win.t+TAB_HEIGHT+13-15,(uint8_t*)timings_DM[0],LEFT_MODE);
   b_apply_timings->disabled=!apply_timings_button_enabled;
   b_apply_all_timings->disabled=!apply_timings_button_enabled;
}
void textedits_repaint(void)
{
   textedit_repaint(mac_textedit);
   for(int i=0;i<PRESET_CB_MAX-1;i++)
      textedit_repaint(preset_textedit[i]);
   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // +1 emu_extra
      textedit_repaint(timings_phase_textedit[i]);
   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      textedit_repaint(timings_divider_textedit[i]);
   for(int i=0;i<TIMINGS_DM_MAX;i++)
      textedit_repaint(timings_muldiv_textedit[i]);
   ns_repaint();
}
void textedits_timings_reload(void)
{
   sprintf(timings_phase_textedit[0]->text,"%d",cd[timing_selected].pclk.phase);
   sprintf(timings_phase_textedit[1]->text,"%d",cd[timing_selected].clken.phase);
   sprintf(timings_phase_textedit[2]->text,"%d",cd[timing_selected].bclk.phase);
   sprintf(timings_phase_textedit[3]->text,"%d",cd[timing_selected].cpuclk.phase);
   sprintf(timings_phase_textedit[4]->text,"%d",cd[timing_selected].clk90.phase);
   sprintf(timings_phase_textedit[5]->text,"%d",cd[timing_selected].emu_extra_phase);

   for(int i=0;i<TIMINGS_TE_PHASE_MAX+1;i++) // emu_extra
      for(int j=strlen(timings_phase_textedit[i]->text);j<50;j++)
         timings_phase_textedit[i]->text[j]=0;

   sprintf(timings_divider_textedit[0]->text,"%d",cd[timing_selected].pclk.divider);
   sprintf(timings_divider_textedit[1]->text,"%d",cd[timing_selected].clken.divider);
   sprintf(timings_divider_textedit[2]->text,"%d",cd[timing_selected].bclk.divider);
   sprintf(timings_divider_textedit[3]->text,"%d",cd[timing_selected].cpuclk.divider);
   sprintf(timings_divider_textedit[4]->text,"%d",cd[timing_selected].clk90.divider);

   for(int i=0;i<TIMINGS_TE_DIVIDER_MAX;i++)
      for(int j=strlen(timings_divider_textedit[i]->text);j<50;j++)
         timings_divider_textedit[i]->text[j]=0;

   sprintf(timings_muldiv_textedit[0]->text,"%d",cd[timing_selected].M);
   sprintf(timings_muldiv_textedit[1]->text,"%d",cd[timing_selected].D);

   for(int i=0;i<TIMINGS_DM_MAX;i++)
      for(int j=strlen(timings_muldiv_textedit[i]->text);j<50;j++)
         timings_muldiv_textedit[i]->text[j]=0;

   paint_timings_phase_pclk_textedit();
   paint_timings_phase_clken_textedit();
   paint_timings_phase_bclk_textedit();
   paint_timings_phase_cpuclk_textedit();
   paint_timings_phase_clk90_textedit();
   paint_timings_emu_extra_phase_textedit();
   paint_timings_divider_pclk_textedit();
   paint_timings_divider_clken_textedit();
   paint_timings_divider_bclk_textedit();
   paint_timings_divider_cpuclk_textedit();
   paint_timings_divider_clk90_textedit();
   paint_timings_multiplier_textedit();
   paint_timings_divider_textedit();
}
