	incdir	include:
	ifnd	_LVOSupervisor
	include	lvo/exec_lib.i
	;include	exec/exec_lib.i
	endc
	include exec/execbase.i

	ifnd	_LVORawDoFmt
_LVORawDoFmt			equ 	-$20a
	endc
	ifnd	_LVORawPutChar
_LVORawPutChar			equ 	-$204
_LVORawIOInit			equ	-$1f8
_LVORawMayGetChar		equ 	-$1fe
	endc

	XDEF	_KPrintF

;A0 = format
;A1 = arguments
_KPrintF:
;	ifd	DEBUG

	movem.l	d0-d2/a0-a3/a6,-(sp)

	move.l	4.w,a6
	jsr	_LVORawIOInit(a6)		;better safe than sorry

	lea	.kprintfproc(pc),a2
	suba.l	a3,a3				;data pointer (0 here)
	jsr	_LVORawDoFmt(a6)	

	movem.l	(sp)+,d0-d2/a0-a3/a6
	rts
.kprintfproc:	
	movem.l	d0/d1/a0/a1/a6,-(sp)
	move.l	4.w,a6
	jsr	_LVORawPutChar(a6)	;	move.b	d0,(a3)+
	movem.l	(sp)+,d0/d1/a0/a1/a6

;	endc	;DEBUG

	rts


