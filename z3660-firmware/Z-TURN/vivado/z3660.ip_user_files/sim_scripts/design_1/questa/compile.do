vlib questa_lib/work
vlib questa_lib/msim

vlib questa_lib/msim/xilinx_vip
vlib questa_lib/msim/xpm
vlib questa_lib/msim/axi_infrastructure_v1_1_0
vlib questa_lib/msim/axi_vip_v1_1_15
vlib questa_lib/msim/processing_system7_vip_v1_0_17
vlib questa_lib/msim/xil_defaultlib
vlib questa_lib/msim/lib_cdc_v1_0_2
vlib questa_lib/msim/proc_sys_reset_v5_0_14
vlib questa_lib/msim/lib_pkg_v1_0_3
vlib questa_lib/msim/fifo_generator_v13_2_9
vlib questa_lib/msim/lib_fifo_v1_0_18
vlib questa_lib/msim/blk_mem_gen_v8_4_7
vlib questa_lib/msim/lib_bmg_v1_0_16
vlib questa_lib/msim/lib_srl_fifo_v1_0_3
vlib questa_lib/msim/axi_datamover_v5_1_31
vlib questa_lib/msim/axi_vdma_v6_3_17
vlib questa_lib/msim/axis_infrastructure_v1_1_1
vlib questa_lib/msim/axis_data_fifo_v2_0_11
vlib questa_lib/msim/util_vector_logic_v2_0_3
vlib questa_lib/msim/xlconcat_v2_1_5
vlib questa_lib/msim/xlslice_v1_0_3
vlib questa_lib/msim/audio_formatter_v1_0_11
vlib questa_lib/msim/axi_register_slice_v2_1_29
vlib questa_lib/msim/axis_register_slice_v1_1_29
vlib questa_lib/msim/i2s_transmitter_v1_0_7
vlib questa_lib/msim/generic_baseblocks_v2_1_1
vlib questa_lib/msim/axi_data_fifo_v2_1_28
vlib questa_lib/msim/axi_protocol_converter_v2_1_29
vlib questa_lib/msim/axi_clock_converter_v2_1_28
vlib questa_lib/msim/axi_dwidth_converter_v2_1_29
vlib questa_lib/msim/axi_crossbar_v2_1_30

vmap xilinx_vip questa_lib/msim/xilinx_vip
vmap xpm questa_lib/msim/xpm
vmap axi_infrastructure_v1_1_0 questa_lib/msim/axi_infrastructure_v1_1_0
vmap axi_vip_v1_1_15 questa_lib/msim/axi_vip_v1_1_15
vmap processing_system7_vip_v1_0_17 questa_lib/msim/processing_system7_vip_v1_0_17
vmap xil_defaultlib questa_lib/msim/xil_defaultlib
vmap lib_cdc_v1_0_2 questa_lib/msim/lib_cdc_v1_0_2
vmap proc_sys_reset_v5_0_14 questa_lib/msim/proc_sys_reset_v5_0_14
vmap lib_pkg_v1_0_3 questa_lib/msim/lib_pkg_v1_0_3
vmap fifo_generator_v13_2_9 questa_lib/msim/fifo_generator_v13_2_9
vmap lib_fifo_v1_0_18 questa_lib/msim/lib_fifo_v1_0_18
vmap blk_mem_gen_v8_4_7 questa_lib/msim/blk_mem_gen_v8_4_7
vmap lib_bmg_v1_0_16 questa_lib/msim/lib_bmg_v1_0_16
vmap lib_srl_fifo_v1_0_3 questa_lib/msim/lib_srl_fifo_v1_0_3
vmap axi_datamover_v5_1_31 questa_lib/msim/axi_datamover_v5_1_31
vmap axi_vdma_v6_3_17 questa_lib/msim/axi_vdma_v6_3_17
vmap axis_infrastructure_v1_1_1 questa_lib/msim/axis_infrastructure_v1_1_1
vmap axis_data_fifo_v2_0_11 questa_lib/msim/axis_data_fifo_v2_0_11
vmap util_vector_logic_v2_0_3 questa_lib/msim/util_vector_logic_v2_0_3
vmap xlconcat_v2_1_5 questa_lib/msim/xlconcat_v2_1_5
vmap xlslice_v1_0_3 questa_lib/msim/xlslice_v1_0_3
vmap audio_formatter_v1_0_11 questa_lib/msim/audio_formatter_v1_0_11
vmap axi_register_slice_v2_1_29 questa_lib/msim/axi_register_slice_v2_1_29
vmap axis_register_slice_v1_1_29 questa_lib/msim/axis_register_slice_v1_1_29
vmap i2s_transmitter_v1_0_7 questa_lib/msim/i2s_transmitter_v1_0_7
vmap generic_baseblocks_v2_1_1 questa_lib/msim/generic_baseblocks_v2_1_1
vmap axi_data_fifo_v2_1_28 questa_lib/msim/axi_data_fifo_v2_1_28
vmap axi_protocol_converter_v2_1_29 questa_lib/msim/axi_protocol_converter_v2_1_29
vmap axi_clock_converter_v2_1_28 questa_lib/msim/axi_clock_converter_v2_1_28
vmap axi_dwidth_converter_v2_1_29 questa_lib/msim/axi_dwidth_converter_v2_1_29
vmap axi_crossbar_v2_1_30 questa_lib/msim/axi_crossbar_v2_1_30

