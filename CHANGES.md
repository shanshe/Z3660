# Z3660 Changes

##v1.01
 * First public version

##v1.02
 * SCSI SD emulation first steps.
 * The CPLD firmware doesn't implement DMA accesses, so you can't use busmaster Zorro III boards (like the A4091).
Also A4000T and A3000(T) will not work with any SCSI attached unit.
<br>A4000T can boot, but not use the internal SCSI, if you use the A4000D kickstart.

##v1.03 beta 1
 * Fixed some troubles with 8 bit modes and miniterm 12 (copy_rect_nomask).
 * Added pre-alfa version of warp3d.library. There is too much work to be done here... but... I want to show you this... Warning: the addresses are hardcoded and you will need to have Z3 RAM enabled (autoconfigured RTG has to be at $50000000)!!!!
 * I2C measures are now controlled by a state machine. If I2C get stuck, then I2c measures are stopped (and don't affect to general behavior...).
 * Added new commands to console, so you can increment or decrement some "adjustment parameters" like EMU read/write delays, and ethernet nag delay.
 * ZTop app adjustments for different fonts. And now shows the Beta version.
 * As RTG driver has changed, you need to delete your devs:Picasso96Settings and start again with p96prefs... 
 * SCSI stability fixes<br>
 * The kickstart load has been changed in z3660cfg.txt. You firstly declare upto 9 different kickstarts, and finaly you select one of them using a index. Now you can change between kickstarts in Ztop application.
 * Also, it has been implemented the extended kistart load, in the same way than current kickstarts (they load at $00F00000).
 * The reset behavor has also changed:
<br> 1. If you maintain the reset for 4 seconds, then the CPU will cycle between 060, Musashi, UAE and UAEJIT. 
<br> 2. But if you maintain the reset for 8 seconds, then all env files will be deleted (the configuration used will be what you declared in z3660cfg.txt file as default).

##v1.03 beta 2
 * New config variable "enable_test". You can force a intensive test to CHIP RAM from ARM bus. This will be useful for testing purposes.
 * Update system. After turning on your Amiga, you can press 'C' in the Amiga keyboard to access to a BOOT.bin update system. It is not silly-proof, so don't expect now too much info for now. The sequence of keys you should press should be: 'C', 'I', 'U', 'R'. That is: enable 'C'onsole on HDMI, connect to 'I'nternet, 'U'pdate the Z3660.bin file, and finally 'R'eboot the system. Please wait until some message appears into the HDMI screen... It can take up to 20 seconds for some timeout (USB serial console shows more info).
Now BOOT.bin will search for the file Z3660.bin to boot from it (instead of BOOT.bin). If Z3660.bin is not correct, damaged or has something wrong, then BOOT.bin should continue to boot from itself. So, this should be somehow secure... but please take in mind that I have tried this only for two days...
For this you will need the new CPLD firmware, if you don't update it, then 060 will not boot because in the previous firmware the SNOOP signal was not activated when ARM makes bus accesses...
I think I haven't to say it but... obviously, you will need to connect an ethernet cable to the z-turn...
 * From serial console, you can select different phase for clocks. The setting can't be stored, this is only for my own tests.
 * Warp3D working in 060 and EMU, you will need the Wazp3D-Prefs and set Renderer to "Soft to Bitmap"
 