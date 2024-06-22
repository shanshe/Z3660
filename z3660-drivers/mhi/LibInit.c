/*
** Library init code
**
** Based on CLib37x by Andreas R. Kleinert
*/

#define __USE_SYSBASE

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "version.h"
#include "mhilib.h"

ULONG __stdargs L_OpenLibs(struct MHI_LibBase *MhiLibBase);
void  __stdargs L_CloseLibs(struct MHI_LibBase *MhiLibBase);
BOOL UserLibInit(struct MHI_LibBase *MhiLibBase);
void UserLibCleanup(struct MHI_LibBase *MhiLibBase);


struct ExecBase   *SysBase = NULL;
struct DosLibrary *DOSBase = NULL;


const char __aligned MpLibName [] __attribute__((used)) = LIBNAME".library";
const char __aligned MpLibID   [] __attribute__((used)) = LIBNAME" "NUM2STR(VERSION)"."NUM2STR(REVISION)" ("NUM2STR(DATE)")\0";
const char __aligned VERSTRING [] __attribute__((used)) = "\0$VER: " LIBNAME".library "NUM2STR(VERSION)"."NUM2STR(REVISION)" ("NUM2STR(DATE)")\0";

/* ----------------------------------------------------------------------------------------
   ! ROMTag and Library inilitalization structure:
   !
   ! Below you find the ROMTag, which is the most important "magic" part of a library
   ! (as for any other resident module). You should not need to modify any of the
   ! structures directly, since all the data is referenced from constants from somewhere else.
   !
   ! You may place the ROMTag directly after the LibStart (-> StartUp.c) function as well.
   !
   ! Note, that the data initialization structure may be somewhat redundant - it's
   ! for demonstration purposes.
   !
   ! EndResident can be placed somewhere else - but it must follow the ROMTag and
   ! it must not be placed in a different SECTION.
   ---------------------------------------------------------------------------------------- */

extern ULONG InitTab[];
extern APTR EndResident; /* below */

struct Resident __aligned ROMTag = {    /* do not change */
	RTC_MATCHWORD,
	&ROMTag,
	&EndResident,
	RTF_AUTOINIT,
	VERSION,
	NT_LIBRARY,
	0,
	(char*)&MpLibName[0],
	(char*)&MpLibID[0],
	&InitTab[0]
};

APTR EndResident;

struct MyDataInit {                     /* do not change */
	UWORD ln_Type_Init;      UWORD ln_Type_Offset;      UWORD ln_Type_Content;
	UBYTE ln_Name_Init;      UBYTE ln_Name_Offset;      ULONG ln_Name_Content;
	UWORD lib_Flags_Init;    UWORD lib_Flags_Offset;    UWORD lib_Flags_Content;
	UWORD lib_Version_Init;  UWORD lib_Version_Offset;  UWORD lib_Version_Content;
	UWORD lib_Revision_Init; UWORD lib_Revision_Offset; UWORD lib_Revision_Content;
	UBYTE lib_IdString_Init; UBYTE lib_IdString_Offset; ULONG lib_IdString_Content;
	ULONG ENDMARK;
} DataTab = {
	INITBYTE(OFFSET(Node,         ln_Type),      NT_LIBRARY),
	0x80, (UBYTE) (ULONG)OFFSET(Node,    ln_Name),      (ULONG) &MpLibName[0],
	INITBYTE(OFFSET(Library,      lib_Flags),    LIBF_SUMUSED|LIBF_CHANGED),
	INITWORD(OFFSET(Library,      lib_Version),  VERSION),
	INITWORD(OFFSET(Library,      lib_Revision), REVISION),
	0x80, (UBYTE) (ULONG)OFFSET(Library, lib_IdString), (ULONG) &MpLibID[0],
	(ULONG) 0
};


/* ----------------------------------------------------------------------------------------
   ! L_OpenLibs:
   !
   ! Since this one is called by InitLib, libraries not shareable between Processes or
   ! libraries messing with RamLib (deadlock and crash) may not be opened here.
   !
   ! You may bypass this by calling this function fromout LibOpen, but then you will
   ! have to a) protect it by a semaphore and b) make sure, that libraries are only
   ! opened once (when using global library bases).
   ---------------------------------------------------------------------------------------- */

ULONG __stdargs L_OpenLibs(struct MHI_LibBase *MhiLibBase) {
	SysBase = (*((struct ExecBase **) 4));
	MhiLibBase->mhi_SysBase = SysBase;

	DOSBase = (struct DosLibrary*)OpenLibrary((CONST_STRPTR)"dos.library", 36);
	if(!DOSBase) return(FALSE);
	MhiLibBase->mhi_DOSBase = DOSBase;

	return UserLibInit(MhiLibBase);
}

/* ----------------------------------------------------------------------------------------
   ! L_CloseLibs:
   !
   ! This one by default is called by ExpungeLib, which only can take place once
   ! and thus per definition is single-threaded.
   !
   ! When calling this fromout LibClose instead, you will have to protect it by a
   ! semaphore, since you don't know whether a given CloseLibrary(foobase) may cause a Wait().
   ! Additionally, there should be protection, that a library won't be closed twice.
   ---------------------------------------------------------------------------------------- */

void __stdargs L_CloseLibs(struct MHI_LibBase *MhiLibBase) {
	UserLibCleanup(MhiLibBase);
	if(DOSBase) CloseLibrary((struct Library*)DOSBase);
	DOSBase = NULL;
}

