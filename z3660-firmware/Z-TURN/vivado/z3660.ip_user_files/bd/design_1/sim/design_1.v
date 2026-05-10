//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2023.2 (win64) Build 4029153 Fri Oct 13 20:14:34 MDT 2023
//Date        : Mon Feb  9 15:18:03 2026
//Host        : CoreBox-X running 64-bit major release  (build 9200)
//Command     : generate_target design_1.bd
//Design      : design_1
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module audio_video_engine_imp_14WDLA5
   (I2SO_D0,
    I2S_FSYNC_OUT,
    I2S_SCLK,
    M00_ACLK,
    M00_AXI_araddr,
    M00_AXI_arburst,
    M00_AXI_arcache,
    M00_AXI_arlen,
    M00_AXI_arlock,
    M00_AXI_arprot,
    M00_AXI_arqos,
    M00_AXI_arready,
    M00_AXI_arsize,
    M00_AXI_arvalid,
    M00_AXI_rdata,
    M00_AXI_rlast,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    M_AXI_araddr,
    M_AXI_arburst,
    M_AXI_arcache,
    M_AXI_arlen,
    M_AXI_arlock,
    M_AXI_arprot,
    M_AXI_arqos,
    M_AXI_arready,
    M_AXI_arsize,
    M_AXI_arvalid,
    M_AXI_rdata,
    M_AXI_rlast,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    S_AXI_LITE_araddr,
    S_AXI_LITE_arready,
    S_AXI_LITE_arvalid,
    S_AXI_LITE_awaddr,
    S_AXI_LITE_awready,
    S_AXI_LITE_awvalid,
    S_AXI_LITE_bready,
    S_AXI_LITE_bresp,
    S_AXI_LITE_bvalid,
    S_AXI_LITE_rdata,
    S_AXI_LITE_rready,
    S_AXI_LITE_rresp,
    S_AXI_LITE_rvalid,
    S_AXI_LITE_wdata,
    S_AXI_LITE_wready,
    S_AXI_LITE_wvalid,
    S_AXI_araddr,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awprot,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid,
    aud_mclk,
    control_vblank,
    dout,
    ext_reset_in,
    hdmi_clk,
    hdmi_data,
    hdmi_de,
    hdmi_hs,
    hdmi_intn,
    hdmi_vs,
    s_axi_ctrl_aclk,
    s_axi_ctrl_araddr,
    s_axi_ctrl_aresetn,
    s_axi_ctrl_arready,
    s_axi_ctrl_arvalid,
    s_axi_ctrl_awaddr,
    s_axi_ctrl_awready,
    s_axi_ctrl_awvalid,
    s_axi_ctrl_bready,
    s_axi_ctrl_bresp,
    s_axi_ctrl_bvalid,
    s_axi_ctrl_rdata,
    s_axi_ctrl_rready,
    s_axi_ctrl_rresp,
    s_axi_ctrl_rvalid,
    s_axi_ctrl_wdata,
    s_axi_ctrl_wready,
    s_axi_ctrl_wvalid,
    s_axi_lite1_araddr,
    s_axi_lite1_arready,
    s_axi_lite1_arvalid,
    s_axi_lite1_awaddr,
    s_axi_lite1_awready,
    s_axi_lite1_awvalid,
    s_axi_lite1_bready,
    s_axi_lite1_bresp,
    s_axi_lite1_bvalid,
    s_axi_lite1_rdata,
    s_axi_lite1_rready,
    s_axi_lite1_rresp,
    s_axi_lite1_rvalid,
    s_axi_lite1_wdata,
    s_axi_lite1_wready,
    s_axi_lite1_wvalid,
    s_axis_aud_aclk,
    vid_clk);
  output I2SO_D0;
  output I2S_FSYNC_OUT;
  output I2S_SCLK;
  input M00_ACLK;
  output [31:0]M00_AXI_araddr;
  output [1:0]M00_AXI_arburst;
  output [3:0]M00_AXI_arcache;
  output [3:0]M00_AXI_arlen;
  output [1:0]M00_AXI_arlock;
  output [2:0]M00_AXI_arprot;
  output [3:0]M00_AXI_arqos;
  input M00_AXI_arready;
  output [2:0]M00_AXI_arsize;
  output M00_AXI_arvalid;
  input [31:0]M00_AXI_rdata;
  input M00_AXI_rlast;
  output M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input M00_AXI_rvalid;
  output [31:0]M_AXI_araddr;
  output [1:0]M_AXI_arburst;
  output [3:0]M_AXI_arcache;
  output [3:0]M_AXI_arlen;
  output [1:0]M_AXI_arlock;
  output [2:0]M_AXI_arprot;
  output [3:0]M_AXI_arqos;
  input M_AXI_arready;
  output [2:0]M_AXI_arsize;
  output M_AXI_arvalid;
  input [31:0]M_AXI_rdata;
  input M_AXI_rlast;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  input [31:0]S_AXI_LITE_araddr;
  output [0:0]S_AXI_LITE_arready;
  input [0:0]S_AXI_LITE_arvalid;
  input [31:0]S_AXI_LITE_awaddr;
  output [0:0]S_AXI_LITE_awready;
  input [0:0]S_AXI_LITE_awvalid;
  input [0:0]S_AXI_LITE_bready;
  output [1:0]S_AXI_LITE_bresp;
  output [0:0]S_AXI_LITE_bvalid;
  output [31:0]S_AXI_LITE_rdata;
  input [0:0]S_AXI_LITE_rready;
  output [1:0]S_AXI_LITE_rresp;
  output [0:0]S_AXI_LITE_rvalid;
  input [31:0]S_AXI_LITE_wdata;
  output [0:0]S_AXI_LITE_wready;
  input [0:0]S_AXI_LITE_wvalid;
  input [31:0]S_AXI_araddr;
  input [2:0]S_AXI_arprot;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [2:0]S_AXI_awprot;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input [0:0]S_AXI_wvalid;
  input aud_mclk;
  output [1:0]control_vblank;
  output [4:0]dout;
  input ext_reset_in;
  output hdmi_clk;
  output [15:0]hdmi_data;
  output hdmi_de;
  output hdmi_hs;
  input [0:0]hdmi_intn;
  output hdmi_vs;
  input s_axi_ctrl_aclk;
  input [31:0]s_axi_ctrl_araddr;
  input s_axi_ctrl_aresetn;
  output [0:0]s_axi_ctrl_arready;
  input [0:0]s_axi_ctrl_arvalid;
  input [31:0]s_axi_ctrl_awaddr;
  output [0:0]s_axi_ctrl_awready;
  input [0:0]s_axi_ctrl_awvalid;
  input [0:0]s_axi_ctrl_bready;
  output [1:0]s_axi_ctrl_bresp;
  output [0:0]s_axi_ctrl_bvalid;
  output [31:0]s_axi_ctrl_rdata;
  input [0:0]s_axi_ctrl_rready;
  output [1:0]s_axi_ctrl_rresp;
  output [0:0]s_axi_ctrl_rvalid;
  input [31:0]s_axi_ctrl_wdata;
  output [0:0]s_axi_ctrl_wready;
  input [0:0]s_axi_ctrl_wvalid;
  input [31:0]s_axi_lite1_araddr;
  output [0:0]s_axi_lite1_arready;
  input [0:0]s_axi_lite1_arvalid;
  input [31:0]s_axi_lite1_awaddr;
  output [0:0]s_axi_lite1_awready;
  input [0:0]s_axi_lite1_awvalid;
  input [0:0]s_axi_lite1_bready;
  output [1:0]s_axi_lite1_bresp;
  output [0:0]s_axi_lite1_bvalid;
  output [31:0]s_axi_lite1_rdata;
  input [0:0]s_axi_lite1_rready;
  output [1:0]s_axi_lite1_rresp;
  output [0:0]s_axi_lite1_rvalid;
  input [31:0]s_axi_lite1_wdata;
  output [0:0]s_axi_lite1_wready;
  input [0:0]s_axi_lite1_wvalid;
  input s_axis_aud_aclk;
  input vid_clk;

  wire [31:0]Conn2_ARADDR;
  wire Conn2_ARREADY;
  wire [0:0]Conn2_ARVALID;
  wire [31:0]Conn2_AWADDR;
  wire Conn2_AWREADY;
  wire [0:0]Conn2_AWVALID;
  wire [0:0]Conn2_BREADY;
  wire [1:0]Conn2_BRESP;
  wire Conn2_BVALID;
  wire [31:0]Conn2_RDATA;
  wire [0:0]Conn2_RREADY;
  wire [1:0]Conn2_RRESP;
  wire Conn2_RVALID;
  wire [31:0]Conn2_WDATA;
  wire Conn2_WREADY;
  wire [0:0]Conn2_WVALID;
  wire [31:0]Conn3_ARADDR;
  wire [1:0]Conn3_ARBURST;
  wire [3:0]Conn3_ARCACHE;
  wire [3:0]Conn3_ARLEN;
  wire [1:0]Conn3_ARLOCK;
  wire [2:0]Conn3_ARPROT;
  wire [3:0]Conn3_ARQOS;
  wire Conn3_ARREADY;
  wire [2:0]Conn3_ARSIZE;
  wire Conn3_ARVALID;
  wire [31:0]Conn3_RDATA;
  wire Conn3_RLAST;
  wire Conn3_RREADY;
  wire [1:0]Conn3_RRESP;
  wire Conn3_RVALID;
  wire [31:0]Conn4_ARADDR;
  wire [2:0]Conn4_ARPROT;
  wire [0:0]Conn4_ARREADY;
  wire [0:0]Conn4_ARVALID;
  wire [31:0]Conn4_AWADDR;
  wire [2:0]Conn4_AWPROT;
  wire [0:0]Conn4_AWREADY;
  wire [0:0]Conn4_AWVALID;
  wire [0:0]Conn4_BREADY;
  wire [1:0]Conn4_BRESP;
  wire [0:0]Conn4_BVALID;
  wire [31:0]Conn4_RDATA;
  wire [0:0]Conn4_RREADY;
  wire [1:0]Conn4_RRESP;
  wire [0:0]Conn4_RVALID;
  wire [31:0]Conn4_WDATA;
  wire [0:0]Conn4_WREADY;
  wire [3:0]Conn4_WSTRB;
  wire [0:0]Conn4_WVALID;
  wire [31:0]Conn5_ARADDR;
  wire [0:0]Conn5_ARREADY;
  wire [0:0]Conn5_ARVALID;
  wire [31:0]Conn5_AWADDR;
  wire [0:0]Conn5_AWREADY;
  wire [0:0]Conn5_AWVALID;
  wire [0:0]Conn5_BREADY;
  wire [1:0]Conn5_BRESP;
  wire [0:0]Conn5_BVALID;
  wire [31:0]Conn5_RDATA;
  wire [0:0]Conn5_RREADY;
  wire [1:0]Conn5_RRESP;
  wire [0:0]Conn5_RVALID;
  wire [31:0]Conn5_WDATA;
  wire [0:0]Conn5_WREADY;
  wire [0:0]Conn5_WVALID;
  wire [31:0]Conn6_ARADDR;
  wire Conn6_ARREADY;
  wire [0:0]Conn6_ARVALID;
  wire [31:0]Conn6_AWADDR;
  wire Conn6_AWREADY;
  wire [0:0]Conn6_AWVALID;
  wire [0:0]Conn6_BREADY;
  wire [1:0]Conn6_BRESP;
  wire Conn6_BVALID;
  wire [31:0]Conn6_RDATA;
  wire [0:0]Conn6_RREADY;
  wire [1:0]Conn6_RRESP;
  wire Conn6_RVALID;
  wire [31:0]Conn6_WDATA;
  wire Conn6_WREADY;
  wire [0:0]Conn6_WVALID;
  wire M00_ACLK_1;
  wire [0:0]M00_ARESETN_1;
  wire [31:0]S00_AXI_1_ARADDR;
  wire [1:0]S00_AXI_1_ARBURST;
  wire [3:0]S00_AXI_1_ARCACHE;
  wire [7:0]S00_AXI_1_ARLEN;
  wire [2:0]S00_AXI_1_ARPROT;
  wire S00_AXI_1_ARREADY;
  wire [2:0]S00_AXI_1_ARSIZE;
  wire [3:0]S00_AXI_1_ARUSER;
  wire S00_AXI_1_ARVALID;
  wire [31:0]S00_AXI_1_RDATA;
  wire S00_AXI_1_RLAST;
  wire S00_AXI_1_RREADY;
  wire [1:0]S00_AXI_1_RRESP;
  wire S00_AXI_1_RVALID;
  wire aud_mclk_1;
  wire [0:0]aud_mrst_1;
  wire [31:0]audio_formatter_0_m_axis_mm2s_TDATA;
  wire [7:0]audio_formatter_0_m_axis_mm2s_TID;
  wire audio_formatter_0_m_axis_mm2s_TREADY;
  wire audio_formatter_0_m_axis_mm2s_TVALID;
  wire audio_irq_1;
  wire [31:0]axi_interconnect_0_M00_AXI_ARADDR;
  wire [1:0]axi_interconnect_0_M00_AXI_ARBURST;
  wire [3:0]axi_interconnect_0_M00_AXI_ARCACHE;
  wire [3:0]axi_interconnect_0_M00_AXI_ARLEN;
  wire [1:0]axi_interconnect_0_M00_AXI_ARLOCK;
  wire [2:0]axi_interconnect_0_M00_AXI_ARPROT;
  wire [3:0]axi_interconnect_0_M00_AXI_ARQOS;
  wire axi_interconnect_0_M00_AXI_ARREADY;
  wire [2:0]axi_interconnect_0_M00_AXI_ARSIZE;
  wire axi_interconnect_0_M00_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M00_AXI_RDATA;
  wire axi_interconnect_0_M00_AXI_RLAST;
  wire axi_interconnect_0_M00_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M00_AXI_RRESP;
  wire axi_interconnect_0_M00_AXI_RVALID;
  wire [31:0]axis_register_slice_0_M_AXIS_TDATA;
  wire [7:0]axis_register_slice_0_M_AXIS_TID;
  wire axis_register_slice_0_M_AXIS_TREADY;
  wire axis_register_slice_0_M_AXIS_TVALID;
  wire ext_reset_in_1;
  wire [0:0]hdmi_intn_1;
  wire i2s_transmitter_0_lrclk_out;
  wire i2s_transmitter_0_sclk_out;
  wire i2s_transmitter_0_sdata_0_out;
  wire s_axi_ctrl_aclk_1;
  wire s_axi_ctrl_aresetn_1;
  wire s_axis_aud_aclk_1;
  wire vid_clk_1;
  wire [31:0]video_M00_AXI_ARADDR;
  wire [1:0]video_M00_AXI_ARBURST;
  wire [3:0]video_M00_AXI_ARCACHE;
  wire [3:0]video_M00_AXI_ARLEN;
  wire [1:0]video_M00_AXI_ARLOCK;
  wire [2:0]video_M00_AXI_ARPROT;
  wire [3:0]video_M00_AXI_ARQOS;
  wire video_M00_AXI_ARREADY;
  wire [2:0]video_M00_AXI_ARSIZE;
  wire video_M00_AXI_ARVALID;
  wire [31:0]video_M00_AXI_RDATA;
  wire video_M00_AXI_RLAST;
  wire video_M00_AXI_RREADY;
  wire [1:0]video_M00_AXI_RRESP;
  wire video_M00_AXI_RVALID;
  wire [0:0]video_aresetn;
  wire [1:0]video_control_vblank;
  wire [4:0]video_dout;
  wire video_hdmi_clk;
  wire [15:0]video_hdmi_data;
  wire video_hdmi_de;
  wire video_hdmi_hs;
  wire video_hdmi_vs;
  wire [0:0]video_peripheral_aresetn;

  assign Conn2_ARADDR = s_axi_ctrl_araddr[31:0];
  assign Conn2_ARVALID = s_axi_ctrl_arvalid[0];
  assign Conn2_AWADDR = s_axi_ctrl_awaddr[31:0];
  assign Conn2_AWVALID = s_axi_ctrl_awvalid[0];
  assign Conn2_BREADY = s_axi_ctrl_bready[0];
  assign Conn2_RREADY = s_axi_ctrl_rready[0];
  assign Conn2_WDATA = s_axi_ctrl_wdata[31:0];
  assign Conn2_WVALID = s_axi_ctrl_wvalid[0];
  assign Conn3_ARREADY = M_AXI_arready;
  assign Conn3_RDATA = M_AXI_rdata[31:0];
  assign Conn3_RLAST = M_AXI_rlast;
  assign Conn3_RRESP = M_AXI_rresp[1:0];
  assign Conn3_RVALID = M_AXI_rvalid;
  assign Conn4_ARADDR = S_AXI_araddr[31:0];
  assign Conn4_ARPROT = S_AXI_arprot[2:0];
  assign Conn4_ARVALID = S_AXI_arvalid[0];
  assign Conn4_AWADDR = S_AXI_awaddr[31:0];
  assign Conn4_AWPROT = S_AXI_awprot[2:0];
  assign Conn4_AWVALID = S_AXI_awvalid[0];
  assign Conn4_BREADY = S_AXI_bready[0];
  assign Conn4_RREADY = S_AXI_rready[0];
  assign Conn4_WDATA = S_AXI_wdata[31:0];
  assign Conn4_WSTRB = S_AXI_wstrb[3:0];
  assign Conn4_WVALID = S_AXI_wvalid[0];
  assign Conn5_ARADDR = S_AXI_LITE_araddr[31:0];
  assign Conn5_ARVALID = S_AXI_LITE_arvalid[0];
  assign Conn5_AWADDR = S_AXI_LITE_awaddr[31:0];
  assign Conn5_AWVALID = S_AXI_LITE_awvalid[0];
  assign Conn5_BREADY = S_AXI_LITE_bready[0];
  assign Conn5_RREADY = S_AXI_LITE_rready[0];
  assign Conn5_WDATA = S_AXI_LITE_wdata[31:0];
  assign Conn5_WVALID = S_AXI_LITE_wvalid[0];
  assign Conn6_ARADDR = s_axi_lite1_araddr[31:0];
  assign Conn6_ARVALID = s_axi_lite1_arvalid[0];
  assign Conn6_AWADDR = s_axi_lite1_awaddr[31:0];
  assign Conn6_AWVALID = s_axi_lite1_awvalid[0];
  assign Conn6_BREADY = s_axi_lite1_bready[0];
  assign Conn6_RREADY = s_axi_lite1_rready[0];
  assign Conn6_WDATA = s_axi_lite1_wdata[31:0];
  assign Conn6_WVALID = s_axi_lite1_wvalid[0];
  assign I2SO_D0 = i2s_transmitter_0_sdata_0_out;
  assign I2S_FSYNC_OUT = i2s_transmitter_0_lrclk_out;
  assign I2S_SCLK = i2s_transmitter_0_sclk_out;
  assign M00_ACLK_1 = M00_ACLK;
  assign M00_AXI_araddr[31:0] = axi_interconnect_0_M00_AXI_ARADDR;
  assign M00_AXI_arburst[1:0] = axi_interconnect_0_M00_AXI_ARBURST;
  assign M00_AXI_arcache[3:0] = axi_interconnect_0_M00_AXI_ARCACHE;
  assign M00_AXI_arlen[3:0] = axi_interconnect_0_M00_AXI_ARLEN;
  assign M00_AXI_arlock[1:0] = axi_interconnect_0_M00_AXI_ARLOCK;
  assign M00_AXI_arprot[2:0] = axi_interconnect_0_M00_AXI_ARPROT;
  assign M00_AXI_arqos[3:0] = axi_interconnect_0_M00_AXI_ARQOS;
  assign M00_AXI_arsize[2:0] = axi_interconnect_0_M00_AXI_ARSIZE;
  assign M00_AXI_arvalid = axi_interconnect_0_M00_AXI_ARVALID;
  assign M00_AXI_rready = axi_interconnect_0_M00_AXI_RREADY;
  assign M_AXI_araddr[31:0] = Conn3_ARADDR;
  assign M_AXI_arburst[1:0] = Conn3_ARBURST;
  assign M_AXI_arcache[3:0] = Conn3_ARCACHE;
  assign M_AXI_arlen[3:0] = Conn3_ARLEN;
  assign M_AXI_arlock[1:0] = Conn3_ARLOCK;
  assign M_AXI_arprot[2:0] = Conn3_ARPROT;
  assign M_AXI_arqos[3:0] = Conn3_ARQOS;
  assign M_AXI_arsize[2:0] = Conn3_ARSIZE;
  assign M_AXI_arvalid = Conn3_ARVALID;
  assign M_AXI_rready = Conn3_RREADY;
  assign S_AXI_LITE_arready[0] = Conn5_ARREADY;
  assign S_AXI_LITE_awready[0] = Conn5_AWREADY;
  assign S_AXI_LITE_bresp[1:0] = Conn5_BRESP;
  assign S_AXI_LITE_bvalid[0] = Conn5_BVALID;
  assign S_AXI_LITE_rdata[31:0] = Conn5_RDATA;
  assign S_AXI_LITE_rresp[1:0] = Conn5_RRESP;
  assign S_AXI_LITE_rvalid[0] = Conn5_RVALID;
  assign S_AXI_LITE_wready[0] = Conn5_WREADY;
  assign S_AXI_arready[0] = Conn4_ARREADY;
  assign S_AXI_awready[0] = Conn4_AWREADY;
  assign S_AXI_bresp[1:0] = Conn4_BRESP;
  assign S_AXI_bvalid[0] = Conn4_BVALID;
  assign S_AXI_rdata[31:0] = Conn4_RDATA;
  assign S_AXI_rresp[1:0] = Conn4_RRESP;
  assign S_AXI_rvalid[0] = Conn4_RVALID;
  assign S_AXI_wready[0] = Conn4_WREADY;
  assign aud_mclk_1 = aud_mclk;
  assign axi_interconnect_0_M00_AXI_ARREADY = M00_AXI_arready;
  assign axi_interconnect_0_M00_AXI_RDATA = M00_AXI_rdata[31:0];
  assign axi_interconnect_0_M00_AXI_RLAST = M00_AXI_rlast;
  assign axi_interconnect_0_M00_AXI_RRESP = M00_AXI_rresp[1:0];
  assign axi_interconnect_0_M00_AXI_RVALID = M00_AXI_rvalid;
  assign control_vblank[1:0] = video_control_vblank;
  assign dout[4:0] = video_dout;
  assign ext_reset_in_1 = ext_reset_in;
  assign hdmi_clk = video_hdmi_clk;
  assign hdmi_data[15:0] = video_hdmi_data;
  assign hdmi_de = video_hdmi_de;
  assign hdmi_hs = video_hdmi_hs;
  assign hdmi_intn_1 = hdmi_intn[0];
  assign hdmi_vs = video_hdmi_vs;
  assign s_axi_ctrl_aclk_1 = s_axi_ctrl_aclk;
  assign s_axi_ctrl_aresetn_1 = s_axi_ctrl_aresetn;
  assign s_axi_ctrl_arready[0] = Conn2_ARREADY;
  assign s_axi_ctrl_awready[0] = Conn2_AWREADY;
  assign s_axi_ctrl_bresp[1:0] = Conn2_BRESP;
  assign s_axi_ctrl_bvalid[0] = Conn2_BVALID;
  assign s_axi_ctrl_rdata[31:0] = Conn2_RDATA;
  assign s_axi_ctrl_rresp[1:0] = Conn2_RRESP;
  assign s_axi_ctrl_rvalid[0] = Conn2_RVALID;
  assign s_axi_ctrl_wready[0] = Conn2_WREADY;
  assign s_axi_lite1_arready[0] = Conn6_ARREADY;
  assign s_axi_lite1_awready[0] = Conn6_AWREADY;
  assign s_axi_lite1_bresp[1:0] = Conn6_BRESP;
  assign s_axi_lite1_bvalid[0] = Conn6_BVALID;
  assign s_axi_lite1_rdata[31:0] = Conn6_RDATA;
  assign s_axi_lite1_rresp[1:0] = Conn6_RRESP;
  assign s_axi_lite1_rvalid[0] = Conn6_RVALID;
  assign s_axi_lite1_wready[0] = Conn6_WREADY;
  assign s_axis_aud_aclk_1 = s_axis_aud_aclk;
  assign vid_clk_1 = vid_clk;
  design_1_audio_formatter_0_0 audio_formatter_0
       (.aud_mclk(aud_mclk_1),
        .aud_mreset(aud_mrst_1),
        .irq_mm2s(audio_irq_1),
        .m_axi_mm2s_araddr(S00_AXI_1_ARADDR),
        .m_axi_mm2s_arburst(S00_AXI_1_ARBURST),
        .m_axi_mm2s_arcache(S00_AXI_1_ARCACHE),
        .m_axi_mm2s_arlen(S00_AXI_1_ARLEN),
        .m_axi_mm2s_arprot(S00_AXI_1_ARPROT),
        .m_axi_mm2s_arready(S00_AXI_1_ARREADY),
        .m_axi_mm2s_arsize(S00_AXI_1_ARSIZE),
        .m_axi_mm2s_aruser(S00_AXI_1_ARUSER),
        .m_axi_mm2s_arvalid(S00_AXI_1_ARVALID),
        .m_axi_mm2s_rdata(S00_AXI_1_RDATA),
        .m_axi_mm2s_rlast(S00_AXI_1_RLAST),
        .m_axi_mm2s_rready(S00_AXI_1_RREADY),
        .m_axi_mm2s_rresp(S00_AXI_1_RRESP),
        .m_axi_mm2s_rvalid(S00_AXI_1_RVALID),
        .m_axis_mm2s_aclk(s_axis_aud_aclk_1),
        .m_axis_mm2s_aresetn(video_peripheral_aresetn),
        .m_axis_mm2s_tdata(audio_formatter_0_m_axis_mm2s_TDATA),
        .m_axis_mm2s_tid(audio_formatter_0_m_axis_mm2s_TID),
        .m_axis_mm2s_tready(audio_formatter_0_m_axis_mm2s_TREADY),
        .m_axis_mm2s_tvalid(audio_formatter_0_m_axis_mm2s_TVALID),
        .s_axi_lite_aclk(s_axi_ctrl_aclk_1),
        .s_axi_lite_araddr(Conn6_ARADDR[11:0]),
        .s_axi_lite_aresetn(s_axi_ctrl_aresetn_1),
        .s_axi_lite_arready(Conn6_ARREADY),
        .s_axi_lite_arvalid(Conn6_ARVALID),
        .s_axi_lite_awaddr(Conn6_AWADDR[11:0]),
        .s_axi_lite_awready(Conn6_AWREADY),
        .s_axi_lite_awvalid(Conn6_AWVALID),
        .s_axi_lite_bready(Conn6_BREADY),
        .s_axi_lite_bresp(Conn6_BRESP),
        .s_axi_lite_bvalid(Conn6_BVALID),
        .s_axi_lite_rdata(Conn6_RDATA),
        .s_axi_lite_rready(Conn6_RREADY),
        .s_axi_lite_rresp(Conn6_RRESP),
        .s_axi_lite_rvalid(Conn6_RVALID),
        .s_axi_lite_wdata(Conn6_WDATA),
        .s_axi_lite_wready(Conn6_WREADY),
        .s_axi_lite_wvalid(Conn6_WVALID));
  design_1_axi_interconnect_0_3 axi_interconnect_0
       (.ACLK(M00_ACLK_1),
        .ARESETN(M00_ARESETN_1),
        .M00_ACLK(M00_ACLK_1),
        .M00_ARESETN(M00_ARESETN_1),
        .M00_AXI_araddr(axi_interconnect_0_M00_AXI_ARADDR),
        .M00_AXI_arburst(axi_interconnect_0_M00_AXI_ARBURST),
        .M00_AXI_arcache(axi_interconnect_0_M00_AXI_ARCACHE),
        .M00_AXI_arlen(axi_interconnect_0_M00_AXI_ARLEN),
        .M00_AXI_arlock(axi_interconnect_0_M00_AXI_ARLOCK),
        .M00_AXI_arprot(axi_interconnect_0_M00_AXI_ARPROT),
        .M00_AXI_arqos(axi_interconnect_0_M00_AXI_ARQOS),
        .M00_AXI_arready(axi_interconnect_0_M00_AXI_ARREADY),
        .M00_AXI_arsize(axi_interconnect_0_M00_AXI_ARSIZE),
        .M00_AXI_arvalid(axi_interconnect_0_M00_AXI_ARVALID),
        .M00_AXI_rdata(axi_interconnect_0_M00_AXI_RDATA),
        .M00_AXI_rlast(axi_interconnect_0_M00_AXI_RLAST),
        .M00_AXI_rready(axi_interconnect_0_M00_AXI_RREADY),
        .M00_AXI_rresp(axi_interconnect_0_M00_AXI_RRESP),
        .M00_AXI_rvalid(axi_interconnect_0_M00_AXI_RVALID),
        .S00_ACLK(s_axis_aud_aclk_1),
        .S00_ARESETN(video_peripheral_aresetn),
        .S00_AXI_araddr(S00_AXI_1_ARADDR),
        .S00_AXI_arburst(S00_AXI_1_ARBURST),
        .S00_AXI_arcache(S00_AXI_1_ARCACHE),
        .S00_AXI_arlen(S00_AXI_1_ARLEN),
        .S00_AXI_arprot(S00_AXI_1_ARPROT),
        .S00_AXI_arready(S00_AXI_1_ARREADY),
        .S00_AXI_arsize(S00_AXI_1_ARSIZE),
        .S00_AXI_aruser(S00_AXI_1_ARUSER),
        .S00_AXI_arvalid(S00_AXI_1_ARVALID),
        .S00_AXI_rdata(S00_AXI_1_RDATA),
        .S00_AXI_rlast(S00_AXI_1_RLAST),
        .S00_AXI_rready(S00_AXI_1_RREADY),
        .S00_AXI_rresp(S00_AXI_1_RRESP),
        .S00_AXI_rvalid(S00_AXI_1_RVALID));
  design_1_axi_register_slice_0_0 axi_register_slice_0
       (.aclk(s_axis_aud_aclk_1),
        .aresetn(video_aresetn),
        .m_axi_araddr(Conn3_ARADDR),
        .m_axi_arburst(Conn3_ARBURST),
        .m_axi_arcache(Conn3_ARCACHE),
        .m_axi_arlen(Conn3_ARLEN),
        .m_axi_arlock(Conn3_ARLOCK),
        .m_axi_arprot(Conn3_ARPROT),
        .m_axi_arqos(Conn3_ARQOS),
        .m_axi_arready(Conn3_ARREADY),
        .m_axi_arsize(Conn3_ARSIZE),
        .m_axi_arvalid(Conn3_ARVALID),
        .m_axi_rdata(Conn3_RDATA),
        .m_axi_rlast(Conn3_RLAST),
        .m_axi_rready(Conn3_RREADY),
        .m_axi_rresp(Conn3_RRESP),
        .m_axi_rvalid(Conn3_RVALID),
        .s_axi_araddr(video_M00_AXI_ARADDR),
        .s_axi_arburst(video_M00_AXI_ARBURST),
        .s_axi_arcache(video_M00_AXI_ARCACHE),
        .s_axi_arlen(video_M00_AXI_ARLEN),
        .s_axi_arlock(video_M00_AXI_ARLOCK),
        .s_axi_arprot(video_M00_AXI_ARPROT),
        .s_axi_arqos(video_M00_AXI_ARQOS),
        .s_axi_arready(video_M00_AXI_ARREADY),
        .s_axi_arsize(video_M00_AXI_ARSIZE),
        .s_axi_arvalid(video_M00_AXI_ARVALID),
        .s_axi_rdata(video_M00_AXI_RDATA),
        .s_axi_rlast(video_M00_AXI_RLAST),
        .s_axi_rready(video_M00_AXI_RREADY),
        .s_axi_rresp(video_M00_AXI_RRESP),
        .s_axi_rvalid(video_M00_AXI_RVALID));
  design_1_axis_register_slice_0_0 axis_register_slice_0
       (.aclk(s_axis_aud_aclk_1),
        .aresetn(video_peripheral_aresetn),
        .m_axis_tdata(axis_register_slice_0_M_AXIS_TDATA),
        .m_axis_tid(axis_register_slice_0_M_AXIS_TID),
        .m_axis_tready(axis_register_slice_0_M_AXIS_TREADY),
        .m_axis_tvalid(axis_register_slice_0_M_AXIS_TVALID),
        .s_axis_tdata(audio_formatter_0_m_axis_mm2s_TDATA),
        .s_axis_tid(audio_formatter_0_m_axis_mm2s_TID),
        .s_axis_tready(audio_formatter_0_m_axis_mm2s_TREADY),
        .s_axis_tvalid(audio_formatter_0_m_axis_mm2s_TVALID));
  design_1_i2s_transmitter_0_0 i2s_transmitter_0
       (.aud_mclk(aud_mclk_1),
        .aud_mrst(aud_mrst_1),
        .lrclk_out(i2s_transmitter_0_lrclk_out),
        .s_axi_ctrl_aclk(s_axi_ctrl_aclk_1),
        .s_axi_ctrl_araddr(Conn2_ARADDR[7:0]),
        .s_axi_ctrl_aresetn(s_axi_ctrl_aresetn_1),
        .s_axi_ctrl_arready(Conn2_ARREADY),
        .s_axi_ctrl_arvalid(Conn2_ARVALID),
        .s_axi_ctrl_awaddr(Conn2_AWADDR[7:0]),
        .s_axi_ctrl_awready(Conn2_AWREADY),
        .s_axi_ctrl_awvalid(Conn2_AWVALID),
        .s_axi_ctrl_bready(Conn2_BREADY),
        .s_axi_ctrl_bresp(Conn2_BRESP),
        .s_axi_ctrl_bvalid(Conn2_BVALID),
        .s_axi_ctrl_rdata(Conn2_RDATA),
        .s_axi_ctrl_rready(Conn2_RREADY),
        .s_axi_ctrl_rresp(Conn2_RRESP),
        .s_axi_ctrl_rvalid(Conn2_RVALID),
        .s_axi_ctrl_wdata(Conn2_WDATA),
        .s_axi_ctrl_wready(Conn2_WREADY),
        .s_axi_ctrl_wvalid(Conn2_WVALID),
        .s_axis_aud_aclk(s_axis_aud_aclk_1),
        .s_axis_aud_aresetn(video_peripheral_aresetn),
        .s_axis_aud_tdata(axis_register_slice_0_M_AXIS_TDATA),
        .s_axis_aud_tid(axis_register_slice_0_M_AXIS_TID[2:0]),
        .s_axis_aud_tready(axis_register_slice_0_M_AXIS_TREADY),
        .s_axis_aud_tvalid(axis_register_slice_0_M_AXIS_TVALID),
        .sclk_out(i2s_transmitter_0_sclk_out),
        .sdata_0_out(i2s_transmitter_0_sdata_0_out));
  design_1_proc_sys_reset_1_0 proc_sys_reset_1
       (.aux_reset_in(1'b1),
        .dcm_locked(1'b1),
        .ext_reset_in(ext_reset_in_1),
        .mb_debug_sys_rst(1'b0),
        .peripheral_reset(aud_mrst_1),
        .slowest_sync_clk(aud_mclk_1));
  design_1_rst_ps7_0_200M_1_2 rst_ps7_0_200M_1
       (.aux_reset_in(1'b1),
        .dcm_locked(1'b1),
        .ext_reset_in(ext_reset_in_1),
        .mb_debug_sys_rst(1'b0),
        .peripheral_aresetn(M00_ARESETN_1),
        .slowest_sync_clk(M00_ACLK_1));
  video_imp_1FOBHOA video
       (.M00_AXI_araddr(video_M00_AXI_ARADDR),
        .M00_AXI_arburst(video_M00_AXI_ARBURST),
        .M00_AXI_arcache(video_M00_AXI_ARCACHE),
        .M00_AXI_arlen(video_M00_AXI_ARLEN),
        .M00_AXI_arlock(video_M00_AXI_ARLOCK),
        .M00_AXI_arprot(video_M00_AXI_ARPROT),
        .M00_AXI_arqos(video_M00_AXI_ARQOS),
        .M00_AXI_arready(video_M00_AXI_ARREADY),
        .M00_AXI_arsize(video_M00_AXI_ARSIZE),
        .M00_AXI_arvalid(video_M00_AXI_ARVALID),
        .M00_AXI_rdata(video_M00_AXI_RDATA),
        .M00_AXI_rlast(video_M00_AXI_RLAST),
        .M00_AXI_rready(video_M00_AXI_RREADY),
        .M00_AXI_rresp(video_M00_AXI_RRESP),
        .M00_AXI_rvalid(video_M00_AXI_RVALID),
        .S_AXI_LITE_araddr(Conn5_ARADDR),
        .S_AXI_LITE_arready(Conn5_ARREADY),
        .S_AXI_LITE_arvalid(Conn5_ARVALID),
        .S_AXI_LITE_awaddr(Conn5_AWADDR),
        .S_AXI_LITE_awready(Conn5_AWREADY),
        .S_AXI_LITE_awvalid(Conn5_AWVALID),
        .S_AXI_LITE_bready(Conn5_BREADY),
        .S_AXI_LITE_bresp(Conn5_BRESP),
        .S_AXI_LITE_bvalid(Conn5_BVALID),
        .S_AXI_LITE_rdata(Conn5_RDATA),
        .S_AXI_LITE_rready(Conn5_RREADY),
        .S_AXI_LITE_rresp(Conn5_RRESP),
        .S_AXI_LITE_rvalid(Conn5_RVALID),
        .S_AXI_LITE_wdata(Conn5_WDATA),
        .S_AXI_LITE_wready(Conn5_WREADY),
        .S_AXI_LITE_wvalid(Conn5_WVALID),
        .S_AXI_araddr(Conn4_ARADDR),
        .S_AXI_arprot(Conn4_ARPROT),
        .S_AXI_arready(Conn4_ARREADY),
        .S_AXI_arvalid(Conn4_ARVALID),
        .S_AXI_awaddr(Conn4_AWADDR),
        .S_AXI_awprot(Conn4_AWPROT),
        .S_AXI_awready(Conn4_AWREADY),
        .S_AXI_awvalid(Conn4_AWVALID),
        .S_AXI_bready(Conn4_BREADY),
        .S_AXI_bresp(Conn4_BRESP),
        .S_AXI_bvalid(Conn4_BVALID),
        .S_AXI_rdata(Conn4_RDATA),
        .S_AXI_rready(Conn4_RREADY),
        .S_AXI_rresp(Conn4_RRESP),
        .S_AXI_rvalid(Conn4_RVALID),
        .S_AXI_wdata(Conn4_WDATA),
        .S_AXI_wready(Conn4_WREADY),
        .S_AXI_wstrb(Conn4_WSTRB),
        .S_AXI_wvalid(Conn4_WVALID),
        .aresetn(video_aresetn),
        .audio_irq(audio_irq_1),
        .control_vblank(video_control_vblank),
        .dout(video_dout),
        .ext_reset_in(ext_reset_in_1),
        .hdmi_clk(video_hdmi_clk),
        .hdmi_data(video_hdmi_data),
        .hdmi_de(video_hdmi_de),
        .hdmi_hs(video_hdmi_hs),
        .hdmi_intn(hdmi_intn_1),
        .hdmi_vs(video_hdmi_vs),
        .peripheral_aresetn(video_peripheral_aresetn),
        .s_axi_aclk(s_axi_ctrl_aclk_1),
        .s_axi_aresetn(s_axi_ctrl_aresetn_1),
        .s_axis_aclk(s_axis_aud_aclk_1),
        .vid_clk(vid_clk_1));
endmodule

module clock_generation_imp_1SL5TTO
   (AXI_clk,
    BCLK_clk,
    CLK90_pin,
    CPUCLK_clk,
    PCLK_clk,
    PCLK_reg,
    clk90_detected,
    clk_in1,
    clk_out1,
    clk_out2,
    cpuclk_detected,
    enable_clk_output,
    nCLKEN_clk,
    nCLKEN_reg,
    s_axi_aclk,
    s_axi_aresetn,
    s_axi_lite1_araddr,
    s_axi_lite1_arready,
    s_axi_lite1_arvalid,
    s_axi_lite1_awaddr,
    s_axi_lite1_awready,
    s_axi_lite1_awvalid,
    s_axi_lite1_bready,
    s_axi_lite1_bresp,
    s_axi_lite1_bvalid,
    s_axi_lite1_rdata,
    s_axi_lite1_rready,
    s_axi_lite1_rresp,
    s_axi_lite1_rvalid,
    s_axi_lite1_wdata,
    s_axi_lite1_wready,
    s_axi_lite1_wstrb,
    s_axi_lite1_wvalid,
    s_axi_lite_araddr,
    s_axi_lite_arready,
    s_axi_lite_arvalid,
    s_axi_lite_awaddr,
    s_axi_lite_awready,
    s_axi_lite_awvalid,
    s_axi_lite_bready,
    s_axi_lite_bresp,
    s_axi_lite_bvalid,
    s_axi_lite_rdata,
    s_axi_lite_rready,
    s_axi_lite_rresp,
    s_axi_lite_rvalid,
    s_axi_lite_wdata,
    s_axi_lite_wready,
    s_axi_lite_wstrb,
    s_axi_lite_wvalid);
  output AXI_clk;
  output BCLK_clk;
  inout CLK90_pin;
  inout CPUCLK_clk;
  output PCLK_clk;
  output PCLK_reg;
  output clk90_detected;
  input clk_in1;
  output clk_out1;
  output clk_out2;
  output cpuclk_detected;
  input enable_clk_output;
  output nCLKEN_clk;
  output nCLKEN_reg;
  input s_axi_aclk;
  input s_axi_aresetn;
  input [31:0]s_axi_lite1_araddr;
  output [0:0]s_axi_lite1_arready;
  input [0:0]s_axi_lite1_arvalid;
  input [31:0]s_axi_lite1_awaddr;
  output [0:0]s_axi_lite1_awready;
  input [0:0]s_axi_lite1_awvalid;
  input [0:0]s_axi_lite1_bready;
  output [1:0]s_axi_lite1_bresp;
  output [0:0]s_axi_lite1_bvalid;
  output [31:0]s_axi_lite1_rdata;
  input [0:0]s_axi_lite1_rready;
  output [1:0]s_axi_lite1_rresp;
  output [0:0]s_axi_lite1_rvalid;
  input [31:0]s_axi_lite1_wdata;
  output [0:0]s_axi_lite1_wready;
  input [3:0]s_axi_lite1_wstrb;
  input [0:0]s_axi_lite1_wvalid;
  input [31:0]s_axi_lite_araddr;
  output [0:0]s_axi_lite_arready;
  input [0:0]s_axi_lite_arvalid;
  input [31:0]s_axi_lite_awaddr;
  output [0:0]s_axi_lite_awready;
  input [0:0]s_axi_lite_awvalid;
  input [0:0]s_axi_lite_bready;
  output [1:0]s_axi_lite_bresp;
  output [0:0]s_axi_lite_bvalid;
  output [31:0]s_axi_lite_rdata;
  input [0:0]s_axi_lite_rready;
  output [1:0]s_axi_lite_rresp;
  output [0:0]s_axi_lite_rvalid;
  input [31:0]s_axi_lite_wdata;
  output [0:0]s_axi_lite_wready;
  input [3:0]s_axi_lite_wstrb;
  input [0:0]s_axi_lite_wvalid;

  wire [31:0]Conn1_ARADDR;
  wire Conn1_ARREADY;
  wire [0:0]Conn1_ARVALID;
  wire [31:0]Conn1_AWADDR;
  wire Conn1_AWREADY;
  wire [0:0]Conn1_AWVALID;
  wire [0:0]Conn1_BREADY;
  wire [1:0]Conn1_BRESP;
  wire Conn1_BVALID;
  wire [31:0]Conn1_RDATA;
  wire [0:0]Conn1_RREADY;
  wire [1:0]Conn1_RRESP;
  wire Conn1_RVALID;
  wire [31:0]Conn1_WDATA;
  wire Conn1_WREADY;
  wire [3:0]Conn1_WSTRB;
  wire [0:0]Conn1_WVALID;
  wire [31:0]Conn2_ARADDR;
  wire Conn2_ARREADY;
  wire [0:0]Conn2_ARVALID;
  wire [31:0]Conn2_AWADDR;
  wire Conn2_AWREADY;
  wire [0:0]Conn2_AWVALID;
  wire [0:0]Conn2_BREADY;
  wire [1:0]Conn2_BRESP;
  wire Conn2_BVALID;
  wire [31:0]Conn2_RDATA;
  wire [0:0]Conn2_RREADY;
  wire [1:0]Conn2_RRESP;
  wire Conn2_RVALID;
  wire [31:0]Conn2_WDATA;
  wire Conn2_WREADY;
  wire [3:0]Conn2_WSTRB;
  wire [0:0]Conn2_WVALID;
  wire Net;
  wire Net1;
  wire clk_in1_1;
  wire clk_wiz_0_AXI_clk;
  wire clk_wiz_0_BCLK_clk;
  wire clk_wiz_0_CLK90_clk;
  wire clk_wiz_0_CPUCLK_clk;
  wire clk_wiz_0_PCLK_clk;
  wire clk_wiz_0_nCLKEN_clk;
  wire clk_wiz_2_clk_out1;
  wire clk_wiz_3_clk_out1;
  wire cpuclk_iob_0_clk90_detected;
  wire cpuclk_iob_0_cpuclk_detected;
  wire enable_clk_output_1;
  wire s_axi_aclk_1;
  wire s_axi_aresetn_1;

  assign AXI_clk = clk_wiz_0_AXI_clk;
  assign BCLK_clk = clk_wiz_0_BCLK_clk;
  assign Conn1_ARADDR = s_axi_lite_araddr[31:0];
  assign Conn1_ARVALID = s_axi_lite_arvalid[0];
  assign Conn1_AWADDR = s_axi_lite_awaddr[31:0];
  assign Conn1_AWVALID = s_axi_lite_awvalid[0];
  assign Conn1_BREADY = s_axi_lite_bready[0];
  assign Conn1_RREADY = s_axi_lite_rready[0];
  assign Conn1_WDATA = s_axi_lite_wdata[31:0];
  assign Conn1_WSTRB = s_axi_lite_wstrb[3:0];
  assign Conn1_WVALID = s_axi_lite_wvalid[0];
  assign Conn2_ARADDR = s_axi_lite1_araddr[31:0];
  assign Conn2_ARVALID = s_axi_lite1_arvalid[0];
  assign Conn2_AWADDR = s_axi_lite1_awaddr[31:0];
  assign Conn2_AWVALID = s_axi_lite1_awvalid[0];
  assign Conn2_BREADY = s_axi_lite1_bready[0];
  assign Conn2_RREADY = s_axi_lite1_rready[0];
  assign Conn2_WDATA = s_axi_lite1_wdata[31:0];
  assign Conn2_WSTRB = s_axi_lite1_wstrb[3:0];
  assign Conn2_WVALID = s_axi_lite1_wvalid[0];
  assign PCLK_clk = clk_wiz_0_PCLK_clk;
  assign PCLK_reg = clk_wiz_0_PCLK_clk;
  assign clk90_detected = cpuclk_iob_0_clk90_detected;
  assign clk_in1_1 = clk_in1;
  assign clk_out1 = clk_wiz_2_clk_out1;
  assign clk_out2 = clk_wiz_3_clk_out1;
  assign cpuclk_detected = cpuclk_iob_0_cpuclk_detected;
  assign enable_clk_output_1 = enable_clk_output;
  assign nCLKEN_clk = clk_wiz_0_nCLKEN_clk;
  assign nCLKEN_reg = clk_wiz_0_nCLKEN_clk;
  assign s_axi_aclk_1 = s_axi_aclk;
  assign s_axi_aresetn_1 = s_axi_aresetn;
  assign s_axi_lite1_arready[0] = Conn2_ARREADY;
  assign s_axi_lite1_awready[0] = Conn2_AWREADY;
  assign s_axi_lite1_bresp[1:0] = Conn2_BRESP;
  assign s_axi_lite1_bvalid[0] = Conn2_BVALID;
  assign s_axi_lite1_rdata[31:0] = Conn2_RDATA;
  assign s_axi_lite1_rresp[1:0] = Conn2_RRESP;
  assign s_axi_lite1_rvalid[0] = Conn2_RVALID;
  assign s_axi_lite1_wready[0] = Conn2_WREADY;
  assign s_axi_lite_arready[0] = Conn1_ARREADY;
  assign s_axi_lite_awready[0] = Conn1_AWREADY;
  assign s_axi_lite_bresp[1:0] = Conn1_BRESP;
  assign s_axi_lite_bvalid[0] = Conn1_BVALID;
  assign s_axi_lite_rdata[31:0] = Conn1_RDATA;
  assign s_axi_lite_rresp[1:0] = Conn1_RRESP;
  assign s_axi_lite_rvalid[0] = Conn1_RVALID;
  assign s_axi_lite_wready[0] = Conn1_WREADY;
  design_1_clk_wiz_0_0 clk_wiz_0
       (.AXI_clk(clk_wiz_0_AXI_clk),
        .BCLK_clk(clk_wiz_0_BCLK_clk),
        .CLK90_clk(clk_wiz_0_CLK90_clk),
        .CPUCLK_clk(clk_wiz_0_CPUCLK_clk),
        .PCLK_clk(clk_wiz_0_PCLK_clk),
        .clk_in1(clk_in1_1),
        .nCLKEN_clk(clk_wiz_0_nCLKEN_clk),
        .s_axi_aclk(s_axi_aclk_1),
        .s_axi_araddr(Conn2_ARADDR[10:0]),
        .s_axi_aresetn(s_axi_aresetn_1),
        .s_axi_arready(Conn2_ARREADY),
        .s_axi_arvalid(Conn2_ARVALID),
        .s_axi_awaddr(Conn2_AWADDR[10:0]),
        .s_axi_awready(Conn2_AWREADY),
        .s_axi_awvalid(Conn2_AWVALID),
        .s_axi_bready(Conn2_BREADY),
        .s_axi_bresp(Conn2_BRESP),
        .s_axi_bvalid(Conn2_BVALID),
        .s_axi_rdata(Conn2_RDATA),
        .s_axi_rready(Conn2_RREADY),
        .s_axi_rresp(Conn2_RRESP),
        .s_axi_rvalid(Conn2_RVALID),
        .s_axi_wdata(Conn2_WDATA),
        .s_axi_wready(Conn2_WREADY),
        .s_axi_wstrb(Conn2_WSTRB),
        .s_axi_wvalid(Conn2_WVALID));
  design_1_clk_wiz_2_0 clk_wiz_2
       (.clk_in1(clk_in1_1),
        .clk_out1(clk_wiz_2_clk_out1),
        .s_axi_aclk(s_axi_aclk_1),
        .s_axi_araddr(Conn1_ARADDR[10:0]),
        .s_axi_aresetn(s_axi_aresetn_1),
        .s_axi_arready(Conn1_ARREADY),
        .s_axi_arvalid(Conn1_ARVALID),
        .s_axi_awaddr(Conn1_AWADDR[10:0]),
        .s_axi_awready(Conn1_AWREADY),
        .s_axi_awvalid(Conn1_AWVALID),
        .s_axi_bready(Conn1_BREADY),
        .s_axi_bresp(Conn1_BRESP),
        .s_axi_bvalid(Conn1_BVALID),
        .s_axi_rdata(Conn1_RDATA),
        .s_axi_rready(Conn1_RREADY),
        .s_axi_rresp(Conn1_RRESP),
        .s_axi_rvalid(Conn1_RVALID),
        .s_axi_wdata(Conn1_WDATA),
        .s_axi_wready(Conn1_WREADY),
        .s_axi_wstrb(Conn1_WSTRB),
        .s_axi_wvalid(Conn1_WVALID));
  design_1_clk_wiz_3_0 clk_wiz_3
       (.clk_in1(clk_in1_1),
        .clk_out1(clk_wiz_3_clk_out1));
  design_1_cpuclk_iob_0_0 cpuclk_iob_0
       (.CLK90_in(clk_wiz_0_CLK90_clk),
        .CLK90_pin(CLK90_pin),
        .CPUCLK_in(clk_wiz_0_CPUCLK_clk),
        .CPUCLK_pin(CPUCLK_clk),
        .clk90_detected(cpuclk_iob_0_clk90_detected),
        .cpuclk_detected(cpuclk_iob_0_cpuclk_detected),
        .enable_output(enable_clk_output_1),
        .fast_clk(clk_wiz_0_AXI_clk));
endmodule

(* CORE_GENERATION_INFO = "design_1,IP_Integrator,{x_ipVendor=xilinx.com,x_ipLibrary=BlockDiagram,x_ipName=design_1,x_ipVersion=1.00.a,x_ipLanguage=VERILOG,numBlks=54,numReposBlks=35,numNonXlnxBlks=0,numHierBlks=19,maxHierDepth=3,numSysgenBlks=0,numHlsBlks=0,numHdlrefBlks=3,numPkgbdBlks=0,bdsource=USER,da_axi4_cnt=10,da_board_cnt=13,da_clkrst_cnt=57,da_ps7_cnt=1,synth_mode=None}" *) (* HW_HANDOFF = "design_1.hwdef" *) 
module design_1
   (A060,
    BCLK_clk,
    BP,
    CLK90_clk,
    CPUCLK_clk,
    D040,
    DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    I2SO_D0,
    I2S_FSYNC_OUT,
    I2S_SCLK,
    IIC_0_scl_i,
    IIC_0_scl_o,
    IIC_0_scl_t,
    IIC_0_sda_i,
    IIC_0_sda_o,
    IIC_0_sda_t,
    INT6_ARM,
    PCLK_clk,
    R_W040,
    SIZ40,
    hdmi_clk,
    hdmi_data,
    hdmi_de,
    hdmi_hs,
    hdmi_intn,
    hdmi_vs,
    nCLKEN_clk,
    nTA,
    nTBI,
    nTCI,
    nTEA,
    nTS,
    nTS_FPGA);
  (* X_INTERFACE_INFO = "xilinx.com:signal:data:1.0 DATA.A060 DATA" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME DATA.A060, LAYERED_METADATA undef" *) inout [31:0]A060;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.BCLK_CLK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.BCLK_CLK, CLK_DOMAIN /processing_av_system/clock_generation/clk_wiz_0_clk_out1, FREQ_HZ 25000000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 40.0" *) output BCLK_clk;
  output BP;
  inout CLK90_clk;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.CPUCLK_CLK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.CPUCLK_CLK, FREQ_HZ 25000000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 0.0" *) inout CPUCLK_clk;
  (* X_INTERFACE_INFO = "xilinx.com:signal:data:1.0 DATA.D040 DATA" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME DATA.D040, LAYERED_METADATA undef" *) inout [31:0]D040;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR ADDR" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME DDR, AXI_ARBITRATION_SCHEME TDM, BURST_LENGTH 8, CAN_DEBUG false, CAS_LATENCY 11, CAS_WRITE_LATENCY 11, CS_ENABLED true, DATA_MASK_ENABLED true, DATA_WIDTH 8, MEMORY_TYPE COMPONENTS, MEM_ADDR_MAP ROW_COLUMN_BANK, SLOT Single, TIMEPERIOD_PS 1250" *) inout [14:0]DDR_addr;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR BA" *) inout [2:0]DDR_ba;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR CAS_N" *) inout DDR_cas_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR CK_N" *) inout DDR_ck_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR CK_P" *) inout DDR_ck_p;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR CKE" *) inout DDR_cke;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR CS_N" *) inout DDR_cs_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR DM" *) inout [3:0]DDR_dm;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR DQ" *) inout [31:0]DDR_dq;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR DQS_N" *) inout [3:0]DDR_dqs_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR DQS_P" *) inout [3:0]DDR_dqs_p;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR ODT" *) inout DDR_odt;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR RAS_N" *) inout DDR_ras_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR RESET_N" *) inout DDR_reset_n;
  (* X_INTERFACE_INFO = "xilinx.com:interface:ddrx:1.0 DDR WE_N" *) inout DDR_we_n;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO DDR_VRN" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME FIXED_IO, CAN_DEBUG false" *) inout FIXED_IO_ddr_vrn;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO DDR_VRP" *) inout FIXED_IO_ddr_vrp;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO MIO" *) inout [53:0]FIXED_IO_mio;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO PS_CLK" *) inout FIXED_IO_ps_clk;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO PS_PORB" *) inout FIXED_IO_ps_porb;
  (* X_INTERFACE_INFO = "xilinx.com:display_processing_system7:fixedio:1.0 FIXED_IO PS_SRSTB" *) inout FIXED_IO_ps_srstb;
  output I2SO_D0;
  output I2S_FSYNC_OUT;
  output I2S_SCLK;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SCL_I" *) input IIC_0_scl_i;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SCL_O" *) output IIC_0_scl_o;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SCL_T" *) output IIC_0_scl_t;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SDA_I" *) input IIC_0_sda_i;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SDA_O" *) output IIC_0_sda_o;
  (* X_INTERFACE_INFO = "xilinx.com:interface:iic:1.0 IIC_0 SDA_T" *) output IIC_0_sda_t;
  (* X_INTERFACE_INFO = "xilinx.com:signal:interrupt:1.0 INTR.INT6_ARM INTERRUPT" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME INTR.INT6_ARM, PortWidth 1, SENSITIVITY LEVEL_HIGH" *) output INT6_ARM;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.PCLK_CLK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.PCLK_CLK, CLK_DOMAIN /processing_av_system/clock_generation/clk_wiz_0_clk_out1, FREQ_HZ 100000000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 30.0" *) output PCLK_clk;
  inout R_W040;
  inout [1:0]SIZ40;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.HDMI_CLK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.HDMI_CLK, CLK_DOMAIN /processing_av_system/clock_generation/clk_wiz_2_clk_out1, FREQ_HZ 148500000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 0.0" *) output hdmi_clk;
  (* X_INTERFACE_INFO = "xilinx.com:signal:data:1.0 DATA.HDMI_DATA DATA" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME DATA.HDMI_DATA, LAYERED_METADATA undef" *) output [15:0]hdmi_data;
  output hdmi_de;
  output hdmi_hs;
  input [0:0]hdmi_intn;
  output hdmi_vs;
  (* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.NCLKEN_CLK CLK" *) (* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME CLK.NCLKEN_CLK, CLK_DOMAIN /processing_av_system/clock_generation/clk_wiz_0_clk_out1, FREQ_HZ 50000000, FREQ_TOLERANCE_HZ 0, INSERT_VIP 0, PHASE 60.0" *) output nCLKEN_clk;
  inout nTA;
  output nTBI;
  input nTCI;
  input nTEA;
  input nTS;
  output nTS_FPGA;

  wire Net;
  wire [31:0]Net1;
  wire [31:0]Net2;
  wire Net3;
  wire Net4;
  wire [1:0]Net5;
  wire Net6;
  wire clk_wiz_0_PCLK_clk;
  wire clk_wiz_0_nCLKEN_clk;
  wire clk_wiz_1_BCLK_clk;
  wire [0:0]hdmi_intn_1;
  wire nTCI_1;
  wire nTEA_1;
  wire nTS_1;
  wire processing_av_system_AXI1_clk;
  wire processing_av_system_AXI_clk;
  wire processing_av_system_CLK18M_clk;
  wire [14:0]processing_av_system_DDR_ADDR;
  wire [2:0]processing_av_system_DDR_BA;
  wire processing_av_system_DDR_CAS_N;
  wire processing_av_system_DDR_CKE;
  wire processing_av_system_DDR_CK_N;
  wire processing_av_system_DDR_CK_P;
  wire processing_av_system_DDR_CS_N;
  wire [3:0]processing_av_system_DDR_DM;
  wire [31:0]processing_av_system_DDR_DQ;
  wire [3:0]processing_av_system_DDR_DQS_N;
  wire [3:0]processing_av_system_DDR_DQS_P;
  wire processing_av_system_DDR_ODT;
  wire processing_av_system_DDR_RAS_N;
  wire processing_av_system_DDR_RESET_N;
  wire processing_av_system_DDR_WE_N;
  wire processing_av_system_FIXED_IO_DDR_VRN;
  wire processing_av_system_FIXED_IO_DDR_VRP;
  wire [53:0]processing_av_system_FIXED_IO_MIO;
  wire processing_av_system_FIXED_IO_PS_CLK;
  wire processing_av_system_FIXED_IO_PS_PORB;
  wire processing_av_system_FIXED_IO_PS_SRSTB;
  wire processing_av_system_I2SO_D0;
  wire processing_av_system_I2S_FSYNC_OUT;
  wire processing_av_system_I2S_SCLK;
  wire processing_av_system_IIC_0_SCL_I;
  wire processing_av_system_IIC_0_SCL_O;
  wire processing_av_system_IIC_0_SCL_T;
  wire processing_av_system_IIC_0_SDA_I;
  wire processing_av_system_IIC_0_SDA_O;
  wire processing_av_system_IIC_0_SDA_T;
  wire [31:0]processing_av_system_M_AXI1_ARADDR;
  wire [2:0]processing_av_system_M_AXI1_ARPROT;
  wire processing_av_system_M_AXI1_ARREADY;
  wire processing_av_system_M_AXI1_ARVALID;
  wire [31:0]processing_av_system_M_AXI1_AWADDR;
  wire [2:0]processing_av_system_M_AXI1_AWPROT;
  wire processing_av_system_M_AXI1_AWREADY;
  wire processing_av_system_M_AXI1_AWVALID;
  wire processing_av_system_M_AXI1_BREADY;
  wire [1:0]processing_av_system_M_AXI1_BRESP;
  wire processing_av_system_M_AXI1_BVALID;
  wire [31:0]processing_av_system_M_AXI1_RDATA;
  wire processing_av_system_M_AXI1_RREADY;
  wire [1:0]processing_av_system_M_AXI1_RRESP;
  wire processing_av_system_M_AXI1_RVALID;
  wire [31:0]processing_av_system_M_AXI1_WDATA;
  wire processing_av_system_M_AXI1_WREADY;
  wire [3:0]processing_av_system_M_AXI1_WSTRB;
  wire processing_av_system_M_AXI1_WVALID;
  wire [31:0]processing_av_system_M_AXI_ARADDR;
  wire [2:0]processing_av_system_M_AXI_ARPROT;
  wire processing_av_system_M_AXI_ARREADY;
  wire processing_av_system_M_AXI_ARVALID;
  wire [31:0]processing_av_system_M_AXI_AWADDR;
  wire [2:0]processing_av_system_M_AXI_AWPROT;
  wire processing_av_system_M_AXI_AWREADY;
  wire processing_av_system_M_AXI_AWVALID;
  wire processing_av_system_M_AXI_BREADY;
  wire [1:0]processing_av_system_M_AXI_BRESP;
  wire processing_av_system_M_AXI_BVALID;
  wire [31:0]processing_av_system_M_AXI_RDATA;
  wire processing_av_system_M_AXI_RREADY;
  wire [1:0]processing_av_system_M_AXI_RRESP;
  wire processing_av_system_M_AXI_RVALID;
  wire [31:0]processing_av_system_M_AXI_WDATA;
  wire processing_av_system_M_AXI_WREADY;
  wire [3:0]processing_av_system_M_AXI_WSTRB;
  wire processing_av_system_M_AXI_WVALID;
  wire processing_av_system_PCLK_reg;
  wire [0:0]processing_av_system_aresetn;
  wire [0:0]processing_av_system_aresetn1;
  wire processing_av_system_clk90_detected;
  wire [1:0]processing_av_system_control_vblank;
  wire processing_av_system_cpuclk_detected;
  wire processing_av_system_hdmi_clk;
  wire [15:0]processing_av_system_hdmi_data;
  wire processing_av_system_hdmi_de;
  wire processing_av_system_hdmi_hs;
  wire processing_av_system_hdmi_vs;
  wire processing_av_system_nCLKEN_reg;
  wire [0:0]processing_av_system_peripheral_reset;
  wire z3660_0_BP;
  wire z3660_0_NU_1;
  wire z3660_0_enable_clk_output;
  wire z3660_0_interrupt;
  wire [31:0]z3660_0_m00_axi_ARADDR;
  wire [1:0]z3660_0_m00_axi_ARBURST;
  wire [3:0]z3660_0_m00_axi_ARCACHE;
  wire [3:0]z3660_0_m00_axi_ARLEN;
  wire z3660_0_m00_axi_ARLOCK;
  wire [2:0]z3660_0_m00_axi_ARPROT;
  wire z3660_0_m00_axi_ARREADY;
  wire [2:0]z3660_0_m00_axi_ARSIZE;
  wire [4:0]z3660_0_m00_axi_ARUSER;
  wire z3660_0_m00_axi_ARVALID;
  wire [31:0]z3660_0_m00_axi_AWADDR;
  wire [1:0]z3660_0_m00_axi_AWBURST;
  wire [3:0]z3660_0_m00_axi_AWCACHE;
  wire [3:0]z3660_0_m00_axi_AWLEN;
  wire z3660_0_m00_axi_AWLOCK;
  wire [2:0]z3660_0_m00_axi_AWPROT;
  wire z3660_0_m00_axi_AWREADY;
  wire [2:0]z3660_0_m00_axi_AWSIZE;
  wire [4:0]z3660_0_m00_axi_AWUSER;
  wire z3660_0_m00_axi_AWVALID;
  wire z3660_0_m00_axi_BREADY;
  wire [1:0]z3660_0_m00_axi_BRESP;
  wire z3660_0_m00_axi_BVALID;
  wire [31:0]z3660_0_m00_axi_RDATA;
  wire z3660_0_m00_axi_RLAST;
  wire z3660_0_m00_axi_RREADY;
  wire [1:0]z3660_0_m00_axi_RRESP;
  wire z3660_0_m00_axi_RVALID;
  wire [31:0]z3660_0_m00_axi_WDATA;
  wire z3660_0_m00_axi_WLAST;
  wire z3660_0_m00_axi_WREADY;
  wire [3:0]z3660_0_m00_axi_WSTRB;
  wire z3660_0_m00_axi_WVALID;
  wire z3660_0_nTBI;

  assign BCLK_clk = clk_wiz_1_BCLK_clk;
  assign BP = z3660_0_BP;
  assign I2SO_D0 = processing_av_system_I2SO_D0;
  assign I2S_FSYNC_OUT = processing_av_system_I2S_FSYNC_OUT;
  assign I2S_SCLK = processing_av_system_I2S_SCLK;
  assign IIC_0_scl_o = processing_av_system_IIC_0_SCL_O;
  assign IIC_0_scl_t = processing_av_system_IIC_0_SCL_T;
  assign IIC_0_sda_o = processing_av_system_IIC_0_SDA_O;
  assign IIC_0_sda_t = processing_av_system_IIC_0_SDA_T;
  assign INT6_ARM = z3660_0_interrupt;
  assign PCLK_clk = clk_wiz_0_PCLK_clk;
  assign hdmi_clk = processing_av_system_hdmi_clk;
  assign hdmi_data[15:0] = processing_av_system_hdmi_data;
  assign hdmi_de = processing_av_system_hdmi_de;
  assign hdmi_hs = processing_av_system_hdmi_hs;
  assign hdmi_intn_1 = hdmi_intn[0];
  assign hdmi_vs = processing_av_system_hdmi_vs;
  assign nCLKEN_clk = clk_wiz_0_nCLKEN_clk;
  assign nTBI = z3660_0_nTBI;
  assign nTCI_1 = nTCI;
  assign nTEA_1 = nTEA;
  assign nTS_1 = nTS;
  assign nTS_FPGA = z3660_0_NU_1;
  assign processing_av_system_IIC_0_SCL_I = IIC_0_scl_i;
  assign processing_av_system_IIC_0_SDA_I = IIC_0_sda_i;
  processing_av_system_imp_2DPZH9 processing_av_system
       (.AXI1_clk(processing_av_system_AXI1_clk),
        .AXI_clk(processing_av_system_AXI_clk),
        .BCLK_clk(clk_wiz_1_BCLK_clk),
        .CLK18M_clk(processing_av_system_CLK18M_clk),
        .CLK90_clk(CLK90_clk),
        .CPUCLK_clk(CPUCLK_clk),
        .DDR_addr(DDR_addr[14:0]),
        .DDR_ba(DDR_ba[2:0]),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm[3:0]),
        .DDR_dq(DDR_dq[31:0]),
        .DDR_dqs_n(DDR_dqs_n[3:0]),
        .DDR_dqs_p(DDR_dqs_p[3:0]),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio[53:0]),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .I2SO_D0(processing_av_system_I2SO_D0),
        .I2S_FSYNC_OUT(processing_av_system_I2S_FSYNC_OUT),
        .I2S_SCLK(processing_av_system_I2S_SCLK),
        .IIC_0_scl_i(processing_av_system_IIC_0_SCL_I),
        .IIC_0_scl_o(processing_av_system_IIC_0_SCL_O),
        .IIC_0_scl_t(processing_av_system_IIC_0_SCL_T),
        .IIC_0_sda_i(processing_av_system_IIC_0_SDA_I),
        .IIC_0_sda_o(processing_av_system_IIC_0_SDA_O),
        .IIC_0_sda_t(processing_av_system_IIC_0_SDA_T),
        .M_AXI1_araddr(processing_av_system_M_AXI1_ARADDR),
        .M_AXI1_arprot(processing_av_system_M_AXI1_ARPROT),
        .M_AXI1_arready(processing_av_system_M_AXI1_ARREADY),
        .M_AXI1_arvalid(processing_av_system_M_AXI1_ARVALID),
        .M_AXI1_awaddr(processing_av_system_M_AXI1_AWADDR),
        .M_AXI1_awprot(processing_av_system_M_AXI1_AWPROT),
        .M_AXI1_awready(processing_av_system_M_AXI1_AWREADY),
        .M_AXI1_awvalid(processing_av_system_M_AXI1_AWVALID),
        .M_AXI1_bready(processing_av_system_M_AXI1_BREADY),
        .M_AXI1_bresp(processing_av_system_M_AXI1_BRESP),
        .M_AXI1_bvalid(processing_av_system_M_AXI1_BVALID),
        .M_AXI1_rdata(processing_av_system_M_AXI1_RDATA),
        .M_AXI1_rready(processing_av_system_M_AXI1_RREADY),
        .M_AXI1_rresp(processing_av_system_M_AXI1_RRESP),
        .M_AXI1_rvalid(processing_av_system_M_AXI1_RVALID),
        .M_AXI1_wdata(processing_av_system_M_AXI1_WDATA),
        .M_AXI1_wready(processing_av_system_M_AXI1_WREADY),
        .M_AXI1_wstrb(processing_av_system_M_AXI1_WSTRB),
        .M_AXI1_wvalid(processing_av_system_M_AXI1_WVALID),
        .M_AXI_araddr(processing_av_system_M_AXI_ARADDR),
        .M_AXI_arprot(processing_av_system_M_AXI_ARPROT),
        .M_AXI_arready(processing_av_system_M_AXI_ARREADY),
        .M_AXI_arvalid(processing_av_system_M_AXI_ARVALID),
        .M_AXI_awaddr(processing_av_system_M_AXI_AWADDR),
        .M_AXI_awprot(processing_av_system_M_AXI_AWPROT),
        .M_AXI_awready(processing_av_system_M_AXI_AWREADY),
        .M_AXI_awvalid(processing_av_system_M_AXI_AWVALID),
        .M_AXI_bready(processing_av_system_M_AXI_BREADY),
        .M_AXI_bresp(processing_av_system_M_AXI_BRESP),
        .M_AXI_bvalid(processing_av_system_M_AXI_BVALID),
        .M_AXI_rdata(processing_av_system_M_AXI_RDATA),
        .M_AXI_rready(processing_av_system_M_AXI_RREADY),
        .M_AXI_rresp(processing_av_system_M_AXI_RRESP),
        .M_AXI_rvalid(processing_av_system_M_AXI_RVALID),
        .M_AXI_wdata(processing_av_system_M_AXI_WDATA),
        .M_AXI_wready(processing_av_system_M_AXI_WREADY),
        .M_AXI_wstrb(processing_av_system_M_AXI_WSTRB),
        .M_AXI_wvalid(processing_av_system_M_AXI_WVALID),
        .PCLK_clk(clk_wiz_0_PCLK_clk),
        .PCLK_reg(processing_av_system_PCLK_reg),
        .S_AXI_araddr(z3660_0_m00_axi_ARADDR),
        .S_AXI_arburst(z3660_0_m00_axi_ARBURST),
        .S_AXI_arcache(z3660_0_m00_axi_ARCACHE),
        .S_AXI_arlen(z3660_0_m00_axi_ARLEN),
        .S_AXI_arlock(z3660_0_m00_axi_ARLOCK),
        .S_AXI_arprot(z3660_0_m00_axi_ARPROT),
        .S_AXI_arready(z3660_0_m00_axi_ARREADY),
        .S_AXI_arsize(z3660_0_m00_axi_ARSIZE),
        .S_AXI_aruser(z3660_0_m00_axi_ARUSER),
        .S_AXI_arvalid(z3660_0_m00_axi_ARVALID),
        .S_AXI_awaddr(z3660_0_m00_axi_AWADDR),
        .S_AXI_awburst(z3660_0_m00_axi_AWBURST),
        .S_AXI_awcache(z3660_0_m00_axi_AWCACHE),
        .S_AXI_awlen(z3660_0_m00_axi_AWLEN),
        .S_AXI_awlock(z3660_0_m00_axi_AWLOCK),
        .S_AXI_awprot(z3660_0_m00_axi_AWPROT),
        .S_AXI_awready(z3660_0_m00_axi_AWREADY),
        .S_AXI_awsize(z3660_0_m00_axi_AWSIZE),
        .S_AXI_awuser(z3660_0_m00_axi_AWUSER),
        .S_AXI_awvalid(z3660_0_m00_axi_AWVALID),
        .S_AXI_bready(z3660_0_m00_axi_BREADY),
        .S_AXI_bresp(z3660_0_m00_axi_BRESP),
        .S_AXI_bvalid(z3660_0_m00_axi_BVALID),
        .S_AXI_rdata(z3660_0_m00_axi_RDATA),
        .S_AXI_rlast(z3660_0_m00_axi_RLAST),
        .S_AXI_rready(z3660_0_m00_axi_RREADY),
        .S_AXI_rresp(z3660_0_m00_axi_RRESP),
        .S_AXI_rvalid(z3660_0_m00_axi_RVALID),
        .S_AXI_wdata(z3660_0_m00_axi_WDATA),
        .S_AXI_wlast(z3660_0_m00_axi_WLAST),
        .S_AXI_wready(z3660_0_m00_axi_WREADY),
        .S_AXI_wstrb(z3660_0_m00_axi_WSTRB),
        .S_AXI_wvalid(z3660_0_m00_axi_WVALID),
        .aresetn(processing_av_system_aresetn),
        .aresetn1(processing_av_system_aresetn1),
        .clk90_detected(processing_av_system_clk90_detected),
        .control_vblank(processing_av_system_control_vblank),
        .cpuclk_detected(processing_av_system_cpuclk_detected),
        .enable_clk_output(z3660_0_enable_clk_output),
        .hdmi_clk(processing_av_system_hdmi_clk),
        .hdmi_data(processing_av_system_hdmi_data),
        .hdmi_de(processing_av_system_hdmi_de),
        .hdmi_hs(processing_av_system_hdmi_hs),
        .hdmi_intn(hdmi_intn_1),
        .hdmi_vs(processing_av_system_hdmi_vs),
        .nCLKEN_clk(clk_wiz_0_nCLKEN_clk),
        .nCLKEN_reg(processing_av_system_nCLKEN_reg),
        .peripheral_reset(processing_av_system_peripheral_reset));
  design_1_z3660_0_0 z3660_0
       (.A060(A060[31:0]),
        .BP(z3660_0_BP),
        .CLK18M_clk(processing_av_system_CLK18M_clk),
        .D040(D040[31:0]),
        .PCLK_clk(processing_av_system_PCLK_reg),
        .RESET_IN(processing_av_system_peripheral_reset),
        .R_W040(R_W040),
        .S00_AXI_ACLK(processing_av_system_AXI1_clk),
        .S00_AXI_ARADDR(processing_av_system_M_AXI1_ARADDR),
        .S00_AXI_ARESETN(processing_av_system_aresetn1),
        .S00_AXI_ARPROT(processing_av_system_M_AXI1_ARPROT),
        .S00_AXI_ARREADY(processing_av_system_M_AXI1_ARREADY),
        .S00_AXI_ARVALID(processing_av_system_M_AXI1_ARVALID),
        .S00_AXI_AWADDR(processing_av_system_M_AXI1_AWADDR),
        .S00_AXI_AWPROT(processing_av_system_M_AXI1_AWPROT),
        .S00_AXI_AWREADY(processing_av_system_M_AXI1_AWREADY),
        .S00_AXI_AWVALID(processing_av_system_M_AXI1_AWVALID),
        .S00_AXI_BREADY(processing_av_system_M_AXI1_BREADY),
        .S00_AXI_BRESP(processing_av_system_M_AXI1_BRESP),
        .S00_AXI_BVALID(processing_av_system_M_AXI1_BVALID),
        .S00_AXI_RDATA(processing_av_system_M_AXI1_RDATA),
        .S00_AXI_RREADY(processing_av_system_M_AXI1_RREADY),
        .S00_AXI_RRESP(processing_av_system_M_AXI1_RRESP),
        .S00_AXI_RVALID(processing_av_system_M_AXI1_RVALID),
        .S00_AXI_WDATA(processing_av_system_M_AXI1_WDATA),
        .S00_AXI_WREADY(processing_av_system_M_AXI1_WREADY),
        .S00_AXI_WSTRB(processing_av_system_M_AXI1_WSTRB),
        .S00_AXI_WVALID(processing_av_system_M_AXI1_WVALID),
        .S01_AXI_ACLK(processing_av_system_AXI_clk),
        .S01_AXI_ARADDR(processing_av_system_M_AXI_ARADDR[4:0]),
        .S01_AXI_ARESETN(processing_av_system_aresetn),
        .S01_AXI_ARPROT(processing_av_system_M_AXI_ARPROT),
        .S01_AXI_ARREADY(processing_av_system_M_AXI_ARREADY),
        .S01_AXI_ARVALID(processing_av_system_M_AXI_ARVALID),
        .S01_AXI_AWADDR(processing_av_system_M_AXI_AWADDR[4:0]),
        .S01_AXI_AWPROT(processing_av_system_M_AXI_AWPROT),
        .S01_AXI_AWREADY(processing_av_system_M_AXI_AWREADY),
        .S01_AXI_AWVALID(processing_av_system_M_AXI_AWVALID),
        .S01_AXI_BREADY(processing_av_system_M_AXI_BREADY),
        .S01_AXI_BRESP(processing_av_system_M_AXI_BRESP),
        .S01_AXI_BVALID(processing_av_system_M_AXI_BVALID),
        .S01_AXI_RDATA(processing_av_system_M_AXI_RDATA),
        .S01_AXI_RREADY(processing_av_system_M_AXI_RREADY),
        .S01_AXI_RRESP(processing_av_system_M_AXI_RRESP),
        .S01_AXI_RVALID(processing_av_system_M_AXI_RVALID),
        .S01_AXI_WDATA(processing_av_system_M_AXI_WDATA),
        .S01_AXI_WREADY(processing_av_system_M_AXI_WREADY),
        .S01_AXI_WSTRB(processing_av_system_M_AXI_WSTRB),
        .S01_AXI_WVALID(processing_av_system_M_AXI_WVALID),
        .SIZ40(SIZ40[1:0]),
        .clk90_detected(processing_av_system_clk90_detected),
        .control_vblank(processing_av_system_control_vblank),
        .cpuclk_detected(processing_av_system_cpuclk_detected),
        .enable_clk_output(z3660_0_enable_clk_output),
        .interrupt(z3660_0_interrupt),
        .m00_axi_aclk(processing_av_system_AXI1_clk),
        .m00_axi_araddr(z3660_0_m00_axi_ARADDR),
        .m00_axi_arburst(z3660_0_m00_axi_ARBURST),
        .m00_axi_arcache(z3660_0_m00_axi_ARCACHE),
        .m00_axi_aresetn(processing_av_system_aresetn1),
        .m00_axi_arlen(z3660_0_m00_axi_ARLEN),
        .m00_axi_arlock(z3660_0_m00_axi_ARLOCK),
        .m00_axi_arprot(z3660_0_m00_axi_ARPROT),
        .m00_axi_arready(z3660_0_m00_axi_ARREADY),
        .m00_axi_arsize(z3660_0_m00_axi_ARSIZE),
        .m00_axi_aruser(z3660_0_m00_axi_ARUSER),
        .m00_axi_arvalid(z3660_0_m00_axi_ARVALID),
        .m00_axi_awaddr(z3660_0_m00_axi_AWADDR),
        .m00_axi_awburst(z3660_0_m00_axi_AWBURST),
        .m00_axi_awcache(z3660_0_m00_axi_AWCACHE),
        .m00_axi_awlen(z3660_0_m00_axi_AWLEN),
        .m00_axi_awlock(z3660_0_m00_axi_AWLOCK),
        .m00_axi_awprot(z3660_0_m00_axi_AWPROT),
        .m00_axi_awready(z3660_0_m00_axi_AWREADY),
        .m00_axi_awsize(z3660_0_m00_axi_AWSIZE),
        .m00_axi_awuser(z3660_0_m00_axi_AWUSER),
        .m00_axi_awvalid(z3660_0_m00_axi_AWVALID),
        .m00_axi_bready(z3660_0_m00_axi_BREADY),
        .m00_axi_bresp(z3660_0_m00_axi_BRESP),
        .m00_axi_bvalid(z3660_0_m00_axi_BVALID),
        .m00_axi_rdata(z3660_0_m00_axi_RDATA),
        .m00_axi_rlast(z3660_0_m00_axi_RLAST),
        .m00_axi_rready(z3660_0_m00_axi_RREADY),
        .m00_axi_rresp(z3660_0_m00_axi_RRESP),
        .m00_axi_rvalid(z3660_0_m00_axi_RVALID),
        .m00_axi_wdata(z3660_0_m00_axi_WDATA),
        .m00_axi_wlast(z3660_0_m00_axi_WLAST),
        .m00_axi_wready(z3660_0_m00_axi_WREADY),
        .m00_axi_wstrb(z3660_0_m00_axi_WSTRB),
        .m00_axi_wvalid(z3660_0_m00_axi_WVALID),
        .nCLKEN_clk(processing_av_system_nCLKEN_reg),
        .nTA(nTA),
        .nTBI(z3660_0_nTBI),
        .nTCI(nTCI_1),
        .nTEA(nTEA_1),
        .nTS(nTS_1),
        .nTS_FPGA_out(z3660_0_NU_1));
endmodule

module design_1_axi_interconnect_0_0
   (ACLK,
    ARESETN,
    M00_ACLK,
    M00_ARESETN,
    M00_AXI_araddr,
    M00_AXI_arready,
    M00_AXI_arvalid,
    M00_AXI_awaddr,
    M00_AXI_awready,
    M00_AXI_awvalid,
    M00_AXI_bready,
    M00_AXI_bresp,
    M00_AXI_bvalid,
    M00_AXI_rdata,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    M00_AXI_wdata,
    M00_AXI_wready,
    M00_AXI_wstrb,
    M00_AXI_wvalid,
    M01_ACLK,
    M01_ARESETN,
    M01_AXI_araddr,
    M01_AXI_arprot,
    M01_AXI_arready,
    M01_AXI_arvalid,
    M01_AXI_awaddr,
    M01_AXI_awprot,
    M01_AXI_awready,
    M01_AXI_awvalid,
    M01_AXI_bready,
    M01_AXI_bresp,
    M01_AXI_bvalid,
    M01_AXI_rdata,
    M01_AXI_rready,
    M01_AXI_rresp,
    M01_AXI_rvalid,
    M01_AXI_wdata,
    M01_AXI_wready,
    M01_AXI_wstrb,
    M01_AXI_wvalid,
    M02_ACLK,
    M02_ARESETN,
    M02_AXI_araddr,
    M02_AXI_arprot,
    M02_AXI_arready,
    M02_AXI_arvalid,
    M02_AXI_awaddr,
    M02_AXI_awprot,
    M02_AXI_awready,
    M02_AXI_awvalid,
    M02_AXI_bready,
    M02_AXI_bresp,
    M02_AXI_bvalid,
    M02_AXI_rdata,
    M02_AXI_rready,
    M02_AXI_rresp,
    M02_AXI_rvalid,
    M02_AXI_wdata,
    M02_AXI_wready,
    M02_AXI_wstrb,
    M02_AXI_wvalid,
    M03_ACLK,
    M03_ARESETN,
    M03_AXI_araddr,
    M03_AXI_arready,
    M03_AXI_arvalid,
    M03_AXI_awaddr,
    M03_AXI_awready,
    M03_AXI_awvalid,
    M03_AXI_bready,
    M03_AXI_bresp,
    M03_AXI_bvalid,
    M03_AXI_rdata,
    M03_AXI_rready,
    M03_AXI_rresp,
    M03_AXI_rvalid,
    M03_AXI_wdata,
    M03_AXI_wready,
    M03_AXI_wvalid,
    M04_ACLK,
    M04_ARESETN,
    M04_AXI_araddr,
    M04_AXI_arready,
    M04_AXI_arvalid,
    M04_AXI_awaddr,
    M04_AXI_awready,
    M04_AXI_awvalid,
    M04_AXI_bready,
    M04_AXI_bresp,
    M04_AXI_bvalid,
    M04_AXI_rdata,
    M04_AXI_rready,
    M04_AXI_rresp,
    M04_AXI_rvalid,
    M04_AXI_wdata,
    M04_AXI_wready,
    M04_AXI_wstrb,
    M04_AXI_wvalid,
    M05_ACLK,
    M05_ARESETN,
    M05_AXI_araddr,
    M05_AXI_arready,
    M05_AXI_arvalid,
    M05_AXI_awaddr,
    M05_AXI_awready,
    M05_AXI_awvalid,
    M05_AXI_bready,
    M05_AXI_bresp,
    M05_AXI_bvalid,
    M05_AXI_rdata,
    M05_AXI_rready,
    M05_AXI_rresp,
    M05_AXI_rvalid,
    M05_AXI_wdata,
    M05_AXI_wready,
    M05_AXI_wvalid,
    M06_ACLK,
    M06_ARESETN,
    M06_AXI_araddr,
    M06_AXI_arready,
    M06_AXI_arvalid,
    M06_AXI_awaddr,
    M06_AXI_awready,
    M06_AXI_awvalid,
    M06_AXI_bready,
    M06_AXI_bresp,
    M06_AXI_bvalid,
    M06_AXI_rdata,
    M06_AXI_rready,
    M06_AXI_rresp,
    M06_AXI_rvalid,
    M06_AXI_wdata,
    M06_AXI_wready,
    M06_AXI_wvalid,
    M07_ACLK,
    M07_ARESETN,
    M07_AXI_araddr,
    M07_AXI_arready,
    M07_AXI_arvalid,
    M07_AXI_awaddr,
    M07_AXI_awready,
    M07_AXI_awvalid,
    M07_AXI_bready,
    M07_AXI_bresp,
    M07_AXI_bvalid,
    M07_AXI_rdata,
    M07_AXI_rready,
    M07_AXI_rresp,
    M07_AXI_rvalid,
    M07_AXI_wdata,
    M07_AXI_wready,
    M07_AXI_wstrb,
    M07_AXI_wvalid,
    S00_ACLK,
    S00_ARESETN,
    S00_AXI_araddr,
    S00_AXI_arburst,
    S00_AXI_arcache,
    S00_AXI_arid,
    S00_AXI_arlen,
    S00_AXI_arlock,
    S00_AXI_arprot,
    S00_AXI_arqos,
    S00_AXI_arready,
    S00_AXI_arsize,
    S00_AXI_arvalid,
    S00_AXI_awaddr,
    S00_AXI_awburst,
    S00_AXI_awcache,
    S00_AXI_awid,
    S00_AXI_awlen,
    S00_AXI_awlock,
    S00_AXI_awprot,
    S00_AXI_awqos,
    S00_AXI_awready,
    S00_AXI_awsize,
    S00_AXI_awvalid,
    S00_AXI_bid,
    S00_AXI_bready,
    S00_AXI_bresp,
    S00_AXI_bvalid,
    S00_AXI_rdata,
    S00_AXI_rid,
    S00_AXI_rlast,
    S00_AXI_rready,
    S00_AXI_rresp,
    S00_AXI_rvalid,
    S00_AXI_wdata,
    S00_AXI_wid,
    S00_AXI_wlast,
    S00_AXI_wready,
    S00_AXI_wstrb,
    S00_AXI_wvalid);
  input ACLK;
  input ARESETN;
  input M00_ACLK;
  input M00_ARESETN;
  output [31:0]M00_AXI_araddr;
  input [0:0]M00_AXI_arready;
  output [0:0]M00_AXI_arvalid;
  output [31:0]M00_AXI_awaddr;
  input [0:0]M00_AXI_awready;
  output [0:0]M00_AXI_awvalid;
  output [0:0]M00_AXI_bready;
  input [1:0]M00_AXI_bresp;
  input [0:0]M00_AXI_bvalid;
  input [31:0]M00_AXI_rdata;
  output [0:0]M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input [0:0]M00_AXI_rvalid;
  output [31:0]M00_AXI_wdata;
  input [0:0]M00_AXI_wready;
  output [3:0]M00_AXI_wstrb;
  output [0:0]M00_AXI_wvalid;
  input M01_ACLK;
  input M01_ARESETN;
  output [31:0]M01_AXI_araddr;
  output [2:0]M01_AXI_arprot;
  input M01_AXI_arready;
  output M01_AXI_arvalid;
  output [31:0]M01_AXI_awaddr;
  output [2:0]M01_AXI_awprot;
  input M01_AXI_awready;
  output M01_AXI_awvalid;
  output M01_AXI_bready;
  input [1:0]M01_AXI_bresp;
  input M01_AXI_bvalid;
  input [31:0]M01_AXI_rdata;
  output M01_AXI_rready;
  input [1:0]M01_AXI_rresp;
  input M01_AXI_rvalid;
  output [31:0]M01_AXI_wdata;
  input M01_AXI_wready;
  output [3:0]M01_AXI_wstrb;
  output M01_AXI_wvalid;
  input M02_ACLK;
  input M02_ARESETN;
  output [31:0]M02_AXI_araddr;
  output [2:0]M02_AXI_arprot;
  input [0:0]M02_AXI_arready;
  output [0:0]M02_AXI_arvalid;
  output [31:0]M02_AXI_awaddr;
  output [2:0]M02_AXI_awprot;
  input [0:0]M02_AXI_awready;
  output [0:0]M02_AXI_awvalid;
  output [0:0]M02_AXI_bready;
  input [1:0]M02_AXI_bresp;
  input [0:0]M02_AXI_bvalid;
  input [31:0]M02_AXI_rdata;
  output [0:0]M02_AXI_rready;
  input [1:0]M02_AXI_rresp;
  input [0:0]M02_AXI_rvalid;
  output [31:0]M02_AXI_wdata;
  input [0:0]M02_AXI_wready;
  output [3:0]M02_AXI_wstrb;
  output [0:0]M02_AXI_wvalid;
  input M03_ACLK;
  input M03_ARESETN;
  output [31:0]M03_AXI_araddr;
  input [0:0]M03_AXI_arready;
  output [0:0]M03_AXI_arvalid;
  output [31:0]M03_AXI_awaddr;
  input [0:0]M03_AXI_awready;
  output [0:0]M03_AXI_awvalid;
  output [0:0]M03_AXI_bready;
  input [1:0]M03_AXI_bresp;
  input [0:0]M03_AXI_bvalid;
  input [31:0]M03_AXI_rdata;
  output [0:0]M03_AXI_rready;
  input [1:0]M03_AXI_rresp;
  input [0:0]M03_AXI_rvalid;
  output [31:0]M03_AXI_wdata;
  input [0:0]M03_AXI_wready;
  output [0:0]M03_AXI_wvalid;
  input M04_ACLK;
  input M04_ARESETN;
  output [31:0]M04_AXI_araddr;
  input [0:0]M04_AXI_arready;
  output [0:0]M04_AXI_arvalid;
  output [31:0]M04_AXI_awaddr;
  input [0:0]M04_AXI_awready;
  output [0:0]M04_AXI_awvalid;
  output [0:0]M04_AXI_bready;
  input [1:0]M04_AXI_bresp;
  input [0:0]M04_AXI_bvalid;
  input [31:0]M04_AXI_rdata;
  output [0:0]M04_AXI_rready;
  input [1:0]M04_AXI_rresp;
  input [0:0]M04_AXI_rvalid;
  output [31:0]M04_AXI_wdata;
  input [0:0]M04_AXI_wready;
  output [3:0]M04_AXI_wstrb;
  output [0:0]M04_AXI_wvalid;
  input M05_ACLK;
  input M05_ARESETN;
  output [31:0]M05_AXI_araddr;
  input [0:0]M05_AXI_arready;
  output [0:0]M05_AXI_arvalid;
  output [31:0]M05_AXI_awaddr;
  input [0:0]M05_AXI_awready;
  output [0:0]M05_AXI_awvalid;
  output [0:0]M05_AXI_bready;
  input [1:0]M05_AXI_bresp;
  input [0:0]M05_AXI_bvalid;
  input [31:0]M05_AXI_rdata;
  output [0:0]M05_AXI_rready;
  input [1:0]M05_AXI_rresp;
  input [0:0]M05_AXI_rvalid;
  output [31:0]M05_AXI_wdata;
  input [0:0]M05_AXI_wready;
  output [0:0]M05_AXI_wvalid;
  input M06_ACLK;
  input M06_ARESETN;
  output [31:0]M06_AXI_araddr;
  input [0:0]M06_AXI_arready;
  output [0:0]M06_AXI_arvalid;
  output [31:0]M06_AXI_awaddr;
  input [0:0]M06_AXI_awready;
  output [0:0]M06_AXI_awvalid;
  output [0:0]M06_AXI_bready;
  input [1:0]M06_AXI_bresp;
  input [0:0]M06_AXI_bvalid;
  input [31:0]M06_AXI_rdata;
  output [0:0]M06_AXI_rready;
  input [1:0]M06_AXI_rresp;
  input [0:0]M06_AXI_rvalid;
  output [31:0]M06_AXI_wdata;
  input [0:0]M06_AXI_wready;
  output [0:0]M06_AXI_wvalid;
  input M07_ACLK;
  input M07_ARESETN;
  output [31:0]M07_AXI_araddr;
  input M07_AXI_arready;
  output M07_AXI_arvalid;
  output [31:0]M07_AXI_awaddr;
  input M07_AXI_awready;
  output M07_AXI_awvalid;
  output M07_AXI_bready;
  input [1:0]M07_AXI_bresp;
  input M07_AXI_bvalid;
  input [31:0]M07_AXI_rdata;
  output M07_AXI_rready;
  input [1:0]M07_AXI_rresp;
  input M07_AXI_rvalid;
  output [31:0]M07_AXI_wdata;
  input M07_AXI_wready;
  output [3:0]M07_AXI_wstrb;
  output M07_AXI_wvalid;
  input S00_ACLK;
  input S00_ARESETN;
  input [31:0]S00_AXI_araddr;
  input [1:0]S00_AXI_arburst;
  input [3:0]S00_AXI_arcache;
  input [11:0]S00_AXI_arid;
  input [3:0]S00_AXI_arlen;
  input [1:0]S00_AXI_arlock;
  input [2:0]S00_AXI_arprot;
  input [3:0]S00_AXI_arqos;
  output S00_AXI_arready;
  input [2:0]S00_AXI_arsize;
  input S00_AXI_arvalid;
  input [31:0]S00_AXI_awaddr;
  input [1:0]S00_AXI_awburst;
  input [3:0]S00_AXI_awcache;
  input [11:0]S00_AXI_awid;
  input [3:0]S00_AXI_awlen;
  input [1:0]S00_AXI_awlock;
  input [2:0]S00_AXI_awprot;
  input [3:0]S00_AXI_awqos;
  output S00_AXI_awready;
  input [2:0]S00_AXI_awsize;
  input S00_AXI_awvalid;
  output [11:0]S00_AXI_bid;
  input S00_AXI_bready;
  output [1:0]S00_AXI_bresp;
  output S00_AXI_bvalid;
  output [31:0]S00_AXI_rdata;
  output [11:0]S00_AXI_rid;
  output S00_AXI_rlast;
  input S00_AXI_rready;
  output [1:0]S00_AXI_rresp;
  output S00_AXI_rvalid;
  input [31:0]S00_AXI_wdata;
  input [11:0]S00_AXI_wid;
  input S00_AXI_wlast;
  output S00_AXI_wready;
  input [3:0]S00_AXI_wstrb;
  input S00_AXI_wvalid;

  wire M00_ACLK_1;
  wire M00_ARESETN_1;
  wire M01_ACLK_1;
  wire M01_ARESETN_1;
  wire M02_ACLK_1;
  wire M02_ARESETN_1;
  wire M03_ACLK_1;
  wire M03_ARESETN_1;
  wire M04_ACLK_1;
  wire M04_ARESETN_1;
  wire M05_ACLK_1;
  wire M05_ARESETN_1;
  wire M06_ACLK_1;
  wire M06_ARESETN_1;
  wire M07_ACLK_1;
  wire M07_ARESETN_1;
  wire S00_ACLK_1;
  wire S00_ARESETN_1;
  wire axi_interconnect_0_ACLK_net;
  wire axi_interconnect_0_ARESETN_net;
  wire [31:0]axi_interconnect_0_to_s00_couplers_ARADDR;
  wire [1:0]axi_interconnect_0_to_s00_couplers_ARBURST;
  wire [3:0]axi_interconnect_0_to_s00_couplers_ARCACHE;
  wire [11:0]axi_interconnect_0_to_s00_couplers_ARID;
  wire [3:0]axi_interconnect_0_to_s00_couplers_ARLEN;
  wire [1:0]axi_interconnect_0_to_s00_couplers_ARLOCK;
  wire [2:0]axi_interconnect_0_to_s00_couplers_ARPROT;
  wire [3:0]axi_interconnect_0_to_s00_couplers_ARQOS;
  wire axi_interconnect_0_to_s00_couplers_ARREADY;
  wire [2:0]axi_interconnect_0_to_s00_couplers_ARSIZE;
  wire axi_interconnect_0_to_s00_couplers_ARVALID;
  wire [31:0]axi_interconnect_0_to_s00_couplers_AWADDR;
  wire [1:0]axi_interconnect_0_to_s00_couplers_AWBURST;
  wire [3:0]axi_interconnect_0_to_s00_couplers_AWCACHE;
  wire [11:0]axi_interconnect_0_to_s00_couplers_AWID;
  wire [3:0]axi_interconnect_0_to_s00_couplers_AWLEN;
  wire [1:0]axi_interconnect_0_to_s00_couplers_AWLOCK;
  wire [2:0]axi_interconnect_0_to_s00_couplers_AWPROT;
  wire [3:0]axi_interconnect_0_to_s00_couplers_AWQOS;
  wire axi_interconnect_0_to_s00_couplers_AWREADY;
  wire [2:0]axi_interconnect_0_to_s00_couplers_AWSIZE;
  wire axi_interconnect_0_to_s00_couplers_AWVALID;
  wire [11:0]axi_interconnect_0_to_s00_couplers_BID;
  wire axi_interconnect_0_to_s00_couplers_BREADY;
  wire [1:0]axi_interconnect_0_to_s00_couplers_BRESP;
  wire axi_interconnect_0_to_s00_couplers_BVALID;
  wire [31:0]axi_interconnect_0_to_s00_couplers_RDATA;
  wire [11:0]axi_interconnect_0_to_s00_couplers_RID;
  wire axi_interconnect_0_to_s00_couplers_RLAST;
  wire axi_interconnect_0_to_s00_couplers_RREADY;
  wire [1:0]axi_interconnect_0_to_s00_couplers_RRESP;
  wire axi_interconnect_0_to_s00_couplers_RVALID;
  wire [31:0]axi_interconnect_0_to_s00_couplers_WDATA;
  wire [11:0]axi_interconnect_0_to_s00_couplers_WID;
  wire axi_interconnect_0_to_s00_couplers_WLAST;
  wire axi_interconnect_0_to_s00_couplers_WREADY;
  wire [3:0]axi_interconnect_0_to_s00_couplers_WSTRB;
  wire axi_interconnect_0_to_s00_couplers_WVALID;
  wire [31:0]m00_couplers_to_axi_interconnect_0_ARADDR;
  wire [0:0]m00_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m00_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m00_couplers_to_axi_interconnect_0_AWADDR;
  wire [0:0]m00_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m00_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m00_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m00_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m00_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m00_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m00_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m00_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m00_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m00_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m00_couplers_to_axi_interconnect_0_WREADY;
  wire [3:0]m00_couplers_to_axi_interconnect_0_WSTRB;
  wire [0:0]m00_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m01_couplers_to_axi_interconnect_0_ARADDR;
  wire [2:0]m01_couplers_to_axi_interconnect_0_ARPROT;
  wire m01_couplers_to_axi_interconnect_0_ARREADY;
  wire m01_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m01_couplers_to_axi_interconnect_0_AWADDR;
  wire [2:0]m01_couplers_to_axi_interconnect_0_AWPROT;
  wire m01_couplers_to_axi_interconnect_0_AWREADY;
  wire m01_couplers_to_axi_interconnect_0_AWVALID;
  wire m01_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m01_couplers_to_axi_interconnect_0_BRESP;
  wire m01_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m01_couplers_to_axi_interconnect_0_RDATA;
  wire m01_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m01_couplers_to_axi_interconnect_0_RRESP;
  wire m01_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m01_couplers_to_axi_interconnect_0_WDATA;
  wire m01_couplers_to_axi_interconnect_0_WREADY;
  wire [3:0]m01_couplers_to_axi_interconnect_0_WSTRB;
  wire m01_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m02_couplers_to_axi_interconnect_0_ARADDR;
  wire [2:0]m02_couplers_to_axi_interconnect_0_ARPROT;
  wire [0:0]m02_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m02_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m02_couplers_to_axi_interconnect_0_AWADDR;
  wire [2:0]m02_couplers_to_axi_interconnect_0_AWPROT;
  wire [0:0]m02_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m02_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m02_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m02_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m02_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m02_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m02_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m02_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m02_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m02_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m02_couplers_to_axi_interconnect_0_WREADY;
  wire [3:0]m02_couplers_to_axi_interconnect_0_WSTRB;
  wire [0:0]m02_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m03_couplers_to_axi_interconnect_0_ARADDR;
  wire [0:0]m03_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m03_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m03_couplers_to_axi_interconnect_0_AWADDR;
  wire [0:0]m03_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m03_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m03_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m03_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m03_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m03_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m03_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m03_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m03_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m03_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m03_couplers_to_axi_interconnect_0_WREADY;
  wire [0:0]m03_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m04_couplers_to_axi_interconnect_0_ARADDR;
  wire [0:0]m04_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m04_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m04_couplers_to_axi_interconnect_0_AWADDR;
  wire [0:0]m04_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m04_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m04_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m04_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m04_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m04_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m04_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m04_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m04_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m04_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m04_couplers_to_axi_interconnect_0_WREADY;
  wire [3:0]m04_couplers_to_axi_interconnect_0_WSTRB;
  wire [0:0]m04_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m05_couplers_to_axi_interconnect_0_ARADDR;
  wire [0:0]m05_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m05_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m05_couplers_to_axi_interconnect_0_AWADDR;
  wire [0:0]m05_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m05_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m05_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m05_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m05_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m05_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m05_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m05_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m05_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m05_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m05_couplers_to_axi_interconnect_0_WREADY;
  wire [0:0]m05_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m06_couplers_to_axi_interconnect_0_ARADDR;
  wire [0:0]m06_couplers_to_axi_interconnect_0_ARREADY;
  wire [0:0]m06_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m06_couplers_to_axi_interconnect_0_AWADDR;
  wire [0:0]m06_couplers_to_axi_interconnect_0_AWREADY;
  wire [0:0]m06_couplers_to_axi_interconnect_0_AWVALID;
  wire [0:0]m06_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m06_couplers_to_axi_interconnect_0_BRESP;
  wire [0:0]m06_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m06_couplers_to_axi_interconnect_0_RDATA;
  wire [0:0]m06_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m06_couplers_to_axi_interconnect_0_RRESP;
  wire [0:0]m06_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m06_couplers_to_axi_interconnect_0_WDATA;
  wire [0:0]m06_couplers_to_axi_interconnect_0_WREADY;
  wire [0:0]m06_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]m07_couplers_to_axi_interconnect_0_ARADDR;
  wire m07_couplers_to_axi_interconnect_0_ARREADY;
  wire m07_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]m07_couplers_to_axi_interconnect_0_AWADDR;
  wire m07_couplers_to_axi_interconnect_0_AWREADY;
  wire m07_couplers_to_axi_interconnect_0_AWVALID;
  wire m07_couplers_to_axi_interconnect_0_BREADY;
  wire [1:0]m07_couplers_to_axi_interconnect_0_BRESP;
  wire m07_couplers_to_axi_interconnect_0_BVALID;
  wire [31:0]m07_couplers_to_axi_interconnect_0_RDATA;
  wire m07_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]m07_couplers_to_axi_interconnect_0_RRESP;
  wire m07_couplers_to_axi_interconnect_0_RVALID;
  wire [31:0]m07_couplers_to_axi_interconnect_0_WDATA;
  wire m07_couplers_to_axi_interconnect_0_WREADY;
  wire [3:0]m07_couplers_to_axi_interconnect_0_WSTRB;
  wire m07_couplers_to_axi_interconnect_0_WVALID;
  wire [31:0]s00_couplers_to_xbar_ARADDR;
  wire [2:0]s00_couplers_to_xbar_ARPROT;
  wire [0:0]s00_couplers_to_xbar_ARREADY;
  wire s00_couplers_to_xbar_ARVALID;
  wire [31:0]s00_couplers_to_xbar_AWADDR;
  wire [2:0]s00_couplers_to_xbar_AWPROT;
  wire [0:0]s00_couplers_to_xbar_AWREADY;
  wire s00_couplers_to_xbar_AWVALID;
  wire s00_couplers_to_xbar_BREADY;
  wire [1:0]s00_couplers_to_xbar_BRESP;
  wire [0:0]s00_couplers_to_xbar_BVALID;
  wire [31:0]s00_couplers_to_xbar_RDATA;
  wire s00_couplers_to_xbar_RREADY;
  wire [1:0]s00_couplers_to_xbar_RRESP;
  wire [0:0]s00_couplers_to_xbar_RVALID;
  wire [31:0]s00_couplers_to_xbar_WDATA;
  wire [0:0]s00_couplers_to_xbar_WREADY;
  wire [3:0]s00_couplers_to_xbar_WSTRB;
  wire s00_couplers_to_xbar_WVALID;
  wire [31:0]xbar_to_m00_couplers_ARADDR;
  wire [0:0]xbar_to_m00_couplers_ARREADY;
  wire [0:0]xbar_to_m00_couplers_ARVALID;
  wire [31:0]xbar_to_m00_couplers_AWADDR;
  wire [0:0]xbar_to_m00_couplers_AWREADY;
  wire [0:0]xbar_to_m00_couplers_AWVALID;
  wire [0:0]xbar_to_m00_couplers_BREADY;
  wire [1:0]xbar_to_m00_couplers_BRESP;
  wire [0:0]xbar_to_m00_couplers_BVALID;
  wire [31:0]xbar_to_m00_couplers_RDATA;
  wire [0:0]xbar_to_m00_couplers_RREADY;
  wire [1:0]xbar_to_m00_couplers_RRESP;
  wire [0:0]xbar_to_m00_couplers_RVALID;
  wire [31:0]xbar_to_m00_couplers_WDATA;
  wire [0:0]xbar_to_m00_couplers_WREADY;
  wire [3:0]xbar_to_m00_couplers_WSTRB;
  wire [0:0]xbar_to_m00_couplers_WVALID;
  wire [63:32]xbar_to_m01_couplers_ARADDR;
  wire [5:3]xbar_to_m01_couplers_ARPROT;
  wire xbar_to_m01_couplers_ARREADY;
  wire [1:1]xbar_to_m01_couplers_ARVALID;
  wire [63:32]xbar_to_m01_couplers_AWADDR;
  wire [5:3]xbar_to_m01_couplers_AWPROT;
  wire xbar_to_m01_couplers_AWREADY;
  wire [1:1]xbar_to_m01_couplers_AWVALID;
  wire [1:1]xbar_to_m01_couplers_BREADY;
  wire [1:0]xbar_to_m01_couplers_BRESP;
  wire xbar_to_m01_couplers_BVALID;
  wire [31:0]xbar_to_m01_couplers_RDATA;
  wire [1:1]xbar_to_m01_couplers_RREADY;
  wire [1:0]xbar_to_m01_couplers_RRESP;
  wire xbar_to_m01_couplers_RVALID;
  wire [63:32]xbar_to_m01_couplers_WDATA;
  wire xbar_to_m01_couplers_WREADY;
  wire [7:4]xbar_to_m01_couplers_WSTRB;
  wire [1:1]xbar_to_m01_couplers_WVALID;
  wire [95:64]xbar_to_m02_couplers_ARADDR;
  wire [8:6]xbar_to_m02_couplers_ARPROT;
  wire [0:0]xbar_to_m02_couplers_ARREADY;
  wire [2:2]xbar_to_m02_couplers_ARVALID;
  wire [95:64]xbar_to_m02_couplers_AWADDR;
  wire [8:6]xbar_to_m02_couplers_AWPROT;
  wire [0:0]xbar_to_m02_couplers_AWREADY;
  wire [2:2]xbar_to_m02_couplers_AWVALID;
  wire [2:2]xbar_to_m02_couplers_BREADY;
  wire [1:0]xbar_to_m02_couplers_BRESP;
  wire [0:0]xbar_to_m02_couplers_BVALID;
  wire [31:0]xbar_to_m02_couplers_RDATA;
  wire [2:2]xbar_to_m02_couplers_RREADY;
  wire [1:0]xbar_to_m02_couplers_RRESP;
  wire [0:0]xbar_to_m02_couplers_RVALID;
  wire [95:64]xbar_to_m02_couplers_WDATA;
  wire [0:0]xbar_to_m02_couplers_WREADY;
  wire [11:8]xbar_to_m02_couplers_WSTRB;
  wire [2:2]xbar_to_m02_couplers_WVALID;
  wire [127:96]xbar_to_m03_couplers_ARADDR;
  wire [0:0]xbar_to_m03_couplers_ARREADY;
  wire [3:3]xbar_to_m03_couplers_ARVALID;
  wire [127:96]xbar_to_m03_couplers_AWADDR;
  wire [0:0]xbar_to_m03_couplers_AWREADY;
  wire [3:3]xbar_to_m03_couplers_AWVALID;
  wire [3:3]xbar_to_m03_couplers_BREADY;
  wire [1:0]xbar_to_m03_couplers_BRESP;
  wire [0:0]xbar_to_m03_couplers_BVALID;
  wire [31:0]xbar_to_m03_couplers_RDATA;
  wire [3:3]xbar_to_m03_couplers_RREADY;
  wire [1:0]xbar_to_m03_couplers_RRESP;
  wire [0:0]xbar_to_m03_couplers_RVALID;
  wire [127:96]xbar_to_m03_couplers_WDATA;
  wire [0:0]xbar_to_m03_couplers_WREADY;
  wire [3:3]xbar_to_m03_couplers_WVALID;
  wire [159:128]xbar_to_m04_couplers_ARADDR;
  wire [0:0]xbar_to_m04_couplers_ARREADY;
  wire [4:4]xbar_to_m04_couplers_ARVALID;
  wire [159:128]xbar_to_m04_couplers_AWADDR;
  wire [0:0]xbar_to_m04_couplers_AWREADY;
  wire [4:4]xbar_to_m04_couplers_AWVALID;
  wire [4:4]xbar_to_m04_couplers_BREADY;
  wire [1:0]xbar_to_m04_couplers_BRESP;
  wire [0:0]xbar_to_m04_couplers_BVALID;
  wire [31:0]xbar_to_m04_couplers_RDATA;
  wire [4:4]xbar_to_m04_couplers_RREADY;
  wire [1:0]xbar_to_m04_couplers_RRESP;
  wire [0:0]xbar_to_m04_couplers_RVALID;
  wire [159:128]xbar_to_m04_couplers_WDATA;
  wire [0:0]xbar_to_m04_couplers_WREADY;
  wire [19:16]xbar_to_m04_couplers_WSTRB;
  wire [4:4]xbar_to_m04_couplers_WVALID;
  wire [191:160]xbar_to_m05_couplers_ARADDR;
  wire [0:0]xbar_to_m05_couplers_ARREADY;
  wire [5:5]xbar_to_m05_couplers_ARVALID;
  wire [191:160]xbar_to_m05_couplers_AWADDR;
  wire [0:0]xbar_to_m05_couplers_AWREADY;
  wire [5:5]xbar_to_m05_couplers_AWVALID;
  wire [5:5]xbar_to_m05_couplers_BREADY;
  wire [1:0]xbar_to_m05_couplers_BRESP;
  wire [0:0]xbar_to_m05_couplers_BVALID;
  wire [31:0]xbar_to_m05_couplers_RDATA;
  wire [5:5]xbar_to_m05_couplers_RREADY;
  wire [1:0]xbar_to_m05_couplers_RRESP;
  wire [0:0]xbar_to_m05_couplers_RVALID;
  wire [191:160]xbar_to_m05_couplers_WDATA;
  wire [0:0]xbar_to_m05_couplers_WREADY;
  wire [5:5]xbar_to_m05_couplers_WVALID;
  wire [223:192]xbar_to_m06_couplers_ARADDR;
  wire [0:0]xbar_to_m06_couplers_ARREADY;
  wire [6:6]xbar_to_m06_couplers_ARVALID;
  wire [223:192]xbar_to_m06_couplers_AWADDR;
  wire [0:0]xbar_to_m06_couplers_AWREADY;
  wire [6:6]xbar_to_m06_couplers_AWVALID;
  wire [6:6]xbar_to_m06_couplers_BREADY;
  wire [1:0]xbar_to_m06_couplers_BRESP;
  wire [0:0]xbar_to_m06_couplers_BVALID;
  wire [31:0]xbar_to_m06_couplers_RDATA;
  wire [6:6]xbar_to_m06_couplers_RREADY;
  wire [1:0]xbar_to_m06_couplers_RRESP;
  wire [0:0]xbar_to_m06_couplers_RVALID;
  wire [223:192]xbar_to_m06_couplers_WDATA;
  wire [0:0]xbar_to_m06_couplers_WREADY;
  wire [6:6]xbar_to_m06_couplers_WVALID;
  wire [255:224]xbar_to_m07_couplers_ARADDR;
  wire xbar_to_m07_couplers_ARREADY;
  wire [7:7]xbar_to_m07_couplers_ARVALID;
  wire [255:224]xbar_to_m07_couplers_AWADDR;
  wire xbar_to_m07_couplers_AWREADY;
  wire [7:7]xbar_to_m07_couplers_AWVALID;
  wire [7:7]xbar_to_m07_couplers_BREADY;
  wire [1:0]xbar_to_m07_couplers_BRESP;
  wire xbar_to_m07_couplers_BVALID;
  wire [31:0]xbar_to_m07_couplers_RDATA;
  wire [7:7]xbar_to_m07_couplers_RREADY;
  wire [1:0]xbar_to_m07_couplers_RRESP;
  wire xbar_to_m07_couplers_RVALID;
  wire [255:224]xbar_to_m07_couplers_WDATA;
  wire xbar_to_m07_couplers_WREADY;
  wire [31:28]xbar_to_m07_couplers_WSTRB;
  wire [7:7]xbar_to_m07_couplers_WVALID;
  wire [23:0]NLW_xbar_m_axi_arprot_UNCONNECTED;
  wire [23:0]NLW_xbar_m_axi_awprot_UNCONNECTED;
  wire [31:0]NLW_xbar_m_axi_wstrb_UNCONNECTED;

  assign M00_ACLK_1 = M00_ACLK;
  assign M00_ARESETN_1 = M00_ARESETN;
  assign M00_AXI_araddr[31:0] = m00_couplers_to_axi_interconnect_0_ARADDR;
  assign M00_AXI_arvalid[0] = m00_couplers_to_axi_interconnect_0_ARVALID;
  assign M00_AXI_awaddr[31:0] = m00_couplers_to_axi_interconnect_0_AWADDR;
  assign M00_AXI_awvalid[0] = m00_couplers_to_axi_interconnect_0_AWVALID;
  assign M00_AXI_bready[0] = m00_couplers_to_axi_interconnect_0_BREADY;
  assign M00_AXI_rready[0] = m00_couplers_to_axi_interconnect_0_RREADY;
  assign M00_AXI_wdata[31:0] = m00_couplers_to_axi_interconnect_0_WDATA;
  assign M00_AXI_wstrb[3:0] = m00_couplers_to_axi_interconnect_0_WSTRB;
  assign M00_AXI_wvalid[0] = m00_couplers_to_axi_interconnect_0_WVALID;
  assign M01_ACLK_1 = M01_ACLK;
  assign M01_ARESETN_1 = M01_ARESETN;
  assign M01_AXI_araddr[31:0] = m01_couplers_to_axi_interconnect_0_ARADDR;
  assign M01_AXI_arprot[2:0] = m01_couplers_to_axi_interconnect_0_ARPROT;
  assign M01_AXI_arvalid = m01_couplers_to_axi_interconnect_0_ARVALID;
  assign M01_AXI_awaddr[31:0] = m01_couplers_to_axi_interconnect_0_AWADDR;
  assign M01_AXI_awprot[2:0] = m01_couplers_to_axi_interconnect_0_AWPROT;
  assign M01_AXI_awvalid = m01_couplers_to_axi_interconnect_0_AWVALID;
  assign M01_AXI_bready = m01_couplers_to_axi_interconnect_0_BREADY;
  assign M01_AXI_rready = m01_couplers_to_axi_interconnect_0_RREADY;
  assign M01_AXI_wdata[31:0] = m01_couplers_to_axi_interconnect_0_WDATA;
  assign M01_AXI_wstrb[3:0] = m01_couplers_to_axi_interconnect_0_WSTRB;
  assign M01_AXI_wvalid = m01_couplers_to_axi_interconnect_0_WVALID;
  assign M02_ACLK_1 = M02_ACLK;
  assign M02_ARESETN_1 = M02_ARESETN;
  assign M02_AXI_araddr[31:0] = m02_couplers_to_axi_interconnect_0_ARADDR;
  assign M02_AXI_arprot[2:0] = m02_couplers_to_axi_interconnect_0_ARPROT;
  assign M02_AXI_arvalid[0] = m02_couplers_to_axi_interconnect_0_ARVALID;
  assign M02_AXI_awaddr[31:0] = m02_couplers_to_axi_interconnect_0_AWADDR;
  assign M02_AXI_awprot[2:0] = m02_couplers_to_axi_interconnect_0_AWPROT;
  assign M02_AXI_awvalid[0] = m02_couplers_to_axi_interconnect_0_AWVALID;
  assign M02_AXI_bready[0] = m02_couplers_to_axi_interconnect_0_BREADY;
  assign M02_AXI_rready[0] = m02_couplers_to_axi_interconnect_0_RREADY;
  assign M02_AXI_wdata[31:0] = m02_couplers_to_axi_interconnect_0_WDATA;
  assign M02_AXI_wstrb[3:0] = m02_couplers_to_axi_interconnect_0_WSTRB;
  assign M02_AXI_wvalid[0] = m02_couplers_to_axi_interconnect_0_WVALID;
  assign M03_ACLK_1 = M03_ACLK;
  assign M03_ARESETN_1 = M03_ARESETN;
  assign M03_AXI_araddr[31:0] = m03_couplers_to_axi_interconnect_0_ARADDR;
  assign M03_AXI_arvalid[0] = m03_couplers_to_axi_interconnect_0_ARVALID;
  assign M03_AXI_awaddr[31:0] = m03_couplers_to_axi_interconnect_0_AWADDR;
  assign M03_AXI_awvalid[0] = m03_couplers_to_axi_interconnect_0_AWVALID;
  assign M03_AXI_bready[0] = m03_couplers_to_axi_interconnect_0_BREADY;
  assign M03_AXI_rready[0] = m03_couplers_to_axi_interconnect_0_RREADY;
  assign M03_AXI_wdata[31:0] = m03_couplers_to_axi_interconnect_0_WDATA;
  assign M03_AXI_wvalid[0] = m03_couplers_to_axi_interconnect_0_WVALID;
  assign M04_ACLK_1 = M04_ACLK;
  assign M04_ARESETN_1 = M04_ARESETN;
  assign M04_AXI_araddr[31:0] = m04_couplers_to_axi_interconnect_0_ARADDR;
  assign M04_AXI_arvalid[0] = m04_couplers_to_axi_interconnect_0_ARVALID;
  assign M04_AXI_awaddr[31:0] = m04_couplers_to_axi_interconnect_0_AWADDR;
  assign M04_AXI_awvalid[0] = m04_couplers_to_axi_interconnect_0_AWVALID;
  assign M04_AXI_bready[0] = m04_couplers_to_axi_interconnect_0_BREADY;
  assign M04_AXI_rready[0] = m04_couplers_to_axi_interconnect_0_RREADY;
  assign M04_AXI_wdata[31:0] = m04_couplers_to_axi_interconnect_0_WDATA;
  assign M04_AXI_wstrb[3:0] = m04_couplers_to_axi_interconnect_0_WSTRB;
  assign M04_AXI_wvalid[0] = m04_couplers_to_axi_interconnect_0_WVALID;
  assign M05_ACLK_1 = M05_ACLK;
  assign M05_ARESETN_1 = M05_ARESETN;
  assign M05_AXI_araddr[31:0] = m05_couplers_to_axi_interconnect_0_ARADDR;
  assign M05_AXI_arvalid[0] = m05_couplers_to_axi_interconnect_0_ARVALID;
  assign M05_AXI_awaddr[31:0] = m05_couplers_to_axi_interconnect_0_AWADDR;
  assign M05_AXI_awvalid[0] = m05_couplers_to_axi_interconnect_0_AWVALID;
  assign M05_AXI_bready[0] = m05_couplers_to_axi_interconnect_0_BREADY;
  assign M05_AXI_rready[0] = m05_couplers_to_axi_interconnect_0_RREADY;
  assign M05_AXI_wdata[31:0] = m05_couplers_to_axi_interconnect_0_WDATA;
  assign M05_AXI_wvalid[0] = m05_couplers_to_axi_interconnect_0_WVALID;
  assign M06_ACLK_1 = M06_ACLK;
  assign M06_ARESETN_1 = M06_ARESETN;
  assign M06_AXI_araddr[31:0] = m06_couplers_to_axi_interconnect_0_ARADDR;
  assign M06_AXI_arvalid[0] = m06_couplers_to_axi_interconnect_0_ARVALID;
  assign M06_AXI_awaddr[31:0] = m06_couplers_to_axi_interconnect_0_AWADDR;
  assign M06_AXI_awvalid[0] = m06_couplers_to_axi_interconnect_0_AWVALID;
  assign M06_AXI_bready[0] = m06_couplers_to_axi_interconnect_0_BREADY;
  assign M06_AXI_rready[0] = m06_couplers_to_axi_interconnect_0_RREADY;
  assign M06_AXI_wdata[31:0] = m06_couplers_to_axi_interconnect_0_WDATA;
  assign M06_AXI_wvalid[0] = m06_couplers_to_axi_interconnect_0_WVALID;
  assign M07_ACLK_1 = M07_ACLK;
  assign M07_ARESETN_1 = M07_ARESETN;
  assign M07_AXI_araddr[31:0] = m07_couplers_to_axi_interconnect_0_ARADDR;
  assign M07_AXI_arvalid = m07_couplers_to_axi_interconnect_0_ARVALID;
  assign M07_AXI_awaddr[31:0] = m07_couplers_to_axi_interconnect_0_AWADDR;
  assign M07_AXI_awvalid = m07_couplers_to_axi_interconnect_0_AWVALID;
  assign M07_AXI_bready = m07_couplers_to_axi_interconnect_0_BREADY;
  assign M07_AXI_rready = m07_couplers_to_axi_interconnect_0_RREADY;
  assign M07_AXI_wdata[31:0] = m07_couplers_to_axi_interconnect_0_WDATA;
  assign M07_AXI_wstrb[3:0] = m07_couplers_to_axi_interconnect_0_WSTRB;
  assign M07_AXI_wvalid = m07_couplers_to_axi_interconnect_0_WVALID;
  assign S00_ACLK_1 = S00_ACLK;
  assign S00_ARESETN_1 = S00_ARESETN;
  assign S00_AXI_arready = axi_interconnect_0_to_s00_couplers_ARREADY;
  assign S00_AXI_awready = axi_interconnect_0_to_s00_couplers_AWREADY;
  assign S00_AXI_bid[11:0] = axi_interconnect_0_to_s00_couplers_BID;
  assign S00_AXI_bresp[1:0] = axi_interconnect_0_to_s00_couplers_BRESP;
  assign S00_AXI_bvalid = axi_interconnect_0_to_s00_couplers_BVALID;
  assign S00_AXI_rdata[31:0] = axi_interconnect_0_to_s00_couplers_RDATA;
  assign S00_AXI_rid[11:0] = axi_interconnect_0_to_s00_couplers_RID;
  assign S00_AXI_rlast = axi_interconnect_0_to_s00_couplers_RLAST;
  assign S00_AXI_rresp[1:0] = axi_interconnect_0_to_s00_couplers_RRESP;
  assign S00_AXI_rvalid = axi_interconnect_0_to_s00_couplers_RVALID;
  assign S00_AXI_wready = axi_interconnect_0_to_s00_couplers_WREADY;
  assign axi_interconnect_0_ACLK_net = ACLK;
  assign axi_interconnect_0_ARESETN_net = ARESETN;
  assign axi_interconnect_0_to_s00_couplers_ARADDR = S00_AXI_araddr[31:0];
  assign axi_interconnect_0_to_s00_couplers_ARBURST = S00_AXI_arburst[1:0];
  assign axi_interconnect_0_to_s00_couplers_ARCACHE = S00_AXI_arcache[3:0];
  assign axi_interconnect_0_to_s00_couplers_ARID = S00_AXI_arid[11:0];
  assign axi_interconnect_0_to_s00_couplers_ARLEN = S00_AXI_arlen[3:0];
  assign axi_interconnect_0_to_s00_couplers_ARLOCK = S00_AXI_arlock[1:0];
  assign axi_interconnect_0_to_s00_couplers_ARPROT = S00_AXI_arprot[2:0];
  assign axi_interconnect_0_to_s00_couplers_ARQOS = S00_AXI_arqos[3:0];
  assign axi_interconnect_0_to_s00_couplers_ARSIZE = S00_AXI_arsize[2:0];
  assign axi_interconnect_0_to_s00_couplers_ARVALID = S00_AXI_arvalid;
  assign axi_interconnect_0_to_s00_couplers_AWADDR = S00_AXI_awaddr[31:0];
  assign axi_interconnect_0_to_s00_couplers_AWBURST = S00_AXI_awburst[1:0];
  assign axi_interconnect_0_to_s00_couplers_AWCACHE = S00_AXI_awcache[3:0];
  assign axi_interconnect_0_to_s00_couplers_AWID = S00_AXI_awid[11:0];
  assign axi_interconnect_0_to_s00_couplers_AWLEN = S00_AXI_awlen[3:0];
  assign axi_interconnect_0_to_s00_couplers_AWLOCK = S00_AXI_awlock[1:0];
  assign axi_interconnect_0_to_s00_couplers_AWPROT = S00_AXI_awprot[2:0];
  assign axi_interconnect_0_to_s00_couplers_AWQOS = S00_AXI_awqos[3:0];
  assign axi_interconnect_0_to_s00_couplers_AWSIZE = S00_AXI_awsize[2:0];
  assign axi_interconnect_0_to_s00_couplers_AWVALID = S00_AXI_awvalid;
  assign axi_interconnect_0_to_s00_couplers_BREADY = S00_AXI_bready;
  assign axi_interconnect_0_to_s00_couplers_RREADY = S00_AXI_rready;
  assign axi_interconnect_0_to_s00_couplers_WDATA = S00_AXI_wdata[31:0];
  assign axi_interconnect_0_to_s00_couplers_WID = S00_AXI_wid[11:0];
  assign axi_interconnect_0_to_s00_couplers_WLAST = S00_AXI_wlast;
  assign axi_interconnect_0_to_s00_couplers_WSTRB = S00_AXI_wstrb[3:0];
  assign axi_interconnect_0_to_s00_couplers_WVALID = S00_AXI_wvalid;
  assign m00_couplers_to_axi_interconnect_0_ARREADY = M00_AXI_arready[0];
  assign m00_couplers_to_axi_interconnect_0_AWREADY = M00_AXI_awready[0];
  assign m00_couplers_to_axi_interconnect_0_BRESP = M00_AXI_bresp[1:0];
  assign m00_couplers_to_axi_interconnect_0_BVALID = M00_AXI_bvalid[0];
  assign m00_couplers_to_axi_interconnect_0_RDATA = M00_AXI_rdata[31:0];
  assign m00_couplers_to_axi_interconnect_0_RRESP = M00_AXI_rresp[1:0];
  assign m00_couplers_to_axi_interconnect_0_RVALID = M00_AXI_rvalid[0];
  assign m00_couplers_to_axi_interconnect_0_WREADY = M00_AXI_wready[0];
  assign m01_couplers_to_axi_interconnect_0_ARREADY = M01_AXI_arready;
  assign m01_couplers_to_axi_interconnect_0_AWREADY = M01_AXI_awready;
  assign m01_couplers_to_axi_interconnect_0_BRESP = M01_AXI_bresp[1:0];
  assign m01_couplers_to_axi_interconnect_0_BVALID = M01_AXI_bvalid;
  assign m01_couplers_to_axi_interconnect_0_RDATA = M01_AXI_rdata[31:0];
  assign m01_couplers_to_axi_interconnect_0_RRESP = M01_AXI_rresp[1:0];
  assign m01_couplers_to_axi_interconnect_0_RVALID = M01_AXI_rvalid;
  assign m01_couplers_to_axi_interconnect_0_WREADY = M01_AXI_wready;
  assign m02_couplers_to_axi_interconnect_0_ARREADY = M02_AXI_arready[0];
  assign m02_couplers_to_axi_interconnect_0_AWREADY = M02_AXI_awready[0];
  assign m02_couplers_to_axi_interconnect_0_BRESP = M02_AXI_bresp[1:0];
  assign m02_couplers_to_axi_interconnect_0_BVALID = M02_AXI_bvalid[0];
  assign m02_couplers_to_axi_interconnect_0_RDATA = M02_AXI_rdata[31:0];
  assign m02_couplers_to_axi_interconnect_0_RRESP = M02_AXI_rresp[1:0];
  assign m02_couplers_to_axi_interconnect_0_RVALID = M02_AXI_rvalid[0];
  assign m02_couplers_to_axi_interconnect_0_WREADY = M02_AXI_wready[0];
  assign m03_couplers_to_axi_interconnect_0_ARREADY = M03_AXI_arready[0];
  assign m03_couplers_to_axi_interconnect_0_AWREADY = M03_AXI_awready[0];
  assign m03_couplers_to_axi_interconnect_0_BRESP = M03_AXI_bresp[1:0];
  assign m03_couplers_to_axi_interconnect_0_BVALID = M03_AXI_bvalid[0];
  assign m03_couplers_to_axi_interconnect_0_RDATA = M03_AXI_rdata[31:0];
  assign m03_couplers_to_axi_interconnect_0_RRESP = M03_AXI_rresp[1:0];
  assign m03_couplers_to_axi_interconnect_0_RVALID = M03_AXI_rvalid[0];
  assign m03_couplers_to_axi_interconnect_0_WREADY = M03_AXI_wready[0];
  assign m04_couplers_to_axi_interconnect_0_ARREADY = M04_AXI_arready[0];
  assign m04_couplers_to_axi_interconnect_0_AWREADY = M04_AXI_awready[0];
  assign m04_couplers_to_axi_interconnect_0_BRESP = M04_AXI_bresp[1:0];
  assign m04_couplers_to_axi_interconnect_0_BVALID = M04_AXI_bvalid[0];
  assign m04_couplers_to_axi_interconnect_0_RDATA = M04_AXI_rdata[31:0];
  assign m04_couplers_to_axi_interconnect_0_RRESP = M04_AXI_rresp[1:0];
  assign m04_couplers_to_axi_interconnect_0_RVALID = M04_AXI_rvalid[0];
  assign m04_couplers_to_axi_interconnect_0_WREADY = M04_AXI_wready[0];
  assign m05_couplers_to_axi_interconnect_0_ARREADY = M05_AXI_arready[0];
  assign m05_couplers_to_axi_interconnect_0_AWREADY = M05_AXI_awready[0];
  assign m05_couplers_to_axi_interconnect_0_BRESP = M05_AXI_bresp[1:0];
  assign m05_couplers_to_axi_interconnect_0_BVALID = M05_AXI_bvalid[0];
  assign m05_couplers_to_axi_interconnect_0_RDATA = M05_AXI_rdata[31:0];
  assign m05_couplers_to_axi_interconnect_0_RRESP = M05_AXI_rresp[1:0];
  assign m05_couplers_to_axi_interconnect_0_RVALID = M05_AXI_rvalid[0];
  assign m05_couplers_to_axi_interconnect_0_WREADY = M05_AXI_wready[0];
  assign m06_couplers_to_axi_interconnect_0_ARREADY = M06_AXI_arready[0];
  assign m06_couplers_to_axi_interconnect_0_AWREADY = M06_AXI_awready[0];
  assign m06_couplers_to_axi_interconnect_0_BRESP = M06_AXI_bresp[1:0];
  assign m06_couplers_to_axi_interconnect_0_BVALID = M06_AXI_bvalid[0];
  assign m06_couplers_to_axi_interconnect_0_RDATA = M06_AXI_rdata[31:0];
  assign m06_couplers_to_axi_interconnect_0_RRESP = M06_AXI_rresp[1:0];
  assign m06_couplers_to_axi_interconnect_0_RVALID = M06_AXI_rvalid[0];
  assign m06_couplers_to_axi_interconnect_0_WREADY = M06_AXI_wready[0];
  assign m07_couplers_to_axi_interconnect_0_ARREADY = M07_AXI_arready;
  assign m07_couplers_to_axi_interconnect_0_AWREADY = M07_AXI_awready;
  assign m07_couplers_to_axi_interconnect_0_BRESP = M07_AXI_bresp[1:0];
  assign m07_couplers_to_axi_interconnect_0_BVALID = M07_AXI_bvalid;
  assign m07_couplers_to_axi_interconnect_0_RDATA = M07_AXI_rdata[31:0];
  assign m07_couplers_to_axi_interconnect_0_RRESP = M07_AXI_rresp[1:0];
  assign m07_couplers_to_axi_interconnect_0_RVALID = M07_AXI_rvalid;
  assign m07_couplers_to_axi_interconnect_0_WREADY = M07_AXI_wready;
  m00_couplers_imp_1YPJ1TK m00_couplers
       (.M_ACLK(M00_ACLK_1),
        .M_ARESETN(M00_ARESETN_1),
        .M_AXI_araddr(m00_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m00_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m00_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m00_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m00_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m00_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m00_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m00_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m00_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m00_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m00_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m00_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m00_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m00_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m00_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wstrb(m00_couplers_to_axi_interconnect_0_WSTRB),
        .M_AXI_wvalid(m00_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m00_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m00_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m00_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m00_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m00_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m00_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m00_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m00_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m00_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m00_couplers_RDATA),
        .S_AXI_rready(xbar_to_m00_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m00_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m00_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m00_couplers_WDATA),
        .S_AXI_wready(xbar_to_m00_couplers_WREADY),
        .S_AXI_wstrb(xbar_to_m00_couplers_WSTRB),
        .S_AXI_wvalid(xbar_to_m00_couplers_WVALID));
  m01_couplers_imp_4V0QOP m01_couplers
       (.M_ACLK(M01_ACLK_1),
        .M_ARESETN(M01_ARESETN_1),
        .M_AXI_araddr(m01_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arprot(m01_couplers_to_axi_interconnect_0_ARPROT),
        .M_AXI_arready(m01_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m01_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m01_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awprot(m01_couplers_to_axi_interconnect_0_AWPROT),
        .M_AXI_awready(m01_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m01_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m01_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m01_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m01_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m01_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m01_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m01_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m01_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m01_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m01_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wstrb(m01_couplers_to_axi_interconnect_0_WSTRB),
        .M_AXI_wvalid(m01_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m01_couplers_ARADDR),
        .S_AXI_arprot(xbar_to_m01_couplers_ARPROT),
        .S_AXI_arready(xbar_to_m01_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m01_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m01_couplers_AWADDR),
        .S_AXI_awprot(xbar_to_m01_couplers_AWPROT),
        .S_AXI_awready(xbar_to_m01_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m01_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m01_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m01_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m01_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m01_couplers_RDATA),
        .S_AXI_rready(xbar_to_m01_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m01_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m01_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m01_couplers_WDATA),
        .S_AXI_wready(xbar_to_m01_couplers_WREADY),
        .S_AXI_wstrb(xbar_to_m01_couplers_WSTRB),
        .S_AXI_wvalid(xbar_to_m01_couplers_WVALID));
  m02_couplers_imp_1XNAPOB m02_couplers
       (.M_ACLK(M02_ACLK_1),
        .M_ARESETN(M02_ARESETN_1),
        .M_AXI_araddr(m02_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arprot(m02_couplers_to_axi_interconnect_0_ARPROT),
        .M_AXI_arready(m02_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m02_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m02_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awprot(m02_couplers_to_axi_interconnect_0_AWPROT),
        .M_AXI_awready(m02_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m02_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m02_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m02_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m02_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m02_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m02_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m02_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m02_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m02_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m02_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wstrb(m02_couplers_to_axi_interconnect_0_WSTRB),
        .M_AXI_wvalid(m02_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m02_couplers_ARADDR),
        .S_AXI_arprot(xbar_to_m02_couplers_ARPROT),
        .S_AXI_arready(xbar_to_m02_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m02_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m02_couplers_AWADDR),
        .S_AXI_awprot(xbar_to_m02_couplers_AWPROT),
        .S_AXI_awready(xbar_to_m02_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m02_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m02_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m02_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m02_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m02_couplers_RDATA),
        .S_AXI_rready(xbar_to_m02_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m02_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m02_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m02_couplers_WDATA),
        .S_AXI_wready(xbar_to_m02_couplers_WREADY),
        .S_AXI_wstrb(xbar_to_m02_couplers_WSTRB),
        .S_AXI_wvalid(xbar_to_m02_couplers_WVALID));
  m03_couplers_imp_5MZ1QY m03_couplers
       (.M_ACLK(M03_ACLK_1),
        .M_ARESETN(M03_ARESETN_1),
        .M_AXI_araddr(m03_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m03_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m03_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m03_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m03_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m03_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m03_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m03_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m03_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m03_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m03_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m03_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m03_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m03_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m03_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wvalid(m03_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m03_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m03_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m03_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m03_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m03_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m03_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m03_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m03_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m03_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m03_couplers_RDATA),
        .S_AXI_rready(xbar_to_m03_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m03_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m03_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m03_couplers_WDATA),
        .S_AXI_wready(xbar_to_m03_couplers_WREADY),
        .S_AXI_wvalid(xbar_to_m03_couplers_WVALID));
  m04_couplers_imp_1W489VI m04_couplers
       (.M_ACLK(M04_ACLK_1),
        .M_ARESETN(M04_ARESETN_1),
        .M_AXI_araddr(m04_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m04_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m04_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m04_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m04_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m04_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m04_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m04_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m04_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m04_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m04_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m04_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m04_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m04_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m04_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wstrb(m04_couplers_to_axi_interconnect_0_WSTRB),
        .M_AXI_wvalid(m04_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m04_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m04_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m04_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m04_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m04_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m04_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m04_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m04_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m04_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m04_couplers_RDATA),
        .S_AXI_rready(xbar_to_m04_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m04_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m04_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m04_couplers_WDATA),
        .S_AXI_wready(xbar_to_m04_couplers_WREADY),
        .S_AXI_wstrb(xbar_to_m04_couplers_WSTRB),
        .S_AXI_wvalid(xbar_to_m04_couplers_WVALID));
  m05_couplers_imp_79H0DB m05_couplers
       (.M_ACLK(M05_ACLK_1),
        .M_ARESETN(M05_ARESETN_1),
        .M_AXI_araddr(m05_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m05_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m05_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m05_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m05_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m05_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m05_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m05_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m05_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m05_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m05_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m05_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m05_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m05_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m05_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wvalid(m05_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m05_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m05_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m05_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m05_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m05_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m05_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m05_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m05_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m05_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m05_couplers_RDATA),
        .S_AXI_rready(xbar_to_m05_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m05_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m05_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m05_couplers_WDATA),
        .S_AXI_wready(xbar_to_m05_couplers_WREADY),
        .S_AXI_wvalid(xbar_to_m05_couplers_WVALID));
  m06_couplers_imp_1URZLVH m06_couplers
       (.M_ACLK(M06_ACLK_1),
        .M_ARESETN(M06_ARESETN_1),
        .M_AXI_araddr(m06_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m06_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m06_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m06_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m06_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m06_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m06_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m06_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m06_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m06_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m06_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m06_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m06_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m06_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m06_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wvalid(m06_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m06_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m06_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m06_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m06_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m06_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m06_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m06_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m06_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m06_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m06_couplers_RDATA),
        .S_AXI_rready(xbar_to_m06_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m06_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m06_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m06_couplers_WDATA),
        .S_AXI_wready(xbar_to_m06_couplers_WREADY),
        .S_AXI_wvalid(xbar_to_m06_couplers_WVALID));
  m07_couplers_imp_8VDEOS m07_couplers
       (.M_ACLK(M07_ACLK_1),
        .M_ARESETN(M07_ARESETN_1),
        .M_AXI_araddr(m07_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arready(m07_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arvalid(m07_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_awaddr(m07_couplers_to_axi_interconnect_0_AWADDR),
        .M_AXI_awready(m07_couplers_to_axi_interconnect_0_AWREADY),
        .M_AXI_awvalid(m07_couplers_to_axi_interconnect_0_AWVALID),
        .M_AXI_bready(m07_couplers_to_axi_interconnect_0_BREADY),
        .M_AXI_bresp(m07_couplers_to_axi_interconnect_0_BRESP),
        .M_AXI_bvalid(m07_couplers_to_axi_interconnect_0_BVALID),
        .M_AXI_rdata(m07_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rready(m07_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(m07_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(m07_couplers_to_axi_interconnect_0_RVALID),
        .M_AXI_wdata(m07_couplers_to_axi_interconnect_0_WDATA),
        .M_AXI_wready(m07_couplers_to_axi_interconnect_0_WREADY),
        .M_AXI_wstrb(m07_couplers_to_axi_interconnect_0_WSTRB),
        .M_AXI_wvalid(m07_couplers_to_axi_interconnect_0_WVALID),
        .S_ACLK(axi_interconnect_0_ACLK_net),
        .S_ARESETN(axi_interconnect_0_ARESETN_net),
        .S_AXI_araddr(xbar_to_m07_couplers_ARADDR),
        .S_AXI_arready(xbar_to_m07_couplers_ARREADY),
        .S_AXI_arvalid(xbar_to_m07_couplers_ARVALID),
        .S_AXI_awaddr(xbar_to_m07_couplers_AWADDR),
        .S_AXI_awready(xbar_to_m07_couplers_AWREADY),
        .S_AXI_awvalid(xbar_to_m07_couplers_AWVALID),
        .S_AXI_bready(xbar_to_m07_couplers_BREADY),
        .S_AXI_bresp(xbar_to_m07_couplers_BRESP),
        .S_AXI_bvalid(xbar_to_m07_couplers_BVALID),
        .S_AXI_rdata(xbar_to_m07_couplers_RDATA),
        .S_AXI_rready(xbar_to_m07_couplers_RREADY),
        .S_AXI_rresp(xbar_to_m07_couplers_RRESP),
        .S_AXI_rvalid(xbar_to_m07_couplers_RVALID),
        .S_AXI_wdata(xbar_to_m07_couplers_WDATA),
        .S_AXI_wready(xbar_to_m07_couplers_WREADY),
        .S_AXI_wstrb(xbar_to_m07_couplers_WSTRB),
        .S_AXI_wvalid(xbar_to_m07_couplers_WVALID));
  s00_couplers_imp_271I2I s00_couplers
       (.M_ACLK(axi_interconnect_0_ACLK_net),
        .M_ARESETN(axi_interconnect_0_ARESETN_net),
        .M_AXI_araddr(s00_couplers_to_xbar_ARADDR),
        .M_AXI_arprot(s00_couplers_to_xbar_ARPROT),
        .M_AXI_arready(s00_couplers_to_xbar_ARREADY),
        .M_AXI_arvalid(s00_couplers_to_xbar_ARVALID),
        .M_AXI_awaddr(s00_couplers_to_xbar_AWADDR),
        .M_AXI_awprot(s00_couplers_to_xbar_AWPROT),
        .M_AXI_awready(s00_couplers_to_xbar_AWREADY),
        .M_AXI_awvalid(s00_couplers_to_xbar_AWVALID),
        .M_AXI_bready(s00_couplers_to_xbar_BREADY),
        .M_AXI_bresp(s00_couplers_to_xbar_BRESP),
        .M_AXI_bvalid(s00_couplers_to_xbar_BVALID),
        .M_AXI_rdata(s00_couplers_to_xbar_RDATA),
        .M_AXI_rready(s00_couplers_to_xbar_RREADY),
        .M_AXI_rresp(s00_couplers_to_xbar_RRESP),
        .M_AXI_rvalid(s00_couplers_to_xbar_RVALID),
        .M_AXI_wdata(s00_couplers_to_xbar_WDATA),
        .M_AXI_wready(s00_couplers_to_xbar_WREADY),
        .M_AXI_wstrb(s00_couplers_to_xbar_WSTRB),
        .M_AXI_wvalid(s00_couplers_to_xbar_WVALID),
        .S_ACLK(S00_ACLK_1),
        .S_ARESETN(S00_ARESETN_1),
        .S_AXI_araddr(axi_interconnect_0_to_s00_couplers_ARADDR),
        .S_AXI_arburst(axi_interconnect_0_to_s00_couplers_ARBURST),
        .S_AXI_arcache(axi_interconnect_0_to_s00_couplers_ARCACHE),
        .S_AXI_arid(axi_interconnect_0_to_s00_couplers_ARID),
        .S_AXI_arlen(axi_interconnect_0_to_s00_couplers_ARLEN),
        .S_AXI_arlock(axi_interconnect_0_to_s00_couplers_ARLOCK),
        .S_AXI_arprot(axi_interconnect_0_to_s00_couplers_ARPROT),
        .S_AXI_arqos(axi_interconnect_0_to_s00_couplers_ARQOS),
        .S_AXI_arready(axi_interconnect_0_to_s00_couplers_ARREADY),
        .S_AXI_arsize(axi_interconnect_0_to_s00_couplers_ARSIZE),
        .S_AXI_arvalid(axi_interconnect_0_to_s00_couplers_ARVALID),
        .S_AXI_awaddr(axi_interconnect_0_to_s00_couplers_AWADDR),
        .S_AXI_awburst(axi_interconnect_0_to_s00_couplers_AWBURST),
        .S_AXI_awcache(axi_interconnect_0_to_s00_couplers_AWCACHE),
        .S_AXI_awid(axi_interconnect_0_to_s00_couplers_AWID),
        .S_AXI_awlen(axi_interconnect_0_to_s00_couplers_AWLEN),
        .S_AXI_awlock(axi_interconnect_0_to_s00_couplers_AWLOCK),
        .S_AXI_awprot(axi_interconnect_0_to_s00_couplers_AWPROT),
        .S_AXI_awqos(axi_interconnect_0_to_s00_couplers_AWQOS),
        .S_AXI_awready(axi_interconnect_0_to_s00_couplers_AWREADY),
        .S_AXI_awsize(axi_interconnect_0_to_s00_couplers_AWSIZE),
        .S_AXI_awvalid(axi_interconnect_0_to_s00_couplers_AWVALID),
        .S_AXI_bid(axi_interconnect_0_to_s00_couplers_BID),
        .S_AXI_bready(axi_interconnect_0_to_s00_couplers_BREADY),
        .S_AXI_bresp(axi_interconnect_0_to_s00_couplers_BRESP),
        .S_AXI_bvalid(axi_interconnect_0_to_s00_couplers_BVALID),
        .S_AXI_rdata(axi_interconnect_0_to_s00_couplers_RDATA),
        .S_AXI_rid(axi_interconnect_0_to_s00_couplers_RID),
        .S_AXI_rlast(axi_interconnect_0_to_s00_couplers_RLAST),
        .S_AXI_rready(axi_interconnect_0_to_s00_couplers_RREADY),
        .S_AXI_rresp(axi_interconnect_0_to_s00_couplers_RRESP),
        .S_AXI_rvalid(axi_interconnect_0_to_s00_couplers_RVALID),
        .S_AXI_wdata(axi_interconnect_0_to_s00_couplers_WDATA),
        .S_AXI_wid(axi_interconnect_0_to_s00_couplers_WID),
        .S_AXI_wlast(axi_interconnect_0_to_s00_couplers_WLAST),
        .S_AXI_wready(axi_interconnect_0_to_s00_couplers_WREADY),
        .S_AXI_wstrb(axi_interconnect_0_to_s00_couplers_WSTRB),
        .S_AXI_wvalid(axi_interconnect_0_to_s00_couplers_WVALID));
  design_1_xbar_0 xbar
       (.aclk(axi_interconnect_0_ACLK_net),
        .aresetn(axi_interconnect_0_ARESETN_net),
        .m_axi_araddr({xbar_to_m07_couplers_ARADDR,xbar_to_m06_couplers_ARADDR,xbar_to_m05_couplers_ARADDR,xbar_to_m04_couplers_ARADDR,xbar_to_m03_couplers_ARADDR,xbar_to_m02_couplers_ARADDR,xbar_to_m01_couplers_ARADDR,xbar_to_m00_couplers_ARADDR}),
        .m_axi_arprot({xbar_to_m02_couplers_ARPROT,xbar_to_m01_couplers_ARPROT,NLW_xbar_m_axi_arprot_UNCONNECTED[2:0]}),
        .m_axi_arready({xbar_to_m07_couplers_ARREADY,xbar_to_m06_couplers_ARREADY,xbar_to_m05_couplers_ARREADY,xbar_to_m04_couplers_ARREADY,xbar_to_m03_couplers_ARREADY,xbar_to_m02_couplers_ARREADY,xbar_to_m01_couplers_ARREADY,xbar_to_m00_couplers_ARREADY}),
        .m_axi_arvalid({xbar_to_m07_couplers_ARVALID,xbar_to_m06_couplers_ARVALID,xbar_to_m05_couplers_ARVALID,xbar_to_m04_couplers_ARVALID,xbar_to_m03_couplers_ARVALID,xbar_to_m02_couplers_ARVALID,xbar_to_m01_couplers_ARVALID,xbar_to_m00_couplers_ARVALID}),
        .m_axi_awaddr({xbar_to_m07_couplers_AWADDR,xbar_to_m06_couplers_AWADDR,xbar_to_m05_couplers_AWADDR,xbar_to_m04_couplers_AWADDR,xbar_to_m03_couplers_AWADDR,xbar_to_m02_couplers_AWADDR,xbar_to_m01_couplers_AWADDR,xbar_to_m00_couplers_AWADDR}),
        .m_axi_awprot({xbar_to_m02_couplers_AWPROT,xbar_to_m01_couplers_AWPROT,NLW_xbar_m_axi_awprot_UNCONNECTED[2:0]}),
        .m_axi_awready({xbar_to_m07_couplers_AWREADY,xbar_to_m06_couplers_AWREADY,xbar_to_m05_couplers_AWREADY,xbar_to_m04_couplers_AWREADY,xbar_to_m03_couplers_AWREADY,xbar_to_m02_couplers_AWREADY,xbar_to_m01_couplers_AWREADY,xbar_to_m00_couplers_AWREADY}),
        .m_axi_awvalid({xbar_to_m07_couplers_AWVALID,xbar_to_m06_couplers_AWVALID,xbar_to_m05_couplers_AWVALID,xbar_to_m04_couplers_AWVALID,xbar_to_m03_couplers_AWVALID,xbar_to_m02_couplers_AWVALID,xbar_to_m01_couplers_AWVALID,xbar_to_m00_couplers_AWVALID}),
        .m_axi_bready({xbar_to_m07_couplers_BREADY,xbar_to_m06_couplers_BREADY,xbar_to_m05_couplers_BREADY,xbar_to_m04_couplers_BREADY,xbar_to_m03_couplers_BREADY,xbar_to_m02_couplers_BREADY,xbar_to_m01_couplers_BREADY,xbar_to_m00_couplers_BREADY}),
        .m_axi_bresp({xbar_to_m07_couplers_BRESP,xbar_to_m06_couplers_BRESP,xbar_to_m05_couplers_BRESP,xbar_to_m04_couplers_BRESP,xbar_to_m03_couplers_BRESP,xbar_to_m02_couplers_BRESP,xbar_to_m01_couplers_BRESP,xbar_to_m00_couplers_BRESP}),
        .m_axi_bvalid({xbar_to_m07_couplers_BVALID,xbar_to_m06_couplers_BVALID,xbar_to_m05_couplers_BVALID,xbar_to_m04_couplers_BVALID,xbar_to_m03_couplers_BVALID,xbar_to_m02_couplers_BVALID,xbar_to_m01_couplers_BVALID,xbar_to_m00_couplers_BVALID}),
        .m_axi_rdata({xbar_to_m07_couplers_RDATA,xbar_to_m06_couplers_RDATA,xbar_to_m05_couplers_RDATA,xbar_to_m04_couplers_RDATA,xbar_to_m03_couplers_RDATA,xbar_to_m02_couplers_RDATA,xbar_to_m01_couplers_RDATA,xbar_to_m00_couplers_RDATA}),
        .m_axi_rready({xbar_to_m07_couplers_RREADY,xbar_to_m06_couplers_RREADY,xbar_to_m05_couplers_RREADY,xbar_to_m04_couplers_RREADY,xbar_to_m03_couplers_RREADY,xbar_to_m02_couplers_RREADY,xbar_to_m01_couplers_RREADY,xbar_to_m00_couplers_RREADY}),
        .m_axi_rresp({xbar_to_m07_couplers_RRESP,xbar_to_m06_couplers_RRESP,xbar_to_m05_couplers_RRESP,xbar_to_m04_couplers_RRESP,xbar_to_m03_couplers_RRESP,xbar_to_m02_couplers_RRESP,xbar_to_m01_couplers_RRESP,xbar_to_m00_couplers_RRESP}),
        .m_axi_rvalid({xbar_to_m07_couplers_RVALID,xbar_to_m06_couplers_RVALID,xbar_to_m05_couplers_RVALID,xbar_to_m04_couplers_RVALID,xbar_to_m03_couplers_RVALID,xbar_to_m02_couplers_RVALID,xbar_to_m01_couplers_RVALID,xbar_to_m00_couplers_RVALID}),
        .m_axi_wdata({xbar_to_m07_couplers_WDATA,xbar_to_m06_couplers_WDATA,xbar_to_m05_couplers_WDATA,xbar_to_m04_couplers_WDATA,xbar_to_m03_couplers_WDATA,xbar_to_m02_couplers_WDATA,xbar_to_m01_couplers_WDATA,xbar_to_m00_couplers_WDATA}),
        .m_axi_wready({xbar_to_m07_couplers_WREADY,xbar_to_m06_couplers_WREADY,xbar_to_m05_couplers_WREADY,xbar_to_m04_couplers_WREADY,xbar_to_m03_couplers_WREADY,xbar_to_m02_couplers_WREADY,xbar_to_m01_couplers_WREADY,xbar_to_m00_couplers_WREADY}),
        .m_axi_wstrb({xbar_to_m07_couplers_WSTRB,NLW_xbar_m_axi_wstrb_UNCONNECTED[27:20],xbar_to_m04_couplers_WSTRB,NLW_xbar_m_axi_wstrb_UNCONNECTED[15:12],xbar_to_m02_couplers_WSTRB,xbar_to_m01_couplers_WSTRB,xbar_to_m00_couplers_WSTRB}),
        .m_axi_wvalid({xbar_to_m07_couplers_WVALID,xbar_to_m06_couplers_WVALID,xbar_to_m05_couplers_WVALID,xbar_to_m04_couplers_WVALID,xbar_to_m03_couplers_WVALID,xbar_to_m02_couplers_WVALID,xbar_to_m01_couplers_WVALID,xbar_to_m00_couplers_WVALID}),
        .s_axi_araddr(s00_couplers_to_xbar_ARADDR),
        .s_axi_arprot(s00_couplers_to_xbar_ARPROT),
        .s_axi_arready(s00_couplers_to_xbar_ARREADY),
        .s_axi_arvalid(s00_couplers_to_xbar_ARVALID),
        .s_axi_awaddr(s00_couplers_to_xbar_AWADDR),
        .s_axi_awprot(s00_couplers_to_xbar_AWPROT),
        .s_axi_awready(s00_couplers_to_xbar_AWREADY),
        .s_axi_awvalid(s00_couplers_to_xbar_AWVALID),
        .s_axi_bready(s00_couplers_to_xbar_BREADY),
        .s_axi_bresp(s00_couplers_to_xbar_BRESP),
        .s_axi_bvalid(s00_couplers_to_xbar_BVALID),
        .s_axi_rdata(s00_couplers_to_xbar_RDATA),
        .s_axi_rready(s00_couplers_to_xbar_RREADY),
        .s_axi_rresp(s00_couplers_to_xbar_RRESP),
        .s_axi_rvalid(s00_couplers_to_xbar_RVALID),
        .s_axi_wdata(s00_couplers_to_xbar_WDATA),
        .s_axi_wready(s00_couplers_to_xbar_WREADY),
        .s_axi_wstrb(s00_couplers_to_xbar_WSTRB),
        .s_axi_wvalid(s00_couplers_to_xbar_WVALID));
endmodule

module design_1_axi_interconnect_0_3
   (ACLK,
    ARESETN,
    M00_ACLK,
    M00_ARESETN,
    M00_AXI_araddr,
    M00_AXI_arburst,
    M00_AXI_arcache,
    M00_AXI_arlen,
    M00_AXI_arlock,
    M00_AXI_arprot,
    M00_AXI_arqos,
    M00_AXI_arready,
    M00_AXI_arsize,
    M00_AXI_arvalid,
    M00_AXI_rdata,
    M00_AXI_rlast,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    S00_ACLK,
    S00_ARESETN,
    S00_AXI_araddr,
    S00_AXI_arburst,
    S00_AXI_arcache,
    S00_AXI_arlen,
    S00_AXI_arprot,
    S00_AXI_arready,
    S00_AXI_arsize,
    S00_AXI_aruser,
    S00_AXI_arvalid,
    S00_AXI_rdata,
    S00_AXI_rlast,
    S00_AXI_rready,
    S00_AXI_rresp,
    S00_AXI_rvalid);
  input ACLK;
  input ARESETN;
  input M00_ACLK;
  input M00_ARESETN;
  output [31:0]M00_AXI_araddr;
  output [1:0]M00_AXI_arburst;
  output [3:0]M00_AXI_arcache;
  output [3:0]M00_AXI_arlen;
  output [1:0]M00_AXI_arlock;
  output [2:0]M00_AXI_arprot;
  output [3:0]M00_AXI_arqos;
  input M00_AXI_arready;
  output [2:0]M00_AXI_arsize;
  output M00_AXI_arvalid;
  input [31:0]M00_AXI_rdata;
  input M00_AXI_rlast;
  output M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input M00_AXI_rvalid;
  input S00_ACLK;
  input S00_ARESETN;
  input [31:0]S00_AXI_araddr;
  input [1:0]S00_AXI_arburst;
  input [3:0]S00_AXI_arcache;
  input [7:0]S00_AXI_arlen;
  input [2:0]S00_AXI_arprot;
  output S00_AXI_arready;
  input [2:0]S00_AXI_arsize;
  input [3:0]S00_AXI_aruser;
  input S00_AXI_arvalid;
  output [31:0]S00_AXI_rdata;
  output S00_AXI_rlast;
  input S00_AXI_rready;
  output [1:0]S00_AXI_rresp;
  output S00_AXI_rvalid;

  wire S00_ACLK_1;
  wire S00_ARESETN_1;
  wire axi_interconnect_0_ACLK_net;
  wire axi_interconnect_0_ARESETN_net;
  wire [31:0]axi_interconnect_0_to_s00_couplers_ARADDR;
  wire [1:0]axi_interconnect_0_to_s00_couplers_ARBURST;
  wire [3:0]axi_interconnect_0_to_s00_couplers_ARCACHE;
  wire [7:0]axi_interconnect_0_to_s00_couplers_ARLEN;
  wire [2:0]axi_interconnect_0_to_s00_couplers_ARPROT;
  wire axi_interconnect_0_to_s00_couplers_ARREADY;
  wire [2:0]axi_interconnect_0_to_s00_couplers_ARSIZE;
  wire [3:0]axi_interconnect_0_to_s00_couplers_ARUSER;
  wire axi_interconnect_0_to_s00_couplers_ARVALID;
  wire [31:0]axi_interconnect_0_to_s00_couplers_RDATA;
  wire axi_interconnect_0_to_s00_couplers_RLAST;
  wire axi_interconnect_0_to_s00_couplers_RREADY;
  wire [1:0]axi_interconnect_0_to_s00_couplers_RRESP;
  wire axi_interconnect_0_to_s00_couplers_RVALID;
  wire [31:0]s00_couplers_to_axi_interconnect_0_ARADDR;
  wire [1:0]s00_couplers_to_axi_interconnect_0_ARBURST;
  wire [3:0]s00_couplers_to_axi_interconnect_0_ARCACHE;
  wire [3:0]s00_couplers_to_axi_interconnect_0_ARLEN;
  wire [1:0]s00_couplers_to_axi_interconnect_0_ARLOCK;
  wire [2:0]s00_couplers_to_axi_interconnect_0_ARPROT;
  wire [3:0]s00_couplers_to_axi_interconnect_0_ARQOS;
  wire s00_couplers_to_axi_interconnect_0_ARREADY;
  wire [2:0]s00_couplers_to_axi_interconnect_0_ARSIZE;
  wire s00_couplers_to_axi_interconnect_0_ARVALID;
  wire [31:0]s00_couplers_to_axi_interconnect_0_RDATA;
  wire s00_couplers_to_axi_interconnect_0_RLAST;
  wire s00_couplers_to_axi_interconnect_0_RREADY;
  wire [1:0]s00_couplers_to_axi_interconnect_0_RRESP;
  wire s00_couplers_to_axi_interconnect_0_RVALID;

  assign M00_AXI_araddr[31:0] = s00_couplers_to_axi_interconnect_0_ARADDR;
  assign M00_AXI_arburst[1:0] = s00_couplers_to_axi_interconnect_0_ARBURST;
  assign M00_AXI_arcache[3:0] = s00_couplers_to_axi_interconnect_0_ARCACHE;
  assign M00_AXI_arlen[3:0] = s00_couplers_to_axi_interconnect_0_ARLEN;
  assign M00_AXI_arlock[1:0] = s00_couplers_to_axi_interconnect_0_ARLOCK;
  assign M00_AXI_arprot[2:0] = s00_couplers_to_axi_interconnect_0_ARPROT;
  assign M00_AXI_arqos[3:0] = s00_couplers_to_axi_interconnect_0_ARQOS;
  assign M00_AXI_arsize[2:0] = s00_couplers_to_axi_interconnect_0_ARSIZE;
  assign M00_AXI_arvalid = s00_couplers_to_axi_interconnect_0_ARVALID;
  assign M00_AXI_rready = s00_couplers_to_axi_interconnect_0_RREADY;
  assign S00_ACLK_1 = S00_ACLK;
  assign S00_ARESETN_1 = S00_ARESETN;
  assign S00_AXI_arready = axi_interconnect_0_to_s00_couplers_ARREADY;
  assign S00_AXI_rdata[31:0] = axi_interconnect_0_to_s00_couplers_RDATA;
  assign S00_AXI_rlast = axi_interconnect_0_to_s00_couplers_RLAST;
  assign S00_AXI_rresp[1:0] = axi_interconnect_0_to_s00_couplers_RRESP;
  assign S00_AXI_rvalid = axi_interconnect_0_to_s00_couplers_RVALID;
  assign axi_interconnect_0_ACLK_net = M00_ACLK;
  assign axi_interconnect_0_ARESETN_net = M00_ARESETN;
  assign axi_interconnect_0_to_s00_couplers_ARADDR = S00_AXI_araddr[31:0];
  assign axi_interconnect_0_to_s00_couplers_ARBURST = S00_AXI_arburst[1:0];
  assign axi_interconnect_0_to_s00_couplers_ARCACHE = S00_AXI_arcache[3:0];
  assign axi_interconnect_0_to_s00_couplers_ARLEN = S00_AXI_arlen[7:0];
  assign axi_interconnect_0_to_s00_couplers_ARPROT = S00_AXI_arprot[2:0];
  assign axi_interconnect_0_to_s00_couplers_ARSIZE = S00_AXI_arsize[2:0];
  assign axi_interconnect_0_to_s00_couplers_ARUSER = S00_AXI_aruser[3:0];
  assign axi_interconnect_0_to_s00_couplers_ARVALID = S00_AXI_arvalid;
  assign axi_interconnect_0_to_s00_couplers_RREADY = S00_AXI_rready;
  assign s00_couplers_to_axi_interconnect_0_ARREADY = M00_AXI_arready;
  assign s00_couplers_to_axi_interconnect_0_RDATA = M00_AXI_rdata[31:0];
  assign s00_couplers_to_axi_interconnect_0_RLAST = M00_AXI_rlast;
  assign s00_couplers_to_axi_interconnect_0_RRESP = M00_AXI_rresp[1:0];
  assign s00_couplers_to_axi_interconnect_0_RVALID = M00_AXI_rvalid;
  s00_couplers_imp_GW5FJ5 s00_couplers
       (.M_ACLK(axi_interconnect_0_ACLK_net),
        .M_ARESETN(axi_interconnect_0_ARESETN_net),
        .M_AXI_araddr(s00_couplers_to_axi_interconnect_0_ARADDR),
        .M_AXI_arburst(s00_couplers_to_axi_interconnect_0_ARBURST),
        .M_AXI_arcache(s00_couplers_to_axi_interconnect_0_ARCACHE),
        .M_AXI_arlen(s00_couplers_to_axi_interconnect_0_ARLEN),
        .M_AXI_arlock(s00_couplers_to_axi_interconnect_0_ARLOCK),
        .M_AXI_arprot(s00_couplers_to_axi_interconnect_0_ARPROT),
        .M_AXI_arqos(s00_couplers_to_axi_interconnect_0_ARQOS),
        .M_AXI_arready(s00_couplers_to_axi_interconnect_0_ARREADY),
        .M_AXI_arsize(s00_couplers_to_axi_interconnect_0_ARSIZE),
        .M_AXI_arvalid(s00_couplers_to_axi_interconnect_0_ARVALID),
        .M_AXI_rdata(s00_couplers_to_axi_interconnect_0_RDATA),
        .M_AXI_rlast(s00_couplers_to_axi_interconnect_0_RLAST),
        .M_AXI_rready(s00_couplers_to_axi_interconnect_0_RREADY),
        .M_AXI_rresp(s00_couplers_to_axi_interconnect_0_RRESP),
        .M_AXI_rvalid(s00_couplers_to_axi_interconnect_0_RVALID),
        .S_ACLK(S00_ACLK_1),
        .S_ARESETN(S00_ARESETN_1),
        .S_AXI_araddr(axi_interconnect_0_to_s00_couplers_ARADDR),
        .S_AXI_arburst(axi_interconnect_0_to_s00_couplers_ARBURST),
        .S_AXI_arcache(axi_interconnect_0_to_s00_couplers_ARCACHE),
        .S_AXI_arlen(axi_interconnect_0_to_s00_couplers_ARLEN),
        .S_AXI_arprot(axi_interconnect_0_to_s00_couplers_ARPROT),
        .S_AXI_arready(axi_interconnect_0_to_s00_couplers_ARREADY),
        .S_AXI_arsize(axi_interconnect_0_to_s00_couplers_ARSIZE),
        .S_AXI_aruser(axi_interconnect_0_to_s00_couplers_ARUSER),
        .S_AXI_arvalid(axi_interconnect_0_to_s00_couplers_ARVALID),
        .S_AXI_rdata(axi_interconnect_0_to_s00_couplers_RDATA),
        .S_AXI_rlast(axi_interconnect_0_to_s00_couplers_RLAST),
        .S_AXI_rready(axi_interconnect_0_to_s00_couplers_RREADY),
        .S_AXI_rresp(axi_interconnect_0_to_s00_couplers_RRESP),
        .S_AXI_rvalid(axi_interconnect_0_to_s00_couplers_RVALID));
endmodule

module design_1_axi_interconnect_1_0
   (ACLK,
    ARESETN,
    M00_ACLK,
    M00_ARESETN,
    M00_AXI_araddr,
    M00_AXI_arburst,
    M00_AXI_arcache,
    M00_AXI_arlen,
    M00_AXI_arlock,
    M00_AXI_arprot,
    M00_AXI_arqos,
    M00_AXI_arready,
    M00_AXI_arsize,
    M00_AXI_arvalid,
    M00_AXI_rdata,
    M00_AXI_rlast,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    S00_ACLK,
    S00_ARESETN,
    S00_AXI_araddr,
    S00_AXI_arburst,
    S00_AXI_arcache,
    S00_AXI_arlen,
    S00_AXI_arprot,
    S00_AXI_arready,
    S00_AXI_arsize,
    S00_AXI_arvalid,
    S00_AXI_rdata,
    S00_AXI_rlast,
    S00_AXI_rready,
    S00_AXI_rresp,
    S00_AXI_rvalid);
  input ACLK;
  input ARESETN;
  input M00_ACLK;
  input M00_ARESETN;
  output [31:0]M00_AXI_araddr;
  output [1:0]M00_AXI_arburst;
  output [3:0]M00_AXI_arcache;
  output [3:0]M00_AXI_arlen;
  output [1:0]M00_AXI_arlock;
  output [2:0]M00_AXI_arprot;
  output [3:0]M00_AXI_arqos;
  input M00_AXI_arready;
  output [2:0]M00_AXI_arsize;
  output M00_AXI_arvalid;
  input [31:0]M00_AXI_rdata;
  input M00_AXI_rlast;
  output M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input M00_AXI_rvalid;
  input S00_ACLK;
  input S00_ARESETN;
  input [31:0]S00_AXI_araddr;
  input [1:0]S00_AXI_arburst;
  input [3:0]S00_AXI_arcache;
  input [7:0]S00_AXI_arlen;
  input [2:0]S00_AXI_arprot;
  output S00_AXI_arready;
  input [2:0]S00_AXI_arsize;
  input S00_AXI_arvalid;
  output [31:0]S00_AXI_rdata;
  output S00_AXI_rlast;
  input S00_AXI_rready;
  output [1:0]S00_AXI_rresp;
  output S00_AXI_rvalid;

  wire S00_ACLK_1;
  wire S00_ARESETN_1;
  wire axi_interconnect_1_ACLK_net;
  wire axi_interconnect_1_ARESETN_net;
  wire [31:0]axi_interconnect_1_to_s00_couplers_ARADDR;
  wire [1:0]axi_interconnect_1_to_s00_couplers_ARBURST;
  wire [3:0]axi_interconnect_1_to_s00_couplers_ARCACHE;
  wire [7:0]axi_interconnect_1_to_s00_couplers_ARLEN;
  wire [2:0]axi_interconnect_1_to_s00_couplers_ARPROT;
  wire axi_interconnect_1_to_s00_couplers_ARREADY;
  wire [2:0]axi_interconnect_1_to_s00_couplers_ARSIZE;
  wire axi_interconnect_1_to_s00_couplers_ARVALID;
  wire [31:0]axi_interconnect_1_to_s00_couplers_RDATA;
  wire axi_interconnect_1_to_s00_couplers_RLAST;
  wire axi_interconnect_1_to_s00_couplers_RREADY;
  wire [1:0]axi_interconnect_1_to_s00_couplers_RRESP;
  wire axi_interconnect_1_to_s00_couplers_RVALID;
  wire [31:0]s00_couplers_to_axi_interconnect_1_ARADDR;
  wire [1:0]s00_couplers_to_axi_interconnect_1_ARBURST;
  wire [3:0]s00_couplers_to_axi_interconnect_1_ARCACHE;
  wire [3:0]s00_couplers_to_axi_interconnect_1_ARLEN;
  wire [1:0]s00_couplers_to_axi_interconnect_1_ARLOCK;
  wire [2:0]s00_couplers_to_axi_interconnect_1_ARPROT;
  wire [3:0]s00_couplers_to_axi_interconnect_1_ARQOS;
  wire s00_couplers_to_axi_interconnect_1_ARREADY;
  wire [2:0]s00_couplers_to_axi_interconnect_1_ARSIZE;
  wire s00_couplers_to_axi_interconnect_1_ARVALID;
  wire [31:0]s00_couplers_to_axi_interconnect_1_RDATA;
  wire s00_couplers_to_axi_interconnect_1_RLAST;
  wire s00_couplers_to_axi_interconnect_1_RREADY;
  wire [1:0]s00_couplers_to_axi_interconnect_1_RRESP;
  wire s00_couplers_to_axi_interconnect_1_RVALID;

  assign M00_AXI_araddr[31:0] = s00_couplers_to_axi_interconnect_1_ARADDR;
  assign M00_AXI_arburst[1:0] = s00_couplers_to_axi_interconnect_1_ARBURST;
  assign M00_AXI_arcache[3:0] = s00_couplers_to_axi_interconnect_1_ARCACHE;
  assign M00_AXI_arlen[3:0] = s00_couplers_to_axi_interconnect_1_ARLEN;
  assign M00_AXI_arlock[1:0] = s00_couplers_to_axi_interconnect_1_ARLOCK;
  assign M00_AXI_arprot[2:0] = s00_couplers_to_axi_interconnect_1_ARPROT;
  assign M00_AXI_arqos[3:0] = s00_couplers_to_axi_interconnect_1_ARQOS;
  assign M00_AXI_arsize[2:0] = s00_couplers_to_axi_interconnect_1_ARSIZE;
  assign M00_AXI_arvalid = s00_couplers_to_axi_interconnect_1_ARVALID;
  assign M00_AXI_rready = s00_couplers_to_axi_interconnect_1_RREADY;
  assign S00_ACLK_1 = S00_ACLK;
  assign S00_ARESETN_1 = S00_ARESETN;
  assign S00_AXI_arready = axi_interconnect_1_to_s00_couplers_ARREADY;
  assign S00_AXI_rdata[31:0] = axi_interconnect_1_to_s00_couplers_RDATA;
  assign S00_AXI_rlast = axi_interconnect_1_to_s00_couplers_RLAST;
  assign S00_AXI_rresp[1:0] = axi_interconnect_1_to_s00_couplers_RRESP;
  assign S00_AXI_rvalid = axi_interconnect_1_to_s00_couplers_RVALID;
  assign axi_interconnect_1_ACLK_net = M00_ACLK;
  assign axi_interconnect_1_ARESETN_net = M00_ARESETN;
  assign axi_interconnect_1_to_s00_couplers_ARADDR = S00_AXI_araddr[31:0];
  assign axi_interconnect_1_to_s00_couplers_ARBURST = S00_AXI_arburst[1:0];
  assign axi_interconnect_1_to_s00_couplers_ARCACHE = S00_AXI_arcache[3:0];
  assign axi_interconnect_1_to_s00_couplers_ARLEN = S00_AXI_arlen[7:0];
  assign axi_interconnect_1_to_s00_couplers_ARPROT = S00_AXI_arprot[2:0];
  assign axi_interconnect_1_to_s00_couplers_ARSIZE = S00_AXI_arsize[2:0];
  assign axi_interconnect_1_to_s00_couplers_ARVALID = S00_AXI_arvalid;
  assign axi_interconnect_1_to_s00_couplers_RREADY = S00_AXI_rready;
  assign s00_couplers_to_axi_interconnect_1_ARREADY = M00_AXI_arready;
  assign s00_couplers_to_axi_interconnect_1_RDATA = M00_AXI_rdata[31:0];
  assign s00_couplers_to_axi_interconnect_1_RLAST = M00_AXI_rlast;
  assign s00_couplers_to_axi_interconnect_1_RRESP = M00_AXI_rresp[1:0];
  assign s00_couplers_to_axi_interconnect_1_RVALID = M00_AXI_rvalid;
  s00_couplers_imp_17YN31X s00_couplers
       (.M_ACLK(axi_interconnect_1_ACLK_net),
        .M_ARESETN(axi_interconnect_1_ARESETN_net),
        .M_AXI_araddr(s00_couplers_to_axi_interconnect_1_ARADDR),
        .M_AXI_arburst(s00_couplers_to_axi_interconnect_1_ARBURST),
        .M_AXI_arcache(s00_couplers_to_axi_interconnect_1_ARCACHE),
        .M_AXI_arlen(s00_couplers_to_axi_interconnect_1_ARLEN),
        .M_AXI_arlock(s00_couplers_to_axi_interconnect_1_ARLOCK),
        .M_AXI_arprot(s00_couplers_to_axi_interconnect_1_ARPROT),
        .M_AXI_arqos(s00_couplers_to_axi_interconnect_1_ARQOS),
        .M_AXI_arready(s00_couplers_to_axi_interconnect_1_ARREADY),
        .M_AXI_arsize(s00_couplers_to_axi_interconnect_1_ARSIZE),
        .M_AXI_arvalid(s00_couplers_to_axi_interconnect_1_ARVALID),
        .M_AXI_rdata(s00_couplers_to_axi_interconnect_1_RDATA),
        .M_AXI_rlast(s00_couplers_to_axi_interconnect_1_RLAST),
        .M_AXI_rready(s00_couplers_to_axi_interconnect_1_RREADY),
        .M_AXI_rresp(s00_couplers_to_axi_interconnect_1_RRESP),
        .M_AXI_rvalid(s00_couplers_to_axi_interconnect_1_RVALID),
        .S_ACLK(S00_ACLK_1),
        .S_ARESETN(S00_ARESETN_1),
        .S_AXI_araddr(axi_interconnect_1_to_s00_couplers_ARADDR),
        .S_AXI_arburst(axi_interconnect_1_to_s00_couplers_ARBURST),
        .S_AXI_arcache(axi_interconnect_1_to_s00_couplers_ARCACHE),
        .S_AXI_arlen(axi_interconnect_1_to_s00_couplers_ARLEN),
        .S_AXI_arprot(axi_interconnect_1_to_s00_couplers_ARPROT),
        .S_AXI_arready(axi_interconnect_1_to_s00_couplers_ARREADY),
        .S_AXI_arsize(axi_interconnect_1_to_s00_couplers_ARSIZE),
        .S_AXI_arvalid(axi_interconnect_1_to_s00_couplers_ARVALID),
        .S_AXI_rdata(axi_interconnect_1_to_s00_couplers_RDATA),
        .S_AXI_rlast(axi_interconnect_1_to_s00_couplers_RLAST),
        .S_AXI_rready(axi_interconnect_1_to_s00_couplers_RREADY),
        .S_AXI_rresp(axi_interconnect_1_to_s00_couplers_RRESP),
        .S_AXI_rvalid(axi_interconnect_1_to_s00_couplers_RVALID));
endmodule

module interconnect_matrix_imp_1Q897BS
   (M00_AXI_araddr,
    M00_AXI_arready,
    M00_AXI_arvalid,
    M00_AXI_awaddr,
    M00_AXI_awready,
    M00_AXI_awvalid,
    M00_AXI_bready,
    M00_AXI_bresp,
    M00_AXI_bvalid,
    M00_AXI_rdata,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    M00_AXI_wdata,
    M00_AXI_wready,
    M00_AXI_wstrb,
    M00_AXI_wvalid,
    M01_AXI_araddr,
    M01_AXI_arprot,
    M01_AXI_arready,
    M01_AXI_arvalid,
    M01_AXI_awaddr,
    M01_AXI_awprot,
    M01_AXI_awready,
    M01_AXI_awvalid,
    M01_AXI_bready,
    M01_AXI_bresp,
    M01_AXI_bvalid,
    M01_AXI_rdata,
    M01_AXI_rready,
    M01_AXI_rresp,
    M01_AXI_rvalid,
    M01_AXI_wdata,
    M01_AXI_wready,
    M01_AXI_wstrb,
    M01_AXI_wvalid,
    M03_AXI_araddr,
    M03_AXI_arprot,
    M03_AXI_arready,
    M03_AXI_arvalid,
    M03_AXI_awaddr,
    M03_AXI_awprot,
    M03_AXI_awready,
    M03_AXI_awvalid,
    M03_AXI_bready,
    M03_AXI_bresp,
    M03_AXI_bvalid,
    M03_AXI_rdata,
    M03_AXI_rready,
    M03_AXI_rresp,
    M03_AXI_rvalid,
    M03_AXI_wdata,
    M03_AXI_wready,
    M03_AXI_wstrb,
    M03_AXI_wvalid,
    M04_AXI_araddr,
    M04_AXI_arready,
    M04_AXI_arvalid,
    M04_AXI_awaddr,
    M04_AXI_awready,
    M04_AXI_awvalid,
    M04_AXI_bready,
    M04_AXI_bresp,
    M04_AXI_bvalid,
    M04_AXI_rdata,
    M04_AXI_rready,
    M04_AXI_rresp,
    M04_AXI_rvalid,
    M04_AXI_wdata,
    M04_AXI_wready,
    M04_AXI_wvalid,
    M05_AXI_araddr,
    M05_AXI_arready,
    M05_AXI_arvalid,
    M05_AXI_awaddr,
    M05_AXI_awready,
    M05_AXI_awvalid,
    M05_AXI_bready,
    M05_AXI_bresp,
    M05_AXI_bvalid,
    M05_AXI_rdata,
    M05_AXI_rready,
    M05_AXI_rresp,
    M05_AXI_rvalid,
    M05_AXI_wdata,
    M05_AXI_wready,
    M05_AXI_wstrb,
    M05_AXI_wvalid,
    M06_AXI_araddr,
    M06_AXI_arready,
    M06_AXI_arvalid,
    M06_AXI_awaddr,
    M06_AXI_awready,
    M06_AXI_awvalid,
    M06_AXI_bready,
    M06_AXI_bresp,
    M06_AXI_bvalid,
    M06_AXI_rdata,
    M06_AXI_rready,
    M06_AXI_rresp,
    M06_AXI_rvalid,
    M06_AXI_wdata,
    M06_AXI_wready,
    M06_AXI_wvalid,
    M07_AXI_araddr,
    M07_AXI_arready,
    M07_AXI_arvalid,
    M07_AXI_awaddr,
    M07_AXI_awready,
    M07_AXI_awvalid,
    M07_AXI_bready,
    M07_AXI_bresp,
    M07_AXI_bvalid,
    M07_AXI_rdata,
    M07_AXI_rready,
    M07_AXI_rresp,
    M07_AXI_rvalid,
    M07_AXI_wdata,
    M07_AXI_wready,
    M07_AXI_wvalid,
    S00_ACLK,
    S00_AXI_araddr,
    S00_AXI_arburst,
    S00_AXI_arcache,
    S00_AXI_arid,
    S00_AXI_arlen,
    S00_AXI_arlock,
    S00_AXI_arprot,
    S00_AXI_arqos,
    S00_AXI_arready,
    S00_AXI_arsize,
    S00_AXI_arvalid,
    S00_AXI_awaddr,
    S00_AXI_awburst,
    S00_AXI_awcache,
    S00_AXI_awid,
    S00_AXI_awlen,
    S00_AXI_awlock,
    S00_AXI_awprot,
    S00_AXI_awqos,
    S00_AXI_awready,
    S00_AXI_awsize,
    S00_AXI_awvalid,
    S00_AXI_bid,
    S00_AXI_bready,
    S00_AXI_bresp,
    S00_AXI_bvalid,
    S00_AXI_rdata,
    S00_AXI_rid,
    S00_AXI_rlast,
    S00_AXI_rready,
    S00_AXI_rresp,
    S00_AXI_rvalid,
    S00_AXI_wdata,
    S00_AXI_wid,
    S00_AXI_wlast,
    S00_AXI_wready,
    S00_AXI_wstrb,
    S00_AXI_wvalid,
    ext_reset_in,
    interconnect_aresetn,
    peripheral_aresetn);
  output [31:0]M00_AXI_araddr;
  input [0:0]M00_AXI_arready;
  output [0:0]M00_AXI_arvalid;
  output [31:0]M00_AXI_awaddr;
  input [0:0]M00_AXI_awready;
  output [0:0]M00_AXI_awvalid;
  output [0:0]M00_AXI_bready;
  input [1:0]M00_AXI_bresp;
  input [0:0]M00_AXI_bvalid;
  input [31:0]M00_AXI_rdata;
  output [0:0]M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input [0:0]M00_AXI_rvalid;
  output [31:0]M00_AXI_wdata;
  input [0:0]M00_AXI_wready;
  output [3:0]M00_AXI_wstrb;
  output [0:0]M00_AXI_wvalid;
  output [31:0]M01_AXI_araddr;
  output [2:0]M01_AXI_arprot;
  input M01_AXI_arready;
  output M01_AXI_arvalid;
  output [31:0]M01_AXI_awaddr;
  output [2:0]M01_AXI_awprot;
  input M01_AXI_awready;
  output M01_AXI_awvalid;
  output M01_AXI_bready;
  input [1:0]M01_AXI_bresp;
  input M01_AXI_bvalid;
  input [31:0]M01_AXI_rdata;
  output M01_AXI_rready;
  input [1:0]M01_AXI_rresp;
  input M01_AXI_rvalid;
  output [31:0]M01_AXI_wdata;
  input M01_AXI_wready;
  output [3:0]M01_AXI_wstrb;
  output M01_AXI_wvalid;
  output [31:0]M03_AXI_araddr;
  output [2:0]M03_AXI_arprot;
  input [0:0]M03_AXI_arready;
  output [0:0]M03_AXI_arvalid;
  output [31:0]M03_AXI_awaddr;
  output [2:0]M03_AXI_awprot;
  input [0:0]M03_AXI_awready;
  output [0:0]M03_AXI_awvalid;
  output [0:0]M03_AXI_bready;
  input [1:0]M03_AXI_bresp;
  input [0:0]M03_AXI_bvalid;
  input [31:0]M03_AXI_rdata;
  output [0:0]M03_AXI_rready;
  input [1:0]M03_AXI_rresp;
  input [0:0]M03_AXI_rvalid;
  output [31:0]M03_AXI_wdata;
  input [0:0]M03_AXI_wready;
  output [3:0]M03_AXI_wstrb;
  output [0:0]M03_AXI_wvalid;
  output [31:0]M04_AXI_araddr;
  input [0:0]M04_AXI_arready;
  output [0:0]M04_AXI_arvalid;
  output [31:0]M04_AXI_awaddr;
  input [0:0]M04_AXI_awready;
  output [0:0]M04_AXI_awvalid;
  output [0:0]M04_AXI_bready;
  input [1:0]M04_AXI_bresp;
  input [0:0]M04_AXI_bvalid;
  input [31:0]M04_AXI_rdata;
  output [0:0]M04_AXI_rready;
  input [1:0]M04_AXI_rresp;
  input [0:0]M04_AXI_rvalid;
  output [31:0]M04_AXI_wdata;
  input [0:0]M04_AXI_wready;
  output [0:0]M04_AXI_wvalid;
  output [31:0]M05_AXI_araddr;
  input [0:0]M05_AXI_arready;
  output [0:0]M05_AXI_arvalid;
  output [31:0]M05_AXI_awaddr;
  input [0:0]M05_AXI_awready;
  output [0:0]M05_AXI_awvalid;
  output [0:0]M05_AXI_bready;
  input [1:0]M05_AXI_bresp;
  input [0:0]M05_AXI_bvalid;
  input [31:0]M05_AXI_rdata;
  output [0:0]M05_AXI_rready;
  input [1:0]M05_AXI_rresp;
  input [0:0]M05_AXI_rvalid;
  output [31:0]M05_AXI_wdata;
  input [0:0]M05_AXI_wready;
  output [3:0]M05_AXI_wstrb;
  output [0:0]M05_AXI_wvalid;
  output [31:0]M06_AXI_araddr;
  input [0:0]M06_AXI_arready;
  output [0:0]M06_AXI_arvalid;
  output [31:0]M06_AXI_awaddr;
  input [0:0]M06_AXI_awready;
  output [0:0]M06_AXI_awvalid;
  output [0:0]M06_AXI_bready;
  input [1:0]M06_AXI_bresp;
  input [0:0]M06_AXI_bvalid;
  input [31:0]M06_AXI_rdata;
  output [0:0]M06_AXI_rready;
  input [1:0]M06_AXI_rresp;
  input [0:0]M06_AXI_rvalid;
  output [31:0]M06_AXI_wdata;
  input [0:0]M06_AXI_wready;
  output [0:0]M06_AXI_wvalid;
  output [31:0]M07_AXI_araddr;
  input [0:0]M07_AXI_arready;
  output [0:0]M07_AXI_arvalid;
  output [31:0]M07_AXI_awaddr;
  input [0:0]M07_AXI_awready;
  output [0:0]M07_AXI_awvalid;
  output [0:0]M07_AXI_bready;
  input [1:0]M07_AXI_bresp;
  input [0:0]M07_AXI_bvalid;
  input [31:0]M07_AXI_rdata;
  output [0:0]M07_AXI_rready;
  input [1:0]M07_AXI_rresp;
  input [0:0]M07_AXI_rvalid;
  output [31:0]M07_AXI_wdata;
  input [0:0]M07_AXI_wready;
  output [0:0]M07_AXI_wvalid;
  input S00_ACLK;
  input [31:0]S00_AXI_araddr;
  input [1:0]S00_AXI_arburst;
  input [3:0]S00_AXI_arcache;
  input [11:0]S00_AXI_arid;
  input [3:0]S00_AXI_arlen;
  input [1:0]S00_AXI_arlock;
  input [2:0]S00_AXI_arprot;
  input [3:0]S00_AXI_arqos;
  output S00_AXI_arready;
  input [2:0]S00_AXI_arsize;
  input S00_AXI_arvalid;
  input [31:0]S00_AXI_awaddr;
  input [1:0]S00_AXI_awburst;
  input [3:0]S00_AXI_awcache;
  input [11:0]S00_AXI_awid;
  input [3:0]S00_AXI_awlen;
  input [1:0]S00_AXI_awlock;
  input [2:0]S00_AXI_awprot;
  input [3:0]S00_AXI_awqos;
  output S00_AXI_awready;
  input [2:0]S00_AXI_awsize;
  input S00_AXI_awvalid;
  output [11:0]S00_AXI_bid;
  input S00_AXI_bready;
  output [1:0]S00_AXI_bresp;
  output S00_AXI_bvalid;
  output [31:0]S00_AXI_rdata;
  output [11:0]S00_AXI_rid;
  output S00_AXI_rlast;
  input S00_AXI_rready;
  output [1:0]S00_AXI_rresp;
  output S00_AXI_rvalid;
  input [31:0]S00_AXI_wdata;
  input [11:0]S00_AXI_wid;
  input S00_AXI_wlast;
  output S00_AXI_wready;
  input [3:0]S00_AXI_wstrb;
  input S00_AXI_wvalid;
  input ext_reset_in;
  output [0:0]interconnect_aresetn;
  output [0:0]peripheral_aresetn;

  wire [0:0]ARESETN_1;
  wire [31:0]Conn10_ARADDR;
  wire [0:0]Conn10_ARREADY;
  wire [0:0]Conn10_ARVALID;
  wire [31:0]Conn10_AWADDR;
  wire [0:0]Conn10_AWREADY;
  wire [0:0]Conn10_AWVALID;
  wire [0:0]Conn10_BREADY;
  wire [1:0]Conn10_BRESP;
  wire [0:0]Conn10_BVALID;
  wire [31:0]Conn10_RDATA;
  wire [0:0]Conn10_RREADY;
  wire [1:0]Conn10_RRESP;
  wire [0:0]Conn10_RVALID;
  wire [31:0]Conn10_WDATA;
  wire [0:0]Conn10_WREADY;
  wire [3:0]Conn10_WSTRB;
  wire [0:0]Conn10_WVALID;
  wire [31:0]Conn3_ARADDR;
  wire [1:0]Conn3_ARBURST;
  wire [3:0]Conn3_ARCACHE;
  wire [11:0]Conn3_ARID;
  wire [3:0]Conn3_ARLEN;
  wire [1:0]Conn3_ARLOCK;
  wire [2:0]Conn3_ARPROT;
  wire [3:0]Conn3_ARQOS;
  wire Conn3_ARREADY;
  wire [2:0]Conn3_ARSIZE;
  wire Conn3_ARVALID;
  wire [31:0]Conn3_AWADDR;
  wire [1:0]Conn3_AWBURST;
  wire [3:0]Conn3_AWCACHE;
  wire [11:0]Conn3_AWID;
  wire [3:0]Conn3_AWLEN;
  wire [1:0]Conn3_AWLOCK;
  wire [2:0]Conn3_AWPROT;
  wire [3:0]Conn3_AWQOS;
  wire Conn3_AWREADY;
  wire [2:0]Conn3_AWSIZE;
  wire Conn3_AWVALID;
  wire [11:0]Conn3_BID;
  wire Conn3_BREADY;
  wire [1:0]Conn3_BRESP;
  wire Conn3_BVALID;
  wire [31:0]Conn3_RDATA;
  wire [11:0]Conn3_RID;
  wire Conn3_RLAST;
  wire Conn3_RREADY;
  wire [1:0]Conn3_RRESP;
  wire Conn3_RVALID;
  wire [31:0]Conn3_WDATA;
  wire [11:0]Conn3_WID;
  wire Conn3_WLAST;
  wire Conn3_WREADY;
  wire [3:0]Conn3_WSTRB;
  wire Conn3_WVALID;
  wire [31:0]Conn4_ARADDR;
  wire [2:0]Conn4_ARPROT;
  wire Conn4_ARREADY;
  wire Conn4_ARVALID;
  wire [31:0]Conn4_AWADDR;
  wire [2:0]Conn4_AWPROT;
  wire Conn4_AWREADY;
  wire Conn4_AWVALID;
  wire Conn4_BREADY;
  wire [1:0]Conn4_BRESP;
  wire Conn4_BVALID;
  wire [31:0]Conn4_RDATA;
  wire Conn4_RREADY;
  wire [1:0]Conn4_RRESP;
  wire Conn4_RVALID;
  wire [31:0]Conn4_WDATA;
  wire Conn4_WREADY;
  wire [3:0]Conn4_WSTRB;
  wire Conn4_WVALID;
  wire S00_ACLK_1;
  wire [31:0]axi_interconnect_0_M02_AXI_ARADDR;
  wire [2:0]axi_interconnect_0_M02_AXI_ARPROT;
  wire [0:0]axi_interconnect_0_M02_AXI_ARREADY;
  wire [0:0]axi_interconnect_0_M02_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M02_AXI_AWADDR;
  wire [2:0]axi_interconnect_0_M02_AXI_AWPROT;
  wire [0:0]axi_interconnect_0_M02_AXI_AWREADY;
  wire [0:0]axi_interconnect_0_M02_AXI_AWVALID;
  wire [0:0]axi_interconnect_0_M02_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M02_AXI_BRESP;
  wire [0:0]axi_interconnect_0_M02_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M02_AXI_RDATA;
  wire [0:0]axi_interconnect_0_M02_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M02_AXI_RRESP;
  wire [0:0]axi_interconnect_0_M02_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M02_AXI_WDATA;
  wire [0:0]axi_interconnect_0_M02_AXI_WREADY;
  wire [3:0]axi_interconnect_0_M02_AXI_WSTRB;
  wire [0:0]axi_interconnect_0_M02_AXI_WVALID;
  wire [31:0]axi_interconnect_0_M03_AXI_ARADDR;
  wire [0:0]axi_interconnect_0_M03_AXI_ARREADY;
  wire [0:0]axi_interconnect_0_M03_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M03_AXI_AWADDR;
  wire [0:0]axi_interconnect_0_M03_AXI_AWREADY;
  wire [0:0]axi_interconnect_0_M03_AXI_AWVALID;
  wire [0:0]axi_interconnect_0_M03_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M03_AXI_BRESP;
  wire [0:0]axi_interconnect_0_M03_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M03_AXI_RDATA;
  wire [0:0]axi_interconnect_0_M03_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M03_AXI_RRESP;
  wire [0:0]axi_interconnect_0_M03_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M03_AXI_WDATA;
  wire [0:0]axi_interconnect_0_M03_AXI_WREADY;
  wire [0:0]axi_interconnect_0_M03_AXI_WVALID;
  wire [31:0]axi_interconnect_0_M04_AXI_ARADDR;
  wire [0:0]axi_interconnect_0_M04_AXI_ARREADY;
  wire [0:0]axi_interconnect_0_M04_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M04_AXI_AWADDR;
  wire [0:0]axi_interconnect_0_M04_AXI_AWREADY;
  wire [0:0]axi_interconnect_0_M04_AXI_AWVALID;
  wire [0:0]axi_interconnect_0_M04_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M04_AXI_BRESP;
  wire [0:0]axi_interconnect_0_M04_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M04_AXI_RDATA;
  wire [0:0]axi_interconnect_0_M04_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M04_AXI_RRESP;
  wire [0:0]axi_interconnect_0_M04_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M04_AXI_WDATA;
  wire [0:0]axi_interconnect_0_M04_AXI_WREADY;
  wire [3:0]axi_interconnect_0_M04_AXI_WSTRB;
  wire [0:0]axi_interconnect_0_M04_AXI_WVALID;
  wire [31:0]axi_interconnect_0_M05_AXI_ARADDR;
  wire [0:0]axi_interconnect_0_M05_AXI_ARREADY;
  wire [0:0]axi_interconnect_0_M05_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M05_AXI_AWADDR;
  wire [0:0]axi_interconnect_0_M05_AXI_AWREADY;
  wire [0:0]axi_interconnect_0_M05_AXI_AWVALID;
  wire [0:0]axi_interconnect_0_M05_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M05_AXI_BRESP;
  wire [0:0]axi_interconnect_0_M05_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M05_AXI_RDATA;
  wire [0:0]axi_interconnect_0_M05_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M05_AXI_RRESP;
  wire [0:0]axi_interconnect_0_M05_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M05_AXI_WDATA;
  wire [0:0]axi_interconnect_0_M05_AXI_WREADY;
  wire [0:0]axi_interconnect_0_M05_AXI_WVALID;
  wire [31:0]axi_interconnect_0_M06_AXI_ARADDR;
  wire [0:0]axi_interconnect_0_M06_AXI_ARREADY;
  wire [0:0]axi_interconnect_0_M06_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M06_AXI_AWADDR;
  wire [0:0]axi_interconnect_0_M06_AXI_AWREADY;
  wire [0:0]axi_interconnect_0_M06_AXI_AWVALID;
  wire [0:0]axi_interconnect_0_M06_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M06_AXI_BRESP;
  wire [0:0]axi_interconnect_0_M06_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M06_AXI_RDATA;
  wire [0:0]axi_interconnect_0_M06_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M06_AXI_RRESP;
  wire [0:0]axi_interconnect_0_M06_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M06_AXI_WDATA;
  wire [0:0]axi_interconnect_0_M06_AXI_WREADY;
  wire [0:0]axi_interconnect_0_M06_AXI_WVALID;
  wire [31:0]axi_interconnect_0_M07_AXI_ARADDR;
  wire axi_interconnect_0_M07_AXI_ARREADY;
  wire axi_interconnect_0_M07_AXI_ARVALID;
  wire [31:0]axi_interconnect_0_M07_AXI_AWADDR;
  wire axi_interconnect_0_M07_AXI_AWREADY;
  wire axi_interconnect_0_M07_AXI_AWVALID;
  wire axi_interconnect_0_M07_AXI_BREADY;
  wire [1:0]axi_interconnect_0_M07_AXI_BRESP;
  wire axi_interconnect_0_M07_AXI_BVALID;
  wire [31:0]axi_interconnect_0_M07_AXI_RDATA;
  wire axi_interconnect_0_M07_AXI_RREADY;
  wire [1:0]axi_interconnect_0_M07_AXI_RRESP;
  wire axi_interconnect_0_M07_AXI_RVALID;
  wire [31:0]axi_interconnect_0_M07_AXI_WDATA;
  wire axi_interconnect_0_M07_AXI_WREADY;
  wire [3:0]axi_interconnect_0_M07_AXI_WSTRB;
  wire axi_interconnect_0_M07_AXI_WVALID;
  wire ext_reset_in_1;
  wire [0:0]rst_ps7_0_200M_peripheral_aresetn;

  assign Conn10_ARREADY = M00_AXI_arready[0];
  assign Conn10_AWREADY = M00_AXI_awready[0];
  assign Conn10_BRESP = M00_AXI_bresp[1:0];
  assign Conn10_BVALID = M00_AXI_bvalid[0];
  assign Conn10_RDATA = M00_AXI_rdata[31:0];
  assign Conn10_RRESP = M00_AXI_rresp[1:0];
  assign Conn10_RVALID = M00_AXI_rvalid[0];
  assign Conn10_WREADY = M00_AXI_wready[0];
  assign Conn3_ARADDR = S00_AXI_araddr[31:0];
  assign Conn3_ARBURST = S00_AXI_arburst[1:0];
  assign Conn3_ARCACHE = S00_AXI_arcache[3:0];
  assign Conn3_ARID = S00_AXI_arid[11:0];
  assign Conn3_ARLEN = S00_AXI_arlen[3:0];
  assign Conn3_ARLOCK = S00_AXI_arlock[1:0];
  assign Conn3_ARPROT = S00_AXI_arprot[2:0];
  assign Conn3_ARQOS = S00_AXI_arqos[3:0];
  assign Conn3_ARSIZE = S00_AXI_arsize[2:0];
  assign Conn3_ARVALID = S00_AXI_arvalid;
  assign Conn3_AWADDR = S00_AXI_awaddr[31:0];
  assign Conn3_AWBURST = S00_AXI_awburst[1:0];
  assign Conn3_AWCACHE = S00_AXI_awcache[3:0];
  assign Conn3_AWID = S00_AXI_awid[11:0];
  assign Conn3_AWLEN = S00_AXI_awlen[3:0];
  assign Conn3_AWLOCK = S00_AXI_awlock[1:0];
  assign Conn3_AWPROT = S00_AXI_awprot[2:0];
  assign Conn3_AWQOS = S00_AXI_awqos[3:0];
  assign Conn3_AWSIZE = S00_AXI_awsize[2:0];
  assign Conn3_AWVALID = S00_AXI_awvalid;
  assign Conn3_BREADY = S00_AXI_bready;
  assign Conn3_RREADY = S00_AXI_rready;
  assign Conn3_WDATA = S00_AXI_wdata[31:0];
  assign Conn3_WID = S00_AXI_wid[11:0];
  assign Conn3_WLAST = S00_AXI_wlast;
  assign Conn3_WSTRB = S00_AXI_wstrb[3:0];
  assign Conn3_WVALID = S00_AXI_wvalid;
  assign Conn4_ARREADY = M01_AXI_arready;
  assign Conn4_AWREADY = M01_AXI_awready;
  assign Conn4_BRESP = M01_AXI_bresp[1:0];
  assign Conn4_BVALID = M01_AXI_bvalid;
  assign Conn4_RDATA = M01_AXI_rdata[31:0];
  assign Conn4_RRESP = M01_AXI_rresp[1:0];
  assign Conn4_RVALID = M01_AXI_rvalid;
  assign Conn4_WREADY = M01_AXI_wready;
  assign M00_AXI_araddr[31:0] = Conn10_ARADDR;
  assign M00_AXI_arvalid[0] = Conn10_ARVALID;
  assign M00_AXI_awaddr[31:0] = Conn10_AWADDR;
  assign M00_AXI_awvalid[0] = Conn10_AWVALID;
  assign M00_AXI_bready[0] = Conn10_BREADY;
  assign M00_AXI_rready[0] = Conn10_RREADY;
  assign M00_AXI_wdata[31:0] = Conn10_WDATA;
  assign M00_AXI_wstrb[3:0] = Conn10_WSTRB;
  assign M00_AXI_wvalid[0] = Conn10_WVALID;
  assign M01_AXI_araddr[31:0] = Conn4_ARADDR;
  assign M01_AXI_arprot[2:0] = Conn4_ARPROT;
  assign M01_AXI_arvalid = Conn4_ARVALID;
  assign M01_AXI_awaddr[31:0] = Conn4_AWADDR;
  assign M01_AXI_awprot[2:0] = Conn4_AWPROT;
  assign M01_AXI_awvalid = Conn4_AWVALID;
  assign M01_AXI_bready = Conn4_BREADY;
  assign M01_AXI_rready = Conn4_RREADY;
  assign M01_AXI_wdata[31:0] = Conn4_WDATA;
  assign M01_AXI_wstrb[3:0] = Conn4_WSTRB;
  assign M01_AXI_wvalid = Conn4_WVALID;
  assign M03_AXI_araddr[31:0] = axi_interconnect_0_M02_AXI_ARADDR;
  assign M03_AXI_arprot[2:0] = axi_interconnect_0_M02_AXI_ARPROT;
  assign M03_AXI_arvalid[0] = axi_interconnect_0_M02_AXI_ARVALID;
  assign M03_AXI_awaddr[31:0] = axi_interconnect_0_M02_AXI_AWADDR;
  assign M03_AXI_awprot[2:0] = axi_interconnect_0_M02_AXI_AWPROT;
  assign M03_AXI_awvalid[0] = axi_interconnect_0_M02_AXI_AWVALID;
  assign M03_AXI_bready[0] = axi_interconnect_0_M02_AXI_BREADY;
  assign M03_AXI_rready[0] = axi_interconnect_0_M02_AXI_RREADY;
  assign M03_AXI_wdata[31:0] = axi_interconnect_0_M02_AXI_WDATA;
  assign M03_AXI_wstrb[3:0] = axi_interconnect_0_M02_AXI_WSTRB;
  assign M03_AXI_wvalid[0] = axi_interconnect_0_M02_AXI_WVALID;
  assign M04_AXI_araddr[31:0] = axi_interconnect_0_M03_AXI_ARADDR;
  assign M04_AXI_arvalid[0] = axi_interconnect_0_M03_AXI_ARVALID;
  assign M04_AXI_awaddr[31:0] = axi_interconnect_0_M03_AXI_AWADDR;
  assign M04_AXI_awvalid[0] = axi_interconnect_0_M03_AXI_AWVALID;
  assign M04_AXI_bready[0] = axi_interconnect_0_M03_AXI_BREADY;
  assign M04_AXI_rready[0] = axi_interconnect_0_M03_AXI_RREADY;
  assign M04_AXI_wdata[31:0] = axi_interconnect_0_M03_AXI_WDATA;
  assign M04_AXI_wvalid[0] = axi_interconnect_0_M03_AXI_WVALID;
  assign M05_AXI_araddr[31:0] = axi_interconnect_0_M04_AXI_ARADDR;
  assign M05_AXI_arvalid[0] = axi_interconnect_0_M04_AXI_ARVALID;
  assign M05_AXI_awaddr[31:0] = axi_interconnect_0_M04_AXI_AWADDR;
  assign M05_AXI_awvalid[0] = axi_interconnect_0_M04_AXI_AWVALID;
  assign M05_AXI_bready[0] = axi_interconnect_0_M04_AXI_BREADY;
  assign M05_AXI_rready[0] = axi_interconnect_0_M04_AXI_RREADY;
  assign M05_AXI_wdata[31:0] = axi_interconnect_0_M04_AXI_WDATA;
  assign M05_AXI_wstrb[3:0] = axi_interconnect_0_M04_AXI_WSTRB;
  assign M05_AXI_wvalid[0] = axi_interconnect_0_M04_AXI_WVALID;
  assign M06_AXI_araddr[31:0] = axi_interconnect_0_M05_AXI_ARADDR;
  assign M06_AXI_arvalid[0] = axi_interconnect_0_M05_AXI_ARVALID;
  assign M06_AXI_awaddr[31:0] = axi_interconnect_0_M05_AXI_AWADDR;
  assign M06_AXI_awvalid[0] = axi_interconnect_0_M05_AXI_AWVALID;
  assign M06_AXI_bready[0] = axi_interconnect_0_M05_AXI_BREADY;
  assign M06_AXI_rready[0] = axi_interconnect_0_M05_AXI_RREADY;
  assign M06_AXI_wdata[31:0] = axi_interconnect_0_M05_AXI_WDATA;
  assign M06_AXI_wvalid[0] = axi_interconnect_0_M05_AXI_WVALID;
  assign M07_AXI_araddr[31:0] = axi_interconnect_0_M06_AXI_ARADDR;
  assign M07_AXI_arvalid[0] = axi_interconnect_0_M06_AXI_ARVALID;
  assign M07_AXI_awaddr[31:0] = axi_interconnect_0_M06_AXI_AWADDR;
  assign M07_AXI_awvalid[0] = axi_interconnect_0_M06_AXI_AWVALID;
  assign M07_AXI_bready[0] = axi_interconnect_0_M06_AXI_BREADY;
  assign M07_AXI_rready[0] = axi_interconnect_0_M06_AXI_RREADY;
  assign M07_AXI_wdata[31:0] = axi_interconnect_0_M06_AXI_WDATA;
  assign M07_AXI_wvalid[0] = axi_interconnect_0_M06_AXI_WVALID;
  assign S00_ACLK_1 = S00_ACLK;
  assign S00_AXI_arready = Conn3_ARREADY;
  assign S00_AXI_awready = Conn3_AWREADY;
  assign S00_AXI_bid[11:0] = Conn3_BID;
  assign S00_AXI_bresp[1:0] = Conn3_BRESP;
  assign S00_AXI_bvalid = Conn3_BVALID;
  assign S00_AXI_rdata[31:0] = Conn3_RDATA;
  assign S00_AXI_rid[11:0] = Conn3_RID;
  assign S00_AXI_rlast = Conn3_RLAST;
  assign S00_AXI_rresp[1:0] = Conn3_RRESP;
  assign S00_AXI_rvalid = Conn3_RVALID;
  assign S00_AXI_wready = Conn3_WREADY;
  assign axi_interconnect_0_M02_AXI_ARREADY = M03_AXI_arready[0];
  assign axi_interconnect_0_M02_AXI_AWREADY = M03_AXI_awready[0];
  assign axi_interconnect_0_M02_AXI_BRESP = M03_AXI_bresp[1:0];
  assign axi_interconnect_0_M02_AXI_BVALID = M03_AXI_bvalid[0];
  assign axi_interconnect_0_M02_AXI_RDATA = M03_AXI_rdata[31:0];
  assign axi_interconnect_0_M02_AXI_RRESP = M03_AXI_rresp[1:0];
  assign axi_interconnect_0_M02_AXI_RVALID = M03_AXI_rvalid[0];
  assign axi_interconnect_0_M02_AXI_WREADY = M03_AXI_wready[0];
  assign axi_interconnect_0_M03_AXI_ARREADY = M04_AXI_arready[0];
  assign axi_interconnect_0_M03_AXI_AWREADY = M04_AXI_awready[0];
  assign axi_interconnect_0_M03_AXI_BRESP = M04_AXI_bresp[1:0];
  assign axi_interconnect_0_M03_AXI_BVALID = M04_AXI_bvalid[0];
  assign axi_interconnect_0_M03_AXI_RDATA = M04_AXI_rdata[31:0];
  assign axi_interconnect_0_M03_AXI_RRESP = M04_AXI_rresp[1:0];
  assign axi_interconnect_0_M03_AXI_RVALID = M04_AXI_rvalid[0];
  assign axi_interconnect_0_M03_AXI_WREADY = M04_AXI_wready[0];
  assign axi_interconnect_0_M04_AXI_ARREADY = M05_AXI_arready[0];
  assign axi_interconnect_0_M04_AXI_AWREADY = M05_AXI_awready[0];
  assign axi_interconnect_0_M04_AXI_BRESP = M05_AXI_bresp[1:0];
  assign axi_interconnect_0_M04_AXI_BVALID = M05_AXI_bvalid[0];
  assign axi_interconnect_0_M04_AXI_RDATA = M05_AXI_rdata[31:0];
  assign axi_interconnect_0_M04_AXI_RRESP = M05_AXI_rresp[1:0];
  assign axi_interconnect_0_M04_AXI_RVALID = M05_AXI_rvalid[0];
  assign axi_interconnect_0_M04_AXI_WREADY = M05_AXI_wready[0];
  assign axi_interconnect_0_M05_AXI_ARREADY = M06_AXI_arready[0];
  assign axi_interconnect_0_M05_AXI_AWREADY = M06_AXI_awready[0];
  assign axi_interconnect_0_M05_AXI_BRESP = M06_AXI_bresp[1:0];
  assign axi_interconnect_0_M05_AXI_BVALID = M06_AXI_bvalid[0];
  assign axi_interconnect_0_M05_AXI_RDATA = M06_AXI_rdata[31:0];
  assign axi_interconnect_0_M05_AXI_RRESP = M06_AXI_rresp[1:0];
  assign axi_interconnect_0_M05_AXI_RVALID = M06_AXI_rvalid[0];
  assign axi_interconnect_0_M05_AXI_WREADY = M06_AXI_wready[0];
  assign axi_interconnect_0_M06_AXI_ARREADY = M07_AXI_arready[0];
  assign axi_interconnect_0_M06_AXI_AWREADY = M07_AXI_awready[0];
  assign axi_interconnect_0_M06_AXI_BRESP = M07_AXI_bresp[1:0];
  assign axi_interconnect_0_M06_AXI_BVALID = M07_AXI_bvalid[0];
  assign axi_interconnect_0_M06_AXI_RDATA = M07_AXI_rdata[31:0];
  assign axi_interconnect_0_M06_AXI_RRESP = M07_AXI_rresp[1:0];
  assign axi_interconnect_0_M06_AXI_RVALID = M07_AXI_rvalid[0];
  assign axi_interconnect_0_M06_AXI_WREADY = M07_AXI_wready[0];
  assign ext_reset_in_1 = ext_reset_in;
  assign interconnect_aresetn[0] = ARESETN_1;
  assign peripheral_aresetn[0] = rst_ps7_0_200M_peripheral_aresetn;
  design_1_axi_interconnect_0_0 axi_interconnect_0
       (.ACLK(S00_ACLK_1),
        .ARESETN(ARESETN_1),
        .M00_ACLK(S00_ACLK_1),
        .M00_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M00_AXI_araddr(Conn10_ARADDR),
        .M00_AXI_arready(Conn10_ARREADY),
        .M00_AXI_arvalid(Conn10_ARVALID),
        .M00_AXI_awaddr(Conn10_AWADDR),
        .M00_AXI_awready(Conn10_AWREADY),
        .M00_AXI_awvalid(Conn10_AWVALID),
        .M00_AXI_bready(Conn10_BREADY),
        .M00_AXI_bresp(Conn10_BRESP),
        .M00_AXI_bvalid(Conn10_BVALID),
        .M00_AXI_rdata(Conn10_RDATA),
        .M00_AXI_rready(Conn10_RREADY),
        .M00_AXI_rresp(Conn10_RRESP),
        .M00_AXI_rvalid(Conn10_RVALID),
        .M00_AXI_wdata(Conn10_WDATA),
        .M00_AXI_wready(Conn10_WREADY),
        .M00_AXI_wstrb(Conn10_WSTRB),
        .M00_AXI_wvalid(Conn10_WVALID),
        .M01_ACLK(S00_ACLK_1),
        .M01_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M01_AXI_araddr(Conn4_ARADDR),
        .M01_AXI_arprot(Conn4_ARPROT),
        .M01_AXI_arready(Conn4_ARREADY),
        .M01_AXI_arvalid(Conn4_ARVALID),
        .M01_AXI_awaddr(Conn4_AWADDR),
        .M01_AXI_awprot(Conn4_AWPROT),
        .M01_AXI_awready(Conn4_AWREADY),
        .M01_AXI_awvalid(Conn4_AWVALID),
        .M01_AXI_bready(Conn4_BREADY),
        .M01_AXI_bresp(Conn4_BRESP),
        .M01_AXI_bvalid(Conn4_BVALID),
        .M01_AXI_rdata(Conn4_RDATA),
        .M01_AXI_rready(Conn4_RREADY),
        .M01_AXI_rresp(Conn4_RRESP),
        .M01_AXI_rvalid(Conn4_RVALID),
        .M01_AXI_wdata(Conn4_WDATA),
        .M01_AXI_wready(Conn4_WREADY),
        .M01_AXI_wstrb(Conn4_WSTRB),
        .M01_AXI_wvalid(Conn4_WVALID),
        .M02_ACLK(S00_ACLK_1),
        .M02_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M02_AXI_araddr(axi_interconnect_0_M02_AXI_ARADDR),
        .M02_AXI_arprot(axi_interconnect_0_M02_AXI_ARPROT),
        .M02_AXI_arready(axi_interconnect_0_M02_AXI_ARREADY),
        .M02_AXI_arvalid(axi_interconnect_0_M02_AXI_ARVALID),
        .M02_AXI_awaddr(axi_interconnect_0_M02_AXI_AWADDR),
        .M02_AXI_awprot(axi_interconnect_0_M02_AXI_AWPROT),
        .M02_AXI_awready(axi_interconnect_0_M02_AXI_AWREADY),
        .M02_AXI_awvalid(axi_interconnect_0_M02_AXI_AWVALID),
        .M02_AXI_bready(axi_interconnect_0_M02_AXI_BREADY),
        .M02_AXI_bresp(axi_interconnect_0_M02_AXI_BRESP),
        .M02_AXI_bvalid(axi_interconnect_0_M02_AXI_BVALID),
        .M02_AXI_rdata(axi_interconnect_0_M02_AXI_RDATA),
        .M02_AXI_rready(axi_interconnect_0_M02_AXI_RREADY),
        .M02_AXI_rresp(axi_interconnect_0_M02_AXI_RRESP),
        .M02_AXI_rvalid(axi_interconnect_0_M02_AXI_RVALID),
        .M02_AXI_wdata(axi_interconnect_0_M02_AXI_WDATA),
        .M02_AXI_wready(axi_interconnect_0_M02_AXI_WREADY),
        .M02_AXI_wstrb(axi_interconnect_0_M02_AXI_WSTRB),
        .M02_AXI_wvalid(axi_interconnect_0_M02_AXI_WVALID),
        .M03_ACLK(S00_ACLK_1),
        .M03_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M03_AXI_araddr(axi_interconnect_0_M03_AXI_ARADDR),
        .M03_AXI_arready(axi_interconnect_0_M03_AXI_ARREADY),
        .M03_AXI_arvalid(axi_interconnect_0_M03_AXI_ARVALID),
        .M03_AXI_awaddr(axi_interconnect_0_M03_AXI_AWADDR),
        .M03_AXI_awready(axi_interconnect_0_M03_AXI_AWREADY),
        .M03_AXI_awvalid(axi_interconnect_0_M03_AXI_AWVALID),
        .M03_AXI_bready(axi_interconnect_0_M03_AXI_BREADY),
        .M03_AXI_bresp(axi_interconnect_0_M03_AXI_BRESP),
        .M03_AXI_bvalid(axi_interconnect_0_M03_AXI_BVALID),
        .M03_AXI_rdata(axi_interconnect_0_M03_AXI_RDATA),
        .M03_AXI_rready(axi_interconnect_0_M03_AXI_RREADY),
        .M03_AXI_rresp(axi_interconnect_0_M03_AXI_RRESP),
        .M03_AXI_rvalid(axi_interconnect_0_M03_AXI_RVALID),
        .M03_AXI_wdata(axi_interconnect_0_M03_AXI_WDATA),
        .M03_AXI_wready(axi_interconnect_0_M03_AXI_WREADY),
        .M03_AXI_wvalid(axi_interconnect_0_M03_AXI_WVALID),
        .M04_ACLK(S00_ACLK_1),
        .M04_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M04_AXI_araddr(axi_interconnect_0_M04_AXI_ARADDR),
        .M04_AXI_arready(axi_interconnect_0_M04_AXI_ARREADY),
        .M04_AXI_arvalid(axi_interconnect_0_M04_AXI_ARVALID),
        .M04_AXI_awaddr(axi_interconnect_0_M04_AXI_AWADDR),
        .M04_AXI_awready(axi_interconnect_0_M04_AXI_AWREADY),
        .M04_AXI_awvalid(axi_interconnect_0_M04_AXI_AWVALID),
        .M04_AXI_bready(axi_interconnect_0_M04_AXI_BREADY),
        .M04_AXI_bresp(axi_interconnect_0_M04_AXI_BRESP),
        .M04_AXI_bvalid(axi_interconnect_0_M04_AXI_BVALID),
        .M04_AXI_rdata(axi_interconnect_0_M04_AXI_RDATA),
        .M04_AXI_rready(axi_interconnect_0_M04_AXI_RREADY),
        .M04_AXI_rresp(axi_interconnect_0_M04_AXI_RRESP),
        .M04_AXI_rvalid(axi_interconnect_0_M04_AXI_RVALID),
        .M04_AXI_wdata(axi_interconnect_0_M04_AXI_WDATA),
        .M04_AXI_wready(axi_interconnect_0_M04_AXI_WREADY),
        .M04_AXI_wstrb(axi_interconnect_0_M04_AXI_WSTRB),
        .M04_AXI_wvalid(axi_interconnect_0_M04_AXI_WVALID),
        .M05_ACLK(S00_ACLK_1),
        .M05_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M05_AXI_araddr(axi_interconnect_0_M05_AXI_ARADDR),
        .M05_AXI_arready(axi_interconnect_0_M05_AXI_ARREADY),
        .M05_AXI_arvalid(axi_interconnect_0_M05_AXI_ARVALID),
        .M05_AXI_awaddr(axi_interconnect_0_M05_AXI_AWADDR),
        .M05_AXI_awready(axi_interconnect_0_M05_AXI_AWREADY),
        .M05_AXI_awvalid(axi_interconnect_0_M05_AXI_AWVALID),
        .M05_AXI_bready(axi_interconnect_0_M05_AXI_BREADY),
        .M05_AXI_bresp(axi_interconnect_0_M05_AXI_BRESP),
        .M05_AXI_bvalid(axi_interconnect_0_M05_AXI_BVALID),
        .M05_AXI_rdata(axi_interconnect_0_M05_AXI_RDATA),
        .M05_AXI_rready(axi_interconnect_0_M05_AXI_RREADY),
        .M05_AXI_rresp(axi_interconnect_0_M05_AXI_RRESP),
        .M05_AXI_rvalid(axi_interconnect_0_M05_AXI_RVALID),
        .M05_AXI_wdata(axi_interconnect_0_M05_AXI_WDATA),
        .M05_AXI_wready(axi_interconnect_0_M05_AXI_WREADY),
        .M05_AXI_wvalid(axi_interconnect_0_M05_AXI_WVALID),
        .M06_ACLK(S00_ACLK_1),
        .M06_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M06_AXI_araddr(axi_interconnect_0_M06_AXI_ARADDR),
        .M06_AXI_arready(axi_interconnect_0_M06_AXI_ARREADY),
        .M06_AXI_arvalid(axi_interconnect_0_M06_AXI_ARVALID),
        .M06_AXI_awaddr(axi_interconnect_0_M06_AXI_AWADDR),
        .M06_AXI_awready(axi_interconnect_0_M06_AXI_AWREADY),
        .M06_AXI_awvalid(axi_interconnect_0_M06_AXI_AWVALID),
        .M06_AXI_bready(axi_interconnect_0_M06_AXI_BREADY),
        .M06_AXI_bresp(axi_interconnect_0_M06_AXI_BRESP),
        .M06_AXI_bvalid(axi_interconnect_0_M06_AXI_BVALID),
        .M06_AXI_rdata(axi_interconnect_0_M06_AXI_RDATA),
        .M06_AXI_rready(axi_interconnect_0_M06_AXI_RREADY),
        .M06_AXI_rresp(axi_interconnect_0_M06_AXI_RRESP),
        .M06_AXI_rvalid(axi_interconnect_0_M06_AXI_RVALID),
        .M06_AXI_wdata(axi_interconnect_0_M06_AXI_WDATA),
        .M06_AXI_wready(axi_interconnect_0_M06_AXI_WREADY),
        .M06_AXI_wvalid(axi_interconnect_0_M06_AXI_WVALID),
        .M07_ACLK(S00_ACLK_1),
        .M07_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .M07_AXI_araddr(axi_interconnect_0_M07_AXI_ARADDR),
        .M07_AXI_arready(axi_interconnect_0_M07_AXI_ARREADY),
        .M07_AXI_arvalid(axi_interconnect_0_M07_AXI_ARVALID),
        .M07_AXI_awaddr(axi_interconnect_0_M07_AXI_AWADDR),
        .M07_AXI_awready(axi_interconnect_0_M07_AXI_AWREADY),
        .M07_AXI_awvalid(axi_interconnect_0_M07_AXI_AWVALID),
        .M07_AXI_bready(axi_interconnect_0_M07_AXI_BREADY),
        .M07_AXI_bresp(axi_interconnect_0_M07_AXI_BRESP),
        .M07_AXI_bvalid(axi_interconnect_0_M07_AXI_BVALID),
        .M07_AXI_rdata(axi_interconnect_0_M07_AXI_RDATA),
        .M07_AXI_rready(axi_interconnect_0_M07_AXI_RREADY),
        .M07_AXI_rresp(axi_interconnect_0_M07_AXI_RRESP),
        .M07_AXI_rvalid(axi_interconnect_0_M07_AXI_RVALID),
        .M07_AXI_wdata(axi_interconnect_0_M07_AXI_WDATA),
        .M07_AXI_wready(axi_interconnect_0_M07_AXI_WREADY),
        .M07_AXI_wstrb(axi_interconnect_0_M07_AXI_WSTRB),
        .M07_AXI_wvalid(axi_interconnect_0_M07_AXI_WVALID),
        .S00_ACLK(S00_ACLK_1),
        .S00_ARESETN(rst_ps7_0_200M_peripheral_aresetn),
        .S00_AXI_araddr(Conn3_ARADDR),
        .S00_AXI_arburst(Conn3_ARBURST),
        .S00_AXI_arcache(Conn3_ARCACHE),
        .S00_AXI_arid(Conn3_ARID),
        .S00_AXI_arlen(Conn3_ARLEN),
        .S00_AXI_arlock(Conn3_ARLOCK),
        .S00_AXI_arprot(Conn3_ARPROT),
        .S00_AXI_arqos(Conn3_ARQOS),
        .S00_AXI_arready(Conn3_ARREADY),
        .S00_AXI_arsize(Conn3_ARSIZE),
        .S00_AXI_arvalid(Conn3_ARVALID),
        .S00_AXI_awaddr(Conn3_AWADDR),
        .S00_AXI_awburst(Conn3_AWBURST),
        .S00_AXI_awcache(Conn3_AWCACHE),
        .S00_AXI_awid(Conn3_AWID),
        .S00_AXI_awlen(Conn3_AWLEN),
        .S00_AXI_awlock(Conn3_AWLOCK),
        .S00_AXI_awprot(Conn3_AWPROT),
        .S00_AXI_awqos(Conn3_AWQOS),
        .S00_AXI_awready(Conn3_AWREADY),
        .S00_AXI_awsize(Conn3_AWSIZE),
        .S00_AXI_awvalid(Conn3_AWVALID),
        .S00_AXI_bid(Conn3_BID),
        .S00_AXI_bready(Conn3_BREADY),
        .S00_AXI_bresp(Conn3_BRESP),
        .S00_AXI_bvalid(Conn3_BVALID),
        .S00_AXI_rdata(Conn3_RDATA),
        .S00_AXI_rid(Conn3_RID),
        .S00_AXI_rlast(Conn3_RLAST),
        .S00_AXI_rready(Conn3_RREADY),
        .S00_AXI_rresp(Conn3_RRESP),
        .S00_AXI_rvalid(Conn3_RVALID),
        .S00_AXI_wdata(Conn3_WDATA),
        .S00_AXI_wid(Conn3_WID),
        .S00_AXI_wlast(Conn3_WLAST),
        .S00_AXI_wready(Conn3_WREADY),
        .S00_AXI_wstrb(Conn3_WSTRB),
        .S00_AXI_wvalid(Conn3_WVALID));
  design_1_rst_ps7_0_200M_0 rst_ps7_0_200M
       (.aux_reset_in(1'b1),
        .dcm_locked(1'b1),
        .ext_reset_in(ext_reset_in_1),
        .interconnect_aresetn(ARESETN_1),
        .mb_debug_sys_rst(1'b0),
        .peripheral_aresetn(rst_ps7_0_200M_peripheral_aresetn),
        .slowest_sync_clk(S00_ACLK_1));
  design_1_xadc_wiz_0_0 xadc_wiz_0
       (.s_axi_aclk(S00_ACLK_1),
        .s_axi_araddr(axi_interconnect_0_M07_AXI_ARADDR[10:0]),
        .s_axi_aresetn(rst_ps7_0_200M_peripheral_aresetn),
        .s_axi_arready(axi_interconnect_0_M07_AXI_ARREADY),
        .s_axi_arvalid(axi_interconnect_0_M07_AXI_ARVALID),
        .s_axi_awaddr(axi_interconnect_0_M07_AXI_AWADDR[10:0]),
        .s_axi_awready(axi_interconnect_0_M07_AXI_AWREADY),
        .s_axi_awvalid(axi_interconnect_0_M07_AXI_AWVALID),
        .s_axi_bready(axi_interconnect_0_M07_AXI_BREADY),
        .s_axi_bresp(axi_interconnect_0_M07_AXI_BRESP),
        .s_axi_bvalid(axi_interconnect_0_M07_AXI_BVALID),
        .s_axi_rdata(axi_interconnect_0_M07_AXI_RDATA),
        .s_axi_rready(axi_interconnect_0_M07_AXI_RREADY),
        .s_axi_rresp(axi_interconnect_0_M07_AXI_RRESP),
        .s_axi_rvalid(axi_interconnect_0_M07_AXI_RVALID),
        .s_axi_wdata(axi_interconnect_0_M07_AXI_WDATA),
        .s_axi_wready(axi_interconnect_0_M07_AXI_WREADY),
        .s_axi_wstrb(axi_interconnect_0_M07_AXI_WSTRB),
        .s_axi_wvalid(axi_interconnect_0_M07_AXI_WVALID),
        .vn_in(1'b0),
        .vp_in(1'b0));
endmodule

module m00_couplers_imp_1YPJ1TK
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m00_couplers_to_m00_couplers_ARADDR;
  wire [0:0]m00_couplers_to_m00_couplers_ARREADY;
  wire [0:0]m00_couplers_to_m00_couplers_ARVALID;
  wire [31:0]m00_couplers_to_m00_couplers_AWADDR;
  wire [0:0]m00_couplers_to_m00_couplers_AWREADY;
  wire [0:0]m00_couplers_to_m00_couplers_AWVALID;
  wire [0:0]m00_couplers_to_m00_couplers_BREADY;
  wire [1:0]m00_couplers_to_m00_couplers_BRESP;
  wire [0:0]m00_couplers_to_m00_couplers_BVALID;
  wire [31:0]m00_couplers_to_m00_couplers_RDATA;
  wire [0:0]m00_couplers_to_m00_couplers_RREADY;
  wire [1:0]m00_couplers_to_m00_couplers_RRESP;
  wire [0:0]m00_couplers_to_m00_couplers_RVALID;
  wire [31:0]m00_couplers_to_m00_couplers_WDATA;
  wire [0:0]m00_couplers_to_m00_couplers_WREADY;
  wire [3:0]m00_couplers_to_m00_couplers_WSTRB;
  wire [0:0]m00_couplers_to_m00_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m00_couplers_to_m00_couplers_ARADDR;
  assign M_AXI_arvalid[0] = m00_couplers_to_m00_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m00_couplers_to_m00_couplers_AWADDR;
  assign M_AXI_awvalid[0] = m00_couplers_to_m00_couplers_AWVALID;
  assign M_AXI_bready[0] = m00_couplers_to_m00_couplers_BREADY;
  assign M_AXI_rready[0] = m00_couplers_to_m00_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m00_couplers_to_m00_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = m00_couplers_to_m00_couplers_WSTRB;
  assign M_AXI_wvalid[0] = m00_couplers_to_m00_couplers_WVALID;
  assign S_AXI_arready[0] = m00_couplers_to_m00_couplers_ARREADY;
  assign S_AXI_awready[0] = m00_couplers_to_m00_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m00_couplers_to_m00_couplers_BRESP;
  assign S_AXI_bvalid[0] = m00_couplers_to_m00_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m00_couplers_to_m00_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m00_couplers_to_m00_couplers_RRESP;
  assign S_AXI_rvalid[0] = m00_couplers_to_m00_couplers_RVALID;
  assign S_AXI_wready[0] = m00_couplers_to_m00_couplers_WREADY;
  assign m00_couplers_to_m00_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m00_couplers_to_m00_couplers_ARREADY = M_AXI_arready[0];
  assign m00_couplers_to_m00_couplers_ARVALID = S_AXI_arvalid[0];
  assign m00_couplers_to_m00_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m00_couplers_to_m00_couplers_AWREADY = M_AXI_awready[0];
  assign m00_couplers_to_m00_couplers_AWVALID = S_AXI_awvalid[0];
  assign m00_couplers_to_m00_couplers_BREADY = S_AXI_bready[0];
  assign m00_couplers_to_m00_couplers_BRESP = M_AXI_bresp[1:0];
  assign m00_couplers_to_m00_couplers_BVALID = M_AXI_bvalid[0];
  assign m00_couplers_to_m00_couplers_RDATA = M_AXI_rdata[31:0];
  assign m00_couplers_to_m00_couplers_RREADY = S_AXI_rready[0];
  assign m00_couplers_to_m00_couplers_RRESP = M_AXI_rresp[1:0];
  assign m00_couplers_to_m00_couplers_RVALID = M_AXI_rvalid[0];
  assign m00_couplers_to_m00_couplers_WDATA = S_AXI_wdata[31:0];
  assign m00_couplers_to_m00_couplers_WREADY = M_AXI_wready[0];
  assign m00_couplers_to_m00_couplers_WSTRB = S_AXI_wstrb[3:0];
  assign m00_couplers_to_m00_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m01_couplers_imp_4V0QOP
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arprot,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awprot,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awprot,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  output [2:0]M_AXI_arprot;
  input M_AXI_arready;
  output M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  output [2:0]M_AXI_awprot;
  input M_AXI_awready;
  output M_AXI_awvalid;
  output M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  input [2:0]S_AXI_arprot;
  output S_AXI_arready;
  input S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [2:0]S_AXI_awprot;
  output S_AXI_awready;
  input S_AXI_awvalid;
  input S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input S_AXI_wvalid;

  wire [31:0]m01_couplers_to_m01_couplers_ARADDR;
  wire [2:0]m01_couplers_to_m01_couplers_ARPROT;
  wire m01_couplers_to_m01_couplers_ARREADY;
  wire m01_couplers_to_m01_couplers_ARVALID;
  wire [31:0]m01_couplers_to_m01_couplers_AWADDR;
  wire [2:0]m01_couplers_to_m01_couplers_AWPROT;
  wire m01_couplers_to_m01_couplers_AWREADY;
  wire m01_couplers_to_m01_couplers_AWVALID;
  wire m01_couplers_to_m01_couplers_BREADY;
  wire [1:0]m01_couplers_to_m01_couplers_BRESP;
  wire m01_couplers_to_m01_couplers_BVALID;
  wire [31:0]m01_couplers_to_m01_couplers_RDATA;
  wire m01_couplers_to_m01_couplers_RREADY;
  wire [1:0]m01_couplers_to_m01_couplers_RRESP;
  wire m01_couplers_to_m01_couplers_RVALID;
  wire [31:0]m01_couplers_to_m01_couplers_WDATA;
  wire m01_couplers_to_m01_couplers_WREADY;
  wire [3:0]m01_couplers_to_m01_couplers_WSTRB;
  wire m01_couplers_to_m01_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m01_couplers_to_m01_couplers_ARADDR;
  assign M_AXI_arprot[2:0] = m01_couplers_to_m01_couplers_ARPROT;
  assign M_AXI_arvalid = m01_couplers_to_m01_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m01_couplers_to_m01_couplers_AWADDR;
  assign M_AXI_awprot[2:0] = m01_couplers_to_m01_couplers_AWPROT;
  assign M_AXI_awvalid = m01_couplers_to_m01_couplers_AWVALID;
  assign M_AXI_bready = m01_couplers_to_m01_couplers_BREADY;
  assign M_AXI_rready = m01_couplers_to_m01_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m01_couplers_to_m01_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = m01_couplers_to_m01_couplers_WSTRB;
  assign M_AXI_wvalid = m01_couplers_to_m01_couplers_WVALID;
  assign S_AXI_arready = m01_couplers_to_m01_couplers_ARREADY;
  assign S_AXI_awready = m01_couplers_to_m01_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m01_couplers_to_m01_couplers_BRESP;
  assign S_AXI_bvalid = m01_couplers_to_m01_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m01_couplers_to_m01_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m01_couplers_to_m01_couplers_RRESP;
  assign S_AXI_rvalid = m01_couplers_to_m01_couplers_RVALID;
  assign S_AXI_wready = m01_couplers_to_m01_couplers_WREADY;
  assign m01_couplers_to_m01_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m01_couplers_to_m01_couplers_ARPROT = S_AXI_arprot[2:0];
  assign m01_couplers_to_m01_couplers_ARREADY = M_AXI_arready;
  assign m01_couplers_to_m01_couplers_ARVALID = S_AXI_arvalid;
  assign m01_couplers_to_m01_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m01_couplers_to_m01_couplers_AWPROT = S_AXI_awprot[2:0];
  assign m01_couplers_to_m01_couplers_AWREADY = M_AXI_awready;
  assign m01_couplers_to_m01_couplers_AWVALID = S_AXI_awvalid;
  assign m01_couplers_to_m01_couplers_BREADY = S_AXI_bready;
  assign m01_couplers_to_m01_couplers_BRESP = M_AXI_bresp[1:0];
  assign m01_couplers_to_m01_couplers_BVALID = M_AXI_bvalid;
  assign m01_couplers_to_m01_couplers_RDATA = M_AXI_rdata[31:0];
  assign m01_couplers_to_m01_couplers_RREADY = S_AXI_rready;
  assign m01_couplers_to_m01_couplers_RRESP = M_AXI_rresp[1:0];
  assign m01_couplers_to_m01_couplers_RVALID = M_AXI_rvalid;
  assign m01_couplers_to_m01_couplers_WDATA = S_AXI_wdata[31:0];
  assign m01_couplers_to_m01_couplers_WREADY = M_AXI_wready;
  assign m01_couplers_to_m01_couplers_WSTRB = S_AXI_wstrb[3:0];
  assign m01_couplers_to_m01_couplers_WVALID = S_AXI_wvalid;
endmodule

module m02_couplers_imp_1XNAPOB
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arprot,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awprot,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awprot,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  output [2:0]M_AXI_arprot;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  output [2:0]M_AXI_awprot;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  input [2:0]S_AXI_arprot;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [2:0]S_AXI_awprot;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m02_couplers_to_m02_couplers_ARADDR;
  wire [2:0]m02_couplers_to_m02_couplers_ARPROT;
  wire [0:0]m02_couplers_to_m02_couplers_ARREADY;
  wire [0:0]m02_couplers_to_m02_couplers_ARVALID;
  wire [31:0]m02_couplers_to_m02_couplers_AWADDR;
  wire [2:0]m02_couplers_to_m02_couplers_AWPROT;
  wire [0:0]m02_couplers_to_m02_couplers_AWREADY;
  wire [0:0]m02_couplers_to_m02_couplers_AWVALID;
  wire [0:0]m02_couplers_to_m02_couplers_BREADY;
  wire [1:0]m02_couplers_to_m02_couplers_BRESP;
  wire [0:0]m02_couplers_to_m02_couplers_BVALID;
  wire [31:0]m02_couplers_to_m02_couplers_RDATA;
  wire [0:0]m02_couplers_to_m02_couplers_RREADY;
  wire [1:0]m02_couplers_to_m02_couplers_RRESP;
  wire [0:0]m02_couplers_to_m02_couplers_RVALID;
  wire [31:0]m02_couplers_to_m02_couplers_WDATA;
  wire [0:0]m02_couplers_to_m02_couplers_WREADY;
  wire [3:0]m02_couplers_to_m02_couplers_WSTRB;
  wire [0:0]m02_couplers_to_m02_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m02_couplers_to_m02_couplers_ARADDR;
  assign M_AXI_arprot[2:0] = m02_couplers_to_m02_couplers_ARPROT;
  assign M_AXI_arvalid[0] = m02_couplers_to_m02_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m02_couplers_to_m02_couplers_AWADDR;
  assign M_AXI_awprot[2:0] = m02_couplers_to_m02_couplers_AWPROT;
  assign M_AXI_awvalid[0] = m02_couplers_to_m02_couplers_AWVALID;
  assign M_AXI_bready[0] = m02_couplers_to_m02_couplers_BREADY;
  assign M_AXI_rready[0] = m02_couplers_to_m02_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m02_couplers_to_m02_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = m02_couplers_to_m02_couplers_WSTRB;
  assign M_AXI_wvalid[0] = m02_couplers_to_m02_couplers_WVALID;
  assign S_AXI_arready[0] = m02_couplers_to_m02_couplers_ARREADY;
  assign S_AXI_awready[0] = m02_couplers_to_m02_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m02_couplers_to_m02_couplers_BRESP;
  assign S_AXI_bvalid[0] = m02_couplers_to_m02_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m02_couplers_to_m02_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m02_couplers_to_m02_couplers_RRESP;
  assign S_AXI_rvalid[0] = m02_couplers_to_m02_couplers_RVALID;
  assign S_AXI_wready[0] = m02_couplers_to_m02_couplers_WREADY;
  assign m02_couplers_to_m02_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m02_couplers_to_m02_couplers_ARPROT = S_AXI_arprot[2:0];
  assign m02_couplers_to_m02_couplers_ARREADY = M_AXI_arready[0];
  assign m02_couplers_to_m02_couplers_ARVALID = S_AXI_arvalid[0];
  assign m02_couplers_to_m02_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m02_couplers_to_m02_couplers_AWPROT = S_AXI_awprot[2:0];
  assign m02_couplers_to_m02_couplers_AWREADY = M_AXI_awready[0];
  assign m02_couplers_to_m02_couplers_AWVALID = S_AXI_awvalid[0];
  assign m02_couplers_to_m02_couplers_BREADY = S_AXI_bready[0];
  assign m02_couplers_to_m02_couplers_BRESP = M_AXI_bresp[1:0];
  assign m02_couplers_to_m02_couplers_BVALID = M_AXI_bvalid[0];
  assign m02_couplers_to_m02_couplers_RDATA = M_AXI_rdata[31:0];
  assign m02_couplers_to_m02_couplers_RREADY = S_AXI_rready[0];
  assign m02_couplers_to_m02_couplers_RRESP = M_AXI_rresp[1:0];
  assign m02_couplers_to_m02_couplers_RVALID = M_AXI_rvalid[0];
  assign m02_couplers_to_m02_couplers_WDATA = S_AXI_wdata[31:0];
  assign m02_couplers_to_m02_couplers_WREADY = M_AXI_wready[0];
  assign m02_couplers_to_m02_couplers_WSTRB = S_AXI_wstrb[3:0];
  assign m02_couplers_to_m02_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m03_couplers_imp_5MZ1QY
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m03_couplers_to_m03_couplers_ARADDR;
  wire [0:0]m03_couplers_to_m03_couplers_ARREADY;
  wire [0:0]m03_couplers_to_m03_couplers_ARVALID;
  wire [31:0]m03_couplers_to_m03_couplers_AWADDR;
  wire [0:0]m03_couplers_to_m03_couplers_AWREADY;
  wire [0:0]m03_couplers_to_m03_couplers_AWVALID;
  wire [0:0]m03_couplers_to_m03_couplers_BREADY;
  wire [1:0]m03_couplers_to_m03_couplers_BRESP;
  wire [0:0]m03_couplers_to_m03_couplers_BVALID;
  wire [31:0]m03_couplers_to_m03_couplers_RDATA;
  wire [0:0]m03_couplers_to_m03_couplers_RREADY;
  wire [1:0]m03_couplers_to_m03_couplers_RRESP;
  wire [0:0]m03_couplers_to_m03_couplers_RVALID;
  wire [31:0]m03_couplers_to_m03_couplers_WDATA;
  wire [0:0]m03_couplers_to_m03_couplers_WREADY;
  wire [0:0]m03_couplers_to_m03_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m03_couplers_to_m03_couplers_ARADDR;
  assign M_AXI_arvalid[0] = m03_couplers_to_m03_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m03_couplers_to_m03_couplers_AWADDR;
  assign M_AXI_awvalid[0] = m03_couplers_to_m03_couplers_AWVALID;
  assign M_AXI_bready[0] = m03_couplers_to_m03_couplers_BREADY;
  assign M_AXI_rready[0] = m03_couplers_to_m03_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m03_couplers_to_m03_couplers_WDATA;
  assign M_AXI_wvalid[0] = m03_couplers_to_m03_couplers_WVALID;
  assign S_AXI_arready[0] = m03_couplers_to_m03_couplers_ARREADY;
  assign S_AXI_awready[0] = m03_couplers_to_m03_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m03_couplers_to_m03_couplers_BRESP;
  assign S_AXI_bvalid[0] = m03_couplers_to_m03_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m03_couplers_to_m03_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m03_couplers_to_m03_couplers_RRESP;
  assign S_AXI_rvalid[0] = m03_couplers_to_m03_couplers_RVALID;
  assign S_AXI_wready[0] = m03_couplers_to_m03_couplers_WREADY;
  assign m03_couplers_to_m03_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m03_couplers_to_m03_couplers_ARREADY = M_AXI_arready[0];
  assign m03_couplers_to_m03_couplers_ARVALID = S_AXI_arvalid[0];
  assign m03_couplers_to_m03_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m03_couplers_to_m03_couplers_AWREADY = M_AXI_awready[0];
  assign m03_couplers_to_m03_couplers_AWVALID = S_AXI_awvalid[0];
  assign m03_couplers_to_m03_couplers_BREADY = S_AXI_bready[0];
  assign m03_couplers_to_m03_couplers_BRESP = M_AXI_bresp[1:0];
  assign m03_couplers_to_m03_couplers_BVALID = M_AXI_bvalid[0];
  assign m03_couplers_to_m03_couplers_RDATA = M_AXI_rdata[31:0];
  assign m03_couplers_to_m03_couplers_RREADY = S_AXI_rready[0];
  assign m03_couplers_to_m03_couplers_RRESP = M_AXI_rresp[1:0];
  assign m03_couplers_to_m03_couplers_RVALID = M_AXI_rvalid[0];
  assign m03_couplers_to_m03_couplers_WDATA = S_AXI_wdata[31:0];
  assign m03_couplers_to_m03_couplers_WREADY = M_AXI_wready[0];
  assign m03_couplers_to_m03_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m04_couplers_imp_1W489VI
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m04_couplers_to_m04_couplers_ARADDR;
  wire [0:0]m04_couplers_to_m04_couplers_ARREADY;
  wire [0:0]m04_couplers_to_m04_couplers_ARVALID;
  wire [31:0]m04_couplers_to_m04_couplers_AWADDR;
  wire [0:0]m04_couplers_to_m04_couplers_AWREADY;
  wire [0:0]m04_couplers_to_m04_couplers_AWVALID;
  wire [0:0]m04_couplers_to_m04_couplers_BREADY;
  wire [1:0]m04_couplers_to_m04_couplers_BRESP;
  wire [0:0]m04_couplers_to_m04_couplers_BVALID;
  wire [31:0]m04_couplers_to_m04_couplers_RDATA;
  wire [0:0]m04_couplers_to_m04_couplers_RREADY;
  wire [1:0]m04_couplers_to_m04_couplers_RRESP;
  wire [0:0]m04_couplers_to_m04_couplers_RVALID;
  wire [31:0]m04_couplers_to_m04_couplers_WDATA;
  wire [0:0]m04_couplers_to_m04_couplers_WREADY;
  wire [3:0]m04_couplers_to_m04_couplers_WSTRB;
  wire [0:0]m04_couplers_to_m04_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m04_couplers_to_m04_couplers_ARADDR;
  assign M_AXI_arvalid[0] = m04_couplers_to_m04_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m04_couplers_to_m04_couplers_AWADDR;
  assign M_AXI_awvalid[0] = m04_couplers_to_m04_couplers_AWVALID;
  assign M_AXI_bready[0] = m04_couplers_to_m04_couplers_BREADY;
  assign M_AXI_rready[0] = m04_couplers_to_m04_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m04_couplers_to_m04_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = m04_couplers_to_m04_couplers_WSTRB;
  assign M_AXI_wvalid[0] = m04_couplers_to_m04_couplers_WVALID;
  assign S_AXI_arready[0] = m04_couplers_to_m04_couplers_ARREADY;
  assign S_AXI_awready[0] = m04_couplers_to_m04_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m04_couplers_to_m04_couplers_BRESP;
  assign S_AXI_bvalid[0] = m04_couplers_to_m04_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m04_couplers_to_m04_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m04_couplers_to_m04_couplers_RRESP;
  assign S_AXI_rvalid[0] = m04_couplers_to_m04_couplers_RVALID;
  assign S_AXI_wready[0] = m04_couplers_to_m04_couplers_WREADY;
  assign m04_couplers_to_m04_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m04_couplers_to_m04_couplers_ARREADY = M_AXI_arready[0];
  assign m04_couplers_to_m04_couplers_ARVALID = S_AXI_arvalid[0];
  assign m04_couplers_to_m04_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m04_couplers_to_m04_couplers_AWREADY = M_AXI_awready[0];
  assign m04_couplers_to_m04_couplers_AWVALID = S_AXI_awvalid[0];
  assign m04_couplers_to_m04_couplers_BREADY = S_AXI_bready[0];
  assign m04_couplers_to_m04_couplers_BRESP = M_AXI_bresp[1:0];
  assign m04_couplers_to_m04_couplers_BVALID = M_AXI_bvalid[0];
  assign m04_couplers_to_m04_couplers_RDATA = M_AXI_rdata[31:0];
  assign m04_couplers_to_m04_couplers_RREADY = S_AXI_rready[0];
  assign m04_couplers_to_m04_couplers_RRESP = M_AXI_rresp[1:0];
  assign m04_couplers_to_m04_couplers_RVALID = M_AXI_rvalid[0];
  assign m04_couplers_to_m04_couplers_WDATA = S_AXI_wdata[31:0];
  assign m04_couplers_to_m04_couplers_WREADY = M_AXI_wready[0];
  assign m04_couplers_to_m04_couplers_WSTRB = S_AXI_wstrb[3:0];
  assign m04_couplers_to_m04_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m05_couplers_imp_79H0DB
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m05_couplers_to_m05_couplers_ARADDR;
  wire [0:0]m05_couplers_to_m05_couplers_ARREADY;
  wire [0:0]m05_couplers_to_m05_couplers_ARVALID;
  wire [31:0]m05_couplers_to_m05_couplers_AWADDR;
  wire [0:0]m05_couplers_to_m05_couplers_AWREADY;
  wire [0:0]m05_couplers_to_m05_couplers_AWVALID;
  wire [0:0]m05_couplers_to_m05_couplers_BREADY;
  wire [1:0]m05_couplers_to_m05_couplers_BRESP;
  wire [0:0]m05_couplers_to_m05_couplers_BVALID;
  wire [31:0]m05_couplers_to_m05_couplers_RDATA;
  wire [0:0]m05_couplers_to_m05_couplers_RREADY;
  wire [1:0]m05_couplers_to_m05_couplers_RRESP;
  wire [0:0]m05_couplers_to_m05_couplers_RVALID;
  wire [31:0]m05_couplers_to_m05_couplers_WDATA;
  wire [0:0]m05_couplers_to_m05_couplers_WREADY;
  wire [0:0]m05_couplers_to_m05_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m05_couplers_to_m05_couplers_ARADDR;
  assign M_AXI_arvalid[0] = m05_couplers_to_m05_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m05_couplers_to_m05_couplers_AWADDR;
  assign M_AXI_awvalid[0] = m05_couplers_to_m05_couplers_AWVALID;
  assign M_AXI_bready[0] = m05_couplers_to_m05_couplers_BREADY;
  assign M_AXI_rready[0] = m05_couplers_to_m05_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m05_couplers_to_m05_couplers_WDATA;
  assign M_AXI_wvalid[0] = m05_couplers_to_m05_couplers_WVALID;
  assign S_AXI_arready[0] = m05_couplers_to_m05_couplers_ARREADY;
  assign S_AXI_awready[0] = m05_couplers_to_m05_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m05_couplers_to_m05_couplers_BRESP;
  assign S_AXI_bvalid[0] = m05_couplers_to_m05_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m05_couplers_to_m05_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m05_couplers_to_m05_couplers_RRESP;
  assign S_AXI_rvalid[0] = m05_couplers_to_m05_couplers_RVALID;
  assign S_AXI_wready[0] = m05_couplers_to_m05_couplers_WREADY;
  assign m05_couplers_to_m05_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m05_couplers_to_m05_couplers_ARREADY = M_AXI_arready[0];
  assign m05_couplers_to_m05_couplers_ARVALID = S_AXI_arvalid[0];
  assign m05_couplers_to_m05_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m05_couplers_to_m05_couplers_AWREADY = M_AXI_awready[0];
  assign m05_couplers_to_m05_couplers_AWVALID = S_AXI_awvalid[0];
  assign m05_couplers_to_m05_couplers_BREADY = S_AXI_bready[0];
  assign m05_couplers_to_m05_couplers_BRESP = M_AXI_bresp[1:0];
  assign m05_couplers_to_m05_couplers_BVALID = M_AXI_bvalid[0];
  assign m05_couplers_to_m05_couplers_RDATA = M_AXI_rdata[31:0];
  assign m05_couplers_to_m05_couplers_RREADY = S_AXI_rready[0];
  assign m05_couplers_to_m05_couplers_RRESP = M_AXI_rresp[1:0];
  assign m05_couplers_to_m05_couplers_RVALID = M_AXI_rvalid[0];
  assign m05_couplers_to_m05_couplers_WDATA = S_AXI_wdata[31:0];
  assign m05_couplers_to_m05_couplers_WREADY = M_AXI_wready[0];
  assign m05_couplers_to_m05_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m06_couplers_imp_1URZLVH
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input [0:0]M_AXI_arready;
  output [0:0]M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input [0:0]M_AXI_awready;
  output [0:0]M_AXI_awvalid;
  output [0:0]M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input [0:0]M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output [0:0]M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input [0:0]M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input [0:0]M_AXI_wready;
  output [0:0]M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [0:0]S_AXI_wvalid;

  wire [31:0]m06_couplers_to_m06_couplers_ARADDR;
  wire [0:0]m06_couplers_to_m06_couplers_ARREADY;
  wire [0:0]m06_couplers_to_m06_couplers_ARVALID;
  wire [31:0]m06_couplers_to_m06_couplers_AWADDR;
  wire [0:0]m06_couplers_to_m06_couplers_AWREADY;
  wire [0:0]m06_couplers_to_m06_couplers_AWVALID;
  wire [0:0]m06_couplers_to_m06_couplers_BREADY;
  wire [1:0]m06_couplers_to_m06_couplers_BRESP;
  wire [0:0]m06_couplers_to_m06_couplers_BVALID;
  wire [31:0]m06_couplers_to_m06_couplers_RDATA;
  wire [0:0]m06_couplers_to_m06_couplers_RREADY;
  wire [1:0]m06_couplers_to_m06_couplers_RRESP;
  wire [0:0]m06_couplers_to_m06_couplers_RVALID;
  wire [31:0]m06_couplers_to_m06_couplers_WDATA;
  wire [0:0]m06_couplers_to_m06_couplers_WREADY;
  wire [0:0]m06_couplers_to_m06_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m06_couplers_to_m06_couplers_ARADDR;
  assign M_AXI_arvalid[0] = m06_couplers_to_m06_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m06_couplers_to_m06_couplers_AWADDR;
  assign M_AXI_awvalid[0] = m06_couplers_to_m06_couplers_AWVALID;
  assign M_AXI_bready[0] = m06_couplers_to_m06_couplers_BREADY;
  assign M_AXI_rready[0] = m06_couplers_to_m06_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m06_couplers_to_m06_couplers_WDATA;
  assign M_AXI_wvalid[0] = m06_couplers_to_m06_couplers_WVALID;
  assign S_AXI_arready[0] = m06_couplers_to_m06_couplers_ARREADY;
  assign S_AXI_awready[0] = m06_couplers_to_m06_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m06_couplers_to_m06_couplers_BRESP;
  assign S_AXI_bvalid[0] = m06_couplers_to_m06_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m06_couplers_to_m06_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m06_couplers_to_m06_couplers_RRESP;
  assign S_AXI_rvalid[0] = m06_couplers_to_m06_couplers_RVALID;
  assign S_AXI_wready[0] = m06_couplers_to_m06_couplers_WREADY;
  assign m06_couplers_to_m06_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m06_couplers_to_m06_couplers_ARREADY = M_AXI_arready[0];
  assign m06_couplers_to_m06_couplers_ARVALID = S_AXI_arvalid[0];
  assign m06_couplers_to_m06_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m06_couplers_to_m06_couplers_AWREADY = M_AXI_awready[0];
  assign m06_couplers_to_m06_couplers_AWVALID = S_AXI_awvalid[0];
  assign m06_couplers_to_m06_couplers_BREADY = S_AXI_bready[0];
  assign m06_couplers_to_m06_couplers_BRESP = M_AXI_bresp[1:0];
  assign m06_couplers_to_m06_couplers_BVALID = M_AXI_bvalid[0];
  assign m06_couplers_to_m06_couplers_RDATA = M_AXI_rdata[31:0];
  assign m06_couplers_to_m06_couplers_RREADY = S_AXI_rready[0];
  assign m06_couplers_to_m06_couplers_RRESP = M_AXI_rresp[1:0];
  assign m06_couplers_to_m06_couplers_RVALID = M_AXI_rvalid[0];
  assign m06_couplers_to_m06_couplers_WDATA = S_AXI_wdata[31:0];
  assign m06_couplers_to_m06_couplers_WREADY = M_AXI_wready[0];
  assign m06_couplers_to_m06_couplers_WVALID = S_AXI_wvalid[0];
endmodule

module m07_couplers_imp_8VDEOS
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  input M_AXI_arready;
  output M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  input M_AXI_awready;
  output M_AXI_awvalid;
  output M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  output S_AXI_arready;
  input S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  output S_AXI_awready;
  input S_AXI_awvalid;
  input S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input S_AXI_wvalid;

  wire [31:0]m07_couplers_to_m07_couplers_ARADDR;
  wire m07_couplers_to_m07_couplers_ARREADY;
  wire m07_couplers_to_m07_couplers_ARVALID;
  wire [31:0]m07_couplers_to_m07_couplers_AWADDR;
  wire m07_couplers_to_m07_couplers_AWREADY;
  wire m07_couplers_to_m07_couplers_AWVALID;
  wire m07_couplers_to_m07_couplers_BREADY;
  wire [1:0]m07_couplers_to_m07_couplers_BRESP;
  wire m07_couplers_to_m07_couplers_BVALID;
  wire [31:0]m07_couplers_to_m07_couplers_RDATA;
  wire m07_couplers_to_m07_couplers_RREADY;
  wire [1:0]m07_couplers_to_m07_couplers_RRESP;
  wire m07_couplers_to_m07_couplers_RVALID;
  wire [31:0]m07_couplers_to_m07_couplers_WDATA;
  wire m07_couplers_to_m07_couplers_WREADY;
  wire [3:0]m07_couplers_to_m07_couplers_WSTRB;
  wire m07_couplers_to_m07_couplers_WVALID;

  assign M_AXI_araddr[31:0] = m07_couplers_to_m07_couplers_ARADDR;
  assign M_AXI_arvalid = m07_couplers_to_m07_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = m07_couplers_to_m07_couplers_AWADDR;
  assign M_AXI_awvalid = m07_couplers_to_m07_couplers_AWVALID;
  assign M_AXI_bready = m07_couplers_to_m07_couplers_BREADY;
  assign M_AXI_rready = m07_couplers_to_m07_couplers_RREADY;
  assign M_AXI_wdata[31:0] = m07_couplers_to_m07_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = m07_couplers_to_m07_couplers_WSTRB;
  assign M_AXI_wvalid = m07_couplers_to_m07_couplers_WVALID;
  assign S_AXI_arready = m07_couplers_to_m07_couplers_ARREADY;
  assign S_AXI_awready = m07_couplers_to_m07_couplers_AWREADY;
  assign S_AXI_bresp[1:0] = m07_couplers_to_m07_couplers_BRESP;
  assign S_AXI_bvalid = m07_couplers_to_m07_couplers_BVALID;
  assign S_AXI_rdata[31:0] = m07_couplers_to_m07_couplers_RDATA;
  assign S_AXI_rresp[1:0] = m07_couplers_to_m07_couplers_RRESP;
  assign S_AXI_rvalid = m07_couplers_to_m07_couplers_RVALID;
  assign S_AXI_wready = m07_couplers_to_m07_couplers_WREADY;
  assign m07_couplers_to_m07_couplers_ARADDR = S_AXI_araddr[31:0];
  assign m07_couplers_to_m07_couplers_ARREADY = M_AXI_arready;
  assign m07_couplers_to_m07_couplers_ARVALID = S_AXI_arvalid;
  assign m07_couplers_to_m07_couplers_AWADDR = S_AXI_awaddr[31:0];
  assign m07_couplers_to_m07_couplers_AWREADY = M_AXI_awready;
  assign m07_couplers_to_m07_couplers_AWVALID = S_AXI_awvalid;
  assign m07_couplers_to_m07_couplers_BREADY = S_AXI_bready;
  assign m07_couplers_to_m07_couplers_BRESP = M_AXI_bresp[1:0];
  assign m07_couplers_to_m07_couplers_BVALID = M_AXI_bvalid;
  assign m07_couplers_to_m07_couplers_RDATA = M_AXI_rdata[31:0];
  assign m07_couplers_to_m07_couplers_RREADY = S_AXI_rready;
  assign m07_couplers_to_m07_couplers_RRESP = M_AXI_rresp[1:0];
  assign m07_couplers_to_m07_couplers_RVALID = M_AXI_rvalid;
  assign m07_couplers_to_m07_couplers_WDATA = S_AXI_wdata[31:0];
  assign m07_couplers_to_m07_couplers_WREADY = M_AXI_wready;
  assign m07_couplers_to_m07_couplers_WSTRB = S_AXI_wstrb[3:0];
  assign m07_couplers_to_m07_couplers_WVALID = S_AXI_wvalid;
endmodule

module processing_av_system_imp_2DPZH9
   (AXI1_clk,
    AXI_clk,
    BCLK_clk,
    CLK18M_clk,
    CLK90_clk,
    CPUCLK_clk,
    DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    I2SO_D0,
    I2S_FSYNC_OUT,
    I2S_SCLK,
    IIC_0_scl_i,
    IIC_0_scl_o,
    IIC_0_scl_t,
    IIC_0_sda_i,
    IIC_0_sda_o,
    IIC_0_sda_t,
    M_AXI1_araddr,
    M_AXI1_arprot,
    M_AXI1_arready,
    M_AXI1_arvalid,
    M_AXI1_awaddr,
    M_AXI1_awprot,
    M_AXI1_awready,
    M_AXI1_awvalid,
    M_AXI1_bready,
    M_AXI1_bresp,
    M_AXI1_bvalid,
    M_AXI1_rdata,
    M_AXI1_rready,
    M_AXI1_rresp,
    M_AXI1_rvalid,
    M_AXI1_wdata,
    M_AXI1_wready,
    M_AXI1_wstrb,
    M_AXI1_wvalid,
    M_AXI_araddr,
    M_AXI_arprot,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awprot,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    PCLK_clk,
    PCLK_reg,
    S_AXI_araddr,
    S_AXI_arburst,
    S_AXI_arcache,
    S_AXI_arlen,
    S_AXI_arlock,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arsize,
    S_AXI_aruser,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awburst,
    S_AXI_awcache,
    S_AXI_awlen,
    S_AXI_awlock,
    S_AXI_awprot,
    S_AXI_awready,
    S_AXI_awsize,
    S_AXI_awuser,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rlast,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wlast,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid,
    aresetn,
    aresetn1,
    clk90_detected,
    control_vblank,
    cpuclk_detected,
    enable_clk_output,
    hdmi_clk,
    hdmi_data,
    hdmi_de,
    hdmi_hs,
    hdmi_intn,
    hdmi_vs,
    nCLKEN_clk,
    nCLKEN_reg,
    peripheral_reset);
  output AXI1_clk;
  output AXI_clk;
  output BCLK_clk;
  output CLK18M_clk;
  inout CLK90_clk;
  inout CPUCLK_clk;
  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  output I2SO_D0;
  output I2S_FSYNC_OUT;
  output I2S_SCLK;
  input IIC_0_scl_i;
  output IIC_0_scl_o;
  output IIC_0_scl_t;
  input IIC_0_sda_i;
  output IIC_0_sda_o;
  output IIC_0_sda_t;
  output [31:0]M_AXI1_araddr;
  output [2:0]M_AXI1_arprot;
  input M_AXI1_arready;
  output M_AXI1_arvalid;
  output [31:0]M_AXI1_awaddr;
  output [2:0]M_AXI1_awprot;
  input M_AXI1_awready;
  output M_AXI1_awvalid;
  output M_AXI1_bready;
  input [1:0]M_AXI1_bresp;
  input M_AXI1_bvalid;
  input [31:0]M_AXI1_rdata;
  output M_AXI1_rready;
  input [1:0]M_AXI1_rresp;
  input M_AXI1_rvalid;
  output [31:0]M_AXI1_wdata;
  input M_AXI1_wready;
  output [3:0]M_AXI1_wstrb;
  output M_AXI1_wvalid;
  output [31:0]M_AXI_araddr;
  output [2:0]M_AXI_arprot;
  input M_AXI_arready;
  output M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  output [2:0]M_AXI_awprot;
  input M_AXI_awready;
  output M_AXI_awvalid;
  output M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output M_AXI_wvalid;
  output PCLK_clk;
  output PCLK_reg;
  input [31:0]S_AXI_araddr;
  input [1:0]S_AXI_arburst;
  input [3:0]S_AXI_arcache;
  input [3:0]S_AXI_arlen;
  input S_AXI_arlock;
  input [2:0]S_AXI_arprot;
  output S_AXI_arready;
  input [2:0]S_AXI_arsize;
  input [4:0]S_AXI_aruser;
  input S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [1:0]S_AXI_awburst;
  input [3:0]S_AXI_awcache;
  input [3:0]S_AXI_awlen;
  input S_AXI_awlock;
  input [2:0]S_AXI_awprot;
  output S_AXI_awready;
  input [2:0]S_AXI_awsize;
  input [4:0]S_AXI_awuser;
  input S_AXI_awvalid;
  input S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  output S_AXI_rlast;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  input S_AXI_wlast;
  output S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input S_AXI_wvalid;
  output [0:0]aresetn;
  output [0:0]aresetn1;
  output clk90_detected;
  output [1:0]control_vblank;
  output cpuclk_detected;
  input enable_clk_output;
  output hdmi_clk;
  output [15:0]hdmi_data;
  output hdmi_de;
  output hdmi_hs;
  input [0:0]hdmi_intn;
  output hdmi_vs;
  output nCLKEN_clk;
  output nCLKEN_reg;
  output [0:0]peripheral_reset;

  wire Conn1_DDR_VRN;
  wire Conn1_DDR_VRP;
  wire [53:0]Conn1_MIO;
  wire Conn1_PS_CLK;
  wire Conn1_PS_PORB;
  wire Conn1_PS_SRSTB;
  wire Conn2_SCL_I;
  wire Conn2_SCL_O;
  wire Conn2_SCL_T;
  wire Conn2_SDA_I;
  wire Conn2_SDA_O;
  wire Conn2_SDA_T;
  wire [14:0]Conn3_ADDR;
  wire [2:0]Conn3_BA;
  wire Conn3_CAS_N;
  wire Conn3_CKE;
  wire Conn3_CK_N;
  wire Conn3_CK_P;
  wire Conn3_CS_N;
  wire [3:0]Conn3_DM;
  wire [31:0]Conn3_DQ;
  wire [3:0]Conn3_DQS_N;
  wire [3:0]Conn3_DQS_P;
  wire Conn3_ODT;
  wire Conn3_RAS_N;
  wire Conn3_RESET_N;
  wire Conn3_WE_N;
  wire Net;
  wire Net1;
  wire [31:0]S_AXI_1_ARADDR;
  wire [2:0]S_AXI_1_ARPROT;
  wire [0:0]S_AXI_1_ARREADY;
  wire [0:0]S_AXI_1_ARVALID;
  wire [31:0]S_AXI_1_AWADDR;
  wire [2:0]S_AXI_1_AWPROT;
  wire [0:0]S_AXI_1_AWREADY;
  wire [0:0]S_AXI_1_AWVALID;
  wire [0:0]S_AXI_1_BREADY;
  wire [1:0]S_AXI_1_BRESP;
  wire [0:0]S_AXI_1_BVALID;
  wire [31:0]S_AXI_1_RDATA;
  wire [0:0]S_AXI_1_RREADY;
  wire [1:0]S_AXI_1_RRESP;
  wire [0:0]S_AXI_1_RVALID;
  wire [31:0]S_AXI_1_WDATA;
  wire [0:0]S_AXI_1_WREADY;
  wire [3:0]S_AXI_1_WSTRB;
  wire [0:0]S_AXI_1_WVALID;
  wire [31:0]S_AXI_2_ARADDR;
  wire [1:0]S_AXI_2_ARBURST;
  wire [3:0]S_AXI_2_ARCACHE;
  wire [3:0]S_AXI_2_ARLEN;
  wire S_AXI_2_ARLOCK;
  wire [2:0]S_AXI_2_ARPROT;
  wire S_AXI_2_ARREADY;
  wire [2:0]S_AXI_2_ARSIZE;
  wire [4:0]S_AXI_2_ARUSER;
  wire S_AXI_2_ARVALID;
  wire [31:0]S_AXI_2_AWADDR;
  wire [1:0]S_AXI_2_AWBURST;
  wire [3:0]S_AXI_2_AWCACHE;
  wire [3:0]S_AXI_2_AWLEN;
  wire S_AXI_2_AWLOCK;
  wire [2:0]S_AXI_2_AWPROT;
  wire S_AXI_2_AWREADY;
  wire [2:0]S_AXI_2_AWSIZE;
  wire [4:0]S_AXI_2_AWUSER;
  wire S_AXI_2_AWVALID;
  wire S_AXI_2_BREADY;
  wire [1:0]S_AXI_2_BRESP;
  wire S_AXI_2_BVALID;
  wire [31:0]S_AXI_2_RDATA;
  wire S_AXI_2_RLAST;
  wire S_AXI_2_RREADY;
  wire [1:0]S_AXI_2_RRESP;
  wire S_AXI_2_RVALID;
  wire [31:0]S_AXI_2_WDATA;
  wire S_AXI_2_WLAST;
  wire S_AXI_2_WREADY;
  wire [3:0]S_AXI_2_WSTRB;
  wire S_AXI_2_WVALID;
  wire [31:0]S_AXI_LITE_1_ARADDR;
  wire [0:0]S_AXI_LITE_1_ARREADY;
  wire [0:0]S_AXI_LITE_1_ARVALID;
  wire [31:0]S_AXI_LITE_1_AWADDR;
  wire [0:0]S_AXI_LITE_1_AWREADY;
  wire [0:0]S_AXI_LITE_1_AWVALID;
  wire [0:0]S_AXI_LITE_1_BREADY;
  wire [1:0]S_AXI_LITE_1_BRESP;
  wire [0:0]S_AXI_LITE_1_BVALID;
  wire [31:0]S_AXI_LITE_1_RDATA;
  wire [0:0]S_AXI_LITE_1_RREADY;
  wire [1:0]S_AXI_LITE_1_RRESP;
  wire [0:0]S_AXI_LITE_1_RVALID;
  wire [31:0]S_AXI_LITE_1_WDATA;
  wire [0:0]S_AXI_LITE_1_WREADY;
  wire [0:0]S_AXI_LITE_1_WVALID;
  wire aud_mclk_1;
  wire audio_video_engine_I2SO_D0;
  wire audio_video_engine_I2S_FSYNC_OUT;
  wire audio_video_engine_I2S_SCLK;
  wire [31:0]audio_video_engine_M00_AXI_ARADDR;
  wire [1:0]audio_video_engine_M00_AXI_ARBURST;
  wire [3:0]audio_video_engine_M00_AXI_ARCACHE;
  wire [3:0]audio_video_engine_M00_AXI_ARLEN;
  wire [1:0]audio_video_engine_M00_AXI_ARLOCK;
  wire [2:0]audio_video_engine_M00_AXI_ARPROT;
  wire [3:0]audio_video_engine_M00_AXI_ARQOS;
  wire audio_video_engine_M00_AXI_ARREADY;
  wire [2:0]audio_video_engine_M00_AXI_ARSIZE;
  wire audio_video_engine_M00_AXI_ARVALID;
  wire [31:0]audio_video_engine_M00_AXI_RDATA;
  wire audio_video_engine_M00_AXI_RLAST;
  wire audio_video_engine_M00_AXI_RREADY;
  wire [1:0]audio_video_engine_M00_AXI_RRESP;
  wire audio_video_engine_M00_AXI_RVALID;
  wire [31:0]audio_video_engine_M_AXI_ARADDR;
  wire [1:0]audio_video_engine_M_AXI_ARBURST;
  wire [3:0]audio_video_engine_M_AXI_ARCACHE;
  wire [3:0]audio_video_engine_M_AXI_ARLEN;
  wire [1:0]audio_video_engine_M_AXI_ARLOCK;
  wire [2:0]audio_video_engine_M_AXI_ARPROT;
  wire [3:0]audio_video_engine_M_AXI_ARQOS;
  wire audio_video_engine_M_AXI_ARREADY;
  wire [2:0]audio_video_engine_M_AXI_ARSIZE;
  wire audio_video_engine_M_AXI_ARVALID;
  wire [31:0]audio_video_engine_M_AXI_RDATA;
  wire audio_video_engine_M_AXI_RLAST;
  wire audio_video_engine_M_AXI_RREADY;
  wire [1:0]audio_video_engine_M_AXI_RRESP;
  wire audio_video_engine_M_AXI_RVALID;
  wire [1:0]audio_video_engine_control_vblank;
  wire [4:0]audio_video_engine_dout;
  wire audio_video_engine_hdmi_clk;
  wire [15:0]audio_video_engine_hdmi_data;
  wire audio_video_engine_hdmi_de;
  wire audio_video_engine_hdmi_hs;
  wire audio_video_engine_hdmi_vs;
  wire [31:0]axi_dwidth_converter_0_M_AXI_ARADDR;
  wire [1:0]axi_dwidth_converter_0_M_AXI_ARBURST;
  wire [3:0]axi_dwidth_converter_0_M_AXI_ARCACHE;
  wire [3:0]axi_dwidth_converter_0_M_AXI_ARLEN;
  wire [1:0]axi_dwidth_converter_0_M_AXI_ARLOCK;
  wire [2:0]axi_dwidth_converter_0_M_AXI_ARPROT;
  wire [3:0]axi_dwidth_converter_0_M_AXI_ARQOS;
  wire axi_dwidth_converter_0_M_AXI_ARREADY;
  wire [2:0]axi_dwidth_converter_0_M_AXI_ARSIZE;
  wire axi_dwidth_converter_0_M_AXI_ARVALID;
  wire [31:0]axi_dwidth_converter_0_M_AXI_AWADDR;
  wire [1:0]axi_dwidth_converter_0_M_AXI_AWBURST;
  wire [3:0]axi_dwidth_converter_0_M_AXI_AWCACHE;
  wire [3:0]axi_dwidth_converter_0_M_AXI_AWLEN;
  wire [1:0]axi_dwidth_converter_0_M_AXI_AWLOCK;
  wire [2:0]axi_dwidth_converter_0_M_AXI_AWPROT;
  wire [3:0]axi_dwidth_converter_0_M_AXI_AWQOS;
  wire axi_dwidth_converter_0_M_AXI_AWREADY;
  wire [2:0]axi_dwidth_converter_0_M_AXI_AWSIZE;
  wire axi_dwidth_converter_0_M_AXI_AWVALID;
  wire axi_dwidth_converter_0_M_AXI_BREADY;
  wire [1:0]axi_dwidth_converter_0_M_AXI_BRESP;
  wire axi_dwidth_converter_0_M_AXI_BVALID;
  wire [63:0]axi_dwidth_converter_0_M_AXI_RDATA;
  wire axi_dwidth_converter_0_M_AXI_RLAST;
  wire axi_dwidth_converter_0_M_AXI_RREADY;
  wire [1:0]axi_dwidth_converter_0_M_AXI_RRESP;
  wire axi_dwidth_converter_0_M_AXI_RVALID;
  wire [63:0]axi_dwidth_converter_0_M_AXI_WDATA;
  wire axi_dwidth_converter_0_M_AXI_WLAST;
  wire axi_dwidth_converter_0_M_AXI_WREADY;
  wire [7:0]axi_dwidth_converter_0_M_AXI_WSTRB;
  wire axi_dwidth_converter_0_M_AXI_WVALID;
  wire [31:0]axi_protocol_convert_0_M_AXI_ARADDR;
  wire [2:0]axi_protocol_convert_0_M_AXI_ARPROT;
  wire axi_protocol_convert_0_M_AXI_ARREADY;
  wire axi_protocol_convert_0_M_AXI_ARVALID;
  wire [31:0]axi_protocol_convert_0_M_AXI_AWADDR;
  wire [2:0]axi_protocol_convert_0_M_AXI_AWPROT;
  wire axi_protocol_convert_0_M_AXI_AWREADY;
  wire axi_protocol_convert_0_M_AXI_AWVALID;
  wire axi_protocol_convert_0_M_AXI_BREADY;
  wire [1:0]axi_protocol_convert_0_M_AXI_BRESP;
  wire axi_protocol_convert_0_M_AXI_BVALID;
  wire [31:0]axi_protocol_convert_0_M_AXI_RDATA;
  wire axi_protocol_convert_0_M_AXI_RREADY;
  wire [1:0]axi_protocol_convert_0_M_AXI_RRESP;
  wire axi_protocol_convert_0_M_AXI_RVALID;
  wire [31:0]axi_protocol_convert_0_M_AXI_WDATA;
  wire axi_protocol_convert_0_M_AXI_WREADY;
  wire [3:0]axi_protocol_convert_0_M_AXI_WSTRB;
  wire axi_protocol_convert_0_M_AXI_WVALID;
  wire [31:0]axi_register_slice_0_M_AXI_ARADDR;
  wire [1:0]axi_register_slice_0_M_AXI_ARBURST;
  wire [3:0]axi_register_slice_0_M_AXI_ARCACHE;
  wire [3:0]axi_register_slice_0_M_AXI_ARLEN;
  wire [1:0]axi_register_slice_0_M_AXI_ARLOCK;
  wire [2:0]axi_register_slice_0_M_AXI_ARPROT;
  wire [3:0]axi_register_slice_0_M_AXI_ARQOS;
  wire axi_register_slice_0_M_AXI_ARREADY;
  wire [2:0]axi_register_slice_0_M_AXI_ARSIZE;
  wire axi_register_slice_0_M_AXI_ARVALID;
  wire [31:0]axi_register_slice_0_M_AXI_AWADDR;
  wire [1:0]axi_register_slice_0_M_AXI_AWBURST;
  wire [3:0]axi_register_slice_0_M_AXI_AWCACHE;
  wire [3:0]axi_register_slice_0_M_AXI_AWLEN;
  wire [1:0]axi_register_slice_0_M_AXI_AWLOCK;
  wire [2:0]axi_register_slice_0_M_AXI_AWPROT;
  wire [3:0]axi_register_slice_0_M_AXI_AWQOS;
  wire axi_register_slice_0_M_AXI_AWREADY;
  wire [2:0]axi_register_slice_0_M_AXI_AWSIZE;
  wire axi_register_slice_0_M_AXI_AWVALID;
  wire axi_register_slice_0_M_AXI_BREADY;
  wire [1:0]axi_register_slice_0_M_AXI_BRESP;
  wire axi_register_slice_0_M_AXI_BVALID;
  wire [63:0]axi_register_slice_0_M_AXI_RDATA;
  wire axi_register_slice_0_M_AXI_RLAST;
  wire axi_register_slice_0_M_AXI_RREADY;
  wire [1:0]axi_register_slice_0_M_AXI_RRESP;
  wire axi_register_slice_0_M_AXI_RVALID;
  wire [63:0]axi_register_slice_0_M_AXI_WDATA;
  wire axi_register_slice_0_M_AXI_WLAST;
  wire axi_register_slice_0_M_AXI_WREADY;
  wire [7:0]axi_register_slice_0_M_AXI_WSTRB;
  wire axi_register_slice_0_M_AXI_WVALID;
  wire [31:0]axi_register_slice_1_M_AXI_ARADDR;
  wire [1:0]axi_register_slice_1_M_AXI_ARBURST;
  wire [3:0]axi_register_slice_1_M_AXI_ARCACHE;
  wire [3:0]axi_register_slice_1_M_AXI_ARLEN;
  wire [1:0]axi_register_slice_1_M_AXI_ARLOCK;
  wire [2:0]axi_register_slice_1_M_AXI_ARPROT;
  wire [3:0]axi_register_slice_1_M_AXI_ARQOS;
  wire axi_register_slice_1_M_AXI_ARREADY;
  wire [2:0]axi_register_slice_1_M_AXI_ARSIZE;
  wire axi_register_slice_1_M_AXI_ARVALID;
  wire [31:0]axi_register_slice_1_M_AXI_AWADDR;
  wire [1:0]axi_register_slice_1_M_AXI_AWBURST;
  wire [3:0]axi_register_slice_1_M_AXI_AWCACHE;
  wire [3:0]axi_register_slice_1_M_AXI_AWLEN;
  wire [1:0]axi_register_slice_1_M_AXI_AWLOCK;
  wire [2:0]axi_register_slice_1_M_AXI_AWPROT;
  wire [3:0]axi_register_slice_1_M_AXI_AWQOS;
  wire axi_register_slice_1_M_AXI_AWREADY;
  wire [2:0]axi_register_slice_1_M_AXI_AWSIZE;
  wire axi_register_slice_1_M_AXI_AWVALID;
  wire axi_register_slice_1_M_AXI_BREADY;
  wire [1:0]axi_register_slice_1_M_AXI_BRESP;
  wire axi_register_slice_1_M_AXI_BVALID;
  wire [31:0]axi_register_slice_1_M_AXI_RDATA;
  wire axi_register_slice_1_M_AXI_RLAST;
  wire axi_register_slice_1_M_AXI_RREADY;
  wire [1:0]axi_register_slice_1_M_AXI_RRESP;
  wire axi_register_slice_1_M_AXI_RVALID;
  wire [31:0]axi_register_slice_1_M_AXI_WDATA;
  wire axi_register_slice_1_M_AXI_WLAST;
  wire axi_register_slice_1_M_AXI_WREADY;
  wire [3:0]axi_register_slice_1_M_AXI_WSTRB;
  wire axi_register_slice_1_M_AXI_WVALID;
  wire clock_generation_AXI_clk;
  wire clock_generation_BCLK_clk;
  wire clock_generation_PCLK_clk;
  wire clock_generation_PCLK_reg;
  wire clock_generation_clk90_detected;
  wire clock_generation_cpuclk_detected;
  wire clock_generation_nCLKEN_clk;
  wire clock_generation_nCLKEN_reg;
  wire enable_clk_output_1;
  wire [0:0]hdmi_intn_1;
  wire [31:0]interconnect_matrix_M01_AXI_ARADDR;
  wire [2:0]interconnect_matrix_M01_AXI_ARPROT;
  wire interconnect_matrix_M01_AXI_ARREADY;
  wire interconnect_matrix_M01_AXI_ARVALID;
  wire [31:0]interconnect_matrix_M01_AXI_AWADDR;
  wire [2:0]interconnect_matrix_M01_AXI_AWPROT;
  wire interconnect_matrix_M01_AXI_AWREADY;
  wire interconnect_matrix_M01_AXI_AWVALID;
  wire interconnect_matrix_M01_AXI_BREADY;
  wire [1:0]interconnect_matrix_M01_AXI_BRESP;
  wire interconnect_matrix_M01_AXI_BVALID;
  wire [31:0]interconnect_matrix_M01_AXI_RDATA;
  wire interconnect_matrix_M01_AXI_RREADY;
  wire [1:0]interconnect_matrix_M01_AXI_RRESP;
  wire interconnect_matrix_M01_AXI_RVALID;
  wire [31:0]interconnect_matrix_M01_AXI_WDATA;
  wire interconnect_matrix_M01_AXI_WREADY;
  wire [3:0]interconnect_matrix_M01_AXI_WSTRB;
  wire interconnect_matrix_M01_AXI_WVALID;
  wire [0:0]interconnect_matrix_interconnect_aresetn;
  wire [0:0]interconnect_matrix_peripheral_aresetn1;
  wire [0:0]proc_sys_reset_0_peripheral_aresetn;
  wire [0:0]proc_sys_reset_0_peripheral_reset;
  wire processing_system7_0_FCLK_CLK0;
  wire processing_system7_0_FCLK_CLK1;
  wire processing_system7_0_FCLK_CLK2;
  wire processing_system7_0_FCLK_RESET0_N;
  wire processing_system7_0_IRQ_P2F_USB0;
  wire [31:0]processing_system7_0_M_AXI_GP0_ARADDR;
  wire [1:0]processing_system7_0_M_AXI_GP0_ARBURST;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP0_ARID;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARLEN;
  wire [1:0]processing_system7_0_M_AXI_GP0_ARLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP0_ARPROT;
  wire [3:0]processing_system7_0_M_AXI_GP0_ARQOS;
  wire processing_system7_0_M_AXI_GP0_ARREADY;
  wire [2:0]processing_system7_0_M_AXI_GP0_ARSIZE;
  wire processing_system7_0_M_AXI_GP0_ARVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_AWADDR;
  wire [1:0]processing_system7_0_M_AXI_GP0_AWBURST;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP0_AWID;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWLEN;
  wire [1:0]processing_system7_0_M_AXI_GP0_AWLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP0_AWPROT;
  wire [3:0]processing_system7_0_M_AXI_GP0_AWQOS;
  wire processing_system7_0_M_AXI_GP0_AWREADY;
  wire [2:0]processing_system7_0_M_AXI_GP0_AWSIZE;
  wire processing_system7_0_M_AXI_GP0_AWVALID;
  wire [11:0]processing_system7_0_M_AXI_GP0_BID;
  wire processing_system7_0_M_AXI_GP0_BREADY;
  wire [1:0]processing_system7_0_M_AXI_GP0_BRESP;
  wire processing_system7_0_M_AXI_GP0_BVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_RDATA;
  wire [11:0]processing_system7_0_M_AXI_GP0_RID;
  wire processing_system7_0_M_AXI_GP0_RLAST;
  wire processing_system7_0_M_AXI_GP0_RREADY;
  wire [1:0]processing_system7_0_M_AXI_GP0_RRESP;
  wire processing_system7_0_M_AXI_GP0_RVALID;
  wire [31:0]processing_system7_0_M_AXI_GP0_WDATA;
  wire [11:0]processing_system7_0_M_AXI_GP0_WID;
  wire processing_system7_0_M_AXI_GP0_WLAST;
  wire processing_system7_0_M_AXI_GP0_WREADY;
  wire [3:0]processing_system7_0_M_AXI_GP0_WSTRB;
  wire processing_system7_0_M_AXI_GP0_WVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_ARADDR;
  wire [1:0]processing_system7_0_M_AXI_GP1_ARBURST;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP1_ARID;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARLEN;
  wire [1:0]processing_system7_0_M_AXI_GP1_ARLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP1_ARPROT;
  wire [3:0]processing_system7_0_M_AXI_GP1_ARQOS;
  wire processing_system7_0_M_AXI_GP1_ARREADY;
  wire [2:0]processing_system7_0_M_AXI_GP1_ARSIZE;
  wire processing_system7_0_M_AXI_GP1_ARVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_AWADDR;
  wire [1:0]processing_system7_0_M_AXI_GP1_AWBURST;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWCACHE;
  wire [11:0]processing_system7_0_M_AXI_GP1_AWID;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWLEN;
  wire [1:0]processing_system7_0_M_AXI_GP1_AWLOCK;
  wire [2:0]processing_system7_0_M_AXI_GP1_AWPROT;
  wire [3:0]processing_system7_0_M_AXI_GP1_AWQOS;
  wire processing_system7_0_M_AXI_GP1_AWREADY;
  wire [2:0]processing_system7_0_M_AXI_GP1_AWSIZE;
  wire processing_system7_0_M_AXI_GP1_AWVALID;
  wire [11:0]processing_system7_0_M_AXI_GP1_BID;
  wire processing_system7_0_M_AXI_GP1_BREADY;
  wire [1:0]processing_system7_0_M_AXI_GP1_BRESP;
  wire processing_system7_0_M_AXI_GP1_BVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_RDATA;
  wire [11:0]processing_system7_0_M_AXI_GP1_RID;
  wire processing_system7_0_M_AXI_GP1_RLAST;
  wire processing_system7_0_M_AXI_GP1_RREADY;
  wire [1:0]processing_system7_0_M_AXI_GP1_RRESP;
  wire processing_system7_0_M_AXI_GP1_RVALID;
  wire [31:0]processing_system7_0_M_AXI_GP1_WDATA;
  wire [11:0]processing_system7_0_M_AXI_GP1_WID;
  wire processing_system7_0_M_AXI_GP1_WLAST;
  wire processing_system7_0_M_AXI_GP1_WREADY;
  wire [3:0]processing_system7_0_M_AXI_GP1_WSTRB;
  wire processing_system7_0_M_AXI_GP1_WVALID;
  wire [31:0]s_axi_ctrl_1_ARADDR;
  wire [0:0]s_axi_ctrl_1_ARREADY;
  wire [0:0]s_axi_ctrl_1_ARVALID;
  wire [31:0]s_axi_ctrl_1_AWADDR;
  wire [0:0]s_axi_ctrl_1_AWREADY;
  wire [0:0]s_axi_ctrl_1_AWVALID;
  wire [0:0]s_axi_ctrl_1_BREADY;
  wire [1:0]s_axi_ctrl_1_BRESP;
  wire [0:0]s_axi_ctrl_1_BVALID;
  wire [31:0]s_axi_ctrl_1_RDATA;
  wire [0:0]s_axi_ctrl_1_RREADY;
  wire [1:0]s_axi_ctrl_1_RRESP;
  wire [0:0]s_axi_ctrl_1_RVALID;
  wire [31:0]s_axi_ctrl_1_WDATA;
  wire [0:0]s_axi_ctrl_1_WREADY;
  wire [0:0]s_axi_ctrl_1_WVALID;
  wire [31:0]s_axi_lite1_1_ARADDR;
  wire [0:0]s_axi_lite1_1_ARREADY;
  wire [0:0]s_axi_lite1_1_ARVALID;
  wire [31:0]s_axi_lite1_1_AWADDR;
  wire [0:0]s_axi_lite1_1_AWREADY;
  wire [0:0]s_axi_lite1_1_AWVALID;
  wire [0:0]s_axi_lite1_1_BREADY;
  wire [1:0]s_axi_lite1_1_BRESP;
  wire [0:0]s_axi_lite1_1_BVALID;
  wire [31:0]s_axi_lite1_1_RDATA;
  wire [0:0]s_axi_lite1_1_RREADY;
  wire [1:0]s_axi_lite1_1_RRESP;
  wire [0:0]s_axi_lite1_1_RVALID;
  wire [31:0]s_axi_lite1_1_WDATA;
  wire [0:0]s_axi_lite1_1_WREADY;
  wire [0:0]s_axi_lite1_1_WVALID;
  wire [31:0]s_axi_lite1_2_ARADDR;
  wire [0:0]s_axi_lite1_2_ARREADY;
  wire [0:0]s_axi_lite1_2_ARVALID;
  wire [31:0]s_axi_lite1_2_AWADDR;
  wire [0:0]s_axi_lite1_2_AWREADY;
  wire [0:0]s_axi_lite1_2_AWVALID;
  wire [0:0]s_axi_lite1_2_BREADY;
  wire [1:0]s_axi_lite1_2_BRESP;
  wire [0:0]s_axi_lite1_2_BVALID;
  wire [31:0]s_axi_lite1_2_RDATA;
  wire [0:0]s_axi_lite1_2_RREADY;
  wire [1:0]s_axi_lite1_2_RRESP;
  wire [0:0]s_axi_lite1_2_RVALID;
  wire [31:0]s_axi_lite1_2_WDATA;
  wire [0:0]s_axi_lite1_2_WREADY;
  wire [3:0]s_axi_lite1_2_WSTRB;
  wire [0:0]s_axi_lite1_2_WVALID;
  wire [31:0]s_axi_lite_2_ARADDR;
  wire [0:0]s_axi_lite_2_ARREADY;
  wire [0:0]s_axi_lite_2_ARVALID;
  wire [31:0]s_axi_lite_2_AWADDR;
  wire [0:0]s_axi_lite_2_AWREADY;
  wire [0:0]s_axi_lite_2_AWVALID;
  wire [0:0]s_axi_lite_2_BREADY;
  wire [1:0]s_axi_lite_2_BRESP;
  wire [0:0]s_axi_lite_2_BVALID;
  wire [31:0]s_axi_lite_2_RDATA;
  wire [0:0]s_axi_lite_2_RREADY;
  wire [1:0]s_axi_lite_2_RRESP;
  wire [0:0]s_axi_lite_2_RVALID;
  wire [31:0]s_axi_lite_2_WDATA;
  wire [0:0]s_axi_lite_2_WREADY;
  wire [3:0]s_axi_lite_2_WSTRB;
  wire [0:0]s_axi_lite_2_WVALID;
  wire vid_clk_1;
  wire [5:0]xlconcat_0_dout;

  assign AXI1_clk = clock_generation_AXI_clk;
  assign AXI_clk = processing_system7_0_FCLK_CLK1;
  assign BCLK_clk = clock_generation_BCLK_clk;
  assign CLK18M_clk = aud_mclk_1;
  assign Conn2_SCL_I = IIC_0_scl_i;
  assign Conn2_SDA_I = IIC_0_sda_i;
  assign I2SO_D0 = audio_video_engine_I2SO_D0;
  assign I2S_FSYNC_OUT = audio_video_engine_I2S_FSYNC_OUT;
  assign I2S_SCLK = audio_video_engine_I2S_SCLK;
  assign IIC_0_scl_o = Conn2_SCL_O;
  assign IIC_0_scl_t = Conn2_SCL_T;
  assign IIC_0_sda_o = Conn2_SDA_O;
  assign IIC_0_sda_t = Conn2_SDA_T;
  assign M_AXI1_araddr[31:0] = axi_protocol_convert_0_M_AXI_ARADDR;
  assign M_AXI1_arprot[2:0] = axi_protocol_convert_0_M_AXI_ARPROT;
  assign M_AXI1_arvalid = axi_protocol_convert_0_M_AXI_ARVALID;
  assign M_AXI1_awaddr[31:0] = axi_protocol_convert_0_M_AXI_AWADDR;
  assign M_AXI1_awprot[2:0] = axi_protocol_convert_0_M_AXI_AWPROT;
  assign M_AXI1_awvalid = axi_protocol_convert_0_M_AXI_AWVALID;
  assign M_AXI1_bready = axi_protocol_convert_0_M_AXI_BREADY;
  assign M_AXI1_rready = axi_protocol_convert_0_M_AXI_RREADY;
  assign M_AXI1_wdata[31:0] = axi_protocol_convert_0_M_AXI_WDATA;
  assign M_AXI1_wstrb[3:0] = axi_protocol_convert_0_M_AXI_WSTRB;
  assign M_AXI1_wvalid = axi_protocol_convert_0_M_AXI_WVALID;
  assign M_AXI_araddr[31:0] = interconnect_matrix_M01_AXI_ARADDR;
  assign M_AXI_arprot[2:0] = interconnect_matrix_M01_AXI_ARPROT;
  assign M_AXI_arvalid = interconnect_matrix_M01_AXI_ARVALID;
  assign M_AXI_awaddr[31:0] = interconnect_matrix_M01_AXI_AWADDR;
  assign M_AXI_awprot[2:0] = interconnect_matrix_M01_AXI_AWPROT;
  assign M_AXI_awvalid = interconnect_matrix_M01_AXI_AWVALID;
  assign M_AXI_bready = interconnect_matrix_M01_AXI_BREADY;
  assign M_AXI_rready = interconnect_matrix_M01_AXI_RREADY;
  assign M_AXI_wdata[31:0] = interconnect_matrix_M01_AXI_WDATA;
  assign M_AXI_wstrb[3:0] = interconnect_matrix_M01_AXI_WSTRB;
  assign M_AXI_wvalid = interconnect_matrix_M01_AXI_WVALID;
  assign PCLK_clk = clock_generation_PCLK_clk;
  assign PCLK_reg = clock_generation_PCLK_reg;
  assign S_AXI_2_ARADDR = S_AXI_araddr[31:0];
  assign S_AXI_2_ARBURST = S_AXI_arburst[1:0];
  assign S_AXI_2_ARCACHE = S_AXI_arcache[3:0];
  assign S_AXI_2_ARLEN = S_AXI_arlen[3:0];
  assign S_AXI_2_ARLOCK = S_AXI_arlock;
  assign S_AXI_2_ARPROT = S_AXI_arprot[2:0];
  assign S_AXI_2_ARSIZE = S_AXI_arsize[2:0];
  assign S_AXI_2_ARUSER = S_AXI_aruser[4:0];
  assign S_AXI_2_ARVALID = S_AXI_arvalid;
  assign S_AXI_2_AWADDR = S_AXI_awaddr[31:0];
  assign S_AXI_2_AWBURST = S_AXI_awburst[1:0];
  assign S_AXI_2_AWCACHE = S_AXI_awcache[3:0];
  assign S_AXI_2_AWLEN = S_AXI_awlen[3:0];
  assign S_AXI_2_AWLOCK = S_AXI_awlock;
  assign S_AXI_2_AWPROT = S_AXI_awprot[2:0];
  assign S_AXI_2_AWSIZE = S_AXI_awsize[2:0];
  assign S_AXI_2_AWUSER = S_AXI_awuser[4:0];
  assign S_AXI_2_AWVALID = S_AXI_awvalid;
  assign S_AXI_2_BREADY = S_AXI_bready;
  assign S_AXI_2_RREADY = S_AXI_rready;
  assign S_AXI_2_WDATA = S_AXI_wdata[31:0];
  assign S_AXI_2_WLAST = S_AXI_wlast;
  assign S_AXI_2_WSTRB = S_AXI_wstrb[3:0];
  assign S_AXI_2_WVALID = S_AXI_wvalid;
  assign S_AXI_arready = S_AXI_2_ARREADY;
  assign S_AXI_awready = S_AXI_2_AWREADY;
  assign S_AXI_bresp[1:0] = S_AXI_2_BRESP;
  assign S_AXI_bvalid = S_AXI_2_BVALID;
  assign S_AXI_rdata[31:0] = S_AXI_2_RDATA;
  assign S_AXI_rlast = S_AXI_2_RLAST;
  assign S_AXI_rresp[1:0] = S_AXI_2_RRESP;
  assign S_AXI_rvalid = S_AXI_2_RVALID;
  assign S_AXI_wready = S_AXI_2_WREADY;
  assign aresetn[0] = interconnect_matrix_interconnect_aresetn;
  assign aresetn1[0] = proc_sys_reset_0_peripheral_aresetn;
  assign axi_protocol_convert_0_M_AXI_ARREADY = M_AXI1_arready;
  assign axi_protocol_convert_0_M_AXI_AWREADY = M_AXI1_awready;
  assign axi_protocol_convert_0_M_AXI_BRESP = M_AXI1_bresp[1:0];
  assign axi_protocol_convert_0_M_AXI_BVALID = M_AXI1_bvalid;
  assign axi_protocol_convert_0_M_AXI_RDATA = M_AXI1_rdata[31:0];
  assign axi_protocol_convert_0_M_AXI_RRESP = M_AXI1_rresp[1:0];
  assign axi_protocol_convert_0_M_AXI_RVALID = M_AXI1_rvalid;
  assign axi_protocol_convert_0_M_AXI_WREADY = M_AXI1_wready;
  assign clk90_detected = clock_generation_clk90_detected;
  assign control_vblank[1:0] = audio_video_engine_control_vblank;
  assign cpuclk_detected = clock_generation_cpuclk_detected;
  assign enable_clk_output_1 = enable_clk_output;
  assign hdmi_clk = audio_video_engine_hdmi_clk;
  assign hdmi_data[15:0] = audio_video_engine_hdmi_data;
  assign hdmi_de = audio_video_engine_hdmi_de;
  assign hdmi_hs = audio_video_engine_hdmi_hs;
  assign hdmi_intn_1 = hdmi_intn[0];
  assign hdmi_vs = audio_video_engine_hdmi_vs;
  assign interconnect_matrix_M01_AXI_ARREADY = M_AXI_arready;
  assign interconnect_matrix_M01_AXI_AWREADY = M_AXI_awready;
  assign interconnect_matrix_M01_AXI_BRESP = M_AXI_bresp[1:0];
  assign interconnect_matrix_M01_AXI_BVALID = M_AXI_bvalid;
  assign interconnect_matrix_M01_AXI_RDATA = M_AXI_rdata[31:0];
  assign interconnect_matrix_M01_AXI_RRESP = M_AXI_rresp[1:0];
  assign interconnect_matrix_M01_AXI_RVALID = M_AXI_rvalid;
  assign interconnect_matrix_M01_AXI_WREADY = M_AXI_wready;
  assign nCLKEN_clk = clock_generation_nCLKEN_clk;
  assign nCLKEN_reg = clock_generation_nCLKEN_reg;
  assign peripheral_reset[0] = proc_sys_reset_0_peripheral_reset;
  audio_video_engine_imp_14WDLA5 audio_video_engine
       (.I2SO_D0(audio_video_engine_I2SO_D0),
        .I2S_FSYNC_OUT(audio_video_engine_I2S_FSYNC_OUT),
        .I2S_SCLK(audio_video_engine_I2S_SCLK),
        .M00_ACLK(processing_system7_0_FCLK_CLK0),
        .M00_AXI_araddr(audio_video_engine_M00_AXI_ARADDR),
        .M00_AXI_arburst(audio_video_engine_M00_AXI_ARBURST),
        .M00_AXI_arcache(audio_video_engine_M00_AXI_ARCACHE),
        .M00_AXI_arlen(audio_video_engine_M00_AXI_ARLEN),
        .M00_AXI_arlock(audio_video_engine_M00_AXI_ARLOCK),
        .M00_AXI_arprot(audio_video_engine_M00_AXI_ARPROT),
        .M00_AXI_arqos(audio_video_engine_M00_AXI_ARQOS),
        .M00_AXI_arready(audio_video_engine_M00_AXI_ARREADY),
        .M00_AXI_arsize(audio_video_engine_M00_AXI_ARSIZE),
        .M00_AXI_arvalid(audio_video_engine_M00_AXI_ARVALID),
        .M00_AXI_rdata(audio_video_engine_M00_AXI_RDATA),
        .M00_AXI_rlast(audio_video_engine_M00_AXI_RLAST),
        .M00_AXI_rready(audio_video_engine_M00_AXI_RREADY),
        .M00_AXI_rresp(audio_video_engine_M00_AXI_RRESP),
        .M00_AXI_rvalid(audio_video_engine_M00_AXI_RVALID),
        .M_AXI_araddr(audio_video_engine_M_AXI_ARADDR),
        .M_AXI_arburst(audio_video_engine_M_AXI_ARBURST),
        .M_AXI_arcache(audio_video_engine_M_AXI_ARCACHE),
        .M_AXI_arlen(audio_video_engine_M_AXI_ARLEN),
        .M_AXI_arlock(audio_video_engine_M_AXI_ARLOCK),
        .M_AXI_arprot(audio_video_engine_M_AXI_ARPROT),
        .M_AXI_arqos(audio_video_engine_M_AXI_ARQOS),
        .M_AXI_arready(audio_video_engine_M_AXI_ARREADY),
        .M_AXI_arsize(audio_video_engine_M_AXI_ARSIZE),
        .M_AXI_arvalid(audio_video_engine_M_AXI_ARVALID),
        .M_AXI_rdata(audio_video_engine_M_AXI_RDATA),
        .M_AXI_rlast(audio_video_engine_M_AXI_RLAST),
        .M_AXI_rready(audio_video_engine_M_AXI_RREADY),
        .M_AXI_rresp(audio_video_engine_M_AXI_RRESP),
        .M_AXI_rvalid(audio_video_engine_M_AXI_RVALID),
        .S_AXI_LITE_araddr(S_AXI_LITE_1_ARADDR),
        .S_AXI_LITE_arready(S_AXI_LITE_1_ARREADY),
        .S_AXI_LITE_arvalid(S_AXI_LITE_1_ARVALID),
        .S_AXI_LITE_awaddr(S_AXI_LITE_1_AWADDR),
        .S_AXI_LITE_awready(S_AXI_LITE_1_AWREADY),
        .S_AXI_LITE_awvalid(S_AXI_LITE_1_AWVALID),
        .S_AXI_LITE_bready(S_AXI_LITE_1_BREADY),
        .S_AXI_LITE_bresp(S_AXI_LITE_1_BRESP),
        .S_AXI_LITE_bvalid(S_AXI_LITE_1_BVALID),
        .S_AXI_LITE_rdata(S_AXI_LITE_1_RDATA),
        .S_AXI_LITE_rready(S_AXI_LITE_1_RREADY),
        .S_AXI_LITE_rresp(S_AXI_LITE_1_RRESP),
        .S_AXI_LITE_rvalid(S_AXI_LITE_1_RVALID),
        .S_AXI_LITE_wdata(S_AXI_LITE_1_WDATA),
        .S_AXI_LITE_wready(S_AXI_LITE_1_WREADY),
        .S_AXI_LITE_wvalid(S_AXI_LITE_1_WVALID),
        .S_AXI_araddr(S_AXI_1_ARADDR),
        .S_AXI_arprot(S_AXI_1_ARPROT),
        .S_AXI_arready(S_AXI_1_ARREADY),
        .S_AXI_arvalid(S_AXI_1_ARVALID),
        .S_AXI_awaddr(S_AXI_1_AWADDR),
        .S_AXI_awprot(S_AXI_1_AWPROT),
        .S_AXI_awready(S_AXI_1_AWREADY),
        .S_AXI_awvalid(S_AXI_1_AWVALID),
        .S_AXI_bready(S_AXI_1_BREADY),
        .S_AXI_bresp(S_AXI_1_BRESP),
        .S_AXI_bvalid(S_AXI_1_BVALID),
        .S_AXI_rdata(S_AXI_1_RDATA),
        .S_AXI_rready(S_AXI_1_RREADY),
        .S_AXI_rresp(S_AXI_1_RRESP),
        .S_AXI_rvalid(S_AXI_1_RVALID),
        .S_AXI_wdata(S_AXI_1_WDATA),
        .S_AXI_wready(S_AXI_1_WREADY),
        .S_AXI_wstrb(S_AXI_1_WSTRB),
        .S_AXI_wvalid(S_AXI_1_WVALID),
        .aud_mclk(aud_mclk_1),
        .control_vblank(audio_video_engine_control_vblank),
        .dout(audio_video_engine_dout),
        .ext_reset_in(processing_system7_0_FCLK_RESET0_N),
        .hdmi_clk(audio_video_engine_hdmi_clk),
        .hdmi_data(audio_video_engine_hdmi_data),
        .hdmi_de(audio_video_engine_hdmi_de),
        .hdmi_hs(audio_video_engine_hdmi_hs),
        .hdmi_intn(hdmi_intn_1),
        .hdmi_vs(audio_video_engine_hdmi_vs),
        .s_axi_ctrl_aclk(processing_system7_0_FCLK_CLK1),
        .s_axi_ctrl_araddr(s_axi_ctrl_1_ARADDR),
        .s_axi_ctrl_aresetn(interconnect_matrix_peripheral_aresetn1),
        .s_axi_ctrl_arready(s_axi_ctrl_1_ARREADY),
        .s_axi_ctrl_arvalid(s_axi_ctrl_1_ARVALID),
        .s_axi_ctrl_awaddr(s_axi_ctrl_1_AWADDR),
        .s_axi_ctrl_awready(s_axi_ctrl_1_AWREADY),
        .s_axi_ctrl_awvalid(s_axi_ctrl_1_AWVALID),
        .s_axi_ctrl_bready(s_axi_ctrl_1_BREADY),
        .s_axi_ctrl_bresp(s_axi_ctrl_1_BRESP),
        .s_axi_ctrl_bvalid(s_axi_ctrl_1_BVALID),
        .s_axi_ctrl_rdata(s_axi_ctrl_1_RDATA),
        .s_axi_ctrl_rready(s_axi_ctrl_1_RREADY),
        .s_axi_ctrl_rresp(s_axi_ctrl_1_RRESP),
        .s_axi_ctrl_rvalid(s_axi_ctrl_1_RVALID),
        .s_axi_ctrl_wdata(s_axi_ctrl_1_WDATA),
        .s_axi_ctrl_wready(s_axi_ctrl_1_WREADY),
        .s_axi_ctrl_wvalid(s_axi_ctrl_1_WVALID),
        .s_axi_lite1_araddr(s_axi_lite1_1_ARADDR),
        .s_axi_lite1_arready(s_axi_lite1_1_ARREADY),
        .s_axi_lite1_arvalid(s_axi_lite1_1_ARVALID),
        .s_axi_lite1_awaddr(s_axi_lite1_1_AWADDR),
        .s_axi_lite1_awready(s_axi_lite1_1_AWREADY),
        .s_axi_lite1_awvalid(s_axi_lite1_1_AWVALID),
        .s_axi_lite1_bready(s_axi_lite1_1_BREADY),
        .s_axi_lite1_bresp(s_axi_lite1_1_BRESP),
        .s_axi_lite1_bvalid(s_axi_lite1_1_BVALID),
        .s_axi_lite1_rdata(s_axi_lite1_1_RDATA),
        .s_axi_lite1_rready(s_axi_lite1_1_RREADY),
        .s_axi_lite1_rresp(s_axi_lite1_1_RRESP),
        .s_axi_lite1_rvalid(s_axi_lite1_1_RVALID),
        .s_axi_lite1_wdata(s_axi_lite1_1_WDATA),
        .s_axi_lite1_wready(s_axi_lite1_1_WREADY),
        .s_axi_lite1_wvalid(s_axi_lite1_1_WVALID),
        .s_axis_aud_aclk(processing_system7_0_FCLK_CLK2),
        .vid_clk(vid_clk_1));
  design_1_axi_dwidth_converter_0_0 axi_dwidth_converter_0
       (.m_axi_araddr(axi_dwidth_converter_0_M_AXI_ARADDR),
        .m_axi_arburst(axi_dwidth_converter_0_M_AXI_ARBURST),
        .m_axi_arcache(axi_dwidth_converter_0_M_AXI_ARCACHE),
        .m_axi_arlen(axi_dwidth_converter_0_M_AXI_ARLEN),
        .m_axi_arlock(axi_dwidth_converter_0_M_AXI_ARLOCK),
        .m_axi_arprot(axi_dwidth_converter_0_M_AXI_ARPROT),
        .m_axi_arqos(axi_dwidth_converter_0_M_AXI_ARQOS),
        .m_axi_arready(axi_dwidth_converter_0_M_AXI_ARREADY),
        .m_axi_arsize(axi_dwidth_converter_0_M_AXI_ARSIZE),
        .m_axi_arvalid(axi_dwidth_converter_0_M_AXI_ARVALID),
        .m_axi_awaddr(axi_dwidth_converter_0_M_AXI_AWADDR),
        .m_axi_awburst(axi_dwidth_converter_0_M_AXI_AWBURST),
        .m_axi_awcache(axi_dwidth_converter_0_M_AXI_AWCACHE),
        .m_axi_awlen(axi_dwidth_converter_0_M_AXI_AWLEN),
        .m_axi_awlock(axi_dwidth_converter_0_M_AXI_AWLOCK),
        .m_axi_awprot(axi_dwidth_converter_0_M_AXI_AWPROT),
        .m_axi_awqos(axi_dwidth_converter_0_M_AXI_AWQOS),
        .m_axi_awready(axi_dwidth_converter_0_M_AXI_AWREADY),
        .m_axi_awsize(axi_dwidth_converter_0_M_AXI_AWSIZE),
        .m_axi_awvalid(axi_dwidth_converter_0_M_AXI_AWVALID),
        .m_axi_bready(axi_dwidth_converter_0_M_AXI_BREADY),
        .m_axi_bresp(axi_dwidth_converter_0_M_AXI_BRESP),
        .m_axi_bvalid(axi_dwidth_converter_0_M_AXI_BVALID),
        .m_axi_rdata(axi_dwidth_converter_0_M_AXI_RDATA),
        .m_axi_rlast(axi_dwidth_converter_0_M_AXI_RLAST),
        .m_axi_rready(axi_dwidth_converter_0_M_AXI_RREADY),
        .m_axi_rresp(axi_dwidth_converter_0_M_AXI_RRESP),
        .m_axi_rvalid(axi_dwidth_converter_0_M_AXI_RVALID),
        .m_axi_wdata(axi_dwidth_converter_0_M_AXI_WDATA),
        .m_axi_wlast(axi_dwidth_converter_0_M_AXI_WLAST),
        .m_axi_wready(axi_dwidth_converter_0_M_AXI_WREADY),
        .m_axi_wstrb(axi_dwidth_converter_0_M_AXI_WSTRB),
        .m_axi_wvalid(axi_dwidth_converter_0_M_AXI_WVALID),
        .s_axi_aclk(clock_generation_AXI_clk),
        .s_axi_araddr(axi_register_slice_1_M_AXI_ARADDR),
        .s_axi_arburst(axi_register_slice_1_M_AXI_ARBURST),
        .s_axi_arcache(axi_register_slice_1_M_AXI_ARCACHE),
        .s_axi_aresetn(proc_sys_reset_0_peripheral_aresetn),
        .s_axi_arlen(axi_register_slice_1_M_AXI_ARLEN),
        .s_axi_arlock(axi_register_slice_1_M_AXI_ARLOCK),
        .s_axi_arprot(axi_register_slice_1_M_AXI_ARPROT),
        .s_axi_arqos(axi_register_slice_1_M_AXI_ARQOS),
        .s_axi_arready(axi_register_slice_1_M_AXI_ARREADY),
        .s_axi_arsize(axi_register_slice_1_M_AXI_ARSIZE),
        .s_axi_arvalid(axi_register_slice_1_M_AXI_ARVALID),
        .s_axi_awaddr(axi_register_slice_1_M_AXI_AWADDR),
        .s_axi_awburst(axi_register_slice_1_M_AXI_AWBURST),
        .s_axi_awcache(axi_register_slice_1_M_AXI_AWCACHE),
        .s_axi_awlen(axi_register_slice_1_M_AXI_AWLEN),
        .s_axi_awlock(axi_register_slice_1_M_AXI_AWLOCK),
        .s_axi_awprot(axi_register_slice_1_M_AXI_AWPROT),
        .s_axi_awqos(axi_register_slice_1_M_AXI_AWQOS),
        .s_axi_awready(axi_register_slice_1_M_AXI_AWREADY),
        .s_axi_awsize(axi_register_slice_1_M_AXI_AWSIZE),
        .s_axi_awvalid(axi_register_slice_1_M_AXI_AWVALID),
        .s_axi_bready(axi_register_slice_1_M_AXI_BREADY),
        .s_axi_bresp(axi_register_slice_1_M_AXI_BRESP),
        .s_axi_bvalid(axi_register_slice_1_M_AXI_BVALID),
        .s_axi_rdata(axi_register_slice_1_M_AXI_RDATA),
        .s_axi_rlast(axi_register_slice_1_M_AXI_RLAST),
        .s_axi_rready(axi_register_slice_1_M_AXI_RREADY),
        .s_axi_rresp(axi_register_slice_1_M_AXI_RRESP),
        .s_axi_rvalid(axi_register_slice_1_M_AXI_RVALID),
        .s_axi_wdata(axi_register_slice_1_M_AXI_WDATA),
        .s_axi_wlast(axi_register_slice_1_M_AXI_WLAST),
        .s_axi_wready(axi_register_slice_1_M_AXI_WREADY),
        .s_axi_wstrb(axi_register_slice_1_M_AXI_WSTRB),
        .s_axi_wvalid(axi_register_slice_1_M_AXI_WVALID));
  design_1_axi_protocol_convert_0_0 axi_protocol_convert_0
       (.aclk(clock_generation_AXI_clk),
        .aresetn(proc_sys_reset_0_peripheral_aresetn),
        .m_axi_araddr(axi_protocol_convert_0_M_AXI_ARADDR),
        .m_axi_arprot(axi_protocol_convert_0_M_AXI_ARPROT),
        .m_axi_arready(axi_protocol_convert_0_M_AXI_ARREADY),
        .m_axi_arvalid(axi_protocol_convert_0_M_AXI_ARVALID),
        .m_axi_awaddr(axi_protocol_convert_0_M_AXI_AWADDR),
        .m_axi_awprot(axi_protocol_convert_0_M_AXI_AWPROT),
        .m_axi_awready(axi_protocol_convert_0_M_AXI_AWREADY),
        .m_axi_awvalid(axi_protocol_convert_0_M_AXI_AWVALID),
        .m_axi_bready(axi_protocol_convert_0_M_AXI_BREADY),
        .m_axi_bresp(axi_protocol_convert_0_M_AXI_BRESP),
        .m_axi_bvalid(axi_protocol_convert_0_M_AXI_BVALID),
        .m_axi_rdata(axi_protocol_convert_0_M_AXI_RDATA),
        .m_axi_rready(axi_protocol_convert_0_M_AXI_RREADY),
        .m_axi_rresp(axi_protocol_convert_0_M_AXI_RRESP),
        .m_axi_rvalid(axi_protocol_convert_0_M_AXI_RVALID),
        .m_axi_wdata(axi_protocol_convert_0_M_AXI_WDATA),
        .m_axi_wready(axi_protocol_convert_0_M_AXI_WREADY),
        .m_axi_wstrb(axi_protocol_convert_0_M_AXI_WSTRB),
        .m_axi_wvalid(axi_protocol_convert_0_M_AXI_WVALID),
        .s_axi_araddr(processing_system7_0_M_AXI_GP1_ARADDR),
        .s_axi_arburst(processing_system7_0_M_AXI_GP1_ARBURST),
        .s_axi_arcache(processing_system7_0_M_AXI_GP1_ARCACHE),
        .s_axi_arid(processing_system7_0_M_AXI_GP1_ARID),
        .s_axi_arlen(processing_system7_0_M_AXI_GP1_ARLEN),
        .s_axi_arlock(processing_system7_0_M_AXI_GP1_ARLOCK),
        .s_axi_arprot(processing_system7_0_M_AXI_GP1_ARPROT),
        .s_axi_arqos(processing_system7_0_M_AXI_GP1_ARQOS),
        .s_axi_arready(processing_system7_0_M_AXI_GP1_ARREADY),
        .s_axi_arsize(processing_system7_0_M_AXI_GP1_ARSIZE),
        .s_axi_arvalid(processing_system7_0_M_AXI_GP1_ARVALID),
        .s_axi_awaddr(processing_system7_0_M_AXI_GP1_AWADDR),
        .s_axi_awburst(processing_system7_0_M_AXI_GP1_AWBURST),
        .s_axi_awcache(processing_system7_0_M_AXI_GP1_AWCACHE),
        .s_axi_awid(processing_system7_0_M_AXI_GP1_AWID),
        .s_axi_awlen(processing_system7_0_M_AXI_GP1_AWLEN),
        .s_axi_awlock(processing_system7_0_M_AXI_GP1_AWLOCK),
        .s_axi_awprot(processing_system7_0_M_AXI_GP1_AWPROT),
        .s_axi_awqos(processing_system7_0_M_AXI_GP1_AWQOS),
        .s_axi_awready(processing_system7_0_M_AXI_GP1_AWREADY),
        .s_axi_awsize(processing_system7_0_M_AXI_GP1_AWSIZE),
        .s_axi_awvalid(processing_system7_0_M_AXI_GP1_AWVALID),
        .s_axi_bid(processing_system7_0_M_AXI_GP1_BID),
        .s_axi_bready(processing_system7_0_M_AXI_GP1_BREADY),
        .s_axi_bresp(processing_system7_0_M_AXI_GP1_BRESP),
        .s_axi_bvalid(processing_system7_0_M_AXI_GP1_BVALID),
        .s_axi_rdata(processing_system7_0_M_AXI_GP1_RDATA),
        .s_axi_rid(processing_system7_0_M_AXI_GP1_RID),
        .s_axi_rlast(processing_system7_0_M_AXI_GP1_RLAST),
        .s_axi_rready(processing_system7_0_M_AXI_GP1_RREADY),
        .s_axi_rresp(processing_system7_0_M_AXI_GP1_RRESP),
        .s_axi_rvalid(processing_system7_0_M_AXI_GP1_RVALID),
        .s_axi_wdata(processing_system7_0_M_AXI_GP1_WDATA),
        .s_axi_wid(processing_system7_0_M_AXI_GP1_WID),
        .s_axi_wlast(processing_system7_0_M_AXI_GP1_WLAST),
        .s_axi_wready(processing_system7_0_M_AXI_GP1_WREADY),
        .s_axi_wstrb(processing_system7_0_M_AXI_GP1_WSTRB),
        .s_axi_wvalid(processing_system7_0_M_AXI_GP1_WVALID));
  design_1_axi_register_slice_0_1 axi_register_slice_0
       (.aclk(clock_generation_AXI_clk),
        .aresetn(proc_sys_reset_0_peripheral_aresetn),
        .m_axi_araddr(axi_register_slice_0_M_AXI_ARADDR),
        .m_axi_arburst(axi_register_slice_0_M_AXI_ARBURST),
        .m_axi_arcache(axi_register_slice_0_M_AXI_ARCACHE),
        .m_axi_arlen(axi_register_slice_0_M_AXI_ARLEN),
        .m_axi_arlock(axi_register_slice_0_M_AXI_ARLOCK),
        .m_axi_arprot(axi_register_slice_0_M_AXI_ARPROT),
        .m_axi_arqos(axi_register_slice_0_M_AXI_ARQOS),
        .m_axi_arready(axi_register_slice_0_M_AXI_ARREADY),
        .m_axi_arsize(axi_register_slice_0_M_AXI_ARSIZE),
        .m_axi_arvalid(axi_register_slice_0_M_AXI_ARVALID),
        .m_axi_awaddr(axi_register_slice_0_M_AXI_AWADDR),
        .m_axi_awburst(axi_register_slice_0_M_AXI_AWBURST),
        .m_axi_awcache(axi_register_slice_0_M_AXI_AWCACHE),
        .m_axi_awlen(axi_register_slice_0_M_AXI_AWLEN),
        .m_axi_awlock(axi_register_slice_0_M_AXI_AWLOCK),
        .m_axi_awprot(axi_register_slice_0_M_AXI_AWPROT),
        .m_axi_awqos(axi_register_slice_0_M_AXI_AWQOS),
        .m_axi_awready(axi_register_slice_0_M_AXI_AWREADY),
        .m_axi_awsize(axi_register_slice_0_M_AXI_AWSIZE),
        .m_axi_awvalid(axi_register_slice_0_M_AXI_AWVALID),
        .m_axi_bready(axi_register_slice_0_M_AXI_BREADY),
        .m_axi_bresp(axi_register_slice_0_M_AXI_BRESP),
        .m_axi_bvalid(axi_register_slice_0_M_AXI_BVALID),
        .m_axi_rdata(axi_register_slice_0_M_AXI_RDATA),
        .m_axi_rlast(axi_register_slice_0_M_AXI_RLAST),
        .m_axi_rready(axi_register_slice_0_M_AXI_RREADY),
        .m_axi_rresp(axi_register_slice_0_M_AXI_RRESP),
        .m_axi_rvalid(axi_register_slice_0_M_AXI_RVALID),
        .m_axi_wdata(axi_register_slice_0_M_AXI_WDATA),
        .m_axi_wlast(axi_register_slice_0_M_AXI_WLAST),
        .m_axi_wready(axi_register_slice_0_M_AXI_WREADY),
        .m_axi_wstrb(axi_register_slice_0_M_AXI_WSTRB),
        .m_axi_wvalid(axi_register_slice_0_M_AXI_WVALID),
        .s_axi_araddr(axi_dwidth_converter_0_M_AXI_ARADDR),
        .s_axi_arburst(axi_dwidth_converter_0_M_AXI_ARBURST),
        .s_axi_arcache(axi_dwidth_converter_0_M_AXI_ARCACHE),
        .s_axi_arlen(axi_dwidth_converter_0_M_AXI_ARLEN),
        .s_axi_arlock(axi_dwidth_converter_0_M_AXI_ARLOCK),
        .s_axi_arprot(axi_dwidth_converter_0_M_AXI_ARPROT),
        .s_axi_arqos(axi_dwidth_converter_0_M_AXI_ARQOS),
        .s_axi_arready(axi_dwidth_converter_0_M_AXI_ARREADY),
        .s_axi_arsize(axi_dwidth_converter_0_M_AXI_ARSIZE),
        .s_axi_arvalid(axi_dwidth_converter_0_M_AXI_ARVALID),
        .s_axi_awaddr(axi_dwidth_converter_0_M_AXI_AWADDR),
        .s_axi_awburst(axi_dwidth_converter_0_M_AXI_AWBURST),
        .s_axi_awcache(axi_dwidth_converter_0_M_AXI_AWCACHE),
        .s_axi_awlen(axi_dwidth_converter_0_M_AXI_AWLEN),
        .s_axi_awlock(axi_dwidth_converter_0_M_AXI_AWLOCK),
        .s_axi_awprot(axi_dwidth_converter_0_M_AXI_AWPROT),
        .s_axi_awqos(axi_dwidth_converter_0_M_AXI_AWQOS),
        .s_axi_awready(axi_dwidth_converter_0_M_AXI_AWREADY),
        .s_axi_awsize(axi_dwidth_converter_0_M_AXI_AWSIZE),
        .s_axi_awvalid(axi_dwidth_converter_0_M_AXI_AWVALID),
        .s_axi_bready(axi_dwidth_converter_0_M_AXI_BREADY),
        .s_axi_bresp(axi_dwidth_converter_0_M_AXI_BRESP),
        .s_axi_bvalid(axi_dwidth_converter_0_M_AXI_BVALID),
        .s_axi_rdata(axi_dwidth_converter_0_M_AXI_RDATA),
        .s_axi_rlast(axi_dwidth_converter_0_M_AXI_RLAST),
        .s_axi_rready(axi_dwidth_converter_0_M_AXI_RREADY),
        .s_axi_rresp(axi_dwidth_converter_0_M_AXI_RRESP),
        .s_axi_rvalid(axi_dwidth_converter_0_M_AXI_RVALID),
        .s_axi_wdata(axi_dwidth_converter_0_M_AXI_WDATA),
        .s_axi_wlast(axi_dwidth_converter_0_M_AXI_WLAST),
        .s_axi_wready(axi_dwidth_converter_0_M_AXI_WREADY),
        .s_axi_wstrb(axi_dwidth_converter_0_M_AXI_WSTRB),
        .s_axi_wvalid(axi_dwidth_converter_0_M_AXI_WVALID));
  design_1_axi_register_slice_1_0 axi_register_slice_1
       (.aclk(clock_generation_AXI_clk),
        .aresetn(proc_sys_reset_0_peripheral_aresetn),
        .m_axi_araddr(axi_register_slice_1_M_AXI_ARADDR),
        .m_axi_arburst(axi_register_slice_1_M_AXI_ARBURST),
        .m_axi_arcache(axi_register_slice_1_M_AXI_ARCACHE),
        .m_axi_arlen(axi_register_slice_1_M_AXI_ARLEN),
        .m_axi_arlock(axi_register_slice_1_M_AXI_ARLOCK),
        .m_axi_arprot(axi_register_slice_1_M_AXI_ARPROT),
        .m_axi_arqos(axi_register_slice_1_M_AXI_ARQOS),
        .m_axi_arready(axi_register_slice_1_M_AXI_ARREADY),
        .m_axi_arsize(axi_register_slice_1_M_AXI_ARSIZE),
        .m_axi_arvalid(axi_register_slice_1_M_AXI_ARVALID),
        .m_axi_awaddr(axi_register_slice_1_M_AXI_AWADDR),
        .m_axi_awburst(axi_register_slice_1_M_AXI_AWBURST),
        .m_axi_awcache(axi_register_slice_1_M_AXI_AWCACHE),
        .m_axi_awlen(axi_register_slice_1_M_AXI_AWLEN),
        .m_axi_awlock(axi_register_slice_1_M_AXI_AWLOCK),
        .m_axi_awprot(axi_register_slice_1_M_AXI_AWPROT),
        .m_axi_awqos(axi_register_slice_1_M_AXI_AWQOS),
        .m_axi_awready(axi_register_slice_1_M_AXI_AWREADY),
        .m_axi_awsize(axi_register_slice_1_M_AXI_AWSIZE),
        .m_axi_awvalid(axi_register_slice_1_M_AXI_AWVALID),
        .m_axi_bready(axi_register_slice_1_M_AXI_BREADY),
        .m_axi_bresp(axi_register_slice_1_M_AXI_BRESP),
        .m_axi_bvalid(axi_register_slice_1_M_AXI_BVALID),
        .m_axi_rdata(axi_register_slice_1_M_AXI_RDATA),
        .m_axi_rlast(axi_register_slice_1_M_AXI_RLAST),
        .m_axi_rready(axi_register_slice_1_M_AXI_RREADY),
        .m_axi_rresp(axi_register_slice_1_M_AXI_RRESP),
        .m_axi_rvalid(axi_register_slice_1_M_AXI_RVALID),
        .m_axi_wdata(axi_register_slice_1_M_AXI_WDATA),
        .m_axi_wlast(axi_register_slice_1_M_AXI_WLAST),
        .m_axi_wready(axi_register_slice_1_M_AXI_WREADY),
        .m_axi_wstrb(axi_register_slice_1_M_AXI_WSTRB),
        .m_axi_wvalid(axi_register_slice_1_M_AXI_WVALID),
        .s_axi_araddr(S_AXI_2_ARADDR),
        .s_axi_arburst(S_AXI_2_ARBURST),
        .s_axi_arcache(S_AXI_2_ARCACHE),
        .s_axi_arlen(S_AXI_2_ARLEN),
        .s_axi_arlock({S_AXI_2_ARLOCK,S_AXI_2_ARLOCK}),
        .s_axi_arprot(S_AXI_2_ARPROT),
        .s_axi_arqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arready(S_AXI_2_ARREADY),
        .s_axi_arsize(S_AXI_2_ARSIZE),
        .s_axi_aruser(S_AXI_2_ARUSER),
        .s_axi_arvalid(S_AXI_2_ARVALID),
        .s_axi_awaddr(S_AXI_2_AWADDR),
        .s_axi_awburst(S_AXI_2_AWBURST),
        .s_axi_awcache(S_AXI_2_AWCACHE),
        .s_axi_awlen(S_AXI_2_AWLEN),
        .s_axi_awlock({S_AXI_2_AWLOCK,S_AXI_2_AWLOCK}),
        .s_axi_awprot(S_AXI_2_AWPROT),
        .s_axi_awqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_awready(S_AXI_2_AWREADY),
        .s_axi_awsize(S_AXI_2_AWSIZE),
        .s_axi_awuser(S_AXI_2_AWUSER),
        .s_axi_awvalid(S_AXI_2_AWVALID),
        .s_axi_bready(S_AXI_2_BREADY),
        .s_axi_bresp(S_AXI_2_BRESP),
        .s_axi_bvalid(S_AXI_2_BVALID),
        .s_axi_rdata(S_AXI_2_RDATA),
        .s_axi_rlast(S_AXI_2_RLAST),
        .s_axi_rready(S_AXI_2_RREADY),
        .s_axi_rresp(S_AXI_2_RRESP),
        .s_axi_rvalid(S_AXI_2_RVALID),
        .s_axi_wdata(S_AXI_2_WDATA),
        .s_axi_wlast(S_AXI_2_WLAST),
        .s_axi_wready(S_AXI_2_WREADY),
        .s_axi_wstrb(S_AXI_2_WSTRB),
        .s_axi_wvalid(S_AXI_2_WVALID));
  clock_generation_imp_1SL5TTO clock_generation
       (.AXI_clk(clock_generation_AXI_clk),
        .BCLK_clk(clock_generation_BCLK_clk),
        .CLK90_pin(CLK90_clk),
        .CPUCLK_clk(CPUCLK_clk),
        .PCLK_clk(clock_generation_PCLK_clk),
        .PCLK_reg(clock_generation_PCLK_reg),
        .clk90_detected(clock_generation_clk90_detected),
        .clk_in1(processing_system7_0_FCLK_CLK0),
        .clk_out1(vid_clk_1),
        .clk_out2(aud_mclk_1),
        .cpuclk_detected(clock_generation_cpuclk_detected),
        .enable_clk_output(enable_clk_output_1),
        .nCLKEN_clk(clock_generation_nCLKEN_clk),
        .nCLKEN_reg(clock_generation_nCLKEN_reg),
        .s_axi_aclk(processing_system7_0_FCLK_CLK1),
        .s_axi_aresetn(interconnect_matrix_peripheral_aresetn1),
        .s_axi_lite1_araddr(s_axi_lite1_2_ARADDR),
        .s_axi_lite1_arready(s_axi_lite1_2_ARREADY),
        .s_axi_lite1_arvalid(s_axi_lite1_2_ARVALID),
        .s_axi_lite1_awaddr(s_axi_lite1_2_AWADDR),
        .s_axi_lite1_awready(s_axi_lite1_2_AWREADY),
        .s_axi_lite1_awvalid(s_axi_lite1_2_AWVALID),
        .s_axi_lite1_bready(s_axi_lite1_2_BREADY),
        .s_axi_lite1_bresp(s_axi_lite1_2_BRESP),
        .s_axi_lite1_bvalid(s_axi_lite1_2_BVALID),
        .s_axi_lite1_rdata(s_axi_lite1_2_RDATA),
        .s_axi_lite1_rready(s_axi_lite1_2_RREADY),
        .s_axi_lite1_rresp(s_axi_lite1_2_RRESP),
        .s_axi_lite1_rvalid(s_axi_lite1_2_RVALID),
        .s_axi_lite1_wdata(s_axi_lite1_2_WDATA),
        .s_axi_lite1_wready(s_axi_lite1_2_WREADY),
        .s_axi_lite1_wstrb(s_axi_lite1_2_WSTRB),
        .s_axi_lite1_wvalid(s_axi_lite1_2_WVALID),
        .s_axi_lite_araddr(s_axi_lite_2_ARADDR),
        .s_axi_lite_arready(s_axi_lite_2_ARREADY),
        .s_axi_lite_arvalid(s_axi_lite_2_ARVALID),
        .s_axi_lite_awaddr(s_axi_lite_2_AWADDR),
        .s_axi_lite_awready(s_axi_lite_2_AWREADY),
        .s_axi_lite_awvalid(s_axi_lite_2_AWVALID),
        .s_axi_lite_bready(s_axi_lite_2_BREADY),
        .s_axi_lite_bresp(s_axi_lite_2_BRESP),
        .s_axi_lite_bvalid(s_axi_lite_2_BVALID),
        .s_axi_lite_rdata(s_axi_lite_2_RDATA),
        .s_axi_lite_rready(s_axi_lite_2_RREADY),
        .s_axi_lite_rresp(s_axi_lite_2_RRESP),
        .s_axi_lite_rvalid(s_axi_lite_2_RVALID),
        .s_axi_lite_wdata(s_axi_lite_2_WDATA),
        .s_axi_lite_wready(s_axi_lite_2_WREADY),
        .s_axi_lite_wstrb(s_axi_lite_2_WSTRB),
        .s_axi_lite_wvalid(s_axi_lite_2_WVALID));
  interconnect_matrix_imp_1Q897BS interconnect_matrix
       (.M00_AXI_araddr(s_axi_lite1_2_ARADDR),
        .M00_AXI_arready(s_axi_lite1_2_ARREADY),
        .M00_AXI_arvalid(s_axi_lite1_2_ARVALID),
        .M00_AXI_awaddr(s_axi_lite1_2_AWADDR),
        .M00_AXI_awready(s_axi_lite1_2_AWREADY),
        .M00_AXI_awvalid(s_axi_lite1_2_AWVALID),
        .M00_AXI_bready(s_axi_lite1_2_BREADY),
        .M00_AXI_bresp(s_axi_lite1_2_BRESP),
        .M00_AXI_bvalid(s_axi_lite1_2_BVALID),
        .M00_AXI_rdata(s_axi_lite1_2_RDATA),
        .M00_AXI_rready(s_axi_lite1_2_RREADY),
        .M00_AXI_rresp(s_axi_lite1_2_RRESP),
        .M00_AXI_rvalid(s_axi_lite1_2_RVALID),
        .M00_AXI_wdata(s_axi_lite1_2_WDATA),
        .M00_AXI_wready(s_axi_lite1_2_WREADY),
        .M00_AXI_wstrb(s_axi_lite1_2_WSTRB),
        .M00_AXI_wvalid(s_axi_lite1_2_WVALID),
        .M01_AXI_araddr(interconnect_matrix_M01_AXI_ARADDR),
        .M01_AXI_arprot(interconnect_matrix_M01_AXI_ARPROT),
        .M01_AXI_arready(interconnect_matrix_M01_AXI_ARREADY),
        .M01_AXI_arvalid(interconnect_matrix_M01_AXI_ARVALID),
        .M01_AXI_awaddr(interconnect_matrix_M01_AXI_AWADDR),
        .M01_AXI_awprot(interconnect_matrix_M01_AXI_AWPROT),
        .M01_AXI_awready(interconnect_matrix_M01_AXI_AWREADY),
        .M01_AXI_awvalid(interconnect_matrix_M01_AXI_AWVALID),
        .M01_AXI_bready(interconnect_matrix_M01_AXI_BREADY),
        .M01_AXI_bresp(interconnect_matrix_M01_AXI_BRESP),
        .M01_AXI_bvalid(interconnect_matrix_M01_AXI_BVALID),
        .M01_AXI_rdata(interconnect_matrix_M01_AXI_RDATA),
        .M01_AXI_rready(interconnect_matrix_M01_AXI_RREADY),
        .M01_AXI_rresp(interconnect_matrix_M01_AXI_RRESP),
        .M01_AXI_rvalid(interconnect_matrix_M01_AXI_RVALID),
        .M01_AXI_wdata(interconnect_matrix_M01_AXI_WDATA),
        .M01_AXI_wready(interconnect_matrix_M01_AXI_WREADY),
        .M01_AXI_wstrb(interconnect_matrix_M01_AXI_WSTRB),
        .M01_AXI_wvalid(interconnect_matrix_M01_AXI_WVALID),
        .M03_AXI_araddr(S_AXI_1_ARADDR),
        .M03_AXI_arprot(S_AXI_1_ARPROT),
        .M03_AXI_arready(S_AXI_1_ARREADY),
        .M03_AXI_arvalid(S_AXI_1_ARVALID),
        .M03_AXI_awaddr(S_AXI_1_AWADDR),
        .M03_AXI_awprot(S_AXI_1_AWPROT),
        .M03_AXI_awready(S_AXI_1_AWREADY),
        .M03_AXI_awvalid(S_AXI_1_AWVALID),
        .M03_AXI_bready(S_AXI_1_BREADY),
        .M03_AXI_bresp(S_AXI_1_BRESP),
        .M03_AXI_bvalid(S_AXI_1_BVALID),
        .M03_AXI_rdata(S_AXI_1_RDATA),
        .M03_AXI_rready(S_AXI_1_RREADY),
        .M03_AXI_rresp(S_AXI_1_RRESP),
        .M03_AXI_rvalid(S_AXI_1_RVALID),
        .M03_AXI_wdata(S_AXI_1_WDATA),
        .M03_AXI_wready(S_AXI_1_WREADY),
        .M03_AXI_wstrb(S_AXI_1_WSTRB),
        .M03_AXI_wvalid(S_AXI_1_WVALID),
        .M04_AXI_araddr(S_AXI_LITE_1_ARADDR),
        .M04_AXI_arready(S_AXI_LITE_1_ARREADY),
        .M04_AXI_arvalid(S_AXI_LITE_1_ARVALID),
        .M04_AXI_awaddr(S_AXI_LITE_1_AWADDR),
        .M04_AXI_awready(S_AXI_LITE_1_AWREADY),
        .M04_AXI_awvalid(S_AXI_LITE_1_AWVALID),
        .M04_AXI_bready(S_AXI_LITE_1_BREADY),
        .M04_AXI_bresp(S_AXI_LITE_1_BRESP),
        .M04_AXI_bvalid(S_AXI_LITE_1_BVALID),
        .M04_AXI_rdata(S_AXI_LITE_1_RDATA),
        .M04_AXI_rready(S_AXI_LITE_1_RREADY),
        .M04_AXI_rresp(S_AXI_LITE_1_RRESP),
        .M04_AXI_rvalid(S_AXI_LITE_1_RVALID),
        .M04_AXI_wdata(S_AXI_LITE_1_WDATA),
        .M04_AXI_wready(S_AXI_LITE_1_WREADY),
        .M04_AXI_wvalid(S_AXI_LITE_1_WVALID),
        .M05_AXI_araddr(s_axi_lite_2_ARADDR),
        .M05_AXI_arready(s_axi_lite_2_ARREADY),
        .M05_AXI_arvalid(s_axi_lite_2_ARVALID),
        .M05_AXI_awaddr(s_axi_lite_2_AWADDR),
        .M05_AXI_awready(s_axi_lite_2_AWREADY),
        .M05_AXI_awvalid(s_axi_lite_2_AWVALID),
        .M05_AXI_bready(s_axi_lite_2_BREADY),
        .M05_AXI_bresp(s_axi_lite_2_BRESP),
        .M05_AXI_bvalid(s_axi_lite_2_BVALID),
        .M05_AXI_rdata(s_axi_lite_2_RDATA),
        .M05_AXI_rready(s_axi_lite_2_RREADY),
        .M05_AXI_rresp(s_axi_lite_2_RRESP),
        .M05_AXI_rvalid(s_axi_lite_2_RVALID),
        .M05_AXI_wdata(s_axi_lite_2_WDATA),
        .M05_AXI_wready(s_axi_lite_2_WREADY),
        .M05_AXI_wstrb(s_axi_lite_2_WSTRB),
        .M05_AXI_wvalid(s_axi_lite_2_WVALID),
        .M06_AXI_araddr(s_axi_ctrl_1_ARADDR),
        .M06_AXI_arready(s_axi_ctrl_1_ARREADY),
        .M06_AXI_arvalid(s_axi_ctrl_1_ARVALID),
        .M06_AXI_awaddr(s_axi_ctrl_1_AWADDR),
        .M06_AXI_awready(s_axi_ctrl_1_AWREADY),
        .M06_AXI_awvalid(s_axi_ctrl_1_AWVALID),
        .M06_AXI_bready(s_axi_ctrl_1_BREADY),
        .M06_AXI_bresp(s_axi_ctrl_1_BRESP),
        .M06_AXI_bvalid(s_axi_ctrl_1_BVALID),
        .M06_AXI_rdata(s_axi_ctrl_1_RDATA),
        .M06_AXI_rready(s_axi_ctrl_1_RREADY),
        .M06_AXI_rresp(s_axi_ctrl_1_RRESP),
        .M06_AXI_rvalid(s_axi_ctrl_1_RVALID),
        .M06_AXI_wdata(s_axi_ctrl_1_WDATA),
        .M06_AXI_wready(s_axi_ctrl_1_WREADY),
        .M06_AXI_wvalid(s_axi_ctrl_1_WVALID),
        .M07_AXI_araddr(s_axi_lite1_1_ARADDR),
        .M07_AXI_arready(s_axi_lite1_1_ARREADY),
        .M07_AXI_arvalid(s_axi_lite1_1_ARVALID),
        .M07_AXI_awaddr(s_axi_lite1_1_AWADDR),
        .M07_AXI_awready(s_axi_lite1_1_AWREADY),
        .M07_AXI_awvalid(s_axi_lite1_1_AWVALID),
        .M07_AXI_bready(s_axi_lite1_1_BREADY),
        .M07_AXI_bresp(s_axi_lite1_1_BRESP),
        .M07_AXI_bvalid(s_axi_lite1_1_BVALID),
        .M07_AXI_rdata(s_axi_lite1_1_RDATA),
        .M07_AXI_rready(s_axi_lite1_1_RREADY),
        .M07_AXI_rresp(s_axi_lite1_1_RRESP),
        .M07_AXI_rvalid(s_axi_lite1_1_RVALID),
        .M07_AXI_wdata(s_axi_lite1_1_WDATA),
        .M07_AXI_wready(s_axi_lite1_1_WREADY),
        .M07_AXI_wvalid(s_axi_lite1_1_WVALID),
        .S00_ACLK(processing_system7_0_FCLK_CLK1),
        .S00_AXI_araddr(processing_system7_0_M_AXI_GP0_ARADDR),
        .S00_AXI_arburst(processing_system7_0_M_AXI_GP0_ARBURST),
        .S00_AXI_arcache(processing_system7_0_M_AXI_GP0_ARCACHE),
        .S00_AXI_arid(processing_system7_0_M_AXI_GP0_ARID),
        .S00_AXI_arlen(processing_system7_0_M_AXI_GP0_ARLEN),
        .S00_AXI_arlock(processing_system7_0_M_AXI_GP0_ARLOCK),
        .S00_AXI_arprot(processing_system7_0_M_AXI_GP0_ARPROT),
        .S00_AXI_arqos(processing_system7_0_M_AXI_GP0_ARQOS),
        .S00_AXI_arready(processing_system7_0_M_AXI_GP0_ARREADY),
        .S00_AXI_arsize(processing_system7_0_M_AXI_GP0_ARSIZE),
        .S00_AXI_arvalid(processing_system7_0_M_AXI_GP0_ARVALID),
        .S00_AXI_awaddr(processing_system7_0_M_AXI_GP0_AWADDR),
        .S00_AXI_awburst(processing_system7_0_M_AXI_GP0_AWBURST),
        .S00_AXI_awcache(processing_system7_0_M_AXI_GP0_AWCACHE),
        .S00_AXI_awid(processing_system7_0_M_AXI_GP0_AWID),
        .S00_AXI_awlen(processing_system7_0_M_AXI_GP0_AWLEN),
        .S00_AXI_awlock(processing_system7_0_M_AXI_GP0_AWLOCK),
        .S00_AXI_awprot(processing_system7_0_M_AXI_GP0_AWPROT),
        .S00_AXI_awqos(processing_system7_0_M_AXI_GP0_AWQOS),
        .S00_AXI_awready(processing_system7_0_M_AXI_GP0_AWREADY),
        .S00_AXI_awsize(processing_system7_0_M_AXI_GP0_AWSIZE),
        .S00_AXI_awvalid(processing_system7_0_M_AXI_GP0_AWVALID),
        .S00_AXI_bid(processing_system7_0_M_AXI_GP0_BID),
        .S00_AXI_bready(processing_system7_0_M_AXI_GP0_BREADY),
        .S00_AXI_bresp(processing_system7_0_M_AXI_GP0_BRESP),
        .S00_AXI_bvalid(processing_system7_0_M_AXI_GP0_BVALID),
        .S00_AXI_rdata(processing_system7_0_M_AXI_GP0_RDATA),
        .S00_AXI_rid(processing_system7_0_M_AXI_GP0_RID),
        .S00_AXI_rlast(processing_system7_0_M_AXI_GP0_RLAST),
        .S00_AXI_rready(processing_system7_0_M_AXI_GP0_RREADY),
        .S00_AXI_rresp(processing_system7_0_M_AXI_GP0_RRESP),
        .S00_AXI_rvalid(processing_system7_0_M_AXI_GP0_RVALID),
        .S00_AXI_wdata(processing_system7_0_M_AXI_GP0_WDATA),
        .S00_AXI_wid(processing_system7_0_M_AXI_GP0_WID),
        .S00_AXI_wlast(processing_system7_0_M_AXI_GP0_WLAST),
        .S00_AXI_wready(processing_system7_0_M_AXI_GP0_WREADY),
        .S00_AXI_wstrb(processing_system7_0_M_AXI_GP0_WSTRB),
        .S00_AXI_wvalid(processing_system7_0_M_AXI_GP0_WVALID),
        .ext_reset_in(processing_system7_0_FCLK_RESET0_N),
        .interconnect_aresetn(interconnect_matrix_interconnect_aresetn),
        .peripheral_aresetn(interconnect_matrix_peripheral_aresetn1));
  design_1_proc_sys_reset_0_0 proc_sys_reset_0
       (.aux_reset_in(1'b1),
        .dcm_locked(1'b1),
        .ext_reset_in(processing_system7_0_FCLK_RESET0_N),
        .mb_debug_sys_rst(1'b0),
        .peripheral_aresetn(proc_sys_reset_0_peripheral_aresetn),
        .peripheral_reset(proc_sys_reset_0_peripheral_reset),
        .slowest_sync_clk(clock_generation_AXI_clk));
  design_1_processing_system7_0_0 processing_system7_0
       (.DDR_Addr(DDR_addr[14:0]),
        .DDR_BankAddr(DDR_ba[2:0]),
        .DDR_CAS_n(DDR_cas_n),
        .DDR_CKE(DDR_cke),
        .DDR_CS_n(DDR_cs_n),
        .DDR_Clk(DDR_ck_p),
        .DDR_Clk_n(DDR_ck_n),
        .DDR_DM(DDR_dm[3:0]),
        .DDR_DQ(DDR_dq[31:0]),
        .DDR_DQS(DDR_dqs_p[3:0]),
        .DDR_DQS_n(DDR_dqs_n[3:0]),
        .DDR_DRSTB(DDR_reset_n),
        .DDR_ODT(DDR_odt),
        .DDR_RAS_n(DDR_ras_n),
        .DDR_VRN(FIXED_IO_ddr_vrn),
        .DDR_VRP(FIXED_IO_ddr_vrp),
        .DDR_WEB(DDR_we_n),
        .FCLK_CLK0(processing_system7_0_FCLK_CLK0),
        .FCLK_CLK1(processing_system7_0_FCLK_CLK1),
        .FCLK_CLK2(processing_system7_0_FCLK_CLK2),
        .FCLK_RESET0_N(processing_system7_0_FCLK_RESET0_N),
        .I2C0_SCL_I(Conn2_SCL_I),
        .I2C0_SCL_O(Conn2_SCL_O),
        .I2C0_SCL_T(Conn2_SCL_T),
        .I2C0_SDA_I(Conn2_SDA_I),
        .I2C0_SDA_O(Conn2_SDA_O),
        .I2C0_SDA_T(Conn2_SDA_T),
        .IRQ_F2P(xlconcat_0_dout),
        .IRQ_P2F_USB0(processing_system7_0_IRQ_P2F_USB0),
        .MIO(FIXED_IO_mio[53:0]),
        .M_AXI_GP0_ACLK(processing_system7_0_FCLK_CLK1),
        .M_AXI_GP0_ARADDR(processing_system7_0_M_AXI_GP0_ARADDR),
        .M_AXI_GP0_ARBURST(processing_system7_0_M_AXI_GP0_ARBURST),
        .M_AXI_GP0_ARCACHE(processing_system7_0_M_AXI_GP0_ARCACHE),
        .M_AXI_GP0_ARID(processing_system7_0_M_AXI_GP0_ARID),
        .M_AXI_GP0_ARLEN(processing_system7_0_M_AXI_GP0_ARLEN),
        .M_AXI_GP0_ARLOCK(processing_system7_0_M_AXI_GP0_ARLOCK),
        .M_AXI_GP0_ARPROT(processing_system7_0_M_AXI_GP0_ARPROT),
        .M_AXI_GP0_ARQOS(processing_system7_0_M_AXI_GP0_ARQOS),
        .M_AXI_GP0_ARREADY(processing_system7_0_M_AXI_GP0_ARREADY),
        .M_AXI_GP0_ARSIZE(processing_system7_0_M_AXI_GP0_ARSIZE),
        .M_AXI_GP0_ARVALID(processing_system7_0_M_AXI_GP0_ARVALID),
        .M_AXI_GP0_AWADDR(processing_system7_0_M_AXI_GP0_AWADDR),
        .M_AXI_GP0_AWBURST(processing_system7_0_M_AXI_GP0_AWBURST),
        .M_AXI_GP0_AWCACHE(processing_system7_0_M_AXI_GP0_AWCACHE),
        .M_AXI_GP0_AWID(processing_system7_0_M_AXI_GP0_AWID),
        .M_AXI_GP0_AWLEN(processing_system7_0_M_AXI_GP0_AWLEN),
        .M_AXI_GP0_AWLOCK(processing_system7_0_M_AXI_GP0_AWLOCK),
        .M_AXI_GP0_AWPROT(processing_system7_0_M_AXI_GP0_AWPROT),
        .M_AXI_GP0_AWQOS(processing_system7_0_M_AXI_GP0_AWQOS),
        .M_AXI_GP0_AWREADY(processing_system7_0_M_AXI_GP0_AWREADY),
        .M_AXI_GP0_AWSIZE(processing_system7_0_M_AXI_GP0_AWSIZE),
        .M_AXI_GP0_AWVALID(processing_system7_0_M_AXI_GP0_AWVALID),
        .M_AXI_GP0_BID(processing_system7_0_M_AXI_GP0_BID),
        .M_AXI_GP0_BREADY(processing_system7_0_M_AXI_GP0_BREADY),
        .M_AXI_GP0_BRESP(processing_system7_0_M_AXI_GP0_BRESP),
        .M_AXI_GP0_BVALID(processing_system7_0_M_AXI_GP0_BVALID),
        .M_AXI_GP0_RDATA(processing_system7_0_M_AXI_GP0_RDATA),
        .M_AXI_GP0_RID(processing_system7_0_M_AXI_GP0_RID),
        .M_AXI_GP0_RLAST(processing_system7_0_M_AXI_GP0_RLAST),
        .M_AXI_GP0_RREADY(processing_system7_0_M_AXI_GP0_RREADY),
        .M_AXI_GP0_RRESP(processing_system7_0_M_AXI_GP0_RRESP),
        .M_AXI_GP0_RVALID(processing_system7_0_M_AXI_GP0_RVALID),
        .M_AXI_GP0_WDATA(processing_system7_0_M_AXI_GP0_WDATA),
        .M_AXI_GP0_WID(processing_system7_0_M_AXI_GP0_WID),
        .M_AXI_GP0_WLAST(processing_system7_0_M_AXI_GP0_WLAST),
        .M_AXI_GP0_WREADY(processing_system7_0_M_AXI_GP0_WREADY),
        .M_AXI_GP0_WSTRB(processing_system7_0_M_AXI_GP0_WSTRB),
        .M_AXI_GP0_WVALID(processing_system7_0_M_AXI_GP0_WVALID),
        .M_AXI_GP1_ACLK(clock_generation_AXI_clk),
        .M_AXI_GP1_ARADDR(processing_system7_0_M_AXI_GP1_ARADDR),
        .M_AXI_GP1_ARBURST(processing_system7_0_M_AXI_GP1_ARBURST),
        .M_AXI_GP1_ARCACHE(processing_system7_0_M_AXI_GP1_ARCACHE),
        .M_AXI_GP1_ARID(processing_system7_0_M_AXI_GP1_ARID),
        .M_AXI_GP1_ARLEN(processing_system7_0_M_AXI_GP1_ARLEN),
        .M_AXI_GP1_ARLOCK(processing_system7_0_M_AXI_GP1_ARLOCK),
        .M_AXI_GP1_ARPROT(processing_system7_0_M_AXI_GP1_ARPROT),
        .M_AXI_GP1_ARQOS(processing_system7_0_M_AXI_GP1_ARQOS),
        .M_AXI_GP1_ARREADY(processing_system7_0_M_AXI_GP1_ARREADY),
        .M_AXI_GP1_ARSIZE(processing_system7_0_M_AXI_GP1_ARSIZE),
        .M_AXI_GP1_ARVALID(processing_system7_0_M_AXI_GP1_ARVALID),
        .M_AXI_GP1_AWADDR(processing_system7_0_M_AXI_GP1_AWADDR),
        .M_AXI_GP1_AWBURST(processing_system7_0_M_AXI_GP1_AWBURST),
        .M_AXI_GP1_AWCACHE(processing_system7_0_M_AXI_GP1_AWCACHE),
        .M_AXI_GP1_AWID(processing_system7_0_M_AXI_GP1_AWID),
        .M_AXI_GP1_AWLEN(processing_system7_0_M_AXI_GP1_AWLEN),
        .M_AXI_GP1_AWLOCK(processing_system7_0_M_AXI_GP1_AWLOCK),
        .M_AXI_GP1_AWPROT(processing_system7_0_M_AXI_GP1_AWPROT),
        .M_AXI_GP1_AWQOS(processing_system7_0_M_AXI_GP1_AWQOS),
        .M_AXI_GP1_AWREADY(processing_system7_0_M_AXI_GP1_AWREADY),
        .M_AXI_GP1_AWSIZE(processing_system7_0_M_AXI_GP1_AWSIZE),
        .M_AXI_GP1_AWVALID(processing_system7_0_M_AXI_GP1_AWVALID),
        .M_AXI_GP1_BID(processing_system7_0_M_AXI_GP1_BID),
        .M_AXI_GP1_BREADY(processing_system7_0_M_AXI_GP1_BREADY),
        .M_AXI_GP1_BRESP(processing_system7_0_M_AXI_GP1_BRESP),
        .M_AXI_GP1_BVALID(processing_system7_0_M_AXI_GP1_BVALID),
        .M_AXI_GP1_RDATA(processing_system7_0_M_AXI_GP1_RDATA),
        .M_AXI_GP1_RID(processing_system7_0_M_AXI_GP1_RID),
        .M_AXI_GP1_RLAST(processing_system7_0_M_AXI_GP1_RLAST),
        .M_AXI_GP1_RREADY(processing_system7_0_M_AXI_GP1_RREADY),
        .M_AXI_GP1_RRESP(processing_system7_0_M_AXI_GP1_RRESP),
        .M_AXI_GP1_RVALID(processing_system7_0_M_AXI_GP1_RVALID),
        .M_AXI_GP1_WDATA(processing_system7_0_M_AXI_GP1_WDATA),
        .M_AXI_GP1_WID(processing_system7_0_M_AXI_GP1_WID),
        .M_AXI_GP1_WLAST(processing_system7_0_M_AXI_GP1_WLAST),
        .M_AXI_GP1_WREADY(processing_system7_0_M_AXI_GP1_WREADY),
        .M_AXI_GP1_WSTRB(processing_system7_0_M_AXI_GP1_WSTRB),
        .M_AXI_GP1_WVALID(processing_system7_0_M_AXI_GP1_WVALID),
        .PS_CLK(FIXED_IO_ps_clk),
        .PS_PORB(FIXED_IO_ps_porb),
        .PS_SRSTB(FIXED_IO_ps_srstb),
        .S_AXI_ACP_ACLK(clock_generation_AXI_clk),
        .S_AXI_ACP_ARADDR(axi_register_slice_0_M_AXI_ARADDR),
        .S_AXI_ACP_ARBURST(axi_register_slice_0_M_AXI_ARBURST),
        .S_AXI_ACP_ARCACHE(axi_register_slice_0_M_AXI_ARCACHE),
        .S_AXI_ACP_ARID({1'b0,1'b0,1'b0}),
        .S_AXI_ACP_ARLEN(axi_register_slice_0_M_AXI_ARLEN),
        .S_AXI_ACP_ARLOCK(axi_register_slice_0_M_AXI_ARLOCK),
        .S_AXI_ACP_ARPROT(axi_register_slice_0_M_AXI_ARPROT),
        .S_AXI_ACP_ARQOS(axi_register_slice_0_M_AXI_ARQOS),
        .S_AXI_ACP_ARREADY(axi_register_slice_0_M_AXI_ARREADY),
        .S_AXI_ACP_ARSIZE(axi_register_slice_0_M_AXI_ARSIZE),
        .S_AXI_ACP_ARUSER({1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_ACP_ARVALID(axi_register_slice_0_M_AXI_ARVALID),
        .S_AXI_ACP_AWADDR(axi_register_slice_0_M_AXI_AWADDR),
        .S_AXI_ACP_AWBURST(axi_register_slice_0_M_AXI_AWBURST),
        .S_AXI_ACP_AWCACHE(axi_register_slice_0_M_AXI_AWCACHE),
        .S_AXI_ACP_AWID({1'b0,1'b0,1'b0}),
        .S_AXI_ACP_AWLEN(axi_register_slice_0_M_AXI_AWLEN),
        .S_AXI_ACP_AWLOCK(axi_register_slice_0_M_AXI_AWLOCK),
        .S_AXI_ACP_AWPROT(axi_register_slice_0_M_AXI_AWPROT),
        .S_AXI_ACP_AWQOS(axi_register_slice_0_M_AXI_AWQOS),
        .S_AXI_ACP_AWREADY(axi_register_slice_0_M_AXI_AWREADY),
        .S_AXI_ACP_AWSIZE(axi_register_slice_0_M_AXI_AWSIZE),
        .S_AXI_ACP_AWUSER({1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_ACP_AWVALID(axi_register_slice_0_M_AXI_AWVALID),
        .S_AXI_ACP_BREADY(axi_register_slice_0_M_AXI_BREADY),
        .S_AXI_ACP_BRESP(axi_register_slice_0_M_AXI_BRESP),
        .S_AXI_ACP_BVALID(axi_register_slice_0_M_AXI_BVALID),
        .S_AXI_ACP_RDATA(axi_register_slice_0_M_AXI_RDATA),
        .S_AXI_ACP_RLAST(axi_register_slice_0_M_AXI_RLAST),
        .S_AXI_ACP_RREADY(axi_register_slice_0_M_AXI_RREADY),
        .S_AXI_ACP_RRESP(axi_register_slice_0_M_AXI_RRESP),
        .S_AXI_ACP_RVALID(axi_register_slice_0_M_AXI_RVALID),
        .S_AXI_ACP_WDATA(axi_register_slice_0_M_AXI_WDATA),
        .S_AXI_ACP_WID({1'b0,1'b0,1'b0}),
        .S_AXI_ACP_WLAST(axi_register_slice_0_M_AXI_WLAST),
        .S_AXI_ACP_WREADY(axi_register_slice_0_M_AXI_WREADY),
        .S_AXI_ACP_WSTRB(axi_register_slice_0_M_AXI_WSTRB),
        .S_AXI_ACP_WVALID(axi_register_slice_0_M_AXI_WVALID),
        .S_AXI_HP0_ACLK(processing_system7_0_FCLK_CLK2),
        .S_AXI_HP0_ARADDR(audio_video_engine_M_AXI_ARADDR),
        .S_AXI_HP0_ARBURST(audio_video_engine_M_AXI_ARBURST),
        .S_AXI_HP0_ARCACHE(audio_video_engine_M_AXI_ARCACHE),
        .S_AXI_HP0_ARID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_ARLEN(audio_video_engine_M_AXI_ARLEN),
        .S_AXI_HP0_ARLOCK(audio_video_engine_M_AXI_ARLOCK),
        .S_AXI_HP0_ARPROT(audio_video_engine_M_AXI_ARPROT),
        .S_AXI_HP0_ARQOS(audio_video_engine_M_AXI_ARQOS),
        .S_AXI_HP0_ARREADY(audio_video_engine_M_AXI_ARREADY),
        .S_AXI_HP0_ARSIZE(audio_video_engine_M_AXI_ARSIZE),
        .S_AXI_HP0_ARVALID(audio_video_engine_M_AXI_ARVALID),
        .S_AXI_HP0_AWADDR({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_AWBURST({1'b0,1'b1}),
        .S_AXI_HP0_AWCACHE({1'b0,1'b0,1'b1,1'b1}),
        .S_AXI_HP0_AWID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_AWLEN({1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_AWLOCK({1'b0,1'b0}),
        .S_AXI_HP0_AWPROT({1'b0,1'b0,1'b0}),
        .S_AXI_HP0_AWQOS({1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_AWSIZE({1'b0,1'b1,1'b0}),
        .S_AXI_HP0_AWVALID(1'b0),
        .S_AXI_HP0_BREADY(1'b0),
        .S_AXI_HP0_RDATA(audio_video_engine_M_AXI_RDATA),
        .S_AXI_HP0_RDISSUECAP1_EN(1'b0),
        .S_AXI_HP0_RLAST(audio_video_engine_M_AXI_RLAST),
        .S_AXI_HP0_RREADY(audio_video_engine_M_AXI_RREADY),
        .S_AXI_HP0_RRESP(audio_video_engine_M_AXI_RRESP),
        .S_AXI_HP0_RVALID(audio_video_engine_M_AXI_RVALID),
        .S_AXI_HP0_WDATA({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_WID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP0_WLAST(1'b0),
        .S_AXI_HP0_WRISSUECAP1_EN(1'b0),
        .S_AXI_HP0_WSTRB({1'b1,1'b1,1'b1,1'b1}),
        .S_AXI_HP0_WVALID(1'b0),
        .S_AXI_HP1_ACLK(processing_system7_0_FCLK_CLK0),
        .S_AXI_HP1_ARADDR(audio_video_engine_M00_AXI_ARADDR),
        .S_AXI_HP1_ARBURST(audio_video_engine_M00_AXI_ARBURST),
        .S_AXI_HP1_ARCACHE(audio_video_engine_M00_AXI_ARCACHE),
        .S_AXI_HP1_ARID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_ARLEN(audio_video_engine_M00_AXI_ARLEN),
        .S_AXI_HP1_ARLOCK(audio_video_engine_M00_AXI_ARLOCK),
        .S_AXI_HP1_ARPROT(audio_video_engine_M00_AXI_ARPROT),
        .S_AXI_HP1_ARQOS(audio_video_engine_M00_AXI_ARQOS),
        .S_AXI_HP1_ARREADY(audio_video_engine_M00_AXI_ARREADY),
        .S_AXI_HP1_ARSIZE(audio_video_engine_M00_AXI_ARSIZE),
        .S_AXI_HP1_ARVALID(audio_video_engine_M00_AXI_ARVALID),
        .S_AXI_HP1_AWADDR({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_AWBURST({1'b0,1'b1}),
        .S_AXI_HP1_AWCACHE({1'b0,1'b0,1'b1,1'b1}),
        .S_AXI_HP1_AWID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_AWLEN({1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_AWLOCK({1'b0,1'b0}),
        .S_AXI_HP1_AWPROT({1'b0,1'b0,1'b0}),
        .S_AXI_HP1_AWQOS({1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_AWSIZE({1'b0,1'b1,1'b0}),
        .S_AXI_HP1_AWVALID(1'b0),
        .S_AXI_HP1_BREADY(1'b0),
        .S_AXI_HP1_RDATA(audio_video_engine_M00_AXI_RDATA),
        .S_AXI_HP1_RDISSUECAP1_EN(1'b0),
        .S_AXI_HP1_RLAST(audio_video_engine_M00_AXI_RLAST),
        .S_AXI_HP1_RREADY(audio_video_engine_M00_AXI_RREADY),
        .S_AXI_HP1_RRESP(audio_video_engine_M00_AXI_RRESP),
        .S_AXI_HP1_RVALID(audio_video_engine_M00_AXI_RVALID),
        .S_AXI_HP1_WDATA({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_WID({1'b0,1'b0,1'b0,1'b0,1'b0,1'b0}),
        .S_AXI_HP1_WLAST(1'b0),
        .S_AXI_HP1_WRISSUECAP1_EN(1'b0),
        .S_AXI_HP1_WSTRB({1'b1,1'b1,1'b1,1'b1}),
        .S_AXI_HP1_WVALID(1'b0),
        .USB0_VBUS_PWRFAULT(1'b0));
  design_1_xlconcat_0_2 xlconcat_0
       (.In0(audio_video_engine_dout),
        .In1(processing_system7_0_IRQ_P2F_USB0),
        .dout(xlconcat_0_dout));
endmodule

module s00_couplers_imp_17YN31X
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arburst,
    M_AXI_arcache,
    M_AXI_arlen,
    M_AXI_arlock,
    M_AXI_arprot,
    M_AXI_arqos,
    M_AXI_arready,
    M_AXI_arsize,
    M_AXI_arvalid,
    M_AXI_rdata,
    M_AXI_rlast,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arburst,
    S_AXI_arcache,
    S_AXI_arlen,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arsize,
    S_AXI_arvalid,
    S_AXI_rdata,
    S_AXI_rlast,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  output [1:0]M_AXI_arburst;
  output [3:0]M_AXI_arcache;
  output [3:0]M_AXI_arlen;
  output [1:0]M_AXI_arlock;
  output [2:0]M_AXI_arprot;
  output [3:0]M_AXI_arqos;
  input M_AXI_arready;
  output [2:0]M_AXI_arsize;
  output M_AXI_arvalid;
  input [31:0]M_AXI_rdata;
  input M_AXI_rlast;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  input [1:0]S_AXI_arburst;
  input [3:0]S_AXI_arcache;
  input [7:0]S_AXI_arlen;
  input [2:0]S_AXI_arprot;
  output S_AXI_arready;
  input [2:0]S_AXI_arsize;
  input S_AXI_arvalid;
  output [31:0]S_AXI_rdata;
  output S_AXI_rlast;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;

  wire S_ACLK_1;
  wire S_ARESETN_1;
  wire [31:0]auto_pc_to_s00_couplers_ARADDR;
  wire [1:0]auto_pc_to_s00_couplers_ARBURST;
  wire [3:0]auto_pc_to_s00_couplers_ARCACHE;
  wire [3:0]auto_pc_to_s00_couplers_ARLEN;
  wire [1:0]auto_pc_to_s00_couplers_ARLOCK;
  wire [2:0]auto_pc_to_s00_couplers_ARPROT;
  wire [3:0]auto_pc_to_s00_couplers_ARQOS;
  wire auto_pc_to_s00_couplers_ARREADY;
  wire [2:0]auto_pc_to_s00_couplers_ARSIZE;
  wire auto_pc_to_s00_couplers_ARVALID;
  wire [31:0]auto_pc_to_s00_couplers_RDATA;
  wire auto_pc_to_s00_couplers_RLAST;
  wire auto_pc_to_s00_couplers_RREADY;
  wire [1:0]auto_pc_to_s00_couplers_RRESP;
  wire auto_pc_to_s00_couplers_RVALID;
  wire [31:0]s00_couplers_to_s00_regslice_ARADDR;
  wire [1:0]s00_couplers_to_s00_regslice_ARBURST;
  wire [3:0]s00_couplers_to_s00_regslice_ARCACHE;
  wire [7:0]s00_couplers_to_s00_regslice_ARLEN;
  wire [2:0]s00_couplers_to_s00_regslice_ARPROT;
  wire s00_couplers_to_s00_regslice_ARREADY;
  wire [2:0]s00_couplers_to_s00_regslice_ARSIZE;
  wire s00_couplers_to_s00_regslice_ARVALID;
  wire [31:0]s00_couplers_to_s00_regslice_RDATA;
  wire s00_couplers_to_s00_regslice_RLAST;
  wire s00_couplers_to_s00_regslice_RREADY;
  wire [1:0]s00_couplers_to_s00_regslice_RRESP;
  wire s00_couplers_to_s00_regslice_RVALID;
  wire [31:0]s00_regslice_to_auto_pc_ARADDR;
  wire [1:0]s00_regslice_to_auto_pc_ARBURST;
  wire [3:0]s00_regslice_to_auto_pc_ARCACHE;
  wire [7:0]s00_regslice_to_auto_pc_ARLEN;
  wire [0:0]s00_regslice_to_auto_pc_ARLOCK;
  wire [2:0]s00_regslice_to_auto_pc_ARPROT;
  wire [3:0]s00_regslice_to_auto_pc_ARQOS;
  wire s00_regslice_to_auto_pc_ARREADY;
  wire [3:0]s00_regslice_to_auto_pc_ARREGION;
  wire [2:0]s00_regslice_to_auto_pc_ARSIZE;
  wire s00_regslice_to_auto_pc_ARVALID;
  wire [31:0]s00_regslice_to_auto_pc_RDATA;
  wire s00_regslice_to_auto_pc_RLAST;
  wire s00_regslice_to_auto_pc_RREADY;
  wire [1:0]s00_regslice_to_auto_pc_RRESP;
  wire s00_regslice_to_auto_pc_RVALID;

  assign M_AXI_araddr[31:0] = auto_pc_to_s00_couplers_ARADDR;
  assign M_AXI_arburst[1:0] = auto_pc_to_s00_couplers_ARBURST;
  assign M_AXI_arcache[3:0] = auto_pc_to_s00_couplers_ARCACHE;
  assign M_AXI_arlen[3:0] = auto_pc_to_s00_couplers_ARLEN;
  assign M_AXI_arlock[1:0] = auto_pc_to_s00_couplers_ARLOCK;
  assign M_AXI_arprot[2:0] = auto_pc_to_s00_couplers_ARPROT;
  assign M_AXI_arqos[3:0] = auto_pc_to_s00_couplers_ARQOS;
  assign M_AXI_arsize[2:0] = auto_pc_to_s00_couplers_ARSIZE;
  assign M_AXI_arvalid = auto_pc_to_s00_couplers_ARVALID;
  assign M_AXI_rready = auto_pc_to_s00_couplers_RREADY;
  assign S_ACLK_1 = S_ACLK;
  assign S_ARESETN_1 = S_ARESETN;
  assign S_AXI_arready = s00_couplers_to_s00_regslice_ARREADY;
  assign S_AXI_rdata[31:0] = s00_couplers_to_s00_regslice_RDATA;
  assign S_AXI_rlast = s00_couplers_to_s00_regslice_RLAST;
  assign S_AXI_rresp[1:0] = s00_couplers_to_s00_regslice_RRESP;
  assign S_AXI_rvalid = s00_couplers_to_s00_regslice_RVALID;
  assign auto_pc_to_s00_couplers_ARREADY = M_AXI_arready;
  assign auto_pc_to_s00_couplers_RDATA = M_AXI_rdata[31:0];
  assign auto_pc_to_s00_couplers_RLAST = M_AXI_rlast;
  assign auto_pc_to_s00_couplers_RRESP = M_AXI_rresp[1:0];
  assign auto_pc_to_s00_couplers_RVALID = M_AXI_rvalid;
  assign s00_couplers_to_s00_regslice_ARADDR = S_AXI_araddr[31:0];
  assign s00_couplers_to_s00_regslice_ARBURST = S_AXI_arburst[1:0];
  assign s00_couplers_to_s00_regslice_ARCACHE = S_AXI_arcache[3:0];
  assign s00_couplers_to_s00_regslice_ARLEN = S_AXI_arlen[7:0];
  assign s00_couplers_to_s00_regslice_ARPROT = S_AXI_arprot[2:0];
  assign s00_couplers_to_s00_regslice_ARSIZE = S_AXI_arsize[2:0];
  assign s00_couplers_to_s00_regslice_ARVALID = S_AXI_arvalid;
  assign s00_couplers_to_s00_regslice_RREADY = S_AXI_rready;
  design_1_auto_pc_0 auto_pc
       (.aclk(S_ACLK_1),
        .aresetn(S_ARESETN_1),
        .m_axi_araddr(auto_pc_to_s00_couplers_ARADDR),
        .m_axi_arburst(auto_pc_to_s00_couplers_ARBURST),
        .m_axi_arcache(auto_pc_to_s00_couplers_ARCACHE),
        .m_axi_arlen(auto_pc_to_s00_couplers_ARLEN),
        .m_axi_arlock(auto_pc_to_s00_couplers_ARLOCK),
        .m_axi_arprot(auto_pc_to_s00_couplers_ARPROT),
        .m_axi_arqos(auto_pc_to_s00_couplers_ARQOS),
        .m_axi_arready(auto_pc_to_s00_couplers_ARREADY),
        .m_axi_arsize(auto_pc_to_s00_couplers_ARSIZE),
        .m_axi_arvalid(auto_pc_to_s00_couplers_ARVALID),
        .m_axi_rdata(auto_pc_to_s00_couplers_RDATA),
        .m_axi_rlast(auto_pc_to_s00_couplers_RLAST),
        .m_axi_rready(auto_pc_to_s00_couplers_RREADY),
        .m_axi_rresp(auto_pc_to_s00_couplers_RRESP),
        .m_axi_rvalid(auto_pc_to_s00_couplers_RVALID),
        .s_axi_araddr(s00_regslice_to_auto_pc_ARADDR),
        .s_axi_arburst(s00_regslice_to_auto_pc_ARBURST),
        .s_axi_arcache(s00_regslice_to_auto_pc_ARCACHE),
        .s_axi_arlen(s00_regslice_to_auto_pc_ARLEN),
        .s_axi_arlock(s00_regslice_to_auto_pc_ARLOCK),
        .s_axi_arprot(s00_regslice_to_auto_pc_ARPROT),
        .s_axi_arqos(s00_regslice_to_auto_pc_ARQOS),
        .s_axi_arready(s00_regslice_to_auto_pc_ARREADY),
        .s_axi_arregion(s00_regslice_to_auto_pc_ARREGION),
        .s_axi_arsize(s00_regslice_to_auto_pc_ARSIZE),
        .s_axi_arvalid(s00_regslice_to_auto_pc_ARVALID),
        .s_axi_rdata(s00_regslice_to_auto_pc_RDATA),
        .s_axi_rlast(s00_regslice_to_auto_pc_RLAST),
        .s_axi_rready(s00_regslice_to_auto_pc_RREADY),
        .s_axi_rresp(s00_regslice_to_auto_pc_RRESP),
        .s_axi_rvalid(s00_regslice_to_auto_pc_RVALID));
  design_1_s00_regslice_128 s00_regslice
       (.aclk(S_ACLK_1),
        .aresetn(S_ARESETN_1),
        .m_axi_araddr(s00_regslice_to_auto_pc_ARADDR),
        .m_axi_arburst(s00_regslice_to_auto_pc_ARBURST),
        .m_axi_arcache(s00_regslice_to_auto_pc_ARCACHE),
        .m_axi_arlen(s00_regslice_to_auto_pc_ARLEN),
        .m_axi_arlock(s00_regslice_to_auto_pc_ARLOCK),
        .m_axi_arprot(s00_regslice_to_auto_pc_ARPROT),
        .m_axi_arqos(s00_regslice_to_auto_pc_ARQOS),
        .m_axi_arready(s00_regslice_to_auto_pc_ARREADY),
        .m_axi_arregion(s00_regslice_to_auto_pc_ARREGION),
        .m_axi_arsize(s00_regslice_to_auto_pc_ARSIZE),
        .m_axi_arvalid(s00_regslice_to_auto_pc_ARVALID),
        .m_axi_rdata(s00_regslice_to_auto_pc_RDATA),
        .m_axi_rlast(s00_regslice_to_auto_pc_RLAST),
        .m_axi_rready(s00_regslice_to_auto_pc_RREADY),
        .m_axi_rresp(s00_regslice_to_auto_pc_RRESP),
        .m_axi_rvalid(s00_regslice_to_auto_pc_RVALID),
        .s_axi_araddr(s00_couplers_to_s00_regslice_ARADDR),
        .s_axi_arburst(s00_couplers_to_s00_regslice_ARBURST),
        .s_axi_arcache(s00_couplers_to_s00_regslice_ARCACHE),
        .s_axi_arlen(s00_couplers_to_s00_regslice_ARLEN),
        .s_axi_arlock(1'b0),
        .s_axi_arprot(s00_couplers_to_s00_regslice_ARPROT),
        .s_axi_arqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arready(s00_couplers_to_s00_regslice_ARREADY),
        .s_axi_arregion({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arsize(s00_couplers_to_s00_regslice_ARSIZE),
        .s_axi_arvalid(s00_couplers_to_s00_regslice_ARVALID),
        .s_axi_rdata(s00_couplers_to_s00_regslice_RDATA),
        .s_axi_rlast(s00_couplers_to_s00_regslice_RLAST),
        .s_axi_rready(s00_couplers_to_s00_regslice_RREADY),
        .s_axi_rresp(s00_couplers_to_s00_regslice_RRESP),
        .s_axi_rvalid(s00_couplers_to_s00_regslice_RVALID));
endmodule

module s00_couplers_imp_271I2I
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arprot,
    M_AXI_arready,
    M_AXI_arvalid,
    M_AXI_awaddr,
    M_AXI_awprot,
    M_AXI_awready,
    M_AXI_awvalid,
    M_AXI_bready,
    M_AXI_bresp,
    M_AXI_bvalid,
    M_AXI_rdata,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    M_AXI_wdata,
    M_AXI_wready,
    M_AXI_wstrb,
    M_AXI_wvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arburst,
    S_AXI_arcache,
    S_AXI_arid,
    S_AXI_arlen,
    S_AXI_arlock,
    S_AXI_arprot,
    S_AXI_arqos,
    S_AXI_arready,
    S_AXI_arsize,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awburst,
    S_AXI_awcache,
    S_AXI_awid,
    S_AXI_awlen,
    S_AXI_awlock,
    S_AXI_awprot,
    S_AXI_awqos,
    S_AXI_awready,
    S_AXI_awsize,
    S_AXI_awvalid,
    S_AXI_bid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rid,
    S_AXI_rlast,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wid,
    S_AXI_wlast,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  output [2:0]M_AXI_arprot;
  input M_AXI_arready;
  output M_AXI_arvalid;
  output [31:0]M_AXI_awaddr;
  output [2:0]M_AXI_awprot;
  input M_AXI_awready;
  output M_AXI_awvalid;
  output M_AXI_bready;
  input [1:0]M_AXI_bresp;
  input M_AXI_bvalid;
  input [31:0]M_AXI_rdata;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  output [31:0]M_AXI_wdata;
  input M_AXI_wready;
  output [3:0]M_AXI_wstrb;
  output M_AXI_wvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  input [1:0]S_AXI_arburst;
  input [3:0]S_AXI_arcache;
  input [11:0]S_AXI_arid;
  input [3:0]S_AXI_arlen;
  input [1:0]S_AXI_arlock;
  input [2:0]S_AXI_arprot;
  input [3:0]S_AXI_arqos;
  output S_AXI_arready;
  input [2:0]S_AXI_arsize;
  input S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [1:0]S_AXI_awburst;
  input [3:0]S_AXI_awcache;
  input [11:0]S_AXI_awid;
  input [3:0]S_AXI_awlen;
  input [1:0]S_AXI_awlock;
  input [2:0]S_AXI_awprot;
  input [3:0]S_AXI_awqos;
  output S_AXI_awready;
  input [2:0]S_AXI_awsize;
  input S_AXI_awvalid;
  output [11:0]S_AXI_bid;
  input S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  output [11:0]S_AXI_rid;
  output S_AXI_rlast;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  input [11:0]S_AXI_wid;
  input S_AXI_wlast;
  output S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input S_AXI_wvalid;

  wire S_ACLK_1;
  wire S_ARESETN_1;
  wire [31:0]auto_pc_to_s00_couplers_ARADDR;
  wire [2:0]auto_pc_to_s00_couplers_ARPROT;
  wire auto_pc_to_s00_couplers_ARREADY;
  wire auto_pc_to_s00_couplers_ARVALID;
  wire [31:0]auto_pc_to_s00_couplers_AWADDR;
  wire [2:0]auto_pc_to_s00_couplers_AWPROT;
  wire auto_pc_to_s00_couplers_AWREADY;
  wire auto_pc_to_s00_couplers_AWVALID;
  wire auto_pc_to_s00_couplers_BREADY;
  wire [1:0]auto_pc_to_s00_couplers_BRESP;
  wire auto_pc_to_s00_couplers_BVALID;
  wire [31:0]auto_pc_to_s00_couplers_RDATA;
  wire auto_pc_to_s00_couplers_RREADY;
  wire [1:0]auto_pc_to_s00_couplers_RRESP;
  wire auto_pc_to_s00_couplers_RVALID;
  wire [31:0]auto_pc_to_s00_couplers_WDATA;
  wire auto_pc_to_s00_couplers_WREADY;
  wire [3:0]auto_pc_to_s00_couplers_WSTRB;
  wire auto_pc_to_s00_couplers_WVALID;
  wire [31:0]s00_couplers_to_s00_regslice_ARADDR;
  wire [1:0]s00_couplers_to_s00_regslice_ARBURST;
  wire [3:0]s00_couplers_to_s00_regslice_ARCACHE;
  wire [11:0]s00_couplers_to_s00_regslice_ARID;
  wire [3:0]s00_couplers_to_s00_regslice_ARLEN;
  wire [1:0]s00_couplers_to_s00_regslice_ARLOCK;
  wire [2:0]s00_couplers_to_s00_regslice_ARPROT;
  wire [3:0]s00_couplers_to_s00_regslice_ARQOS;
  wire s00_couplers_to_s00_regslice_ARREADY;
  wire [2:0]s00_couplers_to_s00_regslice_ARSIZE;
  wire s00_couplers_to_s00_regslice_ARVALID;
  wire [31:0]s00_couplers_to_s00_regslice_AWADDR;
  wire [1:0]s00_couplers_to_s00_regslice_AWBURST;
  wire [3:0]s00_couplers_to_s00_regslice_AWCACHE;
  wire [11:0]s00_couplers_to_s00_regslice_AWID;
  wire [3:0]s00_couplers_to_s00_regslice_AWLEN;
  wire [1:0]s00_couplers_to_s00_regslice_AWLOCK;
  wire [2:0]s00_couplers_to_s00_regslice_AWPROT;
  wire [3:0]s00_couplers_to_s00_regslice_AWQOS;
  wire s00_couplers_to_s00_regslice_AWREADY;
  wire [2:0]s00_couplers_to_s00_regslice_AWSIZE;
  wire s00_couplers_to_s00_regslice_AWVALID;
  wire [11:0]s00_couplers_to_s00_regslice_BID;
  wire s00_couplers_to_s00_regslice_BREADY;
  wire [1:0]s00_couplers_to_s00_regslice_BRESP;
  wire s00_couplers_to_s00_regslice_BVALID;
  wire [31:0]s00_couplers_to_s00_regslice_RDATA;
  wire [11:0]s00_couplers_to_s00_regslice_RID;
  wire s00_couplers_to_s00_regslice_RLAST;
  wire s00_couplers_to_s00_regslice_RREADY;
  wire [1:0]s00_couplers_to_s00_regslice_RRESP;
  wire s00_couplers_to_s00_regslice_RVALID;
  wire [31:0]s00_couplers_to_s00_regslice_WDATA;
  wire [11:0]s00_couplers_to_s00_regslice_WID;
  wire s00_couplers_to_s00_regslice_WLAST;
  wire s00_couplers_to_s00_regslice_WREADY;
  wire [3:0]s00_couplers_to_s00_regslice_WSTRB;
  wire s00_couplers_to_s00_regslice_WVALID;
  wire [31:0]s00_regslice_to_auto_pc_ARADDR;
  wire [1:0]s00_regslice_to_auto_pc_ARBURST;
  wire [3:0]s00_regslice_to_auto_pc_ARCACHE;
  wire [11:0]s00_regslice_to_auto_pc_ARID;
  wire [3:0]s00_regslice_to_auto_pc_ARLEN;
  wire [1:0]s00_regslice_to_auto_pc_ARLOCK;
  wire [2:0]s00_regslice_to_auto_pc_ARPROT;
  wire [3:0]s00_regslice_to_auto_pc_ARQOS;
  wire s00_regslice_to_auto_pc_ARREADY;
  wire [2:0]s00_regslice_to_auto_pc_ARSIZE;
  wire s00_regslice_to_auto_pc_ARVALID;
  wire [31:0]s00_regslice_to_auto_pc_AWADDR;
  wire [1:0]s00_regslice_to_auto_pc_AWBURST;
  wire [3:0]s00_regslice_to_auto_pc_AWCACHE;
  wire [11:0]s00_regslice_to_auto_pc_AWID;
  wire [3:0]s00_regslice_to_auto_pc_AWLEN;
  wire [1:0]s00_regslice_to_auto_pc_AWLOCK;
  wire [2:0]s00_regslice_to_auto_pc_AWPROT;
  wire [3:0]s00_regslice_to_auto_pc_AWQOS;
  wire s00_regslice_to_auto_pc_AWREADY;
  wire [2:0]s00_regslice_to_auto_pc_AWSIZE;
  wire s00_regslice_to_auto_pc_AWVALID;
  wire [11:0]s00_regslice_to_auto_pc_BID;
  wire s00_regslice_to_auto_pc_BREADY;
  wire [1:0]s00_regslice_to_auto_pc_BRESP;
  wire s00_regslice_to_auto_pc_BVALID;
  wire [31:0]s00_regslice_to_auto_pc_RDATA;
  wire [11:0]s00_regslice_to_auto_pc_RID;
  wire s00_regslice_to_auto_pc_RLAST;
  wire s00_regslice_to_auto_pc_RREADY;
  wire [1:0]s00_regslice_to_auto_pc_RRESP;
  wire s00_regslice_to_auto_pc_RVALID;
  wire [31:0]s00_regslice_to_auto_pc_WDATA;
  wire [11:0]s00_regslice_to_auto_pc_WID;
  wire s00_regslice_to_auto_pc_WLAST;
  wire s00_regslice_to_auto_pc_WREADY;
  wire [3:0]s00_regslice_to_auto_pc_WSTRB;
  wire s00_regslice_to_auto_pc_WVALID;

  assign M_AXI_araddr[31:0] = auto_pc_to_s00_couplers_ARADDR;
  assign M_AXI_arprot[2:0] = auto_pc_to_s00_couplers_ARPROT;
  assign M_AXI_arvalid = auto_pc_to_s00_couplers_ARVALID;
  assign M_AXI_awaddr[31:0] = auto_pc_to_s00_couplers_AWADDR;
  assign M_AXI_awprot[2:0] = auto_pc_to_s00_couplers_AWPROT;
  assign M_AXI_awvalid = auto_pc_to_s00_couplers_AWVALID;
  assign M_AXI_bready = auto_pc_to_s00_couplers_BREADY;
  assign M_AXI_rready = auto_pc_to_s00_couplers_RREADY;
  assign M_AXI_wdata[31:0] = auto_pc_to_s00_couplers_WDATA;
  assign M_AXI_wstrb[3:0] = auto_pc_to_s00_couplers_WSTRB;
  assign M_AXI_wvalid = auto_pc_to_s00_couplers_WVALID;
  assign S_ACLK_1 = S_ACLK;
  assign S_ARESETN_1 = S_ARESETN;
  assign S_AXI_arready = s00_couplers_to_s00_regslice_ARREADY;
  assign S_AXI_awready = s00_couplers_to_s00_regslice_AWREADY;
  assign S_AXI_bid[11:0] = s00_couplers_to_s00_regslice_BID;
  assign S_AXI_bresp[1:0] = s00_couplers_to_s00_regslice_BRESP;
  assign S_AXI_bvalid = s00_couplers_to_s00_regslice_BVALID;
  assign S_AXI_rdata[31:0] = s00_couplers_to_s00_regslice_RDATA;
  assign S_AXI_rid[11:0] = s00_couplers_to_s00_regslice_RID;
  assign S_AXI_rlast = s00_couplers_to_s00_regslice_RLAST;
  assign S_AXI_rresp[1:0] = s00_couplers_to_s00_regslice_RRESP;
  assign S_AXI_rvalid = s00_couplers_to_s00_regslice_RVALID;
  assign S_AXI_wready = s00_couplers_to_s00_regslice_WREADY;
  assign auto_pc_to_s00_couplers_ARREADY = M_AXI_arready;
  assign auto_pc_to_s00_couplers_AWREADY = M_AXI_awready;
  assign auto_pc_to_s00_couplers_BRESP = M_AXI_bresp[1:0];
  assign auto_pc_to_s00_couplers_BVALID = M_AXI_bvalid;
  assign auto_pc_to_s00_couplers_RDATA = M_AXI_rdata[31:0];
  assign auto_pc_to_s00_couplers_RRESP = M_AXI_rresp[1:0];
  assign auto_pc_to_s00_couplers_RVALID = M_AXI_rvalid;
  assign auto_pc_to_s00_couplers_WREADY = M_AXI_wready;
  assign s00_couplers_to_s00_regslice_ARADDR = S_AXI_araddr[31:0];
  assign s00_couplers_to_s00_regslice_ARBURST = S_AXI_arburst[1:0];
  assign s00_couplers_to_s00_regslice_ARCACHE = S_AXI_arcache[3:0];
  assign s00_couplers_to_s00_regslice_ARID = S_AXI_arid[11:0];
  assign s00_couplers_to_s00_regslice_ARLEN = S_AXI_arlen[3:0];
  assign s00_couplers_to_s00_regslice_ARLOCK = S_AXI_arlock[1:0];
  assign s00_couplers_to_s00_regslice_ARPROT = S_AXI_arprot[2:0];
  assign s00_couplers_to_s00_regslice_ARQOS = S_AXI_arqos[3:0];
  assign s00_couplers_to_s00_regslice_ARSIZE = S_AXI_arsize[2:0];
  assign s00_couplers_to_s00_regslice_ARVALID = S_AXI_arvalid;
  assign s00_couplers_to_s00_regslice_AWADDR = S_AXI_awaddr[31:0];
  assign s00_couplers_to_s00_regslice_AWBURST = S_AXI_awburst[1:0];
  assign s00_couplers_to_s00_regslice_AWCACHE = S_AXI_awcache[3:0];
  assign s00_couplers_to_s00_regslice_AWID = S_AXI_awid[11:0];
  assign s00_couplers_to_s00_regslice_AWLEN = S_AXI_awlen[3:0];
  assign s00_couplers_to_s00_regslice_AWLOCK = S_AXI_awlock[1:0];
  assign s00_couplers_to_s00_regslice_AWPROT = S_AXI_awprot[2:0];
  assign s00_couplers_to_s00_regslice_AWQOS = S_AXI_awqos[3:0];
  assign s00_couplers_to_s00_regslice_AWSIZE = S_AXI_awsize[2:0];
  assign s00_couplers_to_s00_regslice_AWVALID = S_AXI_awvalid;
  assign s00_couplers_to_s00_regslice_BREADY = S_AXI_bready;
  assign s00_couplers_to_s00_regslice_RREADY = S_AXI_rready;
  assign s00_couplers_to_s00_regslice_WDATA = S_AXI_wdata[31:0];
  assign s00_couplers_to_s00_regslice_WID = S_AXI_wid[11:0];
  assign s00_couplers_to_s00_regslice_WLAST = S_AXI_wlast;
  assign s00_couplers_to_s00_regslice_WSTRB = S_AXI_wstrb[3:0];
  assign s00_couplers_to_s00_regslice_WVALID = S_AXI_wvalid;
  design_1_auto_pc_2 auto_pc
       (.aclk(S_ACLK_1),
        .aresetn(S_ARESETN_1),
        .m_axi_araddr(auto_pc_to_s00_couplers_ARADDR),
        .m_axi_arprot(auto_pc_to_s00_couplers_ARPROT),
        .m_axi_arready(auto_pc_to_s00_couplers_ARREADY),
        .m_axi_arvalid(auto_pc_to_s00_couplers_ARVALID),
        .m_axi_awaddr(auto_pc_to_s00_couplers_AWADDR),
        .m_axi_awprot(auto_pc_to_s00_couplers_AWPROT),
        .m_axi_awready(auto_pc_to_s00_couplers_AWREADY),
        .m_axi_awvalid(auto_pc_to_s00_couplers_AWVALID),
        .m_axi_bready(auto_pc_to_s00_couplers_BREADY),
        .m_axi_bresp(auto_pc_to_s00_couplers_BRESP),
        .m_axi_bvalid(auto_pc_to_s00_couplers_BVALID),
        .m_axi_rdata(auto_pc_to_s00_couplers_RDATA),
        .m_axi_rready(auto_pc_to_s00_couplers_RREADY),
        .m_axi_rresp(auto_pc_to_s00_couplers_RRESP),
        .m_axi_rvalid(auto_pc_to_s00_couplers_RVALID),
        .m_axi_wdata(auto_pc_to_s00_couplers_WDATA),
        .m_axi_wready(auto_pc_to_s00_couplers_WREADY),
        .m_axi_wstrb(auto_pc_to_s00_couplers_WSTRB),
        .m_axi_wvalid(auto_pc_to_s00_couplers_WVALID),
        .s_axi_araddr(s00_regslice_to_auto_pc_ARADDR),
        .s_axi_arburst(s00_regslice_to_auto_pc_ARBURST),
        .s_axi_arcache(s00_regslice_to_auto_pc_ARCACHE),
        .s_axi_arid(s00_regslice_to_auto_pc_ARID),
        .s_axi_arlen(s00_regslice_to_auto_pc_ARLEN),
        .s_axi_arlock(s00_regslice_to_auto_pc_ARLOCK),
        .s_axi_arprot(s00_regslice_to_auto_pc_ARPROT),
        .s_axi_arqos(s00_regslice_to_auto_pc_ARQOS),
        .s_axi_arready(s00_regslice_to_auto_pc_ARREADY),
        .s_axi_arsize(s00_regslice_to_auto_pc_ARSIZE),
        .s_axi_arvalid(s00_regslice_to_auto_pc_ARVALID),
        .s_axi_awaddr(s00_regslice_to_auto_pc_AWADDR),
        .s_axi_awburst(s00_regslice_to_auto_pc_AWBURST),
        .s_axi_awcache(s00_regslice_to_auto_pc_AWCACHE),
        .s_axi_awid(s00_regslice_to_auto_pc_AWID),
        .s_axi_awlen(s00_regslice_to_auto_pc_AWLEN),
        .s_axi_awlock(s00_regslice_to_auto_pc_AWLOCK),
        .s_axi_awprot(s00_regslice_to_auto_pc_AWPROT),
        .s_axi_awqos(s00_regslice_to_auto_pc_AWQOS),
        .s_axi_awready(s00_regslice_to_auto_pc_AWREADY),
        .s_axi_awsize(s00_regslice_to_auto_pc_AWSIZE),
        .s_axi_awvalid(s00_regslice_to_auto_pc_AWVALID),
        .s_axi_bid(s00_regslice_to_auto_pc_BID),
        .s_axi_bready(s00_regslice_to_auto_pc_BREADY),
        .s_axi_bresp(s00_regslice_to_auto_pc_BRESP),
        .s_axi_bvalid(s00_regslice_to_auto_pc_BVALID),
        .s_axi_rdata(s00_regslice_to_auto_pc_RDATA),
        .s_axi_rid(s00_regslice_to_auto_pc_RID),
        .s_axi_rlast(s00_regslice_to_auto_pc_RLAST),
        .s_axi_rready(s00_regslice_to_auto_pc_RREADY),
        .s_axi_rresp(s00_regslice_to_auto_pc_RRESP),
        .s_axi_rvalid(s00_regslice_to_auto_pc_RVALID),
        .s_axi_wdata(s00_regslice_to_auto_pc_WDATA),
        .s_axi_wid(s00_regslice_to_auto_pc_WID),
        .s_axi_wlast(s00_regslice_to_auto_pc_WLAST),
        .s_axi_wready(s00_regslice_to_auto_pc_WREADY),
        .s_axi_wstrb(s00_regslice_to_auto_pc_WSTRB),
        .s_axi_wvalid(s00_regslice_to_auto_pc_WVALID));
  design_1_s00_regslice_129 s00_regslice
       (.aclk(S_ACLK_1),
        .aresetn(S_ARESETN_1),
        .m_axi_araddr(s00_regslice_to_auto_pc_ARADDR),
        .m_axi_arburst(s00_regslice_to_auto_pc_ARBURST),
        .m_axi_arcache(s00_regslice_to_auto_pc_ARCACHE),
        .m_axi_arid(s00_regslice_to_auto_pc_ARID),
        .m_axi_arlen(s00_regslice_to_auto_pc_ARLEN),
        .m_axi_arlock(s00_regslice_to_auto_pc_ARLOCK),
        .m_axi_arprot(s00_regslice_to_auto_pc_ARPROT),
        .m_axi_arqos(s00_regslice_to_auto_pc_ARQOS),
        .m_axi_arready(s00_regslice_to_auto_pc_ARREADY),
        .m_axi_arsize(s00_regslice_to_auto_pc_ARSIZE),
        .m_axi_arvalid(s00_regslice_to_auto_pc_ARVALID),
        .m_axi_awaddr(s00_regslice_to_auto_pc_AWADDR),
        .m_axi_awburst(s00_regslice_to_auto_pc_AWBURST),
        .m_axi_awcache(s00_regslice_to_auto_pc_AWCACHE),
        .m_axi_awid(s00_regslice_to_auto_pc_AWID),
        .m_axi_awlen(s00_regslice_to_auto_pc_AWLEN),
        .m_axi_awlock(s00_regslice_to_auto_pc_AWLOCK),
        .m_axi_awprot(s00_regslice_to_auto_pc_AWPROT),
        .m_axi_awqos(s00_regslice_to_auto_pc_AWQOS),
        .m_axi_awready(s00_regslice_to_auto_pc_AWREADY),
        .m_axi_awsize(s00_regslice_to_auto_pc_AWSIZE),
        .m_axi_awvalid(s00_regslice_to_auto_pc_AWVALID),
        .m_axi_bid(s00_regslice_to_auto_pc_BID),
        .m_axi_bready(s00_regslice_to_auto_pc_BREADY),
        .m_axi_bresp(s00_regslice_to_auto_pc_BRESP),
        .m_axi_bvalid(s00_regslice_to_auto_pc_BVALID),
        .m_axi_rdata(s00_regslice_to_auto_pc_RDATA),
        .m_axi_rid(s00_regslice_to_auto_pc_RID),
        .m_axi_rlast(s00_regslice_to_auto_pc_RLAST),
        .m_axi_rready(s00_regslice_to_auto_pc_RREADY),
        .m_axi_rresp(s00_regslice_to_auto_pc_RRESP),
        .m_axi_rvalid(s00_regslice_to_auto_pc_RVALID),
        .m_axi_wdata(s00_regslice_to_auto_pc_WDATA),
        .m_axi_wid(s00_regslice_to_auto_pc_WID),
        .m_axi_wlast(s00_regslice_to_auto_pc_WLAST),
        .m_axi_wready(s00_regslice_to_auto_pc_WREADY),
        .m_axi_wstrb(s00_regslice_to_auto_pc_WSTRB),
        .m_axi_wvalid(s00_regslice_to_auto_pc_WVALID),
        .s_axi_araddr(s00_couplers_to_s00_regslice_ARADDR),
        .s_axi_arburst(s00_couplers_to_s00_regslice_ARBURST),
        .s_axi_arcache(s00_couplers_to_s00_regslice_ARCACHE),
        .s_axi_arid(s00_couplers_to_s00_regslice_ARID),
        .s_axi_arlen(s00_couplers_to_s00_regslice_ARLEN),
        .s_axi_arlock(s00_couplers_to_s00_regslice_ARLOCK),
        .s_axi_arprot(s00_couplers_to_s00_regslice_ARPROT),
        .s_axi_arqos(s00_couplers_to_s00_regslice_ARQOS),
        .s_axi_arready(s00_couplers_to_s00_regslice_ARREADY),
        .s_axi_arsize(s00_couplers_to_s00_regslice_ARSIZE),
        .s_axi_arvalid(s00_couplers_to_s00_regslice_ARVALID),
        .s_axi_awaddr(s00_couplers_to_s00_regslice_AWADDR),
        .s_axi_awburst(s00_couplers_to_s00_regslice_AWBURST),
        .s_axi_awcache(s00_couplers_to_s00_regslice_AWCACHE),
        .s_axi_awid(s00_couplers_to_s00_regslice_AWID),
        .s_axi_awlen(s00_couplers_to_s00_regslice_AWLEN),
        .s_axi_awlock(s00_couplers_to_s00_regslice_AWLOCK),
        .s_axi_awprot(s00_couplers_to_s00_regslice_AWPROT),
        .s_axi_awqos(s00_couplers_to_s00_regslice_AWQOS),
        .s_axi_awready(s00_couplers_to_s00_regslice_AWREADY),
        .s_axi_awsize(s00_couplers_to_s00_regslice_AWSIZE),
        .s_axi_awvalid(s00_couplers_to_s00_regslice_AWVALID),
        .s_axi_bid(s00_couplers_to_s00_regslice_BID),
        .s_axi_bready(s00_couplers_to_s00_regslice_BREADY),
        .s_axi_bresp(s00_couplers_to_s00_regslice_BRESP),
        .s_axi_bvalid(s00_couplers_to_s00_regslice_BVALID),
        .s_axi_rdata(s00_couplers_to_s00_regslice_RDATA),
        .s_axi_rid(s00_couplers_to_s00_regslice_RID),
        .s_axi_rlast(s00_couplers_to_s00_regslice_RLAST),
        .s_axi_rready(s00_couplers_to_s00_regslice_RREADY),
        .s_axi_rresp(s00_couplers_to_s00_regslice_RRESP),
        .s_axi_rvalid(s00_couplers_to_s00_regslice_RVALID),
        .s_axi_wdata(s00_couplers_to_s00_regslice_WDATA),
        .s_axi_wid(s00_couplers_to_s00_regslice_WID),
        .s_axi_wlast(s00_couplers_to_s00_regslice_WLAST),
        .s_axi_wready(s00_couplers_to_s00_regslice_WREADY),
        .s_axi_wstrb(s00_couplers_to_s00_regslice_WSTRB),
        .s_axi_wvalid(s00_couplers_to_s00_regslice_WVALID));
endmodule

module s00_couplers_imp_GW5FJ5
   (M_ACLK,
    M_ARESETN,
    M_AXI_araddr,
    M_AXI_arburst,
    M_AXI_arcache,
    M_AXI_arlen,
    M_AXI_arlock,
    M_AXI_arprot,
    M_AXI_arqos,
    M_AXI_arready,
    M_AXI_arsize,
    M_AXI_arvalid,
    M_AXI_rdata,
    M_AXI_rlast,
    M_AXI_rready,
    M_AXI_rresp,
    M_AXI_rvalid,
    S_ACLK,
    S_ARESETN,
    S_AXI_araddr,
    S_AXI_arburst,
    S_AXI_arcache,
    S_AXI_arlen,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arsize,
    S_AXI_aruser,
    S_AXI_arvalid,
    S_AXI_rdata,
    S_AXI_rlast,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid);
  input M_ACLK;
  input M_ARESETN;
  output [31:0]M_AXI_araddr;
  output [1:0]M_AXI_arburst;
  output [3:0]M_AXI_arcache;
  output [3:0]M_AXI_arlen;
  output [1:0]M_AXI_arlock;
  output [2:0]M_AXI_arprot;
  output [3:0]M_AXI_arqos;
  input M_AXI_arready;
  output [2:0]M_AXI_arsize;
  output M_AXI_arvalid;
  input [31:0]M_AXI_rdata;
  input M_AXI_rlast;
  output M_AXI_rready;
  input [1:0]M_AXI_rresp;
  input M_AXI_rvalid;
  input S_ACLK;
  input S_ARESETN;
  input [31:0]S_AXI_araddr;
  input [1:0]S_AXI_arburst;
  input [3:0]S_AXI_arcache;
  input [7:0]S_AXI_arlen;
  input [2:0]S_AXI_arprot;
  output S_AXI_arready;
  input [2:0]S_AXI_arsize;
  input [3:0]S_AXI_aruser;
  input S_AXI_arvalid;
  output [31:0]S_AXI_rdata;
  output S_AXI_rlast;
  input S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output S_AXI_rvalid;

  wire M_ACLK_1;
  wire M_ARESETN_1;
  wire S_ACLK_1;
  wire S_ARESETN_1;
  wire [31:0]auto_cc_to_auto_pc_ARADDR;
  wire [1:0]auto_cc_to_auto_pc_ARBURST;
  wire [3:0]auto_cc_to_auto_pc_ARCACHE;
  wire [7:0]auto_cc_to_auto_pc_ARLEN;
  wire [0:0]auto_cc_to_auto_pc_ARLOCK;
  wire [2:0]auto_cc_to_auto_pc_ARPROT;
  wire [3:0]auto_cc_to_auto_pc_ARQOS;
  wire auto_cc_to_auto_pc_ARREADY;
  wire [3:0]auto_cc_to_auto_pc_ARREGION;
  wire [2:0]auto_cc_to_auto_pc_ARSIZE;
  wire [3:0]auto_cc_to_auto_pc_ARUSER;
  wire auto_cc_to_auto_pc_ARVALID;
  wire [31:0]auto_cc_to_auto_pc_RDATA;
  wire auto_cc_to_auto_pc_RLAST;
  wire auto_cc_to_auto_pc_RREADY;
  wire [1:0]auto_cc_to_auto_pc_RRESP;
  wire auto_cc_to_auto_pc_RVALID;
  wire [31:0]auto_pc_to_s00_couplers_ARADDR;
  wire [1:0]auto_pc_to_s00_couplers_ARBURST;
  wire [3:0]auto_pc_to_s00_couplers_ARCACHE;
  wire [3:0]auto_pc_to_s00_couplers_ARLEN;
  wire [1:0]auto_pc_to_s00_couplers_ARLOCK;
  wire [2:0]auto_pc_to_s00_couplers_ARPROT;
  wire [3:0]auto_pc_to_s00_couplers_ARQOS;
  wire auto_pc_to_s00_couplers_ARREADY;
  wire [2:0]auto_pc_to_s00_couplers_ARSIZE;
  wire auto_pc_to_s00_couplers_ARVALID;
  wire [31:0]auto_pc_to_s00_couplers_RDATA;
  wire auto_pc_to_s00_couplers_RLAST;
  wire auto_pc_to_s00_couplers_RREADY;
  wire [1:0]auto_pc_to_s00_couplers_RRESP;
  wire auto_pc_to_s00_couplers_RVALID;
  wire [31:0]s00_couplers_to_s00_regslice_ARADDR;
  wire [1:0]s00_couplers_to_s00_regslice_ARBURST;
  wire [3:0]s00_couplers_to_s00_regslice_ARCACHE;
  wire [7:0]s00_couplers_to_s00_regslice_ARLEN;
  wire [2:0]s00_couplers_to_s00_regslice_ARPROT;
  wire s00_couplers_to_s00_regslice_ARREADY;
  wire [2:0]s00_couplers_to_s00_regslice_ARSIZE;
  wire [3:0]s00_couplers_to_s00_regslice_ARUSER;
  wire s00_couplers_to_s00_regslice_ARVALID;
  wire [31:0]s00_couplers_to_s00_regslice_RDATA;
  wire s00_couplers_to_s00_regslice_RLAST;
  wire s00_couplers_to_s00_regslice_RREADY;
  wire [1:0]s00_couplers_to_s00_regslice_RRESP;
  wire s00_couplers_to_s00_regslice_RVALID;
  wire [31:0]s00_regslice_to_auto_cc_ARADDR;
  wire [1:0]s00_regslice_to_auto_cc_ARBURST;
  wire [3:0]s00_regslice_to_auto_cc_ARCACHE;
  wire [7:0]s00_regslice_to_auto_cc_ARLEN;
  wire [0:0]s00_regslice_to_auto_cc_ARLOCK;
  wire [2:0]s00_regslice_to_auto_cc_ARPROT;
  wire [3:0]s00_regslice_to_auto_cc_ARQOS;
  wire s00_regslice_to_auto_cc_ARREADY;
  wire [3:0]s00_regslice_to_auto_cc_ARREGION;
  wire [2:0]s00_regslice_to_auto_cc_ARSIZE;
  wire [3:0]s00_regslice_to_auto_cc_ARUSER;
  wire s00_regslice_to_auto_cc_ARVALID;
  wire [31:0]s00_regslice_to_auto_cc_RDATA;
  wire s00_regslice_to_auto_cc_RLAST;
  wire s00_regslice_to_auto_cc_RREADY;
  wire [1:0]s00_regslice_to_auto_cc_RRESP;
  wire s00_regslice_to_auto_cc_RVALID;

  assign M_ACLK_1 = M_ACLK;
  assign M_ARESETN_1 = M_ARESETN;
  assign M_AXI_araddr[31:0] = auto_pc_to_s00_couplers_ARADDR;
  assign M_AXI_arburst[1:0] = auto_pc_to_s00_couplers_ARBURST;
  assign M_AXI_arcache[3:0] = auto_pc_to_s00_couplers_ARCACHE;
  assign M_AXI_arlen[3:0] = auto_pc_to_s00_couplers_ARLEN;
  assign M_AXI_arlock[1:0] = auto_pc_to_s00_couplers_ARLOCK;
  assign M_AXI_arprot[2:0] = auto_pc_to_s00_couplers_ARPROT;
  assign M_AXI_arqos[3:0] = auto_pc_to_s00_couplers_ARQOS;
  assign M_AXI_arsize[2:0] = auto_pc_to_s00_couplers_ARSIZE;
  assign M_AXI_arvalid = auto_pc_to_s00_couplers_ARVALID;
  assign M_AXI_rready = auto_pc_to_s00_couplers_RREADY;
  assign S_ACLK_1 = S_ACLK;
  assign S_ARESETN_1 = S_ARESETN;
  assign S_AXI_arready = s00_couplers_to_s00_regslice_ARREADY;
  assign S_AXI_rdata[31:0] = s00_couplers_to_s00_regslice_RDATA;
  assign S_AXI_rlast = s00_couplers_to_s00_regslice_RLAST;
  assign S_AXI_rresp[1:0] = s00_couplers_to_s00_regslice_RRESP;
  assign S_AXI_rvalid = s00_couplers_to_s00_regslice_RVALID;
  assign auto_pc_to_s00_couplers_ARREADY = M_AXI_arready;
  assign auto_pc_to_s00_couplers_RDATA = M_AXI_rdata[31:0];
  assign auto_pc_to_s00_couplers_RLAST = M_AXI_rlast;
  assign auto_pc_to_s00_couplers_RRESP = M_AXI_rresp[1:0];
  assign auto_pc_to_s00_couplers_RVALID = M_AXI_rvalid;
  assign s00_couplers_to_s00_regslice_ARADDR = S_AXI_araddr[31:0];
  assign s00_couplers_to_s00_regslice_ARBURST = S_AXI_arburst[1:0];
  assign s00_couplers_to_s00_regslice_ARCACHE = S_AXI_arcache[3:0];
  assign s00_couplers_to_s00_regslice_ARLEN = S_AXI_arlen[7:0];
  assign s00_couplers_to_s00_regslice_ARPROT = S_AXI_arprot[2:0];
  assign s00_couplers_to_s00_regslice_ARSIZE = S_AXI_arsize[2:0];
  assign s00_couplers_to_s00_regslice_ARUSER = S_AXI_aruser[3:0];
  assign s00_couplers_to_s00_regslice_ARVALID = S_AXI_arvalid;
  assign s00_couplers_to_s00_regslice_RREADY = S_AXI_rready;
  design_1_auto_cc_0 auto_cc
       (.m_axi_aclk(M_ACLK_1),
        .m_axi_araddr(auto_cc_to_auto_pc_ARADDR),
        .m_axi_arburst(auto_cc_to_auto_pc_ARBURST),
        .m_axi_arcache(auto_cc_to_auto_pc_ARCACHE),
        .m_axi_aresetn(M_ARESETN_1),
        .m_axi_arlen(auto_cc_to_auto_pc_ARLEN),
        .m_axi_arlock(auto_cc_to_auto_pc_ARLOCK),
        .m_axi_arprot(auto_cc_to_auto_pc_ARPROT),
        .m_axi_arqos(auto_cc_to_auto_pc_ARQOS),
        .m_axi_arready(auto_cc_to_auto_pc_ARREADY),
        .m_axi_arregion(auto_cc_to_auto_pc_ARREGION),
        .m_axi_arsize(auto_cc_to_auto_pc_ARSIZE),
        .m_axi_aruser(auto_cc_to_auto_pc_ARUSER),
        .m_axi_arvalid(auto_cc_to_auto_pc_ARVALID),
        .m_axi_rdata(auto_cc_to_auto_pc_RDATA),
        .m_axi_rlast(auto_cc_to_auto_pc_RLAST),
        .m_axi_rready(auto_cc_to_auto_pc_RREADY),
        .m_axi_rresp(auto_cc_to_auto_pc_RRESP),
        .m_axi_rvalid(auto_cc_to_auto_pc_RVALID),
        .s_axi_aclk(S_ACLK_1),
        .s_axi_araddr(s00_regslice_to_auto_cc_ARADDR),
        .s_axi_arburst(s00_regslice_to_auto_cc_ARBURST),
        .s_axi_arcache(s00_regslice_to_auto_cc_ARCACHE),
        .s_axi_aresetn(S_ARESETN_1),
        .s_axi_arlen(s00_regslice_to_auto_cc_ARLEN),
        .s_axi_arlock(s00_regslice_to_auto_cc_ARLOCK),
        .s_axi_arprot(s00_regslice_to_auto_cc_ARPROT),
        .s_axi_arqos(s00_regslice_to_auto_cc_ARQOS),
        .s_axi_arready(s00_regslice_to_auto_cc_ARREADY),
        .s_axi_arregion(s00_regslice_to_auto_cc_ARREGION),
        .s_axi_arsize(s00_regslice_to_auto_cc_ARSIZE),
        .s_axi_aruser(s00_regslice_to_auto_cc_ARUSER),
        .s_axi_arvalid(s00_regslice_to_auto_cc_ARVALID),
        .s_axi_rdata(s00_regslice_to_auto_cc_RDATA),
        .s_axi_rlast(s00_regslice_to_auto_cc_RLAST),
        .s_axi_rready(s00_regslice_to_auto_cc_RREADY),
        .s_axi_rresp(s00_regslice_to_auto_cc_RRESP),
        .s_axi_rvalid(s00_regslice_to_auto_cc_RVALID));
  design_1_auto_pc_1 auto_pc
       (.aclk(M_ACLK_1),
        .aresetn(M_ARESETN_1),
        .m_axi_araddr(auto_pc_to_s00_couplers_ARADDR),
        .m_axi_arburst(auto_pc_to_s00_couplers_ARBURST),
        .m_axi_arcache(auto_pc_to_s00_couplers_ARCACHE),
        .m_axi_arlen(auto_pc_to_s00_couplers_ARLEN),
        .m_axi_arlock(auto_pc_to_s00_couplers_ARLOCK),
        .m_axi_arprot(auto_pc_to_s00_couplers_ARPROT),
        .m_axi_arqos(auto_pc_to_s00_couplers_ARQOS),
        .m_axi_arready(auto_pc_to_s00_couplers_ARREADY),
        .m_axi_arsize(auto_pc_to_s00_couplers_ARSIZE),
        .m_axi_arvalid(auto_pc_to_s00_couplers_ARVALID),
        .m_axi_rdata(auto_pc_to_s00_couplers_RDATA),
        .m_axi_rlast(auto_pc_to_s00_couplers_RLAST),
        .m_axi_rready(auto_pc_to_s00_couplers_RREADY),
        .m_axi_rresp(auto_pc_to_s00_couplers_RRESP),
        .m_axi_rvalid(auto_pc_to_s00_couplers_RVALID),
        .s_axi_araddr(auto_cc_to_auto_pc_ARADDR),
        .s_axi_arburst(auto_cc_to_auto_pc_ARBURST),
        .s_axi_arcache(auto_cc_to_auto_pc_ARCACHE),
        .s_axi_arlen(auto_cc_to_auto_pc_ARLEN),
        .s_axi_arlock(auto_cc_to_auto_pc_ARLOCK),
        .s_axi_arprot(auto_cc_to_auto_pc_ARPROT),
        .s_axi_arqos(auto_cc_to_auto_pc_ARQOS),
        .s_axi_arready(auto_cc_to_auto_pc_ARREADY),
        .s_axi_arregion(auto_cc_to_auto_pc_ARREGION),
        .s_axi_arsize(auto_cc_to_auto_pc_ARSIZE),
        .s_axi_aruser(auto_cc_to_auto_pc_ARUSER),
        .s_axi_arvalid(auto_cc_to_auto_pc_ARVALID),
        .s_axi_rdata(auto_cc_to_auto_pc_RDATA),
        .s_axi_rlast(auto_cc_to_auto_pc_RLAST),
        .s_axi_rready(auto_cc_to_auto_pc_RREADY),
        .s_axi_rresp(auto_cc_to_auto_pc_RRESP),
        .s_axi_rvalid(auto_cc_to_auto_pc_RVALID));
  design_1_s00_regslice_127 s00_regslice
       (.aclk(S_ACLK_1),
        .aresetn(S_ARESETN_1),
        .m_axi_araddr(s00_regslice_to_auto_cc_ARADDR),
        .m_axi_arburst(s00_regslice_to_auto_cc_ARBURST),
        .m_axi_arcache(s00_regslice_to_auto_cc_ARCACHE),
        .m_axi_arlen(s00_regslice_to_auto_cc_ARLEN),
        .m_axi_arlock(s00_regslice_to_auto_cc_ARLOCK),
        .m_axi_arprot(s00_regslice_to_auto_cc_ARPROT),
        .m_axi_arqos(s00_regslice_to_auto_cc_ARQOS),
        .m_axi_arready(s00_regslice_to_auto_cc_ARREADY),
        .m_axi_arregion(s00_regslice_to_auto_cc_ARREGION),
        .m_axi_arsize(s00_regslice_to_auto_cc_ARSIZE),
        .m_axi_aruser(s00_regslice_to_auto_cc_ARUSER),
        .m_axi_arvalid(s00_regslice_to_auto_cc_ARVALID),
        .m_axi_rdata(s00_regslice_to_auto_cc_RDATA),
        .m_axi_rlast(s00_regslice_to_auto_cc_RLAST),
        .m_axi_rready(s00_regslice_to_auto_cc_RREADY),
        .m_axi_rresp(s00_regslice_to_auto_cc_RRESP),
        .m_axi_rvalid(s00_regslice_to_auto_cc_RVALID),
        .s_axi_araddr(s00_couplers_to_s00_regslice_ARADDR),
        .s_axi_arburst(s00_couplers_to_s00_regslice_ARBURST),
        .s_axi_arcache(s00_couplers_to_s00_regslice_ARCACHE),
        .s_axi_arlen(s00_couplers_to_s00_regslice_ARLEN),
        .s_axi_arlock(1'b0),
        .s_axi_arprot(s00_couplers_to_s00_regslice_ARPROT),
        .s_axi_arqos({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arready(s00_couplers_to_s00_regslice_ARREADY),
        .s_axi_arregion({1'b0,1'b0,1'b0,1'b0}),
        .s_axi_arsize(s00_couplers_to_s00_regslice_ARSIZE),
        .s_axi_aruser(s00_couplers_to_s00_regslice_ARUSER),
        .s_axi_arvalid(s00_couplers_to_s00_regslice_ARVALID),
        .s_axi_rdata(s00_couplers_to_s00_regslice_RDATA),
        .s_axi_rlast(s00_couplers_to_s00_regslice_RLAST),
        .s_axi_rready(s00_couplers_to_s00_regslice_RREADY),
        .s_axi_rresp(s00_couplers_to_s00_regslice_RRESP),
        .s_axi_rvalid(s00_couplers_to_s00_regslice_RVALID));
endmodule

module video_imp_1FOBHOA
   (M00_AXI_araddr,
    M00_AXI_arburst,
    M00_AXI_arcache,
    M00_AXI_arlen,
    M00_AXI_arlock,
    M00_AXI_arprot,
    M00_AXI_arqos,
    M00_AXI_arready,
    M00_AXI_arsize,
    M00_AXI_arvalid,
    M00_AXI_rdata,
    M00_AXI_rlast,
    M00_AXI_rready,
    M00_AXI_rresp,
    M00_AXI_rvalid,
    S_AXI_LITE_araddr,
    S_AXI_LITE_arready,
    S_AXI_LITE_arvalid,
    S_AXI_LITE_awaddr,
    S_AXI_LITE_awready,
    S_AXI_LITE_awvalid,
    S_AXI_LITE_bready,
    S_AXI_LITE_bresp,
    S_AXI_LITE_bvalid,
    S_AXI_LITE_rdata,
    S_AXI_LITE_rready,
    S_AXI_LITE_rresp,
    S_AXI_LITE_rvalid,
    S_AXI_LITE_wdata,
    S_AXI_LITE_wready,
    S_AXI_LITE_wvalid,
    S_AXI_araddr,
    S_AXI_arprot,
    S_AXI_arready,
    S_AXI_arvalid,
    S_AXI_awaddr,
    S_AXI_awprot,
    S_AXI_awready,
    S_AXI_awvalid,
    S_AXI_bready,
    S_AXI_bresp,
    S_AXI_bvalid,
    S_AXI_rdata,
    S_AXI_rready,
    S_AXI_rresp,
    S_AXI_rvalid,
    S_AXI_wdata,
    S_AXI_wready,
    S_AXI_wstrb,
    S_AXI_wvalid,
    aresetn,
    audio_irq,
    control_vblank,
    dout,
    ext_reset_in,
    hdmi_clk,
    hdmi_data,
    hdmi_de,
    hdmi_hs,
    hdmi_intn,
    hdmi_vs,
    peripheral_aresetn,
    s_axi_aclk,
    s_axi_aresetn,
    s_axis_aclk,
    vid_clk);
  output [31:0]M00_AXI_araddr;
  output [1:0]M00_AXI_arburst;
  output [3:0]M00_AXI_arcache;
  output [3:0]M00_AXI_arlen;
  output [1:0]M00_AXI_arlock;
  output [2:0]M00_AXI_arprot;
  output [3:0]M00_AXI_arqos;
  input M00_AXI_arready;
  output [2:0]M00_AXI_arsize;
  output M00_AXI_arvalid;
  input [31:0]M00_AXI_rdata;
  input M00_AXI_rlast;
  output M00_AXI_rready;
  input [1:0]M00_AXI_rresp;
  input M00_AXI_rvalid;
  input [31:0]S_AXI_LITE_araddr;
  output [0:0]S_AXI_LITE_arready;
  input [0:0]S_AXI_LITE_arvalid;
  input [31:0]S_AXI_LITE_awaddr;
  output [0:0]S_AXI_LITE_awready;
  input [0:0]S_AXI_LITE_awvalid;
  input [0:0]S_AXI_LITE_bready;
  output [1:0]S_AXI_LITE_bresp;
  output [0:0]S_AXI_LITE_bvalid;
  output [31:0]S_AXI_LITE_rdata;
  input [0:0]S_AXI_LITE_rready;
  output [1:0]S_AXI_LITE_rresp;
  output [0:0]S_AXI_LITE_rvalid;
  input [31:0]S_AXI_LITE_wdata;
  output [0:0]S_AXI_LITE_wready;
  input [0:0]S_AXI_LITE_wvalid;
  input [31:0]S_AXI_araddr;
  input [2:0]S_AXI_arprot;
  output [0:0]S_AXI_arready;
  input [0:0]S_AXI_arvalid;
  input [31:0]S_AXI_awaddr;
  input [2:0]S_AXI_awprot;
  output [0:0]S_AXI_awready;
  input [0:0]S_AXI_awvalid;
  input [0:0]S_AXI_bready;
  output [1:0]S_AXI_bresp;
  output [0:0]S_AXI_bvalid;
  output [31:0]S_AXI_rdata;
  input [0:0]S_AXI_rready;
  output [1:0]S_AXI_rresp;
  output [0:0]S_AXI_rvalid;
  input [31:0]S_AXI_wdata;
  output [0:0]S_AXI_wready;
  input [3:0]S_AXI_wstrb;
  input [0:0]S_AXI_wvalid;
  output [0:0]aresetn;
  input [0:0]audio_irq;
  output [1:0]control_vblank;
  output [4:0]dout;
  input ext_reset_in;
  output hdmi_clk;
  output [15:0]hdmi_data;
  output hdmi_de;
  output hdmi_hs;
  input [0:0]hdmi_intn;
  output hdmi_vs;
  output [0:0]peripheral_aresetn;
  input s_axi_aclk;
  input s_axi_aresetn;
  input s_axis_aclk;
  input vid_clk;

  wire [31:0]Conn2_ARADDR;
  wire Conn2_ARREADY;
  wire [0:0]Conn2_ARVALID;
  wire [31:0]Conn2_AWADDR;
  wire Conn2_AWREADY;
  wire [0:0]Conn2_AWVALID;
  wire [0:0]Conn2_BREADY;
  wire [1:0]Conn2_BRESP;
  wire Conn2_BVALID;
  wire [31:0]Conn2_RDATA;
  wire [0:0]Conn2_RREADY;
  wire [1:0]Conn2_RRESP;
  wire Conn2_RVALID;
  wire [31:0]Conn2_WDATA;
  wire Conn2_WREADY;
  wire [0:0]Conn2_WVALID;
  wire [0:0]In3_1;
  wire [31:0]axi_interconnect_1_M00_AXI_ARADDR;
  wire [1:0]axi_interconnect_1_M00_AXI_ARBURST;
  wire [3:0]axi_interconnect_1_M00_AXI_ARCACHE;
  wire [3:0]axi_interconnect_1_M00_AXI_ARLEN;
  wire [1:0]axi_interconnect_1_M00_AXI_ARLOCK;
  wire [2:0]axi_interconnect_1_M00_AXI_ARPROT;
  wire [3:0]axi_interconnect_1_M00_AXI_ARQOS;
  wire axi_interconnect_1_M00_AXI_ARREADY;
  wire [2:0]axi_interconnect_1_M00_AXI_ARSIZE;
  wire axi_interconnect_1_M00_AXI_ARVALID;
  wire [31:0]axi_interconnect_1_M00_AXI_RDATA;
  wire axi_interconnect_1_M00_AXI_RLAST;
  wire axi_interconnect_1_M00_AXI_RREADY;
  wire [1:0]axi_interconnect_1_M00_AXI_RRESP;
  wire axi_interconnect_1_M00_AXI_RVALID;
  wire [31:0]axi_vdma_0_M_AXIS_MM2S_TDATA;
  wire [3:0]axi_vdma_0_M_AXIS_MM2S_TKEEP;
  wire axi_vdma_0_M_AXIS_MM2S_TLAST;
  wire axi_vdma_0_M_AXIS_MM2S_TREADY;
  wire [0:0]axi_vdma_0_M_AXIS_MM2S_TUSER;
  wire axi_vdma_0_M_AXIS_MM2S_TVALID;
  wire [31:0]axi_vdma_0_M_AXI_MM2S_ARADDR;
  wire [1:0]axi_vdma_0_M_AXI_MM2S_ARBURST;
  wire [3:0]axi_vdma_0_M_AXI_MM2S_ARCACHE;
  wire [7:0]axi_vdma_0_M_AXI_MM2S_ARLEN;
  wire [2:0]axi_vdma_0_M_AXI_MM2S_ARPROT;
  wire axi_vdma_0_M_AXI_MM2S_ARREADY;
  wire [2:0]axi_vdma_0_M_AXI_MM2S_ARSIZE;
  wire axi_vdma_0_M_AXI_MM2S_ARVALID;
  wire [31:0]axi_vdma_0_M_AXI_MM2S_RDATA;
  wire axi_vdma_0_M_AXI_MM2S_RLAST;
  wire axi_vdma_0_M_AXI_MM2S_RREADY;
  wire [1:0]axi_vdma_0_M_AXI_MM2S_RRESP;
  wire axi_vdma_0_M_AXI_MM2S_RVALID;
  wire axi_vdma_0_mm2s_introut;
  wire [31:0]axis_data_fifo_0_M_AXIS_TDATA;
  wire axis_data_fifo_0_M_AXIS_TLAST;
  wire axis_data_fifo_0_M_AXIS_TREADY;
  wire [0:0]axis_data_fifo_0_M_AXIS_TUSER;
  wire axis_data_fifo_0_M_AXIS_TVALID;
  wire [31:0]ctrl_1_ARADDR;
  wire [2:0]ctrl_1_ARPROT;
  wire ctrl_1_ARREADY;
  wire [0:0]ctrl_1_ARVALID;
  wire [31:0]ctrl_1_AWADDR;
  wire [2:0]ctrl_1_AWPROT;
  wire ctrl_1_AWREADY;
  wire [0:0]ctrl_1_AWVALID;
  wire [0:0]ctrl_1_BREADY;
  wire [1:0]ctrl_1_BRESP;
  wire ctrl_1_BVALID;
  wire [31:0]ctrl_1_RDATA;
  wire [0:0]ctrl_1_RREADY;
  wire [1:0]ctrl_1_RRESP;
  wire ctrl_1_RVALID;
  wire [31:0]ctrl_1_WDATA;
  wire ctrl_1_WREADY;
  wire [3:0]ctrl_1_WSTRB;
  wire [0:0]ctrl_1_WVALID;
  wire ext_reset_in_0_1;
  wire [0:0]hdmi_intn_1;
  wire [0:0]rst_ps7_0_200M_1_peripheral_aresetn;
  wire s_axi_aclk_0_1;
  wire s_axi_aresetn_0_1;
  wire s_axis_aclk_1;
  wire [0:0]s_axis_aresetn_1;
  wire [0:0]util_vector_logic_0_Res;
  wire vid_clk_1;
  wire [1:0]video_formatter_0_control_vblank;
  wire video_formatter_0_dvi_active_video;
  wire video_formatter_0_dvi_hsync;
  wire video_formatter_0_dvi_vsync;
  wire [15:0]video_formatter_0_hdmi_data;
  wire [4:0]xlconcat_0_dout;
  wire [0:0]xlslice_0_Dout;

  assign Conn2_ARADDR = S_AXI_LITE_araddr[31:0];
  assign Conn2_ARVALID = S_AXI_LITE_arvalid[0];
  assign Conn2_AWADDR = S_AXI_LITE_awaddr[31:0];
  assign Conn2_AWVALID = S_AXI_LITE_awvalid[0];
  assign Conn2_BREADY = S_AXI_LITE_bready[0];
  assign Conn2_RREADY = S_AXI_LITE_rready[0];
  assign Conn2_WDATA = S_AXI_LITE_wdata[31:0];
  assign Conn2_WVALID = S_AXI_LITE_wvalid[0];
  assign In3_1 = audio_irq[0];
  assign M00_AXI_araddr[31:0] = axi_interconnect_1_M00_AXI_ARADDR;
  assign M00_AXI_arburst[1:0] = axi_interconnect_1_M00_AXI_ARBURST;
  assign M00_AXI_arcache[3:0] = axi_interconnect_1_M00_AXI_ARCACHE;
  assign M00_AXI_arlen[3:0] = axi_interconnect_1_M00_AXI_ARLEN;
  assign M00_AXI_arlock[1:0] = axi_interconnect_1_M00_AXI_ARLOCK;
  assign M00_AXI_arprot[2:0] = axi_interconnect_1_M00_AXI_ARPROT;
  assign M00_AXI_arqos[3:0] = axi_interconnect_1_M00_AXI_ARQOS;
  assign M00_AXI_arsize[2:0] = axi_interconnect_1_M00_AXI_ARSIZE;
  assign M00_AXI_arvalid = axi_interconnect_1_M00_AXI_ARVALID;
  assign M00_AXI_rready = axi_interconnect_1_M00_AXI_RREADY;
  assign S_AXI_LITE_arready[0] = Conn2_ARREADY;
  assign S_AXI_LITE_awready[0] = Conn2_AWREADY;
  assign S_AXI_LITE_bresp[1:0] = Conn2_BRESP;
  assign S_AXI_LITE_bvalid[0] = Conn2_BVALID;
  assign S_AXI_LITE_rdata[31:0] = Conn2_RDATA;
  assign S_AXI_LITE_rresp[1:0] = Conn2_RRESP;
  assign S_AXI_LITE_rvalid[0] = Conn2_RVALID;
  assign S_AXI_LITE_wready[0] = Conn2_WREADY;
  assign S_AXI_arready[0] = ctrl_1_ARREADY;
  assign S_AXI_awready[0] = ctrl_1_AWREADY;
  assign S_AXI_bresp[1:0] = ctrl_1_BRESP;
  assign S_AXI_bvalid[0] = ctrl_1_BVALID;
  assign S_AXI_rdata[31:0] = ctrl_1_RDATA;
  assign S_AXI_rresp[1:0] = ctrl_1_RRESP;
  assign S_AXI_rvalid[0] = ctrl_1_RVALID;
  assign S_AXI_wready[0] = ctrl_1_WREADY;
  assign aresetn[0] = s_axis_aresetn_1;
  assign axi_interconnect_1_M00_AXI_ARREADY = M00_AXI_arready;
  assign axi_interconnect_1_M00_AXI_RDATA = M00_AXI_rdata[31:0];
  assign axi_interconnect_1_M00_AXI_RLAST = M00_AXI_rlast;
  assign axi_interconnect_1_M00_AXI_RRESP = M00_AXI_rresp[1:0];
  assign axi_interconnect_1_M00_AXI_RVALID = M00_AXI_rvalid;
  assign control_vblank[1:0] = video_formatter_0_control_vblank;
  assign ctrl_1_ARADDR = S_AXI_araddr[31:0];
  assign ctrl_1_ARPROT = S_AXI_arprot[2:0];
  assign ctrl_1_ARVALID = S_AXI_arvalid[0];
  assign ctrl_1_AWADDR = S_AXI_awaddr[31:0];
  assign ctrl_1_AWPROT = S_AXI_awprot[2:0];
  assign ctrl_1_AWVALID = S_AXI_awvalid[0];
  assign ctrl_1_BREADY = S_AXI_bready[0];
  assign ctrl_1_RREADY = S_AXI_rready[0];
  assign ctrl_1_WDATA = S_AXI_wdata[31:0];
  assign ctrl_1_WSTRB = S_AXI_wstrb[3:0];
  assign ctrl_1_WVALID = S_AXI_wvalid[0];
  assign dout[4:0] = xlconcat_0_dout;
  assign ext_reset_in_0_1 = ext_reset_in;
  assign hdmi_clk = vid_clk_1;
  assign hdmi_data[15:0] = video_formatter_0_hdmi_data;
  assign hdmi_de = video_formatter_0_dvi_active_video;
  assign hdmi_hs = video_formatter_0_dvi_hsync;
  assign hdmi_intn_1 = hdmi_intn[0];
  assign hdmi_vs = video_formatter_0_dvi_vsync;
  assign peripheral_aresetn[0] = rst_ps7_0_200M_1_peripheral_aresetn;
  assign s_axi_aclk_0_1 = s_axi_aclk;
  assign s_axi_aresetn_0_1 = s_axi_aresetn;
  assign s_axis_aclk_1 = s_axis_aclk;
  assign vid_clk_1 = vid_clk;
  design_1_axi_interconnect_1_0 axi_interconnect_1
       (.ACLK(s_axis_aclk_1),
        .ARESETN(s_axis_aresetn_1),
        .M00_ACLK(s_axis_aclk_1),
        .M00_ARESETN(rst_ps7_0_200M_1_peripheral_aresetn),
        .M00_AXI_araddr(axi_interconnect_1_M00_AXI_ARADDR),
        .M00_AXI_arburst(axi_interconnect_1_M00_AXI_ARBURST),
        .M00_AXI_arcache(axi_interconnect_1_M00_AXI_ARCACHE),
        .M00_AXI_arlen(axi_interconnect_1_M00_AXI_ARLEN),
        .M00_AXI_arlock(axi_interconnect_1_M00_AXI_ARLOCK),
        .M00_AXI_arprot(axi_interconnect_1_M00_AXI_ARPROT),
        .M00_AXI_arqos(axi_interconnect_1_M00_AXI_ARQOS),
        .M00_AXI_arready(axi_interconnect_1_M00_AXI_ARREADY),
        .M00_AXI_arsize(axi_interconnect_1_M00_AXI_ARSIZE),
        .M00_AXI_arvalid(axi_interconnect_1_M00_AXI_ARVALID),
        .M00_AXI_rdata(axi_interconnect_1_M00_AXI_RDATA),
        .M00_AXI_rlast(axi_interconnect_1_M00_AXI_RLAST),
        .M00_AXI_rready(axi_interconnect_1_M00_AXI_RREADY),
        .M00_AXI_rresp(axi_interconnect_1_M00_AXI_RRESP),
        .M00_AXI_rvalid(axi_interconnect_1_M00_AXI_RVALID),
        .S00_ACLK(s_axis_aclk_1),
        .S00_ARESETN(rst_ps7_0_200M_1_peripheral_aresetn),
        .S00_AXI_araddr(axi_vdma_0_M_AXI_MM2S_ARADDR),
        .S00_AXI_arburst(axi_vdma_0_M_AXI_MM2S_ARBURST),
        .S00_AXI_arcache(axi_vdma_0_M_AXI_MM2S_ARCACHE),
        .S00_AXI_arlen(axi_vdma_0_M_AXI_MM2S_ARLEN),
        .S00_AXI_arprot(axi_vdma_0_M_AXI_MM2S_ARPROT),
        .S00_AXI_arready(axi_vdma_0_M_AXI_MM2S_ARREADY),
        .S00_AXI_arsize(axi_vdma_0_M_AXI_MM2S_ARSIZE),
        .S00_AXI_arvalid(axi_vdma_0_M_AXI_MM2S_ARVALID),
        .S00_AXI_rdata(axi_vdma_0_M_AXI_MM2S_RDATA),
        .S00_AXI_rlast(axi_vdma_0_M_AXI_MM2S_RLAST),
        .S00_AXI_rready(axi_vdma_0_M_AXI_MM2S_RREADY),
        .S00_AXI_rresp(axi_vdma_0_M_AXI_MM2S_RRESP),
        .S00_AXI_rvalid(axi_vdma_0_M_AXI_MM2S_RVALID));
  design_1_axi_vdma_0_1 axi_vdma_0
       (.axi_resetn(s_axi_aresetn_0_1),
        .m_axi_mm2s_aclk(s_axis_aclk_1),
        .m_axi_mm2s_araddr(axi_vdma_0_M_AXI_MM2S_ARADDR),
        .m_axi_mm2s_arburst(axi_vdma_0_M_AXI_MM2S_ARBURST),
        .m_axi_mm2s_arcache(axi_vdma_0_M_AXI_MM2S_ARCACHE),
        .m_axi_mm2s_arlen(axi_vdma_0_M_AXI_MM2S_ARLEN),
        .m_axi_mm2s_arprot(axi_vdma_0_M_AXI_MM2S_ARPROT),
        .m_axi_mm2s_arready(axi_vdma_0_M_AXI_MM2S_ARREADY),
        .m_axi_mm2s_arsize(axi_vdma_0_M_AXI_MM2S_ARSIZE),
        .m_axi_mm2s_arvalid(axi_vdma_0_M_AXI_MM2S_ARVALID),
        .m_axi_mm2s_rdata(axi_vdma_0_M_AXI_MM2S_RDATA),
        .m_axi_mm2s_rlast(axi_vdma_0_M_AXI_MM2S_RLAST),
        .m_axi_mm2s_rready(axi_vdma_0_M_AXI_MM2S_RREADY),
        .m_axi_mm2s_rresp(axi_vdma_0_M_AXI_MM2S_RRESP),
        .m_axi_mm2s_rvalid(axi_vdma_0_M_AXI_MM2S_RVALID),
        .m_axis_mm2s_aclk(s_axis_aclk_1),
        .m_axis_mm2s_tdata(axi_vdma_0_M_AXIS_MM2S_TDATA),
        .m_axis_mm2s_tkeep(axi_vdma_0_M_AXIS_MM2S_TKEEP),
        .m_axis_mm2s_tlast(axi_vdma_0_M_AXIS_MM2S_TLAST),
        .m_axis_mm2s_tready(axi_vdma_0_M_AXIS_MM2S_TREADY),
        .m_axis_mm2s_tuser(axi_vdma_0_M_AXIS_MM2S_TUSER),
        .m_axis_mm2s_tvalid(axi_vdma_0_M_AXIS_MM2S_TVALID),
        .mm2s_introut(axi_vdma_0_mm2s_introut),
        .s_axi_lite_aclk(s_axi_aclk_0_1),
        .s_axi_lite_araddr(Conn2_ARADDR[8:0]),
        .s_axi_lite_arready(Conn2_ARREADY),
        .s_axi_lite_arvalid(Conn2_ARVALID),
        .s_axi_lite_awaddr(Conn2_AWADDR[8:0]),
        .s_axi_lite_awready(Conn2_AWREADY),
        .s_axi_lite_awvalid(Conn2_AWVALID),
        .s_axi_lite_bready(Conn2_BREADY),
        .s_axi_lite_bresp(Conn2_BRESP),
        .s_axi_lite_bvalid(Conn2_BVALID),
        .s_axi_lite_rdata(Conn2_RDATA),
        .s_axi_lite_rready(Conn2_RREADY),
        .s_axi_lite_rresp(Conn2_RRESP),
        .s_axi_lite_rvalid(Conn2_RVALID),
        .s_axi_lite_wdata(Conn2_WDATA),
        .s_axi_lite_wready(Conn2_WREADY),
        .s_axi_lite_wvalid(Conn2_WVALID));
  design_1_axis_data_fifo_0_0 axis_data_fifo_0
       (.m_axis_tdata(axis_data_fifo_0_M_AXIS_TDATA),
        .m_axis_tlast(axis_data_fifo_0_M_AXIS_TLAST),
        .m_axis_tready(axis_data_fifo_0_M_AXIS_TREADY),
        .m_axis_tuser(axis_data_fifo_0_M_AXIS_TUSER),
        .m_axis_tvalid(axis_data_fifo_0_M_AXIS_TVALID),
        .s_axis_aclk(s_axis_aclk_1),
        .s_axis_aresetn(rst_ps7_0_200M_1_peripheral_aresetn),
        .s_axis_tdata(axi_vdma_0_M_AXIS_MM2S_TDATA),
        .s_axis_tkeep(axi_vdma_0_M_AXIS_MM2S_TKEEP),
        .s_axis_tlast(axi_vdma_0_M_AXIS_MM2S_TLAST),
        .s_axis_tready(axi_vdma_0_M_AXIS_MM2S_TREADY),
        .s_axis_tuser(axi_vdma_0_M_AXIS_MM2S_TUSER),
        .s_axis_tvalid(axi_vdma_0_M_AXIS_MM2S_TVALID));
  design_1_rst_ps7_0_200M_1_1 rst_ps7_0_200M_1
       (.aux_reset_in(1'b1),
        .dcm_locked(1'b1),
        .ext_reset_in(ext_reset_in_0_1),
        .interconnect_aresetn(s_axis_aresetn_1),
        .mb_debug_sys_rst(1'b0),
        .peripheral_aresetn(rst_ps7_0_200M_1_peripheral_aresetn),
        .slowest_sync_clk(s_axis_aclk_1));
  design_1_util_vector_logic_0_1 util_vector_logic_0
       (.Op1(hdmi_intn_1),
        .Res(util_vector_logic_0_Res));
  design_1_video_formatter_0_0 video_formatter_0
       (.S_AXI_ACLK(s_axi_aclk_0_1),
        .S_AXI_ARADDR(ctrl_1_ARADDR[7:0]),
        .S_AXI_ARESETN(s_axi_aresetn_0_1),
        .S_AXI_ARPROT(ctrl_1_ARPROT),
        .S_AXI_ARREADY(ctrl_1_ARREADY),
        .S_AXI_ARVALID(ctrl_1_ARVALID),
        .S_AXI_AWADDR(ctrl_1_AWADDR[7:0]),
        .S_AXI_AWPROT(ctrl_1_AWPROT),
        .S_AXI_AWREADY(ctrl_1_AWREADY),
        .S_AXI_AWVALID(ctrl_1_AWVALID),
        .S_AXI_BREADY(ctrl_1_BREADY),
        .S_AXI_BRESP(ctrl_1_BRESP),
        .S_AXI_BVALID(ctrl_1_BVALID),
        .S_AXI_RDATA(ctrl_1_RDATA),
        .S_AXI_RREADY(ctrl_1_RREADY),
        .S_AXI_RRESP(ctrl_1_RRESP),
        .S_AXI_RVALID(ctrl_1_RVALID),
        .S_AXI_WDATA(ctrl_1_WDATA),
        .S_AXI_WREADY(ctrl_1_WREADY),
        .S_AXI_WSTRB(ctrl_1_WSTRB),
        .S_AXI_WVALID(ctrl_1_WVALID),
        .aresetn(ext_reset_in_0_1),
        .control_vblank_out(video_formatter_0_control_vblank),
        .dvi_active_video(video_formatter_0_dvi_active_video),
        .dvi_clk(vid_clk_1),
        .dvi_hsync(video_formatter_0_dvi_hsync),
        .dvi_vsync(video_formatter_0_dvi_vsync),
        .hdmi_data(video_formatter_0_hdmi_data),
        .m_axis_vid_aclk(s_axis_aclk_1),
        .m_axis_vid_tdata(axis_data_fifo_0_M_AXIS_TDATA),
        .m_axis_vid_tlast(axis_data_fifo_0_M_AXIS_TLAST),
        .m_axis_vid_tready(axis_data_fifo_0_M_AXIS_TREADY),
        .m_axis_vid_tuser(axis_data_fifo_0_M_AXIS_TUSER),
        .m_axis_vid_tvalid(axis_data_fifo_0_M_AXIS_TVALID));
  design_1_xlconcat_0_1 xlconcat_0
       (.In0(util_vector_logic_0_Res),
        .In1(axi_vdma_0_mm2s_introut),
        .In2(xlslice_0_Dout),
        .In3(In3_1),
        .In4(xlslice_0_Dout),
        .dout(xlconcat_0_dout));
  design_1_xlslice_0_0 xlslice_0
       (.Din(video_formatter_0_control_vblank),
        .Dout(xlslice_0_Dout));
endmodule
