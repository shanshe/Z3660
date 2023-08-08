# Z3660 CPLD

Two versions here:

 * <b>DMA_WIP</b> is a "work in progress" (WIP) version, which works with Z3 DMA masters. Right now has some problems to boot, as it may need some reset before boot (maybe even power off-on cycles). CPU emulation has nothing to do with DMA (need some missing functions on emulation), so don't try it...

 * <b>MASTER_no_DMA</b> version works with both, 060 and CPU emulation.