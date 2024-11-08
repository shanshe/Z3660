;------------------------------------------------------------------------------
;
; MC68060 AmigaOS boot support code
; for Kickstarts 3.0,3.1,3.5,3.9
;
; (C) 2022 Henryk Richter
;
; use vasmm68k_mot -Fbin 
;------------------------------------------------------------------------------
; Adapted to Z3660 RTG config
;------------------------------------------------------------------------------
	INCLUDE "exec/types.i"
    INCLUDE "exec/nodes.i"
    INCLUDE "exec/resident.i"
    INCLUDE "libraries/configvars.i"
    INCLUDE "resources/filesysres.i"
;------------------------------------------------------------------------------
; Compile-time options
;------------------------------------------------------------------------------
P5BARS             EQU    1    ;if set=1, then show green-blue bars at boot
                               ;(much like P5 boards)
;expansion Manufacturer ID
EXPANSION_MANUF    EQU    5195
;expansion ID
EXPANSION_ID       EQU    1

;EXP_MODULE_ENABLED EQU 1

;------------------------------------------------------------------------------
IllegalInstructionVector EQU $10
Line1111Vector           EQU $2C
TempSupStack             EQU $400
CACRB_EnableI            EQU $1        ;Enable instruction cache bit 020/30
CACRF_EnableI            EQU $2

;exec
_LVOOpenLibrary        EQU   -552
_LVOCloseLibrary       EQU   -414
;expansion
_LVOAddConfigDev       EQU   -30
_LVOAllocConfigDev     EQU   -48
_LVOConfigBoard        EQU   -60
_LVOFindConfigDev      EQU   -72

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;CPU variant for assembler
    mc68060

;------------------------------------------------------------------------------
;final location
    ORG    $F00000

;------------------------------------------------------------------------------
; Code
    dc.w      $1111        ;magic DiagROM ID
BOOTROM_ENTER:            ;we arrive here with A5 = return address
    or.w      #$700,SR        ;paranoid Disable()
    move.w    #$7fff,$DFF096        ;disable DMA (for good measure)
    lea       $BFE001,a0
    move.b    #3,$200(a0)        ;/LED, OVL as output
    move.b    #2,(a0)            ;LED=hi, OVL=0 -> ChipRAM available

    move.l    IllegalInstructionVector.w,a1    ;save Illegal Instruction and LineF vectors
    move.l    Line1111Vector.w,a2
    lea       crashvector(pc),a0
    move.l    a0,IllegalInstructionVector.w
    move.l    a0,Line1111Vector.w
    move.l    sp,a0
    lea       TempSupStack.w,sp        ; temporary Stack Pointer (same as Exec)

    ;the code below avoids a crash on xx060
    ;alternatively, just the moveq/movec would suffice where all other CPUs 
    ;(20/30,xx040,LC060,EC060) are pushed into the trap vector trampoline

    ;test for 040/60
    moveq     #CACRF_EnableI,d0        ; 020/30 only flag
    movec.l   d0,CACR                ; this will crash on 68000/010 -> crashvector
    movec.l   CACR,d0
    btst      #CACRB_EnableI,d0        ; this flag is ignored on 68040/60
    bne.s     no_040_60

    movec.l   PCR,d0                ; this will crash 040 -> crashvector
    swap      d0                ; get upper 16 Bit
    cmp.w     #$430,d0            ; ID for full 060 (EC/LC have 0x431)
    bne.s     no_060_FPU            ; nope, no need for FP disable

    moveq     #2,d0
    movec.l   d0,PCR                ; disable FPU in PCR

