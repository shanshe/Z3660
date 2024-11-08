/* Wazp3D Beta 56 : Alain THELLIER - Paris - FRANCE - (November 2006 to 2014)     */
/* Code clean-up and library enhancements from Gunther Nikl                 */
/* Adaptation to AROS from Matthias Rustler                            */
/* Adaptation to Morphos from Szil�rd 'BSzili' Bir�                         */
/* LICENSE: GNU General Public License (GNU GPL) for this file                */

/* This file contain the Warp3D -> Wazp3D wrapper                            */

/*==========================================================================*/
#include "Wazp3D.h"

struct WAZP3D_parameters parameters;
struct WAZP3D_parameters *Wazp3D=&parameters;

UBYTE Wazp3DPrefsName[]  ={PREFSNAME};
IPTR  Wazp3DPrefsNameTag =(IPTR) Wazp3DPrefsName;
struct memory3D *firstME=NULL;        /* Tracked memory-allocation            */

struct WAZP3D_texture *DebugWT=NULL;

#ifdef SOFT3DLIB

/* then soft3d functions are compiled as a separate library */
#include "soft3d/soft3d_protos.h"
//#include "soft3d/soft3d.h"

#else

/* else soft3d functions are inside Wazp3D */
#include "soft3d/soft3d56.c"

#endif
/*==================================================================================*/
struct WAZP3D_texture{
    UBYTE *pt;
    UWORD large,high,bits,format;
    UBYTE name[40];
    UBYTE BlendMode;
    UWORD Tnum;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } EnvRGBA;
    UBYTE BorderRGBA[4];
    UBYTE ChromaTestMinRGBA[4];
    UBYTE ChromaTestMaxRGBA[4];
    ULONG ChromaTestMode;
    ULONG MinFilter,MagFilter,TexEnv;
    BOOL  MinFiltering,MagFiltering,MinMipmapping;
    BOOL  Smode,Tmode;
    void *nextWT;        /* next WAZP3D_texture */
    void *ST;            /* link to this SOFT3D_texture */
    W3D_Texture    texture;    /* include this Warp3D texture */
};
/*==================================================================================*/
struct WAZP3D_blendstage{
    UBYTE ColorInputA,AlphaInputA;
    UBYTE ColorInputB,AlphaInputB;
    UBYTE ColorInputC,AlphaInputC;
    UBYTE TexEnvMode,padding;
    UBYTE ColorCombineMode,AlphaCombineMode;
    UBYTE FactorRGBA[4];    /* a constant blend factor from a W3D_Color structure     */
    UBYTE ScaleRGBA[4];    /* multiply result with this factor ( value is 1 or 2 or 4)    */
};
/*==================================================================================*/
__attribute__((aligned(16)))
struct WAZP3D_context{
    struct WAZP3D_texture *WT;        /* binded one */
    float uresize,vresize;            /* for binded tex */
    struct WAZP3D_texture *firstWT;    /* list of all textures */
    struct WAZP3D_blendstage Stage[MAXSTAGE];    /* V5: multitexturing blending stages */
    BOOL blendstagesready;
    BOOL only_for_padding;
    void *texture;

    BOOL CallFlushFrame;            /* then DoUpdate inside bitmap*/
    BOOL CallSetDrawRegion;
    BOOL CallClearZBuffer;
    BOOL CallSetBlending;
    struct point3D P[MAXPRIM];
    ULONG Pnb;

    struct  face3D *DumpF;                        /* to dump object */
    struct point3D *DumpP;
    struct state3D state;
    struct point3D PolyP[MAXPOLY];
    WORD PolyPnb;
    UWORD Tnb,DumpStage;
    ULONG DumpFnum,DumpPnum,DumpFnb,DumpPnb;        /* Faces & Points count. */

    struct Window *window;                    /* Current window and window->WScreen is this Window's Screen */
    WORD windowX,windowY;
    struct RastPort rastport;                /*  include current bitmap  */
    struct TextFont *font;
    ULONG ModeID;
    UBYTE *bmdata;                            /*  bitmap memory  */
    ULONG  bmformat;                            /*  bitmap format  */
    ULONG  yoffset;                            /*  bitmap offset  */

    HOOKEDFUNCTION SOFT3D_SetBitmap_SetClippingHook;

    UWORD large,high,bits;
    WORD Xmin,Ymin,Xmax,Ymax;                    /* screen scissor */

    UBYTE hints[32];

    ULONG AlphaMode,AlphaRef;
    ULONG SrcFunc,DstFunc;
    ULONG LogicOp;
    ULONG PenMask;

    float FogWmin,FogWmax,FogDensityW;

    ULONG    StippleLine,StippleLineFactor,StipplePolygon;
    ULONG    StencilFunc,StencilRef,StencilMask,StencilSfail,StencilZfail,StencilZpass,StencilWriteMask;
    ULONG ZCompareMode;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } EnvRGBA;            /* global one */
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } MaskRGBA;
    void *ILpointer;            /* current interleaved-array settings */
    ULONG ILvertexFormat;
    ULONG ILflags;
    int   ILstride;

    struct WAZP3D_texture *DebuggedWT;
    UWORD Fps;
    UWORD TimePerFrame;
    ULONG LastMilliTime;
    ULONG PrimitivesDrawn;
    void *SC;
    W3D_Context context;                    /* The Warp3D's context is inside the Wazp3D's context WC */
};
/*==================================================================================*/
/* internal Wazp3D private functions */
void ConvertBitmap(ULONG format,UBYTE *pt1,UBYTE *pt2,UWORD high,UWORD large,ULONG offset1,ULONG offset2,UBYTE *palette);
void DoUpdate(W3D_Context *context);
void DrawText(W3D_Context *context,WORD x,WORD y,UBYTE *text);
void GetPoint(W3D_Context *context,ULONG i);
void GetVertex(struct WAZP3D_context *WC,W3D_Vertex *V);
void PrintAllFunctionsAdresses(void);
void PrintContext(W3D_Context *C);
void PrintDriver(W3D_Driver *D);
void PrintTexture(W3D_Texture *T);
void PrintWT(struct WAZP3D_texture *WT);
void SetDrawRegion(W3D_Context *context, struct BitMap *bm,int yoffset, W3D_Scissor *scissor);
BOOL SetState(W3D_Context *context,ULONG state,BOOL set);
void SetTexStates(W3D_Context* context,W3D_Texture *texture,ULONG primitive);
void ZbufferCheck(W3D_Context *context);
void DrawPrimitive(W3D_Context* context);
void DumpObject(struct WAZP3D_context *WC);
/*==========================================================================*/
/* semi-public Wazp3D function */
void  WAZP3D_Settings();
/*==========================================================================*/
void spf(UBYTE *name,float x)        /* emulate sprintf() from a float %f */
{
    LONG high,low,n,size;

    high=(LONG)(x);
    x=(x-(float)high);
    if(x<0.0) x=-x;
    low =(LONG)(1000000.0*x);
    Libsprintf((char*)name,(char*)"%ld.%6ld ",(ULONG)high,(ULONG)low);
    size=Libstrlen(name)-1;
    NLOOP(size)
        if(name[n]==' ') name[n]='0';

    name[size+1]=' ';
    name[size+2]=0;
}
/*==========================================================================*/
void pf(float x)                /* emulate printf() from a float %f */
{
    UBYTE name[40];

    spf(name,x);
    Libprintf((void*)name);
}
/*==========================================================================*/
void fpf(BPTR file,float x)        /* emulate fprintf() from a float %f */
{
    UBYTE name[40];

    spf(name,x);
    Write(file,name,Libstrlen(name));
}
/*==========================================================================*/
void ph(ULONG x)                /* emulate printf() as hexa */
{
#ifdef WAZP3DDEBUG
    LONG high,low;
    WORD n;
    UBYTE name[9];
    UBYTE hexa[16] = {"0123456789ABCDEF"};

    name[8]=0;
    NLOOP(8)
    {
        high=x>>4;
        low =x-(high<<4);
        x=high;
        name[7-n]=hexa[low];
    }
    NLOOP(7)
    {
        if(name[n]!='0')
        {Libprintf("%s",&name[n]);return;}
    }
#endif
}
/*==================================================================================*/
#include "mymemory.h"
/*==================================================================================*/
static void process_window_events(struct Window *mywin,struct button3D *B)
{
    struct IntuiMessage *imsg;
    struct Gadget *gad;
    BOOL  terminated = FALSE;
    struct button3D *ThisB;
    ULONG value;

    while (!terminated)
    {
        Wait (1 << mywin->UserPort->mp_SigBit);

        /* Use GT_GetIMsg() and GT_ReplyIMsg() for handling */
        /* IntuiMessages with GadTools gadgets.             */
        while ((!terminated) && (imsg = GT_GetIMsg(mywin->UserPort)))
        {
            /* GT_ReplyIMsg() at end of loop */

            switch (imsg->Class)
            {
            case IDCMP_GADGETUP:       /* Buttons only report GADGETUP */
                gad = (struct Gadget *)imsg->IAddress;
                ThisB=&B[gad->GadgetID];

                if(ThisB->CycleNb!=0)
                {
                GT_GetGadgetAttrs(gad, mywin, NULL,GTCY_Active,(ULONG)&value,TAG_DONE);
                ThisB->ON=value;
                }
                else
                {
                ThisB->ON=!ThisB->ON;
                }
                break;

            case IDCMP_CLOSEWINDOW:
                terminated = TRUE;
                break;
            case IDCMP_REFRESHWINDOW:
                /* This handling is REQUIRED with GadTools. */
                GT_BeginRefresh(mywin);
                GT_EndRefresh(mywin, TRUE);
                break;
            }
            /* Use the toolkit message-replying function here... */
            GT_ReplyIMsg(imsg);
        }
    }
}
/*=================================================================*/
void WAZP3D_Settings()
{
    struct button3D *B=&Wazp3D->HardwareLie;
    struct TextAttr Topaz80 = { (STRPTR)"topaz.font", 8, 0, 0, };
    struct Screen    *mysc;
    struct Window    *mywin;
    struct Gadget    *glist=NULL;
    struct Gadget    *gad;
    struct NewGadget ng;
    void   *vi;
    WORD Bnb,n,x,y;
    ULONG value;

    Wazp3D->PrefsIsOpened=TRUE;
#ifdef WAZP3DDEBUG
    Bnb=1+((LONG)&Wazp3D->DebugMemUsage -(LONG)&Wazp3D->HardwareLie)/sizeof(struct button3D);
#else
    Bnb=1+((LONG)&Wazp3D->DebugWazp3D  - (LONG)&Wazp3D->HardwareLie)/sizeof(struct button3D);
#endif
    Wazp3D->DebugWazp3D.ON=LibDebug;

    if ( (mysc = LockPubScreen(NULL)) != NULL )
    {
        if ( (vi = GetVisualInfo(mysc, TAG_END)) != NULL )
        {
            /* GadTools gadgets require this step to be taken */
            gad = CreateContext(&glist);

            NLOOP(Bnb)
            {
                x=220 + mysc->WBorLeft;
                y=12*n + 6 + mysc->WBorTop + (mysc->Font->ta_YSize + 1);
                if((Bnb/2) < n)
                {
                    x=x+230; y=y-Bnb/2*12;
                }
                /* create a button gadget centered below the window title */
                ng.ng_TextAttr    = &Topaz80;
                ng.ng_VisualInfo    = vi;
                ng.ng_LeftEdge    = x;
                ng.ng_TopEdge    = y;
                ng.ng_Width        = 24;
                ng.ng_Height    = 12;
                ng.ng_GadgetText    = (UBYTE*)B[n].name;
                ng.ng_GadgetID    = n;
                ng.ng_Flags        = 0;

                if(B[n].CycleNb!=0)
                {
                    ng.ng_Width        = 24+120;
                    ng.ng_LeftEdge    =  x-120;
                    value            = B[n].ON;
                    gad = CreateGadget(CYCLE_KIND      , gad, &ng, GTCY_Labels,(ULONG)B[n].cyclenames,GTCY_Active,value, TAG_DONE);
                }
                else
                {
                    gad = CreateGadget(CHECKBOX_KIND, gad, &ng, GT_Underscore, '_',  GTCB_Scaled, TRUE,GTCB_Checked,B[n].ON,TAG_DONE);
                }
            }

            if (gad != NULL)
            {
                if ( (mywin = OpenWindowTags(NULL,
                            WA_Title,  Wazp3DPrefsNameTag ,
                            WA_Gadgets, (ULONG)  glist,      WA_AutoAdjust,    TRUE,
                            WA_InnerWidth,       260+220,      WA_InnerHeight,    20+12*Bnb/2,
                            WA_DragBar,    TRUE,      WA_DepthGadget,   TRUE,
                            WA_Activate,   TRUE,      WA_CloseGadget,   TRUE,
                            WA_IDCMP, IDCMP_CLOSEWINDOW |IDCMP_REFRESHWINDOW | BUTTONIDCMP,
                            WA_PubScreen,  (ULONG) mysc,
                            TAG_END)) != NULL )
                {
                    GT_RefreshWindow(mywin, NULL);

                    process_window_events(mywin,B);

                    CloseWindow(mywin);
                }
            }
            /* FreeGadgets() must be called after the context has been
            ** created.  It does nothing if glist is NULL
            */
            FreeGadgets(glist);
            FreeVisualInfo(vi);
        }

        UnlockPubScreen(NULL, mysc);
    }
    LibDebug=Wazp3D->DebugWazp3D.ON;
    Wazp3D->PrefsIsOpened=FALSE;
}
/*=================================================================*/
void WAZP3D_Function(UBYTE n)
{

    if(Wazp3D->FunctionCallsAll!= 0xFFFFFFFF)
        Wazp3D->FunctionCallsAll++;
    if(Wazp3D->FunctionCalls[n]!= 0xFFFFFFFF)
        Wazp3D->FunctionCalls[n]++;

#ifdef WAZP3DDEBUG
    if(!Wazp3D->DebugWazp3D.ON) return;
    if(Wazp3D->DebugCalls.ON)
        Libprintf("[%ld][%ld]",Wazp3D->FunctionCallsAll,Wazp3D->FunctionCalls[n]);
    if(Wazp3D->DebugFunction.ON)
    {
        Libprintf("%s\n" ,Wazp3D->FunctionName[n]);
        if(Wazp3D->DebugAsJSR.ON)
        {
            Libprintf("                    JSR            -$");
            ph(30+6*n);
            Libprintf("(A6)\n");
        }
    }

    if(Wazp3D->StepFunction50.ON)
    if(Wazp3D->FunctionCallsAll % 50 == 0)
        LibAlert("Step 50 functions");

    if(Wazp3D->FunctionCallsAll==Wazp3D->CrashFunctionCall)
        Wazp3D->StepFunction.ON=Wazp3D->StepSOFT3D.ON=TRUE;

    if(Wazp3D->StepFunction.ON)
        LibAlert(Wazp3D->FunctionName[n]);

#endif
}
/*==========================================================================*/
void ColorToRGBA(UBYTE *RGBA,float r,float g,float b,float a)
{
    RGBA[0]=(UBYTE)(r*256.0);
    if(1.0<=r)    RGBA[0]=255;
    if(r<=0.0)    RGBA[0]=0;
    RGBA[1]=(UBYTE)(g*256.0);
    if(1.0<=g)    RGBA[1]=255;
    if(g<=0.0)    RGBA[1]=0;
    RGBA[2]=(UBYTE)(b*256.0);
    if(1.0<=b)    RGBA[2]=255;
    if(b<=0.0)    RGBA[2]=0;
    RGBA[3]=(UBYTE)(a*256.0);
    if(1.0<=a)    RGBA[3]=255;
    if(a<=0.0)    RGBA[3]=0;
}
/*=================================================================*/
void PrintRGBA(uint8_t *RGBA)
{
#ifdef WAZP3DDEBUG
    if (!Wazp3D->DebugVal.ON) return;
    Libprintf(" RGBA %ld %ld %ld %ld\n",(ULONG)RGBA[0],(ULONG)RGBA[1],(ULONG)RGBA[2],(ULONG)RGBA[3]);
#else
    return;
#endif
}
/*=================================================================*/
void PrintP(struct point3D *P)
{
#ifdef WAZP3DDEBUG
    if (!Wazp3D->DebugPoint.ON) return;
    Libprintf(" P XYZ W UV ");
    pf(P->x);
    pf(P->y);
    pf(P->z);
    pf(P->w);
    pf(P->u);
    pf(P->v);
    if (Wazp3D->DebugVal.ON)
        PrintRGBA((UBYTE *)&P->RGBA);
    else
        Libprintf("\n");
#else
    return;
#endif
}
/*=================================================================*/
void PrintWT(struct WAZP3D_texture *WT)
{
#ifdef WAZP3DDEBUG
    if (!Wazp3D->DebugWT.ON) return;
    Libprintf("WAZP3D_texture(%ld) %ldX%ldX%ld  pt%ld ST<%ld> NextWT<%ld> \n",(ULONG)WT,(ULONG)WT->large,(ULONG)WT->high,(ULONG)WT->bits,(ULONG)WT->pt,(ULONG)WT->ST,(ULONG)WT->nextWT);
#else
    return;
#endif
}
/*==========================================================================*/
void PrintAllT(W3D_Context *context)
{
#ifdef WAZP3DDEBUG
    struct WAZP3D_context *WC=context->driver;
    struct WAZP3D_texture *WT;
    WORD Ntexture;

    if (!Wazp3D->DebugWT.ON) return;
    SREM(PrintAllT)
    if(WC->firstWT==NULL)    return;
    Ntexture=0;
    WT=WC->firstWT;

    while(WT!=NULL)
    {
        VAL(Ntexture)
        PrintWT(WT);
        WT=WT->nextWT;    /* my own linkage */
        Ntexture++;
    }
#else
    return;
#endif
}
/*==========================================================================*/
void SOFT3D_SetBitmap_SetClippingStack(struct WAZP3D_context *WC)
{
/* calling SOFT3D in an hook avoid GCC's inlining that cause error "fixed or forbidden register was spilled" */

    SOFT3D_SetBitmap(WC->SC,WC->rastport.BitMap,WC->bmdata,WC->bmformat,0,WC->yoffset,WC->large,WC->high);
    SOFT3D_SetClipping(WC->SC,WC->Xmin,WC->Xmax,WC->Ymin,WC->Ymax);
}
/*=============================================================*/
void SetTexStates(W3D_Context* context,W3D_Texture *texture,ULONG primitive)
{
    struct WAZP3D_context *WC=context->driver;
    struct WAZP3D_texture *WT=NULL;

REM(SetTexStates)
    if(!Wazp3D->UseStateTracker.ON)
        WC->state.Changed=TRUE;        /* then force state changing */

    if(WC->state.primitive!=primitive)
        WC->state.Changed=TRUE;        /* then force state changing */

    if(WC->state.Changed)            /* then flush buffered primitives if any */
    if(WC->Pnb)
        DrawPrimitive(context);


#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugVar.ON)
    {
        Libprintf(" WC->state:Changed%ld UseTex%ld ST%ld ZMode%ld TexEnvMode%ld BlendMode%ld UseGouraud%ld UseFog%ld (tex %ld)\n",
            WC->state.Changed,WC->state.UseTex,WC->state.ST,WC->state.ZMode,WC->state.TexEnvMode,WC->state.BlendMode,WC->state.UseGouraud,WC->state.UseFog,WC->texture);
    }
#endif

    if(!StateON(W3D_TEXMAPPING))        /* same  as BindTexture(NULL); */
        texture=NULL;

    if(WC->texture!=texture)
    {
    if(WC->Pnb)
        DrawPrimitive(context);

    if(Wazp3D->DebugVar.ON) DEBUGPRINTF1(" Changing to new texture %ld\n",texture);

/* define texture */
    if(texture==NULL)
    {
        WC->state.UseTex=FALSE;
        WT=NULL;
        WC->WT=WT;
        WC->state.ST=(ULONG)NULL;
        WC->uresize=1.0;
        WC->vresize=1.0;
        WC->state.TexEnvMode=0;                /* default = no tex ==> no TexEnv effect */
    }
    else
    {
        WC->state.UseTex= (StateON(W3D_TEXMAPPING));
        WT=texture->driver;
        WC->WT=WT;
        WC->state.ST=(ULONG)WT->ST;
        WC->uresize=1.0/(float)(WT->large);
        WC->vresize=1.0/(float)(WT->high );
        WC->state.TexEnvMode=WT->TexEnv;                    /* texture's texture-env-mode  */
        COPYRGBA(WC->state.EnvRGBA.L,WT->EnvRGBA.L);            /* texture's texture-env-color */
        if(StateON(W3D_GLOBALTEXENV))
        {
            WC->state.TexEnvMode=context->globaltexenvmode;     /* global texture-env-mode  */
            COPYRGBA(WC->state.EnvRGBA.L,WC->EnvRGBA.L);            /* global texture-env-color */
        }
        PrintWT(WT);
    }


    WC->texture=texture;
    WC->state.Changed=TRUE;                /* because texture has changed */
    }
    WC->state.primitive=primitive;

/* check state */
    if(Wazp3D->PrefsIsOpened)
        WC->state.Changed=TRUE;            /* because user may change something */

    if(WC->state.Changed==FALSE)
        {REM(  WC->state not changed); return;}
    else
        {REM(  Adjusting WC->state ....);}

    if(texture!=NULL)
        WT=texture->driver;            /* needed for WT->BlendMode,WT->ChromaTestMode etc... */

/* define Zmode */
    if(Wazp3D->DebugTexNumber.ON)            /* disable multi-pass texturing = more readable*/
    {
    if(WC->ZCompareMode==W3D_Z_GEQUAL)   WC->ZCompareMode=W3D_Z_GREATER;
    if(WC->ZCompareMode==W3D_Z_LEQUAL)   WC->ZCompareMode=W3D_Z_LESS;
    if(WC->ZCompareMode==W3D_Z_EQUAL)    WC->ZCompareMode=W3D_Z_NEVER;
    }

    WC->state.ZMode=ZMODE(0,W3D_Z_ALWAYS);           /* default no z buffer = no test ==> always draw & dont update a zbuffer */
    if(context->zbufferalloc)
    if(StateON(W3D_ZBUFFER))
        WC->state.ZMode   =ZMODE(StateON(W3D_ZBUFFERUPDATE),WC->ZCompareMode);

/* define UseGouraud UseTex UseFog */
    WC->state.UseTex        = (StateON(W3D_TEXMAPPING)) et (texture!=NULL);
    WC->state.UseGouraud    = (StateON(W3D_GOURAUD));
    WC->state.UseFog        = (StateON(W3D_FOGGING)) et (WC->state.FogMode!=0) et (Wazp3D->UseFog.ON==TRUE);
    if(!WC->state.UseTex)
        WC->state.TexEnvMode=0;                      /* no tex ==> no TexEnv effect */

/* define PerspMode */
        WC->state.PerspMode=Wazp3D->PerspMode.ON;    /* default: do perspective for edges only */
    if(!StateON(W3D_PERSPECTIVE))                    /* if perspective not enabled in Warp3D */
        WC->state.PerspMode=0;
VAR(WC->state.BlendMode)
/* define BlendMode */
    WC->state.BlendMode=BLENDREPLACE;                /* default : simply write the color (even if color=tex+color) */
    if(Wazp3D->TexMode.ON!=4)                        /* If not set to "No blending" */
    if(StateON(W3D_BLENDING))
        WC->state.BlendMode=(WC->SrcFunc*16 + WC->DstFunc);

VAR(WC->DstFunc)
VAR(WC->SrcFunc)
VAR(WC->state.BlendMode)


/* specials BlendMode */
    if(WC->state.UseTex)
    {
        if(Wazp3D->QuakeMatrixPatch.ON)
        {
            if(WC->state.BlendMode ==BLENDREPLACE)           /* if write flat */
            {
                if(WT->ChromaTestMode > W3D_CHROMATEST_NONE)     /* if chroma (but wipeout do the bad test W3D_CHROMATEST_EXCLUSIVE) */
                {
                /* patch: v51 for WipeOut2097 that use the chroma test for the shadow and on-screen display */
                    SREM(Wipeout chroma )
                    WC->state.BlendMode=BLENDCHROMA;             /* v51: fast simple chromatest : color not black */
                }
                else                                    /* if no chroma */
                {
                /* patch: for Quake use current tex BlendMode (BLENDNOALPHA or BLENDALPHA or BLENDFASTALPHA) */
                    if(WC->state.TexEnvMode==W3D_REPLACE)        /* if tex and no coloring function */
                    {
                    SREM(Quake use own BlendMode )
                    WC->state.BlendMode=WT->BlendMode;
                    }
                }
            }
        }
    }

/* patch: v51 for FPSE that use W3D_SetAlphaMode() with the test Alpha > 0.5 */
    if(StateON(W3D_ALPHATEST))
    if(WC->AlphaMode==W3D_A_GREATER)
    if(WC->AlphaRef<=128)
    if(WC->state.BlendMode==BLENDREPLACE)
    {
        SREM(FPSE use simple alpha )
        WC->state.BlendMode=BLENDFASTALPHA;
    }

/* change some states if user want an hacked TexMode */
    if(WC->state.UseTex)
    {
        if(Wazp3D->TexMode.ON==0)                /* If  texture but no coloring then gouraud is useless */
            WC->state.UseGouraud=FALSE;
        if(Wazp3D->TexMode.ON!=1)                /* If  dont use coloring for lighting*/
            WC->state.TexEnvMode=W3D_REPLACE;    /* then default to W3D_REPLACE  : simply R-eplace the color with tex = copy tex */
        if(Wazp3D->TexMode.ON==2)
        {
            WC->state.TexEnvMode=0;
            WC->state.BlendMode=BLENDREPLACE;
            WC->state.UseFog=FALSE;
            WC->state.UseGouraud=TRUE;
        }
        if(Wazp3D->TexMode.ON==3)
        {
            WC->state.TexEnvMode=0;
            WC->state.BlendMode=BLENDREPLACE;
            WC->state.UseFog=FALSE;
            WC->state.UseGouraud=FALSE;
        }
    }

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugVar.ON)
        Libprintf(" WC->state:Changed%ld UseTex%ld ST%ld ZMode%ld TexEnvMode%ld BlendMode%ld UseGouraud%ld UseFog%ld\n",WC->state.Changed,WC->state.UseTex,WC->state.ST,WC->state.ZMode,WC->state.TexEnvMode,WC->state.BlendMode,WC->state.UseGouraud,WC->state.UseFog);
