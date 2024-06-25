#ifdef BOOT_MENU
#ifdef DEBUG_BOOTMENU
#define USE_SERIAL_OUTPUT
#endif

//#include "port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/libraries.h>

#include <proto/exec.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>

#include <graphics/text.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>

#include <graphics/gfxbase.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>

#include <clib/alib_protos.h>

#include <devices/timer.h>
//#include "device.h"
//#include "scsipiconf.h"
//#include "siopreg.h"
//#include "siopvar.h"
//#include "attach.h"
//#include "scsimsg.h"
//#include "battmem.h"
//#include "amigahw.h"
//#include "ndkcompat.h"
//#include "version.h"
//#include "mounter.h" // for Port/IOReq wrappers
//#include "a4091.h"

#include "z3660_scsi.h"
#include "scsimsg.h"

extern struct ExecBase *SysBase;
struct GfxBase *GfxBase;
struct Library *GadToolsBase;
struct Library *IntuitionBase;
static struct Screen *screen;
static struct Window *window;
static APTR visualInfo;
static struct Gadget *gadgets;
static struct Gadget *DisplayTypeGad;
static struct Gadget *LastAdded;
static struct NewGadget *NewGadget;

#define DISKS_BACK_ID        1
#define CONFIG_BACK_ID       2
#define ABOUT_BACK_ID        3
#define DEBUG_BACK_ID        4
#define MAIN_ABOUT_ID        5
#define MAIN_DISKS_ID        6
#define MAIN_CONFIG_ID       7
#define MAIN_DEBUG_ID        8
#define MAIN_BOOT_ID         9
#define DEBUG_CDROM_BOOT_ID 10
#define DEBUG_BOGUS_ID      11

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
#define WIDTH  640

