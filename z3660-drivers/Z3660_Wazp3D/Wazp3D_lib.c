/* Wazp3D Beta 56 : Alain THELLIER - Paris - FRANCE - (November 2006 to 2014)     */
/* Code clean-up and library enhancements from Gunther Nikl                 */
/* Adaptation to AROS from Matthias Rustler                            */
/* Adaptation to Morphos from Szil�rd 'BSzili' Bir�                         */
/* LICENSE: GNU General Public License (GNU GPL) for this file                */    


/* This file contain the Wazp3d3D OS3/68k library header                    */

#if defined(__GNUC__) && defined(__mc68000__)
asm ("jra _ReturnError");
#endif
/*======================================================================================*/
#define NAMETXT   "Warp3D"
#define VERSION   4
#define REVISION  2
#define DATETXT   "25.09.2006"
#define VERSTXT   "4.2"
/*======================================================================================*/
#include <exec/memory.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <stdarg.h> /* for varargs */
#include "Warp3D_protos.h"
/*======================================================================================*/
BOOL WAZP3D_Init(void *exec);
void WAZP3D_Open();
void WAZP3D_Close();
void WAZP3D_Expunge();
/*======================================================================================*/
/*  Name:            SDI_compiler.h
    Versionstring:   $VER: SDI_compiler.h 1.4 (30.03.2000)
    Author:          SDI
    Distribution:    PD
    Description:     defines to hide compiler stuff

 1.1   25.06.98 : created from data made by Gunter Nikl
 1.2   17.11.99 : added VBCC
 1.3   29.02.00 : fixed VBCC REG define
 1.4   30.03.00 : fixed SAVEDS for VBCC
*/
/*======================================================================================*/
/* first "exceptions" */

#if defined(__MAXON__)
  #define STDARGS
  #define REGARGS
  #define SAVEDS
  #define INLINE inline
#elif defined(__VBCC__)
  #define STDARGS
  #define REGARGS
  #define INLINE
  #define REG(reg,arg) __reg(#reg) arg
#elif defined(__STORM__)
  #define STDARGS
  #define REGARGS
  #define INLINE inline
#elif defined(__SASC)
  #define ASM(arg) arg __asm
#elif defined(__GNUC__)
  #define REG(reg,arg) arg __asm(#reg)
  #define LREG(reg,arg) register REG(reg,arg)
#endif

/* then "common" ones */

#if !defined(ASM)
  #define ASM(arg) arg
#endif
#if !defined(REG)
  #define REG(reg,arg) register __##reg arg
#endif
#if !defined(LREG)
  #define LREG(reg,arg) register arg
#endif
#if !defined(CONST)
  #define CONST const
#endif
#if !defined(SAVEDS)
  #define SAVEDS __saveds
#endif
#if !defined(INLINE)
  #define INLINE __inline
#endif
#if !defined(REGARGS)
  #define REGARGS __regargs
#endif
#if !defined(STDARGS)
  #define STDARGS __stdargs
#endif

#define D0(arg)  REG(d0,arg)
#define D1(arg)  REG(d1,arg)
#define D2(arg)  REG(d2,arg)
#define D3(arg)  REG(d3,arg)
#define D4(arg)  REG(d4,arg)
#define D5(arg)  REG(d5,arg)
#define D6(arg)  REG(d6,arg)
#define D7(arg)  REG(d7,arg)

#define A0(arg)  REG(a0,arg)
#define A1(arg)  REG(a1,arg)
#define A2(arg)  REG(a2,arg)
#define A3(arg)  REG(a3,arg)
#define A4(arg)  REG(a4,arg)
#define A5(arg)  REG(a5,arg)
#define A6(arg)  REG(a6,arg)
#define A7(arg)  REG(a7,arg)
/*======================================================================================*/
#if defined(_M68060) || defined(__M68060) || defined(__mc68060)
    #define CPUTXT    "060"
    #define CPUMSK    AFF_68060
#elif defined(_M68040) || defined(__M68040) || defined(__mc68040)
    #define CPUTXT    "040"
    #define CPUMSK    AFF_68040
#elif defined(_M68030) || defined(__M68030) || defined(__mc68030)
    #define CPUTXT    "030"
    #define CPUMSK    AFF_68030
#elif defined(_M68020) || defined(__M68020) || defined(__mc68020)
    #define CPUTXT    "020"
    #define CPUMSK    AFF_68020
#else
    #define CPUTXT    ""
    #define CPUMSK    0
#endif
/*======================================================================================*/
#define LIBNAME  NAMETXT ".library"
#define IDSTRING NAMETXT " " VERSTXT " (" DATETXT ") " CPUTXT "\r\n"
#define FULLNAME LIBNAME " " VERSTXT " (" DATETXT ")680" CPUTXT
/*======================================================================================*/
/*    SegList pointer definition    */
#if defined(_AROS)
    typedef struct SegList * SEGLISTPTR;
#elif defined(__VBCC__)
    typedef APTR SEGLISTPTR;
#else
    typedef BPTR SEGLISTPTR;
#endif
/*======================================================================================*/
/*    library base private structure. The official one does not contain all the private fields! */
struct ExampleLibrary
    {
    struct Library       LibNode;
    UBYTE                Pad[2];
    ULONG                NumCalls;
    struct ExecBase *    SysBase;
    SEGLISTPTR           SegList;
    };
#define DeleteLibrary(BASE) FreeMem((APTR)((char*)(BASE)-((BASE)->lib_NegSize)),(BASE)->lib_NegSize+(BASE)->lib_PosSize)
/*======================================================================================*/
/* First executable routine of this library; must return an error to the unsuspecting caller */
LONG ReturnError(void)
{
    return -1;
}
/*======================================================================================*/
extern const ULONG LibInitTable[4]; /* the prototype */
/* The library loader looks for this marker in the memory the library code
   and data will occupy. It is responsible setting up the Library base data structure.  */
