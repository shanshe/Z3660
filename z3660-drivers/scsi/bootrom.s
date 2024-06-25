**
** Sample autoboot code fragment
**
** These are the calling conventions for the Diag routine
**
** A7 -- points to at least 2K of stack
** A6 -- ExecBase
** A5 -- ExpansionBase
** A3 -- your board's ConfigDev structure
** A2 -- Base of diag/init area that was copied
** A0 -- Base of your board
**
** Your Diag routine should return a non-zero value in D0 for success.
** If this value is NULL, then the diag/init area that was copied
** will be returned to the free memory pool.
**

    INCLUDE "exec/types.i"
    INCLUDE "exec/nodes.i"
    INCLUDE "exec/resident.i"
    INCLUDE "libraries/configvars.i"


    ; LVO's resolved by linking with library amiga.lib
;    XREF   _LVOFindResident

ROMINFO         EQU   0
ROMOFFS         EQU   $6000

* ROMINFO defines whether you want the AUTOCONFIG information in
* the beginning of your ROM (set to 0 if you instead have PALS
* providing the AUTOCONFIG information instead)
*
* ROMOFFS is the offset from your board base where your ROMs appear.
* Your ROMs might appear at offset 0 and contain your AUTOCONFIG
* information in the high nibbles of the first $40 words ($80 bytes).
* Or, your autoconfig ID information may be in a PAL, with your
* ROMs possibly being addressed at some offset (for example $2000)
* from your board base.  This ROMOFFS constant will be used as an
* additional offset from your configured board address when patching
* structures which require absolute pointers to ROM code or data.

*----- We'll store Version and Revision in serial number
VERSION         EQU   37     ; also the high word of serial number
REVISION        EQU   1      ; also the low word of serial number

* See the Addison-Wesley Amiga Hardware Manual for more info.
    
MANUF_ID        EQU   2011   ; CBM assigned (2011 for hackers only)
PRODUCT_ID      EQU   1      ; Manufacturer picks product ID

BOARDSIZE       EQU   $10000 ; How much address space board decodes
SIZE_FLAG       EQU   3      ; Autoconfig 3-bit flag for BOARDSIZE
                             ;   0=$800000(8meg)  4=$80000(512K)
                             ;   1=$10000(64K)    5=$100000(1meg)
                             ;   2=$20000(128K)   6=$200000(2meg)
                             ;   3=$40000(256K)   7=$400000(4meg)
                CODE

; Exec stuff
AllocMem        EQU -198
InitResident    EQU -102
FindResident    EQU -96
OpenLibrary     EQU -552
CloseLibrary    EQU -414
OpenResource    EQU -$1F2
AddResource     EQU -$1E6

; Expansion stuff
MakeDosNode     EQU -144
AddDosNode      EQU -150
AddBootNode     EQU -36

; PiSCSI stuff
PiSCSI_OFFSET   EQU $2000
PiSCSIAddr1     EQU   $20
PiSCSIAddr2     EQU   $24
PiSCSIAddr3     EQU   $28
PiSCSIAddr4     EQU   $2C
PiSCSIDebugMe   EQU   $30
PiSCSIDriver    EQU   $50
PiSCSINextPart  EQU   $54
PiSCSIGetPart   EQU   $58
PiSCSIGetPrio   EQU   $5C
PiSCSIGetFS     EQU   $70
PiSCSINextFS    EQU   $74
PiSCSICopyFS    EQU   $78
PiSCSIFSSize    EQU   $7C
PiSCSISetFSH    EQU   $80
PiSCSIDbgMsg    EQU $1000
PiSCSIDbg1      EQU $1010
PiSCSIDbg2      EQU $1014
PiSCSIDbg3      EQU $1018
PiSCSIDbg4      EQU $101C
PiSCSIDbg5      EQU $1020
PiSCSIDbg6      EQU $1024
PiSCSIDbg7      EQU $1028
PiSCSIDbg8      EQU $102C

*******  RomStart  ***************************************************
**********************************************************************

RomStart:

