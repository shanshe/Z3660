#ifndef _ZTOP_H_
#define _ZTOP_H_
#include "../rtg/gfx.h"
#include "../rtg/fonts.h"

#include "win.h"
#include "drag.h"
#include "textfield.h"

#define NUM_TABS 6

typedef enum {
   TAB_INFO=1,
   TAB_BOOT,
   TAB_SCSI,
   TAB_MISC,
   TAB_PRESET,
   TAB_TIMINGS,
} Tabs;

#define FILLRECT(x,y,w,h,c) fill_rect_solid(x,y,w,h,\
      argb888_to_rgb565(c),MNTVA_COLOR_16BIT565)

#define PIXEL_DISABLED(x,y,w,h,c) fill_rect(x,y,w,h,\
      argb888_to_rgb565(c),MNTVA_COLOR_16BIT565,0xAA)

#define INVERT_RECT(x,y,w,h) invert_rect(x,y,w,h,\
      0xFF,MNTVA_COLOR_16BIT565)

#endif // _ZTOP_H_
