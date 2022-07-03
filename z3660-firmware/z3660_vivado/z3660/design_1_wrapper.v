//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (win64) Build 2405991 Thu Dec  6 23:38:27 MST 2018
//Date        : Sun Jul  3 15:48:25 2022
//Host        : JUANANTONIO9B59 running 64-bit major release  (build 9200)
//Command     : generate_target design_1_wrapper.bd
//Design      : design_1_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module design_1_wrapper
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
    IIC_0_scl_io,
    IIC_0_sda_io,
    NU_1,
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
    nTS);
  input [31:0]A060;
  output BCLK_clk;
  output BP;
  output CLK90_clk;
  output CPUCLK_clk;
  inout [31:0]D040;
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
  inout IIC_0_scl_io;
  inout IIC_0_sda_io;
  output NU_1;
  output PCLK_clk;
  input R_W040;
  input [1:0]SIZ40;
  output hdmi_clk;
  output [15:0]hdmi_data;
  output hdmi_de;
  output hdmi_hs;
  input [0:0]hdmi_intn;
  output hdmi_vs;
  output nCLKEN_clk;
  output nTA;
  output nTBI;
  input nTCI;
  input nTEA;
  input nTS;

  wire [31:0]A060;
  wire BCLK_clk;
  wire BP;
  wire CLK90_clk;
  wire CPUCLK_clk;
  wire [31:0]D040;
  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire IIC_0_scl_i;
  wire IIC_0_scl_io;
  wire IIC_0_scl_o;
  wire IIC_0_scl_t;
  wire IIC_0_sda_i;
  wire IIC_0_sda_io;
  wire IIC_0_sda_o;
  wire IIC_0_sda_t;
  wire NU_1;
  wire PCLK_clk;
  wire R_W040;
  wire [1:0]SIZ40;
  wire hdmi_clk;
  wire [15:0]hdmi_data;
  wire hdmi_de;
  wire hdmi_hs;
  wire [0:0]hdmi_intn;
  wire hdmi_vs;
  wire nCLKEN_clk;
  wire nTA;
  wire nTBI;
  wire nTCI;
  wire nTEA;
  wire nTS;

  IOBUF IIC_0_scl_iobuf
       (.I(IIC_0_scl_o),
        .IO(IIC_0_scl_io),
        .O(IIC_0_scl_i),
        .T(IIC_0_scl_t));
  IOBUF IIC_0_sda_iobuf
       (.I(IIC_0_sda_o),
        .IO(IIC_0_sda_io),
        .O(IIC_0_sda_i),
        .T(IIC_0_sda_t));
  design_1 design_1_i
       (.A060(A060),
        .BCLK_clk(BCLK_clk),
        .BP(BP),
        .CLK90_clk(CLK90_clk),
        .CPUCLK_clk(CPUCLK_clk),
        .D040(D040),
        .DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .IIC_0_scl_i(IIC_0_scl_i),
        .IIC_0_scl_o(IIC_0_scl_o),
        .IIC_0_scl_t(IIC_0_scl_t),
        .IIC_0_sda_i(IIC_0_sda_i),
        .IIC_0_sda_o(IIC_0_sda_o),
        .IIC_0_sda_t(IIC_0_sda_t),
        .NU_1(NU_1),
        .PCLK_clk(PCLK_clk),
        .R_W040(R_W040),
        .SIZ40(SIZ40),
        .hdmi_clk(hdmi_clk),
        .hdmi_data(hdmi_data),
        .hdmi_de(hdmi_de),
        .hdmi_hs(hdmi_hs),
        .hdmi_intn(hdmi_intn),
        .hdmi_vs(hdmi_vs),
        .nCLKEN_clk(nCLKEN_clk),
        .nTA(nTA),
        .nTBI(nTBI),
        .nTCI(nTCI),
        .nTEA(nTEA),
        .nTS(nTS));
endmodule