*******  DiagStart  **************************************************
DiagStart:  ; This is the DiagArea structure whose relative offset from
            ; your board base appears as the Init Diag vector in your
            ; autoconfig ID information.  This structure is designed
            ; to use all relative pointers (no patching needed).
            dc.b     DAC_WORDWIDE+DAC_CONFIGTIME    ; da_Config
            dc.b     0                              ; da_Flags
            dc.w     EndCopy-DiagStart              ; da_Size
            dc.w     DiagEntry-DiagStart            ; da_DiagPoint
            dc.w     BootEntry-DiagStart            ; da_BootPoint
            dc.w     DevName-DiagStart              ; da_Name
            dc.w     0                              ; da_Reserved01
            dc.w     0                              ; da_Reserved02

*******  Resident Structure  *****************************************
Romtag:
            dc.w     RTC_MATCHWORD      ; UWORD RT_MATCHWORD
rt_Match:   dc.l     Romtag-DiagStart   ; APTR  RT_MATCHTAG
rt_End:     dc.l     EndCopy-DiagStart  ; APTR  RT_ENDSKIP
            dc.b     RTW_COLDSTART      ; UBYTE RT_FLAGS
            dc.b     VERSION            ; UBYTE RT_VERSION
            dc.b     NT_DEVICE          ; UBYTE RT_TYPE
            dc.b     20                 ; BYTE  RT_PRI
rt_Name:    dc.l     DevName-DiagStart  ; APTR  RT_NAME
rt_Id:      dc.l     IdString-DiagStart ; APTR  RT_IDSTRING
rt_Init:    dc.l     Init-RomStart      ; APTR  RT_INIT


******* Strings referenced in Diag Copy area  ************************
DevName:    dc.b     'z3660_scsi.device',0                      ; Name string
IdString    dc.b     'Z3660 SCSI v0.8',0   ; Id string

DosName:        dc.b     'dos.library',0                ; DOS library name
ExpansionName:  dc.b     'expansion.library',0
LibName:        dc.b     'z3660_scsi.device',0

DosDevName: dc.b     'ZHD',0        ; dos device name for MakeDosNode()
                                    ;   (dos device will be ABC:)

            ds.w     0              ; word align

*******  DiagEntry  **************************************************
**********************************************************************
*
*   success = DiagEntry(BoardBase,DiagCopy, configDev)
*   d0                  a0        a2        a3
*
*   Called by expansion architecture to relocate any pointers
*   in the copied diagnostic area.   We will patch the romtag.
*   If you have pre-coded your MakeDosNode packet, BootNode,
*   or device initialization structures, they would also need
*   to be within this copy area, and patched by this routine.
*
**********************************************************************

            align 2
DiagEntry:
            lea     cardbase(pc),a1
            move.l  a0,d0
            add.l   #PiSCSI_OFFSET,d0
            move.l  d0,(a1)
            move.l  cardbase(pc),a1
*            move.l  #$50002000,a1
            move.l  #1,PiSCSIDebugMe(a1)
            move.l  a3,PiSCSIAddr1(a1)

            lea     patchTable-RomStart(a0),a1   ; find patch table
            adda.l  #ROMOFFS,a1                  ; adjusting for ROMOFFS

* Patch relative pointers to labels within DiagCopy area
* by adding Diag RAM copy address.  These pointers were coded as
* long relative offsets from base of the DiagArea structure.
*
dpatches:
            move.l  a2,d1           ;d1=base of ram Diag copy
dloop:
            move.w  (a1)+,d0        ;d0=word offs. into Diag needing patch
            bmi.s   bpatches        ;-1 is end of word patch offset table
            add.l   d1,0(a2,d0.w)   ;add DiagCopy addr to coded rel. offset
            bra.s   dloop

* Patches relative pointers to labels within the ROM by adding
* the board base address + ROMOFFS.  These pointers were coded as
* long relative offsets from RomStart.
*
bpatches:
            move.l  a0,d1           ;d1 = board base address
            add.l   #ROMOFFS,d1     ;add offset to where your ROMs are
rloop:
            move.w  (a1)+,d0        ;d0=word offs. into Diag needing patch
            bmi.s   endpatches       ;-1 is end of patch offset table
            add.l   d1,0(a2,d0.w)    ;add ROM address to coded relative offset
            bra.s   rloop

endpatches:
            moveq.l #1,d0           ; indicate "success"
            rts


*******  BootEntry  **************************************************
**********************************************************************

            align 2
BootEntry:
            move.l  cardbase(pc),a1
