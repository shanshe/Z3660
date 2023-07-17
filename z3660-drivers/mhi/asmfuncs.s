#----------------------------------------------------------------------------------
# Assembler frames for C interrupt servers that need to return with the Z flag set.
#----------------------------------------------------------------------------------
    .globl  _dev_isr
    .globl  _dev_sisr
_dev_isr:
    jbsr   _cdev_isr
    move.l #0,d0
    rts

_dev_sisr:
    jbsr   _cdev_sisr
    move.l #0,d0
    rts

