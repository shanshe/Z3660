
// file: design_1_clk_wiz_0_0.v
// (c) Copyright 2017-2018, 2023 Advanced Micro Devices, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of AMD and is protected under U.S. and international copyright
// and other intellectual property laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// AMD, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND AMD HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) AMD shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or AMD had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// AMD products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of AMD products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
//----------------------------------------------------------------------------
// User entered comments
//----------------------------------------------------------------------------
// None
//
//----------------------------------------------------------------------------
//  Output     Output      Phase    Duty Cycle   Pk-to-Pk     Phase
//   Clock     Freq (MHz)  (degrees)    (%)     Jitter (ps)  Error (ps)
//----------------------------------------------------------------------------
// _AXI_clk__200.00000______0.000______50.0_______83.559_____73.186
// PCLK_clk__100.00000_____30.000______50.0_______95.390_____73.186
// nCLKEN_clk__50.00000_____60.000______50.0______108.931_____73.186
// BCLK_clk__25.00000_____39.750______50.0______124.423_____73.186
// CLK90_clk__25.00000____270.000______50.0______124.423_____73.186
// CPUCLK_clk__25.00000____180.000______50.0______124.423_____73.186
//
//----------------------------------------------------------------------------
// Input Clock   Freq (MHz)    Input Jitter (UI)
//----------------------------------------------------------------------------
// __primary_________200.000____________0.010