no_060_FPU:
no_040_60:
    moveq     #0,d0
    movec.l   d0,CACR                ; courtesy for 020/30/60 (note: doesn't apply to crash situation)
crashvector:
    move.l    a0,sp                ;restore Stack Pointer
    move.l    a1,IllegalInstructionVector.w    ;restore Illegal Instruction and LineF vectors
    move.l    a2,Line1111Vector.w

    ;green-blue bars
    ;
    ifne    P5BARS
     ;init agnus/alice
     lea      $DFF000,A4        ; Base address of custom chip area.
     move.w   #$7FFF,D0
     move.w   D0,$9A(A4)        ; Disable all interrupts.
     move.w   D0,$9C(A4)        ; Clear all pending interrupts.
     move.w   D0,$96(A4)        ; Disable all DMA.
        
     ; Set a blank, black display.
     move.w   #$200,$0100(A4)    ; BPLCON0 = Blank screen.
     move.w   #$0,$0110(A4)    ; Bitplane 0 data = all zeros.
     move.w   #$0,$0180(A4)    ; Background colour = black.
        
     moveq    #-1,d1
     lsr.w    #7,d1             ; longer delay for ZZ9000
     moveq    #-1,d0
.wait2:
     lsr.w    #8,d0            ; $ff.w
.wait:
     move.w   d0,$dff180
     tst.b    $BFE001
     dbf      d0,.wait        ; $ffff.w
     dbf      d1,.wait2
    endc      ;P5BARS

    jmp       (A5)            ; return to Kickstart


;--------------------- Expansion ID module -------------------------
    ifd EXP_MODULE_ENABLED
    ifd EXPANSION_MANUF
    ifd EXPANSION_ID

    cnop    0,4

ExpansionIDMod:
    dc.w    RTC_MATCHWORD
    dc.l    ExpansionIDMod
    dc.l    ExpansionIDModEnd
    dc.b    RTF_COLDSTART
    dc.b    40       ;Version
    dc.b    0        ;Type
    dc.b    5        ;Priority 
    dc.l    ExpansionIDName
    dc.l    ExpansionIDName2
    dc.l    ExpansionIDInitFunc

;--- init code -----
ExpansionIDInitFunc:
    movem.l d2-d3/a2-a3/a5/a6,-(sp)
    move.l  4.w,a6
    lea     ExpName(pc),a1
    moveq   #0,d0
    jsr     _LVOOpenLibrary(a6)
    tst.l   d0
    beq.w   .noexp
    move.l  d0,a6

    ;---------- are we registered yet ? ---------------------
    suba.l  a0,a0                ;oldConfigDev
    move.l  #EXPANSION_MANUF,d0  ;manufacturer
    moveq   #EXPANSION_ID,d1     ;product
    jsr     _LVOFindConfigDev(A6)
    tst.l   d0                   ;huh? Already there? 
    bne.s   .expdone

    jsr     _LVOAllocConfigDev(A6)
    tst.l   d0
    beq.s   .expdone
    move.l  d0,a0
    move.l  d0,a1

    move.l  #$10000000,cd_BoardAddr(a0)           ;Boot ROM address
    move.l  #$08000000,cd_BoardSize(a0)           ;128MB
    move.w  #$1000,cd_SlotAddr(a0)                ;Boot ROM address
    move.w  #$0800,cd_SlotSize(a0)                ;128MB
    move.b  #CDF_CONFIGME,cd_Flags(a0)
    move.b  #ERT_ZORROIII+ERTF_DIAGVALID+3,cd_Rom+er_Type(a0) ; 3 = 128MB
    move.b  #EXPANSION_ID,cd_Rom+er_Product(a0)
    move.b  #$70,cd_Rom+er_Flags(a0)
    move.w  #EXPANSION_MANUF,cd_Rom+er_Manufacturer(a0) ;
	move.l  #$00000000,cd_Rom+er_SerialNumber(a0)
	move.w  #$6000,cd_Rom+er_InitDiagVec(a0)
    jsr     _LVOAddConfigDev(A6)
*	move.l  #$10000000,a0
*	;error = ConfigBoard( board, configDev )
*	;  D0                   A0     A1
*
*	jsr     _LVOConfigBoard(A6)
*   move.l  #$10000000,cd_BoardAddr(a1)           ;Boot ROM address

.expdone:
    move.l  a6,a1
    move.l  4.w,a6
    jsr     _LVOCloseLibrary(a6)
.noexp

    movem.l (sp)+,d2-d3/a2-a3/a5/a6	
    moveq   #0,d0
    rts

ExpansionIDName:  dc.b    "Z3660",10,0
ExpansionIDName2: dc.b    "Z3660 ROM 1.0",10,0
ExpName:          dc.b    "expansion.library",0
    cnop    0,4
ExpansionIDModEnd:

    endc    ;EXPANSION_ID
    endc    ;EXPANSION_MANUF
    endc    ;EXP_MODULE_ENABLED
* SCSI rom will be here...
RomStart:
