# BOOT FILES
Put in this directory the files BOOT.bin, z3660_scsi.rom and z3660cfg.txt.

BOOT.bin contains the FPGA bitstream and ARMs standalone programs.

z3660_scsi.rom contains the bootrom of SD SCSI emulation, so you need it only if you are going to boot from hdf files.

z3660cfg.txt contains some configuration of the Z3660. It should be very self explanatory.

In z3660cfg.txt, you can comment out a line with a `#` on the first character of a line. If you use multiple lines with the same configuration, the last one config is what will be finally selected.

# A note on the type of SD card to use in the Z-Turn
I have received a SD with every Z-turn I have bought, but they are HC and yes, they are really slow!!! Please use XC SDs.