# Z3660 Known Issues / Limitations

##Clock configuration
 * upto v1.02 CPLD and FPGA/ARM versions<br>
The frequency of 060 is limited to 50 and 100 MHz.
<br>When using EMU (MUSASHI, UAE or UAEJIT) then the 060 frequency affects to the emulation chip speed access. So the maximum chip speed is reached when clocking the system at 100 MHz (060 receives the same clock).
<br>

* v1.03 beta 4 FPGA/ARM versions<br>
The frequency of the 060 can be selected between 50 and 100 MHz, in 5 Mhz steps. 90 MHz and 95 MHz are a bit unstable in my systems (A4000 and AA3000). All other frequencies seems to work fine (again, in my systems).

##SCSI SD emulation
 * v1.01 CPLD and FPGA/ARM versions<br>
There is a bug in hunk relocation code that makes that one hdf grows upto fill the SD. Please don't use this version of firmware with SCSI SD emulation.
 <br><br>
 * v1.02 CPLD and FPGA/ARM versions<br>
 1) Copying data between partitions of different hdf files, will result in an unformatted partition and all data will be lost (on the written partition).
<br> Copying data between partitions on the same hdf file, doesn't seem to be affected by this issue.
<br>
 2) The only working filesystem is FFS (DOSx identifiers). Others are not working due a bug in hunk relocation code.
<br><br>
 * v1.03 beta 4 FPGA/ARM versions<br>
 The issues in v1.02 has been fixed. You can copy between partitions and use any filesystems.
 But still please use with caution, make always a backup of your files... WIP...
<br><br>
 * v1.03 beta 14 FPGA/ARM versions<br>
 After beta 12, you can make a third partition in the SD with type 0x76 (like you do with pistorm/emu68). It will be automounted by the SCSI SD emulation.
 
##DMA and Zorro III bus bastmer
 * v1.02 CPLD and FPGA/ARM versions<br>
The CPLD firmware doesn't implement DMA accesses, so you can't use busmaster Zorro III boards (like the A4091).
Also A4000T and A3000(T) will not work with any SCSI attached unit.
<br>A4000T can boot, but not use the internal SCSI, if you use the A4000D kickstart.
<br><br>
 * v1.03 beta 2 CPLD and FPGA/ARM versions<br>
The CPLD firmware implement DMA accesses, but EMU has not been implemented yet. So you can use DMA boards only with a 060 CPU.
