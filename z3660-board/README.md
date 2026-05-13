# Z3660 PCB

In this folder are the fabrication files (Gerber) I have sent to JLCPCB.

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
 * 01/08/2024: Uploaded BOM and CPL files for JLCPCB (thanks to @kavanoz64).
 
 v0.21b
 * 09/02/2024: Updated 060 footprint to 18x18 array pins (full 4 rows and columns).
 
 v0.21c
 * Invalid version - this was a misnamed upload during v0.21a release and should be disregarded.
 
 v.021d
 * 03/03/2024 Changed the footprints for two fans to 3-pin right-angle connectors.
 * 04/10/2024 Z3660_CPLD_prog is a tiny PCB to provide a CPLD programmer for v0.21x versions.
 
 v.022
 * 04/10/2024 Added an I2C gpio expander (PCF8574) to program the CPLD from the ARM.
 * 04/10/2024 Added optional onboard voltage regulator based on torsti76's <span>&#181;</span>VRM v0.1 design.
 
 v.023 (Contributed by @kavanoz)
 * 05/13/2026 Connected BOSS to DBG3 to allow motherboard CPU option.
 * 05/13/2026 Moved ZTurn 5mm towards the 060.
 * 05/13/2026 Removed voltage detector alternatives to save space for moving components.
 * 05/13/2026 Added a shunt for 3.34V solder jumper to set it as default. It needs to be cut before changing the onboard voltage regulator output.
 