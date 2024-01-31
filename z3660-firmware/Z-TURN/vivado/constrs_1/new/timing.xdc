create_clock -period 6.734 -name VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1 -waveform {0.000 3.367}
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -min -add_delay 10.000 [get_ports {hdmi_data[*]}]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -max -add_delay 4.000 [get_ports {hdmi_data[*]}]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -min -add_delay 10.000 [get_ports hdmi_de]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -max -add_delay 4.000 [get_ports hdmi_de]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -min -add_delay 10.000 [get_ports hdmi_hs]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -max -add_delay 4.000 [get_ports hdmi_hs]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -min -add_delay 10.000 [get_ports hdmi_vs]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] -max -add_delay 4.000 [get_ports hdmi_vs]

create_clock -period 54.255 -name VIRTUAL_clk_out1_design_1_clk_wiz_3_0_1 -waveform {0.000 27.128}
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_3_0_1] -min -add_delay -1.000 [get_ports I2SO_D0]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_3_0_1] -max -add_delay 4.000 [get_ports I2SO_D0]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_3_0_1] -min -add_delay -1.000 [get_ports I2S_FSYNC_OUT]
set_output_delay -clock [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_3_0_1] -max -add_delay 4.000 [get_ports I2S_FSYNC_OUT]


set_false_path -from [get_clocks AXI_clk_design_1_clk_wiz_0_0] -to [get_clocks clk_fpga_1]

set_false_path -from [get_clocks clk_fpga_1] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
set_multicycle_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks VIRTUAL_clk_out1_design_1_clk_wiz_2_0_1] 3
set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_fpga_2]
set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_fpga_1]


set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
set_false_path -from [get_clocks clk_fpga_1] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT1]]