static const struct TextAttr font_attr =
{
    (STRPTR)"topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};

static void init_bootmenu(void)
{
   UWORD pens = 0xffff;
   struct TagItem taglist[2];
   ULONG monitor_id;

   if (GfxBase->DisplayFlags & NTSC)
      monitor_id = NTSC_MONITOR_ID | HIRES_KEY;
   else
      monitor_id = PAL_MONITOR_ID | HIRES_KEY;

   taglist[0].ti_Tag  = VTAG_BORDERSPRITE_SET;
   taglist[0].ti_Data = TRUE;
   taglist[1].ti_Tag  = TAG_DONE;

   screen = OpenScreenTags(NULL,
                           SA_Depth,        2,
                           SA_Font,         &font_attr,
                           SA_Type,         CUSTOMSCREEN,
                           SA_DisplayID,    monitor_id,
                           SA_Interleaved,  TRUE,
                           SA_Draggable,    FALSE,
                           SA_Quiet,        TRUE,
                           SA_Pens,         &pens,
                           SA_VideoControl, taglist,
                           TAG_DONE);

   window = OpenWindowTags(NULL,
                           WA_IDCMP,         (IDCMP_RAWKEY | BUTTONIDCMP | LISTVIEWIDCMP | MXIDCMP),
                           WA_CustomScreen,  screen,
                           WA_Flags,         (WFLG_NOCAREREFRESH | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP),
                           TAG_DONE);

   visualInfo = GetVisualInfoA(screen,NULL);
}

static void close_libraries(void)
{
   CloseLibrary(IntuitionBase);
   CloseLibrary((struct Library *)GfxBase);
   CloseLibrary(GadToolsBase);
}

static void cleanup_bootmenu(void)
{
   CloseWindow(window);
   FreeVisualInfo(visualInfo);
   CloseScreen(screen);
   FreeGadgets(gadgets);
   close_libraries();
}

static struct Gadget *create_gadget_custom(UWORD kind, ULONG tag1, ...)
{
   return(CreateGadgetA(kind,LastAdded,NewGadget,(struct TagItem *)&tag1));
}

static struct Gadget *create_gadget(UWORD kind)
{
   return(create_gadget_custom(kind,TAG_DONE));
}

static void Print(STRPTR text, UWORD x, UWORD y, int center)
{
   struct RastPort *rp = &screen->RastPort;
   if (center)
      x += (WIDTH-TextLength(rp,text,strlen((char *)text)))/2;
   Move(rp,x,y);
   Text(rp,text,strlen((char *)text));
}

static void page_header(struct NewGadget *ng, STRPTR title, BOOL welcome)
{
   struct RastPort *rp = &screen->RastPort;
   if (gadgets)
   {
      RemoveGList(window,gadgets,-1);
      FreeGadgets(gadgets);
      DisplayTypeGad = NULL;
      LastAdded = NULL;
      SetRast(&screen->RastPort,0);
   }
   SetAPen(rp,2);
   Print(title,0,13,TRUE);

   gadgets = CreateContext(&LastAdded);

   NewGadget  = ng;
   ng->ng_TextAttr   = NULL;
   ng->ng_Flags      = 0;
   ng->ng_VisualInfo = visualInfo;
   ng->ng_Width      = 87;
   ng->ng_Height     = 14;
   ng->ng_TopEdge    = 183;

   if (welcome) {
      SetAPen(rp,1);
      Print((STRPTR)"Z3660: Accelerator/Emulator Board by sHaNsHe",0,33,TRUE);
      Print((STRPTR)"This project is based on parts from ZZ9000, Pistorm and ReA4091",0,33+8,TRUE);
   }
}

static void page_footer(void)
{
   AddGList(window,gadgets,-1,-1,NULL);
   RefreshGList(gadgets,window,NULL,-1);
   GT_RefreshWindow(window,NULL);
}

static void draw_dipswitch(UWORD x, UWORD y, UWORD off)
{
   struct RastPort *rp = &screen->RastPort;

   SetAPen(rp, 2);
   SetOPen(rp, 1);
   RectFill(rp, x+10, y, x+52,y+8);
   if (!off)
      x+= 20;
   SetAPen(rp, 0);
   RectFill(rp, x+12, y+2, x+30,y+6);
}

static char *dipswitch_text(int val, int num)
{
   static char string[25];
   string[0]=0;
   switch (num) {
      case 8: strcat((char *)string, "SCSI LUNs ");
         strcat((char *)string, val?"Disabled":"Enabled");
         break;
      case 7: strcat((char *)string, "External Termination ");
         strcat((char *)string, val?"On":"Off");
         break;
      case 6: strcat((char *)string, val?"S":"As");
         strcat((char *)string, "ynchronous SCSI Mode");
         break;
      case 5: strcat((char *)string, val?"Short":"Long");
         strcat((char *)string, " Spinup Mode");
         break;
      case 4: strcat((char *)string, val?"SCSI-2 Fast":"SCSI-1 Slow");
         strcat((char *)string, " Bus Mode");
         break;
      case 3:
      case 2:
      case 1: strcat((char *)string, "SCSI Address A? = ?");
         string[14] = num - 1 + '0';
         string[18] = val + '0';
         break;
   }

   return  string;
}

a4091_save_t   asave2;
a4091_save_t   *asave=&asave2;

static void draw_dipswitches(UWORD x, UWORD y)
{
   struct RastPort *rp = &screen->RastPort;
   int i;
   UBYTE dip_switches;
   char *ret, *num="8", *hostid="Host ID: 7";
   asave2.as_addr=0x50005000;

   if (asave) {
      dip_switches = *(uint8_t *)((asave->as_addr) + A4091_OFFSET_SWITCHES);
      printf("addr=%x\n",asave->as_addr);
      printf("dip_switches=%x\n",dip_switches);
   } else {
      dip_switches = 0xff;
   }

   SetAPen(rp, 1);
   SetBPen(rp, 0);
   Move(rp, x+16, y);
   Text(rp, (CONST_STRPTR)"Off/On",6);

   SetAPen(rp, 3);    // Set A pen color.
   SetOPen(rp, 1);
   RectFill(rp, x, y+2, x+70, y+90);

   for (i=0; i<8; i++) {
      draw_dipswitch(x+8, y+7+(i*10), dip_switches&BIT(7-i));
   }

   for (i=0; i<8; i++) {
      SetAPen(rp, 1);
      SetBPen(rp, 3);
      Move(rp, x+6,y+(i*10)+14);
      num[0] = '8' - i;
      Text(rp, (CONST_STRPTR)num, 1);
      Move(rp, x+82,y+(i*10)+14);
      SetBPen(rp, 0);
      ret = dipswitch_text(dip_switches&BIT(7-i), 8-i);
//      Text(rp, (CONST_STRPTR)ret, strlen((char *)ret));
      Text(rp, (CONST_STRPTR)"hola", strlen((char *)ret));
   }

   hostid[9]='0'+(dip_switches&7);
   SetAPen(rp, 1);
   SetBPen(rp, 0);
   Move(rp, x+280, y+64);
   Text(rp, (CONST_STRPTR)hostid,10);
   BNDRYOFF(rp);
}

static void config_page(void)
{
   struct NewGadget ng;
   page_header(&ng, (STRPTR)"Z3660 Configuration", TRUE);

   SetRGB4(&screen->ViewPort,3,11,8,8);

   ng.ng_LeftEdge   = 400;
   ng.ng_TopEdge    = 145;
   ng.ng_Width      = 120;
   ng.ng_GadgetText = (STRPTR)"Back";
   ng.ng_GadgetID   = CONFIG_BACK_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   DrawBevelBox(&screen->RastPort,100,52,440,115,
                GT_VisualInfo,  visualInfo,
                GTBB_Recessed,  TRUE,
                GTBB_FrameType, BBFT_RIDGE,
                TAG_DONE);
   draw_dipswitches(120,65);

   page_footer();
}

static void about_page(void)
{
   struct NewGadget ng;
   page_header(&ng, (STRPTR)"About A4091", TRUE);
   SetAPen(&screen->RastPort, 1);
   Print((STRPTR)"Thank you to Dave Haynie, Scott Schaeffer, Greg", 118,68,FALSE);
   Print((STRPTR)"Berlin and Terry Fisher for the A4091. Driver",118,76,FALSE);
   Print((STRPTR)"based on the NetBSD/Amiga SCSI subsystem and 53C710",118,84,FALSE);
   Print((STRPTR)"code by many fine contributors over the years.",118,92,FALSE);
   Print((STRPTR)"Original RDB mounter by Toni Wilen.",118,100,FALSE);
   Print((STRPTR)"Greetings to the Amiga community",118,122,FALSE);
   Print((STRPTR)"Only Amiga makes it possible.", 204,134,FALSE);
   Print((STRPTR)DEVICE_ID_STRING, 118,155,FALSE);

   ng.ng_LeftEdge   = 400;
   ng.ng_TopEdge    = 145;
   ng.ng_Width      = 120;
   ng.ng_GadgetText = (STRPTR)"Back";
   ng.ng_GadgetID   = ABOUT_BACK_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   DrawBevelBox(&screen->RastPort,100,52,440,115,
                GT_VisualInfo,  visualInfo,
                GTBB_Recessed,  TRUE,
                GTBB_FrameType, BBFT_RIDGE,
                TAG_DONE);
   page_footer();
}

#ifdef DEBUG_BOOTMENU
static char *
trim_spaces(char *str, size_t len)
{
   size_t olen = len;
   char *ptr;

   for (ptr = str; len > 0; ptr++, len--)
      if (*ptr != ' ')
         break;

   if (len == 0) {
      /* Completely empty name */
      *str = '\0';
      return (str);
   } else {
      memmove(str, ptr, len);

      while ((len > 0) && (str[len - 1] == ' '))
         len--;

      if (len < olen)  /* Is there space for a NIL character? */
         str[len] = '\0';
      return (str);
   }
   return (str);
}
#endif
static const char *
devtype_str(uint dtype)
{
   static const char const dt_list[][8] = {
                                           "Disk", "Tape", "Printer", "Proc",
                                           "Worm", "CDROM", "Scanner", "Optical",
                                           "Changer", "Comm", "ASCIT81", "ASCIT82",
   };
   if (dtype < ARRAY_SIZE(dt_list))
      return (dt_list[dtype]);
   return ("Unknown");
}

static char _itoabuf[12]; // MAXINT
int scan_disks(void)
{
   int i;
   struct MsgPort *port = NULL;
   struct IOExtTD *request = NULL;
   struct RastPort *rp = &screen->RastPort;

   int x,y;
   printf("Looking for disks!\n");

   port = W_CreateMsgPort(SysBase);
   if(!port) {
      printf("failed.\n");
      return 0;
   }

   request = (struct IOExtTD*)W_CreateIORequest(port, sizeof(struct IOExtTD), SysBase);
   if(!request) {
      printf("failed.\n");
      W_DeleteMsgPort(port, SysBase);
   }

   for (i=0; i<7; i++) { // FIXME LUNs?
//      ULONG lun = 0;
      x=52+8;
      y=52+(i*10);

      ULONG unitNum = i;
      printf("OpenDevice('%s', %" PRId32 ", %p, 0)\n", (STRPTR)DEVICE_NAME, unitNum, request);
      UBYTE err = OpenDevice((STRPTR)DEVICE_NAME, unitNum, (struct IORequest*)request, 0);
      if (err == 0) {
         scsi_inquiry_data_t inq_res;
         //:ret = -1;

         err = dev_scsi_inquiry(request, unitNum, &inq_res);
         if (err == 0) {
            char unit_str[]="0";
            unit_str[0]='0' + unitNum;
            Move(rp,x,y);
            Text(rp, (CONST_STRPTR)unit_str, 1);
            x+=48;
            Move(rp,x,y);
            Text(rp, inq_res.vendor, 8);
            x+=96;
            Move(rp,x,y);
            Text(rp, inq_res.product, 16);
            x+=176;
            Move(rp,x,y);
            Text(rp, inq_res.revision, 4);
            x+=48;
            Move(rp,x,y);
            const char *dtype=devtype_str(inq_res.device & SID_TYPE);
            Text(rp,(CONST_STRPTR)dtype,strlen(dtype));
            printf(" %-*.*s %-*.*s %-*.*s %-7s\n",
                   sizeof (inq_res.vendor),
                   sizeof (inq_res.vendor),
                   trim_spaces((char *)inq_res.vendor, sizeof (inq_res.vendor)),
                   sizeof (inq_res.product),
                   sizeof (inq_res.product),
                   trim_spaces((char *)inq_res.product, sizeof (inq_res.product)),
                   sizeof (inq_res.revision),
                   sizeof (inq_res.revision),
                   trim_spaces((char *)inq_res.revision, sizeof (inq_res.revision)),
                   devtype_str(inq_res.device & SID_TYPE));
         }

         struct DriveGeometry geom;
         if (dev_scsi_get_drivegeometry(request, &geom) == 0) {
            uint ssize = geom.dg_SectorSize;
            uint cap   = geom.dg_TotalSectors / 1024;
            uint cap_c = 0;  // KMGTPEZY
            if (cap > 100000) {
               cap /= 1024;
               cap_c++;
            }
            cap *= ssize;
            while (cap > 10239) {
               cap /= 1024;
               cap_c++;
            }
            printf("%5u %5u %cB\n", ssize, cap, "KMGTPEZY"[cap_c]);

            x+=76-8-8;
            Move(rp,x,y);
            if (ssize<1000)
               Text(rp,(CONST_STRPTR)" ",1);
            itoa(ssize,_itoabuf,10);
            Text(rp,(CONST_STRPTR)_itoabuf,strlen(_itoabuf));

            x+=48+8;
            Move(rp,x,y);
            if(cap<1000)
               Text(rp,(CONST_STRPTR)" ",1);
            if(cap<100)
               Text(rp,(CONST_STRPTR)" ",1);
            itoa(cap,_itoabuf,10);
            Text(rp,(CONST_STRPTR)_itoabuf,strlen(_itoabuf));
            const char caps[]="KMGTPEZY";
            Text(rp,(CONST_STRPTR)&caps[cap_c],1);
            Text(rp,(CONST_STRPTR)"B",1);
         }
         CloseDevice((struct IORequest*)request);
      }

   }

   W_DeleteIORequest(request, SysBase);
   W_DeleteMsgPort(port, SysBase);
   return 0;
}

static const struct Rectangle disk_table[] =
{
    { 40, 28,560,148},
    { 42, 29, 49, 12}, // Unit
    { 42, 42, 49,133},
    { 92, 29, 95, 12}, // Vendor
    { 92, 42, 95,133},
    {188, 29,175, 12}, // Device
    {188, 42,175,133},
    {364, 29, 49, 12}, // Revision
    {364, 42, 49,133},
    {414, 29, 71, 12}, // Type
    {414, 42, 71,133},
    {486, 29, 47, 12}, // ssize
    {486, 42, 47,133},
    {534, 29, 64, 12}, // Cap
    {534, 42, 64,133}
};

static void disks_page(void)
{
   struct NewGadget ng;
   int i;
   ULONG tag;
   page_header(&ng, (STRPTR)"Z3660 Diagnostics - Disks", FALSE);

   SetRGB4(&screen->ViewPort,3,6,8,11);

   SetAPen(&screen->RastPort,2);
   Print((STRPTR)"Unit  Vendor      Device                Rev.  Type     Blk   Size",52,38,FALSE);
   SetAPen(&screen->RastPort,1);

   ng.ng_LeftEdge   = 400;
   ng.ng_TopEdge    = 185;
   ng.ng_Width      = 120;
   ng.ng_GadgetText = (UBYTE *)"Back";
   ng.ng_GadgetID   = DISKS_BACK_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   tag=GTBB_Recessed;
   for (i = 0; i < 15; i++)
   {
      DrawBevelBox(&screen->RastPort,disk_table[i].MinX,disk_table[i].MinY,
                   disk_table[i].MaxX+1,
                   disk_table[i].MaxY+1,
                   GT_VisualInfo,  visualInfo,
                   tag,            TRUE,
                   TAG_DONE);
      tag = TAG_IGNORE;
   }

   page_footer();

   scan_disks();
}

static void debug_page(void)
{
   struct NewGadget ng;
   page_header(&ng, (STRPTR)"Z3660 Diagnostics - Debug", TRUE);

   BOOL cdrom_boot = asave->cdrom_boot ? TRUE : FALSE;
   SetRGB4(&screen->ViewPort,3,6,8,11);

   ng.ng_LeftEdge   = 400;
   ng.ng_TopEdge    = 60;
   ng.ng_Width      = 175;
   ng.ng_GadgetText = (UBYTE *)"CDROM Boot";
   ng.ng_GadgetID   = DEBUG_CDROM_BOOT_ID;
   LastAdded = create_gadget(CHECKBOX_KIND);
   GT_SetGadgetAttrs(LastAdded, NULL, NULL, GTCB_Checked, cdrom_boot, TAG_DONE);

   ng.ng_TopEdge    = 76;
   ng.ng_GadgetText = (UBYTE *)"Zorro III magic speed hack";
   ng.ng_GadgetID   = DEBUG_BOGUS_ID;
   LastAdded = create_gadget(CHECKBOX_KIND);
   GT_SetGadgetAttrs(LastAdded, NULL, NULL, GA_Disabled, TRUE, TAG_DONE);

   ng.ng_LeftEdge   = 400;
   ng.ng_TopEdge    = 145;
   ng.ng_Width      = 120;
   ng.ng_GadgetText = (UBYTE *)"Back";
   ng.ng_GadgetID   = DEBUG_BACK_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   DrawBevelBox(&screen->RastPort,100,52,440,115,
                GT_VisualInfo,  visualInfo,
                GTBB_Recessed,  TRUE,
                GTBB_FrameType, BBFT_RIDGE,
                TAG_DONE);

   page_footer();
}

struct drawing {
    BYTE type, pen; // 1: rectangle filled 2: rectangle empty -1: end
    SHORT x, y, w, h;
};

static const struct drawing card[] = {
    { 1, 3,   0,   0, 430, 145 }, // card
    { 1, 3,  57, 145, 163,  10 }, // zslot
    { 1, 2, 431,  16,   1, 155 }, // bracket
    { 1, 2, 431,  16,  12,   2 },
    { 2, 2, 223,  14, 187, 128 }, // silkscreen
    { 1, 0, 270,  64,  90,  30 }, // cutout
    { 2, 1, 414,  41,  16,  56 }, // connectors back
    { 2, 1, 415, 108,  15,  33 },
    { 1, 1,  12,  12,  18,  18 }, // U304 (GALs)
    { 1, 1,  12,  38,  18,  18 }, // U203
    { 1, 1,  12,  64,  18,  18 }, // U305
    { 1, 1,  38,  12,  18,  18 }, // U207
    { 1, 1,  38,  38,  18,  18 }, // U306
    { 1, 1,  64,  12,  18,  18 }, // U205
    { 1, 1,  64,  38,  18,  18 }, // U202
    { 1, 1,  64,  64,  18,  18 }, // U303
    { 1, 1,  88,   6,   2,  10 }, // J100
    { 1, 1,  97,  13,  18,  44 }, // U206 (ROM)
    { 1, 1, 126,   4,  80,   8 }, // CN309
    { 1, 1, 132,  30,  42,  44 }, // U300 (LSI)
    { 1, 2,  38,  80,  18,  26 }, // U301 (OSC)
    { 1, 1,  50,  66,   6,  10 }, // U302
    { 1, 1,  23, 116,  11,  16 }, // U109
    { 1, 1,  62,  90,  10,  16 }, // U200
    { 1, 1,  70, 116,  10,  16 }, // U201
    { 1, 1,  97,  88,  10,  18 }, // U105
    { 1, 1,  96, 116,  10,  18 }, // U100
    { 1, 1, 122,  88,  10,  18 }, // U108
    { 1, 1, 122, 116,  10,  18 }, // U101
    { 1, 1, 140,  88,  10,  18 }, // U106
    { 1, 1, 142, 116,  10,  18 }, // U102
    { 1, 1, 160,  90,  10,  16 }, // U204
    { 1, 1, 160, 116,  10,  18 }, // U107
    { 1, 1, 184,  90,  10,  16 }, // U213
    { 1, 1, 182, 118,  10,  16 }, // U103
    { 1, 1, 208,  90,   6,  10 }, // U307
    { 1, 1, 190,  28,   8,  13 }, // U311
    { 1, 1, 188,  47,   8,  13 }, // U310
    { 1, 2, 198, 114,   2,  24 }, // power connector
    { 1, 2, 201, 116,   0,  20 },
    { 3, 2,  57, 145, 163,  10 }  // the end
};

static void draw_card(const struct drawing c[], int length)
{
   struct RastPort *rp = &screen->RastPort;
   int x=106, y=50, i,j;

   for (i=0; i<length; i++) {
      struct drawing d = c[i];
      UBYTE type=d.type, pen=d.pen;
      SHORT x1=x+d.x, y1=y+d.y/2, x2=x+d.x+d.w, y2=y+d.y/2+d.h/2;
      SetAPen(rp, pen);
      switch (type) {
      case 1:
         RectFill(rp, x1, y1, x2, y2);
         break;
      case 2:
         Move(rp, x1, y1);
         Draw(rp, x2, y1);
         Draw(rp, x2, y2);
         Draw(rp, x1, y2);
         Draw(rp, x1, y1);
         break;
      case 3:
         SetAPen(rp, 2);
         for(j=x1; j<x2; j+=4) {
            RectFill(rp, j, y1, j+1, y2);
         }
      }
   }
}

static void main_page(void)
{
   struct NewGadget ng;

   SetRGB4(&screen->ViewPort,3,6,8,11);
   page_header(&ng, (STRPTR)"Z3660 Early Startup Menu", TRUE);

   draw_card(card, ARRAY_LENGTH(card));

   ng.ng_LeftEdge   = 140;
   ng.ng_TopEdge    = 150;
   ng.ng_Width      = 175;
   ng.ng_GadgetText = (UBYTE *)"Disks";
   ng.ng_GadgetID   = MAIN_DISKS_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   ng.ng_TopEdge    = 170;
   ng.ng_GadgetText = (UBYTE *)"Configuration";
   ng.ng_GadgetID   = MAIN_CONFIG_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   ng.ng_TopEdge    = 150;
   ng.ng_LeftEdge   = 325;
   ng.ng_GadgetText = (UBYTE *)"About";
   ng.ng_GadgetID   = MAIN_ABOUT_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   ng.ng_TopEdge= 170;
   ng.ng_GadgetText = (UBYTE *)"Debug";
   ng.ng_GadgetID   = MAIN_DEBUG_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   ng.ng_TopEdge= 200;
   if (GfxBase->DisplayFlags & NTSC) {
      ng.ng_TopEdge  = 170;
      ng.ng_LeftEdge = 538;
      ng.ng_Width    =  88;
   }
   ng.ng_GadgetText = (UBYTE *)"Boot";
   ng.ng_GadgetID   = MAIN_BOOT_ID;
   LastAdded = create_gadget(BUTTON_KIND);

   DrawBevelBox(&screen->RastPort,125,140,390,55,
                GT_VisualInfo,  visualInfo,
                GTBB_Recessed,  TRUE,
                GTBB_FrameType, BBFT_RIDGE,
                TAG_DONE);
   page_footer();
}

static void event_loop(void)
{
   ULONG class;
   UWORD icode;
   struct IntuiMessage *msg;
   struct Gadget *gad;
   int running = TRUE;

   while (running) {
      WaitPort(window->UserPort);
      if ((msg = GT_GetIMsg(window->UserPort))) {
         class = msg->Class;
         icode = msg->Code;
         gad = msg->IAddress;
         GT_ReplyIMsg(msg);
         switch (class) {
            case IDCMP_RAWKEY:
               if (icode == 0x45) // key down ESC
                  running = FALSE;
               break;
            case IDCMP_GADGETUP:
               switch (gad->GadgetID)
               {
                  case MAIN_DISKS_ID:
                     disks_page();
                     break;
                  case MAIN_CONFIG_ID:
                     config_page();
                     break;
                  case MAIN_ABOUT_ID:
                     about_page();
                     break;
                  case MAIN_DEBUG_ID:
                     debug_page();
                     break;
                  case DISKS_BACK_ID:
                  case CONFIG_BACK_ID:
                  case ABOUT_BACK_ID:
                  case DEBUG_BACK_ID:
                     main_page();
                     break;
                  case MAIN_BOOT_ID:
                     running=FALSE;
                     break;
                  case DEBUG_CDROM_BOOT_ID:
                     asave->cdrom_boot=gad->Flags&GFLG_SELECTED?TRUE:FALSE;
                     Save_BattMem();
                     break;
               }
         }
      }
   }
}
void delete_timer(struct timerequest *tr )
{
   struct MsgPort *tp;

   if (tr != 0 )
   {
      tp = tr->tr_node.io_Message.mn_ReplyPort;

      if (tp != 0)
         DeletePort(tp);

      CloseDevice( (struct IORequest *) tr );
      DeleteExtIO( (struct IORequest *) tr );
   }
}
void wait_for_timer(struct timerequest *tr, struct timeval *tv )
{

   tr->tr_node.io_Command = TR_ADDREQUEST; /* add a new timer request */

   /* structure assignment */
   tr->tr_time = *tv;

   /* post request to the timer -- will go to sleep till done */
   DoIO((struct IORequest *) tr );
}
struct timerequest *create_timer( ULONG unit )
{
   /* return a pointer to a timer request.  If any problem, return NULL */
   LONG error;
   struct MsgPort *timerport;
   struct timerequest *TimerIO;

   timerport = CreatePort( 0, 0 );
   if (timerport == NULL )
      return( NULL );

   TimerIO = (struct timerequest *)
          CreateExtIO( timerport, sizeof( struct timerequest ) );
   if (TimerIO == NULL )
   {
      DeletePort(timerport);   /* Delete message port */
      return( NULL );
   }

   error = OpenDevice( (CONST_STRPTR)TIMERNAME, unit,(struct IORequest *) TimerIO, 0L );
   if (error != 0 )
   {
      delete_timer( TimerIO );
      return( NULL );
   }
   return( TimerIO );
}

LONG time_delay( struct timeval *tv, LONG unit )
{
   struct timerequest *tr;
   /* get a pointer to an initialized timer request block */
   tr = create_timer( unit );

   /* any nonzero return says timedelay routine didn't work. */
   if (tr == NULL )
      return( -1L );

   wait_for_timer( tr, tv );

   /* deallocate temporary structures */
   delete_timer( tr );
   return( 0L );
}
void boot_menu(void)
{
   SysBase = *(struct ExecBase **)4UL;
   printf("Bootmenu:\n");
   // Hack!?
   InitResident(FindResident((CONST_STRPTR)"gadtools.library"), 0);

   IntuitionBase = OpenLibrary((CONST_STRPTR)"intuition.library", 0);
   GfxBase       = (struct GfxBase *)OpenLibrary((CONST_STRPTR)"graphics.library",0);
   GadToolsBase  = OpenLibrary((CONST_STRPTR)"gadtools.library",36);

   /* Check left mouse button */
   if (!(REG_CIAAPRA_PA6 & *((volatile char *)REG_CIAAPRA))) {
      printf("LMB pressed.\n");
      close_libraries();
      return;
   }

   /*
    * Configure Paula POTGO LX + LY pins (Port 0 Button 2 + 3) as
    * output high. Paula implements a pull-up on the pins in this
    * mode, which may be overdriven (low) by a mouse button press.
    */
   *(volatile UWORD *)REG_POTGO = 0x0f00;

   /* Wait for Paula to refresh GPIO state */
   struct timeval currentval;
   currentval.tv_secs = 0;
   currentval.tv_micro = 20; // 20 ticks (based on VBLANK)
   time_delay( &currentval, UNIT_VBLANK );

   /* Check right mouse button */
   if (REG_POTGOR_DATLY & *(volatile UWORD *)REG_POTGOR) {
      printf("RMB mouse not pressed.\n");
      close_libraries();
      return;
   }

   printf("Bootmenu: enter\n");
   init_bootmenu();
   main_page();
   event_loop();
   cleanup_bootmenu();
   printf("Bootmenu: exit\n");
   ColdReboot();
   return;
}

int Save_BattMem(void)
{
   return(0);
}

APTR W_CreateIORequest(struct MsgPort *ioReplyPort, ULONG size, struct ExecBase *SysBase)
{
   struct IORequest *ret = NULL;
   if(ioReplyPort == NULL)
      return NULL;
   ret = (struct IORequest*)AllocMem(size, MEMF_PUBLIC | MEMF_CLEAR);
   if(ret != NULL)
   {
      ret->io_Message.mn_ReplyPort = ioReplyPort;
      ret->io_Message.mn_Length = size;
   }
   return ret;
}
void W_DeleteIORequest(APTR iorequest, struct ExecBase *SysBase)
{
   if(iorequest != NULL) {
      FreeMem(iorequest, ((struct Message*)iorequest)->mn_Length);
   }
}
struct MsgPort *W_CreateMsgPort(struct ExecBase *SysBase)
{
   struct MsgPort *ret;
   ret = (struct MsgPort*)AllocMem(sizeof(struct MsgPort), MEMF_PUBLIC | MEMF_CLEAR);
   if(ret != NULL)
   {
      BYTE sb = AllocSignal(-1);
      if (sb != -1)
      {
         ret->mp_Flags = PA_SIGNAL;
         ret->mp_Node.ln_Type = NT_MSGPORT;
         NewList(&ret->mp_MsgList);
         ret->mp_SigBit = sb;
         ret->mp_SigTask = FindTask(NULL);
         return ret;
      }
      FreeMem(ret, sizeof(struct MsgPort));
   }
   return NULL;
}
void W_DeleteMsgPort(struct MsgPort *port, struct ExecBase *SysBase)
{
   if(port != NULL)
   {
      FreeSignal(port->mp_SigBit);
      FreeMem(port, sizeof(struct MsgPort));
   }
}
void exit(int humm)
{
   while(1);
}
#endif