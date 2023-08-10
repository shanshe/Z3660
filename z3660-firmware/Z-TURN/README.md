# Z3660 Z-Turn

To build the file BOOT.bin, I am using Vitis 2023 (Vivado for RTL and Vitis IDE for ARMs).
<br>* You should import Block Design from vivado/design_1.tcl file.
<br>* Generate Bitstream.
<br>* Export hardware (Include bitstream).
<br>* Launch Vitis IDE (from Tools).
<br>* Choose as workspace the directory vitis_ide
<br>* Build all.
<br>* Build Z3660_system in order to create Boot Image.
<br>The "BOOT.bin" file (Z3660_system/DEBUG/sd_card directory) must be copied to the root of your Z-turn SD (first FAT32 partition).