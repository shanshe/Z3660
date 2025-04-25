# KICKSTART FILES
Put in this directory all kickstarts images that you want to use when using CPU emulation. (Not yet for real CPU, work in progress).

Remember their names, you will need to name them exactly in the z3660cfg.txt file.

There is a way of dumping your real kickstart to a file, but it is done manually using some ARM commented out code. Don't know if it is necessary anyway, as everyone has they're own collection of kickstart files for a lot of machines, emulators, etc...

Please, if you want to use 3.1 kickstart, remember that a 060 doesn't boot with original 3.1 kickstart. You need a patched kickstart to disable FPU... You can of course use 3.1.4 or 3.2.x kickstarts files.

Supplied files:

<b>DiagROM1.3.rom</b> - The fantastic diagnostic rom from John "Chucky" Hertell
<br><br>
<b>kick060.rom</b> - Expansion rom to disable FPU and make some colors. For 060 with 3.1 kickstart compatibility.
<br><br>
<b>kick_z3660scsi.rom</b> - Same as kick060.rom, and support for Z3660 scsi boot (useful when AUTOC RTG enabled checkbox of ZTop is unchecked, so RTG will not be autoconfigured and located at $10000000).
<br><br>
<b>kick060_z3660scsi_noscsi.rom</b> - Same as kick_z3660scsi.rom, and disables the mainboard SCSI (useful in A3000/AA3000/A4000T when you don't want the internal SCSI to work).
