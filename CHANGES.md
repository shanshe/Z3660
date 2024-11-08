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

##v1.03 beta 3
 * SCSI SD emulation had two main issues.
<br> 1. The first one was that when copying a lot of data sometines the change between units was no applied, so the destination hdf had a bad write to a unknown block sector... this made the hdf corruption.
<br> 2. The second issue was a bug in the relocation of the filesystems. Even with the fixed code, I had a partition that was not working. When I debugged the problem I could see that the loaded filesystem had a bigger size than it should. It had in the hdf two repeated blocks. When I deleted the filesystem and back to write it (using FSUAE), then the problem went away. So be careful with the filesystems. Corrupted filesystems can't be repaired easily and posibly will make something wrong with the SD...
Finally, I have working and booting several hdf and partitions with PFS3
 * Warp3D should work now with and without Zorro3 RAM. It had some addreses hardcoded in beta 2, now all addresses are calculated in the system boot.
 * I have been working on the reset of 060 and ARM (I always reseted the ARM when Amiga was reseted, and this made that "Boot with no Startup-Sequence" didn't work). Now the reset of ARM is not attached to the reset of the Amiga, but sometimes the ARM doesn't reinit the Amiga autoconfig (that is done in FPGA when using the 060. EMU is not affected) so sometimes you will not see the SCSI, Z3 RAM and RTG boards... WIP...

##v1.03 beta 4
 * Ethernet driver got statistics and then apps like netmon now shows info about your network (thanks to @Stefan R )
 * New Boot screen showing a new logo, and info about version and options loaded. You need a new folder named images in exFAT partition.
 * RTG now has different MMU configurations in its different memory zones. So, the cache activity of the ARM RAM is not seen as RTG glitches, but without loosing the acceleration of the cache.
 
##v1.03 beta 5
 * Ethernet address can be selected from z3660_cfg.fiile. But lwip (BOOT.BIN update tool) refuses the new values, I'm still investigating it...
 * Ethernet amiga driver is now compiled with gcc. Thanks to @Stefan R
 * As the ARM code has been growing a lot, and I needed some more state machines. So I have been translating almost all switch case structures to the pt_thread library
 * Restored the initialization of the LTC chip, so now all analog measures are done.
 * Added to the ARM boot, the ZTop app (you need to press 'Z' after power ON). It seems an Amiga app, but it is not... :)
 * Added an icon to the ZTop app, that has the look of the Z3660 logo. 

##v1.03 beta 6
 * [EMU] Restored mother board RAM access
 * [EMU] Defaulting bank adresses with no RAM zones to invalid zones
 * [ZTOP ARM] Fix all resolutions (only 800x600 was tested before). 

##v1.03 beta 7
 * Restored Mother Board RAM access on EMU.
 * Added to env and Ztop ARM the configuration of MAC address in a new tab. (Amiga ZTop hasn't this new tab).
 * Slightly modified the ethernet driver (Amiga and ARM), with double space for incoming frames, and some fix so it seems that it doesn't stops suddenly
 * Added some more video modes (1280x800, 1920x1200 not tested, 1600x900 and 1680x1050). Added a p96 configuration file "p96-z3660-hdmi". Monitor file points to this file in devs:
 * Reworked the IPL read interrupt (it could interfere with other read/writes to other ARM registers). 

##v1.03 beta 8
 * Added more video resolutions 1280x800 1920x1200 1600x900 1680x1050
 * CPU frequency is now selectable between 50 and 100 MHz in 5 MHz steps. (Please report, as I only tested all frequencies on AA3000 and mobo SCSI)

##v1.03 beta 9
 * Added MHI support to OGG and FLAC files (using miniaudio library). AmigaAMP needs 1/2secs of buffer to play ogg files. Also ogg files are fully scanned so they are delayed when startinng playing (probably MHI or AmigaAMP issue)
 * Added a patch to the httpclient autoupdate code (thanks to @apolkosnik ). Under test. 

##v1.03 beta 10
 * Now the RTG autoconfig has two options (selected by a new checkbox in ZTop's Boot tab):
    Option 1 checkbox "AUTOC RTG enabled" checked. This configuration is as it was before this beta 10 (RTG is mapped first, before any autoconfig board, )
    Option 2 checkbox "AUTOC RTG enabled" NOT checked. This will make RTG to appear at $10000000, but it will not make z3660 scsi bootable. For making z3660 scsi bootable, you will need the ext rom kick060_scsi.rom. This will create a tiny Zorro II board of 64 kB, and it will make the z3660 scsi bootable.
 * Hopefully update system from "ARM console" (press 'C' after power on your Amiga) will work finally after beta 10... (thanks to @apolkosnik )

##v1.03 beta 11
 * Fix in the scsi refresh (used when resetting with 060, it shouldn't throw more ARM Data Aborts)
 * Beeper simulates SCSI activity sound. Configurable with ARM Ztop (Amiga Ztop still under development)
 * Changed the timing constraints in the FPGA. Please report your working CPU/EMU bus frequencies.
 * Env files will be replaced by Presets files (only for ARM Ztop). Please see my 4 preset examples I have included in the exFAT_files_and_folders.zip file.
 * Musashi should work again. 
 * I'm still investigating some weird reset made by UAEJIT Emulator (It tries to access to the address 0x6f043004, but in my system in that space, there is nothing. After the reboot, it works fine, so I suspect something about ARM CACHE). 

##v1.03 beta 12
 * A new hardware version (0.22) has been developed. It contains a cpld programmer using the ARM via I2C. For those with 0.21x there is a small PCB that uses two jumpers JP2 and JP1 as I2C pins.
 * A new preset has been added to ARM ZTop for using the z3660cfd.txt file as default.
 * SCSI emulation has been improved so it can make accessible a SD partition to the Amiga. As other systems make (pistorm/WinUAE/...) it will mount 0x76 partitions as a LUN. Normally, now you will have a BOOT FAT partition, hdfs in an exFAT partition, and now a new 0x76 partition. Then the SD will have 3 partitions, the last one will be mounted as SCSI unit 2 LUN 1 (internally unit number 12)
WARNING: make backup of your SDs, I have seen that the bug that makes hdfs to grow is back... 

##v1.03 beta 13
 * Reworked the CPLD programmer for v021x hardware with a tiny delay between I2C transitions.
 * CPLD programmer support for 0.v22 hardware
 * SCSI SD emulation now renames the hdfs/sd partition names if they exists previously in the system (for example from scsi.device).
 * Fix in Ethernet buffers so now it will be faster when receiving data.

##v1.03 beta 14
 * FIX: SCSI SD emulation now doesn't make the hdfs to grow up...
 * Added to ZTop for Amiga the tabs MISC and PRESET. (The mac address still CAN'T be modified, it will in a new version).
 * Added to ZTop (both ARM and Amiga) a button to delete the selected preset (it will delete the current selected preset, reboot the machine and select the default preset).
 * Temporarily fixed a bug in the EMU write words, that make CLUT video modes (8 bit, 256 colors) to not see some icons (for example in WBDOCK2). It is not fixed really, what I do is not use UAE direct access from JIT for writing words (16 bit). It makes this accesses slower. This will be in this way until I discover a JIT compiler solution for this... 
 