vlog -work xilinx_vip  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi4stream_vip_axi4streampc.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi_vip_axi4pc.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/xil_common_vip_pkg.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi4stream_vip_pkg.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi_vip_pkg.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi4stream_vip_if.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/axi_vip_if.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/clk_vip_if.sv" \
"C:/Xilinx/Vivado/2023.2/data/xilinx_vip/hdl/rst_vip_if.sv" \

vlog -work xpm  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"C:/Xilinx/Vivado/2023.2/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"C:/Xilinx/Vivado/2023.2/data/ip/xpm/xpm_fifo/hdl/xpm_fifo.sv" \
"C:/Xilinx/Vivado/2023.2/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -work xpm  -93  \
"C:/Xilinx/Vivado/2023.2/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -work axi_infrastructure_v1_1_0  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl/axi_infrastructure_v1_1_vl_rfs.v" \

vlog -work axi_vip_v1_1_15  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/5753/hdl/axi_vip_v1_1_vl_rfs.sv" \

vlog -work processing_system7_vip_v1_0_17  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl/processing_system7_vip_v1_0_vl_rfs.sv" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_processing_system7_0_0/sim/design_1_processing_system7_0_0.v" \

vcom -work lib_cdc_v1_0_2  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ef1e/hdl/lib_cdc_v1_0_rfs.vhd" \

vcom -work proc_sys_reset_v5_0_14  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/408c/hdl/proc_sys_reset_v5_0_vh_rfs.vhd" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_proc_sys_reset_1_0/sim/design_1_proc_sys_reset_1_0.vhd" \
"../../../bd/design_1/ip/design_1_rst_ps7_0_200M_1_1/sim/design_1_rst_ps7_0_200M_1_1.vhd" \

vcom -work lib_pkg_v1_0_3  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/56d9/hdl/lib_pkg_v1_0_rfs.vhd" \

vlog -work fifo_generator_v13_2_9  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ac72/simulation/fifo_generator_vlog_beh.v" \

vcom -work fifo_generator_v13_2_9  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ac72/hdl/fifo_generator_v13_2_rfs.vhd" \

vlog -work fifo_generator_v13_2_9  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ac72/hdl/fifo_generator_v13_2_rfs.v" \

vcom -work lib_fifo_v1_0_18  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/1531/hdl/lib_fifo_v1_0_rfs.vhd" \

vlog -work blk_mem_gen_v8_4_7  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/3c0c/simulation/blk_mem_gen_v8_4.v" \

vcom -work lib_bmg_v1_0_16  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/5c9c/hdl/lib_bmg_v1_0_rfs.vhd" \

vcom -work lib_srl_fifo_v1_0_3  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/02c4/hdl/lib_srl_fifo_v1_0_rfs.vhd" \

vcom -work axi_datamover_v5_1_31  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/d786/hdl/axi_datamover_v5_1_vh_rfs.vhd" \

