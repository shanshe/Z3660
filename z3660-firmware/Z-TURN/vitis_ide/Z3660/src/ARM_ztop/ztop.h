#ifndef _ZTOP_H_
#define _ZTOP_H_
#include "../rtg/gfx.h"
#include "../rtg/fonts.h"

#include "win.h"
#include "drag.h"
#include "textfield.h"
#include <stdio.h>

#define NUM_TABS 7

typedef enum {
   TAB_INFO=1,
   TAB_BOOT,
   TAB_SCSI,
   TAB_ADF,
   TAB_MISC,
   TAB_PRESET,
   TAB_TIMINGS,
} Tabs;

#define FILLRECT(x,y,w,h,c) do{if(w>0 && h>0) {                                                      \
                                  fill_rect_solid(x,y,w,h,argb888_to_rgb565(c),MNTVA_COLOR_16BIT565);\
                               }                                                                     \
                               /*printf("x %d y %d w %d h %d\n",x,y,w,h);*/                          \
                              }while(0)

#define PIXEL_DISABLED(x,y,w,h,c) fill_rect(x,y,w,h,\
      argb888_to_rgb565(c),MNTVA_COLOR_16BIT565,0xAA)

#define INVERT_RECT(x,y,w,h) invert_rect(x,y,w,h,\
      0xFF,MNTVA_COLOR_16BIT565)

#endif // _ZTOP_H_
