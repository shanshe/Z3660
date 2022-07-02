@ECHO OFF
REM Windows specific vbcc build script
REM This implies that you have the following:
REM 1) vbcc already installed and in your path (check https://blitterstudio.com/)
REM 2) The NDK 3.9 included in your "aos68k" config
REM 
REM Please adapt the script accordingly if your environment is different
REM 
vc +aos68k -nostdlib -I/opt/vbcc/targets/m68k-amigaos/include -I/opt/vbcc/NDK_3.9/Include/include_h -v -k -c99 -O2 -cpu=68060 -fpu=68060 -o rtg/Z3660.card rtg/gfx.c -lamiga