vlog -work axi_vdma_v6_3_17  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl/axi_vdma_v6_3_rfs.v" \

vcom -work axi_vdma_v6_3_17  -93  \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl/axi_vdma_v6_3_rfs.vhd" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_axi_vdma_0_1/sim/design_1_axi_vdma_0_1.vhd" \

vlog -work axis_infrastructure_v1_1_1  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl/axis_infrastructure_v1_1_vl_rfs.v" \

vlog -work axis_data_fifo_v2_0_11  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/68dc/hdl/axis_data_fifo_v2_0_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_axis_data_fifo_0_0/sim/design_1_axis_data_fifo_0_0.v" \

vlog -work util_vector_logic_v2_0_3  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/5e7b/hdl/util_vector_logic_v2_0_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_util_vector_logic_0_1/sim/design_1_util_vector_logic_0_1.v" \

vlog -work xlconcat_v2_1_5  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/147b/hdl/xlconcat_v2_1_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_xlconcat_0_1/sim/design_1_xlconcat_0_1.v" \

vlog -work xlslice_v1_0_3  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/217a/hdl/xlslice_v1_0_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_xlslice_0_0/sim/design_1_xlslice_0_0.v" \
"../../../bd/design_1/ip/design_1_video_formatter_0_0/sim/design_1_video_formatter_0_0.v" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_rst_ps7_0_200M_1_2/sim/design_1_rst_ps7_0_200M_1_2.vhd" \

vlog -work audio_formatter_v1_0_11  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/d22b/hdl/audio_formatter_v1_0_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_audio_formatter_0_0/sim/design_1_audio_formatter_0_0.v" \

vlog -work axi_register_slice_v2_1_29  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/ff9f/hdl/axi_register_slice_v2_1_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_axi_register_slice_0_0_1/sim/design_1_axi_register_slice_0_0.v" \

vlog -work axis_register_slice_v1_1_29  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/af18/hdl/axis_register_slice_v1_1_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_axis_register_slice_0_0/sim/design_1_axis_register_slice_0_0.v" \

vlog -work i2s_transmitter_v1_0_7  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/bf6a/hdl/i2s_transmitter_v1_0_rfs.sv" \

vlog -work xil_defaultlib  -incr -mfcu  -sv -L i2s_transmitter_v1_0_7 -L axi_vip_v1_1_15 -L processing_system7_vip_v1_0_17 -L xilinx_vip "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_i2s_transmitter_0_0/sim/design_1_i2s_transmitter_0_0.sv" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/design_1_clk_wiz_0_0_mmcm_pll_drp.v" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_conv_funs_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_proc_common_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_ipif_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_family_support.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_family.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_soft_reset.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_0_0_pselect_f.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_0_0_address_decoder.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_0_0_slave_attachment.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_0_0_axi_lite_ipif.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/design_1_clk_wiz_0_0_clk_wiz_drp.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/design_1_clk_wiz_0_0_axi_clk_config.vhd" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/design_1_clk_wiz_0_0_clk_wiz.v" \
"../../../bd/design_1/ip/design_1_clk_wiz_0_0/design_1_clk_wiz_0_0.v" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/design_1_clk_wiz_2_0_mmcm_pll_drp.v" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_conv_funs_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_proc_common_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_ipif_pkg.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_family_support.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_family.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_soft_reset.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/proc_common_v3_00_a/hdl/src/vhdl/design_1_clk_wiz_2_0_pselect_f.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_2_0_address_decoder.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_2_0_slave_attachment.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_clk_wiz_2_0_axi_lite_ipif.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/design_1_clk_wiz_2_0_clk_wiz_drp.vhd" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/design_1_clk_wiz_2_0_axi_clk_config.vhd" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/design_1_clk_wiz_2_0_clk_wiz.v" \
"../../../bd/design_1/ip/design_1_clk_wiz_2_0/design_1_clk_wiz_2_0.v" \
"../../../bd/design_1/ip/design_1_clk_wiz_3_0/design_1_clk_wiz_3_0_clk_wiz.v" \
"../../../bd/design_1/ip/design_1_clk_wiz_3_0/design_1_clk_wiz_3_0.v" \
"../../../bd/design_1/ip/design_1_cpuclk_iob_0_0/sim/design_1_cpuclk_iob_0_0.v" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_rst_ps7_0_200M_0/sim/design_1_rst_ps7_0_200M_0.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_conv_funs_pkg.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_proc_common_pkg.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_ipif_pkg.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_family_support.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_family.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_soft_reset.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/proc_common_v3_30_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_pselect_f.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_address_decoder.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_slave_attachment.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/interrupt_control_v2_01_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_interrupt_control.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/axi_lite_ipif_v1_01_a/hdl/src/vhdl/design_1_xadc_wiz_0_0_axi_lite_ipif.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/design_1_xadc_wiz_0_0_xadc_core_drp.vhd" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/design_1_xadc_wiz_0_0_axi_xadc.vhd" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_xadc_wiz_0_0_1/design_1_xadc_wiz_0_0.v" \

