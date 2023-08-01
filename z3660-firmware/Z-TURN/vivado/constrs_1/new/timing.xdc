#set_false_path -from [get_clocks clk_fpga_1] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
#set_false_path -from [get_clocks clk_fpga_1] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_fpga_1]

#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_fpga_2]
#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT1]] -to [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_0/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]]
#set_false_path -from [get_pins {design_1_i/z3660_0/inst/GPIO_c_reg[*]/C}] -to [get_pins {design_1_i/z3660_0/inst/RAM_state_reg[*]/D}]

#set_false_path -from [get_pins design_1_i/processing_av_system/audio_video_engine/video/video_formatter_0/inst/sprite_dbl_reg/C] -to [get_pins {design_1_i/processing_av_system/audio_video_engine/video/video_formatter_0/inst/need_line_fetch_reg2_reg[1]/D}]
#set_false_path -from [get_clocks clk_fpga_1] -to [get_clocks clk_fpga_2]

#create_clock -period 7.000 -name hdmi_clk -waveform {0.000 3.500} [get_nets hdmi_clk]
#set_output_delay -clock [get_clocks hdmi_clk] -min -5.000 [get_ports {hdmi_data[*]}]
#set_output_delay -clock [get_clocks hdmi_clk] -max 5.000 [get_ports {hdmi_data[*]}]
#set_output_delay -clock [get_clocks hdmi_clk] -min -5.000 [get_ports hdmi_de]
#set_output_delay -clock [get_clocks hdmi_clk] -max 5.000 [get_ports hdmi_de]
#set_output_delay -clock [get_clocks hdmi_clk] -min -5.000 [get_ports hdmi_hs]
#set_output_delay -clock [get_clocks hdmi_clk] -max 5.000 [get_ports hdmi_hs]
#set_output_delay -clock [get_clocks hdmi_clk] -min -5.000 [get_ports hdmi_vs]
#set_output_delay -clock [get_clocks hdmi_clk] -max 5.000 [get_ports hdmi_vs]
#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks hdmi_clk]

#set_false_path -from [get_clocks -of_objects [get_pins design_1_i/processing_av_system/clock_generation/clk_wiz_2/inst/CLK_CORE_DRP_I/clk_inst/mmcm_adv_inst/CLKOUT0]] -to [get_clocks clk_fpga_2]


