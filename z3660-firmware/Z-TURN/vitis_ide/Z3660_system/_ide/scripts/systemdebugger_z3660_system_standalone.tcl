# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\Users\shanshe\workspace\Z3660_system\_ide\scripts\systemdebugger_z3660_system_standalone.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\Users\shanshe\workspace\Z3660_system\_ide\scripts\systemdebugger_z3660_system_standalone.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/shanshe/workspace/design_1_wrapper/export/design_1_wrapper/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
stop
source C:/Users/shanshe/workspace/Z3660/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
rst -processor
targets -set -nocase -filter {name =~ "*A9*#1"}
rst -processor
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/Users/shanshe/workspace/Z3660/Debug/Z3660.elf
targets -set -nocase -filter {name =~ "*A9*#1"}
dow C:/Users/shanshe/workspace/Z3660_emu/Debug/Z3660_emu.elf
configparams force-mem-access 0
bpadd -addr &main
