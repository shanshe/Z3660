#include "slider.h"
#include "tabs.h"
#include <stdlib.h>
#include <stdio.h>
#include "../config_file.h"

extern int selected_tab;
extern int16_t mousex,mousey;
extern WIN win;
extern char message[300];
extern sFONT *Font;

Slider *slider_cpufreq;
Slider *slider_bpton;
Slider *slider_bptoff;

int8_t is_cursor_at_slider(Slider *s)
{
   if(mousex>=s->x && mousex<=s->x+s->w
   && mousey>=s->y && mousey<=s->y+s->h)
   {
      return(1);
   }
   return(0);
}

void slider_run(Slider *s)
{
   if( selected_tab==s->tab && is_cursor_at_slider(s) )
      s->is_pressed=1;
   else
      s->is_pressed=0;
}
void slider_action(Slider *s)
{
   if(s->is_pressed==0)
      return;
   s->is_pressed=0;
   s->b_was_at_cursor=0;
   if( selected_tab==s->tab && is_cursor_at_slider(s) )
   {
      s->pos=(mousex-s->x-3)/((s->w-6)/((s->max-s->min)/s->step))*s->step+s->min;
      if(s->pos>s->max) s->pos=s->max;
      if(s->pos<s->min) s->pos=s->min;
      SLIDER(s);
      s->action();
   }
}
void slider_repaint(Slider *s)
{
   if(selected_tab!=s->tab)
      return;
   if(s->is_pressed==0)
      return;
   uint8_t s_is_at_cursor=is_cursor_at_slider(s);
   s->b_was_at_cursor=s_is_at_cursor;
}

void sliders_run(void)
{
   slider_run(slider_cpufreq);
   slider_run(slider_bpton);
   slider_run(slider_bptoff);
}
void slider_cpufreq_action(void)
{
}

void slider_bpton_action(void)
{
   test_beeper(ENV_INT_TON,ENV_INT_TOFF);
}
void slider_bptoff_action(void)
{
   test_beeper(ENV_INT_TON,ENV_INT_TOFF);
}
void slider_cpufreq_strpos(char *str)
{
   sprintf(str,"%3d MHz",slider_cpufreq->pos);
}
void slider_bpton_strpos(char *str)
{
   load_config_from_ztop();
   if(slider_bpton->pos==0)
      sprintf(str," Disabled ");
   else
      sprintf(str,"%7.1f us",(slider_bpton->pos*TON_MAX*1e6)/slider_bpton->max);
}
void slider_bptoff_strpos(char *str)
{
   load_config_from_ztop();
   sprintf(str,"%7.1f ms",(slider_bptoff->pos*TOFF_MAX*1e3)/slider_bptoff->max);
}

void init_sliders(void)
{
   slider_cpufreq=(Slider *)malloc(sizeof(Slider));
   slider_cpufreq->w=(CPUFREQ_MAX-CPUFREQ_MIN)*2;
   slider_cpufreq->h=14;
   slider_cpufreq->is_pressed=0;
   slider_cpufreq->b_was_at_cursor=0;
   slider_cpufreq->action=slider_cpufreq_action;
   slider_cpufreq->tab=TAB_BOOT;
   slider_cpufreq->max=CPUFREQ_MAX;
   slider_cpufreq->min=CPUFREQ_MIN;
   slider_cpufreq->step=5;
   slider_cpufreq->strpos=slider_cpufreq_strpos;
   slider_cpufreq->text_pos=0;

   slider_bpton=(Slider *)malloc(sizeof(Slider));
   slider_bpton->w=200;
   slider_bpton->h=14;
   slider_bpton->is_pressed=0;
   slider_bpton->b_was_at_cursor=0;
   slider_bpton->action=slider_bpton_action;
   slider_bpton->tab=TAB_MISC;
   slider_bpton->max=SLIDER_BPTON_MAX;
   slider_bpton->min=0;
   slider_bpton->step=SLIDER_BPTON_STEP;
   slider_bpton->strpos=slider_bpton_strpos;
   slider_bpton->text_pos=1;

   slider_bptoff=(Slider *)malloc(sizeof(Slider));
   slider_bptoff->w=200;
   slider_bptoff->h=14;
   slider_bptoff->is_pressed=0;
   slider_bptoff->b_was_at_cursor=0;
   slider_bptoff->action=slider_bpton_action;
   slider_bptoff->tab=TAB_MISC;
   slider_bptoff->max=SLIDER_BPTOFF_MAX;
   slider_bptoff->min=0;
   slider_bptoff->step=SLIDER_BPTOFF_STEP;
   slider_bptoff->strpos=slider_bptoff_strpos;
   slider_bptoff->text_pos=1;
}
void sliders_repaint(void)
{
   slider_repaint(slider_cpufreq);
   slider_repaint(slider_bpton);
   slider_repaint(slider_bptoff);
}
void sliders_action(void)
{
   slider_action(slider_cpufreq);
   slider_action(slider_bpton);
   slider_action(slider_bptoff);
}
void recalculate_coords_sliders(void)
{
   slider_cpufreq->x=win.x+300-(CPUFREQ_MAX-100)*2;
   slider_cpufreq->y=win.y+win.t+tab_h+10+12;

   slider_bpton->x=win.x+104;
   slider_bpton->y=win.y+win.t+tab_h+10+12+14;

   slider_bptoff->x=win.x+104;
   slider_bptoff->y=win.y+win.t+tab_h+10+12+14+14+10;
}
void paint_slider_cpufreq(void)
{
   Font->BackColor=0x00A8A8A8;
   Font->TextColor=0x00000000;
   displayStringAt(Font,win.x+12+30,win.y+win.t+tab_h+10,(uint8_t*)"Boot Mode Selection",LEFT_MODE);
   displayStringAt(Font,win.x+200-(CPUFREQ_MAX-100)*2,win.y+win.t+tab_h+10+12+2,(uint8_t*)"CPU Frequency",LEFT_MODE);
   SLIDER(slider_cpufreq);
}
void paint_slider_bpton(void)
{
   Font->BackColor=0x00A8A8A8;
   Font->TextColor=0x00000000;
   displayStringAt(Font,win.x+18,win.y+win.t+tab_h+10+12+2+12,(uint8_t*)"Beeper Ton",LEFT_MODE);
   SLIDER(slider_bpton);
}
void paint_slider_bptoff(void)
{
   Font->BackColor=0x00A8A8A8;
   Font->TextColor=0x00000000;
   displayStringAt(Font,win.x+18,win.y+win.t+tab_h+10+12+2+12+12+12,(uint8_t*)"Beeper Toff",LEFT_MODE);
   SLIDER(slider_bptoff);
}
