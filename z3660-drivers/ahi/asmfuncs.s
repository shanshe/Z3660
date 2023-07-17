#----------------------------------------------------------------------------------
# Assembler frames for C interrupt servers that need to return with the Z flag set.
#----------------------------------------------------------------------------------
    .globl  _dev_isr
_dev_isr:
    jbsr   _cdev_isr
    move.l #0,d0
    rts

#------------------------------------------------------------------------------
# Assembler frames for C library functions that need to preserve all registers.
#------------------------------------------------------------------------------
    .globl  _intAHIsub_Enable
_intAHIsub_Enable:
    movem.l d0-d7/a0-a6,-(sp)
    jbsr    _cintAHIsub_Enable
    movem.l (sp)+,d0-d7/a0-a6
    rts

    .globl  _intAHIsub_Disable
_intAHIsub_Disable:
    movem.l d0-d7/a0-a6,-(sp)
    jbsr    _cintAHIsub_Disable
    movem.l (sp)+,d0-d7/a0-a6
    rts