*            move.l  #$50002000,a1
            move.l  #2,PiSCSIDebugMe(a1)

            lea     DosName(PC),a1        ; 'dos.library',0
            jsr     FindResident(a6)      ; find the DOS resident tag
            move.l  d0,a0                 ; in order to bootstrap
            move.l  RT_INIT(A0),a0        ; set vector to DOS INIT
            jsr     (a0)                  ; and initialize DOS
            rts
cardbase:   dc.l    0

*
* End of the Diag copy area which is copied to RAM
*
EndCopy:
*************************************************************************

*************************************************************************
*
*   Beginning of ROM driver code and data that is accessed only in
*   the ROM space.  This must all be position-independent.
*

patchTable:
* Word offsets into Diag area where pointers need Diag copy address added
            dc.w   rt_Match-DiagStart
            dc.w   rt_End-DiagStart
            dc.w   rt_Name-DiagStart
            dc.w   rt_Id-DiagStart
            dc.w   -1

* Word offsets into Diag area where pointers need boardbase+ROMOFFS added
            dc.w   rt_Init-DiagStart
            dc.w   -1

*******  Romtag InitEntry  **********************************************
*************************************************************************

            align 2
Init:       ; After Diag patching, our romtag will point to this
            ; routine in ROM so that it can be called at Resident
            ; initialization time.
            ; This routine will be similar to a normal expansion device
            ; initialization routine, but will MakeDosNode then set up a
            ; BootNode, and Enqueue() on eb_MountList.
            ;
            movem.l a5-a6,-(a7)           ; Push A6 to stack
            jsr     get_boardbase
            move.w  #$00B8,$dff09a        ; Disable interrupts during init
            move.l  #3,PiSCSIDebugMe(a5)

            move.l  #11,PiSCSIDebugMe(a5)
            movea.l 4,a6
            lea     LibName(pc),a1
            jsr     FindResident(a6)
            jsr     get_boardbase
            move.l  #10,PiSCSIDebugMe(a5)
            cmp.l   #0,d0
            bne.s   SkipDriverLoad        ; Library is already loaded, jump straight to partitions

            move.l  #4,PiSCSIDebugMe(a5)
            movea.l 4,a6
            move.l  #$40000,d0
            moveq   #0,d1
            jsr     AllocMem(a6)          ; Allocate memory for the PiStorm to copy the driver to

            jsr     get_boardbase
            move.l  d0,PiSCSIDriver(a5)   ; Copy the PiSCSI driver to allocated memory and patch offsets

            move.l  #5,PiSCSIDebugMe(a5)
            move.l  d0,a1
            move.l  #0,d1
            movea.l 4,a6
            add.l   #$028,a1              ; points to RTC_MATCHWORD of z3660_scsi.device, it is compiler dependent
            jsr     InitResident(a6)      ; Initialize the PiSCSI driver

SkipDriverLoad:
            jsr     get_boardbase
            move.l  #9,PiSCSIDebugMe(a5)
            bra.w   LoadFileSystems

FSLoadExit:
            lea     ExpansionName(pc),a1
            moveq   #0,d0
            jsr     OpenLibrary(a6)       ; Open expansion.library to make this work, somehow
            move.l  d0,a6

            jsr     get_boardbase
            move.l  #7,PiSCSIDebugMe(a5)
PartitionLoop:
            jsr     get_boardbase
            move.l  PiSCSIGetPart(a5),d0  ; Get the available partition in the current slot
            beq.s   EndPartitions         ; If the next partition returns 0, there's no additional partitions
            move.l  d0,a0
            jsr     MakeDosNode(a6)
            jsr     get_boardbase
            move.l  d0,PiSCSISetFSH(a5)
            move.l  d0,a0
            move.l  PiSCSIGetPrio(a5),d0
            move.l  #0,d1
            move.l  PiSCSIAddr1(a5),a1
            jsr     AddBootNode(a6)
            jsr     get_boardbase
            move.l  #1,PiSCSINextPart(a5) ; Switch to the next partition
            bra.w   PartitionLoop

get_boardbase:
            move.l  d0,-(a7)
            lea     get_boardbase-RomStart(pc),a5
            move.l  a5,d0
            and.l   #$FFF00000,d0
            add.l   #PiSCSI_OFFSET,d0
            move.l  d0,a5
            move.l  (a7)+,d0
*            move.l  #$50002000,a5
            rts
