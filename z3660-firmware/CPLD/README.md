# Z3660 CPLD

Two versions here:

 * <b>DMA_WIP</b> is a "work in progress" (WIP) version, which works with Z3 DMA masters. Right now has some problems to boot, as it may need some reset before boot (maybe even power off-on cycles). This version only works with real cpus. CPU emulation has nothing to do with DMA (need some missing functions on emulation and a lot of test), so don't even try it...

 * <b>MASTER_no_DMA</b> version works with both, 060 and CPU emulation. Real cpu should be a 060 rev 6 as all firmware here is working only at 100 MHz. I have tried other frequencies, but only some "magic" frequencies work. ZTop app (z3660-drivers) will have some kind of frequency adjusment but right now this adjusment is internally disabled in this application. WIP.
 
Version History:
 * <b>v1.01</b>: First "official" version
 * <b>v1.02</b>: Update no_DMA version to get close to DMA branch (but still is a no_DMA version).
<br>
<br>

Important Note:
<br>
<br>You <b><i>should</i></b> use the vX.XX version with the same version numbers of BOOT.bin and drivers!!!