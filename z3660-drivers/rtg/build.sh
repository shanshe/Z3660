export VBCC=/opt/vbcc
export PATH=$PATH:$VBCC/bin

vc +aos68k -nostdlib -I$VBCC/targets/m68k-amigaos/include -I$VBCC/NDK_3.9/Include/include_h -v -k -c99 -O2 -cpu=68060 -fpu=68060 -o rtg/Z3660.card rtg/gfx.c -lamiga

