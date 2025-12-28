#set dir [pwd]
#puts "Directory $dir"
set verilog_file "../../z3660.srcs/sources_1/new/version.vh"
set fp [open $verilog_file r]
set content [read $fp]
close $fp

set version_regex {localparam\s+SYNTHESIS_VERS\s*=\s*8'h([0-9a-fA-F]+)}
if {[regexp $version_regex $content match version]} {

	set decimal_version [expr "0x$version"]
	set new_decimal_version [expr $decimal_version + 1]
	set new_hex_version [format "8'h%02X" $new_decimal_version]
	set updated_content [regsub -all $version_regex $content "localparam SYNTHESIS_VERS = $new_hex_version"]

#puts "version = 8'h$version -> $new_hex_version"
	
	set fp [open $verilog_file w]
	puts $fp $updated_content
	close $fp
	puts "Version number incremented to $new_hex_version en $verilog_file"
} else {
	puts "Error: Version number not found"
}
#update_module_reference design_1_z3660_0_0