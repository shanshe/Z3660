// (c) Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// (c) Copyright 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
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
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: xilinx.com:module_ref:cpuclk_iob:1.0
// IP Revision: 1

(* X_CORE_INFO = "cpuclk_iob,Vivado 2023.2" *)
(* CHECK_LICENSE_TYPE = "design_1_cpuclk_iob_0_0,cpuclk_iob,{}" *)
(* CORE_GENERATION_INFO = "design_1_cpuclk_iob_0_0,cpuclk_iob,{x_ipProduct=Vivado 2023.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=cpuclk_iob,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED}" *)
(* IP_DEFINITION_SOURCE = "module_ref" *)
(* DowngradeIPIdentifiedWarnings = "yes" *)
module design_1_cpuclk_iob_0_0 (
  fast_clk,
  CPUCLK_pin,
  CPUCLK_in,
  CLK90_pin,
  CLK90_in,
  enable_output,
  cpuclk_detected,
  clk90_detected
);

(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME fast_clk, FREQ_HZ 200000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN /processing_av_system/clock_generation/clk_wiz_0_clk_out1, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 fast_clk CLK" *)
input wire fast_clk;
inout wire CPUCLK_pin;
input wire CPUCLK_in;
inout wire CLK90_pin;
input wire CLK90_in;
input wire enable_output;
output wire cpuclk_detected;
output wire clk90_detected;

  cpuclk_iob inst (
    .fast_clk(fast_clk),
    .CPUCLK_pin(CPUCLK_pin),
    .CPUCLK_in(CPUCLK_in),
    .CLK90_pin(CLK90_pin),
    .CLK90_in(CLK90_in),
    .enable_output(enable_output),
    .cpuclk_detected(cpuclk_detected),
    .clk90_detected(clk90_detected)
  );
endmodule
