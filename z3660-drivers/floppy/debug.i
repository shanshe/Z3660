;	ifd DEBUG
;		XDEF		_AbsExecBase
;		XDEF		_LVORawDoFmt
;		XDEF		_LVORawPutChar
;		XDEF		_LVORawMayGetChar
;		XDEF		_AbsExecBase
;	endif

	ifnd	_LVORawDoFmt
_LVORawDoFmt			equ 	-$20a
	endc
	ifnd	_LVORawPutChar
_LVORawPutChar			equ 	-$204
_LVORawIOInit			equ	-$1f8
_LVORawMayGetChar		equ 	-$1fe
	endc

;
;
;
	ifne	DEBUG

	XREF	_KPrintF

WRITEDEBUG	macro
		movem.l			d0/d1/a0/a1,-(sp)	
		ifnb \9
			move.l		\9,-(sp)
		endif
		ifnb \8
			move.l		\8,-(sp)
		endif
		ifnb \7
			move.l		\7,-(sp)
		endif
		ifnb \6
			move.l		\6,-(sp)
		endif
		ifnb \5
			move.l		\5,-(sp)
		endif
		ifnb \4
			move.l		\4,-(sp)
		endif
		ifnb \3
			move.l		\3,-(sp)
		endif
		ifnb \2
			move.l		\2,-(sp)
		endif
		lea.l		(\1),a0
		move.l		sp,a1
		jsr			_KPrintF
		ifnb \2
			add.l		#4,sp
		endif
		ifnb \3
			add.l		#4,sp
		endif
		ifnb \4
			add.l		#4,sp
		endif
		ifnb \5
			add.l		#4,sp
		endif
		ifnb \6
			add.l		#4,sp
		endif
		ifnb \7
			add.l		#4,sp
		endif
		ifnb \8
			add.l		#4,sp
		endif
		ifnb \9
			add.l		#4,sp
		endif
		movem.l			(sp)+,d0/d1/a0/a1
	endm
	else
WRITEDEBUG macro
	;
	endm
	endc


;
;
;
WRITEOUT macro
	movem.l			d0/d1/d2/a0/a1/a6,-(sp)	
	ifnb \9
		move.l		\9,-(sp)
	endif
	ifnb \8
		move.l		\8,-(sp)
	endif
	ifnb \7
		move.l		\7,-(sp)
	endif
	ifnb \6
		move.l		\6,-(sp)
	endif
	ifnb \5
		move.l		\5,-(sp)
	endif
	ifnb \4
		move.l		\4,-(sp)
	endif
	ifnb \3
		move.l		\3,-(sp)
	endif
	ifnb \2
		move.l		\2,-(sp)
	endif
	ifd DEBUG
		move.l		\1,a0
		move.l		sp,a1
		jsr			_KPrintF
	endif
	move.l		\1,d1
	move.l		sp,d2
	movea.l		DOSBase,a6
	jsr			_LVOVPrintf(a6)		
	ifnb \2
		add.l		#4,sp
	endif
	ifnb \3
		add.l		#4,sp
	endif
	ifnb \4
		add.l		#4,sp
	endif
	ifnb \5
		add.l		#4,sp
	endif
	ifnb \6
		add.l		#4,sp
	endif
	ifnb \7
		add.l		#4,sp
	endif
	ifnb \8
		add.l		#4,sp
	endif
	ifnb \9
		add.l		#4,sp
	endif
	movem.l			(sp)+,d0/d1/d2/a0/a1/a6
	endm

;COUNTERS only when DEBUG is on
	ifd	DEBUG
	ifne	DEBUG
DEBUG_COUNTERS	EQU	1
	endc
	endc
	
	ifd	DEBUG_COUNTERS
COUNTER_INC	macro
		addq.l #1,\1
		endm
	else
COUNTER_INC	macro
	nop
		endm
	endc

; write usage of all emulated instructions if DEBUG is 1 to RAWIO (sushi/sashimi/serial)
	ifd	DEBUG
	ifne	DEBUG
HAVE_DEBUGINSTR	EQU	1
DEBUGINSTR	macro
	ifnb	\3
	WRITEDEBUG	\1,\2,\3
	else
	WRITEDEBUG	\1,\2
	endc
		endm
	endc
	endc
	ifnd	HAVE_DEBUGINSTR
DEBUGINSTR	macro
	endm
	endc


