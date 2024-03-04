# Z3660 Known Issues / Limitations

##Clock configuration
 * upto v1.02 CPLD and FPGA/ARM versions<br>
The frequency of 060 is limited to 50 and 100 MHz.
<br>When using EMU (MUSASHI, UAE or UAEJIT) then the 060 frequency affects to the emulation chip speed access. So the maximum chip speed is reached when clocking the system at 100 MHz (060 receives the same clock).
<br>

##SCSI SD emulation
 * v1.01 CPLD and FPGA/ARM versions<br>
There is a bug in hunk relocation code that makes that one hdf grows upto fill the SD. Please don't use this version of firmware with SCSI SD emulation.
 <br><br>
 * v1.02 CPLD and FPGA/ARM versions<br>
 1) Copying data between partitions of different hdf files, will result in an unformatted partition and all data will be lost (on the written partition).
<br> Copying data between partitions on the same hdf file, doesn't seem to be affected by this issue.
<br>
<br> 2) The only working filesystem is FFS (DOSx identifiers). Others are not working due a bug in hunk relocation code.

##DMA and Zorro III bus bastmer
 * v1.02 CPLD and FPGA/ARM versions<br>
The CPLD firmware doesn't implement DMA accesses, so you can't use busmaster Zorro III boards (like the A4091).
Also A4000T and A3000(T) will not work with any SCSI attached unit.
<br>A4000T can boot, but not use the internal SCSI, if you use the A4000D kickstart.
