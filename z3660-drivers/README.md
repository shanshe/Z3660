# Z3660 drivers

To build all these drivers, you will need vbcc and m68k-amigaos-gcc compilers. Also you will need a NDK (I'm using NDK from OS3.9).
<br>To build ZTop app you will need SASC under vamos emulation.
<br>See Makefile in order to adjust your paths.
<br>With the command line:
<br>`make all`
<br>you will compile all necessary drivers.
<br>If you have xdftool installed, then you can create an adf file that contains all drivers to be copied to your Amiga:
<br>`make adf`
<br>This will create an adf file called "0_Z3660.adf".
<br>The "z3660_scsi.rom" file (scsi directory) must be copied to the second partition of the SD (the exFAT formatted partition).