/*======================================================================================*/
const struct Resident RomTag = {
    RTC_MATCHWORD,                   /* Marker value. */
    (struct Resident *)&RomTag,      /* This points back to itself. */
    (struct Resident *)LibInitTable, /* This points somewhere behind this marker. */
    RTF_AUTOINIT,                    /* The Library should be set up according to the given table. */
    VERSION,                         /* The version of this Library. */
    NT_LIBRARY,                      /* This defines this module as a Library. */
    0,                               /* Initialization priority of this Library; unused. */
    LIBNAME,                         /* Points to the name of the Library. */
    IDSTRING,                        /* The identification string of this Library. */
    (APTR)&LibInitTable              /* This table is for initializing the Library. */
};
/*======================================================================================*/
/* The mandatory reserved library function */
ULONG LibReserved(void)
{
    return 0;
}
/*======================================================================================*/
/* Open the library, as called via OpenLibrary() */
ASM(struct Library *) LibOpen(REG(a6, struct ExampleLibrary * libBase))
{
    /* Prevent delayed expunge and increment opencnt */
    libBase->LibNode.lib_Flags &= ~LIBF_DELEXP;
    libBase->LibNode.lib_OpenCnt++;
    WAZP3D_Open();
    return &libBase->LibNode;
}
/*======================================================================================*/
/* Expunge the library, remove it from memory */
ASM(SEGLISTPTR) LibExpunge(REG(a6, struct ExampleLibrary * libBase))
{
    if(!libBase->LibNode.lib_OpenCnt)
        {
        SEGLISTPTR SegList;
        SegList = libBase->SegList;
        WAZP3D_Expunge();

        /* Remove the library from the public list */
        Remove((struct Node *) libBase);

        /* Free the vector table and the library data */
        DeleteLibrary(&libBase->LibNode);

        return SegList;
        }
    else
        libBase->LibNode.lib_Flags |= LIBF_DELEXP;

    /* Return the segment pointer, if any */
    return 0;
}
/*======================================================================================*/
/* Close the library, as called by CloseLibrary() */
ASM(SEGLISTPTR) LibClose(REG(a6, struct ExampleLibrary * libBase))
{
    WAZP3D_Close();
    if(!(--libBase->LibNode.lib_OpenCnt))
        {
        if(libBase->LibNode.lib_Flags & LIBF_DELEXP)
            return LibExpunge(libBase);
        }
    return 0;
}
/*======================================================================================*/
/* Initialize library */
ASM(struct Library *) LibInit(REG(a0, SEGLISTPTR SegList),REG(d0, struct ExampleLibrary * libBase), REG(a6, struct ExecBase *exec))
{
    SysBase = exec;
    if (CPUMSK && (SysBase->AttnFlags & CPUMSK) == 0)
      return 0;

    /* Remember stuff */
    libBase->SegList = SegList;
    libBase->SysBase = SysBase;

    if(WAZP3D_Init(exec) )
        return &libBase->LibNode;

    /* Free the vector table and the library data */
    DeleteLibrary(&libBase->LibNode);
    return 0;
}
/*======================================================================================*/
/*    your own library's accessables functions    */
#include "Wazp3D_functions_glue.h"
/*======================================================================================*/
struct LibInitData {
    UBYTE i_Type;     UBYTE o_Type;     UBYTE    d_Type;     UBYTE p_Type;
    UBYTE i_Name;     UBYTE o_Name;     STRPTR   d_Name;
    UBYTE i_Flags;    UBYTE o_Flags;    UBYTE    d_Flags;    UBYTE p_Flags;
    UBYTE i_Version;  UBYTE o_Version;  UWORD    d_Version;
    UBYTE i_Revision; UBYTE o_Revision; UWORD    d_Revision;
    UBYTE i_IdString; UBYTE o_IdString; STRPTR   d_IdString;
    ULONG endmark;
};
/*======================================================================================*/
static const struct LibInitData LibInitData = {
#ifdef __VBCC__        /* VBCC does not like OFFSET macro */
 0xA0,    8, NT_LIBRARY,              0,
 0x80, 10, LIBNAME,
 0xA0, 14, LIBF_SUMUSED|LIBF_CHANGED, 0,
 0x90, 20, VERSION,
 0x90, 22, REVISION,
 0x80, 24, IDSTRING,
#else
 0xA0, (UBYTE) (LONG)OFFSET(Node,    ln_Type),      NT_LIBRARY,                0,
 0x80, (UBYTE) (LONG)OFFSET(Node,    ln_Name),      (unsigned char*)LIBNAME,
 0xA0, (UBYTE) (LONG)OFFSET(Library, lib_Flags),    LIBF_SUMUSED|LIBF_CHANGED, 0,
 0x90, (UBYTE) (LONG)OFFSET(Library, lib_Version),  VERSION,
 0x90, (UBYTE) (LONG)OFFSET(Library, lib_Revision), REVISION,
 0x80, (UBYTE) (LONG)OFFSET(Library, lib_IdString), (unsigned char*)IDSTRING,
#endif
 0
};
/*======================================================================================*/
/* The following data structures and data are responsible for*/
/*     setting up the Library base data structure and the library*/
/*     function vector.*/
/*======================================================================================*/
const ULONG LibInitTable[4] = {
    (ULONG)sizeof(struct ExampleLibrary), /* Size of the base data structure */
    (ULONG)LibVectors,                    /* Points to the function vector */
    (ULONG)&LibInitData,                  /* Library base data structure setup table */
    (ULONG)LibInit                        /* The address of the routine to do the setup */
};
/*======================================================================================*/
