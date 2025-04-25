# TIMING FILES
The <b>timings.txt</b> is a text file with the name of the timings file that the ARM of the ZTurn board will use to generate all clocks needed for the Z3660.<br>
For now, we have only two sets of timings: A4000 and AA3000 (the systems that I own).<br>
The idea is that those files can be adapted to different systems, and if a user discovers that some different phases work better than others, the user can share those files and more users can benefit from it.
<br>
Starting version 1.03 BETA 18, the clock system has been changed so when an EMU is used, then the 060 is clocked to 50 MHz or less. For this reason the CPLD now needs to be programmed with the jed BETA 18 version.
