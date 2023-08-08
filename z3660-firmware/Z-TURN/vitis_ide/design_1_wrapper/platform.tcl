# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\shanshe\workspace\design_1_wrapper\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\shanshe\workspace\design_1_wrapper\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {design_1_wrapper}\
-hw {C:\Users\shanshe\OneDrive\Escritorio\z3660\design_1_wrapper.xsa}\
-out {C:/Users/shanshe/workspace}

platform write
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform active {design_1_wrapper}
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
platform active {design_1_wrapper}
domain create -name {standalone_ps7_cortexa9_1} -display-name {standalone_ps7_cortexa9_1} -os {standalone} -proc {ps7_cortexa9_1} -runtime {cpp} -arch {32-bit} -support-app {empty_application}
platform generate -domains 
platform write
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
domain active {standalone_ps7_cortexa9_1}
platform generate -quick
bsp reload
domain active {standalone_ps7_cortexa9_0}
bsp reload
bsp setlib -name xilffs -ver 5.0
bsp config use_strfunc "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform clean
platform generate
domain active {zynq_fsbl}
bsp reload
bsp reload
bsp config use_strfunc "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains zynq_fsbl 
platform clean
platform generate
platform generate -domains standalone_ps7_cortexa9_0 
bsp config use_strfunc "0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains zynq_fsbl 
domain active {standalone_ps7_cortexa9_0}
bsp config use_lfn "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_0 
bsp reload
domain active {zynq_fsbl}
bsp reload
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp reload
platform clean
platform generate
platform generate -domains standalone_ps7_cortexa9_0 
domain active {standalone_ps7_cortexa9_0}
bsp reload
platform generate
platform active {design_1_wrapper}
domain active {zynq_fsbl}
bsp reload
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -nostartfiles -g -Wall -Wextra"
bsp write
bsp reload
catch {bsp regenerate}
domain active {standalone_ps7_cortexa9_0}
platform generate -domains standalone_ps7_cortexa9_1 
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -nostartfiles -g -Wall -Wextra"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_0 