`timescale 1ps/1ps

module design_1_clk_wiz_0_0_clk_wiz 

 (// Clock in ports
  // Clock out ports
  output        AXI_clk,
  output        PCLK_clk,
  output        nCLKEN_clk,
  output        BCLK_clk,
  output        CLK90_clk,
  output        CPUCLK_clk,
  // Dynamic reconfiguration ports
  input   [6:0] daddr,
  input         dclk,
  input         den,
  input  [15:0] din,
  output [15:0] dout,
  output        drdy,
  input         dwe,
  // Status and control signals
  input         reset,
  output        locked,
  input         clk_in1
 );
  // Input buffering
  //------------------------------------
wire clk_in1_design_1_clk_wiz_0_0;
wire clk_in2_design_1_clk_wiz_0_0;
  BUFG clkin1_bufg
   (.O (clk_in1_design_1_clk_wiz_0_0),
    .I (clk_in1));




  // Clocking PRIMITIVE
  //------------------------------------

  // Instantiation of the MMCM PRIMITIVE
  //    * Unused inputs are tied off
  //    * Unused outputs are labeled unused

  wire        AXI_clk_design_1_clk_wiz_0_0;
  wire        PCLK_clk_design_1_clk_wiz_0_0;
  wire        nCLKEN_clk_design_1_clk_wiz_0_0;
  wire        BCLK_clk_design_1_clk_wiz_0_0;
  wire        CLK90_clk_design_1_clk_wiz_0_0;
  wire        CPUCLK_clk_design_1_clk_wiz_0_0;
  wire        clk_out7_design_1_clk_wiz_0_0;

  wire        psdone_unused;
  wire        locked_int;
  wire        clkfbout_design_1_clk_wiz_0_0;
  wire        clkfbout_buf_design_1_clk_wiz_0_0;
  wire        clkfboutb_unused;
    wire clkout0b_unused;
   wire clkout1b_unused;
   wire clkout2b_unused;
   wire clkout3b_unused;
  wire        clkout6_unused;
  wire        clkfbstopped_unused;
  wire        clkinstopped_unused;
  wire        reset_high;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg1 = 0;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg2 = 0;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg3 = 0;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg4 = 0;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg5 = 0;
  (* KEEP = "TRUE" *) 
  (* ASYNC_REG = "TRUE" *)
  reg  [7 :0] seq_reg6 = 0;

  MMCME2_ADV
  #(.BANDWIDTH            ("HIGH"),
    .CLKOUT4_CASCADE      ("FALSE"),
    .COMPENSATION         ("ZHOLD"),
    .STARTUP_WAIT         ("FALSE"),
    .DIVCLK_DIVIDE        (1),
    .CLKFBOUT_MULT_F      (7.500),
    .CLKFBOUT_PHASE       (0.000),
    .CLKFBOUT_USE_FINE_PS ("FALSE"),
    .CLKOUT0_DIVIDE_F     (7.500),
    .CLKOUT0_PHASE        (0.000),
    .CLKOUT0_DUTY_CYCLE   (0.500),
    .CLKOUT0_USE_FINE_PS  ("FALSE"),
    .CLKOUT1_DIVIDE       (15),
    .CLKOUT1_PHASE        (30.000),
    .CLKOUT1_DUTY_CYCLE   (0.500),
    .CLKOUT1_USE_FINE_PS  ("FALSE"),
    .CLKOUT2_DIVIDE       (30),
    .CLKOUT2_PHASE        (60.000),
    .CLKOUT2_DUTY_CYCLE   (0.500),
    .CLKOUT2_USE_FINE_PS  ("FALSE"),
    .CLKOUT3_DIVIDE       (60),
    .CLKOUT3_PHASE        (39.750),
    .CLKOUT3_DUTY_CYCLE   (0.500),
    .CLKOUT3_USE_FINE_PS  ("FALSE"),
    .CLKOUT4_DIVIDE       (60),
    .CLKOUT4_PHASE        (270.000),
    .CLKOUT4_DUTY_CYCLE   (0.500),
    .CLKOUT4_USE_FINE_PS  ("FALSE"),
    .CLKOUT5_DIVIDE       (60),
    .CLKOUT5_PHASE        (180.000),
    .CLKOUT5_DUTY_CYCLE   (0.500),
    .CLKOUT5_USE_FINE_PS  ("FALSE"),
    .CLKIN1_PERIOD        (5.000))
  mmcm_adv_inst
    // Output clocks
   (
    .CLKFBOUT            (clkfbout_design_1_clk_wiz_0_0),
    .CLKFBOUTB           (clkfboutb_unused),
    .CLKOUT0             (AXI_clk_design_1_clk_wiz_0_0),
    .CLKOUT0B            (clkout0b_unused),
    .CLKOUT1             (PCLK_clk_design_1_clk_wiz_0_0),
    .CLKOUT1B            (clkout1b_unused),
    .CLKOUT2             (nCLKEN_clk_design_1_clk_wiz_0_0),
    .CLKOUT2B            (clkout2b_unused),
    .CLKOUT3             (BCLK_clk_design_1_clk_wiz_0_0),
    .CLKOUT3B            (clkout3b_unused),
    .CLKOUT4             (CLK90_clk_design_1_clk_wiz_0_0),
    .CLKOUT5             (CPUCLK_clk_design_1_clk_wiz_0_0),
    .CLKOUT6             (clkout6_unused),
     // Input clock control
    .CLKFBIN             (clkfbout_buf_design_1_clk_wiz_0_0),
    .CLKIN1              (clk_in1_design_1_clk_wiz_0_0),
    .CLKIN2              (1'b0),
     // Tied to always select the primary input clock
    .CLKINSEL            (1'b1),
    // Ports for dynamic reconfiguration
    .DADDR               (daddr),
    .DCLK                (dclk),
    .DEN                 (den),
    .DI                  (din),
    .DO                  (dout),
    .DRDY                (drdy),
    .DWE                 (dwe),
    // Ports for dynamic phase shift
    .PSCLK               (1'b0),
    .PSEN                (1'b0),
    .PSINCDEC            (1'b0),
    .PSDONE              (psdone_unused),
    // Other control and status signals
    .LOCKED              (locked_int),
    .CLKINSTOPPED        (clkinstopped_unused),
    .CLKFBSTOPPED        (clkfbstopped_unused),
    .PWRDWN              (1'b0),
    .RST                 (reset_high));
  assign reset_high = reset; 

  assign locked = locked_int;
// Clock Monitor clock assigning
//--------------------------------------
 // Output buffering
  //-----------------------------------

  BUFG clkf_buf
   (.O (clkfbout_buf_design_1_clk_wiz_0_0),
    .I (clkfbout_design_1_clk_wiz_0_0));







  BUFGCE clkout1_buf
   (.O   (AXI_clk),
    .CE  (seq_reg1[7]),
    .I   (AXI_clk_design_1_clk_wiz_0_0));

  BUFH clkout1_buf_en
   (.O   (AXI_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (AXI_clk_design_1_clk_wiz_0_0));
  always @(posedge AXI_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	    seq_reg1 <= 8'h00;
    end
    else begin
        seq_reg1 <= {seq_reg1[6:0],locked_int};
  
    end
  end


  BUFGCE clkout2_buf
   (.O   (PCLK_clk),
    .CE  (seq_reg2[7]),
    .I   (PCLK_clk_design_1_clk_wiz_0_0));
 
  BUFH clkout2_buf_en
   (.O   (PCLK_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (PCLK_clk_design_1_clk_wiz_0_0));
 
  always @(posedge PCLK_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	  seq_reg2 <= 8'h00;
    end
    else begin
        seq_reg2 <= {seq_reg2[6:0],locked_int};
  
    end
  end


  BUFGCE clkout3_buf
   (.O   (nCLKEN_clk),
    .CE  (seq_reg3[7]),
    .I   (nCLKEN_clk_design_1_clk_wiz_0_0));
 
  BUFH clkout3_buf_en
   (.O   (nCLKEN_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (nCLKEN_clk_design_1_clk_wiz_0_0));
 
  always @(posedge nCLKEN_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	  seq_reg3 <= 8'h00;
    end
    else begin
        seq_reg3 <= {seq_reg3[6:0],locked_int};
  
    end
  end


  BUFGCE clkout4_buf
   (.O   (BCLK_clk),
    .CE  (seq_reg4[7]),
    .I   (BCLK_clk_design_1_clk_wiz_0_0));

  BUFH clkout4_buf_en
   (.O   (BCLK_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (BCLK_clk_design_1_clk_wiz_0_0));
	
  always @(posedge BCLK_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	  seq_reg4 <= 8'h00;
    end
    else begin
        seq_reg4 <= {seq_reg4[6:0],locked_int};
  
    end
  end


  BUFGCE clkout5_buf
   (.O   (CLK90_clk),
    .CE  (seq_reg5[7]),
    .I   (CLK90_clk_design_1_clk_wiz_0_0));
 
  BUFH clkout5_buf_en
   (.O   (CLK90_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (CLK90_clk_design_1_clk_wiz_0_0));
 
  always @(posedge CLK90_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	  seq_reg5 <= 8'h00;
    end
    else begin
        seq_reg5 <= {seq_reg5[6:0],locked_int};
  
    end
  end


  BUFGCE clkout6_buf
   (.O   (CPUCLK_clk),
    .CE  (seq_reg6[7]),
    .I   (CPUCLK_clk_design_1_clk_wiz_0_0));
 
  BUFH clkout6_buf_en
   (.O   (CPUCLK_clk_design_1_clk_wiz_0_0_en_clk),
    .I   (CPUCLK_clk_design_1_clk_wiz_0_0));
 
  always @(posedge CPUCLK_clk_design_1_clk_wiz_0_0_en_clk or posedge reset_high) begin
    if(reset_high == 1'b1) begin
	  seq_reg6 <= 8'h00;
    end
    else begin
        seq_reg6 <= {seq_reg6[6:0],locked_int};
  
    end
  end





endmodule
