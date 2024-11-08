# BOOT FILES
To boot your Amiga with the Z3660, you must have a microSD inserted in the Z-Turn. It must have two partitions (MBR). First partition must be formatted with FAT32, and the sencond partition can be formatted with exFAT (to support >4GB files).

In this directory you can find the content of the two partitions. In the first partition will be only the BOOT.bin file. In the second partition will be z3660_scsi.rom and z3660cfg.txt files, and some folders.
 * First Partition (FAT32 formatted)
 
   <b>BOOT.bin</b> contains the FPGA bitstream and ARMs standalone programs. It needs to be located in the first FAT32 partition.

 * Second Partition (exFAT formatted)

   <b>z3660_scsi.rom</b> contains the bootrom of SD SCSI emulation, so you need it only if you are going to boot from hdf files.

   <b>z3660.jed</b> is the CPLD firmware that can be flashed with the CPLD programmer.

   <b>z3660cfg.txt</b> contains some configuration of the Z3660. It should be very self explanatory.
In z3660cfg.txt, you can comment out a line with a `#` on the first character of a line. If you use multiple lines with the same configuration, the last one config is what will be finally selected.

   <b>Folders</b>: each folder has a readme...

# A note on the type of microSD card to use in the Z-Turn
I have received a microSD with every Z-turn I have bought, but they are HC type... they are really slow!!! Please use XC type microSDs.