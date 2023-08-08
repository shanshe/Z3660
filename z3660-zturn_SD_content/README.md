# BOOT FILES
In this directory you can find the files BOOT.bin, z3660_scsi.rom and z3660cfg.txt, and some folders.
All this content will go into the root of your Z-Turn microSD (must have MBR, first partition FAT32 formatted, so we are limited to 4GB files. Second partition could be exFAT formatted, but it doesn't work right now...).

BOOT.bin contains the FPGA bitstream and ARMs standalone programs.

z3660_scsi.rom contains the bootrom of SD SCSI emulation, so you need it only if you are going to boot from hdf files.

z3660cfg.txt contains some configuration of the Z3660. It should be very self explanatory.

In z3660cfg.txt, you can comment out a line with a `#` on the first character of a line. If you use multiple lines with the same configuration, the last one config is what will be finally selected.

Folders: each folder has a readme...

# A note on the type of microSD card to use in the Z-Turn
I have received a microSD with every Z-turn I have bought, but they are HC and yes, they are really slow!!! Please use XC microSDs.