#endif

    SOFT3D_SetDrawState(WC->SC,&WC->state);
    SOFT3D_Debug(&WC->state);
    SOFT3D_Debug(&WC->state.ST);
    SOFT3D_Debug((APTR)WC->state.ST);
    struct state3D *state2=&WC->state;
    SOFT3D_Debug((APTR)state2->ST);
    WC->state.Changed=FALSE;            /* change done */
}
#ifdef WAZP3DDEBUG
/*=================================================================*/
ULONG PrintError(LONG error)
{
    if (!Wazp3D->DebugError.ON) return(error);
    ERROR(W3D_SUCCESS,"success ")
    ERROR(W3D_BUSY,"graphics hardware is busy ")
    ERROR(W3D_ILLEGALINPUT,"failure,illegal input ")
    ERROR(W3D_NOMEMORY,"no memory available ")
    ERROR(W3D_NODRIVER,"no hardware driver available ")
    ERROR(W3D_NOTEXTURE,"texture is missing ")
    ERROR(W3D_TEXNOTRESIDENT,"texture not resident ")
    ERROR(W3D_NOMIPMAPS,"mipmaps are not supported by this texture object")
    ERROR(W3D_NOGFXMEM,"no graphics memory available ")
    ERROR(W3D_NOTVISIBLE,"drawing area not or bitmap swapped out")
    ERROR(W3D_UNSUPPORTEDFILTER,"unsupported filter ")
    ERROR(W3D_UNSUPPORTEDTEXENV,"unsupported filter ")
    ERROR(W3D_UNSUPPORTEDWRAPMODE,"unsupported wrap mode ")
    ERROR(W3D_UNSUPPORTEDZCMP,"unsupported Z compare mode ")
    ERROR(W3D_UNSUPPORTEDATEST,"unsupported alpha test ")
    ERROR(W3D_UNSUPPORTEDBLEND,"unsupported blending function ")
    ERROR(W3D_UNSUPPORTEDFOG,"unsupported fog mode ")
    ERROR(W3D_UNSUPPORTEDSTATE,"can't enable/disable state ")
    ERROR(W3D_UNSUPPORTEDFMT,"unsupported bitmap format ")
    ERROR(W3D_UNSUPPORTEDTEXSIZE,"unsupported texture border size ")
    ERROR(W3D_UNSUPPORTEDLOGICOP,"unsupported logical operation ")
    ERROR(W3D_UNSUPPORTEDSTTEST,"unsupported stencil test ")
    ERROR(W3D_ILLEGALBITMAP,"illegal bitmap ")
    ERROR(W3D_NOZBUFFER,"Zbuffer is missing or unavailable ")
    ERROR(W3D_NOPALETTE,"Palette missing for chunky textures")
    ERROR(W3D_MASKNOTSUPPORTED,"color/index masking not supported ")
    ERROR(W3D_NOSTENCILBUFFER,"Stencil buffer is missing or unavailable ")
    ERROR(W3D_QUEUEFAILED,"The request can not be queued ")
    ERROR(W3D_UNSUPPORTEDTEXFMT,"Texformat unsupported ")
    ERROR(W3D_WARNING,"Partial success ")
    ERROR(W3D_UNSUPPORTED,"Requested feature is unsufxpported ")
    ERROR(W3D_H_TEXMAPPING,"Quality of general texture mapping")
    ERROR(W3D_H_MIPMAPPING,"Quality of mipmapping ")
    ERROR(W3D_H_BILINEARFILTER,"Quality of bilinear filtering ")
    ERROR(W3D_H_MMFILTER,"Quality of depth filter ")
    ERROR(W3D_H_PERSPECTIVE,"Quality of perspective correction ")
    ERROR(W3D_H_BLENDING,"Quality of alpha blending ")
    ERROR(W3D_H_FOGGING,"Quality of fogging ")
    ERROR(W3D_H_ANTIALIASING,"Quality of antialiasing ")
    ERROR(W3D_H_DITHERING,"Quality of dithering ")
    ERROR(W3D_H_ZBUFFER,"Quality of zbuffering ")
    ERROR(W3D_H_POINTDRAW," ")
    return(error);
}
/*==========================================================================*/
void PrintDriver(W3D_Driver *D)
{
//char *drivertype="";

    if(!Wazp3D->DebugDriver.ON) return;
//    if(D->swdriver==W3D_TRUE)    drivertype="Software";
//    if(D->swdriver==W3D_FALSE)   drivertype="Hardware";

    WINFO(D->ChipID,W3D_CHIP_UNKNOWN," ");
    WINFO(D->ChipID,W3D_CHIP_VIRGE," ");
    WINFO(D->ChipID,W3D_CHIP_PERMEDIA2," ");
    WINFO(D->ChipID,W3D_CHIP_VOODOO1," ");

/* Chip constants (5,6,7,8,9)  that changed (!!!) from Warp3D V4 to V5 */
#ifdef WARP3DV5
    WINFO(D->ChipID,W3D_CHIP_OBSOLETE1," ");
    WINFO(D->ChipID,W3D_CHIP_OBSOLETE2," ");
    WINFO(D->ChipID,W3D_CHIP_RADEON," ");
    WINFO(D->ChipID,W3D_CHIP_AVENGER,"Voodoo 3")
    WINFO(D->ChipID,W3D_CHIP_NAPALM," Voodoo 4/5 ")
    WINFO(D->ChipID,W3D_CHIP_RADEON_R200,"R200, R280, RV280 etc ")
    WINFO(D->ChipID,W3D_CHIP_RADEON_R300,"R300, R350, R380, RV3xx etc ")
#else
    WINFO(D->ChipID,W3D_CHIP_AVENGER_LE,"Voodoo 3 LE= Little endian mode")
    WINFO(D->ChipID,W3D_CHIP_AVENGER_BE,"Voodoo 3 BE= Big endian mode ")
    WINFO(D->ChipID,W3D_CHIP_PERMEDIA3," ");
    WINFO(D->ChipID,W3D_CHIP_RADEON," ");
    WINFO(D->ChipID,W3D_CHIP_RADEON2," ");
#endif

    SREM(Support formats:)
    WINFOB(D->formats,W3D_FMT_CLUT,"chunky ")
    WINFOB(D->formats,W3D_FMT_R5G5B5,"0 rrrrr ggggg bbbbb ")
    WINFOB(D->formats,W3D_FMT_B5G5R5,"0 bbbbb ggggg rrrrr ")
    WINFOB(D->formats,W3D_FMT_R5G5B5PC,"ggg bbbbb 0 rrrrr gg ")
    WINFOB(D->formats,W3D_FMT_B5G5R5PC,"ggg rrrrr 0 bbbbb gg ")
    WINFOB(D->formats,W3D_FMT_R5G6B5,"rrrrr gggggg bbbbb ")
    WINFOB(D->formats,W3D_FMT_B5G6R5,"bbbbb gggggg rrrrr ")
    WINFOB(D->formats,W3D_FMT_R5G6B5PC,"ggg bbbbb rrrrr ggg ")
    WINFOB(D->formats,W3D_FMT_B5G6R5PC,"ggg rrrrr bbbbb ggg ")
    WINFOB(D->formats,W3D_FMT_R8G8B8,"rrrrrrrr gggggggg bbbbbbbb ")
    WINFOB(D->formats,W3D_FMT_B8G8R8,"bbbbbbbb gggggggg rrrrrrrr ")
    WINFOB(D->formats,W3D_FMT_A8R8G8B8,"aaaaaaaa rrrrrrrr gggggggg bbbbbbbb ")
    WINFOB(D->formats,W3D_FMT_A8B8G8R8,"aaaaaaaa bbbbbbbb gggggggg rrrrrrrr ")
    WINFOB(D->formats,W3D_FMT_R8G8B8A8,"rrrrrrrr gggggggg bbbbbbbb aaaaaaaa ")
    WINFOB(D->formats,W3D_FMT_B8G8R8A8,"bbbbbbbb gggggggg rrrrrrrr aaaaaaaa ")
}
/*==========================================================================*/
void PrintContext(W3D_Context *C)
{
WORD n;

    if(!Wazp3D->DebugContext.ON) return;
    VAR(C->driver)
    VAR(C->gfxdriver)
    VAR(C->drivertype)
    VAR(C->regbase)
    VAR(C->vmembase)
    VAR(C->zbuffer)
    VAR(C->stencilbuffer)
    VAR(C->state)
    VAR(C->drawregion)
    VAR(C->supportedfmt)
    VAR(C->format)
    VAR(C->yoffset)
    VAR(C->bprow)
    VAR(C->width)
    VAR(C->height)
    VAR(C->depth)
    VAR(C->chunky)
    VAR(C->destalpha)
    VAR(C->zbufferalloc)
    VAR(C->stbufferalloc)
    VAR(C->HWlocked)
    VAR(C->w3dbitmap)
    VAR(C->zbufferlost)
    VAR(C->reserved3)
    VAR(&C->restex)
    VAR(&C->tex)
    VAR(C->maxtexwidth)
    VAR(C->maxtexheight)
    VAR(C->maxtexwidthp)
    VAR(C->maxtexheightp)
    VAR(C->scissor.left)
    VAR(C->scissor.top)
    VAR(C->scissor.width)
    VAR(C->scissor.height)

    VARF(C->fog.fog_start)
    VARF(C->fog.fog_end)
    VARF(C->fog.fog_density)
    VARF(C->fog.fog_color.r)
    VARF(C->fog.fog_color.g)
    VARF(C->fog.fog_color.b)
    VAR(C->envsupmask)
    VAR(C->queue)
    VAR(C->drawmem)
    VAR(C->globaltexenvmode)
    VARF(C->globaltexenvcolor[0])
    VARF(C->globaltexenvcolor[1])
    VARF(C->globaltexenvcolor[2])
    VARF(C->globaltexenvcolor[3])
    VAR(C->DriverBase)
    VAR(C->EnableMask)
    VAR(C->DisableMask)
    VAR(C->CurrentChip)
    VAR(C->DriverVersion)

    VAR(C->VertexPointer)
    VAR(C->VPStride)
    VAR(C->VPMode)
    VAR(C->VPFlags)
    NLOOP(W3D_MAX_TMU)
    {
    VAR(C->TexCoordPointer[n])
    VAR(C->TPStride[n])
    VAR(C->CurrentTex[n])
    VAR(C->TPVOffs[n])
    VAR(C->TPWOffs[n])
    VAR(C->TPFlags[n])
    }
    VAR(C->ColorPointer)
    VAR(C->CPStride)
    VAR(C->CPMode)
    VAR(C->CPFlags)
    VAR(C->FrontFaceOrder)
    VAR(C->specialbuffer)
}
/*==========================================================================*/
void PrintTexture(W3D_Texture *T)
{
    WORD n;

    if(!Wazp3D->DebugTexture.ON) return;
    VAR(&T->link)
    VAR(T->resident)
    VAR(T->mipmap)
    VAR(T->dirty)
    VAR(T->matchfmt)
    VAR(T->reserved1)
    VAR(T->reserved2)
    VAR(T->mipmapmask)
    VAR(T->texsource)
    NLOOP(16)
        VAR(T->mipmaps[n])
    VAR(T->texfmtsrc)
    VAR(T->palette)
    VAR(T->texdata)
    VAR(T->texdest)
    VAR(T->texdestsize)
    VAR(T->texwidth)
    VAR(T->texwidthexp)
    VAR(T->texheight)
    VAR(T->texheightexp)
    VAR(T->bytesperpix)
    VAR(T->bytesperrow)
    VAR(T->driver)
}
/*=================================================================*/
#else
#define PrintError(error) ;
#define PrintDriver(x)  ;
#define PrintContext(x) ;
#define PrintTexture(x) ;
#endif
/*==================================================================================*/
ULONG SupportedFormats(void)
{
ULONG formats=0;

    if(Wazp3D->Renderer.ON==0)       /* use Soft to Image */
        formats=HIGHCOLORFORMATS;
    if(Wazp3D->Renderer.ON==1)       /* use Soft to bitmap */
        formats=ALLCOLORFORMATS;     /* v46: now support 8/15/16/24/32 bits */
#ifdef SOFT3DLIB                     /* if is winuae PC */
    if(Wazp3D->Renderer.ON==2)       /* use Hard */
        formats=W3D_FMT_B8G8R8A8;
    if(Wazp3D->Renderer.ON==3)       /* use Hard(Overlay)*/
        formats=ALLCOLORFORMATS;
#else                            /* is aros/morphos */
    if(Wazp3D->Renderer.ON==2)       /* use Hard */
        formats=ALLCOLORFORMATS;
    if(Wazp3D->Renderer.ON==3)       /* use Hard(Overlay)*/
        formats=ALLCOLORFORMATS;
#endif
    if(Wazp3D->OnlyTrueColor.ON)
        formats=TRUECOLORFORMATS;
    return(formats);
}
/*==================================================================================*/
void WritePrefs()
{
//#define SAVECFGFILE
#ifdef SAVECFGFILE
    struct button3D *B=(struct button3D *)&Wazp3D->HardwareLie;
    ULONG Bnb=1+((LONG)&Wazp3D->DebugMemUsage -(LONG)&Wazp3D->HardwareLie)/sizeof(struct button3D);
    char prefs[100];
    ULONG n;
    REM(Saving Wazp3D.cfg)
    strcpy(prefs,"WAZP3D");
    prefs[7]=CFGVERSION;
    prefs[8]=CFGSUBVERSION;
    prefs[9]='F';    /*Full Amiga 68k version */
#ifdef SOFT3DLIB
    prefs[9]='W';    /*WinUAE version using soft3d.library */
#endif
#ifdef __amigaos4__
    prefs[9]='P';    /*PPC version */
#endif
#ifdef __AROS__
    prefs[9]='A';    /*Aros version */
#endif
#ifdef __MORPHOS__
    prefs[9]='M';    /*Morphos version */
#endif
    NLOOP(Bnb)
    {
        prefs[10+n]=B[n].ON+'0';
        DEBUGPRINTF2("%s [%ld]\n",B[n].name,B[n].ON);
    }

    Libsavefile("Wazp3D.cfg",prefs,10+Bnb);
#endif
}
/*==================================================================================*/
void ReadPrefs()
{
    struct button3D *B=(struct button3D *)&Wazp3D->HardwareLie;
    ULONG Bnb=1+((LONG)&Wazp3D->DebugMemUsage -(LONG)&Wazp3D->HardwareLie)/sizeof(struct button3D);
    UBYTE prefs[100];
    ULONG n;

    prefs[0]=0;
    Libprintf("Loading Wazp3D.cfg\n");
    Libloadfile("ENVARC:Wazp3D.cfg",prefs,10+Bnb);
    Libloadfile("Wazp3D.cfg",prefs,10+Bnb);

    if(prefs[0]=='W')
    if(prefs[1]=='A')
    if(prefs[2]=='Z')
    if(prefs[3]=='P')
    if(prefs[4]=='3')
    if(prefs[5]=='D')
    if(prefs[6]==0)
    if(prefs[7]==CFGVERSION)
    if(prefs[8]==CFGSUBVERSION)
    {
        Libprintf("Wazp3D.cfg loaded\n");
        NLOOP(Bnb)
        {
            B[n].ON=prefs[10+n]-'0';
            DEBUGPRINTF2("%s [%ld]\n",B[n].name,B[n].ON);
        }
    }
}
/*==================================================================================*/
void WAZP3D_Open()
{
    ReadPrefs();
}
volatile UBYTE* z3660_regs;
void open_soft3d56(void);
/*==================================================================================*/
BOOL WAZP3D_Init(void *exec)
{
struct button3D *B=(struct button3D *)&Wazp3D->HardwareLie;
WORD Bnb,n,x,large;
    //if(OpenAmigaLibraries(exec)==FALSE) return(FALSE);
    OpenAmigaLibraries(exec);
    open_soft3d56();

    SREM(WAZP3D_Init)
    firstME=NULL;    /* Tracked memory-allocation    */

    NLOOP(MAXFUNCTIONS)
    {
        Libstrcpy(Wazp3D->FunctionName[1+n],"W3D_");
        Wazp3D->FunctionCalls[1+n]=0;
    }
    Wazp3D->FunctionCallsAll=0;
    Wazp3D->CrashFunctionCall=0;        /*to start the step-by-step with this call */

    Libstrcat(Wazp3D->FunctionName[ 1],"CreateContext");
    Libstrcat(Wazp3D->FunctionName[ 2],"CreateContextTags");
    Libstrcat(Wazp3D->FunctionName[ 3],"DestroyContext");
    Libstrcat(Wazp3D->FunctionName[ 4],"GetState");
    Libstrcat(Wazp3D->FunctionName[ 5],"SetState");
    Libstrcat(Wazp3D->FunctionName[ 6],"Hint");
    Libstrcat(Wazp3D->FunctionName[ 7],"CheckDriver");
    Libstrcat(Wazp3D->FunctionName[ 8],"LockHardware");
    Libstrcat(Wazp3D->FunctionName[ 9],"UnLockHardware");
    Libstrcat(Wazp3D->FunctionName[10],"WaitIdle");
    Libstrcat(Wazp3D->FunctionName[11],"CheckIdle");
    Libstrcat(Wazp3D->FunctionName[12],"Query");
    Libstrcat(Wazp3D->FunctionName[13],"GetTexFmtInfo");
    Libstrcat(Wazp3D->FunctionName[14],"GetDriverState");
    Libstrcat(Wazp3D->FunctionName[15],"GetDestFmt");
    Libstrcat(Wazp3D->FunctionName[16],"GetDrivers");
    Libstrcat(Wazp3D->FunctionName[17],"QueryDriver");
    Libstrcat(Wazp3D->FunctionName[18],"GetDriverTexFmtInfo");
    Libstrcat(Wazp3D->FunctionName[19],"RequestMode");
    Libstrcat(Wazp3D->FunctionName[20],"RequestModeTags");
    Libstrcat(Wazp3D->FunctionName[21],"TestMode");
    Libstrcat(Wazp3D->FunctionName[22],"AllocTexObj");
    Libstrcat(Wazp3D->FunctionName[23],"AllocTexObjTags");
    Libstrcat(Wazp3D->FunctionName[24],"FreeTexObj");
    Libstrcat(Wazp3D->FunctionName[25],"ReleaseTexture");
    Libstrcat(Wazp3D->FunctionName[26],"FlushTextures");
    Libstrcat(Wazp3D->FunctionName[27],"SetFilter");
    Libstrcat(Wazp3D->FunctionName[28],"SetTexEnv");
    Libstrcat(Wazp3D->FunctionName[29],"SetWrapMode");
    Libstrcat(Wazp3D->FunctionName[30],"UpdateTexImage");
    Libstrcat(Wazp3D->FunctionName[31],"UpdateTexSubImage");
    Libstrcat(Wazp3D->FunctionName[32],"UploadTexture");
    Libstrcat(Wazp3D->FunctionName[33],"FreeAllTexObj");
    Libstrcat(Wazp3D->FunctionName[34],"SetChromaTestBounds");
    Libstrcat(Wazp3D->FunctionName[35],"DrawLine");
    Libstrcat(Wazp3D->FunctionName[36],"DrawPoint");
    Libstrcat(Wazp3D->FunctionName[37],"DrawTriangle");
    Libstrcat(Wazp3D->FunctionName[38],"DrawTriFan");
    Libstrcat(Wazp3D->FunctionName[39],"DrawTriStrip");
    Libstrcat(Wazp3D->FunctionName[40],"Flush");
    Libstrcat(Wazp3D->FunctionName[41],"DrawLineStrip");
    Libstrcat(Wazp3D->FunctionName[42],"DrawLineLoop");
    Libstrcat(Wazp3D->FunctionName[43],"ClearDrawRegion");
    Libstrcat(Wazp3D->FunctionName[44],"SetAlphaMode");
    Libstrcat(Wazp3D->FunctionName[45],"SetBlendMode");
    Libstrcat(Wazp3D->FunctionName[46],"SetDrawRegion");
    Libstrcat(Wazp3D->FunctionName[47],"SetDrawRegionWBM");
    Libstrcat(Wazp3D->FunctionName[48],"SetFogParams");
    Libstrcat(Wazp3D->FunctionName[49],"SetLogicOp");
    Libstrcat(Wazp3D->FunctionName[50],"SetColorMask");
    Libstrcat(Wazp3D->FunctionName[51],"SetPenMask");
    Libstrcat(Wazp3D->FunctionName[52],"SetCurrentColor");
    Libstrcat(Wazp3D->FunctionName[53],"SetCurrentPen");
    Libstrcat(Wazp3D->FunctionName[54],"SetScissor");
    Libstrcat(Wazp3D->FunctionName[55],"FlushFrame");
    Libstrcat(Wazp3D->FunctionName[56],"AllocZBuffer");
    Libstrcat(Wazp3D->FunctionName[57],"FreeZBuffer");
    Libstrcat(Wazp3D->FunctionName[58],"ClearZBuffer");
    Libstrcat(Wazp3D->FunctionName[59],"ReadZPixel");
    Libstrcat(Wazp3D->FunctionName[60],"ReadZSpan");
    Libstrcat(Wazp3D->FunctionName[61],"SetZCompareMode");
    Libstrcat(Wazp3D->FunctionName[62],"WriteZPixel");
    Libstrcat(Wazp3D->FunctionName[63],"WriteZSpan");
    Libstrcat(Wazp3D->FunctionName[64],"AllocStencilBuffer");
    Libstrcat(Wazp3D->FunctionName[65],"ClearStencilBuffer");
    Libstrcat(Wazp3D->FunctionName[66],"FillStencilBuffer");
    Libstrcat(Wazp3D->FunctionName[67],"FreeStencilBuffer");
    Libstrcat(Wazp3D->FunctionName[68],"ReadStencilPixel");
    Libstrcat(Wazp3D->FunctionName[69],"ReadStencilSpan");
    Libstrcat(Wazp3D->FunctionName[70],"SetStencilFunc");
    Libstrcat(Wazp3D->FunctionName[71],"SetStencilOp");
    Libstrcat(Wazp3D->FunctionName[72],"SetWriteMask");
    Libstrcat(Wazp3D->FunctionName[73],"WriteStencilPixel");
    Libstrcat(Wazp3D->FunctionName[74],"WriteStencilSpan");
    Libstrcat(Wazp3D->FunctionName[75],"DrawTriangleV");
    Libstrcat(Wazp3D->FunctionName[76],"DrawTriFanV");
    Libstrcat(Wazp3D->FunctionName[77],"DrawTriStripV");
    Libstrcat(Wazp3D->FunctionName[78],"GetScreenmodeList");
    Libstrcat(Wazp3D->FunctionName[79],"FreeScreenmodeList");
    Libstrcat(Wazp3D->FunctionName[80],"BestModeID");
    Libstrcat(Wazp3D->FunctionName[81],"BestModeIDTags");
    Libstrcat(Wazp3D->FunctionName[82],"VertexPointer");
    Libstrcat(Wazp3D->FunctionName[83],"TexCoordPointer");
    Libstrcat(Wazp3D->FunctionName[84],"ColorPointer");
    Libstrcat(Wazp3D->FunctionName[85],"BindTexture");
    Libstrcat(Wazp3D->FunctionName[86],"DrawArray");
    Libstrcat(Wazp3D->FunctionName[87],"DrawElements"),
    Libstrcat(Wazp3D->FunctionName[88],"SetFrontFace");
#ifdef WARP3DV5
    Libstrcat(Wazp3D->FunctionName[89],"SetTextureBlend");
    Libstrcat(Wazp3D->FunctionName[90],"SetTextureBlendTags");
    Libstrcat(Wazp3D->FunctionName[91],"SecondaryColorPointer");
    Libstrcat(Wazp3D->FunctionName[92],"FogCoordPointer");
    Libstrcat(Wazp3D->FunctionName[93],"InterleavedArray");
    Libstrcat(Wazp3D->FunctionName[94],"ClearBuffers");
    Libstrcat(Wazp3D->FunctionName[95],"SetParameter");
    Libstrcat(Wazp3D->FunctionName[96],"PinTexture");
    Libstrcat(Wazp3D->FunctionName[97],"SetDrawRegionTexture");
#endif

    Libstrcpy(Wazp3D->HardwareLie.name,"+HardwareDriver Lie");
    Libstrcpy(Wazp3D->TexFmtLie.name,"+TexFmt Lie");
    Libstrcpy(Wazp3D->HackTexs.name,"+Hack Texs|No (safe)|RGBA (fast)|RGBA/RGB|RGBA/RGB/ARGB");
    Libstrcpy(Wazp3D->UseRatioAlpha.name,"+Use RatioAlpha(20%)");
    Libstrcpy(Wazp3D->UseAlphaMinMax.name,"+Use AlphaMin&Max");
    Libstrcpy(Wazp3D->OnlyTrueColor.name,"+Only TrueColor 24&32");
    Libstrcpy(Wazp3D->SmoothTextures.name,"+Smooth Textures");
    Libstrcpy(Wazp3D->DoMipMaps.name,"+Do MipMaps");
#ifdef SOFT3DLIB
    Libstrcpy(Wazp3D->Renderer.name,"+Renderer|Soft to Image|Soft to bitmap|Hard|Hard(overlay)");
#else

#ifdef __amigaos4__
    Libstrcpy(Wazp3D->Renderer.name,"+Renderer|SoftPPC to Image|SoftPPC to bitmap|Compositing2D");
#else
    Libstrcpy(Wazp3D->Renderer.name,"+Renderer|Soft68k to Image|Soft68k to bitmap");
#endif

#ifdef __AROS__
    Libstrcpy(Wazp3D->Renderer.name,"+Renderer|Soft to Image|Soft to bitmap|Hard|Hard(overlay)");
#endif

#endif
    Libstrcpy(Wazp3D->ReloadTextures.name,"+Reload new Textures");

#ifdef __MORPHOS__
    Libstrcpy(Wazp3D->Renderer.name,"+Renderer|Soft to Image|Soft to bitmap|Hard");
#endif



    Libstrcpy(Wazp3D->PolyMode.name,"PolyHack|No (normal)|Hack:up to 5|More:up to 7");
    Libstrcpy(Wazp3D->PerspMode.name,"Perspective|No (fast)  |Edges only|Simulated");
    Libstrcpy(Wazp3D->TexMode.name,"Texture|No Coloring|GL Coloring|Hacked: Gouraud|Hacked: Flat|No blending");

    Libstrcpy(Wazp3D->UseFog.name,"Use Fog");
    Libstrcpy(Wazp3D->UseColorHack.name,"Use BGcolor Hack");
    Libstrcpy(Wazp3D->UseClearDrawRegion.name,"Use ClearDrawRegion");
    Libstrcpy(Wazp3D->UseClearImage.name,"Use Clear Image");
    Libstrcpy(Wazp3D->UseMinUpdate.name,"Use Min. Update");
    Libstrcpy(Wazp3D->UseAntiImage.name,"AntiAlias Image");
    Libstrcpy(Wazp3D->UseFiltering.name,"Use Filtering");

    Libstrcpy(Wazp3D->QuakeMatrixPatch.name,"Quake,glMatrix Patch");
    Libstrcpy(Wazp3D->IndirectMode.name,"Force IndirectMode");
    Libstrcpy(Wazp3D->UseStateTracker.name,"Use StateTracker");

    Libstrcpy(Wazp3D->DebugWazp3D.name,">>> Enable Debugger >>>");
    Libstrcpy(Wazp3D->DisplayFPS.name,"Display FPS");
    Libstrcpy(Wazp3D->DebugFunction.name,"Debug Function");
    Libstrcpy(Wazp3D->StepFunction.name, "[Step]  Function");
    Libstrcpy(Wazp3D->DebugCalls.name,"Debug Calls");
    Libstrcpy(Wazp3D->DebugAdresses.name,"Debug Adresses");
    Libstrcpy(Wazp3D->DebugAsJSR.name,"Debug as JSR");
    Libstrcpy(Wazp3D->DebugVar.name,"Debug Var name ");
    Libstrcpy(Wazp3D->DebugVal.name,"Debug Var value");
    Libstrcpy(Wazp3D->DebugDoc.name,"Debug Var doc  ");
    Libstrcpy(Wazp3D->DebugState.name,"Debug State");
    Libstrcpy(Wazp3D->DebugDriver.name,"Debug Driver");
    Libstrcpy(Wazp3D->DebugContext.name,"Debug Context");
    Libstrcpy(Wazp3D->DebugTexture.name,"Debug Texture");
    Libstrcpy(Wazp3D->DebugTexNumber.name,"Debug Tex number");
    Libstrcpy(Wazp3D->DebugTexColor.name,"Debug Tex as Color");
    Libstrcpy(Wazp3D->DebugBlendFunction.name,"Debug BlendFunction");
    Libstrcpy(Wazp3D->DebugPoint.name,"Debug Point");
    Libstrcpy(Wazp3D->DebugError.name,"Debug Error");
    Libstrcpy(Wazp3D->DebugWC.name,"Debug WC");
    Libstrcpy(Wazp3D->DebugWT.name,"Debug WT");
    Libstrcpy(Wazp3D->DebugSOFT3D.name,"Debug SOFT3D");
    Libstrcpy(Wazp3D->StepUpdate.name,"[Step] Update");
    Libstrcpy(Wazp3D->StepDrawPoly.name,"[Step] DrawPoly");
    Libstrcpy(Wazp3D->StepSOFT3D.name, "[Step]  SOFT3D");
    Libstrcpy(Wazp3D->StepFunction50.name, "[Step] 50 Functions");
    Libstrcpy(Wazp3D->DebugSC.name,"Debug SC & Debug Hard");
    Libstrcpy(Wazp3D->DebugST.name,"Debug ST");
    Libstrcpy(Wazp3D->DebugSepiaImage.name,"Debug Sepia Image");
    Libstrcpy(Wazp3D->DebugClipper.name,"Debug Clipper");
    Libstrcpy(Wazp3D->DumpTextures.name,"Dump Textures");
    Libstrcpy(Wazp3D->DumpObject.name,"Dump .OBJ(scaled)");
    Libstrcpy(Wazp3D->DebugMemList.name,"Debug MemList");
    Libstrcpy(Wazp3D->DebugMemUsage.name,"Debug MemUsage");

    Bnb=1+((LONG)&Wazp3D->DebugMemUsage -(LONG)&Wazp3D->HardwareLie)/sizeof(struct button3D);
/* get names for cycle-gadgets */
    NLOOP(Bnb)
    {
        B->ON=FALSE;
        B->CycleNb=0;
        B->cyclenames[0]=B->cyclenames[1]=B->cyclenames[2]=B->cyclenames[3]=B->cyclenames[4]=NULL;
        large=Libstrlen(B->name);
        XLOOP(large)
            if(B->name[x]=='|')
                if(B->CycleNb < 5)
                {
                    B->name[x]=0;
                    B->cyclenames[B->CycleNb]=&B->name[x+1];
                    B->CycleNb++;
                }
        B++;
    }

/* default values for checkbox-gadgets */
    Wazp3D->HardwareLie.ON=TRUE;
    Wazp3D->TexFmtLie.ON=TRUE;
    Wazp3D->HackTexs.ON=3;        /* reuse RGBA/RGB/ARGB tex data*/
    Wazp3D->UseColorHack.ON=TRUE;
    Wazp3D->UseClearDrawRegion.ON=TRUE;
    Wazp3D->UseClearImage.ON=TRUE;
    Wazp3D->UseMinUpdate.ON=TRUE;
    Wazp3D->UseRatioAlpha.ON=TRUE;
    Wazp3D->UseAlphaMinMax.ON=TRUE;
    Wazp3D->UseStateTracker.ON=TRUE;
    Wazp3D->UseDLL=FALSE;
    Wazp3D->IndirectMode.ON=TRUE;        /* default = force to (faster) indirect mode */

/* default values for cycle-gadgets */
    Wazp3D->Renderer.ON=1;            /* use Soft to bitmap */
    Wazp3D->PolyMode.ON=1;
    Wazp3D->PerspMode.ON=1;
    Wazp3D->TexMode.ON=0;

#if defined(__AROS__)
    Wazp3D->TexMode.ON=1;             /* we assume that AROS' cpu is fast enough to enable nice features */
    Wazp3D->PerspMode.ON=2;
#endif

#ifdef __MORPHOS__
    Wazp3D->Renderer.ON=2;            /* use hard via TinyGL            */
    Wazp3D->PerspMode.ON=0;           /* not supported via TinyGL :-(    */
    Wazp3D->UseFiltering.ON=1;        /* nice and hard done            */
#endif

#ifdef __amigaos4__
    Wazp3D->TexMode.ON=1;             /* we assume AmigaOS4's cpu is fast enough to enable nice features */
    Wazp3D->PerspMode.ON=1;           /* but sam440 is too slow to emulate perspective... */
    Wazp3D->IndirectMode.ON=FALSE;    /* direct mode only on OS4 */
#endif

    Wazp3D->smode=(W3D_ScreenMode *)&Wazp3D->smodelist;
    Wazp3D->drivertype=W3D_DRIVER_CPU;
    Wazp3D->DriverList[0]=&Wazp3D->driver;
    Wazp3D->DriverList[1]=NULL;

    Libstrcpy(Wazp3D->DriverName,DRIVERNAME);
    Wazp3D->driver.name      =Wazp3D->DriverName;
    Wazp3D->driver.ChipID    =W3D_CHIP_UNKNOWN;
    Wazp3D->driver.formats   =SupportedFormats();
    Wazp3D->driver.swdriver  =W3D_TRUE;

    if(Wazp3D->HardwareLie.ON)
    {
        Wazp3D->drivertype=W3D_DRIVER_3DHW;
        Wazp3D->driver.swdriver=W3D_FALSE;
    }

    return(TRUE);
}
/*==================================================================================*/
void WAZP3D_Close()
{
#ifdef WAZP3DDEBUG
    WORD n;

    SREM(WAZP3D_Close)
    if(Wazp3D->DebugCalls.ON)
        LibAlert("List of calls");

    NLOOP(MAXFUNCTIONS)
        if(Wazp3D->DebugCalls.ON)
            {
            if (n==40)    LibAlert("More calls...");
            Libprintf("[%ld] %ld\tcalls to %s \n",(ULONG)(1+n),Wazp3D->FunctionCalls[1+n],Wazp3D->FunctionName[1+n]);
            }
    if(Wazp3D->DebugCalls.ON)
        LibAlert("All calls listed");

    if(Wazp3D->DebugAdresses.ON)
         PrintAllFunctionsAdresses();
#endif
}
/*==================================================================================*/
void WAZP3D_Expunge()
{
    SREM(WAZP3D_Expunge)
    CloseAmigaLibraries();
}
/*==================================================================================*/
ULONG BytesPerPix1(ULONG format)
{
ULONG bpp=0;

    if(format==W3D_CHUNKY)     bpp= 8/8;
    if(format==W3D_A8)     bpp= 8/8;
    if(format==W3D_L8)     bpp= 8/8;
    if(format==W3D_I8)     bpp= 8/8;
    if(format==W3D_A4R4G4B4) bpp=16/8;
    if(format==W3D_A1R5G5B5) bpp=16/8;
    if(format==W3D_R5G6B5)     bpp=16/8;
    if(format==W3D_L8A8)     bpp=16/8;
    if(format==W3D_R8G8B8)     bpp=24/8;
    if(format==W3D_A8R8G8B8) bpp=32/8;
    if(format==W3D_R8G8B8A8) bpp=32/8;
    return(bpp);
}
/*==================================================================================*/
ULONG BytesPerPix2(ULONG format)
{
/* after conversion to RGB24 or RGB32 */
ULONG bpp2;

    bpp2=32/8;
    if(format==W3D_R8G8B8)    bpp2=24/8;
    if(format==W3D_R5G6B5)    bpp2=24/8;
    if(format==W3D_L8)    bpp2=24/8;

    if(Wazp3D->HackTexs.ON==1)        /* only reuse RGBA tex data*/
        bpp2=32/8;

    return(bpp2);
}
/*==========================================================================*/
struct pixels9 {
UBYTE *P00;
UBYTE *P01;
UBYTE *P02;
UBYTE *P10;
UBYTE *P11;
UBYTE *P12;
UBYTE *P20;
UBYTE *P21;
UBYTE *P22;
UBYTE *NewP;
};
/*==========================================================================*/
void SmoothPixel(struct pixels9 *R)
{
register UWORD x;
register ULONG four=4;

        x=R->P11[0]; x=x+x; x=x+x; x=x+x;    /* x8 */
        x=x+R->P00[0]+R->P01[0]+R->P02[0]+R->P10[0]+R->P12[0]+R->P20[0]+R->P21[0]+R->P22[0];
        R->NewP[0]=x>>four;            /* /16 */

        x=R->P11[1]; x=x+x; x=x+x; x=x+x;
        x=x+R->P00[1]+R->P01[1]+R->P02[1]+R->P10[1]+R->P12[1]+R->P20[1]+R->P21[1]+R->P22[1];
        R->NewP[1]=x>>four;

        x=R->P11[2]; x=x+x; x=x+x; x=x+x;
        x=x+R->P00[2]+R->P01[2]+R->P02[2]+R->P10[2]+R->P12[2]+R->P20[2]+R->P21[2]+R->P22[2];
        R->NewP[2]=x>>four;

}
/*==========================================================================*/
void SmoothBitmap(void *image,UWORD large,UWORD high,UBYTE bits)
{
UBYTE AliasedLines[4*MAXSCREEN*2];            /* for smoothing texs */
register UWORD x,y;
register UBYTE *Pdone;
register UBYTE *P=(UBYTE *)image;
register UBYTE *linedone;
register UBYTE *line;
register ULONG px=bits/8;
register ULONG py=large*px;
UBYTE *temp;
struct pixels9 R;

    if (large>MAXSCREEN) return;

    linedone=AliasedLines;
    line      =AliasedLines+py;

    YLOOP(high)
    {
    linedone=AliasedLines;
    line      =AliasedLines+py;
    if(y&1)
        SWAP(line,linedone)

        XLOOP(large)
        {
        R.NewP=line;

        R.P10=P-px;
        R.P11=P;
        R.P12=P+px;
        R.P00=R.P10-py;
        R.P01=R.P11-py;
        R.P02=R.P12-py;
         R.P20=R.P10+py;
        R.P21=R.P11+py;
        R.P22=R.P12+py;

        if(x==0)        { R.P00+=px; R.P10+=px; R.P20+=px; }
        if(x==large)    { R.P02-=px; R.P12-=px; R.P22-=px; }
        if(y==0)        { R.P00+=py; R.P01+=py; R.P02+=py; }
        if(y==high)        { R.P20-=py; R.P21-=py; R.P22-=py; }

        SmoothPixel(&R);

        line+=px;
        P+=px;
        }

    /* copy previous line from previous buffer */
        Pdone=P-py-py;
        if(y!=0)
        XLOOP(large)
        {
        Pdone[0]=linedone[0];
        Pdone[1]=linedone[1];
        Pdone[2]=linedone[2];
        Pdone+=px;
        linedone+=px;
        }
    }

    /* copy last line from last buffer */
    Pdone   =P-py;
    linedone=line-py;
    XLOOP(large)
    {
    Pdone[0]=linedone[0];
    Pdone[1]=linedone[1];
    Pdone[2]=linedone[2];
    Pdone+=px;
    linedone+=px;
    }
}
/*==================================================================================*/
void ConvertBitmap(ULONG format,UBYTE *pt1,UBYTE *pt2,UWORD high,UWORD large,ULONG offset1,ULONG offset2,UBYTE *palette)
{
UBYTE *color8=pt1;
UWORD RGBA16;
UBYTE *RGBA=pt2;
UWORD a,r,g,b,x,y;

SREM(ConvertBitmap)
VAR(high)
VAR(large)
VAR(format)
VAR(pt1)
VAR(pt2)
VAR(offset1)
VAR(offset2)
VAR(palette)

if(pt1==NULL) return;
if(pt2==NULL) return;

    if(Wazp3D->DebugST.ON) DEBUGPRINTF4("ConvertBitmap pt:%ld %ldX%ld format:%ld\n",pt1,large,high,format);

if(format==W3D_CHUNKY)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=palette[(*color8*4)+0];
        RGBA[1]=palette[(*color8*4)+1];
        RGBA[2]=palette[(*color8*4)+2];
        RGBA[3]=palette[(*color8*4)+3];
        RGBA+=4;
        color8++;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

/* A1=one bit alpha. If this alpha is one, the pixel is fully opaque. If the alpha is zero, the pixel is invisible/fully transparent. */
if(format==W3D_A1R5G5B5)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA16=color8[0]*256+color8[1];
        a=(RGBA16 >> 15) *   255 ;
        r=(RGBA16 >> 10) << (8-5);
        g=(RGBA16 >>  5) << (8-5);
        b=(RGBA16 >>  0) << (8-5);
        RGBA[0]=r;
        RGBA[1]=g;
        RGBA[2]=b;
        RGBA[3]=a;
        RGBA+=4;
        color8+=2;
        }
    RGBA16+=offset1;
    RGBA   +=offset2;
    }