EndPartitions:
            jsr     get_boardbase
            move.l  #8,PiSCSIDebugMe(a5)
            move.l  a6,a1
            move.l  #800,PiSCSIDebugMe(a5)
            movea.l 4,a6
            move.l  #801,PiSCSIDebugMe(a5)
            jsr     CloseLibrary(a6)
            jsr     get_boardbase
            move.l  #802,PiSCSIDebugMe(a5)

            move.l  #803,PiSCSIDebugMe(a5)

            move.w  #$80B8,$dff09a        ; Re-enable interrupts
            move.l  #804,PiSCSIDebugMe(a5)
            moveq.l #1,d0                 ; indicate "success"
            move.l  #805,PiSCSIDebugMe(a5)
            movem.l (a7)+,a5-a6           ; Pop A6 from stack
            rts

            align 4
FileSysName:    dc.b    'FileSystem.resource',0
FileSysCreator: dc.b    'Z3660',0

            align 4
CurFS:      dc.l    $0
FSResource: dc.l    $0

            align 2
LoadFileSystems:
            movem.l d0-d7/a0-a6,-(sp)     ; Push registers to stack
            jsr     get_boardbase
            move.l  #30,PiSCSIDebugMe(a5)
            lea     FileSysName(pc),a1
            jsr     OpenResource(a6)
            tst.l   d0
            bne     FSRExists

            jsr     get_boardbase
            move.l  #33,PiSCSIDebugMe(a5) ; FileSystem.resource isn't open, create it
            lea     FSRes(pc),a1
            move.l  a1,-(a7)
            jsr     AddResource(a6)
            move.l  (a7)+,a0
            move.l  a0,d0

FSRExists:  
            jsr     get_boardbase
            move.l  d0,PiSCSIAddr2(a5)    ; PiSCSIAddr2 is now FileSystem.resource
            move.l  #31,PiSCSIDebugMe(a5)
            move.l  PiSCSIAddr2(a5),a0
            move.l  PiSCSIGetFS(a5),d0
            cmp.l   #0,d0
            beq.w   FSDone
            move.l  d0,d7

FSNext:     
            jsr     get_boardbase
            move.l  #45,PiSCSIDebugMe(a5)
            lea     fsr_FileSysEntries(a0),a0
            move.l  a0,d2
            move.l  LH_HEAD(a0),d0
            beq.w   NoEntries

FSLoop:     
            jsr     get_boardbase
            move.l  #34,PiSCSIDebugMe(a5)
            move.l  d0,a1
            move.l  #35,PiSCSIDebugMe(a5)
            cmp.l   fse_DosType(a1),d7
            move.l  #36,PiSCSIDebugMe(a5)
            beq.w   AlreadyLoaded
            move.l  #37,PiSCSIDebugMe(a5)
            move.l  LN_SUCC(a1),d0
            bne.w   FSLoop
            move.l  #390,PiSCSIDebugMe(a5)
            bra.w   NoEntries

            align 2
NoEntries:  
            jsr     get_boardbase
            move.l  #39,PiSCSIDebugMe(a5)
            move.l  PiSCSIFSSize(a5),d0
            move.l  #40,PiSCSIDebugMe(a5)
            move.l  #0,d1
            move.l  #41,PiSCSIDebugMe(a5)
            jsr     AllocMem(a6)
            jsr     get_boardbase
            move.l  d0,PiSCSIAddr3(a5)
            move.l  #1,PiSCSICopyFS(a5)

AlreadyLoaded:
            jsr     get_boardbase
            move.l  #480,PiSCSIDebugMe(a5)
            move.l  PiSCSIAddr2(a5),a0
            move.l  #1,PiSCSINextFS(a5)
            move.l  PiSCSIGetFS(a5),d0
            move.l  d0,d7
            cmp.l   #0,d0
            bne.w   FSNext

FSDone: 
            jsr     get_boardbase
            move.l  #37,PiSCSIDebugMe(a5)
            move.l  #32,PiSCSIDebugMe(a5) ; Couldn't open FileSystem.resource, Kick 1.2/1.3?

            movem.l (sp)+,d0-d7/a0-a6     ; Pop registers from stack
            bra.w   FSLoadExit

FileSysRes
            dc.l    0
            dc.l    0
            dc.b    NT_RESOURCE
            dc.b    0
            dc.l    FileSysName
            dc.l    FileSysCreator
.Head
            dc.l    .Tail
.Tail
            dc.l    0
            dc.l    .Head
            dc.b    NT_RESOURCE
            dc.b    0
