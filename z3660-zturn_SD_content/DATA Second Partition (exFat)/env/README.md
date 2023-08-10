# ENV DIRECTORY
Like Amiga OS ENV: directory, here you can config some features of the Z3660 accelerator using some text files that overrides what you have selected in the z3660cfg.txt file. These files can be updated by ZTop Amiga application, or manually.

Right now you can configure these features:

Do you want Z3 RAM expansion? You should write NO if you use some Z3 DMA card.

`autoconfig_ram: YES or NO`

Choose your boot mode, between real cpu (CPU) or emulator (MUSASHI, UAE or UAEJIT). 

`bootmode: CPU, MUSASHI, UAE or UAEJIT`

Do you want to boot from SD scsi emulation? When using a real CPU you should select here NO (still in development)

`scsiboot: YES or NO.`