if(Wazp3D->HackTexs.ON!=1)        /* only reuse RGBA tex data*/
if(format==W3D_R5G6B5)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA16=color8[0]*256+color8[1];
        r=(RGBA16 >> 11) << (8-5);
        g=(RGBA16 >>  5) << (8-6);
        b=(RGBA16 >>  0) << (8-5);
        RGBA[0]=r;
        RGBA[1]=g;
        RGBA[2]=b;
        RGBA+=3;
        color8+=2;
        }
    RGBA16+=offset1;
    RGBA   +=offset2;
    }

if(Wazp3D->HackTexs.ON==1)        /* only reuse RGBA tex data*/
if(format==W3D_R5G6B5)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA16=color8[0]*256+color8[1];
        r=(RGBA16 >> 11) << (8-5);
        g=(RGBA16 >>  5) << (8-6);
        b=(RGBA16 >>  0) << (8-5);
        RGBA[0]=r;
        RGBA[1]=g;
        RGBA[2]=b;
        RGBA[3]=255;
        RGBA+=4;
        color8+=2;
        }
    RGBA16+=offset1;
    RGBA   +=offset2;
    }


if(format==W3D_A4R4G4B4)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA16=color8[0]*256+color8[1];
        a=(RGBA16 >> 12) << (8-4);
        r=(RGBA16 >>  8) << (8-4);
        g=(RGBA16 >>  4) << (8-4);
        b=(RGBA16 >>  0) << (8-4);
        RGBA[0]=r;
        RGBA[1]=g;
        RGBA[2]=b;
        RGBA[3]=a;
        RGBA+=4;
        color8+=2;
        }
    RGBA16+=offset1;
    RGBA   +=offset2;
    }

if(Wazp3D->HackTexs.ON!=1)        /* only reuse RGBA tex data*/
if(format==W3D_R8G8B8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=color8[0];
        RGBA[1]=color8[1];
        RGBA[2]=color8[2];
        RGBA+=3;
        color8+=3;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }
if(Wazp3D->HackTexs.ON==1)        /* only reuse RGBA tex data*/
if(format==W3D_R8G8B8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=color8[0];
        RGBA[1]=color8[1];
        RGBA[2]=color8[2];
        RGBA[3]=255;
        RGBA+=4;
        color8+=3;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }


