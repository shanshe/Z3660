onbreak {quit -f}
onerror {quit -f}

vsim -voptargs="+acc"  -L xilinx_vip -L xpm -L axi_infrastructure_v1_1_0 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xil_defaultlib -L lib_cdc_v1_0_2 -L proc_sys_reset_v5_0_14 -L lib_pkg_v1_0_3 -L fifo_generator_v13_2_9 -L lib_fifo_v1_0_18 -L blk_mem_gen_v8_4_7 -L lib_bmg_v1_0_16 -L lib_srl_fifo_v1_0_3 -L axi_datamover_v5_1_31 -L axi_vdma_v6_3_17 -L axis_infrastructure_v1_1_1 -L axis_data_fifo_v2_0_11 -L util_vector_logic_v2_0_3 -L xlconcat_v2_1_5 -L xlslice_v1_0_3 -L audio_formatter_v1_0_11 -L axi_register_slice_v2_1_29 -L axis_register_slice_v1_1_29 -L i2s_transmitter_v1_0_7 -L generic_baseblocks_v2_1_1 -L axi_data_fifo_v2_1_28 -L axi_protocol_converter_v2_1_29 -L axi_clock_converter_v2_1_28 -L axi_dwidth_converter_v2_1_29 -L axi_crossbar_v2_1_30 -L xilinx_vip -L unisims_ver -L unimacro_ver -L secureip -lib xil_defaultlib xil_defaultlib.design_1 xil_defaultlib.glbl

set NumericStdNoWarnings 1
set StdArithNoWarnings 1

do {wave.do}

view wave
view structure
view signals

do {design_1.udo}

run 1000ns

quit -force
