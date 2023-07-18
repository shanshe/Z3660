# Z3660 by sHaNsHe (Double H Tech)

<br>Z3660 is an Amiga 4000 CPU accelerator board based on A3660 and Z-turn FPGA board.
<br>
<br>Compared to A3660, Z3660 has some key differences:
<br><p style="margin-left:1em;">
<br><b>1</b>. Z3660 accelerator board can only use 060 CPUs. This could be changed in the future, but at this time, only 060 is supported.
<br>
<br><b>2</b>. If Z-turn FPGA board is <b>NOT</b> used, then you get a simple A3660. The only difference is that all A3660 PLDs are replaced by a XC95144XL-TQ144 CPLD. It could be possible to use 060 up to 100MHz, but only 50 MHz and 64 MHz has been tested.  A board with 128 MB of CPU RAM is in the works (to be connected at Z-turn sockets).
<br>
<br><b>3</b>. If you <b>use a Z-turn FPGA board</b>, then you can add 128 MB of CPU RAM (DMA compatible), 256 MB of Z3 RAM (not DMA compatible), RTG 16bit, Ethernet, AHI and MHI. SD to virtual SCSI for using raw RDB disk images, is on WIP state (USB storage maybe in the future). And of course, you can use a 68060 rev6 at 100 MHz.
<br><p style="margin-left:0em;">
<br><b>Wo_FPGA branch. Z3660 <b>without</b> Z-turn FPGA</b>
<br>
<br>This wo_FPGA branch has everything you need to build a Z3660 without Z-turn FPGA.
<br>If you want to build Z3660 with Z-turn FPGA, please go to main branch. The board is the same, but the CPLD firmware and BOM are obviously different.
<br>
<br>Current version is Schematics v0.21. It has all fixes needed by v02 to have DMA compatibility (tested with ReA4091 as bus master), but still have to be tested... WIP...
<br>
<br><p style="text-align:center;"><img src="./Images/Z3660_top_v02.jpg" alt="Z3660_top_v02.jpg" style="width:800px;"><br>v0.2 Z3660 without FPGA</br></p>
<br>Please note that this image is not exactly v021 (it is from v02).
<br>Changes to make it work will be updated as soon I can build one... WIP...
<br>Also BOM of this version has to be revised...WIP...
<br>
<br>Final advice: this Z3660 wo_FPGA version is a <b>W</b>ork <b>I</b>n <b>P</b>rogress!!! Still to be built and tested!!!