if(format==W3D_A8R8G8B8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=color8[1];
        RGBA[1]=color8[2];
        RGBA[2]=color8[3];
        RGBA[3]=color8[0];
        RGBA+=4;
        color8+=4;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

if(format==W3D_R8G8B8A8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=color8[0];
        RGBA[1]=color8[1];
        RGBA[2]=color8[2];
        RGBA[3]=color8[3];
        RGBA+=4;
        color8+=4;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

if(format==W3D_A8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=RGBA[1]=RGBA[2]=0;
        RGBA[3]=color8[0];
        RGBA+=4;
        color8+=1;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

if(Wazp3D->HackTexs.ON!=1)        /* only reuse RGBA tex data*/
if(format==W3D_L8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=RGBA[1]=RGBA[2]=color8[0];
        RGBA+=3;
        color8+=1;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }
if(Wazp3D->HackTexs.ON==1)        /* only reuse RGBA tex data*/
if(format==W3D_L8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=RGBA[1]=RGBA[2]=color8[0];
        RGBA[3]=255;
        RGBA+=4;
        color8+=1;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

if(format==W3D_L8A8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=RGBA[1]=RGBA[2]=color8[0];
        RGBA[3]=color8[1];
        RGBA+=4;
        color8+=2;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

if(format==W3D_I8)
    YLOOP(high)
    {
        XLOOP(large)
        {
        RGBA[0]=RGBA[1]=RGBA[2]=RGBA[3]=color8[0];
        RGBA+=4;
        color8+=1;
        }
    color8+=offset1;
    RGBA  +=offset2;
    }

SREM(ConvertBitmap Done)
}
/*==================================================================================*/
ULONG MakeNewTexdata(W3D_Texture *texture)
{
UWORD high,large,bpp2;
ULONG format=texture->texfmtsrc;

SREM(MakeNewTexdata)

    high =texture->texheight;
    large=texture->texwidth;
    bpp2=BytesPerPix2(format);

VAR(bpp2)
VAR(high)
VAR(large)
    texture->texdata=MMmalloc(high*large*bpp2,"texdata converted");
    if(texture->texdata==NULL)
        return(0);

    ConvertBitmap(format,texture->texsource,texture->texdata,high,large,0,0,(UBYTE *)texture->palette);

    return(bpp2*8);
}
/*=================================================================*/
void TextureAlphaUsage(struct WAZP3D_texture *WT)
{
    ULONG size,n,AllAnb,RatioA;
    ULONG Anb[256];
    UBYTE *RGBA;
    UBYTE A;
    #define RATIOALPHA 20

SREM(TextureAlphaUsage)
    WT->BlendMode=BLENDREPLACE;
    if(WT->bits==32)
    {
        WT->BlendMode=BLENDALPHA;    /* alpha texture with blending */

        if(Wazp3D->UseRatioAlpha.ON)    /* count alpha-pixels  */
        {
            NLOOP(256)
                Anb[n]=0;
            AllAnb=0;
            size=WT->large*WT->high;
            RGBA=WT->pt;
            NLOOP(size)
            {
                A=RGBA[3];
                Anb[A]++;
                if(MINALPHA < A)                /* if source visible   ? */
                if(A < MAXALPHA)                /* if source not solid ? */
                    AllAnb++;
                RGBA+=4;
            }

            if(!Wazp3D->UseAlphaMinMax.ON)            /* then all A values are transparents, except 0 & 255 */
                AllAnb=size-Anb[0]-Anb[255];

            if(AllAnb!=0) RatioA=(100*AllAnb)/size; else RatioA=0;

            if(RatioA<RATIOALPHA)
                WT->BlendMode=BLENDFASTALPHA;

            if(Anb[255] == size) WT->BlendMode=BLENDNOALPHA;    /* all pixels solid ?*/
#ifdef WAZP3DDEBUG
            if(Wazp3D->DebugBlendFunction.ON)
                if(Wazp3D->DebugWazp3D.ON)
                {
                    Libprintf("Tex: %ld pixels A=0:%ld A=255:%ld A=All:%ld RatioA=%ld percent\n",(ULONG)size,(ULONG)Anb[0],(ULONG)Anb[255],(ULONG)AllAnb,(ULONG)RatioA);
                    Libprintf("%s is done, BlendMode(%ld)\n",WT->name,(ULONG)WT->BlendMode);
                }
#endif
        }

    }

}
/*==================================================================================*/
struct VertexFFF{
    float x,y,z;
};
/*==================================================================================*/
struct VertexFFD{
    float x,y;
    double z;
};
/*==================================================================================*/
struct VertexDDD{
    double x,y,z;
};
/*==========================================================================*/
void GetVertex(struct WAZP3D_context *WC,W3D_Vertex *V)
{
struct point3D *P;

    if(!(WC->Pnb < MAXPRIM))
        return;

    P=&(WC->P[WC->Pnb]);
    WC->Pnb++;
    P->x=V->x;
    P->y=V->y;
    P->z=(float)V->z;
    P->w=V->w;
    P->u=V->u * WC->uresize;
    P->v=V->v * WC->vresize;

    if(WC->state.UseGouraud)
        ColorToRGBA(P->RGBA.b,V->color.r,V->color.g,V->color.b,V->color.a);
    else
    {
        COPYRGBA(P->RGBA.L,WC->state.CurrentRGBA.L);    /* default: if a point dont have a color value then it use CurrentColor */
    }
    if (!Wazp3D->DebugPoint.ON) return;
    if(WC->WT!=NULL)
        DEBUGPRINTF1("V Tex%ld ",(ULONG)WC->WT->Tnum)
    else
        DEBUGPRINTF0("V TexNo ")
    PrintP(P);
}
/*==========================================================================*/
void GetPoint(W3D_Context *context,ULONG i)
{
/* MiniGL bug = it dont use W3D_VertexPointer & W3D_TexCoordPointer & W3D_ColorPointer  */
    struct WAZP3D_context *WC=context->driver;
    UBYTE *V        =context->VertexPointer;
    ULONG  Vformat    =context->VPMode;
    ULONG  Vstride    =context->VPStride;

    WORD   unit        =0;
    UBYTE *UV        =context->TexCoordPointer[unit];
    ULONG  UVformat    =context->TPFlags[unit];
    ULONG  UVstride    =context->TPStride[unit];
    ULONG  UVoffsetv    =context->TPVOffs[unit];
    ULONG  UVoffsetw    =context->TPWOffs[unit];

    UBYTE *C        =context->ColorPointer;
    ULONG  Cformat    =context->CPMode;
    ULONG  Cstride    =context->CPStride;

    UBYTE *pt;
    struct VertexFFF *fff;
    struct VertexFFD *ffd;
    struct VertexDDD *ddd;
    float *u;
    float *v;
    float *w;
    float *rgbaF;
    ULONG *rgbaB;
    union
    {
        ULONG L[1];
        uint8_t b[4];
    } RGBA;
    struct point3D *P;

    if(!(WC->Pnb < MAXPRIM))
        return;

    P=&(WC->P[WC->Pnb]);
    P->x=P->y=P->z=P->u=P->v=0.0;
    COPYRGBA(P->RGBA.L,WC->state.CurrentRGBA.L);    /* default: if a point dont have a color value then it use CurrentColor */
    WC->Pnb++;

/* recover XYZ values */
    if(V!=NULL)
    {
        pt=&(V[i*Vstride]);

        if(Vformat==W3D_VERTEX_F_F_F)
        {
            fff=(struct VertexFFF *)pt;
            P->x=fff->x;
            P->y=fff->y;
            P->z=fff->z;
        }

        if(Vformat==W3D_VERTEX_F_F_D)
        {
            ffd=(struct VertexFFD *)pt;
            P->x=ffd->x;
            P->y=ffd->y;
            P->z=(float)ffd->z;
        }

        if(Vformat==W3D_VERTEX_D_D_D)
        {
            ddd=(struct VertexDDD *)pt;
            P->x=(float)ddd->x;
            P->y=(float)ddd->y;
            P->z=(float)ddd->z;
        }

        P->w=1.0/P->z;
    }


/* recover UV values */
    if(UV!=NULL)
    {
        pt=&(UV[i*UVstride]);
        u=(float *)pt;
        v=(float *)&pt[UVoffsetv];
        w=(float *)&pt[UVoffsetw];

        if(UVformat==W3D_TEXCOORD_NORMALIZED)
        {
        P->u=u[0] ;
        P->v=v[0] ;
        P->w=w[0] ;
        }
        else
        {
        P->u=u[0] * WC->uresize;
        P->v=v[0] * WC->vresize;
        P->w=w[0] ;
        }
    }

/* recover Color RGBA values */
    if(C!=NULL)
    {
        pt=&(C[i*Cstride]);
        if(Cformat AND W3D_COLOR_FLOAT)
        {
            rgbaF=(float *)pt;
            ColorToRGBA(RGBA.b,rgbaF[0],rgbaF[1],rgbaF[2],rgbaF[3]);
        }
        if(Cformat AND W3D_COLOR_UBYTE)
        {
            rgbaB=(ULONG *)pt;
            COPYRGBA(RGBA.L,rgbaB);
        }
        if(Cformat AND W3D_CMODE_RGB)     {P->RGBA.b[0]=RGBA.b[0]; P->RGBA.b[1]=RGBA.b[1]; P->RGBA.b[2]=RGBA.b[2]; P->RGBA.b[3]=255;        }
        if(Cformat AND W3D_CMODE_BGR)     {P->RGBA.b[0]=RGBA.b[2]; P->RGBA.b[1]=RGBA.b[1]; P->RGBA.b[2]=RGBA.b[0]; P->RGBA.b[3]=255;        }
        if(Cformat AND W3D_CMODE_RGBA)    {P->RGBA.b[0]=RGBA.b[0]; P->RGBA.b[1]=RGBA.b[1]; P->RGBA.b[2]=RGBA.b[2]; P->RGBA.b[3]=RGBA.b[3];    }
        if(Cformat AND W3D_CMODE_ARGB)    {P->RGBA.b[0]=RGBA.b[1]; P->RGBA.b[1]=RGBA.b[2]; P->RGBA.b[2]=RGBA.b[3]; P->RGBA.b[3]=RGBA.b[0];    }
        if(Cformat AND W3D_CMODE_BGRA)    {P->RGBA.b[0]=RGBA.b[2]; P->RGBA.b[1]=RGBA.b[1]; P->RGBA.b[2]=RGBA.b[0]; P->RGBA.b[3]=RGBA.b[3];    }
    }

    if (!Wazp3D->DebugPoint.ON) return;
    if(WC->WT!=NULL)
        DEBUGPRINTF1("P Tex%ld ",(ULONG)WC->WT->Tnum)
    else
        DEBUGPRINTF0("P TexNo ")
    PrintP(P);
}
/*==========================================================================*/
void     DrawText(W3D_Context *context,WORD x,WORD y,UBYTE *text)
{
    struct WAZP3D_context *WC=context->driver;
    struct RastPort *rp=&WC->rastport;

    SREM(DrawText)
    SetAPen(rp, 0) ;
    RectFill(rp,x-3,y-9,x+8*Libstrlen(text)+3,y+2);

    SetAPen(rp, 2);
    Move(rp,x-2,y-2+context->yoffset);
    Text(rp,(void*)text, Libstrlen(text));

    SetAPen(rp, 2);
    Move(rp,x,y+context->yoffset);
    Text(rp,(void*)text, Libstrlen(text));

    SetAPen(rp, 1);
    Move(rp,x-1,y-1+context->yoffset);
    Text(rp,(void*)text, Libstrlen(text));

    SetAPen(rp, 1);
}
/*==========================================================================*/
void DebugTextureOnScreen(W3D_Context *context)
{
    /* v51: use mouse to grab/show texture parameters */
#ifdef WAZP3DDEBUG
    struct WAZP3D_context *WC=context->driver;
    UBYTE name[256];
    UBYTE *ARGB;
    UBYTE RGBA[4];
    ULONG argb32;
    ULONG TexEnvMode,Tnum,BlendMode,x,y;
    ULONG SrcFunc,DstFunc;
    UBYTE env[5][256];
    UBYTE func[16][256];
    struct WAZP3D_texture *WT;
    struct point3D QuadP[4];
    struct state3D flatstate;


    x=WC->window->MouseX;
    y=WC->window->MouseY;
    argb32=ReadRGBPixel(&WC->rastport,WC->window->MouseX,WC->window->MouseY);
    ARGB=(UBYTE *)&argb32;
    RGBA[0]=ARGB[1]; RGBA[1]=ARGB[2]; RGBA[2]=ARGB[3]; RGBA[3]=ARGB[1];
    Tnum=RGBA[0]; TexEnvMode=RGBA[1]; BlendMode=RGBA[2];

    WT=WC->firstWT;
    while(WT!=NULL)
    {
        if(WT->Tnum==Tnum) break;
        WT=WT->nextWT;
    }

    WC->DebuggedWT=WT;
    if(WT==NULL)
        return;

    strcpy((char*)env[0],"NONE");
    strcpy((char*)env[W3D_REPLACE],"REPLACE");
    strcpy((char*)env[W3D_DECAL],"DECAL");
    strcpy((char*)env[W3D_MODULATE],"MODULATE");
    strcpy((char*)env[W3D_BLEND],"BLEND");

    strcpy((char*)func[0],"NONE");
    strcpy((char*)func[W3D_ZERO],"ZERO");
    strcpy((char*)func[W3D_ONE],"ONE");
    strcpy((char*)func[W3D_SRC_COLOR],"SRC_COLOR");
    strcpy((char*)func[W3D_DST_COLOR],"DST_COLOR");
    strcpy((char*)func[W3D_ONE_MINUS_SRC_COLOR],"ONE_MINUS_SRC_COLOR");
    strcpy((char*)func[W3D_ONE_MINUS_DST_COLOR],"ONE_MINUS_DST_COLOR");
    strcpy((char*)func[W3D_SRC_ALPHA],"SRC_ALPHA");
    strcpy((char*)func[W3D_ONE_MINUS_SRC_ALPHA],"ONE_MINUS_SRC_ALPHA");
    strcpy((char*)func[W3D_DST_ALPHA],"DST_ALPHA");
    strcpy((char*)func[W3D_ONE_MINUS_DST_ALPHA],"ONE_MINUS_DST_ALPHA");
    strcpy((char*)func[W3D_SRC_ALPHA_SATURATE],"SRC_ALPHA_SATURATE");
    strcpy((char*)func[W3D_CONSTANT_COLOR],"CONSTANT_COLOR");
    strcpy((char*)func[W3D_ONE_MINUS_CONSTANT_COLOR],"ONE_MINUS_CONSTANT_COLOR");
    strcpy((char*)func[W3D_CONSTANT_ALPHA],"CONSTANT_ALPHA");
    strcpy((char*)func[W3D_ONE_MINUS_CONSTANT_ALPHA],"ONE_MINUS_CONSTANT_ALPHA");


    if(TexEnvMode>W3D_BLEND) TexEnvMode=0;
    SrcFunc=BlendMode/16; DstFunc=BlendMode%16;
    Libsprintf((char*)name,(char*)"Pos(%ld %ld) Tnum %ld TexEnvMode %ld [%s]",x,y,Tnum,TexEnvMode,env[TexEnvMode]);
    DrawText(context,4,20,name);
    Libsprintf((char*)name,(char*)"BlendMode %ld [%s + %s]",BlendMode,func[SrcFunc],func[DstFunc]);
    if(BlendMode==BLENDFASTALPHA)
        Libsprintf((char*)name,(char*)"BlendMode %ld [BLENDFASTALPHA]",BlendMode);
    if(BlendMode==BLENDCHROMA)
        Libsprintf((char*)name,(char*)"BlendMode %ld [BLENDCHROMA]",BlendMode);
    DrawText(context,4,30,name);

    Libsprintf((char*)name,(char*)"Chroma On%ld Mode%ld Min[%ld %ld %ld %ld] Max[%ld %ld %ld %ld]",StateON(W3D_CHROMATEST),WT->ChromaTestMode,WT->ChromaTestMinRGBA[0],WT->ChromaTestMinRGBA[1],WT->ChromaTestMinRGBA[2],WT->ChromaTestMinRGBA[3],WT->ChromaTestMaxRGBA[0],WT->ChromaTestMaxRGBA[1],WT->ChromaTestMaxRGBA[2],WT->ChromaTestMaxRGBA[3]);
    DrawText(context,4,40,name);

    Libsprintf((char*)name,(char*)"Alpha On%ld Mode%ld Ref[%ld]",StateON(W3D_ALPHATEST),WC->AlphaMode,WC->AlphaRef);
    DrawText(context,4,50,name);

    QuadP[0].x=10;        QuadP[0].y=60;    QuadP[0].z=0; QuadP[0].w=0; QuadP[0].u=0;   QuadP[0].v=0;
    QuadP[1].x=10+128;    QuadP[1].y=60;    QuadP[1].z=0; QuadP[1].w=0; QuadP[1].u=255; QuadP[1].v=0;
    QuadP[2].x=10+128;    QuadP[2].y=60+128;QuadP[2].z=0; QuadP[2].w=0; QuadP[2].u=255; QuadP[2].v=255;
    QuadP[3].x=10;        QuadP[3].y=60+128;QuadP[3].z=0; QuadP[3].w=0; QuadP[3].u=0;   QuadP[3].v=255;
    QuadP[0].RGBA.b[0]=QuadP[1].RGBA.b[0]=QuadP[2].RGBA.b[0]=QuadP[3].RGBA.b[0]=255;
    QuadP[0].RGBA.b[1]=QuadP[1].RGBA.b[1]=QuadP[2].RGBA.b[1]=QuadP[3].RGBA.b[1]=255;
    QuadP[0].RGBA.b[2]=QuadP[1].RGBA.b[2]=QuadP[2].RGBA.b[2]=QuadP[3].RGBA.b[2]=255;
    QuadP[0].RGBA.b[3]=QuadP[1].RGBA.b[3]=QuadP[2].RGBA.b[3]=QuadP[3].RGBA.b[3]=255;

    Wazp3D->DebugTexColor.ON=FALSE;

    flatstate.ZMode=ZMODE(0,W3D_Z_ALWAYS);
    flatstate.BlendMode=BLENDREPLACE;
    flatstate.TexEnvMode=W3D_REPLACE;
    flatstate.PerspMode=0;
    flatstate.CullingMode=W3D_NOW;
    flatstate.UseGouraud=FALSE;
    flatstate.UseTex=TRUE;
    flatstate.UseFog=FALSE;
    flatstate.ST=(ULONG)WT->ST;
    flatstate.Changed=TRUE;

    SOFT3D_SetDrawState(WC->SC,&flatstate);
    SOFT3D_DrawPrimitive(WC->SC,QuadP,4,W3D_PRIMITIVE_POLYGON);
    SOFT3D_Flush(WC->SC);
    Wazp3D->DebugTexColor.ON=TRUE;
#endif
}
/*==========================================================================*/
void DoUpdate(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    UBYTE *ARGB;
    ULONG argb32;
#ifdef WAZP3DDEBUG
    ULONG MilliTime;
    UBYTE name[256];
#endif
REM(DoUpdate)
    DrawPrimitive(context);        /*v53: just in case it remains buffered primitiveS */

    if(WC->PrimitivesDrawn==0)
        return;

    Wazp3D->MaxPolyHack=0;

    if(Wazp3D->Renderer.ON<2)    /* = use soft */
    {

    if(Wazp3D->UseColorHack.ON)
        {
        argb32=ReadRGBPixel(&WC->rastport,0,0);
        ARGB=(UBYTE *)&argb32;
        WC->state.BackRGBA.b[0]=ARGB[1];
        WC->state.BackRGBA.b[1]=ARGB[2];
        WC->state.BackRGBA.b[2]=ARGB[3];
        WC->state.BackRGBA.b[3]=ARGB[0];
        }

    if(Wazp3D->PolyMode.ON==1)
        Wazp3D->MaxPolyHack=MAXPOLYHACK;
    if(Wazp3D->PolyMode.ON==2)
        Wazp3D->MaxPolyHack=MAXPOLYHACK2;
    }

    WC->PrimitivesDrawn=0;
    if(!SOFT3D_DoUpdate(WC->SC))
        return;
#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugWazp3D.ON)
    if(Wazp3D->DumpObject.ON!=0)
        DumpObject(WC);

    MilliTime=LibMilliTimer();
    WC->TimePerFrame= MilliTime - WC->LastMilliTime;
    WC->LastMilliTime=MilliTime;

    if(Wazp3D->DebugWazp3D.ON)
    if(Wazp3D->DisplayFPS.ON)
    {
SREM(DisplayFPS)
    if(WC->TimePerFrame!=0)
        WC->Fps=(((ULONG)1000)/WC->TimePerFrame);
    if(WC->Fps!=0)
        Libsprintf((char*)name,(char*)"Wazp3D FPS:%ld  ",(ULONG)WC->Fps);
    else
        Libsprintf((char*)name,(char*)"Wazp3D %ld ms per frame :-(  ",(ULONG)WC->TimePerFrame);
    DrawText(context,4,10,name);
    DrawText(context,4,10,name);

    if(Wazp3D->DebugTexColor.ON)
        DebugTextureOnScreen(context);
    }

SREM(DoUpdate OK)
#endif
}
/*==========================================================================*/
BOOL OpenSoft3DLib()
{
//#ifdef SOFT3DLIB
#if 0
BOOL DebugState=LibDebug;

    Soft3DBase=OpenLibrary((CONST_STRPTR)"soft3d.library",53);
    if(Soft3DBase == NULL)
        {LibDebug=TRUE;LibAlert("Cant open LIBS:soft3d.library"); LibDebug=DebugState; return(FALSE);}

    Libprintf("soft3d.library opened :-) \n");
#endif
    return(TRUE);    /* Then soft3d is compiled inside Wazp3D so OpenSoft3DLib is allways TRUE */
}
/*==========================================================================*/
W3D_Context    *W3D_CreateContext(ULONG *error, struct TagItem *taglist)
{
    W3D_Context    *context;
    struct WAZP3D_context *WC;
    ULONG tag,data;
    UWORD n;
    ULONG EnableMask,DisableMask;
    ULONG AllStates=~0;
    ULONG UnsupportedStates=  W3D_ANTI_POINT | W3D_ANTI_LINE | W3D_ANTI_POLYGON | W3D_DITHERING | W3D_LOGICOP | W3D_STENCILBUFFER | W3D_ALPHATEST | W3D_SPECULAR | W3D_TEXMAPPING3D | W3D_CHROMATEST;
    ULONG    envsupmask=W3D_REPLACE | W3D_DECAL | W3D_MODULATE  | W3D_BLEND;    /* v40: full implementation */
    BOOL stateOK=TRUE;
    ULONG noerror;

    struct TextAttr attr = {(STRPTR)"topaz.font", 8, 0, 0};         /* For text like the FPS counter          */
    struct IntuitionBase *ibase;
    struct Window *win;
    ULONG WhiteRGBA[1]={0xFFFFFFFF};
	ULONG BlackRGBA[1]={0x000000FF};


#if defined(__AROS__)
    UnsupportedStates= UnsupportedStates | W3D_DOUBLEHEIGHT;
#endif

#ifdef WARP3DV5
    UnsupportedStates= UnsupportedStates | W3D_MULTITEXTURE | W3D_FOG_COORD | W3D_LINE_STIPPLE | W3D_POLYGON_STIPPLE;
#endif

    WAZP3DFUNCTION(1);
    VAR(error)
    VAR(taglist)

    ReadPrefs();        /* for minigl that call Wazp3d's OpenLib only one time */

    if(Wazp3D->HardwareLie.ON)
    {
        Wazp3D->drivertype=W3D_DRIVER_3DHW;
        Wazp3D->driver.swdriver=W3D_FALSE;
    }

    if(error==NULL) error=&noerror; /* StormMesa patch: can be NULL if you don`t want an error code returned */
    *error=W3D_SUCCESS;

    WC=MMmalloc(sizeof(struct WAZP3D_context),"WAZP3D_context");
    if(WC==NULL)
        { *error=W3D_NOMEMORY;PrintError(*error);return(NULL);}
    context=&WC->context;

/* calling SOFT3D in an hook avoid GCC's inlining that cause error "fixed or forbidden register was spilled" */
    WC->SOFT3D_SetBitmap_SetClippingHook=(HOOKEDFUNCTION)SOFT3D_SetBitmap_SetClippingStack;

    DisableMask=AllStates;
    if(Wazp3D->HardwareLie.ON)
        EnableMask=AllStates;
    else
        EnableMask=AllStates & (~UnsupportedStates) ;

    WC->DumpStage=0;
    WC->ModeID=INVALID_ID;

    context->driver=WC;                 /* insert driver specific data here */
    context->gfxdriver=NULL;            /* usable by the GFXdriver */    /* TODO: find if it is the pointer to the driver ? */
    context->drivertype=Wazp3D->drivertype;    /* driver type (3DHW / CPU) */
    context->regbase=NULL;             /* register base */
    context->vmembase=NULL;             /* video memory base */
    context->zbuffer=NULL;             /* Pointer to the Z buffer */
    context->stencilbuffer=NULL;        /* Pointer to the stencil buffer */
    context->state=0;                 /* hardware state (see below) */
    context->drawregion=NULL;        /* destination bitmap */
    context->format=0;             /* bitmap format (see below) */
    context->yoffset=0;             /* Y-Offset (for ScrollVPort-Multibuf.) */
    context->bprow=0;                 /* bytes per row */
    context->width=0;                 /* bitmap width */
    context->height=0;             /* bitmap height */
    context->depth=0;                 /* bitmap depth */
    context->chunky=FALSE;             /* TRUE, if palettized screen mode */
    context->destalpha=FALSE;         /* TRUE, if dest alpha channel available */
    context->zbufferalloc=FALSE;        /* TRUE, is Z buffer is allocated */
    context->stbufferalloc=FALSE;        /* TRUE, is stencil buffer is allocated */
    context->HWlocked=FALSE;         /* TRUE, if 3D HW was locked */
    context->w3dbitmap=FALSE;        /* TRUE, if drawregion points to a W3D_Bitmap */
    context->zbufferlost=FALSE;         /* TRUE, if zbuffer not reallocatable */
    context->reserved3=0;

/*    context->restex=NULL;    */        /* A list of all resident textures */
/*    context->tex=NULL;    */        /* A list of all textures which are not on card */

    context->maxtexwidth  =MAXTEXTURE;        /* -HJF- replaced these for possible */
    context->maxtexheight =MAXTEXTURE;        /* support of non-square textures */
    context->maxtexwidthp =MAXTEXTURE;        /* -HJF- For hardware that has different */
    context->maxtexheightp=MAXTEXTURE;        /* constaints in perspective mode */
                             /* scissor region */
    context->scissor.left    =0;
    context->scissor.top    =0;
    context->scissor.width    =0;
    context->scissor.height    =0;
                             /* fogging parameters */
    context->fog.fog_color.r    =1.0;
    context->fog.fog_color.g    =1.0;
    context->fog.fog_color.b    =1.0;
    context->fog.fog_start        =MINZ;
    context->fog.fog_end        =MAXZ;
    context->fog.fog_density    =0.1;

    context->envsupmask=envsupmask;     /* Mask of supported envmodes */
    context->queue=NULL;             /* queue to buffer drawings */
    context->drawmem=NULL;             /* base address for drawing operations */

    context->globaltexenvmode=W3D_MODULATE;        /* Global texture env mode V41: More OpenGL compatible*/

    context->globaltexenvcolor[0]=1.0;            /* Global texture env color */
    context->globaltexenvcolor[1]=1.0;
    context->globaltexenvcolor[2]=1.0;
    context->globaltexenvcolor[3]=1.0;
    WC->EnvRGBA.b[0]=255;
    WC->EnvRGBA.b[1]=255;
    WC->EnvRGBA.b[2]=255;
    WC->EnvRGBA.b[3]=255;

    context->DriverBase=NULL;        /* Library base of the active driver */

    context->DisableMask=DisableMask;    /* Mask for disable-able states */
    context->EnableMask =EnableMask;    /* Mask for enable-able states */

    context->CurrentChip=W3D_CHIP_UNKNOWN;    /* Chip constant */
    context->DriverVersion=1;            /* Internal driver version */

    context->VertexPointer=NULL;        /* Pointer to the vertex buffer array */
    context->VPStride=0;             /* Stride of vertex array */
    context->VPMode=0;             /* Vertex buffer format */
    context->VPFlags=0;             /* not yet used */

    NLOOP(W3D_MAX_TMU)
    {
    context->CurrentTex[n]=NULL;
    context->TexCoordPointer[n]=NULL;
    context->TPStride[n]=0;            /* Stride of TexCoordPointers */
    context->TPVOffs[n]=0;            /* Offset to V coordinate */
    context->TPWOffs[n]=0;            /* Offset to W coordinate */
    context->TPFlags[n]=0;            /* Flags */
    }

    context->ColorPointer=NULL;         /* Pointer to the color array */
    context->CPStride=0;             /* Color pointer stride */
    context->CPMode=0;             /* Mode and color format */
    context->CPFlags=0;             /* not yet used */

    context->specialbuffer=NULL;        /* Special buffer for chip-dependant use (like command */

    SetState(context,W3D_AUTOTEXMANAGEMENT,TRUE);
    SetState(context,W3D_SYNCHRON ,    FALSE);
    SetState(context,W3D_INDIRECT ,    FALSE);
    SetState(context,W3D_TEXMAPPING ,    TRUE);
    SetState(context,W3D_PERSPECTIVE ,    TRUE);
    SetState(context,W3D_FAST ,        FALSE);
    SetState(context,W3D_GOURAUD ,    TRUE);
    SetState(context,W3D_ZBUFFER ,    FALSE);
    SetState(context,W3D_ZBUFFERUPDATE ,FALSE);
    SetState(context,W3D_BLENDING ,    FALSE);
    SetState(context,W3D_FOGGING ,    FALSE);
    SetState(context,W3D_ANTI_POINT ,    FALSE);
    SetState(context,W3D_ANTI_LINE ,    FALSE);
    SetState(context,W3D_ANTI_POLYGON ,    FALSE);
    SetState(context,W3D_ANTI_FULLSCREEN,FALSE);
    SetState(context,W3D_DITHERING ,    FALSE);
    SetState(context,W3D_LOGICOP ,    FALSE);
    SetState(context,W3D_STENCILBUFFER ,FALSE);
    SetState(context,W3D_DOUBLEHEIGHT ,    FALSE);
    SetState(context,W3D_ALPHATEST ,    FALSE);
    SetState(context,W3D_SPECULAR ,    FALSE);
    SetState(context,W3D_TEXMAPPING3D ,    FALSE);
    SetState(context,W3D_CHROMATEST ,    FALSE);
    SetState(context,W3D_GLOBALTEXENV ,    FALSE);

    while (taglist->ti_Tag != TAG_DONE)
    {

    if (taglist->ti_Tag == TAG_MORE) {
      taglist = (struct TagItem *)taglist->ti_Data;
      continue;
    }

    tag =taglist->ti_Tag  ;    data=taglist->ti_Data ; taglist++;
    VAR(tag)
    VAR(data)

    if(tag==W3D_CC_MODEID )        WC->ModeID            =data;
    if(tag==W3D_CC_BITMAP )        context->drawregion    =(struct BitMap *)data;
    if(tag==W3D_CC_YOFFSET)        context->yoffset        =data;
    if(tag==W3D_CC_DRIVERTYPE)    context->drivertype    =data;
    if(tag==W3D_CC_W3DBM)        context->w3dbitmap    =data;    /* flag */

    if(tag==W3D_CC_INDIRECT)        stateOK=SetState(context,W3D_INDIRECT    ,data);
    if(tag==W3D_CC_GLOBALTEXENV )        stateOK=SetState(context,W3D_GLOBALTEXENV    ,data);
    if(tag==W3D_CC_DOUBLEHEIGHT )        stateOK=SetState(context,W3D_DOUBLEHEIGHT    ,data);
    if(tag==W3D_CC_FAST )            stateOK=SetState(context,W3D_FAST        ,data);

    if(!stateOK)
        *error=W3D_UNSUPPORTEDSTATE;

    if(!Wazp3D->HardwareLie.ON)
    if(context->drivertype == W3D_DRIVER_3DHW)
        *error=W3D_ILLEGALINPUT;

    WTAG(W3D_CC_BITMAP,"destination bitmap")
    WTAG(W3D_CC_YOFFSET,"y-Offset ")
    WTAG(W3D_CC_DRIVERTYPE,"Driver type ")
    WTAG(W3D_CC_W3DBM,"Use W3D_Bitmap")
    WTAG(W3D_CC_INDIRECT,"Indirect drawing ")
    WTAG(W3D_CC_GLOBALTEXENV,"SetTexEnv is global")
    WTAG(W3D_CC_DOUBLEHEIGHT,"Drawing area has double height")
    WTAG(W3D_CC_FAST,"Allow Warp3D to modify passed structures ")
    WTAG(W3D_CC_MODEID,"Specify modeID to use")
    PrintError(*error);

    if(*error!=W3D_SUCCESS)
        {W3D_DestroyContext(context);context=NULL;return(context);}
    }

    WINFO(context->drivertype,W3D_DRIVER_UNAVAILABLE,"driver unavailable ")
    WINFO(context->drivertype,W3D_DRIVER_BEST,"use best mode ")
    WINFO(context->drivertype,W3D_DRIVER_3DHW,"use 3D-HW ")
    WINFO(context->drivertype,W3D_DRIVER_CPU,"use CPU ")

    InitRastPort(&WC->rastport);
    WC->font=OpenFont(&attr);
    SetFont(&WC->rastport,WC->font);
    SetDrMd(&WC->rastport, JAM1);

    if(OpenSoft3DLib()==FALSE)    /* open soft3d.library & soft3d.dll if they exists else is TRUE */
    {
        *error=W3D_NODRIVER;    /* not having soft3d == not having a driver */
        return(NULL);
    }

/* For OpenGL: to know the Warp3D's window so the position & size */
    ibase = (struct IntuitionBase*)IntuitionBase;
    if(ibase)
    {
    win=ibase->ActiveWindow;
    Wazp3D->window=WC->window=win;    /* default: use active window */

    win=win->WScreen->FirstWindow;
    while(win)
        {
        if(win->RPort->BitMap==context->drawregion)
                Wazp3D->window=WC->window=win;    /* or better use bitmap's window */
        win=win->NextWindow;
        }
    }

    if(WC->window)
    {
    WC->windowX=WC->window->LeftEdge;
    WC->windowY=WC->window->TopEdge;

    VAR(WC->window->LeftEdge)
    VAR(WC->window->TopEdge)
    VAR(WC->window->Width)
    VAR(WC->window->Height)
    }

VAR(sizeof(struct WAZP3D_context))
VAR(sizeof(struct WAZP3D_texture))
VAR(sizeof(struct WAZP3D_blendstage))
VAR(sizeof(struct state3D))

/* SetDrawRegion will also do WC->SC=SOFT3D_Start(Wazp3D); so will define SC*/
    SetDrawRegion(context,context->drawregion,context->yoffset,NULL);
    if(WC->SC==NULL)
        {FREEPTR(WC); return(NULL);}

/* If Aros cant do LockBitmapTags then we did a fallback to "soft to Image" so we need to set again context->supportedfmt now */
    context->supportedfmt=SupportedFormats();

    VAR(context->width)
    VAR(context->height)
    VAR(context->depth)
    VAR(context->bprow)
    VAR(context->format)
    VAR(context->drawregion->BytesPerRow)
    VAR(context->drawregion->Rows)
    VAR(context->drawregion->Flags)
    VAR(context->drawregion->Depth)
    VAR(context->drawregion->pad)

    WC->firstWT=NULL;
    WC->CallFlushFrame=WC->CallSetDrawRegion=WC->CallClearZBuffer=FALSE;
    WC->CallSetBlending=FALSE;
    WC->ZCompareMode=W3D_Z_ALWAYS;
    WC->SrcFunc=W3D_SRC_ALPHA;
    WC->DstFunc=W3D_ONE_MINUS_SRC_ALPHA;        /* Seems to be the OpenGL default */
    WC->texture=NULL;
    WC->Tnb=0;            /* texture number */
    WC->Pnb=0;            /* points count in current primitive */

/* Set draw state default values : notex nofog nozbuffer just white color */
    WC->state.ZMode    =ZMODE(0,W3D_Z_ALWAYS);        /* default no z buffer = no test ==> always draw & dont update a zbuffer */
    WC->state.BlendMode=BLENDREPLACE;
    WC->state.UseGouraud=FALSE;
    WC->state.TexEnvMode=0;
    WC->state.PerspMode=0;
    WC->state.CullingMode=W3D_CCW;

    COPYRGBA(WC->state.FogRGBA.L,WhiteRGBA);      /* default white fog           */
    COPYRGBA(WC->state.CurrentRGBA.L,WhiteRGBA);  /* default white color         */
    COPYRGBA(WC->state.EnvRGBA.L,WhiteRGBA);      /* default white env-color     */
    COPYRGBA(WC->state.BackRGBA.L,BlackRGBA);     /* default black background    */

    WC->state.PointSize=1;
    WC->state.LineSize =1;

    WC->state.UseFog=FALSE;
    WC->state.FogMode=0;
    WC->state.FogZmin=MINZ;
    WC->state.FogZmax=MAXZ;
    WC->state.FogDensity=0.0;

    WC->state.UseTex=FALSE;
    WC->state.ST=(ULONG)NULL;
    WC->state.Changed=TRUE;

    if(Wazp3D->IndirectMode.ON)
        SetState(context,W3D_INDIRECT,TRUE);    /*v50: can force to use (faster) indirect mode */

    context->FrontFaceOrder=W3D_CCW;    /* Winding order of front facing triangles . CCW is OpenGL default (Warp3D too?)*/
    W3D_SetFrontFace(context,context->FrontFaceOrder);

    DoUpdate(context);    /* clear Image buffer */
    return(context);
}
/*==========================================================================*/
#if PROVIDE_VARARG_FUNCTIONS
W3D_Context *W3D_CreateContextTags(ULONG *error, Tag tag1, ...)
{
static ULONG tag[100];
va_list va;
WORD n=0;

    WAZP3DFUNCTION(2);
    tag[n] = tag1;
    VAR(tag[n])
    va_start (va, tag1);
    do     {
        n++;    tag[n]= va_arg(va, ULONG);    VAR(tag[n])
        if(n&2) if (tag[n] == TAG_DONE) break;
        }
    while (n<100);
    va_end(va);

    return (W3D_CreateContext(error,(struct TagItem *)tag) );
}
#endif // PROVIDE_VARARG_FUNCTIONS
/*==========================================================================*/
void W3D_DestroyContext(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(3);
    W3D_FreeAllTexObj(context);

    SOFT3D_End(WC->SC);
    VAR(WC->CallFlushFrame)
    VAR(WC->CallSetDrawRegion)
    VAR(WC->CallClearZBuffer)

    FREEPTR(WC);        /* WC also included the W3D_Context */

    if (Wazp3D->DebugContext.ON)
        LibAlert("DestroyContextOK");

    WritePrefs();
}
/*==========================================================================*/
void PrintState(ULONG state, ULONG action)
{
#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugVal.ON)
    if(Wazp3D->DebugState.ON)
        {if(action==W3D_DISABLED) {Libprintf(" [ ] ");} else {Libprintf(" [X] ");}}

    WINFO(state,W3D_AUTOTEXMANAGEMENT,"automatic texture management ")
    WINFO(state,W3D_SYNCHRON,"wait,until HW is idle ")
    WINFO(state,W3D_INDIRECT,"drawing is queued until flushed ")
    WINFO(state,W3D_GLOBALTEXENV,"SetTexEnv is global ")
    WINFO(state,W3D_DOUBLEHEIGHT,"Drawing area is double height ")
    WINFO(state,W3D_FAST,"Allow Warp3D to modify passed structures")
    WINFO(state,W3D_AUTOCLIP,"clip to screen region ")
    WINFO(state,W3D_TEXMAPPING,"texmapping state ")
    WINFO(state,W3D_PERSPECTIVE,"perspective correction state ")
    WINFO(state,W3D_GOURAUD,"gouraud/flat shading ")
    WINFO(state,W3D_ZBUFFER,"Z-Buffer state ")
    WINFO(state,W3D_ZBUFFERUPDATE,"Z-Buffer update state ")
    WINFO(state,W3D_BLENDING,"Alpha blending state ")
    WINFO(state,W3D_FOGGING,"Fogging state ")
    WINFO(state,W3D_ANTI_POINT,"Point antialiasing state ")
    WINFO(state,W3D_ANTI_LINE,"Line antialiasing state ")
    WINFO(state,W3D_ANTI_POLYGON,"Polygon antialiasing state")
    WINFO(state,W3D_ANTI_FULLSCREEN,"Fullscreen antialiasing")
    WINFO(state,W3D_DITHERING,"dithering state ")
    WINFO(state,W3D_LOGICOP,"logic operations ")
    WINFO(state,W3D_STENCILBUFFER,"stencil buffer/stencil");
    WINFO(state,W3D_ALPHATEST,"alpha test ")
    WINFO(state,W3D_SPECULAR,"Specular lighting ")
    WINFO(state,W3D_TEXMAPPING3D,"3d textures ")
    WINFO(state,W3D_SCISSOR,"Scissor test enable ")
    WINFO(state,W3D_CHROMATEST,"Chroma test enable ")
    WINFO(state,W3D_CULLFACE,"Backface culling enable ")
#ifdef WARP3DV5
    WINFO(state,W3D_MULTITEXTURE,"Enable multitexturing & separate blend functions  ")
    WINFO(state,W3D_FOG_COORD,"Use separate fog coord array instead of first texture's w coordinates ")
    WINFO(state,W3D_LINE_STIPPLE,"Line stippling ")
    WINFO(state,W3D_POLYGON_STIPPLE,"Polygon stippling ")
#endif

    if(!Wazp3D->DebugState.ON)
    {
    WINFO(action,W3D_ENABLED,"mode is enabled ")
    WINFO(action,W3D_DISABLED,"mode is disabled ")
    }
#endif
}
/*==========================================================================*/
ULONG W3D_GetState(W3D_Context *context, ULONG state)
{
ULONG action;

    WAZP3DFUNCTION(4);
    if(context->state & state)
        action=W3D_ENABLED;
    else
        action=W3D_DISABLED;
    PrintState(state,action);
    return(action);
}
/*==========================================================================*/
BOOL SetState(W3D_Context *context,ULONG state,BOOL set)
{
struct WAZP3D_context *WC=context->driver;
ULONG newstate;
/* simpler W3D_SetState used internally */

    if(set)
    if(context->EnableMask  & state)
        {
        newstate=context->state |  state;
        if(context->state!=newstate)
            WC->state.Changed=TRUE;
        context->state=newstate;
        return(TRUE);
        }

    if(!set)
    if(context->DisableMask & state)
        {
        newstate=context->state & ~state;
         if(context->state!=newstate)
            WC->state.Changed=TRUE;
        context->state=newstate;
        return(TRUE);
        }
    return(FALSE);
}
/*==========================================================================*/
ULONG W3D_SetState(W3D_Context *context, ULONG state, ULONG action)
{
void W3D_SetFrontFace(W3D_Context* context, ULONG direction);
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(5);
    PrintState(state,action);

/* set state if possible */
    if(SetState(context,state,(action==W3D_ENABLE)) == FALSE)
        WRETURN(W3D_UNSUPPORTEDSTATE);

    if( state==W3D_ZBUFFERUPDATE)
    if(action==W3D_ENABLE)
        SetState(context,W3D_ZBUFFER,W3D_ENABLE);    /*patch: for MiniGL/OS4 that forgot to enable zbuffer*/

    if(state==W3D_BLENDING)
        WC->CallSetBlending=TRUE;
    if(state==W3D_CULLFACE)
        W3D_SetFrontFace(context,context->FrontFaceOrder);

    if(state==W3D_SCISSOR)
    {
        if(action==W3D_ENABLE)            /* V54: Frogatto bug */
        {
        WC->Xmin=context->scissor.left;
        WC->Ymin=context->scissor.top;
        WC->Xmax=WC->Xmin+context->scissor.width-1;
        WC->Ymax=WC->Ymin+context->scissor.height-1;
        }
        else
        {
        WC->Xmin=0;
        WC->Ymin=0;
        WC->Xmax=WC->large;
        WC->Ymax=WC->high;
        }
    SOFT3D_SetClipping(WC->SC,WC->Xmin,WC->Xmax,WC->Ymin,WC->Ymax);
    }

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_Hint(W3D_Context *context, ULONG mode, ULONG quality)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(6);

#ifdef WAZP3DDEBUG
    if(Wazp3D->DebugState.ON)
        {
        if(quality==W3D_H_FAST)        Libprintf(" [FAST] ");
        if(quality==W3D_H_AVERAGE)    Libprintf(" [AVER] ");
        if(quality==W3D_H_NICE)        Libprintf(" [NICE] ");
        ;
        }
    else
        {
        WINFO(quality,W3D_H_FAST,"Low quality,fast rendering ");
        WINFO(quality,W3D_H_AVERAGE,"Medium quality and speed ");
        WINFO(quality,W3D_H_NICE,"Best quality,low speed ");
        }

    WINFO(mode,W3D_H_TEXMAPPING,"quality of general texmapping");
    WINFO(mode,W3D_H_MIPMAPPING,"quality of mipmapping ");
    WINFO(mode,W3D_H_BILINEARFILTER,"quality of bilinear filtering");
    WINFO(mode,W3D_H_MMFILTER,"quality of depth filter");
    WINFO(mode,W3D_H_PERSPECTIVE,"quality of perspective correction");
    WINFO(mode,W3D_H_BLENDING,"quality of alpha blending");
    WINFO(mode,W3D_H_FOGGING,"quality of fogging ");
    WINFO(mode,W3D_H_ANTIALIASING,"quality of antialiasing ");
    WINFO(mode,W3D_H_DITHERING,"quality of dithering ");
    WINFO(mode,W3D_H_ZBUFFER,"quality of ZBuffering ");
#endif

    WC->hints[mode]=quality;

/* else W3D_ILLEGALINPUT */
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_CheckDriver(void)
{
    WAZP3DFUNCTION(7);
    VAR(Wazp3D->drivertype);
    return(Wazp3D->drivertype);
}
/*==========================================================================*/
ULONG W3D_LockHardware(W3D_Context *context)
{
    WAZP3DFUNCTION(8);
    context->HWlocked=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void W3D_UnLockHardware(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(9);

    if(!WC->CallFlushFrame)
    if(!WC->CallSetDrawRegion)
    if(!WC->CallClearZBuffer)
        DoUpdate(context);    /* patch: for Quake = draw the image if the usual update-functions are never called  */

    context->HWlocked=FALSE;
}
/*==========================================================================*/
void W3D_WaitIdle(W3D_Context *context)
{
    WAZP3DFUNCTION(10);
}
/*==========================================================================*/
ULONG W3D_CheckIdle(W3D_Context *context)
{
    WAZP3DFUNCTION(11);
    WRETURN(W3D_SUCCESS);        /* else W3D_BUSY */
}
/*==========================================================================*/
ULONG QueryDriver(W3D_Driver* driver, ULONG query, ULONG destfmt)
{
ULONG support=W3D_NOT_SUPPORTED;
UBYTE sup='N';

    if(query==W3D_Q_SETTINGS)
        {WAZP3D_Settings(); return(W3D_FULLY_SUPPORTED); }        /* 'backdoor' that start the Wazp3D-Prefs interface */

    if(driver!=NULL)                /*patch: StormMesa can send driver=NULL */
    if(driver!=&Wazp3D->driver) return(support);

    VAR(destfmt)
    VAR(query)

    Wazp3D->driver.formats=SupportedFormats();

    if(Wazp3D->HardwareLie.ON)
        {
        Wazp3D->drivertype=W3D_DRIVER_3DHW;
        Wazp3D->driver.swdriver=W3D_FALSE;
        }


    if( destfmt != 0)            /*patch: StormMesa can send this undocumented destfmt=0 meaning all dest_formats */
    if((destfmt & Wazp3D->driver.formats)==0)
        goto querysupport;

    QUERY(W3D_Q_DRAW_POINT,"point drawing ",'Y');
    QUERY(W3D_Q_DRAW_LINE,"line drawing ",'Y');
    QUERY(W3D_Q_DRAW_TRIANGLE,"triangle drawing ",'Y');
    QUERY(W3D_Q_DRAW_POINT_X,"points with size != 1 supported ",'Y');
    QUERY(W3D_Q_DRAW_LINE_X,"lines with width != 1 supported ",'N');
    QUERY(W3D_Q_DRAW_LINE_ST,"line stippling supported ",'N');
    QUERY(W3D_Q_DRAW_POLY_ST,"polygon stippling supported ",'N');
    QUERY(W3D_Q_TEXMAPPING,"texmapping in general ",'Y');
    QUERY(W3D_Q_MIPMAPPING,"mipmapping ",'N');
    QUERY(W3D_Q_BILINEARFILTER,"bilinear filter ",'N');
    QUERY(W3D_Q_MMFILTER,"mipmap filter ",'N');
    QUERY(W3D_Q_LINEAR_REPEAT,"W3D_REPEAT for linear texmapping ",'Y');
    QUERY(W3D_Q_LINEAR_CLAMP,"W3D_CLAMP for linear texmapping ",'N');
    QUERY(W3D_Q_PERSPECTIVE,"perspective correction ",'Y');
    QUERY(W3D_Q_PERSP_REPEAT,"W3D_REPEAT for persp. texmapping ",'Y');
    QUERY(W3D_Q_PERSP_CLAMP,"W3D_CLAMP for persp. texmapping ",'N');
    QUERY(W3D_Q_ENV_REPLACE,"texenv REPLACE ",'Y');
    QUERY(W3D_Q_ENV_DECAL,"texenv DECAL ",'Y');
    QUERY(W3D_Q_ENV_MODULATE,"texenv MODULATE ",'Y');
    QUERY(W3D_Q_ENV_BLEND,"texenv BLEND ",'Y');
    QUERY(W3D_Q_FLATSHADING,"flat shading ",'Y');
    QUERY(W3D_Q_GOURAUDSHADING,"gouraud shading ",'Y');
    QUERY(W3D_Q_ZBUFFER,"Z buffer in general ",'Y');
    QUERY(W3D_Q_ZBUFFERUPDATE,"Z buffer update ",'Y');
    QUERY(W3D_Q_ZCOMPAREMODES,"Z buffer compare modes ",'Y');
    QUERY(W3D_Q_ALPHATEST,"alpha test in general ",'N');
    QUERY(W3D_Q_ALPHATESTMODES,"alpha test modes ",'N');
    QUERY(W3D_Q_BLENDING,"alpha blending ",'Y');
    QUERY(W3D_Q_SRCFACTORS,"source factors ",'Y');
    QUERY(W3D_Q_DESTFACTORS,"destination factors ",'Y');
    QUERY(W3D_Q_FOGGING,"fogging in general ",'Y');
    QUERY(W3D_Q_LINEAR,"linear fogging ",'Y');
    QUERY(W3D_Q_EXPONENTIAL,"exponential fogging ",'Y');
    QUERY(W3D_Q_S_EXPONENTIAL,"square exponential fogging ",'Y');
    QUERY(W3D_Q_ANTIALIASING,"antialiasing in general ",'y');
    QUERY(W3D_Q_ANTI_POINT,"point antialiasing ",'N');
    QUERY(W3D_Q_ANTI_LINE,"line antialiasing ",'N');
    QUERY(W3D_Q_ANTI_POLYGON,"polygon antialiasing ",'N');
    QUERY(W3D_Q_ANTI_FULLSCREEN,"fullscreen antialiasing ",'Y');
    QUERY(W3D_Q_DITHERING,"dithering ",'N');
    QUERY(W3D_Q_SCISSOR,"scissor test ",'Y');
    QUERY(W3D_Q_MAXTEXWIDTH,"max. texture width ",'Y');
    QUERY(W3D_Q_MAXTEXHEIGHT,"max. texture height ",'Y');
    QUERY(W3D_Q_MAXTEXWIDTH_P,"max. texture width persp",'Y');
    QUERY(W3D_Q_MAXTEXHEIGHT_P,"max. texture height persp",'Y');
    QUERY(W3D_Q_RECTTEXTURES,"rectangular textures ",'Y');
    QUERY(W3D_Q_LOGICOP,"logical operations ",'N');
    QUERY(W3D_Q_MASKING,"color/index masking ",'N');
    QUERY(W3D_Q_STENCILBUFFER,"stencil buffer in general ",'N');
    QUERY(W3D_Q_STENCIL_MASK,"mask value ",'N');
    QUERY(W3D_Q_STENCIL_FUNC,"stencil functions ",'N');
    QUERY(W3D_Q_STENCIL_SFAIL,"stencil operation SFAIL ",'N');
    QUERY(W3D_Q_STENCIL_DPFAIL,"stencil operation DPFAIL ",'N');
    QUERY(W3D_Q_STENCIL_DPPASS,"stencil operation DPPASS ",'N');
    QUERY(W3D_Q_STENCIL_WRMASK,"stencil buffer supports write masking ",'N');
    QUERY(W3D_Q_PALETTECONV,"driver can use texture with a palette other than the screen palette on 8 bit screens ",'N');
    QUERY(W3D_Q_DRAW_POINT_FX,"driver supports point fx (fog, zbuffer) ",'Y');
    QUERY(W3D_Q_DRAW_POINT_TEX,"driver supports points textured ",'Y');
    QUERY(W3D_Q_DRAW_LINE_FX,"driver supports line fx ",'Y');
    QUERY(W3D_Q_DRAW_LINE_TEX,"driver supports textured lines ",'Y');
    QUERY(W3D_Q_SPECULAR,"driver supports specular reflection ",'N');
    QUERY(W3D_Q_CULLFACE,"driver supports culling of faces ",'Y');
#ifdef WARP3DV5

    QUERY(W3D_Q_NUM_TMU,"Number of texture units",'N');
    QUERY(W3D_Q_NUM_BLEND,"Number of blend stages",'N');
    QUERY(W3D_Q_ENV_COMBINE,"Supports env combine functions",'N');
    QUERY(W3D_Q_ENV_ADD,"Supports W3D_ADD environment",'N');
    QUERY(W3D_Q_ENV_SUB,"Supports W3D_SUB environment",'N');
    QUERY(W3D_Q_ENV_CROSSBAR,"Supports crossbar texture blending",'N');
    QUERY(W3D_Q_STIPPLE_LINE,"Supports line stippling patterns",'N');
    QUERY(W3D_Q_STIPPLE_POLYGON,"Supports polygon stippling patterns",'N');

    if(query==W3D_Q_STENCIL_FUNC)
            if(Wazp3D->HardwareLie.ON) { support=ALLSTENCILMODES;goto queryvalue;}
    if(query==W3D_Q_STENCIL_SFAIL)
            if(Wazp3D->HardwareLie.ON) { support=ALLSTENCILMODES;goto queryvalue;}
    if(query==W3D_Q_STENCIL_DPFAIL)
            if(Wazp3D->HardwareLie.ON) { support=ALLSTENCILMODES;goto queryvalue;}
    if(query==W3D_Q_STENCIL_DPPASS)
            if(Wazp3D->HardwareLie.ON) { support=ALLSTENCILMODES;goto queryvalue;}

    if(query==W3D_Q_NUM_TMU)
        {support=1; if(Wazp3D->HardwareLie.ON) support=MAXSTAGE;goto queryvalue;}
    if(query==W3D_Q_NUM_BLEND)
        {support=1; if(Wazp3D->HardwareLie.ON) support=MAXSTAGE;goto queryvalue;}

#endif
    if(query==W3D_Q_MAXTEXWIDTH)
        {support=MAXTEXTURE;goto queryvalue;}
    if(query==W3D_Q_MAXTEXHEIGHT)
        {support=MAXTEXTURE;goto queryvalue;}
    if(query==W3D_Q_MAXTEXWIDTH_P)
        {support=MAXTEXTURE;goto queryvalue;}
    if(query==W3D_Q_MAXTEXHEIGHT_P)
        {support=MAXTEXTURE;goto queryvalue;}

    if(Wazp3D->HardwareLie.ON)
        sup='Y';

querysupport:
    if(sup=='Y')
        support=W3D_FULLY_SUPPORTED;
    if(sup=='y')
        support=W3D_PARTIALLY_SUPPORTED;
    if(sup=='N')
        support=W3D_NOT_SUPPORTED;

    WINFO(support,W3D_FULLY_SUPPORTED,"completely supported")
    WINFO(support,W3D_PARTIALLY_SUPPORTED,"partially supported")
    WINFO(support,W3D_NOT_SUPPORTED,"not supported")
    return(support);

queryvalue:
    VAR(support)
    return(support);
}
/*==========================================================================*/
ULONG W3D_Query(W3D_Context *context, ULONG query, ULONG destfmt)
{
    WAZP3DFUNCTION(12);
    return(QueryDriver(&Wazp3D->driver,query,destfmt));    /* only one driver */
}
/*==========================================================================*/
ULONG W3D_GetTexFmtInfo(W3D_Context *context, ULONG texfmt, ULONG destfmt)
{
    WAZP3DFUNCTION(13);
    return(W3D_GetDriverTexFmtInfo(&Wazp3D->driver,texfmt,destfmt));
}
/*==========================================================================*/
ULONG W3D_GetDriverState(W3D_Context *context)
{
    WAZP3DFUNCTION(14);
    WRETURN(W3D_SUCCESS);        /* else W3D_NOTVISIBLE */
}
/*==========================================================================*/
ULONG W3D_GetDestFmt(void)
{
    WAZP3DFUNCTION(15);
    return(Wazp3D->driver.formats);
}
/*==========================================================================*/
W3D_Driver **   W3D_GetDrivers(void)
{
    WAZP3DFUNCTION(16);
    return((W3D_Driver **)&Wazp3D->DriverList);
}
/*==========================================================================*/
ULONG W3D_QueryDriver(W3D_Driver* driver, ULONG query, ULONG destfmt)
{
    WAZP3DFUNCTION(17);
    return(QueryDriver(driver,query,destfmt));
}
/*==========================================================================*/
ULONG W3D_GetDriverTexFmtInfo(W3D_Driver* driver, ULONG texfmt, ULONG destfmt)
{
ULONG support  =W3D_TEXFMT_UNSUPPORTED;
ULONG supported=W3D_TEXFMT_SUPPORTED+W3D_TEXFMT_FAST+W3D_TEXFMT_ARGBFAST;

    WAZP3DFUNCTION(18);
    if(driver!=NULL)                /*patch: StormMesa can send driver=NULL */
    if(driver!=&Wazp3D->driver) return(support);

    if(destfmt==0)                /*patch: StormMesa can send this undocumented destfmt=0 meaning all dest_formats */
        support=supported;

    if(texfmt==W3D_R8G8B8)
        support=supported;

    if(texfmt==W3D_R8G8B8A8)
        support=supported;

    if(Wazp3D->TexFmtLie.ON)
        support=supported;

    if(destfmt !=0)                /*patch: StormMesa can send this undocumented destfmt=0 meaning all dest_formats */
    if((destfmt & Wazp3D->driver.formats)==0)
        support=W3D_TEXFMT_UNSUPPORTED;

    VAR(texfmt)
    VAR(destfmt)
    WINFOB(support,W3D_TEXFMT_SUPPORTED,"format is supported,although it may be converted")
    WINFOB(support,W3D_TEXFMT_FAST,"format directly supported ")
    WINFOB(support,W3D_TEXFMT_CLUTFAST,"format is directly supported on LUT8 screens")
    WINFOB(support,W3D_TEXFMT_ARGBFAST,"format is directly supported on 16/24 bit screens")
    WINFOB(support,W3D_TEXFMT_UNSUPPORTED,"this format is unsupported,and can't be simulated")
    return(support);
}
/*==========================================================================*/
BOOL ScreenModeFilterC(struct Hook* hook,APTR object,APTR message)
{
ULONG ID=(ULONG)message;
struct DimensionInfo dims;
UWORD large,high,bits;
ULONG ok;

    ok=GetDisplayInfoData(NULL, (UBYTE*)&dims, sizeof(struct  DimensionInfo), DTAG_DIMS, ID);
    if(ok)
    {
    large=dims.Nominal.MaxX+1;
    high =dims.Nominal.MaxY+1;
    bits =dims.MaxDepth;

    DEBUGPRINTF4("ScreenModeFilter[%ld]%ld X %ld X %ld\n",(ULONG)ID,(ULONG)large,(ULONG)high,(ULONG)bits);

    if( MAXSCREEN < large) return FALSE;
    if( MAXSCREEN < high ) return FALSE;

    if(Wazp3D->ASLsize)
    {
    if( large < Wazp3D->ASLminX ) return FALSE;
    if( large > Wazp3D->ASLmaxX ) return FALSE;
    if( high  < Wazp3D->ASLminY ) return FALSE;
    if( high  > Wazp3D->ASLmaxY ) return FALSE;
    }

    if(bits < 15) return FALSE;

    if(Wazp3D->OnlyTrueColor.ON)
        if(bits < 24) return FALSE;

    return TRUE;
    }
    return FALSE;

}
/*==========================================================================*/
#ifdef __AROS__
AROS_UFH3(BOOL, ScreenModeFilterASM,
AROS_UFHA(struct Hook *,hook,A0),
AROS_UFHA(APTR ,object , A2),
AROS_UFHA(APTR ,message, A1))
{
    AROS_USERFUNC_INIT
    ScreenModeFilterC(hook,object,message);
    AROS_USERFUNC_EXIT
}

#elif defined(__MORPHOS__)

LONG DispatcherFunc(void)
{
    struct Hook *hook = (struct Hook *)REG_A0;
    return (*(LONG(*)(struct Hook *,LONG,LONG))hook->h_SubEntry)(hook,REG_A2,REG_A1);
}
static struct EmulLibEntry    GATEDispatcherFunc=    { TRAP_LIB, 0, (void (*)(void))DispatcherFunc };
struct Hook screenreqhook = { 0, 0, (ULONG(*)())&GATEDispatcherFunc, (ULONG(*)())ScreenModeFilterC, 0 };

#else
#ifdef __amigaos4__

BOOL ScreenModeFilterASM(void)             /* unused */
{    return(1);      }

#else

BOOL ScreenModeFilterASM(struct Hook* hook __asm("a0"), APTR object __asm("a2"),APTR message __asm("a1"))
{
    return(ScreenModeFilterC(hook,object,message));
}

#endif
#endif
/*==========================================================================*/
#include <utility/hooks.h>
ULONG W3D_RequestMode(struct TagItem *taglist)
{
//    W3D_Driver *driver;
    ULONG tag,data;
    ULONG /*size,format,drivertype,*/ModeID=INVALID_ID;
    struct ScreenModeRequester *requester;
    struct Hook filter;

#ifdef __amigaos4__
    filter.h_Entry =(HOOKFUNC)ScreenModeFilterC;
#elif defined(__MORPHOS__)

/*
    filter.h_Entry = (HOOKFUNC)HookEntry;
    filter.h_SubEntry =(HOOKFUNC)ScreenModeFilterC;
*/

#else
    filter.h_Entry =(HOOKFUNC)ScreenModeFilterASM;
#endif

    WAZP3DFUNCTION(19);
//    size=TRUE;
//    driver=&Wazp3D->driver;
    Wazp3D->ASLminX=0;
    Wazp3D->ASLmaxX=MAXSCREEN;
    Wazp3D->ASLminY=0;
    Wazp3D->ASLmaxY=MAXSCREEN;
    Wazp3D->ASLsize=FALSE;

//    drivertype=Wazp3D->drivertype;
//    format=Wazp3D->driver.formats;

    while (taglist->ti_Tag != TAG_DONE)
    {
    if (taglist->ti_Tag == TAG_MORE) {
      taglist = (struct TagItem *)taglist->ti_Data;
      continue;
    }
    tag =taglist->ti_Tag  ;    data=taglist->ti_Data ; taglist++;
    if(tag==W3D_SMR_SIZEFILTER)    Wazp3D->ASLsize    =TRUE;
//    if(tag==W3D_SMR_DRIVER )    driver        =(W3D_Driver *)data;

//    if(tag==W3D_SMR_DESTFMT)    format        =data;
//    if(tag==W3D_SMR_TYPE)        drivertype        =data;

    if(tag==ASLSM_MinWidth)        Wazp3D->ASLminX    =data;
    if(tag==ASLSM_MaxWidth)        Wazp3D->ASLmaxX    =data;
    if(tag==ASLSM_MinHeight)    Wazp3D->ASLminY    =data;
    if(tag==ASLSM_MaxHeight)    Wazp3D->ASLmaxY    =data;

    WTAG(W3D_SMR_DRIVER,"Driver to filter ")
    WTAG(W3D_SMR_DESTFMT,"Dest Format to filter ")
    WTAG(W3D_SMR_TYPE,"Type to filter ")
    WTAG(W3D_SMR_SIZEFILTER,"Also filter size ")

    WTAG(W3D_SMR_MODEMASK,"AND-Mask for modes ")
    WTAG(ASLSM_MinWidth," ")
    WTAG(ASLSM_MaxWidth," ")
    WTAG(ASLSM_MinHeight," ")
    WTAG(ASLSM_MaxHeight," ")
    }


REM(alloc asl)

#ifdef __MORPHOS__
    requester = (struct ScreenModeRequester *)AllocAslRequestTags(ASL_ScreenModeRequest,
    ASLSM_DoWidth, TRUE,
    ASLSM_DoHeight, TRUE,
    ASLSM_MinDepth, 15,
    ASLSM_MaxDepth, 32,
    ASLSM_PropertyFlags, DIPF_IS_WB,
    ASLSM_PropertyMask, DIPF_IS_WB,
/*    ASLSM_FilterFunc, (ULONG) &screenreqhook,*/
    TAG_DONE);
#else
    requester = (struct ScreenModeRequester *)AllocAslRequestTags(
    ASL_ScreenModeRequest,
    ASLSM_TitleText,(ULONG) "Wazp3D Screen Modes ",
    ASLSM_FilterFunc,(ULONG)&filter,
    TAG_DONE );
#endif


    if(requester)
    {
VAR(requester)
        if(AslRequest(requester,NULL))
            ModeID =requester->sm_DisplayID;
        else
            ModeID=INVALID_ID;
VAR(ModeID)
        FreeAslRequest(requester);
    }

    VAR(ModeID)
    return(ModeID);
}
/*==========================================================================*/
#if PROVIDE_VARARG_FUNCTIONS
ULONG         W3D_RequestModeTags(Tag tag1, ...)
{
static ULONG tag[100];
va_list va;
WORD n=0;

    WAZP3DFUNCTION(20);
    tag[n] = tag1;
    VAR(tag[n])
    va_start (va, tag1);
    do     {
        n++;    tag[n]= va_arg(va, ULONG);    VAR(tag[n])
        if(n&2) if (tag[n] == TAG_DONE) break;
        }
    while (n<100);
    va_end(va);

    return (W3D_RequestMode((struct TagItem *)tag));
}
#endif // PROVIDE_VARARG_FUNCTIONS
/*==========================================================================*/
W3D_Driver *    W3D_TestMode(ULONG ModeID)
{
ULONG /*format,*/bits,bytesperpixel;
W3D_Driver *driver;

    WAZP3DFUNCTION(21);
    VAR(ModeID);
    driver=&Wazp3D->driver;

    if( ModeID==(ULONG)INVALID_ID)
        driver=NULL;
    if(!IsCyberModeID(ModeID ))
        driver=NULL;

//    format          =GetCyberIDAttr(CYBRIDATTR_PIXFMT,ModeID);
    bits            =GetCyberIDAttr(CYBRIDATTR_DEPTH ,ModeID);
    bytesperpixel   =GetCyberIDAttr(CYBRIDATTR_BPPIX ,ModeID);

    if(Wazp3D->Renderer.ON==0)        /* use Soft to Image */
    {
    if(bits<15)
        driver=NULL;
    if(bytesperpixel<2)
        driver=NULL;
    }

    if(Wazp3D->Renderer.ON==1)        /* use Soft to bitmap */
    {
    if(bits<8)
        driver=NULL;
    if(bytesperpixel<1)
        driver=NULL;
    }


#ifdef SOFT3DLIB
    if(Wazp3D->Renderer.ON==2)        /* use Hard */
    {
    if(bits!=32)
        driver=NULL;
    if(bytesperpixel<4)
        driver=NULL;
    }

    if(Wazp3D->Renderer.ON==3)        /* use Hard(Overlay)*/
    {
    if(bits<8)
        driver=NULL;
    if(bytesperpixel<1)
        driver=NULL;
    }
#endif

    if(Wazp3D->OnlyTrueColor.ON)
    if(bytesperpixel<3)
    if(bits<24)
        driver=NULL;

    VAR(driver);
    return(driver);
}
/*==========================================================================*/
void ARGBtoRGBA(UBYTE *RGBA,ULONG size)
{
register UBYTE a,r,g,b;
register ULONG n;

    NLOOP(size)
    {
        a=RGBA[0];
        r=RGBA[1];
        g=RGBA[2];
        b=RGBA[3];
        RGBA[0]=r;
        RGBA[1]=g;
        RGBA[2]=b;
        RGBA[3]=a;
        RGBA+=4;
    }
}
/*==========================================================================*/
W3D_Texture    *W3D_AllocTexObj(W3D_Context *context, ULONG *error,struct TagItem *taglist)
{
W3D_Texture *texture=NULL;
struct WAZP3D_context *WC;
struct WAZP3D_texture *WT;
ULONG tag,data;
ULONG n;
APTR *MipPt=NULL;
LONG size;
UWORD bits=0;
ULONG mask=1;
ULONG noerror;
UBYTE TexFlags;

    WAZP3DFUNCTION(22);
    if(error==NULL) error=&noerror; /* patch: StormMesa send error as NULL cause it don`t want an error code returned */

    WT=MMmalloc(sizeof(struct WAZP3D_texture),"WAZP3D_texture");
    if(WT==NULL)
    {
        *error=W3D_NOMEMORY;
        PrintError(*error);
        return(NULL);
    }
    texture=&WT->texture;
    texture->driver=WT;

    while (taglist->ti_Tag != TAG_DONE)
    {
        if (taglist->ti_Tag == TAG_MORE) {
            taglist = (struct TagItem *)taglist->ti_Data;
            continue;
        }
        tag =taglist->ti_Tag  ;        data=taglist->ti_Data ; taglist++;

        if(tag==W3D_ATO_IMAGE )        texture->texsource =(void *)data;
        if(tag==W3D_ATO_WIDTH )        texture->texwidth  =data;         /* texture width in pixels */
        if(tag==W3D_ATO_HEIGHT)        texture->texheight =data;         /* texture width in pixels */
        if(tag==W3D_ATO_FORMAT)        texture->texfmtsrc =data;         /* texture format (from W3D_ATO_FORMAT) */
        if(tag==W3D_ATO_MIPMAP)
        {
            texture->mipmap=TRUE;             /* TRUE, if mipmaps are supported */
            texture->mipmapmask=data;         /* which mipmaps have to be generated */
        }
        if(tag==W3D_ATO_MIPMAPPTRS)    MipPt           =(void *)data;
        if(tag==W3D_ATO_PALETTE)    texture->palette   =(void *)data;       /* texture palette for chunky textures */

        WTAG(W3D_ATO_IMAGE,"texture image ")
        WTAG(W3D_ATO_FORMAT,"source format ")
        WTAG(W3D_ATO_WIDTH,"border width ")
        WTAG(W3D_ATO_HEIGHT,"border height ")
        WTAG(W3D_ATO_MIPMAP,"mipmap mask ")
        WTAG(W3D_ATO_PALETTE,"texture palette ")
        WTAG(W3D_ATO_MIPMAPPTRS,"array of mipmap")
    }

    WT->large=texture->texwidth ;
    WT->high =texture->texheight;

    if(Wazp3D->HardwareLie.ON)
        texture->resident=TRUE;        /* TRUE, if texture is on card */
    else
        texture->resident=FALSE;

    texture->bytesperpix=BytesPerPix1(texture->texfmtsrc);
    texture->texdata=NULL;

    if(Wazp3D->TexFmtLie.ON)
    if(texture->texfmtsrc==W3D_A8R8G8B8)
    if(Wazp3D->HackTexs.ON==3)        /* reuse ARGB tex data*/
    {
/*directly convert original texture data to RGBA (texsource) */
        ARGBtoRGBA(texture->texsource,texture->texheight*texture->texwidth);
        texture->texfmtsrc=W3D_R8G8B8A8;
    }

    texture->matchfmt=FALSE;        /* TRUE, if srcfmt = destfmt */
    if(texture->texfmtsrc==W3D_R8G8B8)
        {bits=24;texture->matchfmt=TRUE;}

    if(Wazp3D->HackTexs.ON>=2)        /* if reuse RGB tex data*/
    if(texture->texfmtsrc==W3D_R8G8B8A8)
        {bits=32;texture->matchfmt=TRUE;}

    /* patch: for "I have no tomatoes" game => allways MakeNewTexdata(). Because in this game the original tex-picture is freed (so lost)  */
    if(Wazp3D->HackTexs.ON==0)        /* if never reuse tex data*/
        texture->matchfmt=FALSE;

    if(Wazp3D->TexFmtLie.ON)
    if(!texture->matchfmt)
        bits=MakeNewTexdata(texture);

    if(bits==0)
        {FREEPTR(texture->driver); *error=W3D_NOMEMORY; return(NULL); }             /* texture->driver=WT */

    if(!Wazp3D->TexFmtLie.ON)
    if(!texture->matchfmt)
        {FREEPTR(texture->driver); *error=W3D_UNSUPPORTEDTEXFMT; return(NULL); }     /* texture->driver=WT */

    texture->dirty=FALSE;             /* TRUE, if texture image was changed */
    texture->reserved1=FALSE;
    texture->reserved2=FALSE;

    if(MipPt!=NULL)
    NLOOP(16)
    {
        if(texture->mipmapmask & mask )
            texture->mipmaps[n]=NULL;
        else
            texture->mipmaps[n]=*MipPt++;         /* mipmap images are given*/
        mask=mask*2;
    }

    texture->texdest=NULL;                         /* texture location on card */

    texture->bytesperrow=texture->texwidth*texture->bytesperpix;    /* bytes per row */
    texture->texdestsize=texture->bytesperrow*texture->texheight;    /* size of VRAM allocation */
    if(texture->mipmap)
        texture->texdestsize=texture->texdestsize+texture->texdestsize/3;

    size=65536;
    NLOOP(16)
    {
        if(size >= texture->texwidth)
            texture->texwidthexp =n;                 /* texture width exponent */
        if(size >= texture->texheight)
            texture->texheightexp=n;                 /* texture width exponent */
        size=size/2;
    }

    WC=context->driver;            /* insert new texture in textures list */
    WT->nextWT =WC->firstWT;
    WC->firstWT=WT;

    if(texture->matchfmt)
        WT->pt    =texture->texsource;
    else
        WT->pt    =texture->texdata;

    WT->large     =texture->texwidth ;
    WT->high     =texture->texheight;
    WT->bits     =bits;
    if(bits==24) WT->format=W3D_R8G8B8;
    if(bits==32) WT->format=W3D_R8G8B8A8;

    WT->TexEnv=W3D_MODULATE;        /* default texture's env mode (OpenGL) */

    PrintTexture(texture);

    WINFO(texture->texfmtsrc,W3D_CHUNKY,"palettized ")
    WINFO(texture->texfmtsrc,W3D_A1R5G5B5,"a rrrrr ggggg bbbbb ")
    WINFO(texture->texfmtsrc,W3D_R5G6B5,"rrrrr gggggg bbbbb ")
    WINFO(texture->texfmtsrc,W3D_R8G8B8,"rrrrrrrr gggggggg bbbbbbbb ")
    WINFO(texture->texfmtsrc,W3D_A4R4G4B4,"aaaa rrrr gggg bbbb ")
    WINFO(texture->texfmtsrc,W3D_A8R8G8B8,"aaaaaaaa rrrrrrrr gggggggg  ")
    WINFO(texture->texfmtsrc,W3D_A8,"aaaaaaaa ")
    WINFO(texture->texfmtsrc,W3D_L8,"llllllll ")
    WINFO(texture->texfmtsrc,W3D_L8A8,"llllllll aaaaaaaa ")
    WINFO(texture->texfmtsrc,W3D_I8,"iiiiiiii ")
    WINFO(texture->texfmtsrc,W3D_R8G8B8A8,"rrrrrrrr gggggggg bbbbbbbb")
#ifdef WARP3DV5
    WINFO(texture->texfmtsrc,W3D_COMPRESSED_R5G6B5,"Compressed color, no alpha")
    WINFO(texture->texfmtsrc,W3D_A4_COMPRESSED_R5G6B5,"4 bit alpha, compressed color")
    WINFO(texture->texfmtsrc,W3D_COMPRESSED_A8R5G6B5,"compressed alpha/color")
#endif

    WC->Tnb++;     WT->Tnum=WC->Tnb;
    Libsprintf((char*)WT->name,(char*)"T:Texture%ld_%ldX%ldX%ld.RAW",(ULONG)WT->Tnum,(ULONG)WT->large,(ULONG)WT->high,(ULONG)WT->bits);
    if(Wazp3D->DebugWazp3D.ON)
    if(Wazp3D->DumpTextures.ON)
        Libsavefile(WT->name,WT->pt,WT->large*WT->high*WT->bits/8);

    if(Wazp3D->ReloadTextures.ON)
        Libloadfile(&WT->name[2],WT->pt,WT->large*WT->high*WT->bits/8);    /* remove "T:" from WT->name */

    if(Wazp3D->SmoothTextures.ON)
        SmoothBitmap(WT->pt,WT->large,WT->high,WT->bits);

    TextureAlphaUsage(WT);        /* analyze if the tex really got transparent pixels */
    TexFlags=(Wazp3D->UseFiltering.ON*2+Wazp3D->DoMipMaps.ON*1);
    WT->ST=SOFT3D_CreateTexture(WC->SC,WT->pt,WT->large,WT->high,WT->format,TexFlags);
    SOFT3D_Debug(WT->ST);
    if(WT->ST==NULL)
        { *error=W3D_NOMEMORY;PrintError(*error);return(NULL);}

    PrintWT(WT);

    *error=W3D_SUCCESS;
    PrintError(*error);
    return(texture);
}
/*==========================================================================*/
#if PROVIDE_VARARG_FUNCTIONS
W3D_Texture    *W3D_AllocTexObjTags(W3D_Context *context, ULONG *error,Tag tag1, ...)
{
static ULONG tag[100];
va_list va;
WORD n=0;

    WAZP3DFUNCTION(23);
    tag[n] = tag1;
    VAR(tag[n])
    va_start (va, tag1);
    do     {
        n++;    tag[n]= va_arg(va, ULONG);    VAR(tag[n])
        if(n&2) if (tag[n] == TAG_DONE) break;
        }
    while (n<100);
    va_end(va);

    return W3D_AllocTexObj(context,error,(struct TagItem *)tag);
}
#endif // PROVIDE_VARARG_FUNCTIONS
/*==========================================================================*/
void W3D_FreeTexObj(W3D_Context *context, W3D_Texture *texture)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT;
struct WAZP3D_texture fakeWT;
struct WAZP3D_texture *thisWT=&fakeWT;
WORD Ntexture=0,n;

    WAZP3DFUNCTION(24);
    VAR(texture)
    if(texture==NULL) return;     /* patch: StormMesa use this undocumented NULL value for texture */
    WT=texture->driver;
    PrintWT(WT);

    DrawPrimitive(context);        /*v53: just in case it remains buffered primitiveS using this tex */
    SOFT3D_Flush(WC->SC);         /* patch: v50 just in case it remains pixels using this tex */

    NLOOP(W3D_MAX_TMU)
        if(context->CurrentTex[n]==texture)         /* patch: v50 for Blender/MiniGL/OS4 : avoid to use again a freed texture */
            W3D_BindTexture(context,n,NULL);

    thisWT->nextWT=WC->firstWT;
    while(thisWT!=NULL)
    {
    VAL(Ntexture)
    if(thisWT->nextWT==WT)
        {
        SREM(is texture found)
        if(thisWT->nextWT==WC->firstWT)
            WC->firstWT=WT->nextWT;
        else
            thisWT->nextWT=WT->nextWT;
        W3D_ReleaseTexture(context,texture);
        SOFT3D_FreeTexture(WC->SC,WT->ST);
        FREEPTR(WT);             /* WT also contain texture*/
        break;
        }
    thisWT=thisWT->nextWT;
    Ntexture++;
    }

    PrintAllT(context);
}
/*==========================================================================*/
void W3D_ReleaseTexture(W3D_Context *context, W3D_Texture *texture)
{

    WAZP3DFUNCTION(25);
    VAR(texture)
    if(texture==NULL) return;     /* patch: StormMesa use this undocumented NULL value for texture */
    if(texture->texdata==NULL)
    {
        SREM(texture got no converted texdata);
    }
    FREEPTR(texture->texdata);
}
/*==========================================================================*/
void W3D_FlushTextures(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT;

    WAZP3DFUNCTION(26);
    WT=WC->firstWT;
    while(WT!=NULL)
     {
    W3D_ReleaseTexture(context,&WT->texture);
    WT=WT->nextWT;    /* my own linkage */
    }
}
/*==========================================================================*/
ULONG W3D_SetFilter(W3D_Context *context, W3D_Texture *texture,ULONG MinFilter, ULONG MagFilter)
{
struct WAZP3D_texture *WT=texture->driver;

    WAZP3DFUNCTION(27);
    if(MinFilter==W3D_NEAREST)            {WT->MinFiltering=FALSE;WT->MinMipmapping=FALSE;}
    if(MinFilter==W3D_LINEAR)            {WT->MinFiltering=TRUE; WT->MinMipmapping=FALSE;}
    if(MinFilter==W3D_NEAREST_MIP_NEAREST)    {WT->MinFiltering=FALSE;WT->MinMipmapping=TRUE;}
    if(MinFilter==W3D_LINEAR_MIP_NEAREST)    {WT->MinFiltering=TRUE; WT->MinMipmapping=TRUE;}
    if(MinFilter==W3D_NEAREST_MIP_LINEAR)    {WT->MinFiltering=FALSE;WT->MinMipmapping=TRUE;}
    if(MinFilter==W3D_LINEAR_MIP_LINEAR)    {WT->MinFiltering=TRUE; WT->MinMipmapping=TRUE;}
    if(MagFilter==W3D_NEAREST)            {WT->MagFiltering=FALSE;}
    if(MagFilter==W3D_LINEAR)            {WT->MagFiltering=TRUE;}
    WT->MinFilter=MinFilter;
    WT->MagFilter=MagFilter;

    WINFO(MinFilter,W3D_NEAREST,"no mipmapping,no filtering ")
    WINFO(MinFilter,W3D_LINEAR,"no mipmapping,bilinear filtering ")
    WINFO(MinFilter,W3D_NEAREST_MIP_NEAREST,"mipmapping,no filtering ")
    WINFO(MinFilter,W3D_NEAREST_MIP_LINEAR,"mipmapping,bilinear filtering ")
    WINFO(MinFilter,W3D_LINEAR_MIP_NEAREST,"filtered mipmapping,no filtering ")
    WINFO(MinFilter,W3D_LINEAR_MIP_LINEAR,"mipmapping,trilinear filtering ")

    WINFO(MagFilter,W3D_NEAREST,"no mipmapping,no filtering ")
    WINFO(MagFilter,W3D_LINEAR,"no mipmapping,bilinear filtering ")
    WINFO(MagFilter,W3D_NEAREST_MIP_NEAREST,"mipmapping,no filtering ")
    WINFO(MagFilter,W3D_NEAREST_MIP_LINEAR,"mipmapping,bilinear filtering ")
    WINFO(MagFilter,W3D_LINEAR_MIP_NEAREST,"filtered mipmapping,no filtering ")
    WINFO(MagFilter,W3D_LINEAR_MIP_LINEAR,"mipmapping,trilinear filtering ")

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetTexEnv(W3D_Context *context, W3D_Texture *texture,ULONG envparam, W3D_Color *envcolor)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT;
BOOL globaltexenv;

    WAZP3DFUNCTION(28);

    WINFO(envparam,W3D_REPLACE,"unlit texturing ")
    WINFO(envparam,W3D_DECAL,"same as W3D_REPLACE use alpha to blend texture with primitive =lit-texturing")
    WINFO(envparam,W3D_MODULATE,"lit-texturing by modulation ")
    WINFO(envparam,W3D_BLEND,"blend with environment color ")
#ifdef WARP3DV5
    WINFO(envparam,W3D_ADD,"add ")
    WINFO(envparam,W3D_SUB,"sub ")
#endif

    globaltexenv=StateON(W3D_GLOBALTEXENV);
    if(texture==NULL)
        globaltexenv=TRUE;     /* patch: StormMesa use this undocumented NULL value for texture */

    if(globaltexenv)
    {
    context->globaltexenvmode=envparam;                /* Global texture environment mode */
    if(envcolor!=NULL)                        /* envcolor is only specified when envparam == W3D_BLEND so can be NULL */
        {
        context->globaltexenvcolor[0]=envcolor->r;    /* global texture env color */
        context->globaltexenvcolor[1]=envcolor->g;
        context->globaltexenvcolor[2]=envcolor->b;
        context->globaltexenvcolor[3]=envcolor->a;
        ColorToRGBA(WC->EnvRGBA.b,context->globaltexenvcolor[0],context->globaltexenvcolor[1],context->globaltexenvcolor[2],context->globaltexenvcolor[3]);
        PrintRGBA(WC->EnvRGBA.b);
        }
    }
    else
    {
        WT=texture->driver;
        WT->TexEnv=envparam;
        if(envcolor!=NULL)                        /* envcolor is only specified when envparam == W3D_BLEND so can be NULL */
        {
            ColorToRGBA(WT->EnvRGBA.b,envcolor->r,envcolor->g,envcolor->b,envcolor->a);
            PrintRGBA(WT->EnvRGBA.b);
        }
    }

    WC->texture=NULL;            /* so will force SetTexStates() to re-read texture parameters */
    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetWrapMode(W3D_Context *context, W3D_Texture *texture,ULONG s_mode, ULONG t_mode, W3D_Color *bordercolor)
{
struct WAZP3D_texture *WT=texture->driver;

    WAZP3DFUNCTION(29);
    WT->Smode=s_mode;
    WT->Tmode=t_mode;
    if(bordercolor!=NULL)
        ColorToRGBA(WT->BorderRGBA,bordercolor->r,bordercolor->g,bordercolor->b,bordercolor->a);

    WINFO(s_mode,W3D_REPEAT,"texture is repeated ")
    WINFO(s_mode,W3D_CLAMP,"texture is clamped")

    WINFO(t_mode,W3D_REPEAT,"texture is repeated ")
    WINFO(t_mode,W3D_CLAMP,"texture is clamped ")
    PrintRGBA((UBYTE *)& WT->BorderRGBA );
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_UpdateTexImage(W3D_Context *context, W3D_Texture *texture,void *teximage, int level, ULONG *palette)
{
W3D_Scissor scissor;
ULONG srcbpr;

    WAZP3DFUNCTION(30);
VAR(level)
    if(level!=0)
        WRETURN(W3D_SUCCESS);    /* 0 is the tex !=0 are the mipmaps */

    scissor.left=0;
    scissor.top =0;
    scissor.width =texture->texwidth ;
    scissor.height=texture->texheight;
    srcbpr=texture->bytesperrow;
    return(W3D_UpdateTexSubImage(context,texture,teximage,level,palette,&scissor,srcbpr));
}
/*==========================================================================*/
ULONG W3D_UpdateTexSubImage(W3D_Context *context, W3D_Texture *texture, void *teximage,ULONG level, ULONG *palette, W3D_Scissor* scissor, ULONG srcbpr)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT=texture->driver;
ULONG offset1,offset2;
UBYTE *RGB1;
UBYTE *RGB2;
UWORD x,y,high,large,Tlarge,bpp1,bpp2;
ULONG format,sizelarge;

    WAZP3DFUNCTION(31);

VAR(level)
    if(level!=0)
        WRETURN(W3D_SUCCESS);    /* 0 is the tex !=0 are the mipmaps */

    x    =scissor->left;
    y    =scissor->top;
    high    =scissor->height;
    large    =scissor->width;
    Tlarge=texture->texwidth;
    format=texture->texfmtsrc;

    bpp1=texture->bytesperpix;                /* 1=new image */
    offset1=srcbpr - large*bpp1;
    RGB1=(UBYTE *)teximage;

    bpp2=texture->bytesperpix;
    offset2=texture->bytesperrow - large*bpp2;    /*2=original texture */
    RGB2=(UBYTE *)texture->texsource;
    RGB2=(UBYTE *)&RGB2[(y*Tlarge + x)*bpp2];


VAR(x)
VAR(y)
VAR(high )
VAR(large)
VAR(offset1)
VAR(offset2)
VAR(RGB1)
VAR(RGB2)
VAR(texture->texdata)
VAR(high*large*bpp1)

    if((offset1==0) et (offset2==0))
    {
SREM(memcpy)
        Libmemcpy(RGB2,RGB1,high*large*bpp1);    /*update all*/
    }
    else
    {
SREM(loops)
        sizelarge=large*bpp1;
        YLOOP(high)
        {
        XLOOP(sizelarge)
            *RGB2++=*RGB1++;            /*update the area*/
        RGB1+=offset1;
        RGB2+=offset2;
        }
    }

    if(texture->texdata==NULL)
        {
        SOFT3D_UpdateTexture(WC->SC,WT->ST,texture->texsource);
        WRETURN(W3D_SUCCESS);
        }

/* if got texsource->texdata re-convert the updated part */
    bpp1=texture->bytesperpix;                /*1=original  texture */
    RGB1=(UBYTE *)texture->texsource;
    RGB1=(UBYTE *)&RGB1[(y*Tlarge + x)*bpp1];
    offset1=(Tlarge-large)*bpp1;

    bpp2=BytesPerPix2(format);                /*2=converted texture */
    RGB2=(UBYTE *)texture->texdata;
    RGB2=(UBYTE *)&RGB2[(y*Tlarge + x)*bpp2];
    offset2=(Tlarge-large)*bpp2;
VAR(bpp1)
VAR(bpp2)
VAR(offset1)
VAR(offset2)
VAR(RGB1)
VAR(RGB2)


    ConvertBitmap(format,RGB1,RGB2,high,large,offset1,offset2,(UBYTE *)texture->palette);

    TextureAlphaUsage(WT);            /* Test if tex become transparent with the new datas */

    SOFT3D_UpdateTexture(WC->SC,WT->ST,texture->texdata);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_UploadTexture(W3D_Context *context, W3D_Texture *texture)
{
    WAZP3DFUNCTION(32);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_FreeAllTexObj(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT;
struct WAZP3D_texture *nextWT;
WORD Ntexture=0;

    WAZP3DFUNCTION(33);

    if(WC->firstWT==NULL)
        WRETURN(W3D_SUCCESS);

    WT=WC->firstWT;
    while(WT!=NULL)
     {
VAL(Ntexture)
PrintWT(WT);
    nextWT=WT->nextWT;    /* my own linkage */
    W3D_FreeTexObj(context,&WT->texture);
    WT=nextWT;
    Ntexture++;
    }
    WC->firstWT=NULL;

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetChromaTestBounds(W3D_Context *context, W3D_Texture *texture,ULONG rgba_lower, ULONG rgba_upper, ULONG mode)
{
struct WAZP3D_context *WC=context->driver;
struct WAZP3D_texture *WT=texture->driver;
ULONG *color32;

    WAZP3DFUNCTION(34);
    color32=(ULONG *)WT->ChromaTestMinRGBA; *color32=rgba_lower;
    color32=(ULONG *)WT->ChromaTestMaxRGBA; *color32=rgba_upper;
    WT->ChromaTestMode=mode;

    PrintRGBA((UBYTE *)&WT->ChromaTestMinRGBA);
    PrintRGBA((UBYTE *)&WT->ChromaTestMaxRGBA);
    WINFO(mode,W3D_CHROMATEST_NONE,"No chroma test ");
    WINFO(mode,W3D_CHROMATEST_INCLUSIVE,"texels in the range pass the test ");
    WINFO(mode,W3D_CHROMATEST_EXCLUSIVE,"texels in the range are rejected ");

    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawLine(W3D_Context *context, W3D_Line *line)
{
struct WAZP3D_context *WC=context->driver;
#define GETPLINE(a,b) {GetVertex(WC,a);GetVertex(WC,b);}

    WAZP3DFUNCTION(35);
    if(WC->state.LineSize!=line->linewidth)
    {
    WC->state.Changed=TRUE;
    WC->state.LineSize=line->linewidth;
    if(WC->state.LineSize<1)
        WC->state.LineSize=1;
    }

    if( MAXPRIM < (WC->Pnb+2) )
        DrawPrimitive(context);

    SetTexStates(context,line->tex,W3D_PRIMITIVE_LINES);

    GETPLINE(&line->v1,&line->v2)

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawPoint(W3D_Context *context, W3D_Point *point)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(36);
    if(WC->state.PointSize!=point->pointsize)
    {
    WC->state.Changed=TRUE;
    WC->state.PointSize=point->pointsize;
    if(WC->state.PointSize<1)
        WC->state.PointSize=1;    /* patch: skulpt dont set pointsize*/
    }

    if( MAXPRIM < (WC->Pnb+1) )
        DrawPrimitive(context);

    SetTexStates(context,point->tex,W3D_PRIMITIVE_POINTS);        /* ??? tex not used */

    GetVertex(WC,&point->v1);

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriangle(W3D_Context *context, W3D_Triangle *triangle)
{
struct WAZP3D_context *WC=context->driver;

#define GETPTRI(a,b,c) {GetVertex(WC,a);GetVertex(WC,b);GetVertex(WC,c);}

    WAZP3DFUNCTION(37);

    if( MAXPRIM < (WC->Pnb+3) )
        DrawPrimitive(context);

    SetTexStates(context,triangle->tex,W3D_PRIMITIVE_TRIANGLES);

    GETPTRI(&triangle->v1,&triangle->v2,&triangle->v3)

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriFan(W3D_Context *context, W3D_Triangles *triangles)
{
struct WAZP3D_context *WC=context->driver;
W3D_Vertex *v;
LONG n,Pnb;

    WAZP3DFUNCTION(38);
    v=triangles->v;
    Pnb=triangles->vertexcount;

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)    /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_POLYGON);
    NLOOP(triangles->vertexcount)
        GetVertex(WC,&v[n]);
    DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb-2)*3) )
        DrawPrimitive(context);

    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRIANGLES);

    for (n=2;n<Pnb;n++)
        GETPTRI(&v[0],&v[n-1],&v[n])

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriStrip(W3D_Context *context, W3D_Triangles *triangles)
{
struct WAZP3D_context *WC=context->driver;
W3D_Vertex *v;
LONG n,Pnb;

    WAZP3DFUNCTION(39);
    v=triangles->v;
    Pnb=triangles->vertexcount;

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)                /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRISTRIP); /* soft3d will convert that to POLYGON */
    NLOOP(Pnb)
        GetVertex(WC,&v[n]);
    DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb-2)*3) )
        DrawPrimitive(context);

    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRIANGLES);

    for (n=2;n<Pnb;n++)
//    if (n&1)      /* reverse vertex order */
        GETPTRI(&v[n-1],&v[n-2],&v[n-0])
//    else
//        GETPTRI(&v[n-2],&v[n-1],&v[n-0])

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_Flush(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(40);
    DrawPrimitive(context);        /*v53: just in case it remains buffered primitiveS */
    SOFT3D_Flush(WC->SC);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawLineStrip(W3D_Context *context, W3D_Lines *lines)
{
struct WAZP3D_context *WC=context->driver;
W3D_Vertex *v;
LONG n,Pnb;

    WAZP3DFUNCTION(41);
    v=lines->v;
    Pnb=lines->vertexcount;

    if(WC->state.LineSize!=lines->linewidth)
    {
    WC->state.Changed=TRUE;
    WC->state.LineSize=lines->linewidth;
    if(WC->state.LineSize<1)
        WC->state.LineSize=1;
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb-1)*2) )
        DrawPrimitive(context);

    SetTexStates(context,lines->tex,W3D_PRIMITIVE_LINES);

    NLOOP((Pnb-1))
        GETPLINE(&v[n],&v[n+1])

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawLineLoop(W3D_Context *context, W3D_Lines *lines)
{
struct WAZP3D_context *WC=context->driver;
W3D_Vertex *v;
LONG n,Pnb;

    WAZP3DFUNCTION(42);
    v=lines->v;
    Pnb=lines->vertexcount;

    if(WC->state.LineSize!=lines->linewidth)
    {
    WC->state.Changed=TRUE;
    WC->state.LineSize=lines->linewidth;
    if(WC->state.LineSize<1)
        WC->state.LineSize=1;
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb*2)) )
        DrawPrimitive(context);

    SetTexStates(context,lines->tex,W3D_PRIMITIVE_LINES);


    NLOOP((Pnb-1))
        GETPLINE(&v[n],&v[n+1])
    GETPLINE(&v[Pnb-1],&v[0])


    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ClearDrawRegion(W3D_Context *context, ULONG color)
{
struct WAZP3D_context *WC=context->driver;
ULONG ARGB32=color;
UBYTE *ARGB=(UBYTE *)&ARGB32;
ULONG x,y,large,high;

    WAZP3DFUNCTION(43);

    WC->state.BackRGBA.b[0]=ARGB[1];
    WC->state.BackRGBA.b[1]=ARGB[2];
    WC->state.BackRGBA.b[2]=ARGB[3];
    WC->state.BackRGBA.b[3]=ARGB[0];

    if(context->state & W3D_SCISSOR)
    {
    x    =context->scissor.left;
    y    =context->scissor.top + context->yoffset;
    high    =context->scissor.height;
    large    =context->scissor.width;
    }
    else
    {
    x    =0;
    y    =0 + context->yoffset;
    high    =context->height;
    large    =context->width;
    }

    if(Wazp3D->UseClearDrawRegion.ON)
        FillPixelArray(&WC->rastport,x,y,large,high,color);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetAlphaMode(W3D_Context *context, ULONG mode, W3D_Float *refval)
{
struct WAZP3D_context *WC=context->driver;


    WAZP3DFUNCTION(44);
    WC->AlphaMode=mode;
    WC->AlphaRef=(UBYTE)(*refval*256.0);
    if(1.0<=*refval)    WC->AlphaRef=255;
    if(*refval<=0.0)    WC->AlphaRef=0;

    WINFO(mode,W3D_A_NEVER,"discard incoming pixel ")
    WINFO(mode,W3D_A_LESS,"draw,if A < refvalue ")
    WINFO(mode,W3D_A_GEQUAL,"draw,if A >= refvalue ")
    WINFO(mode,W3D_A_LEQUAL,"draw,if A <= refvalue ")
    WINFO(mode,W3D_A_GREATER,"draw,if A > refvalue ")
    WINFO(mode,W3D_A_NOTEQUAL,"draw,if A != refvalue ")
    WINFO(mode,W3D_A_EQUAL,"draw,if A == refvalue ")
    WINFO(mode,W3D_A_ALWAYS,"always draw ")
    VARF(*refval)

    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetBlendMode(W3D_Context *context, ULONG srcfunc, ULONG dstfunc)
{
struct WAZP3D_context *WC=context->driver;
#define GL_CONSTANT_COLOR               0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR     0x8002
#define GL_CONSTANT_ALPHA               0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA     0x8004

    WAZP3DFUNCTION(45);

/* patch: MiniGL/OS4 send some opengl's src & dst unimplemented values */
    if(srcfunc==GL_CONSTANT_COLOR)        srcfunc=W3D_CONSTANT_COLOR;
    if(srcfunc==GL_ONE_MINUS_CONSTANT_COLOR)    srcfunc=W3D_ONE_MINUS_CONSTANT_COLOR;
    if(srcfunc==GL_CONSTANT_ALPHA )        srcfunc=W3D_CONSTANT_ALPHA;
    if(srcfunc==GL_ONE_MINUS_CONSTANT_ALPHA)    srcfunc=W3D_ONE_MINUS_CONSTANT_ALPHA;

    if(dstfunc==GL_CONSTANT_COLOR)        dstfunc=W3D_CONSTANT_COLOR;
    if(dstfunc==GL_ONE_MINUS_CONSTANT_COLOR)    dstfunc=W3D_ONE_MINUS_CONSTANT_COLOR;
    if(dstfunc==GL_CONSTANT_ALPHA )        dstfunc=W3D_CONSTANT_ALPHA;
    if(dstfunc==GL_ONE_MINUS_CONSTANT_ALPHA)    dstfunc=W3D_ONE_MINUS_CONSTANT_ALPHA;

    if(15<srcfunc) WRETURN(W3D_ILLEGALINPUT);
    if(15<dstfunc) WRETURN(W3D_ILLEGALINPUT);

    WINFO(srcfunc,W3D_ZERO,"source + dest ")
    WINFO(srcfunc,W3D_ONE,"source + dest ")
    WINFO(srcfunc,W3D_SRC_COLOR,"dest only ")
    WINFO(srcfunc,W3D_DST_COLOR,"source only ")
    WINFO(srcfunc,W3D_ONE_MINUS_SRC_COLOR,"dest only ")
    WINFO(srcfunc,W3D_ONE_MINUS_DST_COLOR,"source only ")
    WINFO(srcfunc,W3D_SRC_ALPHA,"source + dest ")
    WINFO(srcfunc,W3D_ONE_MINUS_SRC_ALPHA,"source + dest ")
    WINFO(srcfunc,W3D_DST_ALPHA,"source + dest ")
    WINFO(srcfunc,W3D_ONE_MINUS_DST_ALPHA,"source + dest ")
    WINFO(srcfunc,W3D_SRC_ALPHA_SATURATE,"source only ")
    WINFO(srcfunc,W3D_CONSTANT_COLOR," ");
    WINFO(srcfunc,W3D_ONE_MINUS_CONSTANT_COLOR," ");
    WINFO(srcfunc,W3D_CONSTANT_ALPHA," ");
    WINFO(srcfunc,W3D_ONE_MINUS_CONSTANT_ALPHA," ");

    WINFO(dstfunc,W3D_ZERO,"source + dest ")
    WINFO(dstfunc,W3D_ONE,"source + dest ")
    WINFO(dstfunc,W3D_SRC_COLOR,"dest only ")
    WINFO(dstfunc,W3D_DST_COLOR,"source only ")
    WINFO(dstfunc,W3D_ONE_MINUS_SRC_COLOR,"dest only ")
    WINFO(dstfunc,W3D_ONE_MINUS_DST_COLOR,"source only ")
    WINFO(dstfunc,W3D_SRC_ALPHA,"source + dest ")
    WINFO(dstfunc,W3D_ONE_MINUS_SRC_ALPHA,"source + dest ")
    WINFO(dstfunc,W3D_DST_ALPHA,"source + dest ")
    WINFO(dstfunc,W3D_ONE_MINUS_DST_ALPHA,"source + dest ")
    WINFO(dstfunc,W3D_SRC_ALPHA_SATURATE,"source only ")
    WINFO(dstfunc,W3D_CONSTANT_COLOR," ");
    WINFO(dstfunc,W3D_ONE_MINUS_CONSTANT_COLOR," ");
    WINFO(dstfunc,W3D_CONSTANT_ALPHA," ");
    WINFO(dstfunc,W3D_ONE_MINUS_CONSTANT_ALPHA," ");

/* dest only */
    if(srcfunc==W3D_SRC_COLOR) WRETURN(W3D_ILLEGALINPUT);
    if(srcfunc==W3D_ONE_MINUS_SRC_COLOR) WRETURN(W3D_ILLEGALINPUT);

/* source only */
    if(dstfunc==W3D_DST_COLOR) WRETURN(W3D_ILLEGALINPUT);
    if(dstfunc==W3D_ONE_MINUS_DST_COLOR) WRETURN(W3D_ILLEGALINPUT);
    if(dstfunc==W3D_SRC_ALPHA_SATURATE) WRETURN(W3D_ILLEGALINPUT);

/* patch: BlitzQuake/MiniGL use SetBlendMode but forget to activate with SetState() the blending */
    if(!WC->CallSetBlending)
        SetState(context,W3D_BLENDING,TRUE);

/* backdoor: For "TheVague" DiskMag */
    if(srcfunc==W3D_ZERO)
        if(dstfunc==W3D_ZERO)
        {
            Wazp3D->UseClearImage.ON=FALSE;   /* allways draw on the buffer  without clearing*/
            Wazp3D->TexMode.ON =1;            /* allways do true coloring */
            Wazp3D->PerspMode.ON=0;           /* 2D drawings dont need perspective  */
        }

    if( (WC->SrcFunc!=srcfunc) ou (WC->DstFunc!=dstfunc) )
        WC->state.Changed=TRUE;

    WC->SrcFunc=srcfunc;
    WC->DstFunc=dstfunc;

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void SaveMTL(struct WAZP3D_context *WC,UBYTE *filenameMTL)
{
#ifdef WAZP3DDEBUG
struct WAZP3D_texture *WT;
struct  face3D *F=WC->DumpF;
ULONG Fnb=WC->DumpFnum;
BOOL ThisTextureIsUsed;
BPTR file;
ULONG f;
WORD t;
UBYTE name[256];
#define FWRITE Write(file,name,Libstrlen(name));

    if(WC->firstWT==NULL) return;
    SREM(SaveMTL)
    Libprintf("Saving... <%s>\n",filenameMTL);
    file = Open(filenameMTL,MODE_NEWFILE);
    if(file == 0)
        {Libprintf("Cant open file! <%s>\n",filenameMTL);return;}

    Libsprintf(name,"# Dumped with Wazp3D\n\n"); FWRITE

/* mat for untextured faces */
    Libsprintf(name,"\nnewmtl MatFlat\n"); FWRITE
    Libsprintf(name,"illum 4\n"); FWRITE
    Libsprintf(name,"Ni 1.00\nKd 0.00 0.00 0.00\nKa 0.00 0.00 0.00\nTf 1.00 1.00 1.00\n"); FWRITE

/* mats for textured faces */
    t=0;
    WT=WC->firstWT;
    while(WT!=NULL)
    {
        ThisTextureIsUsed=FALSE;
        FLOOP(Fnb)
            if(F[f].tex==WT)
                ThisTextureIsUsed=TRUE;

        if(ThisTextureIsUsed==TRUE)
        {
        Libsprintf(name,"\nnewmtl MatTex%ld\n",(ULONG)t); FWRITE
        Libsprintf(name,"illum 4\n"); FWRITE
        Libsprintf(name,"map_Kd %s\n",&WT->name[2]); FWRITE
        Libsprintf(name,"Ni 1.00\n"); FWRITE
        Libsprintf(name,"Kd 0.66 0.66 1.00\n"); FWRITE
        Libsprintf(name,"Ka 0.20 0.20 0.20\n"); FWRITE
        Libsprintf(name,"Tf 1.00 1.00 1.00\n"); FWRITE
        }
    t++;
    WT=WT->nextWT;
    }
    Close(file);
LibAlert("Dump .MTL done :-)");
#endif
}
/*==========================================================================*/
void SaveOBJ(struct WAZP3D_context *WC,void *pt)
{
#ifdef WAZP3DDEBUG
UBYTE *filename=pt;
struct WAZP3D_texture *WT;
struct point3D *P=WC->DumpP;
struct  face3D *F=WC->DumpF;
ULONG Pnb=WC->DumpPnum;
ULONG Fnb=WC->DumpFnum;
UBYTE filenameOBJ[256];
UBYTE filenameMTL[256];
BOOL ThisTextureIsUsed;
ULONG f,p,t,n,i;
BOOL ScaleObject=TRUE;
BOOL UseZ=FALSE;
BPTR file;
UBYTE name[256];
float flarge,fhigh;

    SREM(SaveOBJ)
    VAR(WC->DumpP)
    VAR(WC->DumpPnb)
    VAR(WC->DumpPnum)
    VAR(WC->DumpF)
    VAR(WC->DumpFnb)
    VAR(WC->DumpFnum)

    Libstrcpy(filenameOBJ,filename);

    Libstrcpy(filenameMTL,filename);
    n=Libstrlen(filenameMTL);
    filenameMTL[n-4]=0;
    Libstrcat(filenameMTL,".mtl");
    SaveMTL(WC,filenameMTL);

    Libprintf("Saving... <%s>\n",filenameOBJ);

    file = Open(filenameOBJ,MODE_NEWFILE);
    if(file == 0)
        {Libprintf("Cant open file! <%s>\n",filenameOBJ);return;}

    Libsprintf(name,"# Dumped with Wazp3D\n\n"); FWRITE

    if(WC->firstWT!=NULL)
        { Libsprintf(name,"mtllib %s\n",&filenameMTL[2]); FWRITE }

    Libsprintf(name,"\ng default\n"); FWRITE

    if(ScaleObject)        /* then x y are scaled to 0.0 to 1.0*/
    {
    flarge=WC->large;
    fhigh =WC->high;
    }
    else
    {
    flarge=1.0;
    fhigh =1.0;
    }

    PLOOP(Pnb)
        {
        if(P[p].z!=0.0) UseZ=TRUE;
        P[p].x=P[p].x/flarge;
        P[p].y=P[p].y/fhigh ;
        }

    PLOOP(Pnb)
        {
        Libsprintf(name,"v  "); FWRITE
        fpf(file,P[p].x);
        fpf(file,P[p].y);
        if(UseZ)
            fpf(file,P[p].z);
        else
            fpf(file,1.0/(P[p].w+0.001));
        Libsprintf(name,"\n");  FWRITE
        }

    PLOOP(Pnb)
        {
        Libsprintf(name,"vt  ");  FWRITE
        fpf(file,P[p].u); fpf(file,P[p].v);
        Libsprintf(name,"\n");  FWRITE
        }



/* 1: untextured faces */
    FLOOP(Fnb)
        if(F[f].tex==NULL)
        {
        Libsprintf(name,"\ng groupflat\n"); FWRITE
        Libsprintf(name,"usemtl MatFlat\n"); FWRITE
        break;
        }

    FLOOP(Fnb)
        if(F[f].tex==NULL)
        {
        Libsprintf(name,"f"); FWRITE
        PLOOP(F[f].Pnb)
            {
            i=F[f].Pnum+p+1;                    /* The numbering start with 1 */
            Libsprintf(name," %ld/%ld",i,i);FWRITE     /* use same indice for Vi UVi */
            }
        Libsprintf(name,"\n"); FWRITE
        }

/* 2: textured faces */
    t=0;
    WT=WC->firstWT;
    while(WT!=NULL)
    {
        ThisTextureIsUsed=FALSE;
        FLOOP(Fnb)
            if(F[f].tex==WT)
                ThisTextureIsUsed=TRUE;

        if(ThisTextureIsUsed==TRUE)
        {
        Libsprintf(name,"\ng group%ld\n",t); FWRITE
        Libsprintf(name,"usemtl MatTex%ld\n",t); FWRITE
        FLOOP(Fnb)
            if(F[f].tex==WT)
            {
            Libsprintf(name,"f"); FWRITE
            PLOOP(F[f].Pnb)
                {
                i=F[f].Pnum+p+1;                    /* The numbering start with 1 */
                Libsprintf(name," %ld/%ld",i,i); FWRITE    /* use same indice for Vi UVi */
                }
            Libsprintf(name,"\n"); FWRITE
            }
        }
    t++;
    WT=WT->nextWT;
    }
    Close(file);
LibAlert("Dump .OBJ done :-)");

    WT=WC->firstWT;
    if(Wazp3D->DumpTextures.ON)
    while(WT!=NULL)
    {
        Libsavefile(WT->name,WT->pt,WT->large*WT->high*WT->bits/8);
        WT=WT->nextWT;
    }
    Wazp3D->DumpTextures.ON=FALSE;

LibAlert("Dump .RAW textures done :-)");

#endif
}
/*================================================================*/
void DumpPoly(struct WAZP3D_context *WC)
{
#ifdef WAZP3DDEBUG
    struct face3D *F;


    if(WC->DumpStage==1)        /* 1: count faces & points */
    {
        WC->DumpFnb+=1;
        WC->DumpPnb+=WC->PolyPnb;
    return;
    }

    if(WC->DumpStage==2)        /* 2: dump faces & points */
    {
        if(WC->DumpFnum+1 >= WC->DumpFnb)
            {WC->DumpStage=3; Libprintf("Dump Fail too much faces\n"); return;}

        if(WC->DumpPnum+WC->PolyPnb >= WC->DumpPnb)
            {WC->DumpStage=3; Libprintf("Dump Fail too much points\n"); return;}

        F=&WC->DumpF[WC->DumpFnum];
        F->Pnum=WC->DumpPnum;
        F->Pnb =WC->PolyPnb;
        F->tex =WC->WT;
        Libmemcpy(&(WC->DumpP[WC->DumpPnum]),&(WC->PolyP[0]),PSIZE*WC->PolyPnb);
        WC->DumpFnum+=1;
        WC->DumpPnum+=WC->PolyPnb;
    }
#endif
}
/*=============================================================*/
#ifdef WAZP3DDEBUG
void DumpTriP(struct WAZP3D_context *WC,register struct point3D *A,register struct point3D *B,register struct point3D *C)
{
    SREM(DumpTriP)

    COPYP(&(WC->PolyP[0]),A);
    COPYP(&(WC->PolyP[1]),B);
    COPYP(&(WC->PolyP[2]),C);
    WC->PolyPnb=3;
    DumpPoly(WC);
}
#endif
/*================================================================*/
void DumpPrimitive(struct WAZP3D_context *WC,struct point3D *P,ULONG Pnb,ULONG primitive)
{
#ifdef WAZP3DDEBUG
UWORD m,n,nb;

SREM(DumpPrimitive)
VAR(primitive)
VAR(Pnb)

    if(primitive==W3D_PRIMITIVE_TRIANGLES)
    {
        nb=Pnb/3;
        VAR(nb)
        NLOOP(nb)
            DumpTriP(WC,&P[3*n+0],&P[3*n+1],&P[3*n+2]);
    }

    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)    /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
    if(primitive==W3D_PRIMITIVE_TRIFAN)
        primitive=W3D_PRIMITIVE_POLYGON;

    if(Wazp3D->PolyMode.ON!=2)
    if(primitive==W3D_PRIMITIVE_TRISTRIP)
        {
        m=n=0;
        while(n<Pnb)
            {COPYP(&(WC->PolyP[m++]),&P[n]);n=n+2;}
        if(Pnb AND 1) n=Pnb-2; else n=Pnb-1;
        while(0<n)
            {COPYP(&(WC->PolyP[m++]),&P[n]);n=n-2;}

        WC->PolyPnb=Pnb;
        DumpPoly(WC);
        return;
        }
    }

    if(primitive==W3D_PRIMITIVE_TRIFAN)
    {
    for (n=2;n<Pnb;n++)
        DumpTriP(WC,&P[0],&P[n-1],&P[n]);
    return;
    }

    if(primitive==W3D_PRIMITIVE_TRISTRIP)
    {
    for (n=2;n<Pnb;n++)
//    if (n&1)      /* reverse vertex order */
        DumpTriP(WC,&P[n-1],&P[n-2],&P[n-0]);
//    else
//        DumpTriP(WC,&P[n-2],&P[n-1],&P[n-0]);
//    return;
    }

    if(primitive==W3D_PRIMITIVE_POINTS)
    NLOOP(Pnb)
    {
        DumpTriP(WC,&P[n],&P[n+1],&P[n]);
    }


    if(primitive==W3D_PRIMITIVE_LINES)
    {
    nb=Pnb/2;
    NLOOP(nb)
    {
        DumpTriP(WC,&P[2*n],&P[2*n+1],&P[2*n+1]);
    }
    return;
    }

    if(primitive==W3D_PRIMITIVE_LINELOOP)
    {
        nb=Pnb-1;
        NLOOP(nb)
        {
            DumpTriP(WC,&P[n],&P[n+1],&P[n+1]);
        }

        DumpTriP(WC,&P[nb],&P[0],&P[0]);
        return;
    }

    if(primitive==W3D_PRIMITIVE_LINESTRIP)
    {
        nb=Pnb-1;
        NLOOP(nb)
        {
            DumpTriP(WC,&P[n],&P[n+1],&P[n+1]);
        }
        return;
    }

    if(primitive==W3D_PRIMITIVE_POLYGON    )
    {
        NLOOP(Pnb)
        {
            COPYP(&(WC->PolyP[n]),&P[n]);
        }
        WC->PolyPnb=Pnb;
        DumpPoly(WC);
    }
#endif
}
/*================================================================*/
void DumpObject(struct WAZP3D_context *WC)
{
#ifdef WAZP3DDEBUG

    if(WC->DumpStage==0)
    {
        if(Wazp3D->DebugVar.ON) Libprintf("DumpObject(0/3): reset\n");
        WC->DumpFnum=WC->DumpPnum=WC->DumpFnb=WC->DumpPnb=0;
        WC->DumpF=NULL;
        WC->DumpP=NULL;
        WC->DumpStage=1;
        return;
    }

    if(WC->DumpStage==1)
    {
        if(Wazp3D->DebugVar.ON) Libprintf("DumpObject(1/3): count&alloc Fnb:%ld Pnb:%ld\n",WC->DumpFnb,WC->DumpPnb);
        if(WC->DumpFnb==0)
            {WC->DumpStage=0;return;}                /* wait a frame that drawn something */
        WC->DumpPnb=WC->DumpPnb+WC->DumpPnb/2;            /* alloc 150% the previous size */
        WC->DumpFnb=WC->DumpFnb+WC->DumpFnb/2;
        WC->DumpP=MMmalloc(WC->DumpPnb*PSIZE,"DumpP");
        WC->DumpF=MMmalloc(WC->DumpFnb*sizeof(struct  face3D),"DumpF");

        if(WC->DumpP!=NULL)
        if(WC->DumpF!=NULL)    /* buffers ok ? then continue */
            {
            if(Wazp3D->DebugVar.ON) Libprintf("DumpObject: allocated\n");
            WC->DumpStage=2; return;
            }
        if(Wazp3D->DebugVar.ON) Libprintf("DumpObject: malloc fail\n");
        WC->DumpStage=3;                /* else free all */
    }

    if(WC->DumpStage==2)
    {
        if(Wazp3D->DebugVar.ON) Libprintf("DumpObject(2/3): dump&save Fnb:%ld\n",WC->DumpFnb);
        if(WC->DumpFnb==0)         /* wait a frame that drawn something */
        {
            WC->DumpStage=2;return;
        }
        SaveOBJ(WC,"T:DumpWazp3D.obj");
        WC->DumpStage=3; return;
    }

    if(WC->DumpStage==3)
    {
        if(Wazp3D->DebugVar.ON) Libprintf("DumpObject(3/3): free all\n");
        FREEPTR(WC->DumpP);
        FREEPTR(WC->DumpF);
        Wazp3D->DumpObject.ON=0;
        WC->DumpStage=0; return;
    }
#endif
}
/*==========================================================================*/
void ZbufferCheck(W3D_Context *context)
{
//    struct WAZP3D_context *WC=context->driver;

    SREM(ZbufferCheck)
    if(context->zbufferlost) /* = need a new zbuffer */
    {
        SREM(ZbufferCheck: Reallocating a new Zbuffer)
        W3D_FreeZBuffer(context);
        W3D_AllocZBuffer(context);
        VAR(context->zbuffer)
    }
}
/*==========================================================================*/
void  DrawPrimitive(W3D_Context* context)
{
    struct WAZP3D_context *WC=context->driver;

    if(WC->Pnb==0)
    {
        REM(   Primitive got 0 points); return;
    }

    REM(DrawPrimitive)
    ZbufferCheck(context);

    if(WC->DebuggedWT!=NULL)
    {
        if(WC->DebuggedWT==WC->WT)
        {
            YYY
        }
        else
        {
            NNN
        }
    }

    if(WC->DumpStage!=0)
        DumpPrimitive(WC,WC->P,WC->Pnb,WC->state.primitive);

    WC->PrimitivesDrawn++;
    SOFT3D_DrawPrimitive(WC->SC,WC->P,WC->Pnb,WC->state.primitive);

    if(!StateON(W3D_INDIRECT))        /*v50: If direct mode do a flush after each poly (else bufferize fragments)*/
        SOFT3D_Flush(WC->SC);

#ifdef WAZP3DDEBUG
    if(Wazp3D->StepDrawPoly.ON)
    {
        SOFT3D_Flush(WC->SC);            /* finish this poly with a flush */
        LibAlert("DrawPolyP() done !!");
    }
#endif
    WC->Pnb=0;
}
/*==========================================================================*/
void SetDrawRegion(W3D_Context *context, struct BitMap *bm,int yoffset, W3D_Scissor *scissor)
{
struct WAZP3D_context *WC=context->driver;
W3D_Bitmap  *w3dbm;
void  *bmHandle;
BOOL SameSize;

SREM(SetDrawRegion)
/* change bitmap */
    if(bm==NULL) return;
SREM(setting the bitmap)
    if(context->w3dbitmap)
    {
        w3dbm=(W3D_Bitmap  *)bm;
        context->width  = w3dbm->width;                /* bitmap width  */
        context->height = w3dbm->height;                /* bitmap height */
        context->depth  = w3dbm->bprow/w3dbm->width;        /* bitmap depth  */
        context->bprow  = w3dbm->bprow;                /* bytes per row */
        context->format = w3dbm->format;                /* bitmap format (see below) */
        context->drawmem= w3dbm->dest;                /* base address for drawing operations */
    }
    else
    {
        context->width  = GetBitMapAttr( bm, BMA_WIDTH  );    /* bitmap width  */
        context->height = GetBitMapAttr( bm, BMA_HEIGHT );    /* bitmap height */
        context->depth  = GetBitMapAttr( bm, BMA_DEPTH  );    /* bitmap depth  */
        context->bprow  = bm->BytesPerRow;
        if(context->depth==24)     context->format=W3D_FMT_R8G8B8;    /* is it allways correct ??? */



        if(context->depth==32)     context->format=W3D_FMT_A8R8G8B8;
        context->format = 0;
        context->drawmem=NULL;
    }

    if(Wazp3D->UseDLL)
    if(Wazp3D->Renderer.ON==0)    /* soft to Image */
    {
        Libprintf("WAZP3D: SOFT3D.DLL cant use Soft to bitmap!!!\n");
        Wazp3D->Renderer.ON=1;    /* soft to bitmap : as SOFT3D.DLL cant do a WritePixelArray(Image)*/
    }

    bmHandle=LockBitMapTags((APTR)bm,LBMI_BASEADDRESS,(ULONG)&WC->bmdata, TAG_DONE);
    WC->bmformat = GetCyberMapAttr(bm,CYBRMATTR_PIXFMT);
    if(bmHandle!=NULL)
        UnLockBitMap(bmHandle);
    if(Wazp3D->Renderer.ON==1)    /* soft to bitmap */
    if(WC->bmdata==NULL)
    {
        Libprintf("WAZP3D: This Aros driver cant use Soft to bitmap!!!\n");
        Wazp3D->Renderer.ON=0;    /* soft to Image */
    }

    WC->bits=context->bprow/context->width*8;
    context->drawregion=bm;
    WC->rastport.BitMap=bm;

/* change yoffset */
    if(StateON(W3D_DOUBLEHEIGHT))
        context->height=context->height/2;    /* if doubleheight use only the half height for the Wazp3D RGBA buffer*/
    else
        yoffset=0;                    /* if cant do double-height (ie AROS) then let this to 0 */

VAR(StateON(W3D_DOUBLEHEIGHT))
VAR(yoffset)
    WC->yoffset=context->yoffset=yoffset;

/* store new (?) size */

    VAR(context->width)
    VAR(context->height)
    VAR(WC->large)
    VAR(WC->high)
    SameSize=((WC->large==context->width) et (WC->high==context->height));
VAR(SameSize)
    WC->large=context->width;
    WC->high =context->height;
    WC->windowX=WC->window->LeftEdge;
    WC->windowY=WC->window->TopEdge;

    VAR(WC->large)
    VAR(WC->high)

/* change scissor */
    if(scissor==NULL)
    {
SREM(no scissor yet )
    context->scissor.left   =0;
    context->scissor.top    =0;
    context->scissor.width  =context->width;
    context->scissor.height =context->height;
    }
    else
    {
SREM(with scissor )
        Libmemcpy(&context->scissor,scissor,sizeof(W3D_Scissor));
    }

    VAR(context->scissor.left)
    VAR(context->scissor.top)
    VAR(context->scissor.width)
    VAR(context->scissor.height)
    WC->Xmin=context->scissor.left;
    WC->Ymin=context->scissor.top;
    WC->Xmax=WC->Xmin+context->scissor.width-1;
    WC->Ymax=WC->Ymin+context->scissor.height-1;
    if(WC->Xmin<0)        WC->Xmin=0;
    if(WC->Ymin<0)        WC->Ymin=0;
    if(WC->large< WC->Xmax)    WC->Xmax=WC->large;
    if(WC->high < WC->Ymax)    WC->Ymax=WC->high;

/* if SOFT3D not yet started then start it*/
    if(WC->SC==NULL)
        WC->SC=SOFT3D_Start(Wazp3D,sizeof(parameters));

/* if bitmap's size has changed then change ImageBuffer (if any) & query for a  new Zbuffer */
    if(!SameSize)
    {
        SREM(Bitmap size changed !!!!)
        if(Wazp3D->Renderer.ON==0)        /* use Soft to Image */
            SOFT3D_AllocImageBuffer(WC->SC,WC->large,WC->high);        /* If use an ImageBuffer32 (Wazp3D's RGBA buffer) then will realloc it */
        if(context->zbufferalloc)                        /* if use a Zbuffer */
        {
            SREM(Will need a new Zbuffer)
            context->zbufferlost=TRUE;
        }
    }

/* SOFT3D: change bitmap  & scissor */
    WC->SOFT3D_SetBitmap_SetClippingHook(WC);
}
/*==========================================================================*/
ULONG W3D_SetDrawRegion(W3D_Context *context, struct BitMap *bm,int yoffset, W3D_Scissor *scissor)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(46);
    VAR(bm)
    VAR(yoffset)
    VAR(scissor->left)
    VAR(scissor->top)
    VAR(scissor->width)
    VAR(scissor->height)

    WC->CallSetDrawRegion=TRUE;
/* draw the image if needed */
    if(!WC->CallFlushFrame)        /* W3D_FlushFrame is the better time to do the Update */
        DoUpdate(context);

/* do nothing if the prog always call this function with same parameters */
    if(context->drawregion      ==bm                 )
    if(context->yoffset        ==yoffset             )
    if(context->scissor.left   ==scissor->left       )
    if(context->scissor.top    ==scissor->top        )
    if(context->scissor.width  ==scissor->width      )
    if(context->scissor.height ==scissor->height     )
    if(WC->windowX             ==WC->window->LeftEdge)
    if(WC->windowY             ==WC->window->TopEdge )
        WRETURN(W3D_SUCCESS);        /* nothing to do ===> return */

    SetDrawRegion(context,bm,yoffset,scissor);

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetDrawRegionWBM(W3D_Context *context, W3D_Bitmap *bm,W3D_Scissor *scissor)
{
ULONG error;

    WAZP3DFUNCTION(47);
    context->w3dbitmap=TRUE;
    error=W3D_SetDrawRegion(context,(struct BitMap *)bm,context->yoffset,scissor);
    WRETURN(error);
}
/*==========================================================================*/
ULONG W3D_SetFogParams(W3D_Context *context, W3D_Fog *fogparams,ULONG fogmode)
{
/*
The fields fog_start and fog_end must be given if linear fog is used.
These values are in 'w-space', meaning 1.0 is the front plane, and 0.0 is the back plane.
The density field is only used in exponential fog modes
*/
struct WAZP3D_context *WC=context->driver;


    WAZP3DFUNCTION(48);
    WC->state.FogMode=0;
    if(fogmode==W3D_FOG_LINEAR) WC->state.FogMode=1;
    if(fogmode==W3D_FOG_EXP)    WC->state.FogMode=2;
    if(fogmode==W3D_FOG_EXP_2)  WC->state.FogMode=3;

    ColorToRGBA(WC->state.FogRGBA.b,fogparams->fog_color.r,fogparams->fog_color.g,fogparams->fog_color.b,1.0);

    if(fogparams->fog_start==0.0)
        fogparams->fog_start=0.0001;
    if(fogparams->fog_end==0.0)
        fogparams->fog_end=0.0001;

/* The fields fog_start and fog_end must be given if linear fog is used. These values are in 'w-space', meaning 1.0 is the front plane, and 0.0 is the back plane */
    WC->state.FogZmin        =1.0 - fogparams->fog_start;
    WC->state.FogZmax        =1.0 - fogparams->fog_end;

    if(WC->state.FogZmin<MINZ)
        WC->state.FogZmin=MINZ;
    if(MAXZ<WC->state.FogZmax)
        WC->state.FogZmax=MAXZ;

    if(fogmode==W3D_FOG_INTERPOLATED)
        fogmode=W3D_FOG_EXP;

    WC->state.FogDensity    =fogparams->fog_density;
    Libmemcpy(&context->fog,fogparams,sizeof(W3D_Fog));

    WINFO(fogmode,W3D_FOG_LINEAR,"linear fogging ")
    WINFO(fogmode,W3D_FOG_EXP,"exponential fogging ")
    WINFO(fogmode,W3D_FOG_EXP_2,"square exponential fogging ")
    WINFO(fogmode,W3D_FOG_INTERPOLATED,"interpolated fogging ")
    PrintRGBA((UBYTE *)&WC->state.FogRGBA.b);
    VARF(fogparams->fog_start)
    VARF(fogparams->fog_end)
    VARF(fogparams->fog_density)
    VARF(WC->state.FogZmin)
    VARF(WC->state.FogZmax)

    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetLogicOp(W3D_Context *context, ULONG operation)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(49);
    WC->LogicOp=operation;
    WINFO(operation,W3D_LO_CLEAR,"dest = 0 ");
    WINFO(operation,W3D_LO_AND,"dest = source AND dest ");
    WINFO(operation,W3D_LO_AND_REVERSE,"dest = source AND NOT dest ");
    WINFO(operation,W3D_LO_COPY,"dest = source ");
    WINFO(operation,W3D_LO_AND_INVERTED,"dest = NOT source AND dest ");
    WINFO(operation,W3D_LO_NOOP,"dest = dest ");
    WINFO(operation,W3D_LO_XOR,"dest = source XOR dest ");
    WINFO(operation,W3D_LO_OR,"dest = source OR dest ");
    WINFO(operation,W3D_LO_NOR,"dest = NOT (source OR dest) ");
    WINFO(operation,W3D_LO_EQUIV,"dest = NOT (source XOR dest) ");
    WINFO(operation,W3D_LO_INVERT,"dest = NOT  dest ");
    WINFO(operation,W3D_LO_OR_REVERSE,"dest = source OR NOT dest ");
    WINFO(operation,W3D_LO_COPY_INVERTED,"dest = NOT source ");
    WINFO(operation,W3D_LO_OR_INVERTED,"dest = NOT source OR dest ");
    WINFO(operation,W3D_LO_NAND,"dest = NOT (source AND dest) ");
    WINFO(operation,W3D_LO_SET,"dest = 1 ");
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetColorMask(W3D_Context *context, W3D_Bool Red, W3D_Bool Green,W3D_Bool Blue, W3D_Bool Alpha)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(50);
    VAR(Red)
    VAR(Green)
    VAR(Blue)
    VAR(Alpha)
    if(Red)    WC->MaskRGBA.b[0]=255; else WC->MaskRGBA.b[0]=0;
    if(Green)  WC->MaskRGBA.b[1]=255; else WC->MaskRGBA.b[1]=0;
    if(Blue)   WC->MaskRGBA.b[2]=255; else WC->MaskRGBA.b[2]=0;
    if(Alpha)  WC->MaskRGBA.b[3]=255; else WC->MaskRGBA.b[3]=0;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetPenMask(W3D_Context *context, ULONG pen)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(51);
    VAR(pen)
    WC->PenMask=pen;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetCurrentColor(W3D_Context *context, W3D_Color *color)
{
    struct WAZP3D_context *WC=context->driver;
    ULONG RGBA[1];

    WAZP3DFUNCTION(52);
    if(color==NULL)
        WRETURN(W3D_ILLEGALINPUT);
    ColorToRGBA((UBYTE*)RGBA,color->r,color->g,color->b,color->a);

    if(NOTSAMERGBA(WC->state.CurrentRGBA.L,RGBA))
        WC->state.Changed=TRUE;
    COPYRGBA(WC->state.CurrentRGBA.L,RGBA);
    PrintRGBA((UBYTE*)RGBA);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetCurrentPen(W3D_Context *context, ULONG pen)
{
struct WAZP3D_context *WC=context->driver;
ULONG RGBA[1];

    WAZP3DFUNCTION(53);
/* horrible hack to recover the pen as RGB TODO: find a better method*/
    SetAPen (&WC->rastport,pen);
    RectFill(&WC->rastport,0,0,1,1);
    RGBA[0]=ReadRGBPixel(&WC->rastport,0,0);

    if(NOTSAMERGBA(WC->state.CurrentRGBA.L,RGBA))
        WC->state.Changed=TRUE;
    COPYRGBA(WC->state.CurrentRGBA.L,RGBA);
    PrintRGBA((UBYTE*)RGBA);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void W3D_SetScissor(W3D_Context *context, W3D_Scissor *scissor)
{

    WAZP3DFUNCTION(54);
    if(context->scissor.left    ==scissor->left   )
    if(context->scissor.top     ==scissor->top    )
    if(context->scissor.width   ==scissor->width  )
    if(context->scissor.height  ==scissor->height )
        return;            /* nothing to do ===> return */

    SetDrawRegion(context,context->drawregion,context->yoffset,scissor);
}
/*==========================================================================*/
void W3D_FlushFrame(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(55);
    WC->CallFlushFrame=TRUE;
    DoUpdate(context);
}
/*==========================================================================*/
ULONG W3D_AllocZBuffer(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(56);
    if(context->zbufferalloc==TRUE)
        WRETURN(W3D_SUCCESS);    /* already allocated ? */

    context->zbuffer=SOFT3D_AllocZbuffer(WC->SC,WC->large,WC->high);
    context->zbufferalloc=TRUE;
    context->zbufferlost =FALSE;         /* Is it TRUE if just allocated ?!? */

    if(Wazp3D->Renderer.ON<2)    /* use soft */
    if(context->zbuffer==NULL)
        context->zbufferalloc=FALSE;

    if(!context->zbufferalloc)
         WRETURN(W3D_NOGFXMEM);

    SOFT3D_ClearZBuffer(WC->SC,1.0);

    WC->state.Changed=TRUE;        /* now we have a zbuffer */
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_FreeZBuffer(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(57);
    if(!context->zbufferalloc) WRETURN(W3D_NOZBUFFER);
    context->zbufferalloc=FALSE;
    context->zbufferlost =TRUE;     /* Is it TRUE if just freed ?!? */
    SOFT3D_AllocZbuffer(WC->SC,0,0);    /* will free() zbuffer */
    context->zbuffer=NULL;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ClearZBuffer(W3D_Context *context, W3D_Double *clearvalue)
{
struct WAZP3D_context *WC=context->driver;
float z=1.0;

    WAZP3DFUNCTION(58);
    ZbufferCheck(context);
    WC->CallClearZBuffer=TRUE;
    if(clearvalue!=NULL)
        z=*clearvalue;
    VARF(z);

    if(!WC->CallFlushFrame)
    if(!WC->CallSetDrawRegion)
        DoUpdate(context);            /*draw the image if any */

    if(!context->zbufferalloc)
        WRETURN(W3D_NOZBUFFER);
    SOFT3D_ClearZBuffer(WC->SC,z);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ReadZPixel(W3D_Context *context, ULONG x, ULONG y,W3D_Double *z)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(59);
    if(!context->zbufferalloc) WRETURN(W3D_NOZBUFFER);
    VAR(x);
    VAR(y);
    VARF((float)*z);

    SOFT3D_ReadZSpan(WC->SC,x,y,1,z);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ReadZSpan(W3D_Context *context, ULONG x, ULONG y,ULONG n, W3D_Double *z)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(60);
    if(!context->zbufferalloc) WRETURN(W3D_NOZBUFFER);
    VAR(x);
    VAR(y);
    VAR(n);
    VAR(z);

    SOFT3D_ReadZSpan(WC->SC,x,y,n,z);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetZCompareMode(W3D_Context *context, ULONG mode)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(61);
    VAR(mode)
    if(mode<W3D_Z_NEVER)
        WRETURN(W3D_ILLEGALINPUT);
    if(W3D_Z_ALWAYS<mode)
        WRETURN(W3D_ILLEGALINPUT);

    if(mode!=W3D_Z_ALWAYS)    /* if truly need a z testing ? then need the zbuffer*/
    if(mode!=W3D_Z_NEVER)
        SetState(context,W3D_ZBUFFER,W3D_ENABLE);    /*patch: for MiniGL/OS4 that forgot to enable zbuffer*/

    WINFO(mode,W3D_Z_NEVER,"discard incoming pixel ")
    WINFO(mode,W3D_Z_LESS,"draw,if Z < Zbuffer ")
    WINFO(mode,W3D_Z_GEQUAL,"draw,if Z >= Zbuffer ")
    WINFO(mode,W3D_Z_LEQUAL,"draw,if Z <= Zbuffer ")
    WINFO(mode,W3D_Z_GREATER,"draw,if Z > Zbuffer ")
    WINFO(mode,W3D_Z_NOTEQUAL,"draw,if Z != Zbuffer ")
    WINFO(mode,W3D_Z_EQUAL,"draw,if Z == Zbuffer ")
    WINFO(mode,W3D_Z_ALWAYS,"always draw ")

    if(WC->ZCompareMode!=mode)
        WC->state.Changed=TRUE;
    WC->ZCompareMode=mode;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void W3D_WriteZPixel(W3D_Context *context, ULONG x, ULONG y,W3D_Double *z)
{
struct WAZP3D_context *WC=context->driver;
UBYTE mask=1;

    WAZP3DFUNCTION(62);
    if(!context->zbufferalloc) return;
    VAR(x);
    VAR(y);
    VARF((float)*z);

    SOFT3D_WriteZSpan(WC->SC,x,y,1,z,&mask);
/*    WRETURN(W3D_SUCCESS);*/
}
/*==========================================================================*/
void W3D_WriteZSpan(W3D_Context *context, ULONG x, ULONG y,ULONG n, W3D_Double *z, UBYTE *mask)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(63);
    ZbufferCheck(context);
    if(!context->zbufferalloc) return;

    VAR(x);
    VAR(y);
    VAR(n);
    VAR(mask)

    SOFT3D_WriteZSpan(WC->SC,x,y,n,z,mask);
}
/*==========================================================================*/
ULONG W3D_AllocStencilBuffer(W3D_Context *context)
{
struct WAZP3D_context *WC=context->driver;
ULONG clearvalue[1];

    WAZP3DFUNCTION(64);
    if(context->stbufferalloc==TRUE)     WRETURN(W3D_SUCCESS);

    context->stencilbuffer=MMmalloc(WC->large*WC->high*8/8,"Stencil8");
    if(context->stencilbuffer==NULL) WRETURN(W3D_NOGFXMEM);
    context->stbufferalloc=TRUE;
    clearvalue[0]=0;
    W3D_ClearStencilBuffer(context,(ULONG *)clearvalue);

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ClearStencilBuffer(W3D_Context *context, ULONG *clearvalue)
{
struct WAZP3D_context *WC=context->driver;
UBYTE s8;

    WAZP3DFUNCTION(65);
    VAR(*clearvalue)
    if(context->stencilbuffer==NULL) WRETURN(W3D_NOSTENCILBUFFER);
    s8=*clearvalue;  /* todo: enhance this conversion */
    memset(context->stencilbuffer,s8,WC->large*WC->high*8/8);

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_FillStencilBuffer(W3D_Context *context, ULONG x, ULONG y,ULONG width, ULONG height, ULONG depth,void *data)
{
struct WAZP3D_context *WC=context->driver;
register UBYTE *Stencil8=context->stencilbuffer;
register ULONG offset;
UBYTE *data8 =data;
UWORD *data16=data;
ULONG *data32=data;

    WAZP3DFUNCTION(66);
    VAR(x);
    VAR(y);
    VAR(width);
    VAR(height);
    VAR(depth);
    VAR(data);
    if(context->stencilbuffer==NULL) WRETURN(W3D_NOSTENCILBUFFER);
    offset=x+y*WC->large;
    Stencil8+=offset;
    offset=WC->large-width;
    if(depth==8)
        YLOOP(height)
            {
            XLOOP(width)
                *Stencil8++=*data8++;
            Stencil8+=offset;
            }
    if(depth==16)
        YLOOP(height)
            {
            XLOOP(width)
                *Stencil8++=*data16++;
            Stencil8+=offset;
            }
    if(depth==32)
        YLOOP(height)
            {
            XLOOP(width)
                *Stencil8++=*data32++;
            Stencil8+=offset;
            }
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_FreeStencilBuffer(W3D_Context *context)
{

    WAZP3DFUNCTION(67);
    if(context->stencilbuffer==NULL) WRETURN(W3D_NOSTENCILBUFFER);
    FREEPTR(context->stencilbuffer);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ReadStencilPixel(W3D_Context *context, ULONG x, ULONG y,ULONG *st)
{
    WAZP3DFUNCTION(68);
    VAR(x);
    VAR(y);
    VAR(st);
    return(W3D_ReadStencilSpan(context,x,y,1,st));
}
/*==========================================================================*/
ULONG W3D_ReadStencilSpan(W3D_Context *context, ULONG x, ULONG y,ULONG n, ULONG *st)
{
struct WAZP3D_context *WC=context->driver;
register UBYTE *Stencil8=context->stencilbuffer;
register ULONG offset;

    WAZP3DFUNCTION(69);
    VAR(x);
    VAR(y);
    VAR(n);
    VAR(st);
    if(context->stencilbuffer==NULL) WRETURN(W3D_NOSTENCILBUFFER);
    offset=x+y*WC->large;
    Stencil8+=offset;
    XLOOP(n)
        st[x]=Stencil8[x];
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetStencilFunc(W3D_Context *context, ULONG func, ULONG refvalue,ULONG mask)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(70);
    WC->StencilFunc=func;
    WC->StencilRef=refvalue;
    WC->StencilMask=mask;

    WINFO(func,W3D_ST_NEVER,"don't draw pixel ")
    WINFO(func,W3D_ST_ALWAYS,"draw always ")
    WINFO(func,W3D_ST_LESS,"draw,if refvalue < ST ")
    WINFO(func,W3D_ST_LEQUAL,"draw,if refvalue <= ST ")
    WINFO(func,W3D_ST_EQUAL,"draw,if refvalue == ST ")
    WINFO(func,W3D_ST_GEQUAL,"draw,if refvalue >= ST ")
    WINFO(func,W3D_ST_GREATER,"draw,if refvalue > ST ")
    WINFO(func,W3D_ST_NOTEQUAL,"draw,if refvalue != ST ")
    VAR(refvalue)
    VAR(mask)
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetStencilOp(W3D_Context *context, ULONG sfail, ULONG dpfail,ULONG dppass)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(71);
    WC->StencilSfail=sfail;
    WC->StencilZfail=dpfail;
    WC->StencilZpass=dppass;

    WINFO(sfail,W3D_ST_KEEP,"keep stencil buffer value ")
    WINFO(sfail,W3D_ST_ZERO,"clear stencil buffer value ")
    WINFO(sfail,W3D_ST_REPLACE,"replace by reference value ")
    WINFO(sfail,W3D_ST_INCR,"increment ")
    WINFO(sfail,W3D_ST_DECR,"decrement ")
    WINFO(sfail,W3D_ST_INVERT,"invert bitwise ")

    WINFO(dpfail,W3D_ST_KEEP,"keep stencil buffer value ")
    WINFO(dpfail,W3D_ST_ZERO,"clear stencil buffer value ")
    WINFO(dpfail,W3D_ST_REPLACE,"replace by reference value ")
    WINFO(dpfail,W3D_ST_INCR,"increment ")
    WINFO(dpfail,W3D_ST_DECR,"decrement ")
    WINFO(dpfail,W3D_ST_INVERT,"invert bitwise ")

    WINFO(dppass,W3D_ST_KEEP,"keep stencil buffer value ")
    WINFO(dppass,W3D_ST_ZERO,"clear stencil buffer value ")
    WINFO(dppass,W3D_ST_REPLACE,"replace by reference value ")
    WINFO(dppass,W3D_ST_INCR,"increment ")
    WINFO(dppass,W3D_ST_DECR,"decrement ")
    WINFO(dppass,W3D_ST_INVERT,"invert bitwise ")

#ifdef WARP3DV5
    WINFO(sfail,W3D_ST_INCR_WRAP,"increment with wrap")
    WINFO(sfail,W3D_ST_DECR_WRAP,"decrement with wrap")

    WINFO(dpfail,W3D_ST_INCR_WRAP,"increment with wrap")
    WINFO(dpfail,W3D_ST_DECR_WRAP,"decrement with wrap")

    WINFO(dppass,W3D_ST_INCR_WRAP,"increment with wrap")
    WINFO(dppass,W3D_ST_DECR_WRAP,"decrement with wrap")
#endif

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetWriteMask(W3D_Context *context, ULONG mask)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(72);
    VAR(mask)
    WC->StencilWriteMask=mask;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_WriteStencilPixel(W3D_Context *context, ULONG x, ULONG y, ULONG st)
{
UBYTE mask=1;
ULONG stencilvalue=st;

    WAZP3DFUNCTION(73);
    VAR(x)
    VAR(y)
    VAR(st)
    return(W3D_WriteStencilSpan(context,x,y,1,&stencilvalue,&mask));
}
/*==========================================================================*/
ULONG W3D_WriteStencilSpan(W3D_Context *context, ULONG x, ULONG y, ULONG n,ULONG *st, UBYTE *mask)
{
struct WAZP3D_context *WC=context->driver;
register UBYTE *Stencil8=context->stencilbuffer;
register ULONG offset;

    WAZP3DFUNCTION(74);
    VAR(x)
    VAR(y)
    VAR(n)
    VAR(st)
    VAR(mask)

    if(context->stencilbuffer==NULL) WRETURN(W3D_NOSTENCILBUFFER);
    offset=x+y*WC->large;
    Stencil8+=offset;
    XLOOP(n)
        if(mask[x]==1)
            Stencil8[x]=st[x];
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriangleV(W3D_Context *context, W3D_TriangleV *triangle)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(75);

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+3) )
        DrawPrimitive(context);

    SetTexStates(context,triangle->tex,W3D_PRIMITIVE_TRIANGLES);

    GETPTRI(triangle->v1,triangle->v2,triangle->v3)

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriFanV(W3D_Context *context, W3D_TrianglesV *triangles)
{
    struct WAZP3D_context *WC=context->driver;
    LONG n,Pnb;

    WAZP3DFUNCTION(76);
    Pnb=triangles->vertexcount;

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( 3 < Pnb)
        if(Pnb <= Wazp3D->MaxPolyHack)                /* a simple quad after a clipping in x y z can have now more than 4 points */
        {
            SetTexStates(context,triangles->tex,W3D_PRIMITIVE_POLYGON);
            NLOOP(triangles->vertexcount)
                GetVertex(WC,triangles->v[n]);
            DrawPrimitive(context);
            WRETURN(W3D_SUCCESS);
        }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb-2)*3) )
        DrawPrimitive(context);

    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRIANGLES);

    for (n=2;n<Pnb;n++)
        GETPTRI(triangles->v[0],triangles->v[n-1],triangles->v[n])

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawTriStripV(W3D_Context *context, W3D_TrianglesV *triangles)
{
    struct WAZP3D_context *WC=context->driver;
    LONG n,Pnb;

    WAZP3DFUNCTION(77);
    Pnb=triangles->vertexcount;

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)                /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
        SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRISTRIP); /* soft3d will convert that to POLYGON */
        NLOOP(Pnb)
            GetVertex(WC,triangles->v[n]);
        DrawPrimitive(context);
        WRETURN(W3D_SUCCESS);
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb+(Pnb-2)*3) )
        DrawPrimitive(context);

    SetTexStates(context,triangles->tex,W3D_PRIMITIVE_TRIANGLES);

    for (n=2;n<Pnb;n++)
//    if (n&1)      /* reverse vertex order */
        GETPTRI(triangles->v[n-1],triangles->v[n-2],triangles->v[n-0])
//    else
//        GETPTRI(triangles->v[n-2],triangles->v[n-1],triangles->v[n-0])

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
W3D_ScreenMode *W3D_GetScreenmodeList(void)
{
    struct CyberModeNode *cmode;
    struct CyberModeNode *cmodelist;
    W3D_ScreenMode *smode=(W3D_ScreenMode *)&Wazp3D->smodelist;
    WORD n=0;

    WAZP3DFUNCTION(78);
    cmodelist=(struct CyberModeNode *)AllocCModeListTags(CYBRMREQ_MinDepth,16,CYBRMREQ_MaxDepth,32,TAG_DONE);

    if (cmodelist)
    {
        cmode=cmodelist;
        while ( (cmode=(struct CyberModeNode *)cmode->Node.ln_Succ) ->Node.ln_Succ )
            if(n<50)
            {

                smode->ModeID    =cmode->DisplayID;
                smode->Width    =cmode->Width;
                smode->Height    =cmode->Height;
                smode->Depth    =cmode->Depth;
                Libstrcpy(smode->DisplayName,cmode->ModeText);
                smode->Driver    =W3D_TestMode(smode->ModeID);
                smode->Next        =&(smode[1]);
                smode++;
                n++;
            }
        smode->Next=NULL;
    }
    FreeCModeList((struct List *)cmodelist);

    if(n==0)
        return(NULL);
    else
        return( (W3D_ScreenMode *)&Wazp3D->smodelist );
}
/*==========================================================================*/
void W3D_FreeScreenmodeList(W3D_ScreenMode *list)
{
    WAZP3DFUNCTION(79);
    /* list is included in WC so cant be freed */
}
/*==========================================================================*/
ULONG         W3D_BestModeID(struct TagItem *taglist)
{
    ULONG tag,data;
    ULONG /*driver,*/ModeID;
    ULONG large,high,bits;

    WAZP3DFUNCTION(80);
//    driver=(ULONG)&Wazp3D->driver;
    large =320;
    high  =240;
    bits  =32;

    while (taglist->ti_Tag != TAG_DONE)
    {
        if (taglist->ti_Tag == TAG_MORE) {
            taglist = (struct TagItem *)taglist->ti_Data;
            continue;
        }
        tag =taglist->ti_Tag ;
        data=taglist->ti_Data;
        taglist++;
//        if(tag==W3D_BMI_DRIVER)    driver =data;
        if(tag==W3D_BMI_WIDTH)     large  =data;
        if(tag==W3D_BMI_HEIGHT)    high   =data;
        if(tag==W3D_BMI_DEPTH)     bits   =data;
        WTAG(W3D_BMI_DRIVER," ")
        WTAG(W3D_BMI_WIDTH," ")
        WTAG(W3D_BMI_HEIGHT," ")
        WTAG(W3D_BMI_DEPTH," ")
    }

     if(Wazp3D->OnlyTrueColor.ON)
        bits=24;

    ModeID = BestCModeIDTags(
        CYBRBIDTG_Depth        ,bits,
        CYBRBIDTG_NominalWidth    ,large,
        CYBRBIDTG_NominalHeight    ,high,
        TAG_DONE);

    if( W3D_TestMode(ModeID) == NULL)
        return(INVALID_ID);
    else
        return(ModeID);
}
/*==========================================================================*/
#if PROVIDE_VARARG_FUNCTIONS
ULONG         W3D_BestModeIDTags(Tag tag1, ...)
{
    static ULONG tag[100];
    va_list va;
    WORD n=0;

    WAZP3DFUNCTION(81);
    tag[n] = tag1;
    VAR(tag[n])
    va_start (va, tag1);
    do{
        n++;    tag[n]= va_arg(va, ULONG);    VAR(tag[n])
        if(n&2) if (tag[n] == TAG_DONE) break;
    }
    while (n<100);
    va_end(va);

    return (W3D_BestModeID((struct TagItem *)tag));
}
#endif // PROVIDE_VARARG_FUNCTIONS
/*==========================================================================*/
ULONG W3D_VertexPointer(W3D_Context* context, void *pointer, int stride,ULONG mode, ULONG flags)
{


    WAZP3DFUNCTION(82);
    VAR(pointer)
    VAR(stride)
    WINFO(mode,W3D_VERTEX_F_F_F," ")
    WINFO(mode,W3D_VERTEX_F_F_D," ")
    WINFO(mode,W3D_VERTEX_D_D_D," ")

    context->VertexPointer=pointer;         /* Pointer to the vertex buffer array */
    context->VPStride=stride;             /* Stride of vertex array */
    context->VPMode=mode;                 /* Vertex buffer format */
    context->VPFlags=flags;                 /* not yet used */
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_TexCoordPointer(W3D_Context* context, void *pointer, int stride,int unit, int off_v, int off_w, ULONG flags)
{

    WAZP3DFUNCTION(83);
    VAR(pointer)
    VAR(stride)
    VAR(unit)
    VAR(off_v)
    VAR(off_w)
    WINFO(flags,W3D_TEXCOORD_NORMALIZED,"Texture coordinates are normalized ")

    if(W3D_MAX_TMU <= unit)
        WRETURN(W3D_ILLEGALINPUT);

    context->TexCoordPointer[unit]=pointer;
    context->TPStride[unit]=stride;         /* Stride of TexCoordPointers */
    context->TPVOffs[unit]=off_v;            /* Offset to V coordinate */
    context->TPWOffs[unit]=off_w;            /* Offset to W coordinate */
    context->TPFlags[unit]=flags;            /* Flags */
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ColorPointer(W3D_Context* context, void *pointer, int stride,ULONG format, ULONG mode, ULONG flags)
{

    WAZP3DFUNCTION(84);
    VAR(pointer)
    VAR(stride)
    WINFO(format,W3D_COLOR_FLOAT," ")
    WINFO(format,(ULONG)W3D_COLOR_UBYTE," ")
    WINFO(mode,W3D_CMODE_RGB," ")
    WINFO(mode,W3D_CMODE_BGR," ")
    WINFO(mode,W3D_CMODE_RGBA," ")
    WINFO(mode,W3D_CMODE_ARGB," ")
    WINFO(mode,W3D_CMODE_BGRA," ")
    VAR(format)
    VAR(mode)
    VAR(flags)

    context->ColorPointer=pointer;    /* Pointer to the color array */
    context->CPStride=stride;        /* Color pointer stride */
    context->CPMode= mode | format;    /* Mode + color format */
    context->CPFlags=flags;            /* not yet used=0 */
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_BindTexture(W3D_Context* context, ULONG tmu, W3D_Texture *texture)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(85);
    VAR(tmu)
    VAR(texture)
    PrintTexture(texture);

    if(W3D_MAX_TMU <= tmu)
        WRETURN(W3D_ILLEGALINPUT);

    if(context->CurrentTex[tmu]!=texture)
        WC->state.Changed=TRUE;
    context->CurrentTex[tmu]=texture;

/* patch: for crisot demo dont use the normalmap in tex0 but standard tex1 */
#ifdef WARP3DV5
    if(tmu<2)
    if(context->state & W3D_MULTITEXTURE)
    if(WC->Stage[0].ColorCombineMode==W3D_COMBINE_DOT3RGB)
    {
        texture=context->CurrentTex[0]=context->CurrentTex[1];    /* tex0=tex1 */
        WC->state.Changed=TRUE;
        /* so become a tex0 change */
        tmu=0;
        /* dont modulate with colors  as they contain the normals */
        W3D_SetTexEnv(context,texture,W3D_REPLACE,(W3D_Color *)context->globaltexenvcolor);
    }
#endif

    if(tmu==0)
        SetTexStates(context,texture,WC->state.primitive);

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_DrawArray(W3D_Context* context, ULONG primitive, ULONG base, ULONG count)
{
    struct WAZP3D_context *WC=context->driver;
    ULONG n,Pnb,NewPnb,newprimitive;

#define  GETPTRI1(a,b,c) {GetPoint(context,base+a);GetPoint(context,base+b);GetPoint(context,base+c);}
#define GETPLINE1(a,b)   {GetPoint(context,base+a);GetPoint(context,base+b);}
#define GETPOINT1(a)     {GetPoint(context,base+a);}

    WAZP3DFUNCTION(86);

    WINFO(primitive,W3D_PRIMITIVE_TRIANGLES," ")
    WINFO(primitive,W3D_PRIMITIVE_TRIFAN," ")
    WINFO(primitive,W3D_PRIMITIVE_TRISTRIP," ")
    WINFO(primitive,W3D_PRIMITIVE_POINTS," ")
    WINFO(primitive,W3D_PRIMITIVE_LINES," ")
    WINFO(primitive,W3D_PRIMITIVE_LINELOOP," ")
    WINFO(primitive,W3D_PRIMITIVE_LINESTRIP," ")
    VAR(base)
    VAR(count)

    if(WC->state.PointSize!=1)
    if(primitive==W3D_PRIMITIVE_POINTS)
    {
        WC->state.PointSize=1;WC->state.Changed=TRUE;
    }

    if(WC->state.LineSize!=1)
    if(primitive==W3D_PRIMITIVE_LINES)
    {
        WC->state.LineSize=1;WC->state.Changed=TRUE;
    }

    NewPnb=Pnb=count;
    newprimitive=primitive;
/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if(primitive==W3D_PRIMITIVE_TRIFAN)        {NewPnb=(Pnb-2)*3; newprimitive=W3D_PRIMITIVE_TRIANGLES;}
    if(primitive==W3D_PRIMITIVE_TRISTRIP)    {NewPnb=(Pnb-2)*3; newprimitive=W3D_PRIMITIVE_TRIANGLES;}
    if(primitive==W3D_PRIMITIVE_LINESTRIP)    {NewPnb=(Pnb-1)*2; newprimitive=W3D_PRIMITIVE_LINES;}
    if(primitive==W3D_PRIMITIVE_LINELOOP)    {NewPnb=(Pnb  )*2; newprimitive=W3D_PRIMITIVE_LINES;}

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( (primitive==W3D_PRIMITIVE_TRISTRIP) ou (primitive==W3D_PRIMITIVE_TRIFAN) )
    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)                /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
        if(primitive==W3D_PRIMITIVE_TRIFAN)
            primitive=W3D_PRIMITIVE_POLYGON;
        SetTexStates(context,context->CurrentTex[0],primitive); /* soft3d will convert W3D_PRIMITIVE_TRISTRIP to POLYGON */
        NLOOP(Pnb)
            GETPOINT1(n);
        DrawPrimitive(context);
        WRETURN(W3D_SUCCESS);
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb + NewPnb) )
        DrawPrimitive(context);

    SetTexStates(context,context->CurrentTex[0],newprimitive);
/* Warning: MiniGL/OS4 can change context->CurrentTex without using W3D_BindTexture */

    if(primitive==W3D_PRIMITIVE_TRIFAN)
    {
        for (n=2;n<Pnb;n++)
            GETPTRI1(0,n-1,n)
    }

    if(primitive==W3D_PRIMITIVE_TRISTRIP)
    {
        for (n=2;n<Pnb;n++)
//        if (n&1)      /* reverse vertex order */
            GETPTRI1(n-1,n-2,n-0)
//        else
//            GETPTRI1(n-2,n-1,n-0)
    }

    if(primitive==W3D_PRIMITIVE_LINESTRIP)
    {
        NLOOP((Pnb-1))
            GETPLINE1(n,n+1)
    }

    if(primitive==W3D_PRIMITIVE_LINELOOP)
    {
        NLOOP((Pnb-1))
            GETPLINE1(n,n+1)
        GETPLINE1(Pnb-1,0)
    }

    if(primitive==newprimitive)        /* unchanged for W3D_PRIMITIVE_TRIANGLES/POINTS/LINES */
    {
        NLOOP(Pnb)
            GETPOINT1(n);
    }

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);

    DEBUGPRINTF6("  primitive %ld new %ld (WC %ld) Pnb %ld new %ld (WC %ld)\n",primitive,newprimitive,WC->state.primitive,Pnb,NewPnb,WC->Pnb);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void GetPoint2(W3D_Context *context,ULONG n,void *indices, ULONG type)
{
    UBYTE *I8 =indices;
    UWORD *I16=indices;
    ULONG *I32=indices;

    if(type==W3D_INDEX_UBYTE)
        {GetPoint(context,I8[n]);return;}

    if(type==W3D_INDEX_UWORD)
        {GetPoint(context,I16[n]);return;}

    if(type==W3D_INDEX_ULONG)
        {GetPoint(context,I32[n]);return;}

}
/*==========================================================================*/
ULONG W3D_DrawElements(W3D_Context* context, ULONG primitive, ULONG type, ULONG count,void *indices)
{
    struct WAZP3D_context *WC=context->driver;
    ULONG n,Pnb,NewPnb,newprimitive;

#define  GETPTRI2(a,b,c) {GetPoint2(context,a,indices,type);GetPoint2(context,b,indices,type);GetPoint2(context,c,indices,type);}
#define GETPLINE2(a,b)   {GetPoint2(context,a,indices,type);GetPoint2(context,b,indices,type);}
#define GETPOINT2(a)     {GetPoint2(context,a,indices,type);}

    WAZP3DFUNCTION(87);

    WINFO(primitive,W3D_PRIMITIVE_TRIANGLES," ")
    WINFO(primitive,W3D_PRIMITIVE_TRIFAN," ")
    WINFO(primitive,W3D_PRIMITIVE_TRISTRIP," ")
    WINFO(primitive,W3D_PRIMITIVE_POINTS," ")
    WINFO(primitive,W3D_PRIMITIVE_LINES," ")
    WINFO(primitive,W3D_PRIMITIVE_LINELOOP," ")
    WINFO(primitive,W3D_PRIMITIVE_LINESTRIP," ")
    VAR(count)

    if(WC->state.PointSize!=1)
    if(primitive==W3D_PRIMITIVE_POINTS)
    {
        WC->state.PointSize=1;WC->state.Changed=TRUE;
    }

    if(WC->state.LineSize!=1)
    if(primitive==W3D_PRIMITIVE_LINES)
    {
        WC->state.LineSize=1;WC->state.Changed=TRUE;
    }

    NewPnb=Pnb=count;
    newprimitive=primitive;
/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if(primitive==W3D_PRIMITIVE_TRIFAN)        {NewPnb=(Pnb-2)*3; newprimitive=W3D_PRIMITIVE_TRIANGLES;}
    if(primitive==W3D_PRIMITIVE_TRISTRIP)    {NewPnb=(Pnb-2)*3; newprimitive=W3D_PRIMITIVE_TRIANGLES;}
    if(primitive==W3D_PRIMITIVE_LINESTRIP)    {NewPnb=(Pnb-1)*2; newprimitive=W3D_PRIMITIVE_LINES;}
    if(primitive==W3D_PRIMITIVE_LINELOOP)    {NewPnb=(Pnb  )*2; newprimitive=W3D_PRIMITIVE_LINES;}

/* for software convert 4-5-6-7 trifans to polygons = faster */
    if( (primitive==W3D_PRIMITIVE_TRISTRIP) ou (primitive==W3D_PRIMITIVE_TRIFAN) )
    if( 3 < Pnb)
    if(Pnb <= Wazp3D->MaxPolyHack)                /* a simple quad after a clipping in x y z can have now more than 4 points */
    {
        if(primitive==W3D_PRIMITIVE_TRIFAN)
            primitive=W3D_PRIMITIVE_POLYGON;
        SetTexStates(context,context->CurrentTex[0],primitive); /* soft3d will convert W3D_PRIMITIVE_TRISTRIP to POLYGON */
        NLOOP(Pnb)
            GETPOINT2(n);
        DrawPrimitive(context);
        WRETURN(W3D_SUCCESS);
    }

/*v54: now unfold all as TRIANGLES/POINTS/LINES and bufferize them */
    if( MAXPRIM < (WC->Pnb + NewPnb) )
        DrawPrimitive(context);

    SetTexStates(context,context->CurrentTex[0],newprimitive);
/* Warning: MiniGL/OS4 can change context->CurrentTex without using W3D_BindTexture */

    if(primitive==W3D_PRIMITIVE_TRIFAN)
    {
        for (n=2;n<Pnb;n++)
            GETPTRI2(0,n-1,n)
    }

    if(primitive==W3D_PRIMITIVE_TRISTRIP)
    {
        for (n=2;n<Pnb;n++)
//        if (n&1)      /* reverse vertex order */
            GETPTRI2(n-1,n-2,n-0)
//        else
//            GETPTRI2(n-2,n-1,n-0)
    }

    if(primitive==W3D_PRIMITIVE_LINESTRIP)
    {
        NLOOP((Pnb-1))
            GETPLINE2(n,n+1)
    }

    if(primitive==W3D_PRIMITIVE_LINELOOP)
    {
        NLOOP((Pnb-1))
            GETPLINE2(n,n+1)
        GETPLINE2(Pnb-1,0)
    }

    if(primitive==newprimitive)        /* unchanged for W3D_PRIMITIVE_TRIANGLES/POINTS/LINES */
    {
        NLOOP(Pnb)
            GETPOINT2(n);
    }

    if(!StateON(W3D_INDIRECT))        /*v53: If direct mode then draw each primitive (else bufferize primitives)*/
        DrawPrimitive(context);

    DEBUGPRINTF6("  primitive %ld new %ld (WC %ld) Pnb %ld new %ld (WC %ld)\n",primitive,newprimitive,WC->state.primitive,Pnb,NewPnb,WC->Pnb);

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
void         W3D_SetFrontFace(W3D_Context* context, ULONG direction)
{
struct WAZP3D_context *WC=context->driver;

    WAZP3DFUNCTION(88);
    WINFO(direction,W3D_CW ,"Front face is clockwise");
    WINFO(direction,W3D_CCW,"Front face is counter clockwise");
    WC->state.CullingMode=context->FrontFaceOrder=direction;

    if(!StateON(W3D_CULLFACE))
        WC->state.CullingMode=W3D_NOW;
    WC->state.Changed=TRUE;
}
/*==========================================================================*/
#ifdef WARP3DV5
/* This part will need the latests V5 includes */
/*==========================================================================*/
ULONG W3D_SetTextureBlend(W3D_Context *context, struct TagItem *taglist)
{
/* function status: implemented but do nothing */
    struct WAZP3D_context *WC=context->driver;
    struct WAZP3D_blendstage *S=&WC->Stage[0];
    W3D_Color *color;
    ULONG tag,data;
    ULONG stage=0;
    BOOL Combine,Input,EnvMode;


    WAZP3DFUNCTION(89);
/*    VAR(taglist) */
/* evaluate the current state of texture blending, returning either a W3D_SUCCESS if the state is valid, or a W3D_INVALIDINPUT */
    if(taglist==NULL)
    {
        if(WC->blendstagesready)
            WRETURN(W3D_SUCCESS)
        else
            WRETURN(W3D_ILLEGALINPUT)
    }

    SetState(context,W3D_GLOBALTEXENV,W3D_ENABLED);    /* if multitexture ==> allways use global-texenv */

    while (taglist->ti_Tag != TAG_DONE)
    {
        Combine=Input=EnvMode=FALSE;

        if (taglist->ti_Tag == TAG_MORE)
        {
            taglist = (struct TagItem *)taglist->ti_Data;
            continue;
        }

        tag =taglist->ti_Tag  ;    data=taglist->ti_Data ; taglist++;
/*     VAR(tag)        VAR(data) */

        if(tag==W3D_BLEND_STAGE    )
        {
            stage=data;
            if(MAXSTAGE<=stage)
                {WC->blendstagesready=FALSE; WRETURN(W3D_SUCCESS);}
            S=&WC->Stage[stage];
        }


        if(W3D_COLOR_ARG_A <= tag)
        if(  tag <= W3D_ALPHA_ARG_C)
        if(W3D_ARG_TEXTURE_COLOR <= data)        /* then use current S texture */
        if( data <= W3D_ARG_TEXTURE)
            data = data - W3D_ARG_TEXTURE_COLOR + W3D_ARG_TEXTURE0_COLOR + 3*stage ;

        if(tag==W3D_COLOR_ARG_A    )        S->ColorInputA=data;
        if(tag==W3D_ALPHA_ARG_A    )        S->AlphaInputA=data;
        if(tag==W3D_COLOR_ARG_B    )        S->ColorInputB=data;
        if(tag==W3D_ALPHA_ARG_B    )        S->AlphaInputB=data;
        if(tag==W3D_COLOR_ARG_C    )        S->ColorInputC=data;
        if(tag==W3D_ALPHA_ARG_C    )        S->AlphaInputC=data;
        if(tag==W3D_ENV_MODE    )        {S->TexEnvMode=data;S->ColorCombineMode=S->AlphaCombineMode=W3D_COMBINE_DISABLED;}
        if(tag==W3D_COLOR_COMBINE)        {S->ColorCombineMode=data; S->TexEnvMode=W3D_OFF; }
        if(tag==W3D_ALPHA_COMBINE)        {S->AlphaCombineMode=data;S->TexEnvMode=W3D_OFF; }
        if(tag==W3D_COLOR_SCALE    )        S->ScaleRGBA[0]=S->ScaleRGBA[1]=S->ScaleRGBA[2]=data;
        if(tag==W3D_ALPHA_SCALE    )        S->ScaleRGBA[3]=data;
        if(tag==W3D_BLEND_FACTOR)
        { color=(W3D_Color *)data; ColorToRGBA(S->FactorRGBA,color->r,color->g,color->b,1.0); }

#ifdef WAZP3DDEBUG
        if(Wazp3D->DebugVal.ON)
        {

            if(tag==W3D_COLOR_ARG_A) {Libprintf(" W3D_COLOR_ARG_A,"); Input=TRUE;}
            if(tag==W3D_COLOR_ARG_B) {Libprintf(" W3D_COLOR_ARG_B,"); Input=TRUE;}
            if(tag==W3D_COLOR_ARG_C) {Libprintf(" W3D_COLOR_ARG_C,"); Input=TRUE;}
            if(tag==W3D_ALPHA_ARG_A) {Libprintf(" W3D_ALPHA_ARG_A,"); Input=TRUE;}
            if(tag==W3D_ALPHA_ARG_B) {Libprintf(" W3D_ALPHA_ARG_B,"); Input=TRUE;}
            if(tag==W3D_ALPHA_ARG_C) {Libprintf(" W3D_ALPHA_ARG_C,"); Input=TRUE;}

            if(Input)
                switch(data)
                {
                    case W3D_ARG_COMPLEMENT:        Libprintf("W3D_ARG_COMPLEMENT\n");break;
                    case W3D_ARG_DIFFUSE:            Libprintf("W3D_ARG_DIFFUSE\n");break;
                    case W3D_ARG_DIFFUSE_ALPHA:        Libprintf("W3D_ARG_DIFFUSE_ALPHA\n");break;
                    case W3D_ARG_DIFFUSE_COLOR:        Libprintf("W3D_ARG_DIFFUSE_COLOR\n");break;
                    case W3D_ARG_FACTOR:            Libprintf("W3D_ARG_FACTOR\n");break;
                    case W3D_ARG_FACTOR_ALPHA:        Libprintf("W3D_ARG_FACTOR_ALPHA\n");break;
                    case W3D_ARG_FACTOR_COLOR:        Libprintf("W3D_ARG_FACTOR_COLOR\n");break;
                    case W3D_ARG_PREVIOUS:            Libprintf("W3D_ARG_PREVIOUS\n");break;
                    case W3D_ARG_PREVIOUS_ALPHA:        Libprintf("W3D_ARG_PREVIOUS_ALPHA\n");break;
                    case W3D_ARG_PREVIOUS_COLOR :        Libprintf("W3D_ARG_PREVIOUS_COLOR \n");break;
                    case W3D_ARG_SPECULAR:            Libprintf("W3D_ARG_SPECULAR\n");break;
                    case W3D_ARG_SPECULAR_ALPHA:        Libprintf("W3D_ARG_SPECULAR_ALPHA\n");break;
                    case W3D_ARG_SPECULAR_COLOR:        Libprintf("W3D_ARG_SPECULAR_COLOR\n");break;
                    case W3D_ARG_TEXTURE:            Libprintf("W3D_ARG_TEXTURE\n");break;
                    case W3D_ARG_TEXTURE_ALPHA:        Libprintf("W3D_ARG_TEXTURE_ALPHA\n");break;
                    case W3D_ARG_TEXTURE_COLOR:        Libprintf("W3D_ARG_TEXTURE_COLOR\n");break;
                    case W3D_ARG_TEXTURE0:            Libprintf("W3D_ARG_TEXTURE0\n");break;
                    case W3D_ARG_TEXTURE0_ALPHA:        Libprintf("W3D_ARG_TEXTURE0_ALPHA\n");break;
                    case W3D_ARG_TEXTURE0_COLOR:        Libprintf("W3D_ARG_TEXTURE0_COLOR\n");break;
                    case W3D_ARG_TEXTURE1:            Libprintf("W3D_ARG_TEXTURE1\n");break;
                    case W3D_ARG_TEXTURE1_ALPHA:        Libprintf("W3D_ARG_TEXTURE1_ALPHA\n");break;
                    case W3D_ARG_TEXTURE1_COLOR:        Libprintf("W3D_ARG_TEXTURE1_COLOR\n");break;
                    case W3D_ARG_TEXTURE10:            Libprintf("W3D_ARG_TEXTURE10\n");break;
                    case W3D_ARG_TEXTURE10_ALPHA:        Libprintf("W3D_ARG_TEXTURE10_ALPHA\n");break;
                    case W3D_ARG_TEXTURE10_COLOR:        Libprintf("W3D_ARG_TEXTURE10_COLOR\n");break;
                    case W3D_ARG_TEXTURE11:            Libprintf("W3D_ARG_TEXTURE11\n");break;
                    case W3D_ARG_TEXTURE11_ALPHA:        Libprintf("W3D_ARG_TEXTURE11_ALPHA\n");break;
                    case W3D_ARG_TEXTURE11_COLOR:        Libprintf("W3D_ARG_TEXTURE11_COLOR\n");break;
                    case W3D_ARG_TEXTURE12:            Libprintf("W3D_ARG_TEXTURE12\n");break;
                    case W3D_ARG_TEXTURE12_ALPHA:        Libprintf("W3D_ARG_TEXTURE12_ALPHA\n");break;
                    case W3D_ARG_TEXTURE12_COLOR:        Libprintf("W3D_ARG_TEXTURE12_COLOR\n");break;
                    case W3D_ARG_TEXTURE13:            Libprintf("W3D_ARG_TEXTURE13\n");break;
                    case W3D_ARG_TEXTURE13_ALPHA:        Libprintf("W3D_ARG_TEXTURE13_ALPHA\n");break;
                    case W3D_ARG_TEXTURE13_COLOR:        Libprintf("W3D_ARG_TEXTURE13_COLOR\n");break;
                    case W3D_ARG_TEXTURE14:            Libprintf("W3D_ARG_TEXTURE14\n");break;
                    case W3D_ARG_TEXTURE14_ALPHA:        Libprintf("W3D_ARG_TEXTURE14_ALPHA\n");break;
                    case W3D_ARG_TEXTURE14_COLOR:        Libprintf("W3D_ARG_TEXTURE14_COLOR\n");break;
                    case W3D_ARG_TEXTURE15:            Libprintf("W3D_ARG_TEXTURE15\n");break;
                    case W3D_ARG_TEXTURE15_ALPHA:        Libprintf("W3D_ARG_TEXTURE15_ALPHA\n");break;
                    case W3D_ARG_TEXTURE15_COLOR:        Libprintf("W3D_ARG_TEXTURE15_COLOR\n");break;
                    case W3D_ARG_TEXTURE2:            Libprintf("W3D_ARG_TEXTURE2\n");break;
                    case W3D_ARG_TEXTURE2_ALPHA:        Libprintf("W3D_ARG_TEXTURE2_ALPHA\n");break;
                    case W3D_ARG_TEXTURE2_COLOR:        Libprintf("W3D_ARG_TEXTURE2_COLOR\n");break;
                    case W3D_ARG_TEXTURE3:            Libprintf("W3D_ARG_TEXTURE3\n");break;
                    case W3D_ARG_TEXTURE3_ALPHA:        Libprintf("W3D_ARG_TEXTURE3_ALPHA\n");break;
                    case W3D_ARG_TEXTURE3_COLOR:        Libprintf("W3D_ARG_TEXTURE3_COLOR\n");break;
                    case W3D_ARG_TEXTURE4:            Libprintf("W3D_ARG_TEXTURE4\n");break;
                    case W3D_ARG_TEXTURE4_ALPHA:        Libprintf("W3D_ARG_TEXTURE4_ALPHA\n");break;
                    case W3D_ARG_TEXTURE4_COLOR:        Libprintf("W3D_ARG_TEXTURE4_COLOR\n");break;
                    case W3D_ARG_TEXTURE5:            Libprintf("W3D_ARG_TEXTURE5\n");break;
                    case W3D_ARG_TEXTURE5_ALPHA:        Libprintf("W3D_ARG_TEXTURE5_ALPHA\n");break;
                    case W3D_ARG_TEXTURE5_COLOR:        Libprintf("W3D_ARG_TEXTURE5_COLOR\n");break;
                    case W3D_ARG_TEXTURE6:            Libprintf("W3D_ARG_TEXTURE6\n");break;
                    case W3D_ARG_TEXTURE6_ALPHA:        Libprintf("W3D_ARG_TEXTURE6_ALPHA\n");break;
                    case W3D_ARG_TEXTURE6_COLOR:        Libprintf("W3D_ARG_TEXTURE6_COLOR\n");break;
                    case W3D_ARG_TEXTURE7:            Libprintf("W3D_ARG_TEXTURE7\n");break;
                    case W3D_ARG_TEXTURE7_ALPHA:        Libprintf("W3D_ARG_TEXTURE7_ALPHA\n");break;
                    case W3D_ARG_TEXTURE7_COLOR:        Libprintf("W3D_ARG_TEXTURE7_COLOR\n");break;
                    case W3D_ARG_TEXTURE8:            Libprintf("W3D_ARG_TEXTURE8\n");break;
                    case W3D_ARG_TEXTURE8_ALPHA:        Libprintf("W3D_ARG_TEXTURE8_ALPHA\n");break;
                    case W3D_ARG_TEXTURE8_COLOR:        Libprintf("W3D_ARG_TEXTURE8_COLOR\n");break;
                    case W3D_ARG_TEXTURE9:            Libprintf("W3D_ARG_TEXTURE9\n");break;
                    case W3D_ARG_TEXTURE9_ALPHA:        Libprintf("W3D_ARG_TEXTURE9_ALPHA\n");break;
                    case W3D_ARG_TEXTURE9_COLOR:        Libprintf("W3D_ARG_TEXTURE9_COLOR\n");break;
                    default:                    VAR(data);break;
                }


            if(tag==W3D_COLOR_COMBINE)    {Libprintf(" W3D_COLOR_COMBINE,");Combine=TRUE;}
            if(tag==W3D_ALPHA_COMBINE)    {Libprintf(" W3D_ALPHA_COMBINE,");Combine=TRUE;}

            if(Combine)
                switch(data)
                {
                case W3D_COMBINE_DISABLED:        Libprintf("W3D_COMBINE_DISABLED\n");break;
                case W3D_COMBINE_SELECT_A:        Libprintf("W3D_COMBINE_SELECT_A\n");break;
                case W3D_COMBINE_SELECT_B:        Libprintf("W3D_COMBINE_SELECT_B\n");break;
                case W3D_COMBINE_SELECT_C:        Libprintf("W3D_COMBINE_SELECT_C\n");break;
                case W3D_COMBINE_MODULATE:        Libprintf("W3D_COMBINE_MODULATE\n");break;
                case W3D_COMBINE_ADD:            Libprintf("W3D_COMBINE_ADD\n");break;
                case W3D_COMBINE_SUBTRACT:        Libprintf("W3D_COMBINE_SUBTRACT\n");break;
                case W3D_COMBINE_ADDSIGNED:        Libprintf("W3D_COMBINE_ADDSIGNED\n");break;
                case W3D_COMBINE_INTERPOLATE:        Libprintf("W3D_COMBINE_INTERPOLATE\n");break;
                case W3D_COMBINE_ACCUM:            Libprintf("W3D_COMBINE_ACCUM\n");break;
                case W3D_COMBINE_DOT3RGB:        Libprintf("W3D_COMBINE_DOT3RGB\n");break;
                case W3D_COMBINE_DOT3RGBA:        Libprintf("W3D_COMBINE_DOT3RGBA\n");break;
                default:                    VAR(data);break;
                }

            if(tag==W3D_ENV_MODE)    {Libprintf(" W3D_ENV_MODE,");EnvMode=TRUE;}

            if(EnvMode)
                switch(data)
                {
                case W3D_ADD:        Libprintf("W3D_ADD\n");break;
                case W3D_BLEND:        Libprintf("W3D_BLEND\n");break;
                case W3D_DECAL:        Libprintf("W3D_DECAL\n");break;
                case W3D_MODULATE:    Libprintf("W3D_MODULATE\n");break;
                case W3D_OFF:        Libprintf("W3D_OFF\n");break;
                case W3D_REPLACE:        Libprintf("W3D_REPLACE\n");break;
                case W3D_SUB:        Libprintf("W3D_SUB\n");break;
                default:            VAR(data);break;
                }

            WTAG(W3D_BLEND_STAGE    ," W3D_BLEND_STAGE")
            WTAG(W3D_COLOR_SCALE    ," W3D_COLOR_SCALE")
            WTAG(W3D_ALPHA_SCALE    ," W3D_ALPHA_SCALE")
            WTAG(W3D_BLEND_FACTOR    ," W3D_BLEND_FACTOR")
            WTAG(W3D_BLEND_FACTOR    ," W3D_BLEND_FACTOR")

        }
#endif

        if(stage==0)
        if(tag==W3D_ENV_MODE    )
        if(W3D_REPLACE    <= S->TexEnvMode    )    /* check if valid texenvmode ? */
        if(S->TexEnvMode    <= W3D_SUB    )
            W3D_SetTexEnv(context,context->CurrentTex[stage],S->TexEnvMode,(W3D_Color *)context->globaltexenvcolor);

    }

/* patch: for crisot demo */
    if(WC->Stage[0].ColorCombineMode==W3D_COMBINE_DOT3RGB)
    {
        W3D_BindTexture(context,0,context->CurrentTex[1]);
        W3D_SetTexEnv(context,context->CurrentTex[0],WC->Stage[1].TexEnvMode,(W3D_Color *)context->globaltexenvcolor);
    }

    WC->blendstagesready=TRUE;
    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
#if PROVIDE_VARARG_FUNCTIONS
ULONG W3D_SetTextureBlendTags(W3D_Context *context,...)
{
    static ULONG tag[100];
    va_list va;
    WORD n=0;

    WAZP3DFUNCTION(90);
    tag[n] = tag1;
    VAR(tag[n])
    va_start (va, tag1);
    do
    {
        n++;    tag[n]= va_arg(va, ULONG);    VAR(tag[n])
        if(n&2) if (tag[n] == TAG_DONE) break;
    }
    while (n<100);
    va_end(va);

    return W3D_SetTextureBlend(context,(struct TagItem *)tag);
}
#endif // PROVIDE_VARARG_FUNCTIONS
/*==========================================================================*/
ULONG W3D_SecondaryColorPointer(W3D_Context* context, void* pointer,int stride, ULONG format, ULONG mode,ULONG flags)
{
/* function status: implemented but do nothing */

    WAZP3DFUNCTION(91);
    VAR(pointer)
    VAR(stride)
    WINFO(format,W3D_COLOR_FLOAT," ")
    WINFO(format,W3D_COLOR_UBYTE," ")
    WINFO(mode,W3D_CMODE_RGB," ")
    WINFO(mode,W3D_CMODE_BGR," ")
    WINFO(mode,W3D_CMODE_RGBA," ")
    WINFO(mode,W3D_CMODE_ARGB," ")
    WINFO(mode,W3D_CMODE_BGRA," ")
    VAR(format)
    VAR(mode)
    VAR(flags)

    context->SecondaryColorPointer=pointer;    /* Pointer to the color array */
    context->SCPStride=stride;            /* Color pointer stride */
    context->SCPMode= mode | format;        /* Mode + color format */
    context->SCPFlags=flags;            /* not yet used=0 */

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG    W3D_FogCoordPointer(W3D_Context *context, void *pointer,int stride, ULONG mode, ULONG flags)
{
/* function status: implemented but do nothing */
    WAZP3DFUNCTION(92);
    VAR(pointer)
    VAR(stride)
    WINFO(mode,W3D_FOGCOORD_FLOAT," ")
    WINFO(mode,W3D_FOGCOORD_DOUBLE," ")

    context->FogCoordPointer=pointer;     /* Pointer to the fog array */
    context->FCPStride=stride;            /* fog pointer stride */
    context->FCPMode= mode;               /* Mode  format */
    context->FCPFlags=flags;              /* not yet used=0 */

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG    W3D_InterleavedArray(W3D_Context *context, void *pointer,int stride, ULONG vertexFormat, ULONG flags)
{
/* function status: implemented should works */
    struct WAZP3D_context *WC=context->driver;
    UBYTE *pt=pointer;
    ULONG size,TexFlags,error,n;

    WAZP3DFUNCTION(93);
    if(WC->ILpointer        ==pointer)
    if(WC->ILstride        ==stride)
    if(WC->ILvertexFormat    ==vertexFormat)
    if(WC->ILflags        ==flags)
    WRETURN(W3D_SUCCESS);                /* nothing changed => so do nothing */

    WC->ILpointer        =pointer;
    WC->ILstride        =stride;
    WC->ILvertexFormat    =vertexFormat;
    WC->ILflags            =flags;

    VAR(pt)
    size=3*sizeof(float);
    error=W3D_VertexPointer(context,pt,stride,W3D_VERTEX_F_F_F,0);
    if(error!=W3D_SUCCESS) return(error);
    pt=pt+size;

    if(( vertexFormat & W3D_VFORMAT_FOG )!=0)
    {
        REM(W3D_VFORMAT_FOG  )
        VAR(pt)
        size=1*sizeof(float);
        error=W3D_FogCoordPointer(context,pt,stride,W3D_FOGCOORD_FLOAT,0);
        if(error!=W3D_SUCCESS) return(error);
        pt=pt+size;
    }

    if(( vertexFormat & W3D_VFORMAT_COLOR )!=0)
    {
        REM(W3D_VFORMAT_COLOR  )
        VAR(pt)
        size=4*sizeof(float);
        error=W3D_ColorPointer(context,pt,stride,W3D_COLOR_FLOAT,W3D_CMODE_RGBA,0);
        if(error!=W3D_SUCCESS) return(error);
        pt=pt+size;
    }
    if(( vertexFormat & W3D_VFORMAT_PACK_COLOR )!=0)
    {
        REM(W3D_VFORMAT_PACK_COLOR )
        VAR(pt)
        size=4*sizeof(UBYTE);
        error=W3D_ColorPointer(context,pt,stride,W3D_COLOR_UBYTE,W3D_CMODE_BGRA,0); /*bug in Warp3Dv5 doc*/
        if(error!=W3D_SUCCESS) return(error);
        pt=pt+size;
    }

    if(( vertexFormat & W3D_VFORMAT_SCOLOR )!=0)
    {
        REM(W3D_VFORMAT_SCOLOR)
        VAR(pt)
        size=4*sizeof(float);
        error=W3D_SecondaryColorPointer(context,pt,stride,W3D_COLOR_FLOAT,W3D_CMODE_RGBA,0); 

        if(error!=W3D_SUCCESS) return(error);
        pt=pt+size;
    }
    if(( vertexFormat & W3D_VFORMAT_PACK_SCOLOR )!=0)
    {
        REM(W3D_VFORMAT_PACK_SCOLOR)
        VAR(pt)
        size=4*sizeof(UBYTE);
        error=W3D_SecondaryColorPointer(context,pt,stride,W3D_COLOR_UBYTE,W3D_CMODE_BGRA,0); /*bug in Warp3Dv5 doc*/
        if(error!=W3D_SUCCESS) return(error);
        pt=pt+size;
    }

    if(( flags & W3D_TEXCOORD_NORMALIZED )!=0)
        {REM(W3D_TEXCOORD_NORMALIZED); TexFlags=W3D_TEXCOORD_NORMALIZED;}
    else
        {REM(W3D_TEXCOORD_NOT_NORMALIZED); TexFlags=0;}

    NLOOP(16)
        if(( vertexFormat & (W3D_VFORMAT_TCOORD_0<<n) )!=0)
        {
            REM(W3D_VFORMAT_TCOORD_n)
            VAR(n)
            VAR(pt)
            size=3*sizeof(float);
            error=W3D_TexCoordPointer(context,pt,stride,n,sizeof(float),2*sizeof(float),TexFlags);
            if(error!=W3D_SUCCESS) return(error);
            pt=pt+size;
        }

    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_ClearBuffers(W3D_Context *context, W3D_Color *color,W3D_Double *depth, ULONG* stencil)
{
union rgba3D ClearColor;
/* function status: implemented should works */

    WAZP3DFUNCTION(94);
VAR(color)
VAR(depth)
VAR(stencil)
    if(color!=NULL)
    {
        ColorToRGBA(&ClearColor.B.RGBA[0],color->a,color->r,color->g,color->b);  /* want ARGB32 */
        W3D_ClearDrawRegion(context,ClearColor.L.RGBA32);
    }

    if(depth!=NULL)                /* patch: for QT on OS4 */
    if(*depth==0.0)
    {
        *depth=0.999;
    }

    if(depth!=NULL)
        W3D_ClearZBuffer(context,depth);
    if(stencil!=NULL)
        W3D_ClearStencilBuffer(context,stencil);
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG    W3D_SetParameter(W3D_Context * context,ULONG target,APTR pattern)
{
    /* function status: implemented : but works only for classic fog/point parameters */
    struct WAZP3D_context *WC=context->driver;
    W3D_Color *fogcolor=pattern;
    ULONG *ul=pattern;
    float  *f=pattern;

    WAZP3DFUNCTION(95);
    VAR(target)
    VARF(*f)
    VAR(*ul)
    if(target==W3D_STIPPLE_LINE)        WC->StippleLine      =*ul;
    if(target==W3D_STIPPLE_LINE_FACTOR) WC->StippleLineFactor=*ul;
    if(target==W3D_STIPPLE_POLYGON)     WC->StipplePolygon   =*ul;
    if(target==W3D_POINT_SIZE)          WC->state.PointSize  =*f;
    if(target==W3D_LINE_WIDTH)          WC->state.LineSize   =*f;
    if(target==W3D_ZFOG_START)          WC->state.FogZmin    =*f;
    if(target==W3D_ZFOG_END)            WC->state.FogZmax    =*f;
    if(target==W3D_ZFOG_DENSITY)        WC->state.FogDensity =*f;
    if(target==W3D_FOG_MODE)            WC->state.FogMode    =*ul;
    if(target==W3D_WFOG_START)          WC->state.FogZmin    =*f;
    if(target==W3D_WFOG_END)            WC->state.FogZmax    =*f;
    if(target==W3D_WFOG_DENSITY)        WC->state.FogDensity=*f;
    if(target==W3D_FOG_COLOR)           ColorToRGBA(WC->state.FogRGBA,fogcolor->r,fogcolor->g,fogcolor->b,1.0);

    WC->state.Changed=TRUE;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_PinTexture(W3D_Context * context,W3D_Texture * texture,BOOL pinning)
{
/* function status: implemented but do nothing */

    WAZP3DFUNCTION(96);
    texture->pinned=pinning;
    WRETURN(W3D_SUCCESS);
}
/*==========================================================================*/
ULONG W3D_SetDrawRegionTexture(W3D_Context * context,W3D_Texture * texture,W3D_Scissor * scissor)
{
    W3D_Bitmap  w3dbm;
    ULONG bpp2;
    ULONG format=texture->texfmtsrc;

/* function status: implemented should works */
    WAZP3DFUNCTION(97);
/* create a W3D_Bitmap based on the texture */
    bpp2=BytesPerPix2(format);                /* converted texture format*/
    context->w3dbitmap=TRUE;
    w3dbm.width =texture->texwidth;
    w3dbm.height=texture->texheight;
    w3dbm.bprow =texture->texwidth*bpp2;

    if(bpp2 == (32/8))
        w3dbm.format=W3D_FMT_R8G8B8A8;
    else
        w3dbm.format=W3D_FMT_R8G8B8;

    if(texture->texdata!=NULL)
        w3dbm.dest=texture->texdata;
    else
        w3dbm.dest=texture->texsource;

/* Backup for draw region pointers */
    context->orig_drawregion    =context->drawregion;
    context->orig_zbuffer        =context->zbuffer;
    context->orig_stencilbuffer    =context->stencilbuffer;
    context->orig_width        =context->width;
    context->orig_height        =context->height;
    context->orig_yoffset        =context->yoffset;
    context->orig_bprow        =context->bprow;
    Libmemcpy(&context->orig_scissor,&context->scissor,sizeof(W3D_Scissor));

/* then use the W3D_Bitmap as DrawRegion */
    WRETURN(W3D_SetDrawRegion(context,(struct BitMap *)&w3dbm,0,scissor));
}
/*==========================================================================*/
#endif
/*==========================================================================*/
void         PrintAllFunctionsAdresses(void)
{
#ifdef WAZP3DDEBUG
#define  VARH(var) { Libprintf(" " #var " Adresse="); ph((ULONG)var); Libprintf("\n"); }


    VARH(W3D_CreateContext)
    VARH(W3D_DestroyContext)
    VARH(W3D_GetState)
    VARH(W3D_SetState)
    VARH(W3D_Hint)
    VARH(W3D_CheckDriver)
    VARH(W3D_LockHardware)
    VARH(W3D_UnLockHardware)
    VARH(W3D_WaitIdle)
    VARH(W3D_CheckIdle)
    VARH(W3D_Query)
    VARH(W3D_GetTexFmtInfo)
    VARH(W3D_GetDriverState)
    VARH(W3D_GetDestFmt)
    VARH(W3D_GetDrivers)
    VARH(W3D_QueryDriver)
    VARH(W3D_GetDriverTexFmtInfo)
    VARH(W3D_RequestMode)
    VARH(W3D_TestMode)
    VARH(W3D_AllocTexObj)
    VARH(W3D_FreeTexObj)
    VARH(W3D_ReleaseTexture)
    VARH(W3D_FlushTextures)
    VARH(W3D_SetFilter)
    VARH(W3D_SetTexEnv)
    VARH(W3D_SetWrapMode)
    VARH(W3D_UpdateTexImage)
    VARH(W3D_UpdateTexSubImage)
    VARH(W3D_UploadTexture)
    VARH(W3D_FreeAllTexObj)
    VARH(W3D_SetChromaTestBounds)
    VARH(W3D_DrawLine)
    VARH(W3D_DrawPoint)
    VARH(W3D_DrawTriangle)
    VARH(W3D_DrawTriFan)
    VARH(W3D_DrawTriStrip)
    VARH(W3D_Flush)
    VARH(W3D_DrawLineStrip)
    VARH(W3D_DrawLineLoop)
    VARH(W3D_ClearDrawRegion)
    VARH(W3D_SetAlphaMode)
    VARH(W3D_SetBlendMode)
    VARH(W3D_SetDrawRegion)
    VARH(W3D_SetDrawRegionWBM)
    VARH(W3D_SetFogParams)
    VARH(W3D_SetLogicOp)
    VARH(W3D_SetColorMask)
    VARH(W3D_SetPenMask)
    VARH(W3D_SetCurrentColor)
    VARH(W3D_SetCurrentPen)
    VARH(W3D_SetScissor)
    VARH(W3D_FlushFrame)
    VARH(W3D_AllocZBuffer)
    VARH(W3D_FreeZBuffer)
    VARH(W3D_ClearZBuffer)
    VARH(W3D_ReadZPixel)
    VARH(W3D_ReadZSpan)
    VARH(W3D_SetZCompareMode)
    VARH(W3D_WriteZPixel)
    VARH(W3D_WriteZSpan)
    VARH(W3D_AllocStencilBuffer)
    VARH(W3D_ClearStencilBuffer)
    VARH(W3D_FillStencilBuffer)
    VARH(W3D_FreeStencilBuffer)
    VARH(W3D_ReadStencilPixel)
    VARH(W3D_ReadStencilSpan)
    VARH(W3D_SetStencilFunc)
    VARH(W3D_SetStencilOp)
    VARH(W3D_SetWriteMask)
    VARH(W3D_WriteStencilPixel)
    VARH(W3D_WriteStencilSpan)
    VARH(W3D_DrawTriangleV)
    VARH(W3D_DrawTriFanV)
    VARH(W3D_DrawTriStripV)
    VARH(W3D_GetScreenmodeList)
    VARH(W3D_FreeScreenmodeList)
    VARH(W3D_BestModeID)
    VARH(W3D_VertexPointer)
    VARH(W3D_TexCoordPointer)
    VARH(W3D_ColorPointer)
    VARH(W3D_BindTexture)
    VARH(W3D_DrawArray)
    VARH(W3D_DrawElements)
    VARH(W3D_SetFrontFace)

#ifdef WARP3DV5
    VARH(W3D_SetTextureBlend)
    VARH(W3D_SecondaryColorPointer)
    VARH(W3D_FogCoordPointer)
    VARH(W3D_InterleavedArray)
    VARH(W3D_ClearBuffers)
    VARH(W3D_SetParameter)
    VARH(W3D_PinTexture)
    VARH(W3D_SetDrawRegionTexture)
#endif
    LibAlert("All adresses listed");
#endif
}
/*==========================================================================*/
#if !defined(STATWAZP3D)

#ifdef  __AROS__
ADD2INITLIB(WAZP3D_Init, 0);
ADD2EXPUNGELIB(WAZP3D_Close, 0);
#endif

#endif
/*==========================================================================*/



