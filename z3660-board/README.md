# Z3660 PCB

In this folder you the fabrication files (gerber) I have sent to JLCPCB.

I have built the first prototype of version v0.21 and it is working as expected.

# BOM revisions
v0.21
 * 08/04/2023: First version uploaded.

 * 08/11/2023: C27 capacitor has been removed from BOM because it is under Z-turn and there is no enough space for an electrolytic capacitor. You can use another 10uF SMD capacitor, or simply ignore it, we have a lot of capacitors in this board...

 * 08/13/2023: Updated BOM with I2C LTC2990 (tested today). It measures 5V, 3V3 and 060 THERM resistor.

v0.21a
 * 01/07/2024: Updated resistor network footprints to 4x (instead of 8x).
 * 01/07/2024: Updated electrolytic capacitor footprints to bigger ones.
 * 01/08/2024: Updated led values to differentiate green and red.
 * 01/08/2024: Uploaded BOM and CPL files for JLCPCB (thanks to @kavanoz).