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
-hw {C:\Users\shanshe\workspace\Z3660_export\design_1_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {C:/Users/shanshe/workspace}

platform write
platform generate -domains 
domain create -name {standalone_ps7_cortexa9_1} -os {standalone} -proc {ps7_cortexa9_1} -arch {32-bit} -display-name {standalone on ps7_cortexa9_1} -desc {} -runtime {cpp}
platform generate -domains 
platform write
domain -report -json
domain active {standalone_domain}
bsp reload
bsp setlib -name xilffs -ver 5.1
bsp write
bsp reload
catch {bsp regenerate}
platform generate
bsp config enable_multi_partition "false"
bsp config enable_multi_partition "true"
bsp config enable_exfat "true"
bsp config use_lfn "1"
bsp config use_strfunc "1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {design_1_wrapper}
domain active {zynq_fsbl}
bsp reload
domain active {standalone_domain}
bsp reload
domain active {standalone_ps7_cortexa9_1}
bsp reload
domain active {standalone_domain}
bsp reload
platform generate -domains standalone_domain 
platform clean
platform generate
bsp reload
bsp config use_mkfs "false"
bsp write
bsp reload
catch {bsp regenerate}
platform active {design_1_wrapper}
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp reload
bsp reload
domain active {standalone_domain}
bsp reload
platform generate
platform clean
platform generate
platform generate -domains standalone_domain 
platform generate
platform clean
platform generate
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains standalone_domain,standalone_ps7_cortexa9_1,zynq_fsbl 
platform generate -domains standalone_domain,standalone_ps7_cortexa9_1,zynq_fsbl 
platform generate -domains standalone_domain,standalone_ps7_cortexa9_1,zynq_fsbl 
platform generate -domains standalone_domain,standalone_ps7_cortexa9_1,zynq_fsbl 
platform generate -domains standalone_ps7_cortexa9_1 
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform generate -domains zynq_fsbl 
platform active {design_1_wrapper}
bsp reload
bsp setlib -name lwip213 -ver 1.1
bsp config api_mode "RAW_API"
bsp write
bsp reload
catch {bsp regenerate}
platform clean
platform generate
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {standalone_domain}
bsp config api_mode "RAW_API"
bsp config socket_mode_thread_prio "2"
bsp reload
bsp reload
platform generate -domains 
bsp config lwip_dhcp "true"
bsp write
bsp reload
catch {bsp regenerate}
platform clean
platform generate
platform generate -domains standalone_domain 
platform generate -domains standalone_domain,standalone_ps7_cortexa9_1,zynq_fsbl 
platform generate -domains standalone_domain 
platform active {design_1_wrapper}
domain active {zynq_fsbl}
bsp reload
bsp reload
domain active {standalone_domain}
bsp reload
bsp config use_axieth_on_zynq "0"
bsp config use_emaclite_on_zynq "0"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
bsp config pbuf_pool_bufsize "9000"
bsp config tcp_wnd "65535"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform clean
platform generate
platform clean
platform generate
platform clean
platform generate
platform clean
platform active {design_1_wrapper}
platform generate
platform clean
platform generate
platform clean
platform generate
platform active {design_1_wrapper}
bsp reload
bsp reload
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp reload
platform generate -domains 
domain active {standalone_domain}
bsp reload
bsp config tcp_debug "true"
bsp config sys_debug "true"
bsp config socket_debug "true"
bsp config pbuf_debug "true"
bsp config netif_debug "true"
bsp config lwip_debug "true"
bsp config ip_debug "true"
bsp config igmp_debug "true"
bsp config icmp_debug "true"
bsp config udp_debug "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
bsp config icmp_debug "false"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
bsp reload
platform clean
platform generate
platform generate
platform clean
platform generate
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform active {design_1_wrapper}
domain active {zynq_fsbl}
domain active {standalone_domain}
bsp reload
bsp reload
bsp config udp_debug "false"
bsp config tcp_debug "false"
bsp config pbuf_debug "false"
bsp config igmp_debug "false"
bsp config ip_debug "false"
bsp config sys_debug "false"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate
platform generate
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform active {design_1_wrapper}
bsp reload
bsp config socket_debug "false"
bsp config lwip_debug "false"
bsp config netif_debug "false"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform generate
platform generate
platform config -updatehw {C:/Users/shanshe/workspace/Z3660_export/design_1_wrapper.xsa}
platform clean
platform generate
platform generate -domains standalone_domain 
platform clean
platform generate