vcom -work xil_defaultlib  -93  \
"../../../bd/design_1/ip/design_1_proc_sys_reset_0_0/sim/design_1_proc_sys_reset_0_0.vhd" \

vlog -work generic_baseblocks_v2_1_1  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/10ab/hdl/generic_baseblocks_v2_1_vl_rfs.v" \

vlog -work axi_data_fifo_v2_1_28  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/279e/hdl/axi_data_fifo_v2_1_vl_rfs.v" \

vlog -work axi_protocol_converter_v2_1_29  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/a63f/hdl/axi_protocol_converter_v2_1_vl_rfs.v" \

vlog -work axi_clock_converter_v2_1_28  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/769c/hdl/axi_clock_converter_v2_1_vl_rfs.v" \

vlog -work axi_dwidth_converter_v2_1_29  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/670d/hdl/axi_dwidth_converter_v2_1_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_axi_dwidth_converter_0_0_1/sim/design_1_axi_dwidth_converter_0_0.v" \
"../../../bd/design_1/ip/design_1_axi_protocol_convert_0_0/sim/design_1_axi_protocol_convert_0_0.v" \
"../../../bd/design_1/ip/design_1_axi_register_slice_0_1/sim/design_1_axi_register_slice_0_1.v" \
"../../../bd/design_1/ip/design_1_axi_register_slice_1_0/sim/design_1_axi_register_slice_1_0.v" \
"../../../bd/design_1/ip/design_1_xlconcat_0_2/sim/design_1_xlconcat_0_2.v" \
"../../../bd/design_1/ip/design_1_z3660_0_0/sim/design_1_z3660_0_0.v" \

vlog -work axi_crossbar_v2_1_30  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../../z3660.gen/sources_1/bd/design_1/ipshared/fb47/hdl/axi_crossbar_v2_1_vl_rfs.v" \

vlog -work xil_defaultlib  -incr -mfcu  "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/ec67/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/6b2b/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/7fb4/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/434f/hdl" "+incdir+../../../../z3660.gen/sources_1/bd/design_1/ipshared/c2c6" "+incdir+C:/Xilinx/Vivado/2023.2/data/xilinx_vip/include" \
"../../../bd/design_1/ip/design_1_xbar_0/sim/design_1_xbar_0.v" \
"../../../bd/design_1/ip/design_1_s00_regslice_128/sim/design_1_s00_regslice_128.v" \
"../../../bd/design_1/ip/design_1_auto_pc_0/sim/design_1_auto_pc_0.v" \
"../../../bd/design_1/ip/design_1_s00_regslice_127/sim/design_1_s00_regslice_127.v" \
"../../../bd/design_1/ip/design_1_auto_cc_0/sim/design_1_auto_cc_0.v" \
"../../../bd/design_1/ip/design_1_auto_pc_1/sim/design_1_auto_pc_1.v" \
"../../../bd/design_1/ip/design_1_s00_regslice_129/sim/design_1_s00_regslice_129.v" \
"../../../bd/design_1/ip/design_1_auto_pc_2/sim/design_1_auto_pc_2.v" \
"../../../bd/design_1/sim/design_1.v" \

vlog -work xil_defaultlib \
"glbl.v"

