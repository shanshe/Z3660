// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps

module audio_formatter_v1_0_11 #(
	parameter C_FAMILY = "virtex7",

	parameter integer C_INCLUDE_S2MM = 1 , //0,1
	parameter integer C_MAX_NUM_CHANNELS_S2MM = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_S2MM = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_S2MM_DATAFORMAT = 1, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
	parameter integer C_INCLUDE_MM2S = 1 , //0,1
	parameter integer C_MAX_NUM_CHANNELS_MM2S = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_MM2S = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_MM2S_DATAFORMAT = 3, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
						 // 3: PCM -> AES
	parameter integer C_S2MM_ADDR_WIDTH = 64, //32,64
	parameter integer C_MM2S_ADDR_WIDTH = 64, //32,64
	parameter integer C_S2MM_ASYNC_CLOCK = 1, //0, 1
	parameter integer C_MM2S_ASYNC_CLOCK = 1  //0, 1
) (
// AXI4-Lite Interface Signals
	input 		s_axi_lite_aclk,
	input 		s_axi_lite_aresetn,

	input  		s_axi_lite_awvalid, 
	output 		s_axi_lite_awready, 
	input  	[11:0]	s_axi_lite_awaddr , 

	input  		s_axi_lite_wvalid , 
	output 		s_axi_lite_wready , 
	input  	[31:0]	s_axi_lite_wdata  , 
	                    
	output  [1:0] 	s_axi_lite_bresp  , 
	output   	s_axi_lite_bvalid , 
	input 		s_axi_lite_bready , 
	                    
	input 		s_axi_lite_arvalid, 
	output		s_axi_lite_arready, 
	input 	[11:0]	s_axi_lite_araddr , 
	                    
	output 		s_axi_lite_rvalid , 
	input 		s_axi_lite_rready , 
	output	[31:0]	s_axi_lite_rdata  , 
	                    
	output  [1:0] 	s_axi_lite_rresp  , 

//------MM2S-----------------
	input 		m_axis_mm2s_aclk,
	input 		m_axis_mm2s_aresetn,

	input 		aud_mclk,
	input 		aud_mreset,

	output 		irq_mm2s,

// AXIS MM2S Channel                                                             
	output 		m_axis_mm2s_tvalid,
	input 		m_axis_mm2s_tready,
	output	[31:0]	m_axis_mm2s_tdata,
	output 	[7:0]	m_axis_mm2s_tid,


//AXI MM2S Channel
        output 	[C_MM2S_ADDR_WIDTH-1:0]	m_axi_mm2s_araddr,
         		                
        output 	[7:0]	m_axi_mm2s_arlen,
        output 	[2:0]	m_axi_mm2s_arsize,
        output 	[1:0]	m_axi_mm2s_arburst,
        output 	[2:0]	m_axi_mm2s_arprot,
        output 	[3:0]	m_axi_mm2s_arcache,
        output 	[3:0]	m_axi_mm2s_aruser,
        output 		m_axi_mm2s_arvalid,
        input 		m_axi_mm2s_arready,
         		                       
        input 	[31:0]	m_axi_mm2s_rdata,
         		                       
        input 	[1:0]	m_axi_mm2s_rresp,
        input 		m_axi_mm2s_rlast       ,
        input 		m_axi_mm2s_rvalid,
        output 		m_axi_mm2s_rready,
         		                       
//------S2MM-----------------
	input 		s_axis_s2mm_aclk,
	input 		s_axis_s2mm_aresetn,
	output 		irq_s2mm,
                         
// AXIS S2MM Channel                                                             
	input 		s_axis_s2mm_tvalid,
	output 		s_axis_s2mm_tready,
	input	[31:0]	s_axis_s2mm_tdata,
	input 	[7:0]	s_axis_s2mm_tid,

//AXI S2MM Channel
        output 	[C_S2MM_ADDR_WIDTH-1:0]	m_axi_s2mm_awaddr,
         		                
        output 	[7:0]	m_axi_s2mm_awlen,
        output 	[2:0]	m_axi_s2mm_awsize,
        output 	[1:0]	m_axi_s2mm_awburst,
        output 	[2:0]	m_axi_s2mm_awprot,
        output 	[3:0]	m_axi_s2mm_awcache,
        output 	[3:0]	m_axi_s2mm_awuser,
        output 		m_axi_s2mm_awvalid,
        input 		m_axi_s2mm_awready,
         		                       
        output 	[31:0]	m_axi_s2mm_wdata,
        output 	[3:0]	m_axi_s2mm_wstrb,
         		                       
        output 		m_axi_s2mm_wlast       ,
        output 		m_axi_s2mm_wvalid,
        input 		m_axi_s2mm_wready,

        input 	[1:0]	m_axi_s2mm_bresp,
        input 		m_axi_s2mm_bvalid,
        output 		m_axi_s2mm_bready

);

//-- AXILite Write path-----------------------
wire s_axi_lite_awvalid_s2mm;
wire s_axi_lite_awready_s2mm;
wire s_axi_lite_wvalid_s2mm;
wire s_axi_lite_wready_s2mm;
wire s_axi_lite_bvalid_s2mm;
wire [1:0] s_axi_lite_bresp_s2mm;
wire s_axi_lite_bready_s2mm;

wire s_axi_lite_awvalid_mm2s;
wire s_axi_lite_awready_mm2s;
wire s_axi_lite_wvalid_mm2s;
wire s_axi_lite_wready_mm2s;
wire s_axi_lite_bvalid_mm2s;
wire [1:0] s_axi_lite_bresp_mm2s;
wire s_axi_lite_bready_mm2s;

generate 
if((C_INCLUDE_S2MM == 1) && (C_INCLUDE_MM2S == 0)) begin : ONLY_S2MM

assign s_axi_lite_awvalid_s2mm = s_axi_lite_awvalid;
assign s_axi_lite_wvalid_s2mm  = s_axi_lite_wvalid;
assign s_axi_lite_bvalid  = s_axi_lite_bvalid_s2mm;
assign s_axi_lite_bresp  = s_axi_lite_bresp_s2mm;

assign s_axi_lite_awready = s_axi_lite_awready_s2mm;
assign s_axi_lite_wready  = s_axi_lite_wready_s2mm;
assign s_axi_lite_bready_s2mm  = s_axi_lite_bready;

end
else if((C_INCLUDE_S2MM == 0) && (C_INCLUDE_MM2S == 1)) begin : ONLY_MM2S

assign s_axi_lite_awvalid_mm2s = s_axi_lite_awvalid;
assign s_axi_lite_wvalid_mm2s  = s_axi_lite_wvalid;
assign s_axi_lite_bvalid  = s_axi_lite_bvalid_mm2s;
assign s_axi_lite_bresp   = s_axi_lite_bresp_mm2s;

assign s_axi_lite_awready = s_axi_lite_awready_mm2s;
assign s_axi_lite_wready  = s_axi_lite_wready_mm2s;
assign s_axi_lite_bready_mm2s  = s_axi_lite_bready;

end
else begin : BOTH_ENABLED

reg transaction_s2mm;
reg transaction_mm2s;

always@(posedge s_axi_lite_aclk)
begin
	if(!s_axi_lite_aresetn) begin
		transaction_s2mm <= 1'b0;
	end
	else if ((s_axi_lite_awaddr[8] == 1'b0) && s_axi_lite_awvalid && (!transaction_mm2s) ) begin
		transaction_s2mm <= 1'b1;
	end
	else if (s_axi_lite_bvalid && s_axi_lite_bready) begin
		transaction_s2mm <= 1'b0;
	end
end
always@(posedge s_axi_lite_aclk)
begin
	if(!s_axi_lite_aresetn) begin
		transaction_mm2s <= 1'b0;
	end
	else if ((s_axi_lite_awaddr[8] == 1'b1) && s_axi_lite_awvalid && (!transaction_s2mm) ) begin
		transaction_mm2s <= 1'b1;
	end
	else if (s_axi_lite_bvalid && s_axi_lite_bready) begin
		transaction_mm2s <= 1'b0;
	end
end

assign s_axi_lite_awvalid_mm2s = (transaction_mm2s && s_axi_lite_awaddr[8]) ? s_axi_lite_awvalid : 1'b0;
assign s_axi_lite_wvalid_mm2s  = (transaction_mm2s) ? s_axi_lite_wvalid : 1'b0;
assign s_axi_lite_bready_mm2s = (transaction_mm2s) ? s_axi_lite_bready : 1'b0;

assign s_axi_lite_awvalid_s2mm = (transaction_s2mm && (s_axi_lite_awaddr[8] == 'b0)) ? s_axi_lite_awvalid : 1'b0;
assign s_axi_lite_wvalid_s2mm  = (transaction_s2mm) ? s_axi_lite_wvalid : 1'b0;
assign s_axi_lite_bready_s2mm = (transaction_s2mm) ? s_axi_lite_bready : 1'b0;

assign s_axi_lite_awready =(transaction_mm2s && s_axi_lite_awready_mm2s) || (transaction_s2mm && s_axi_lite_awready_s2mm);
assign s_axi_lite_wready =(transaction_mm2s && s_axi_lite_wready_mm2s) || (transaction_s2mm && s_axi_lite_wready_s2mm);
assign s_axi_lite_bvalid  = (transaction_mm2s && s_axi_lite_bvalid_mm2s) || (transaction_s2mm && s_axi_lite_bvalid_s2mm);
assign s_axi_lite_bresp = transaction_s2mm ? s_axi_lite_bresp_s2mm : (transaction_mm2s ? s_axi_lite_bresp_mm2s : 'd0);

end
endgenerate


//-- AXILite Read path-----------------------
localparam cAXI4_RESP_OKAY   = 2'b00; // Okay
localparam cAXI4_RESP_SLVERR = 2'b10; // Slave error
localparam sReadReset = 2'd0;
localparam sReadAddr = 2'd1;
localparam sDecodeAddr = 2'd2;
localparam sReadData = 2'd3;
             
reg [1:0] stmRead;

reg        ReadAddrNOK;
reg [ 11:0] rReadAddr;
reg [31:0] nReadData;

  wire        iAxi_ARValid;
  reg         oAxi_ARReady;
  wire [ 11:0] iAxi_ARAddr;
  reg         oAxi_RValid;
  wire        iAxi_RReady; 
  reg  [31:0] oAxi_RData;
  reg  [ 1:0] oAxi_RResp;

wire [31:0] read_data_s2mm;
wire [31:0] read_data_mm2s;
wire NOK_s2mm;
wire NOK_mm2s;
wire read_data_accepted;
wire s2mm_read_trans;
wire mm2s_read_trans;

assign read_data_accepted = s_axi_lite_rvalid && s_axi_lite_rready;
assign s2mm_read_trans = (rReadAddr[11:8] == 4'd0) && read_data_accepted;
assign mm2s_read_trans = (rReadAddr[11:8] == 4'd1) && read_data_accepted;

assign s_axi_lite_arready = oAxi_ARReady;
assign s_axi_lite_rvalid = oAxi_RValid;
assign s_axi_lite_rdata = oAxi_RData;
assign s_axi_lite_rresp = oAxi_RResp;

assign iAxi_ARValid = s_axi_lite_arvalid;
assign iAxi_ARAddr = s_axi_lite_araddr;
assign iAxi_RReady = s_axi_lite_rready;

// Statemachine for taking care of the read signals
always @(posedge s_axi_lite_aclk)
begin
  if (!s_axi_lite_aresetn)
  begin
    oAxi_ARReady        <= 1'b0;    
    oAxi_RResp          <= cAXI4_RESP_OKAY;
    oAxi_RValid         <= 1'b0;
    oAxi_RData          <=  'h0;
    rReadAddr           <=  'h0;
    stmRead             <= sReadReset;
  end
  else
  begin
    case (stmRead) 
      sReadReset :
      begin
        oAxi_ARReady    <= 1'b1;
        oAxi_RResp      <= cAXI4_RESP_OKAY;
        oAxi_RValid     <= 1'b0;
        oAxi_RData      <=  'h0;
        rReadAddr       <=  'h0;
        stmRead         <= sReadAddr;
      end
      
      sReadAddr :
      begin
        oAxi_ARReady    <= 1'b1;
        if (iAxi_ARValid)
        begin
          oAxi_ARReady  <= 1'b0;
          rReadAddr     <= iAxi_ARAddr;
          stmRead       <= sDecodeAddr;
        end
      end
      
      sDecodeAddr :
      begin
        if (ReadAddrNOK)
          oAxi_RResp    <= cAXI4_RESP_SLVERR;
        else
          oAxi_RResp    <= cAXI4_RESP_OKAY;
          
        oAxi_RData      <= nReadData;
        oAxi_RValid     <= 1'b1;
        stmRead         <= sReadData;
      end
      
      sReadData :
      begin
        oAxi_RValid     <= 1'b1;
        if (iAxi_RReady)
        begin
          oAxi_RValid   <= 1'b0;
          stmRead       <= sReadReset;
        end
      end
      default :
        stmRead         <= sReadReset;
    endcase
  end
end

localparam cADDR_VERSION = 12'h000;
localparam cADDR_CONFIG  = 12'h004;
wire [7:0] major_revision = 8'd1;
wire [7:0] minor_revision = 8'd0;
wire [7:0] revision = 8'd0;

wire [31:0] core_version_reg = {
major_revision,
minor_revision,
revision,
8'd0
};

wire [31:0] core_config_reg = {
C_INCLUDE_S2MM[0],
C_S2MM_DATAFORMAT[1:0],
C_PACKING_MODE_S2MM[0],
C_MAX_NUM_CHANNELS_S2MM[3:0],
8'd0,
C_INCLUDE_MM2S[0],
C_MM2S_DATAFORMAT[1:0],
C_PACKING_MODE_MM2S[0],
C_MAX_NUM_CHANNELS_MM2S[3:0],
8'd0
};

always@(*)
begin
    if(rReadAddr == cADDR_VERSION)
    begin
      nReadData   	= core_version_reg;
      ReadAddrNOK       = 1'b0;
    end
    else if(rReadAddr == cADDR_CONFIG)
    begin
      nReadData		= core_config_reg;
      ReadAddrNOK       = 1'b0;
    end
    else if(rReadAddr[11:8] == 4'd0)
    begin
      nReadData		= read_data_s2mm;
      ReadAddrNOK       = NOK_s2mm;
    end
    else if(rReadAddr[11:8] == 4'd1)
    begin
      nReadData		= read_data_mm2s;
      ReadAddrNOK       = NOK_mm2s;
    end
    else 
    begin
      nReadData		= 'd0;
      ReadAddrNOK       = 1'b1;
    end
end

generate 
if(C_INCLUDE_S2MM) begin: S2MM_INCLUDED
wire s2mm_lite_resetn_sync;

   xpm_cdc_async_rst #(
       .DEST_SYNC_FF(2),
       .INIT_SYNC_FF(0),
       .RST_ACTIVE_HIGH(0)
    )
    xpm_cdc_async_s2mm_rst_inst (
       .dest_arst(s2mm_lite_resetn_sync),
       .dest_clk(s_axis_s2mm_aclk),
       .src_arst(s_axi_lite_aresetn)
    );
  

audio_formatter_v1_0_11_s2mm_top #(
 .C_FAMILY(C_FAMILY),
 .C_MAX_NUM_CHANNELS_S2MM(C_MAX_NUM_CHANNELS_S2MM),
 .C_PACKING_MODE_S2MM(C_PACKING_MODE_S2MM),
 .C_S2MM_DATAFORMAT(C_S2MM_DATAFORMAT),
 .C_S2MM_ADDR_WIDTH(C_S2MM_ADDR_WIDTH),
 .C_S2MM_ASYNC_CLOCK(C_S2MM_ASYNC_CLOCK)
) s2mm_top_1 
(
	.s_axi_lite_aclk(s_axi_lite_aclk),
	.s_axi_lite_aresetn(s_axi_lite_aresetn),

	.s_axi_lite_awvalid(s_axi_lite_awvalid_s2mm), 
	.s_axi_lite_awready(s_axi_lite_awready_s2mm), 
	.s_axi_lite_awaddr (s_axi_lite_awaddr ), 

	.s_axi_lite_wvalid (s_axi_lite_wvalid_s2mm ),
	.s_axi_lite_wready (s_axi_lite_wready_s2mm ),
	.s_axi_lite_wdata  (s_axi_lite_wdata  ),
	    
	.s_axi_lite_bresp  (s_axi_lite_bresp_s2mm  ),
	.s_axi_lite_bvalid (s_axi_lite_bvalid_s2mm ),
	.s_axi_lite_bready (s_axi_lite_bready_s2mm ),
	    
/*	.s_axi_lite_arvalid(s_axi_lite_arvalid),
	.s_axi_lite_arready(s_axi_lite_arready),
	.s_axi_lite_araddr (s_axi_lite_araddr ),
	    
	.s_axi_lite_rvalid (s_axi_lite_rvalid ),
	.s_axi_lite_rready (s_axi_lite_rready ),
	.s_axi_lite_rdata  (s_axi_lite_rdata  ),
	    
	.s_axi_lite_rresp  (s_axi_lite_rresp  ),*/

	.s_axi_lite_araddr (rReadAddr[7:0]    ),
	.s_axi_lite_rdata  (read_data_s2mm    ),
	.s_axi_lite_rNOK   (NOK_s2mm          ),
	.read_trans	   (s2mm_read_trans),

	.s_axis_s2mm_aclk(s_axis_s2mm_aclk),
	.s_axis_s2mm_aresetn(s_axis_s2mm_aresetn && s2mm_lite_resetn_sync),
	.Irq_s2mm(irq_s2mm),
                         
	.s_axis_s2mm_tvalid(s_axis_s2mm_tvalid),
	.s_axis_s2mm_tready(s_axis_s2mm_tready),
	.s_axis_s2mm_tdata(s_axis_s2mm_tdata),
	.s_axis_s2mm_tid(s_axis_s2mm_tid),

        .m_axi_s2mm_awaddr(m_axi_s2mm_awaddr),
         		                
       .m_axi_s2mm_awlen(m_axi_s2mm_awlen),
       .m_axi_s2mm_awsize(m_axi_s2mm_awsize),
        .m_axi_s2mm_awburst(m_axi_s2mm_awburst),
        .m_axi_s2mm_awprot(m_axi_s2mm_awprot),
        .m_axi_s2mm_awcache(m_axi_s2mm_awcache),
        .m_axi_s2mm_awuser(m_axi_s2mm_awuser),
        .m_axi_s2mm_awvalid(m_axi_s2mm_awvalid),
        .m_axi_s2mm_awready(m_axi_s2mm_awready),
                               
        .m_axi_s2mm_wdata(m_axi_s2mm_wdata),
        .m_axi_s2mm_wstrb(m_axi_s2mm_wstrb),
                               
        .m_axi_s2mm_wlast(m_axi_s2mm_wlast),
        .m_axi_s2mm_wvalid(m_axi_s2mm_wvalid),
        .m_axi_s2mm_wready(m_axi_s2mm_wready),

        .m_axi_s2mm_bresp(m_axi_s2mm_bresp),
        .m_axi_s2mm_bvalid(m_axi_s2mm_bvalid),
        .m_axi_s2mm_bready(m_axi_s2mm_bready)
);
end
else begin: S2MM_NOT_INCLUDED

assign s_axi_lite_awready_s2mm = 1'b0;
assign s_axi_lite_wready_s2mm  = 1'b0;
assign s_axi_lite_bvalid_s2mm  = 1'b0;

assign NOK_s2mm = NOK_mm2s;
assign read_data_s2mm = 32'd0;

assign irq_s2mm = 1'b0;
assign s_axis_s2mm_tready = 1'b0;

assign m_axi_s2mm_awvalid = 1'b0;
assign m_axi_s2mm_wvalid = 1'b0;
assign m_axi_s2mm_bready = 1'b0;

end
endgenerate

generate 
if(C_INCLUDE_MM2S) begin: MM2S_INCLUDED

wire mm2s_lite_resetn_sync;

   xpm_cdc_async_rst #(
       .DEST_SYNC_FF(2),
       .INIT_SYNC_FF(0),
       .RST_ACTIVE_HIGH(0)
    )
    xpm_cdc_async_mm2s_rst_inst (
       .dest_arst(mm2s_lite_resetn_sync),
       .dest_clk(m_axis_mm2s_aclk),
       .src_arst(s_axi_lite_aresetn)
    );
  

audio_formatter_v1_0_11_mm2s_top #(
 .C_FAMILY(C_FAMILY),
 .C_MAX_NUM_CHANNELS_MM2S(C_MAX_NUM_CHANNELS_MM2S),
 .C_PACKING_MODE_MM2S(C_PACKING_MODE_MM2S),
 .C_MM2S_DATAFORMAT(C_MM2S_DATAFORMAT),
 .C_MM2S_ADDR_WIDTH(C_MM2S_ADDR_WIDTH),
 .C_MM2S_ASYNC_CLOCK(C_MM2S_ASYNC_CLOCK)
) mm2s_top_1 
(
	.aud_clk(aud_mclk),
	.aud_resetn(~aud_mreset),

	.s_axi_lite_aclk(s_axi_lite_aclk),
	.s_axi_lite_aresetn(s_axi_lite_aresetn),

	.s_axi_lite_awvalid(s_axi_lite_awvalid_mm2s), 
	.s_axi_lite_awready(s_axi_lite_awready_mm2s), 
	.s_axi_lite_awaddr (s_axi_lite_awaddr ), 

	.s_axi_lite_wvalid (s_axi_lite_wvalid_mm2s ),
	.s_axi_lite_wready (s_axi_lite_wready_mm2s ),
	.s_axi_lite_wdata  (s_axi_lite_wdata  ),
	    
	.s_axi_lite_bresp  (s_axi_lite_bresp_mm2s  ),
	.s_axi_lite_bvalid (s_axi_lite_bvalid_mm2s ),
	.s_axi_lite_bready (s_axi_lite_bready_mm2s ),
/*	    
	.s_axi_lite_arvalid(s_axi_lite_arvalid),
	.s_axi_lite_arready(s_axi_lite_arready),
	.s_axi_lite_araddr (s_axi_lite_araddr ),
	    
	.s_axi_lite_rvalid (s_axi_lite_rvalid ),
	.s_axi_lite_rready (s_axi_lite_rready ),
	.s_axi_lite_rdata  (s_axi_lite_rdata  ),
	    
	.s_axi_lite_rresp  (s_axi_lite_rresp  ),
*/
	.read_trans	   (mm2s_read_trans),
	.s_axi_lite_araddr (rReadAddr[7:0]    ),
	.s_axi_lite_rdata  (read_data_mm2s    ),
	.s_axi_lite_rNOK   (NOK_mm2s          ),

      .m_axis_mm2s_aclk(m_axis_mm2s_aclk),
      .m_axis_mm2s_aresetn(m_axis_mm2s_aresetn && mm2s_lite_resetn_sync),
      .Irq_mm2s(irq_mm2s),

      .m_axis_mm2s_tdata           (m_axis_mm2s_tdata        ),
      .m_axis_mm2s_tid             (m_axis_mm2s_tid        ),
      .m_axis_mm2s_tvalid          (m_axis_mm2s_tvalid       ),
      .m_axis_mm2s_tready          (m_axis_mm2s_tready       ),

      .m_axi_mm2s_araddr           (m_axi_mm2s_araddr              ),
      .m_axi_mm2s_arlen            (m_axi_mm2s_arlen               ),
      .m_axi_mm2s_arsize           (m_axi_mm2s_arsize              ),
      .m_axi_mm2s_arburst          (m_axi_mm2s_arburst             ),
      .m_axi_mm2s_arprot           (m_axi_mm2s_arprot              ),
      .m_axi_mm2s_arcache          (m_axi_mm2s_arcache             ),
      .m_axi_mm2s_aruser           (m_axi_mm2s_aruser             ),
      .m_axi_mm2s_arvalid          (m_axi_mm2s_arvalid             ),
      .m_axi_mm2s_arready          (m_axi_mm2s_arready             ),

      .m_axi_mm2s_rdata            (m_axi_mm2s_rdata               ),
      .m_axi_mm2s_rresp            (m_axi_mm2s_rresp               ),
      .m_axi_mm2s_rlast            (m_axi_mm2s_rlast               ),
      .m_axi_mm2s_rvalid           (m_axi_mm2s_rvalid              ),
      .m_axi_mm2s_rready           (m_axi_mm2s_rready              )
);

end
else begin: MM2S_NOT_INCLUDED

assign s_axi_lite_awready_mm2s = 1'b0;
assign s_axi_lite_wready_mm2s  = 1'b0;
assign s_axi_lite_bvalid_mm2s  = 1'b0;
assign NOK_mm2s = NOK_s2mm;
assign read_data_mm2s = 32'd0;

assign irq_mm2s = 1'b0;
assign m_axis_mm2s_tvalid = 1'b0;

assign m_axi_mm2s_arvalid = 1'b0;
assign m_axi_mm2s_rready = 1'b0;

end
endgenerate





endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps

module audio_formatter_v1_0_11_reset #(
	parameter C_ASYNC_CLOCK = 1
) 
(
input lite_clk,
input lite_resetn,

input axi4_clk,
input axi4_resetn,

input start_dma_lite,
input soft_reset_lite,
input halt_complete_dm,

output soft_reset_clear,
output reg soft_reset_core,
output reg soft_reset_proc,
output halt_dm,
output reg halted
);

reg soft_reset_clr;
reg start_dma_lite_r;
reg soft_reset_lite_r;
wire start_soft_reset;
wire stop_dma;
reg start_halt;
wire soft_reset;
reg soft_reset_done;
wire soft_reset_done_axi;
wire reset_ready_lite;
wire reset_ready;
wire halt_complete;
reg soft_reset_proc_r;
reg soft_reset_core_d1;
reg soft_reset_core_d2;
reg soft_reset_core_d3;
reg soft_reset_core_d4;
reg soft_reset_core_d5;

wire soft_reset_proc_done;
wire soft_reset_proc_done_lite;

//assign halted = halt_complete;
assign soft_reset_clear = soft_reset_clr && soft_reset_lite;

always@(posedge lite_clk)
begin
	if(~lite_resetn ) begin
		halted <= 1'b0;
	end
	else if(start_soft_reset || stop_dma) begin
		halted <= 1'b1;
	end
	else if(reset_ready_lite) begin
		halted <= 1'b0;
	end
end

always@(posedge lite_clk)
begin
	if(~lite_resetn || soft_reset_clear) begin
		start_dma_lite_r <= 1'b0;
	end
	else begin
		start_dma_lite_r <= start_dma_lite;
	end
end
always@(posedge lite_clk)
begin
	if(~lite_resetn) begin
		soft_reset_lite_r <= 1'b0;
	end
	else begin
		soft_reset_lite_r <= soft_reset_lite;
	end
end

assign start_soft_reset = (~soft_reset_lite_r) && soft_reset_lite;
assign stop_dma = (~start_dma_lite) && start_dma_lite_r;

always@(posedge lite_clk)
begin
	if(~lite_resetn) begin
		start_halt <= 1'b0;
	end
	else if((stop_dma || start_soft_reset) && (~halt_complete)) begin
		start_halt <= 1'b1;
	end
	else begin
		start_halt <= 1'b0;
	end
end


generate if (C_ASYNC_CLOCK == 1)
begin: ASYNC_CLOCKS

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_SOFT_RESET_INST (
  .src_clk   (lite_clk),
  .src_in    (soft_reset_lite),
  
  .dest_clk  (axi4_clk),
  .dest_out  (soft_reset)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_SOFT_RESET_AXI_INST (
  .src_clk   (lite_clk),
  .src_in    (soft_reset_done),
  
  .dest_clk  (axi4_clk),
  .dest_out  (soft_reset_done_axi)
);


xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_HALT_INST (
  .src_clk    (lite_clk),
  .src_rst    (~lite_resetn),
  .src_pulse  (start_halt),
  
  .dest_clk   (axi4_clk),
  .dest_rst   (~axi4_resetn),
  .dest_pulse (halt_dm)
);

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_DONE_INST (
  .src_clk    (axi4_clk),
  .src_rst    (~axi4_resetn),
  .src_pulse  (reset_ready),
  
  .dest_clk   (lite_clk),
  .dest_rst   (~lite_resetn),
  .dest_pulse (reset_ready_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_HALT_COMPLETE_INST (
  .src_clk   (axi4_clk),
  .src_in    (halt_complete_dm),
  
  .dest_clk  (lite_clk),
  .dest_out  (halt_complete)
);

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_PROC_DONE_INST (
  .src_clk    (axi4_clk),
  .src_rst    (~axi4_resetn),
  .src_pulse  (soft_reset_proc_done),
  
  .dest_clk   (lite_clk),
  .dest_rst   (~lite_resetn),
  .dest_pulse (soft_reset_proc_done_lite)
);


end else
begin: SYNC_CLOCKS

assign soft_reset = soft_reset_lite;
assign soft_reset_done_axi = soft_reset_done;
assign halt_dm = start_halt;
assign reset_ready_lite = reset_ready;
assign halt_complete = halt_complete_dm;
assign soft_reset_proc_done_lite = soft_reset_proc_done;

end
endgenerate


always@(posedge lite_clk)
begin
	if(~lite_resetn) begin
		soft_reset_clr <= 1'b0;
	end
	else if(reset_ready_lite && soft_reset_lite) begin
		soft_reset_clr <= 1'b1;
	end
	else if (soft_reset_proc_done_lite) begin
		soft_reset_clr <= 1'b0;
	end
end

always@(posedge lite_clk)
begin
	if(~lite_resetn) begin
		soft_reset_done <= 1'b0;
	end
	else begin
		soft_reset_done <= soft_reset_clr;
	end
end

always@(posedge axi4_clk)
begin
	if(~axi4_resetn) begin
		soft_reset_core <= 1'b0;
	end
	else begin
		//if(soft_reset_proc && halt_complete_dm) begin
		if(halt_complete_dm) begin
			soft_reset_core <= 1'b1;	//resetting core when halt and soft reset
		end
		else if (~soft_reset_proc && reset_ready) begin 
							//halt_done and no soft reset
			soft_reset_core <= 1'b0;
		end
		else if (~soft_reset_proc && soft_reset_proc_r) begin 
							//soft reset done
			soft_reset_core <= 1'b0;
		end
	end
end

always@(posedge axi4_clk)
begin
	if(~axi4_resetn) begin
		soft_reset_proc <= 1'b0;
		soft_reset_proc_r <= 1'b0;
	end
	else begin
		soft_reset_proc_r <= soft_reset_proc;
		if(soft_reset) begin
			soft_reset_proc <= 1'b1;
		end
		else if(soft_reset_core_d4 && soft_reset_done_axi) begin
			soft_reset_proc <= 1'b0;
		end
	end
end
assign soft_reset_proc_done = ~soft_reset_proc && soft_reset_proc_r;
assign reset_ready = soft_reset_core_d4 && (~soft_reset_core_d5); //axi4

always@(posedge axi4_clk)
begin
	if(~axi4_resetn) begin
		soft_reset_core_d1 <= 'd0;
		soft_reset_core_d2 <= 'd0;
		soft_reset_core_d3 <= 'd0;
		soft_reset_core_d4 <= 'd0;
		soft_reset_core_d5 <= 'd0;
	end
	else if(soft_reset_core) begin
		soft_reset_core_d1 <= soft_reset_core;
		soft_reset_core_d2 <= soft_reset_core_d1;
		soft_reset_core_d3 <= soft_reset_core_d2;
		soft_reset_core_d4 <= soft_reset_core_d3;
		soft_reset_core_d5 <= soft_reset_core_d4;
	end
	else begin
		soft_reset_core_d1 <= 'd0;
		soft_reset_core_d2 <= 'd0;
		soft_reset_core_d3 <= 'd0;
		soft_reset_core_d4 <= 'd0;
		soft_reset_core_d5 <= 'd0;
	end
end


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

module audio_formatter_v1_0_11_aes_enc 
#(
 parameter integer C_NUM_CHANNELS = 2
)(
input axis_clk,
input axis_resetn,

input [23:0] 	in_data,
input 	     	in_valid,
input [3:0] 	in_id,
input 	     	in_ready,

input [3:0] no_of_valid_channels,
input validity,
output [31:0] out_data,

input		capt_channel_status,
input [191:0]	aes_channel_status
);

localparam PREAMB_Z   = 4'b0001; // Start of AES Block (B/Z)
localparam PREAMB_X   = 4'b0010; // Subframe 1 (M/X)
localparam PREAMB_Y   = 4'b0011; // Subframe 2 (W/Y)

integer k;
function Parity;
    input [26:0] data;            //Strobe
    begin
    Parity = 0;
      for (k=0; k<=26; k=k+1) begin
      if (data[k]) begin
        Parity = ~Parity;
      end
    end
     end
 endfunction

wire cs_bit;
reg valid_bit;
wire user_bit;
wire parity_bit;
wire [3:0] preamble;
wire [26:0] data_for_parity;
reg [191:0] channel_status_reg;
//reg [191:0] current_channel_status;
reg start;
reg [7:0] count;


assign data_for_parity = {cs_bit,user_bit,valid_bit,in_data}; 
assign out_data = {parity_bit,data_for_parity,preamble};

//assign cs_bit = current_channel_status[count];
assign cs_bit = channel_status_reg[count];
assign user_bit = 1'b0;
assign preamble = in_id[0] ? PREAMB_Y : ((count == 'd0) ? PREAMB_Z : PREAMB_X);
assign parity_bit = Parity(data_for_parity);

always@(posedge axis_clk)
begin
	if(!axis_resetn)
	begin
		channel_status_reg <= 'd0;
		start	<= 1'b0;
	end
	else
	begin
		if(capt_channel_status)
		begin
			channel_status_reg <=  aes_channel_status;
		end
		if(in_valid && in_ready)
		begin
			start	<= 1'b1;
		end
	end
end

always@(posedge axis_clk) 
begin
	if(!axis_resetn)
	begin
		count <= 'd0;
	//	current_channel_status <= 'd0;
		valid_bit	<= 1'b0;
	end
	else if(!start) begin
		count	<= 'b0;
	//	current_channel_status <= channel_status_reg;
		valid_bit	<= validity;
	end
	else if(in_valid && in_ready && (in_id >= no_of_valid_channels -1'b1))
	begin
		if(count >= 'd191) begin
			count	<= 'd0;
	//		current_channel_status <= channel_status_reg;
			valid_bit	<= validity;
		end
		else begin
			count	<= count + 1'b1;
		end
	end
end

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

module audio_formatter_v1_0_11_aes_dec 
#(
 parameter integer C_NUM_CHANNELS = 2
)(
input axis_clk,
input axis_resetn,

input [31:0] 	in_data,
input 	     	in_valid,
input [3:0] 	in_id,
input 	     	in_ready,

input		clear_channel_status,

output reg [191:0]	aes_channel_status,
output 		channel_status_updated
);

reg start_dec;
reg [191:0] status_reg;
reg [7:0]  count;
reg status_change;

localparam PREAMBLE_Z   = 4'b0001; // Start of AES Block (B/Z)
localparam PREAMBLE_X   = 4'b0010; // Subframe 1 (M/X)
localparam PREAMBLE_Y   = 4'b0011; // Subframe 2 (W/Y)
localparam cAES_CHSTS_BIT  = 30;
wire [3:0] aes_preamble;
wire first_valid_sample = in_valid && in_ready && (in_id == 'd0);
assign aes_preamble = in_data[3:0];
assign channel_status_updated = status_change;

always@(posedge axis_clk )
begin
	if(!axis_resetn || (clear_channel_status))
	begin
		status_reg      <= 'd0;
		start_dec 	<= 'd0;
		count		<= 8'd0;
	end
	else
	begin
		if(first_valid_sample && (aes_preamble == PREAMBLE_Z))
		begin
			start_dec	<= 1'b1;
			status_reg	<= {in_data[cAES_CHSTS_BIT], 191'd0};
			count 		<= 8'd191;
		end
		else if(first_valid_sample && start_dec)
		begin
			status_reg	<= {in_data[cAES_CHSTS_BIT], status_reg[191:1]};
			count		<= count - 1'b1;
		end
	end
end

always@(posedge axis_clk )
begin
	if(!axis_resetn || (clear_channel_status))
	begin
		status_change   <= 1'b0;
		aes_channel_status	<= 'd0;
	end
	else
	begin
		status_change	<= 1'b0;
		if(count == 'd0) begin
			status_change	<= !(status_reg == aes_channel_status);
			aes_channel_status	<= status_reg;
		end	
	end
end

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps

module audio_formatter_v1_0_11_s2mm_top #(
	parameter C_FAMILY = "virtex7" , 
	parameter integer C_MAX_NUM_CHANNELS_S2MM = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_S2MM = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_S2MM_DATAFORMAT = 1, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
	parameter integer C_S2MM_ADDR_WIDTH = 64, //32,64
 	parameter integer C_S2MM_ASYNC_CLOCK = 1
)(
	input 		s_axi_lite_aclk,
	input 		s_axi_lite_aresetn,

	input  		s_axi_lite_awvalid, 
	output 		s_axi_lite_awready, 
	input  	[11:0]	s_axi_lite_awaddr , 

	input  		s_axi_lite_wvalid , 
	output 		s_axi_lite_wready , 
	input  	[31:0]	s_axi_lite_wdata  , 
	                    
	output  [1:0] 	s_axi_lite_bresp  , 
	output   	s_axi_lite_bvalid , 
	input 		s_axi_lite_bready , 
	                    
/*
	input 		s_axi_lite_arvalid, 
	output		s_axi_lite_arready, 
	input 	[11:0]	s_axi_lite_araddr , 
	                    
	output 		s_axi_lite_rvalid , 
	input 		s_axi_lite_rready , 
	output	[31:0]	s_axi_lite_rdata  , 
	                    
	output  [1:0] 	s_axi_lite_rresp  , */
	
	input 	[7:0]	s_axi_lite_araddr , 
	output	[31:0]	s_axi_lite_rdata  , 
	output	      	s_axi_lite_rNOK   , 
	input 		read_trans, 


//------S2MM-----------------
	input 		s_axis_s2mm_aclk,
	input 		s_axis_s2mm_aresetn,
	output 		Irq_s2mm,
                         
// AXIS S2MM Channel                                                             
	input 		s_axis_s2mm_tvalid,
	output 		s_axis_s2mm_tready,
	input	[31:0]	s_axis_s2mm_tdata,
	input 	[7:0]	s_axis_s2mm_tid,

//AXI S2MM Channel
        output 	[C_S2MM_ADDR_WIDTH-1:0]	m_axi_s2mm_awaddr,
         		                
        output 	[7:0]	m_axi_s2mm_awlen,
        output 	[2:0]	m_axi_s2mm_awsize,
        output 	[1:0]	m_axi_s2mm_awburst,
        output 	[2:0]	m_axi_s2mm_awprot,
        output 	[3:0]	m_axi_s2mm_awcache,
        output 	[3:0]	m_axi_s2mm_awuser,
        output 		m_axi_s2mm_awvalid,
        input 		m_axi_s2mm_awready,
         		                       
        output 	[31:0]	m_axi_s2mm_wdata,
        output 	[3:0]	m_axi_s2mm_wstrb,
         		                       
        output 		m_axi_s2mm_wlast       ,
        output 		m_axi_s2mm_wvalid,
        input 		m_axi_s2mm_wready,

        input 	[1:0]	m_axi_s2mm_bresp,
        input 		m_axi_s2mm_bvalid,
        output 		m_axi_s2mm_bready

);

wire [3:0] no_of_valid_channels_lite;
wire [2:0] pcm_data_width_lite;
wire [7:0] ioc_threshold_lite;
wire soft_reset_lite;
wire start_dma_lite;

wire timeout_err_lite;
wire s2mm_decode_error_lite;
wire s2mm_slave_error_lite;
wire ioc_irq_pulse_lite;
wire aes_channel_status_changed_lite;

wire [31:0] timeout_count_lite;
wire [7:0] no_of_periods_lite;
wire [15:0] period_size_lite;

wire [C_S2MM_ADDR_WIDTH-1 :0] buffer_start_address_lite;
//wire [7:0] no_of_periods_transferred_lite;
wire [24:0] dma_transfer_count_lite;
wire clear_dma_transfer_count_lite;
wire [15:0] channel_offset_lite;
wire clear_aes_channel_status_lite;

wire [3:0] no_of_valid_channels;
wire [2:0] pcm_data_width;
wire [7:0] ioc_threshold;
wire soft_reset;
wire start_dma;

//wire timeout_err = 1'b0;
reg timeout_err;
reg [31:0] count_timeout;
wire s2mm_decode_error;
wire s2mm_slave_error;
wire ioc_irq_pulse;
wire aes_channel_status_changed;  // = 1'b0;

wire [31:0] timeout_count;
wire [7:0] no_of_periods;
wire [15:0] period_size;
wire [15:0] channel_offset;

wire [C_S2MM_ADDR_WIDTH-1 :0] buffer_start_address;
//wire [7:0] no_of_periods_transferred;
wire [24:0] dma_transfer_count;
wire [191:0] aes_channel_status; // = 'd0;
wire clear_dma_transfer_count;
wire clear_aes_channel_status;

wire halted;
wire soft_reset_clr;
wire soft_reset_core;
wire halt_dm;
wire halt_complete_dm;

wire reset_gen;
wire reset_gen_soft;
wire soft_reset_proc;

assign reset_gen = s_axis_s2mm_aresetn && (~soft_reset_core);
assign reset_gen_soft = s_axis_s2mm_aresetn && (~(soft_reset_core && soft_reset_proc));

audio_formatter_v1_0_11_s2mm_registers #(
 .C_ADDR_WIDTH(C_S2MM_ADDR_WIDTH)
) s2mm_registers_1
(
  // AXI4-Lite bus (cpu control)
  .iAxiClk               (s_axi_lite_aclk),
  .iAxiResetn            (s_axi_lite_aresetn),
  // - Write address
  .iAxi_AWValid          (s_axi_lite_awvalid),
  .oAxi_AWReady          (s_axi_lite_awready),
  .iAxi_AWAddr           (s_axi_lite_awaddr[11:0]),
  // - Write data
  .iAxi_WValid           (s_axi_lite_wvalid),
  .oAxi_WReady           (s_axi_lite_wready),
  .iAxi_WData            (s_axi_lite_wdata),
  // - Write response
  .oAxi_BValid           (s_axi_lite_bvalid),
  .iAxi_BReady           (s_axi_lite_bready),
  .oAxi_BResp            (s_axi_lite_bresp),

/*  // - Read address   
  .iAxi_ARValid          (s_axi_lite_arvalid),
  .oAxi_ARReady          (s_axi_lite_arready),
  .iAxi_ARAddr           (s_axi_lite_araddr),
  // - Read data/response
  .oAxi_RValid           (s_axi_lite_rvalid),
  .iAxi_RReady           (s_axi_lite_rready), 
  .oAxi_RData            (s_axi_lite_rdata),
  .oAxi_RResp            (s_axi_lite_rresp),*/

  .rReadAddr		 (s_axi_lite_araddr),
  .nReadData		 (s_axi_lite_rdata ),
  .ReadAddrNOK		 (s_axi_lite_rNOK  ),

  // IRQ
  .Irq_s2mm                  (Irq_s2mm),
  .read_trans			(read_trans),
  // In/Out signals
.no_of_valid_channels	(no_of_valid_channels_lite),
.pcm_data_width		(pcm_data_width_lite),
.ioc_threshold		(ioc_threshold_lite),
.reset			(soft_reset_lite),
.run_stop		(start_dma_lite),

.itimeout_err		(timeout_err_lite),
.is2mm_decode_error	(s2mm_decode_error_lite),
.is2mm_slave_error	(s2mm_slave_error_lite),
.ioc_irq_pulse		(ioc_irq_pulse_lite),
.aes_channel_status_changed	(aes_channel_status_changed_lite),
.clear_channel_status	(clear_aes_channel_status_lite),

.timeout_counter	(timeout_count_lite),
.no_of_periods		(no_of_periods_lite),
.period_size		(period_size_lite),
.channel_offset		(channel_offset_lite),

.buffer_start_address	(buffer_start_address_lite),

//.no_of_periods_transferred	(no_of_periods_transferred_lite),
.clear_dma_transfer_count	(clear_dma_transfer_count_lite),
.dma_transfer_count	(dma_transfer_count_lite),
.aes_channel_status_axi	(aes_channel_status),

.halted			(halted),
.soft_reset_clr		(soft_reset_clr)
);

audio_formatter_v1_0_11_reset #(
	.C_ASYNC_CLOCK(C_S2MM_ASYNC_CLOCK)
) reset_s2mm 
(
 .lite_clk(s_axi_lite_aclk),
 .lite_resetn(s_axi_lite_aresetn),

 .axi4_clk(s_axis_s2mm_aclk),
 .axi4_resetn(s_axis_s2mm_aresetn),

 .start_dma_lite	(start_dma_lite),
 .soft_reset_lite	(soft_reset_lite),
 .halt_complete_dm	(halt_complete_dm),
 
 .soft_reset_clear	(soft_reset_clr),
 .soft_reset_core	(soft_reset_core),
 .soft_reset_proc	(soft_reset_proc),
 .halt_dm		(halt_dm),
 .halted		(halted)
);

audio_formatter_v1_0_11_s2mm_sync #(
	.C_ADDR_WIDTH(C_S2MM_ADDR_WIDTH),
	.C_ASYNC_CLOCK(C_S2MM_ASYNC_CLOCK)
) s2mm_cdc_1
(
 .lite_clk(s_axi_lite_aclk),
 .lite_resetn(s_axi_lite_aresetn),

 .axi4_clk(s_axis_s2mm_aclk),
 .axi4_resetn(reset_gen),
 .axi4_resetn_org(s_axis_s2mm_aresetn),

 .no_of_valid_channels_lite	(no_of_valid_channels_lite),
 .pcm_data_width_lite		(pcm_data_width_lite),
 .ioc_threshold_lite		(ioc_threshold_lite),
 .start_dma_lite		(start_dma_lite),
 
 .timeout_count_lite 			(timeout_count_lite),
 .no_of_periods_lite			(no_of_periods_lite),
 .period_size_lite			(period_size_lite),
 .channel_offset_lite			(channel_offset_lite),
 .buffer_start_address_lite		(buffer_start_address_lite),
 .aes_channel_status_changed_lite	(aes_channel_status_changed_lite),
 .clear_channel_status_lite		(clear_aes_channel_status_lite),
 .clear_dma_transfer_count_lite		(clear_dma_transfer_count_lite),

 .s2mm_timeout_error_lite	(timeout_err_lite),
 .s2mm_decode_error_lite	(s2mm_decode_error_lite),
 .s2mm_slave_error_lite		(s2mm_slave_error_lite),
 .ioc_irq_pulse_lite		(ioc_irq_pulse_lite),
 .dma_transfer_count_lite	(dma_transfer_count_lite),
//-----------------------------------------
 .no_of_valid_channels	(no_of_valid_channels),
 .pcm_data_width	(pcm_data_width),
 .ioc_threshold		(ioc_threshold),
 .start_dma		(start_dma),
 
 .timeout_count 		(timeout_count),
 .no_of_periods			(no_of_periods),
 .period_size			(period_size),
 .channel_offset		(channel_offset),
 .buffer_start_address		(buffer_start_address),
 .aes_channel_status_changed	(aes_channel_status_changed),
 .clear_channel_status		(clear_aes_channel_status),
 .clear_dma_transfer_count	(clear_dma_transfer_count),

 .s2mm_timeout_error	(timeout_err),
 .s2mm_decode_error	(s2mm_decode_error),
 .s2mm_slave_error	(s2mm_slave_error),
 .ioc_irq_pulse		(ioc_irq_pulse),
 .dma_transfer_count	(dma_transfer_count)
);

always@(posedge s_axis_s2mm_aclk) begin
	if(~reset_gen_soft) begin
		timeout_err <= 1'b0;
		count_timeout	<= 'd0;
	end
	else begin
		if(start_dma && (~(s_axis_s2mm_tvalid))) begin
			if(count_timeout >= timeout_count) begin
				timeout_err <= 1'b1;
			end
			else begin
				count_timeout <= count_timeout + 1'b1;
			end
		end
		else begin
			count_timeout <= 'd0;
		end
	end
end

audio_formatter_v1_0_11_s2mm #(
	.C_FAMILY(C_FAMILY),
	.C_MAX_NUM_CHANNELS_S2MM(C_MAX_NUM_CHANNELS_S2MM),
	.C_PACKING_MODE_S2MM(C_PACKING_MODE_S2MM),
	.C_S2MM_DATAFORMAT(C_S2MM_DATAFORMAT),
	.C_S2MM_ADDR_WIDTH(C_S2MM_ADDR_WIDTH)
) s2mm_logic_1
(
	.s2mm_clk	(s_axis_s2mm_aclk),
	.s2mm_resetn	(reset_gen),
 	.s2mm_resetn_org(reset_gen_soft),

	.start_dma		(start_dma),
	.buffer_start_address	(buffer_start_address),
	.period_size		(period_size),
	.no_of_periods		(no_of_periods),
	.no_of_valid_channels	(no_of_valid_channels),
	.pcm_data_width		(pcm_data_width),
	.channel_offset		(channel_offset),

	.clear_aes_channel_status(clear_aes_channel_status),
	.aes_channel_status(aes_channel_status),
	.aes_channel_status_changed(aes_channel_status_changed),

	.clear_dma_transfer_count	(clear_dma_transfer_count),

	.dma_transfer_count		(dma_transfer_count),
	.ioc_interrupt			(ioc_irq_pulse),
	.slave_error			(s2mm_slave_error),
	.decode_error			(s2mm_decode_error),

 	.halt_complete_dm	(halt_complete_dm),
 	.halt_dm		(halt_dm),

      .s_axis_tdata           (s_axis_s2mm_tdata        ),
      .s_axis_tid             (s_axis_s2mm_tid        ),
      .s_axis_tvalid          (s_axis_s2mm_tvalid       ),
      .s_axis_tready          (s_axis_s2mm_tready       ),

	.m_axi_s2mm_awaddr           (m_axi_s2mm_awaddr              ),
	.m_axi_s2mm_awlen            (m_axi_s2mm_awlen               ),
	.m_axi_s2mm_awsize           (m_axi_s2mm_awsize              ),
	.m_axi_s2mm_awburst          (m_axi_s2mm_awburst             ),
	.m_axi_s2mm_awprot           (m_axi_s2mm_awprot              ),
	.m_axi_s2mm_awcache          (m_axi_s2mm_awcache             ),
	.m_axi_s2mm_awuser           (m_axi_s2mm_awuser              ),
	.m_axi_s2mm_awvalid          (m_axi_s2mm_awvalid             ),
	.m_axi_s2mm_awready          (m_axi_s2mm_awready             ),

      .m_axi_s2mm_wdata            (m_axi_s2mm_wdata               ),
      .m_axi_s2mm_wstrb            (m_axi_s2mm_wstrb               ),
      .m_axi_s2mm_wlast            (m_axi_s2mm_wlast               ),
      .m_axi_s2mm_wvalid           (m_axi_s2mm_wvalid              ),
      .m_axi_s2mm_wready           (m_axi_s2mm_wready              ),

      .m_axi_s2mm_bresp            (m_axi_s2mm_bresp               ),
      .m_axi_s2mm_bvalid           (m_axi_s2mm_bvalid              ),
      .m_axi_s2mm_bready           (m_axi_s2mm_bready              )
);


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps


module audio_formatter_v1_0_11_s2mm_registers 
#(
 parameter C_ADDR_WIDTH = 64

)
(

  // AXI4-Lite bus (cpu control)
  input             iAxiClk,
  input             iAxiResetn,
  // - Write address
  input             iAxi_AWValid,
  output reg        oAxi_AWReady,
  input      [ 11:0] iAxi_AWAddr,
  // - Write data
  input             iAxi_WValid,
  output reg        oAxi_WReady,
  input      [31:0] iAxi_WData,
  // - Write response
  output reg        oAxi_BValid,
  input             iAxi_BReady,
  output reg [ 1:0] oAxi_BResp,

/*  // - Read address   
  input             iAxi_ARValid,
  output reg        oAxi_ARReady,
  input      [ 7:0] iAxi_ARAddr,
  // - Read data/response
  output reg        oAxi_RValid,
  input             iAxi_RReady, 
  output reg [31:0] oAxi_RData,
  output reg [ 1:0] oAxi_RResp,
*/

input      [ 7:0] rReadAddr,
output reg [31:0] nReadData,
output reg        ReadAddrNOK,
input read_trans,

output reg [3:0] no_of_valid_channels,
output reg [2:0] pcm_data_width,
output reg [7:0] ioc_threshold,
output reg reset,
output reg run_stop,

input wire itimeout_err,
input wire is2mm_decode_error,
input wire is2mm_slave_error,
input ioc_irq_pulse,
input aes_channel_status_changed,
output reg clear_channel_status,

output reg [31:0] timeout_counter,
output reg [7:0] no_of_periods,
output reg [15:0] period_size,
output wire [15:0] channel_offset,

output wire [C_ADDR_WIDTH-1:0] buffer_start_address,

input halted,
input soft_reset_clr,

//input [7:0] no_of_periods_transferred,
input [24:0] dma_transfer_count,
input [191:0] aes_channel_status_axi,
output clear_dma_transfer_count,
output Irq_s2mm
);

localparam cAXI4_RESP_OKAY   = 2'b00; // Okay
localparam cAXI4_RESP_SLVERR = 2'b10; // Slave error
localparam cADDR_CTRL 	      = 'h10; //Control register
localparam cADDR_STS  	      = 'h14; //Status register
localparam cADDR_TIMEOUT      = 'h18; //Timeout register
localparam cADDR_PERIOD_CFG   = 'h1C; //Period configuration
localparam cADDR_BUFFER_LSB   = 'h20; //LSB Buffer Address
localparam cADDR_BUFFER_MSB   = 'h24; //MSB Buffer Address
localparam cADDR_DMA_COUNT    = 'h28; //DMA S2MM transfer count

localparam cADDR_AES_CHSTS_1  = 'h2C; // AES Channel Status 1
localparam cADDR_AES_CHSTS_2  = 'h30; // AES Channel Status 2
localparam cADDR_AES_CHSTS_3  = 'h34; // AES Channel Status 3
localparam cADDR_AES_CHSTS_4  = 'h38; // AES Channel Status 4
localparam cADDR_AES_CHSTS_5  = 'h3C; // AES Channel Status 5
localparam cADDR_AES_CHSTS_6  = 'h40; // AES Channel Status 6

localparam cADDR_SIZE_PER_CHANNEL    = 'h44; //DMA S2MM size per channel

// Irq Generation
reg ioc_irq;
wire err_irq;
//reg halt_in_process;
//wire halted = 1'b0;
reg ioc_irq_en;
reg err_irq_en;
reg timeout_irq_en;
reg timeout_err;
reg clear_ioc_irq;
//reg clear_channel_status;
reg channel_status_detected;
wire clear_cs_update_bit;
reg reading_status_reg;

reg s2mm_decode_error;
reg s2mm_slave_error;
reg clear_transfer_count;
reg [15:0] size_per_channel;

reg clear_transfer_count_r;
//wire soft_reset_clr = 1'b0;

always@(posedge iAxiClk) 
begin
 if (!iAxiResetn || soft_reset_clr) begin
   clear_transfer_count_r <= 1'b0;
 end
 else begin
   clear_transfer_count_r <= clear_transfer_count;
 end
end

assign channel_offset = size_per_channel;
assign clear_dma_transfer_count = (!clear_transfer_count) && clear_transfer_count_r;
always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	ioc_irq <= 1'b0;
  end
  else begin
	if(ioc_irq_pulse) begin
		ioc_irq <= 1'b1;
	end
	else if(clear_ioc_irq) begin
		ioc_irq <= 1'b0;
	end
  end
end

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	s2mm_decode_error <= 1'b0;
  end
  else begin
	if(is2mm_decode_error) begin
		s2mm_decode_error <= 1'b1;
	end
  end
end

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	s2mm_slave_error <= 1'b0;
  end
  else begin
	if(is2mm_slave_error) begin
		s2mm_slave_error <= 1'b1;
	end
  end
end

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	timeout_err <= 1'b0;
  end
  else begin
	if(itimeout_err) begin
		timeout_err <= 1'b1;
	end
  end
end

assign err_irq = err_irq_en && (s2mm_slave_error || s2mm_decode_error);
assign Irq_s2mm = ioc_irq || err_irq || (timeout_err && timeout_irq_en);

// Input Capture
reg [191:0] aes_channel_status;

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
    aes_channel_status   <= 192'h0;
  end
  else begin
    if (aes_channel_status_changed) begin
      aes_channel_status <= aes_channel_status_axi;
    end
  end
end

assign clear_cs_update_bit = reading_status_reg && read_trans;

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	channel_status_detected <= 1'b0;
  end
  else begin
	if(clear_cs_update_bit) begin
		channel_status_detected <= 1'b0;
	end
	else if(aes_channel_status_changed) begin
		channel_status_detected <= 1'b1;
	end
  end
end

////////////////////////////////////////////////////////
// Write channel

localparam sWriteReset = 3'd0;
localparam sWriteAddr = 3'd1;
localparam sWriteData = 3'd2;
localparam sWriteResp = 3'd3;
             
reg [2:0] stmWrite;

reg [11:0] rWriteAddr;

// Statemachine for taking care of the write signals
always @(posedge iAxiClk)
begin
  if (!iAxiResetn)
  begin
    oAxi_AWReady        <= 1'b0;
    oAxi_WReady         <= 1'b0;
    oAxi_BValid         <= 1'b0;
    rWriteAddr          <=  'h0;
    stmWrite            <= sWriteReset;
  end
  else
  begin
    case (stmWrite) 
      sWriteReset :
      begin
        oAxi_AWReady    <= 1'b1;
        oAxi_WReady     <= 1'b0;
        oAxi_BValid     <= 1'b0;
        stmWrite        <= sWriteAddr;
      end
      
      sWriteAddr :
      begin
        oAxi_AWReady    <= 1'b1;
        if (iAxi_AWValid)
        begin
          oAxi_AWReady  <= 1'b0;
          oAxi_WReady   <= 1'b1;
          rWriteAddr    <= iAxi_AWAddr;
          stmWrite      <= sWriteData;
        end
      end
      
      sWriteData :
      begin
        oAxi_WReady     <= 1'b1;
        
        if (iAxi_WValid)
        begin
          oAxi_WReady   <= 1'b0;
          oAxi_BValid   <= 1'b1;
          stmWrite      <= sWriteResp;
        end
      end
      
      sWriteResp :
      begin
        oAxi_BValid     <= 1'b1;
        if (iAxi_BReady)
        begin
          oAxi_BValid   <= 1'b0;
          stmWrite      <= sWriteReset;
        end
      end 
      
      default :
        stmWrite        <= sWriteReset;
    endcase
  end
end

reg [63:0] buffer_address;
assign buffer_start_address = buffer_address[C_ADDR_WIDTH - 1:0];


wire [31:0] ctrl_reg = {
	9'd0,
	no_of_valid_channels,
	pcm_data_width,
	1'd0,
	timeout_irq_en,
	ioc_irq_en,
	err_irq_en,
	//ioc_threshold,
	8'd0,
	2'd0,
	reset,
	run_stop
};

wire [31:0] status_reg = {
	ioc_irq,
	err_irq,
	channel_status_detected,
	9'd0,
	timeout_err,
	s2mm_decode_error,
	s2mm_slave_error,
	16'd0,
	halted
};

wire [31:0] timeout_reg = {
 timeout_counter
};

wire [31:0] period_confg_reg = {
	8'd0,
	no_of_periods,
	period_size
};

wire [31:0] buffer_addr_lsb_reg = {
	buffer_address[31:0]
};

wire [31:0] buffer_addr_msb_reg = {
	buffer_address[63:32]
};

wire [31:0] s2mm_transfer_count_reg = {
	7'd0,
//	no_of_periods_transferred,
	dma_transfer_count
};

wire [31:0] s2mm_size_per_channel_reg = {
	16'd0,
	size_per_channel
};

// Write address decoder
always @(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr)
  begin
    oAxi_BResp        <= cAXI4_RESP_OKAY;
  //  clear_err_irq   <= 1'b0; 
    clear_ioc_irq   <= 1'b0; 
    clear_channel_status <= 1'b0;

		no_of_valid_channels 	<= 4'd2;
		pcm_data_width 		<= 3'd2;
		timeout_irq_en		<= 1'b1;
		ioc_irq_en		<= 1'b1;
		err_irq_en		<= 1'b0;
		ioc_threshold 		<= 8'd1;
		reset			<= 1'b0;
		run_stop		<= 1'b0;

		timeout_counter		<= 32'h80000000;
		no_of_periods		<= 'd1;
		period_size 		<= 'd0;

		buffer_address		<= 'd0;
		size_per_channel	<= 16'd0;
  end
  else
  begin
    // Defaults
    clear_ioc_irq   <= 1'b0; 
    
    if (oAxi_WReady && iAxi_WValid)
    begin
      oAxi_BResp      <= cAXI4_RESP_OKAY;
     // clear_ioc_irq   <= 1'b0; 
    //  clear_err_irq   <= 1'b0; 
      clear_channel_status <= 1'b0;
      case (rWriteAddr)
        cADDR_CTRL :
        begin
		no_of_valid_channels 	<= iAxi_WData[22:19];
		pcm_data_width 		<= iAxi_WData[18:16];
		timeout_irq_en		<= iAxi_WData[14];
		ioc_irq_en		<= iAxi_WData[13];
		err_irq_en		<= iAxi_WData[12];
		ioc_threshold 		<= iAxi_WData[11:4];
		reset			<= iAxi_WData[1];
		run_stop		<= iAxi_WData[0];
        end
        cADDR_STS :
        begin
		clear_ioc_irq 		<= iAxi_WData[31];
//		clear_err_irq		<= iAxi_WData[30];
        end
        cADDR_TIMEOUT :
        begin
          	timeout_counter     	<= iAxi_WData[31:0];
        end
	cADDR_PERIOD_CFG :
	begin
		no_of_periods		<= iAxi_WData[23:16];
		period_size		<= iAxi_WData[15:0];
	end
	cADDR_BUFFER_LSB :
	begin
		buffer_address[31:0] 	<= iAxi_WData[31:0];
	end
	cADDR_BUFFER_MSB :
	begin
		buffer_address[63:32] 	<= iAxi_WData[31:0];
	end
	cADDR_DMA_COUNT :
	begin
	end
	cADDR_AES_CHSTS_1 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_2 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_3 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_4 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_5 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_6 :
	begin
		clear_channel_status 	<= 1'b1;
	end
	cADDR_SIZE_PER_CHANNEL :
	begin
		size_per_channel	<= iAxi_WData[15:0];
	end	
        default :
	begin
	    if((rWriteAddr[1:0] == 'd0) && (rWriteAddr[8] == 'b1) && (rWriteAddr[7:0] >= 'h10) && (rWriteAddr[7:0] <= 'h44))
            begin
		oAxi_BResp <= cAXI4_RESP_OKAY;	 // For MM2s registers
	    end
	    else
	    begin
	        oAxi_BResp <= cAXI4_RESP_SLVERR;
	    end
	end
      endcase
    end
  end
end

/*
localparam sReadReset = 2'd0;
localparam sReadAddr = 2'd1;
localparam sDecodeAddr = 2'd2;
localparam sReadData = 2'd3;
             
reg [1:0] stmRead;


// Statemachine for taking care of the read signals
always @(posedge iAxiClk)
begin
  if (!iAxiResetn)
  begin
    oAxi_ARReady        <= 1'b0;    
    oAxi_RResp          <= cAXI4_RESP_OKAY;
    oAxi_RValid         <= 1'b0;
    oAxi_RData          <=  'h0;
    rReadAddr           <=  'h0;
    stmRead             <= sReadReset;
  end
  else
  begin
    case (stmRead) 
      sReadReset :
      begin
        oAxi_ARReady    <= 1'b1;
        oAxi_RResp      <= cAXI4_RESP_OKAY;
        oAxi_RValid     <= 1'b0;
        oAxi_RData      <=  'h0;
        rReadAddr       <=  'h0;
        stmRead         <= sReadAddr;
      end
      
      sReadAddr :
      begin
        oAxi_ARReady    <= 1'b1;
        if (iAxi_ARValid)
        begin
          oAxi_ARReady  <= 1'b0;
          rReadAddr     <= iAxi_ARAddr;
          stmRead       <= sDecodeAddr;
        end
      end
      
      sDecodeAddr :
      begin
        if (ReadAddrNOK)
          oAxi_RResp    <= cAXI4_RESP_SLVERR;
        else
          oAxi_RResp    <= cAXI4_RESP_OKAY;
          
        oAxi_RData      <= nReadData;
        oAxi_RValid     <= 1'b1;
        stmRead         <= sReadData;
      end
      
      sReadData :
      begin
        oAxi_RValid     <= 1'b1;
        if (iAxi_RReady)
        begin
          oAxi_RValid   <= 1'b0;
          stmRead       <= sReadReset;
        end
      end
      
      default :
        stmRead         <= sReadReset;
    endcase
  end
end
*/

// Read address decoder
always@(*)
begin
  ReadAddrNOK        = 1'b0;
  nReadData          =  'h0;
   reading_status_reg = 'd0;
  clear_transfer_count = 1'b0;
  case (rReadAddr)
    cADDR_CTRL :
    begin
      nReadData   	= ctrl_reg;
    end
    cADDR_STS :
    begin
      nReadData		= status_reg;
      reading_status_reg = 'd1;
    end
    cADDR_TIMEOUT :
    begin
      nReadData		= timeout_reg;
    end
    cADDR_PERIOD_CFG :
    begin
      nReadData		= period_confg_reg;
    end
    cADDR_BUFFER_LSB :
    begin
      nReadData		= buffer_addr_lsb_reg;
    end
    cADDR_BUFFER_MSB :
    begin
      nReadData		= buffer_addr_msb_reg;
    end
    cADDR_DMA_COUNT :
    begin
      nReadData		= s2mm_transfer_count_reg;
	clear_transfer_count = 1'b1;
    end
    cADDR_AES_CHSTS_1 :
    begin
      nReadData		= aes_channel_status[31:0];
    end
    cADDR_AES_CHSTS_2 :
    begin
      nReadData      = aes_channel_status[63:32];
    end
    cADDR_AES_CHSTS_3 :
    begin
      nReadData      = aes_channel_status[95:64];
    end
    cADDR_AES_CHSTS_4 :
    begin
      nReadData      = aes_channel_status[127:96];
    end
    cADDR_AES_CHSTS_5 :
    begin
      nReadData      = aes_channel_status[159:128];
    end
    cADDR_AES_CHSTS_6 :
    begin
      nReadData      = aes_channel_status[191:160];
    end
    cADDR_SIZE_PER_CHANNEL :
    begin
      nReadData	     = s2mm_size_per_channel_reg;
    end
    default : 
      ReadAddrNOK    = 1'b1;
  endcase  
end

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps

module audio_formatter_v1_0_11_s2mm #(
	parameter C_FAMILY = "virtex7" , 
	parameter integer C_MAX_NUM_CHANNELS_S2MM = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_S2MM = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_S2MM_DATAFORMAT = 1, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
	parameter integer C_S2MM_ADDR_WIDTH = 64 //32,64
)(
	input s2mm_clk,
	input s2mm_resetn,
	input s2mm_resetn_org,

	input start_dma,
	input [C_S2MM_ADDR_WIDTH-1:0] buffer_start_address,
	input [15:0] period_size,
	input [7:0] no_of_periods,
	input [3:0] no_of_valid_channels,
	input [2:0] pcm_data_width,
	input clear_dma_transfer_count,
	input [15:0] channel_offset,

	output [191:0] aes_channel_status,
	output aes_channel_status_changed,
	input clear_aes_channel_status,
	
	output [24:0] dma_transfer_count,
	output ioc_interrupt,
	output slave_error,
	output decode_error,
	
	input halt_dm,
	output halt_complete_dm,
	
	input 		s_axis_tvalid,
	output 		s_axis_tready,
	input	[31:0]	s_axis_tdata,
	input 	[7:0]	s_axis_tid,

        output 	[C_S2MM_ADDR_WIDTH-1:0]	m_axi_s2mm_awaddr,
        output 	[7:0]	m_axi_s2mm_awlen,
        output 	[2:0]	m_axi_s2mm_awsize,
        output 	[1:0]	m_axi_s2mm_awburst,
        output 	[2:0]	m_axi_s2mm_awprot,
        output 	[3:0]	m_axi_s2mm_awcache,
        output 	[3:0]	m_axi_s2mm_awuser,
        output 		m_axi_s2mm_awvalid,
        input 		m_axi_s2mm_awready,
         		                       
        output 	[31:0]	m_axi_s2mm_wdata,
        output 	[3:0]	m_axi_s2mm_wstrb,
         		                       
        output 		m_axi_s2mm_wlast       ,
        output 		m_axi_s2mm_wvalid,
        input 		m_axi_s2mm_wready,

        input 	[1:0]	m_axi_s2mm_bresp,
        input 		m_axi_s2mm_bvalid,
        output 		m_axi_s2mm_bready
);
localparam C_CMD_ADDR_WIDTH = 64;
wire [C_CMD_ADDR_WIDTH-1:0] m_axi_s2mm_awaddr_temp;
assign m_axi_s2mm_awaddr =  m_axi_s2mm_awaddr_temp[C_S2MM_ADDR_WIDTH-1 :0];
wire [31:0] m_axis_tdata;
wire [31:0] m_axis_tdata_dm;
wire m_axis_tlast;
wire m_axis_tvalid;
wire m_axis_tready;
wire m_axis_tvalid_dm;
wire m_axis_tready_dm;
wire [3:0] m_axis_tid;
wire start_command_generation;

wire [40+C_CMD_ADDR_WIDTH-1:0] s2mm_cmd_data;
wire s2mm_cmd_valid;
wire s2mm_cmd_ready;

wire s2mm_status_valid;
wire [7:0] s2mm_status;
wire s2mm_status_ready;
wire s_axis_tready_buffer;
assign s_axis_tready = start_dma ? s_axis_tready_buffer : 1'b1;

audio_formatter_v1_0_11_s2mm_buffer #(
	.C_NUM_CHANNELS(C_MAX_NUM_CHANNELS_S2MM),
	.C_PACKAGING_MODE_S2MM(C_PACKING_MODE_S2MM),
	.C_S2MM_DATAFORMAT(C_S2MM_DATAFORMAT)
) buffering_1 (
	.axis_clk(s2mm_clk),
	.axis_resetn(s2mm_resetn),

	.start_dma(start_dma),
	.no_of_valid_channels(no_of_valid_channels),
	.pcm_data_width(pcm_data_width),
	
	.s_axis_tvalid(s_axis_tvalid),
	.s_axis_tid(s_axis_tid),
	.s_axis_wdata(s_axis_tdata),
	.s_axis_tready(s_axis_tready_buffer),

	.m_axis_tvalid(m_axis_tvalid),
	.m_axis_tdata(m_axis_tdata),
	.m_axis_tlast(m_axis_tlast),
	.m_axis_tready(m_axis_tready),
	.m_axis_tid(m_axis_tid),	
	.start_command_generation(start_command_generation)
);
 

audio_formatter_v1_0_11_s2mm_command_gen #(
	.C_NUM_CHANNELS(C_MAX_NUM_CHANNELS_S2MM),
	.C_PACKAGING_MODE_S2MM(C_PACKING_MODE_S2MM),
	.C_ADDR_WIDTH(C_S2MM_ADDR_WIDTH),
	.C_DATAFORMAT(C_S2MM_DATAFORMAT),
	.C_CMD_ADDR_WIDTH(C_CMD_ADDR_WIDTH)
) command_generator_1 (
	.axis_clk(s2mm_clk),
	.axis_resetn(s2mm_resetn),
	
	.buffer_start_address(buffer_start_address),
	.period_size(period_size),
	.no_of_periods(no_of_periods),
	.input_offset(channel_offset),
	.pcm_data_width(pcm_data_width),

	.start_command_generation(start_command_generation),
	.start_dma(start_dma),

	.no_of_valid_channels(no_of_valid_channels),
	.clear_dma_transfer_count(clear_dma_transfer_count),
	.dma_transfer_count(dma_transfer_count),
	.slave_error(slave_error),
	.decode_error(decode_error),
	.ioc_interrupt(ioc_interrupt),

	.s2mm_status_valid(s2mm_status_valid),
	.s2mm_status_ready(s2mm_status_ready),
	.s2mm_status(s2mm_status),
	
	.s2mm_cmd_tdata(s2mm_cmd_data),
	.s2mm_cmd_tvalid(s2mm_cmd_valid),
	.s2mm_cmd_tready(s2mm_cmd_ready)
);
generate if(C_S2MM_DATAFORMAT == 0 || C_S2MM_DATAFORMAT == 1) begin: AES_INPUT

wire aes_data_valid;
wire aes_data_ready;
wire [3:0] aes_data_id;
wire [31:0] aes_data;

assign aes_data_valid = start_dma ? m_axis_tvalid : s_axis_tvalid;
assign aes_data_ready = start_dma ? m_axis_tready : s_axis_tready;
assign aes_data_id = start_dma ? m_axis_tid[3:0] : s_axis_tid[3:0];
assign aes_data = start_dma ? m_axis_tdata : s_axis_tdata;

audio_formatter_v1_0_11_aes_dec aes_dec_1
(
	.axis_clk(s2mm_clk),
	.axis_resetn(s2mm_resetn_org),

	.in_data(aes_data),
	.in_valid(aes_data_valid),
	.in_ready(aes_data_ready),
	.in_id(aes_data_id),

	.clear_channel_status(clear_aes_channel_status),
	.aes_channel_status(aes_channel_status),
	.channel_status_updated(aes_channel_status_changed)
);


end
else begin: PCM_INPUT

assign aes_channel_status = 192'd0;
assign aes_channel_status_changed = 1'b0;

end
endgenerate

generate if(C_S2MM_DATAFORMAT == 1) begin : AES_TO_PCM
//---- If pcm data width is 8/16 bit-------
reg [31:0] tdata_8bit;
reg [1:0] strb_8;
wire m_axis_tready_8bit;
wire m_axis_tvalid_dm_8bit;
wire [31:0] m_axis_tdata_dm_8bit;
wire [31:0] next_data_8bit = {m_axis_tdata[27:20], tdata_8bit[31:8]};
wire [31:0] next_data_16bit = {m_axis_tdata[27:12], tdata_8bit[31:16]};
wire [1:0] strb_max_value = (pcm_data_width == 'd0) ? 2'd3 : 2'd1;
always@(posedge s2mm_clk) begin
	if(!s2mm_resetn) begin
		tdata_8bit 	<= 'd0;
		strb_8		<= 'd0;
	end
	else begin
		if(m_axis_tvalid_dm && m_axis_tready_dm) begin
			strb_8 <= 2'd0;
		end
		else if (m_axis_tvalid && m_axis_tready) begin
			strb_8 <= strb_8 + 1'b1;
			tdata_8bit <= (pcm_data_width == 'd0) ? next_data_8bit : next_data_16bit;
		end
	end
end

assign m_axis_tready_8bit  = (strb_8 == strb_max_value) ? m_axis_tready_dm : 1'b1;
assign m_axis_tvalid_dm_8bit = m_axis_tvalid && (strb_8 == strb_max_value);
assign m_axis_tdata_dm_8bit  = (pcm_data_width == 'd0) ? next_data_8bit : next_data_16bit;
//--------------------------------------------------


assign m_axis_tdata_dm = (pcm_data_width == 'd0) ? m_axis_tdata_dm_8bit :
		      	(pcm_data_width == 'd1) ? m_axis_tdata_dm_8bit :
		      	(pcm_data_width == 'd2) ? {12'd0,m_axis_tdata[27:8]}  :
		      	(pcm_data_width == 'd4) ? m_axis_tdata	              : {8'd0,m_axis_tdata[27:4]};
assign m_axis_tvalid_dm = ((pcm_data_width == 'd0) || (pcm_data_width == 'd1)) ? m_axis_tvalid_dm_8bit : m_axis_tvalid;
assign m_axis_tready = ((pcm_data_width == 'd0) || (pcm_data_width == 'd1)) ? m_axis_tready_8bit  : m_axis_tready_dm;

end
else if (C_S2MM_DATAFORMAT == 2) begin: PCM_TO_PCM

//---- If pcm data width is 8/16 bit-------
reg [31:0] tdata_8bit;
reg [1:0] strb_8;
wire m_axis_tready_8bit;
wire m_axis_tvalid_dm_8bit;
wire [31:0] m_axis_tdata_dm_8bit;
wire [31:0] next_data_8bit = {m_axis_tdata[7:0], tdata_8bit[31:8]};
wire [31:0] next_data_16bit = {m_axis_tdata[15:0], tdata_8bit[31:16]};
wire [1:0] strb_max_value = (pcm_data_width == 'd0) ? 2'd3 : 2'd1;
always@(posedge s2mm_clk) begin
	if(!s2mm_resetn) begin
		tdata_8bit 	<= 'd0;
		strb_8		<= 'd0;
	end
	else begin
		if(m_axis_tvalid_dm && m_axis_tready_dm) begin
			strb_8 <= 2'd0;
		end
		else if (m_axis_tvalid && m_axis_tready) begin
			strb_8 <= strb_8 + 1'b1;
			tdata_8bit <= (pcm_data_width == 'd0) ? next_data_8bit : next_data_16bit;
		end
	end
end

assign m_axis_tready_8bit  = (strb_8 == strb_max_value) ? m_axis_tready_dm : 1'b1;
assign m_axis_tvalid_dm_8bit = m_axis_tvalid && (strb_8 == strb_max_value);
assign m_axis_tdata_dm_8bit  = (pcm_data_width == 'd0) ? next_data_8bit : next_data_16bit;
//--------------------------------------
assign m_axis_tdata_dm = (pcm_data_width == 'd0) ? m_axis_tdata_dm_8bit :
		      	(pcm_data_width == 'd1) ? m_axis_tdata_dm_8bit :
		      	(pcm_data_width == 'd2) ? {12'd0,m_axis_tdata[19:0]}  :
		      	(pcm_data_width == 'd4) ? m_axis_tdata	              : {8'd0,m_axis_tdata[23:0]};
assign m_axis_tvalid_dm = ((pcm_data_width == 'd0) || (pcm_data_width == 'd1)) ? m_axis_tvalid_dm_8bit : m_axis_tvalid;
assign m_axis_tready = ((pcm_data_width == 'd0) || (pcm_data_width == 'd1)) ? m_axis_tready_8bit  : m_axis_tready_dm;


end
else begin :  SEND_FULL_DATA

assign m_axis_tdata_dm = m_axis_tdata;
assign m_axis_tvalid_dm = m_axis_tvalid;
assign m_axis_tready = m_axis_tready_dm;

end
endgenerate

axi_datamover
   #(
      .C_INCLUDE_MM2S              ( 1'b0	   	     ),
      .C_M_AXI_MM2S_ADDR_WIDTH     ( C_CMD_ADDR_WIDTH      ),
      .C_M_AXI_MM2S_DATA_WIDTH     ( 32          ),
      .C_M_AXIS_MM2S_TDATA_WIDTH   ( 32          ),
      .C_INCLUDE_MM2S_STSFIFO      ( 1     		   ),
      .C_MM2S_STSCMD_FIFO_DEPTH    ( 1	   ),
      .C_MM2S_STSCMD_IS_ASYNC      ( 0   ),
      .C_INCLUDE_MM2S_DRE          ( 0           ),
      .C_MM2S_BURST_SIZE           ( 256   ),
      .C_MM2S_BTT_USED             ( 23            ),
      .C_MM2S_ADDR_PIPE_DEPTH      ( 4 ),
      .C_MM2S_INCLUDE_SF           ( 0                      ),
      .C_ENABLE_MM2S_TKEEP 	   (0) ,

 
      .C_FAMILY                    ( C_FAMILY 		     ),
      .C_ENABLE_CACHE_USER         ( 0                       ),
      .C_ENABLE_SKID_BUF           ( "11000"                 ),
      .C_CMD_WIDTH                 ( 40+C_CMD_ADDR_WIDTH    ),

      .C_INCLUDE_S2MM              ( 1'b1         	),
      .C_M_AXI_S2MM_ADDR_WIDTH     ( C_CMD_ADDR_WIDTH ),
      .C_M_AXI_S2MM_DATA_WIDTH     ( 32      		),
      .C_S_AXIS_S2MM_TDATA_WIDTH   ( 32      		),
      .C_INCLUDE_S2MM_STSFIFO      ( 1     		),
      .C_S2MM_STSCMD_FIFO_DEPTH    ( 1 			),
      .C_S2MM_STSCMD_IS_ASYNC      ( 0      		),
      .C_INCLUDE_S2MM_DRE          ( 0    		),
      .C_S2MM_BURST_SIZE           ( 256   		),
      .C_S2MM_BTT_USED             ( 23         	),
      .C_S2MM_SUPPORT_INDET_BTT    ( 0      		),
      .C_S2MM_ADDR_PIPE_DEPTH      ( 4			),
      .C_S2MM_INCLUDE_SF           ( 0                  ),
      .C_ENABLE_S2MM_TKEEP  	   ( 0 			)
    ) I_DATAMOVER_S2MM 
  (
      // S2MM Primary Clock/Reset input
      .m_axi_s2mm_aclk             (s2mm_clk            ),
      .m_axi_s2mm_aresetn          (s2mm_resetn         ),

      // S2MM Soft Shutdown
      .s2mm_halt                   (halt_dm		),
      .s2mm_halt_cmplt             (halt_complete_dm    ),

      // S2MM Error output discrete
      .s2mm_err                    (           ),

      // Stream to Memory Map Command FIFO and Status FIFO I/O //////////////
      .m_axis_s2mm_cmdsts_awclk    (s2mm_clk                 ),
      .m_axis_s2mm_cmdsts_aresetn  (s2mm_resetn              ),

      // User Command Interface Ports (AXI Stream)
      .s_axis_s2mm_cmd_tvalid      (s2mm_cmd_valid    ),
      .s_axis_s2mm_cmd_tready      (s2mm_cmd_ready    ),
      .s_axis_s2mm_cmd_tdata       (s2mm_cmd_data     ),

      // User Status Interface Ports (AXI Stream)
      .m_axis_s2mm_sts_tvalid      (s2mm_status_valid   ),
      .m_axis_s2mm_sts_tready      (s2mm_status_ready 	),
      .m_axis_s2mm_sts_tdata       (s2mm_status 	),
      .m_axis_s2mm_sts_tkeep       (     		),
      .m_axis_s2mm_sts_tlast       (            	),
   
      // Address posting controls
      .s2mm_allow_addr_req         (1'b1   ),
      .s2mm_addr_req_posted        (       ),
      .s2mm_wr_xfer_cmplt          (       ),
      .s2mm_ld_nxt_len             (       ),
      .s2mm_wr_len                 (       ),
     

      // S2MM AXI Address Channel I/O  //////////////////////////////////////
//      m_axi_s2mm_awid             (open                      ),
      .m_axi_s2mm_awaddr           (m_axi_s2mm_awaddr_temp         ),
      .m_axi_s2mm_awlen            (m_axi_s2mm_awlen               ),
      .m_axi_s2mm_awsize           (m_axi_s2mm_awsize              ),
      .m_axi_s2mm_awburst          (m_axi_s2mm_awburst             ),
      .m_axi_s2mm_awprot           (m_axi_s2mm_awprot              ),
      .m_axi_s2mm_awcache          (m_axi_s2mm_awcache             ),
      .m_axi_s2mm_awuser           (m_axi_s2mm_awuser              ),
      .m_axi_s2mm_awvalid          (m_axi_s2mm_awvalid             ),
      .m_axi_s2mm_awready          (m_axi_s2mm_awready             ),

      // S2MM AXI MMap Write Data Channel I/O  //////////////////////////////
      .m_axi_s2mm_wdata            (m_axi_s2mm_wdata               ),
      .m_axi_s2mm_wstrb            (m_axi_s2mm_wstrb               ),
      .m_axi_s2mm_wlast            (m_axi_s2mm_wlast               ),
      .m_axi_s2mm_wvalid           (m_axi_s2mm_wvalid              ),
      .m_axi_s2mm_wready           (m_axi_s2mm_wready              ),

      // S2MM AXI MMap Write response Channel I/O  //////////////////////////
      .m_axi_s2mm_bresp            (m_axi_s2mm_bresp               ),
      .m_axi_s2mm_bvalid           (m_axi_s2mm_bvalid              ),
      .m_axi_s2mm_bready           (m_axi_s2mm_bready              ),

      // S2MM AXI Slave Stream Channel I/O  ////////////////////////////////-
      .s_axis_s2mm_tdata           (m_axis_tdata_dm     ),
      .s_axis_s2mm_tkeep           (4'hF    		),
      .s_axis_s2mm_tlast           (m_axis_tlast        ),
      .s_axis_s2mm_tvalid          (m_axis_tvalid_dm       ),
      .s_axis_s2mm_tready          (m_axis_tready_dm       ),

      // Testing Support I/O
      .s2mm_dbg_sel                (4'd0           ),
      .s2mm_dbg_data               ()
    );


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps


module audio_formatter_v1_0_11_s2mm_sync 
#(
 parameter integer C_ADDR_WIDTH = 64,
 parameter integer C_ASYNC_CLOCK = 1
)
(
input lite_clk,
input lite_resetn,

input axi4_clk,
input axi4_resetn,
input axi4_resetn_org,

input wire [3:0] no_of_valid_channels_lite,
input wire [2:0] pcm_data_width_lite,
input wire [7:0] ioc_threshold_lite,
input wire start_dma_lite,

input wire [31:0] timeout_count_lite,
input wire [7:0] no_of_periods_lite,
input wire [15:0] period_size_lite,
input wire [15:0] channel_offset_lite,
input wire [C_ADDR_WIDTH-1:0] buffer_start_address_lite,
output wire aes_channel_status_changed_lite,
input wire clear_channel_status_lite,
input wire clear_dma_transfer_count_lite,

output wire s2mm_timeout_error_lite,
output wire s2mm_decode_error_lite,
output wire s2mm_slave_error_lite,
output wire ioc_irq_pulse_lite,
output wire [24:0] dma_transfer_count_lite,
//------------------
output reg [3:0] no_of_valid_channels,
output reg [2:0] pcm_data_width,
output reg [7:0] ioc_threshold,
output wire start_dma,

output reg [31:0] timeout_count,
output reg [7:0] no_of_periods,
output reg [15:0] period_size,
output reg [15:0] channel_offset,
input  wire aes_channel_status_changed,
output wire clear_channel_status,
output wire [C_ADDR_WIDTH-1:0] buffer_start_address,
output wire clear_dma_transfer_count, //pulse

input wire s2mm_timeout_error,
input wire s2mm_decode_error,
input wire s2mm_slave_error,
input wire ioc_irq_pulse,
input wire [24:0] dma_transfer_count

);

wire start_dma_1;
wire start_dma_pos;
reg start_dma_r;
parameter ONE = 1'b1;

generate if (C_ASYNC_CLOCK)
begin: ASYNC_CLOCKS

xpm_cdc_array_single #(

  //Common module parameters
  .DEST_SYNC_FF   (2), 
  .INIT_SYNC_FF   (0), 
  .SIM_ASSERT_CHK (0), 
  .SRC_INPUT_REG  (0), 
  .WIDTH          (25) 
  ) xpm_cdc_array_single_dma_count (
  .src_clk  (axi4_clk),  
  .src_in   (dma_transfer_count),
  .dest_clk (lite_clk),
  .dest_out (dma_transfer_count_lite)
  );

xpm_cdc_array_single #(

  //Common module parameters
  .DEST_SYNC_FF   (2), 
  .INIT_SYNC_FF   (0), 
  .SIM_ASSERT_CHK (0), 
  .SRC_INPUT_REG  (0), 
  .WIDTH          (C_ADDR_WIDTH) 
  ) xpm_cdc_array_singe_bsa (
  .src_clk  (lite_clk),  
  .src_in   (buffer_start_address_lite),
  .dest_clk (axi4_clk),
  .dest_out (buffer_start_address)
  );

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_CLEAR_COUNT_INST (
  .src_clk    (lite_clk),
  .src_rst    (~lite_resetn),
  .src_pulse  (clear_dma_transfer_count_lite),
  
  .dest_clk   (axi4_clk),
  .dest_rst   (~axi4_resetn),
  .dest_pulse (clear_dma_transfer_count)
);

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_PERIOD_INT_INST (
  .src_clk    (axi4_clk),
  .src_rst    (~axi4_resetn),
  .src_pulse  (ioc_irq_pulse),
  
  .dest_clk   (lite_clk),
  .dest_rst   (~lite_resetn),
  .dest_pulse (ioc_irq_pulse_lite)
);

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_AES_CS_CHANGE_INST (
  .src_clk    (axi4_clk),
  .src_rst    (~axi4_resetn_org),
  .src_pulse  (aes_channel_status_changed),
  
  .dest_clk   (lite_clk),
  .dest_rst   (~lite_resetn),
  .dest_pulse (aes_channel_status_changed_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_SLAVE_ERROR_INST (
  .src_clk   (axi4_clk),
  .src_in    (s2mm_slave_error),
  
  .dest_clk  (lite_clk),
  .dest_out  (s2mm_slave_error_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_CS_CLEAR_INST (
  .src_clk   (lite_clk),
  .src_in    (clear_channel_status_lite),
  
  .dest_clk  (axi4_clk),
  .dest_out  (clear_channel_status)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_DECODE_ERROR_INST (
  .src_clk   (axi4_clk),
  .src_in    (s2mm_decode_error),
  
  .dest_clk  (lite_clk),
  .dest_out  (s2mm_decode_error_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_TIMEOUT_ERROR_INST (
  .src_clk   (axi4_clk),
  .src_in    (s2mm_timeout_error),
  
  .dest_clk  (lite_clk),
  .dest_out  (s2mm_timeout_error_lite)
);


xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_START_DMA_INST (
  .src_clk   (lite_clk),
  .src_in    (start_dma_lite),
  
  .dest_clk  (axi4_clk),
  .dest_out  (start_dma_1)
);

end else
begin: SYNC_CLOCKS

assign start_dma_1 = start_dma_lite;
assign s2mm_timeout_error_lite = s2mm_timeout_error;
assign s2mm_decode_error_lite = s2mm_decode_error;
assign s2mm_slave_error_lite = s2mm_slave_error;
assign clear_channel_status = clear_channel_status_lite;
assign aes_channel_status_changed_lite = aes_channel_status_changed;
assign ioc_irq_pulse_lite = ioc_irq_pulse;
assign clear_dma_transfer_count = clear_dma_transfer_count_lite;
assign buffer_start_address = buffer_start_address_lite;
assign dma_transfer_count_lite = dma_transfer_count;

end
endgenerate

assign start_dma = start_dma_r;
assign start_dma_pos = !start_dma_r && start_dma_1;
always@(posedge axi4_clk) begin
	if(!axi4_resetn) begin
		start_dma_r	<= 1'b0;
	end
	else begin
		start_dma_r	<= start_dma_1;
	end
end

generate if (ONE) begin: CAPTURE_S2MM  

always@(posedge axi4_clk) begin
	if(!axi4_resetn) begin
		no_of_valid_channels	<= 'd0;
		pcm_data_width		<= 'd0;
		ioc_threshold		<= 'd0;
		no_of_periods		<= 'd0;
		period_size		<= 'd0;
		channel_offset		<= 'd0;
		timeout_count		<= 'd0;
	end
	else if(start_dma_pos) begin
		timeout_count		<= timeout_count_lite;
		no_of_valid_channels	<= no_of_valid_channels_lite;
		pcm_data_width		<= pcm_data_width_lite;
		ioc_threshold		<= ioc_threshold_lite;
		no_of_periods		<= no_of_periods_lite;
		period_size		<= period_size_lite;
		channel_offset		<= channel_offset_lite;
	end
end

end
endgenerate


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps
module audio_formatter_v1_0_11_s2mm_buffer #(
 parameter C_NUM_CHANNELS = 2,
 parameter C_DATAWIDTH = 32,
 parameter integer C_S2MM_DATAFORMAT = 1, // 0: AES -> AES
					  // 1: AES -> PCM
					  // 2: PCM -> PCM
 parameter C_PACKAGING_MODE_S2MM = 0 //0: Interleaved; 1: non-interleaved
) (
input axis_clk,
input axis_resetn,

input start_dma,
input [3:0] no_of_valid_channels,
input [2:0] pcm_data_width,

input s_axis_tvalid,
input [7:0] s_axis_tid,
input [31:0] s_axis_wdata,

output reg s_axis_tready,

output m_axis_tvalid,
output [31:0] m_axis_tdata,
output m_axis_tlast,
input m_axis_tready,
output [3:0] m_axis_tid,

output start_command_generation
);


localparam C_INTERLEAVED_DATAWIDTH = (C_DATAWIDTH == 32) ? (C_NUM_CHANNELS << 5) : C_DATAWIDTH;
integer k;

reg write_to_buffer;
reg [31 :0] buffer_write_data;
reg [3:0] count;
reg rd_en_r;
wire [C_NUM_CHANNELS-1 : 0] rd_en;
wire [C_NUM_CHANNELS-1 : 0] data_valid;
wire [31:0] dout [C_NUM_CHANNELS-1 : 0];
reg [31:0] wdata [C_NUM_CHANNELS-1 : 0];
reg [C_NUM_CHANNELS-1 :0] wvalid;
wire [C_NUM_CHANNELS-1 :0] wvalid_dummy;
wire [31:0] read_data [C_NUM_CHANNELS-1 : 0];
wire [31:0] read_fifo_data [C_NUM_CHANNELS-1 : 0];
wire [((C_INTERLEAVED_DATAWIDTH)-1) : 0] interleaved_data;

localparam C_BUFFER_CONSTANT = 8;
localparam  WRITE_IDLE = 2'd0;
localparam  WRITE_ACK = 2'd1;
localparam  WRITE_TO_BUFFER = 2'd2;
localparam  WRITE_DONE = 2'd3;

reg [1:0] write_state;
reg [C_NUM_CHANNELS -1 :0] wr_en;
wire [C_NUM_CHANNELS -1 :0] wr_en_r;
reg [7:0] current_channel;
reg start_reading;
reg start_reading_r;
reg [3:0] rd_channel;
wire rd_en_rr;
wire reading;

wire [C_NUM_CHANNELS-1 : 0] empty;
wire [C_NUM_CHANNELS-1 : 0] wr_rst_busy;
wire [C_NUM_CHANNELS-1 : 0] prog_empty;
wire [C_NUM_CHANNELS-1 : 0] prog_full;
wire [C_NUM_CHANNELS-1 : 0] wr_ack;
wire [4:0] wr_data_count [C_NUM_CHANNELS-1 : 0];
wire [31:0] noninterleaved_data;

assign noninterleaved_data = read_data[rd_channel];
wire [C_NUM_CHANNELS-1:0] prog_full_dummy;

genvar i;
generate for (i=0; i < C_NUM_CHANNELS; i= i+1) begin: FIRST_LEVEL_BUFFERS

assign rd_en[i] = data_valid[i] && rd_en_rr && (rd_channel == i);
assign read_fifo_data[i] = data_valid[i] ? dout[i] : 32'd0;
//assign read_data[i] = (C_S2MM_DATAFORMAT == 1) ? {8'd0, read_fifo_data[i][27:4]} : read_fifo_data[i]; 
assign read_data[i] = read_fifo_data[i]; 
assign interleaved_data[(((i+1)*C_DATAWIDTH) - 1):(i*C_DATAWIDTH)] = read_data[i];
assign prog_full_dummy[i] = (i < no_of_valid_channels) ? prog_full[i] : 1'b1;
assign wvalid_dummy[i] = (i< no_of_valid_channels) ? wvalid[i] : 1'b1;
assign wr_en_r[i] = (i< no_of_valid_channels) ? wr_en[i] : 1'b0;

xpm_fifo_sync #(
      .FIFO_WRITE_DEPTH(16),   // DECIMAL
      .PROG_EMPTY_THRESH(8),    // DECIMAL
      .PROG_FULL_THRESH(C_BUFFER_CONSTANT),     // DECIMAL
      .RD_DATA_COUNT_WIDTH(5),   // DECIMAL
      .WR_DATA_COUNT_WIDTH(5),    // DECIMAL
      .WRITE_DATA_WIDTH(32),     // DECIMAL
      .READ_DATA_WIDTH(32),      // DECIMAL

      .FIFO_READ_LATENCY(0),     // DECIMAL
      .DOUT_RESET_VALUE("0"),    // String
      .ECC_MODE("no_ecc"),       // String
      .FIFO_MEMORY_TYPE("auto"), // String
      .FULL_RESET_VALUE(0),      // DECIMAL
      .READ_MODE("FWFT"),         // String
      .USE_ADV_FEATURES("1717"), // String
      .WAKEUP_TIME(0)           // DECIMAL
   )
   xpm_fifo_sync_inst (
      .almost_empty(),   
      .almost_full(),     
      .dbiterr(),             
      .empty(empty[i]),                 
      .full(),                   
      .overflow(),           
      .rd_rst_busy(),     
      .sbiterr(),             
      .underflow(),         
      .wr_rst_busy(wr_rst_busy[i]),     
      .injectdbiterr(1'b0), 
      .injectsbiterr(1'b0), 
      .sleep(1'b0),                 

      .prog_empty(prog_empty[i]),       
      .prog_full(prog_full[i]),         
      .data_valid(data_valid[i]),       
      .rd_data_count(), 
      .rd_en(rd_en[i]),                 
      .dout(dout[i]),                   

      .rst(!axis_resetn),                     
      .wr_clk(axis_clk),               
      .wr_data_count(wr_data_count[i]), 
      .wr_ack(wr_ack[i]),               
      //.din(s_axis_wdata),                     
      .din(wdata[i]),                     
      .wr_en(wr_en_r[i])                  
   );

   // End of xpm_fifo_sync_inst instantiation
end
endgenerate




always@(posedge axis_clk) begin
	if((!axis_resetn) || (!start_dma)) begin
		start_reading <= 1'b0;
		start_reading_r <= 1'b0;
	end
	else begin
		start_reading_r <= start_reading;
		if (&(prog_full_dummy[C_NUM_CHANNELS-1:0]) || (prog_full[s_axis_tid] && s_axis_tvalid)) begin
			start_reading <= 1'b1;
		end
		//else if ((&empty) && (!rd_en_rr) ) begin
		else if ((&empty) && (!reading) ) begin
			start_reading <= 1'b0;
		end
	end
end

reg skip_data;

/*always@(posedge axis_clk) begin
  if((!axis_resetn) || (|wr_rst_busy) || (!start_dma)) begin
  	write_state <= WRITE_IDLE;
  	wr_en <= 'd0;
	s_axis_tready <= 1'b0;
	current_channel <= 8'd0;
	skip_data	<= 1'b0;
  end
  else begin
	case (write_state)
	WRITE_IDLE:
	begin
		current_channel <= 8'd0;
		skip_data	<= 1'b0;
		if(s_axis_tvalid && (s_axis_tid >= no_of_valid_channels)) begin
			write_state <= WRITE_ACK;
			s_axis_tready <= 1'b1;
			skip_data	<= 1'b1;
		end
		else if(s_axis_tvalid && (!(start_reading || (prog_full[s_axis_tid])))) begin
			write_state <= WRITE_ACK;
			wr_en[s_axis_tid] <= 1'b1;
			s_axis_tready <= 1'b1;
			current_channel <= s_axis_tid;
		end
		else begin
			write_state <= WRITE_IDLE;
			wr_en <= 'd0;
			s_axis_tready <= 1'b0;
		end
	end
	WRITE_ACK:
	begin
		s_axis_tready <= 1'b0;
		wr_en <= 'd0;
		if(wr_ack[current_channel] || skip_data) begin
			write_state <= WRITE_IDLE;
		end
	end
	endcase
  end
end*/
always@(posedge axis_clk) begin
  if((!axis_resetn) || (|wr_rst_busy) || (!start_dma)) begin
  	write_state <= WRITE_IDLE;
  	wr_en <= 'd0;
	for(k =0; k < C_NUM_CHANNELS; k= k+1) begin
		wdata[k] <= 'd0;
	end
	wvalid <= 'd0;
	s_axis_tready <= 1'b0;
	current_channel <= 8'd0;
	skip_data	<= 1'b0;
  end
  else begin
	case (write_state)
	WRITE_IDLE:
	begin
		current_channel <= 8'd0;
		skip_data	<= 1'b0;
		wr_en <= 'd0;
		if(s_axis_tvalid && (s_axis_tid >= no_of_valid_channels)) begin
			write_state <= WRITE_ACK;
			s_axis_tready <= 1'b1;
			skip_data	<= 1'b1;
		end
		else if(&wvalid) begin
			write_state <= WRITE_TO_BUFFER;
			s_axis_tready <= 1'b0;
		end
		else if(s_axis_tvalid && (!(start_reading || (prog_full[s_axis_tid])))) begin
			if(wvalid[s_axis_tid] == 1'b1) begin
				write_state <= WRITE_TO_BUFFER;
				s_axis_tready <= 1'b0;
			end
			else begin
				wvalid[s_axis_tid] <= 1'b1;
				write_state <= WRITE_ACK; 
				s_axis_tready <= 1'b1;
				wdata[s_axis_tid] <= s_axis_wdata;
			end
		end
		else begin
			write_state <= WRITE_IDLE;
			wr_en <= 'd0;
			s_axis_tready <= 1'b0;
		end
	end
	WRITE_TO_BUFFER:
	begin
		wr_en <= {C_NUM_CHANNELS{1'b1}};
		write_state <= WRITE_DONE;
		s_axis_tready <= 1'b0;
	end
	WRITE_DONE:
	begin
		wr_en <= 'd0;
		for(k =0; k < C_NUM_CHANNELS; k= k+1) begin
			wdata[k] <= 'd0;
		end
		write_state <= WRITE_IDLE;
		s_axis_tready <= 1'd0;
		wvalid <= 'd0;
	end
	WRITE_ACK:
	begin
		s_axis_tready <= 1'b0;
		wr_en <= 'd0;
		write_state <= WRITE_IDLE;
	end
	endcase
  end
end

assign start_command_generation = !start_reading_r && start_reading;

generate if (C_PACKAGING_MODE_S2MM == 0)
begin: INTERLEAVED
wire fifo_empty;
wire fifo_wr_ack;
assign rd_en_rr = rd_en_r;
assign reading = rd_en_rr;
/*always@(posedge axis_clk or negedge axis_resetn) begin
  if((!axis_resetn)) begin
	write_to_buffer <= 1'b0;
	buffer_write_data <= 'd0;
	count <= 4'd0;
	rd_en_r <= 'd0;
	rd_channel <= 4'd0;
  end
  else begin
	if(!start_reading_r && start_reading) begin
		count <= 4'd8;
		rd_en_r <= 1'd0;
		rd_channel <= 4'd0;
	end
	else if(start_reading && (count > 'd0))	begin
		if ((count == 4'd8) && fifo_empty) begin
			rd_en_r <= 1'b1;
			count <= 4'd7;
			rd_channel <= 4'd0;
		end
		else if (count < 4'd8) begin
			count <= count - 1'b1;
			rd_en_r <= 1'b1;
		end
	end
	else begin
		count <= 'd0;
		rd_en_r <= 'd0;
	end
	if(rd_en_r) begin
	   write_to_buffer <= 1'b1;
	   buffer_write_data <= interleaved_data;
	end
	else begin
	   write_to_buffer <= 1'b0;
	end
  end
end*/

always@(posedge axis_clk ) begin
  if((!axis_resetn)) begin
	write_to_buffer <= 1'b0;
	buffer_write_data <= 'd0;
	rd_channel 	<= 4'd0;
  end
  else begin
	if(rd_en_r) begin
		write_to_buffer <= 1'b1;
		buffer_write_data <= read_data[rd_channel];
		rd_channel <= (rd_channel == no_of_valid_channels - 1'b1) ? 4'd0 : (rd_channel+1'b1);
	end
	else begin
		write_to_buffer <= 1'b0;
		rd_channel <= 'd0;
	end
  end
end
always@(posedge axis_clk ) begin
  if((!axis_resetn) | (!start_dma)) begin
	rd_en_r <= 1'b0;
	count <= 4'd0;
  end
  else begin
	if(start_command_generation) begin
		count <= 4'd8;
		rd_en_r <= 1'd0;
	end
	else if(start_reading && (count == 4'd8) && fifo_empty) begin
		rd_en_r <= 1'b1;
		count <= 4'd7;
	end
	else if(rd_en_r && (rd_channel == (no_of_valid_channels -1'b1))) begin
		if (count == 4'd0) begin
			rd_en_r <= 1'b0;
			count 	<= 4'd0;
		end
		else begin
			count <= count - 1'b1;
		end
	end

  end
end
localparam FIFO_DEPTH = (C_NUM_CHANNELS == 6) ? (128) : (C_NUM_CHANNELS << 4);
xpm_fifo_sync #(
      .FIFO_WRITE_DEPTH(FIFO_DEPTH),   // DECIMAL
      .PROG_EMPTY_THRESH(8),    // DECIMAL
      .PROG_FULL_THRESH(8),     // DECIMAL
      .RD_DATA_COUNT_WIDTH(6),   // DECIMAL
      .WR_DATA_COUNT_WIDTH(6),    // DECIMAL
      .WRITE_DATA_WIDTH(32),     // DECIMAL
      .READ_DATA_WIDTH(32),      // DECIMAL

      .FIFO_READ_LATENCY(0),     // DECIMAL
      .DOUT_RESET_VALUE("0"),    // String
      .ECC_MODE("no_ecc"),       // String
      .FIFO_MEMORY_TYPE("auto"), // String
      .FULL_RESET_VALUE(0),      // DECIMAL
      .READ_MODE("FWFT"),         // String
      .USE_ADV_FEATURES("1717"), // String
      .WAKEUP_TIME(0)           // DECIMAL
   )
   second_level_xpm_fifo_buffer (
      .almost_empty(),   
      .almost_full(),     
      .dbiterr(),             
      .empty(fifo_empty),                 
      .full(),                   
      .overflow(),           
      .rd_rst_busy(),     
      .sbiterr(),             
      .underflow(),         
      .wr_rst_busy(),     
      .injectdbiterr(1'd0), 
      .injectsbiterr(1'd0), 
      .sleep(1'd0),                 

      .prog_empty(),       
      .prog_full(),         
      .data_valid(m_axis_tvalid),       
      .rd_data_count(), 
      .rd_en(m_axis_tvalid & m_axis_tready),                 
      .dout(m_axis_tdata),                   

      .rst(!axis_resetn || (!start_dma) ),                     
      .wr_clk(axis_clk),               
      .wr_data_count(), 
      .wr_ack(fifo_wr_ack),               
      .din(buffer_write_data),                     
      .wr_en(write_to_buffer)                  
   );

reg [15:0] count_datamover_data;
wire [15:0] samples_per_transaction;
reg [3:0] axis_tid;

assign m_axis_tid = axis_tid;
assign samples_per_transaction = (C_BUFFER_CONSTANT == 8) ? ((no_of_valid_channels) << 3) : ((no_of_valid_channels) << 3);

always@(posedge axis_clk ) begin
    if(!axis_resetn || (!start_dma)) begin
        count_datamover_data <= 'd0;
	axis_tid	<= 'd0;
    end
    else begin
    /*	if(!start_reading_r && start_reading) begin
            count_datamover_data <= samples_per_transaction - 1'b1;
        end*/
        if(m_axis_tvalid && m_axis_tready) begin
		if(count_datamover_data == samples_per_transaction - 1'b1) begin
	            count_datamover_data <= 'd0;
		end
		else begin
	            count_datamover_data <= count_datamover_data + 1'b1;
		end
		if(axis_tid == no_of_valid_channels - 1'b1) begin
			axis_tid	<= 'd0;
		end
		else begin
			axis_tid	<= axis_tid + 1'b1;
		end
        end
    end
end

assign m_axis_tlast = m_axis_tvalid && (count_datamover_data == samples_per_transaction - 1'b1);

end
else
begin: NON_INTERLEAVED
reg rd_ack;

//assign m_axis_tvalid = data_valid[rd_channel] && rd_ack;
assign m_axis_tvalid = rd_ack;
assign rd_en_rr = m_axis_tready && rd_ack;
assign m_axis_tdata = noninterleaved_data; 
assign m_axis_tlast = m_axis_tvalid && (count == 3'd0);
assign m_axis_tid = rd_channel;
assign reading = rd_ack;

always@(posedge axis_clk) begin
  if((!axis_resetn) || (!start_dma)) begin
	count <= 4'd0;
	rd_channel <= 4'd0;
	rd_ack <= 1'b0;
  end
  else begin
	if(!start_reading_r && start_reading) begin
		count <= 4'd7;
		rd_ack <= 1'd1;
		rd_channel <= 4'd0;     
	end
	else if(start_reading && m_axis_tvalid && m_axis_tready)	begin
		if(count == 4'd0 && (rd_channel < (no_of_valid_channels-1'd1) )) begin
		  count <= 4'd7;
		  rd_channel <= rd_channel + 1'b1;
		end
		else if (count == 4'd0 && (rd_channel == (no_of_valid_channels - 1))) begin
		  rd_ack <= 1'b0;
		  rd_channel <= 4'd0;
		end
		else begin
		  count <= count - 1'b1;
		end
	end
	else begin
	   count <= count;
	   rd_ack <= rd_ack;
	   rd_channel <= rd_channel;
	end
  end
end


end
endgenerate

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06/08/2018 03:38:39 PM
// Design Name: 
// Module Name: audio_formatter_v1_0_11_command_generator
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////




module audio_formatter_v1_0_11_s2mm_command_gen
#(
 parameter C_NUM_CHANNELS = 2,
 parameter C_PACKAGING_MODE_S2MM = 0,
 parameter C_ADDR_WIDTH = 32,
 parameter C_CMD_ADDR_WIDTH = 64,
 parameter C_DATAFORMAT = 1
)(
input axis_clk,
input axis_resetn,

input [3:0] no_of_valid_channels,
input [C_ADDR_WIDTH-1: 0] buffer_start_address,
input [15:0] period_size,
input [7:0] no_of_periods,
input [2:0] pcm_data_width,

input start_command_generation, 
input start_dma,
input clear_dma_transfer_count,
input [15:0] input_offset,


output [40+C_CMD_ADDR_WIDTH-1:0] s2mm_cmd_tdata,
output s2mm_cmd_tvalid,
input s2mm_cmd_tready,

input [7:0] s2mm_status,
input s2mm_status_valid,
output s2mm_status_ready,

output [24:0] dma_transfer_count,
output slave_error,
output decode_error,
output ioc_interrupt
 );

assign s2mm_status_ready = 1'b1; 
wire [40+C_CMD_ADDR_WIDTH-1:0] cmd_data;
 reg cmd_valid;
 wire cmd_ready;
wire [C_ADDR_WIDTH-1:0] next_channel0_pointer;
wire [C_ADDR_WIDTH-1:0] next_channel1_pointer;
//wire [15:0] offset0;
//wire [15:0] offset1;
wire [C_ADDR_WIDTH-1:0] next_period_starting_address;
reg [C_ADDR_WIDTH -1:0] current_period_address;
reg start_dma_r;
wire [C_CMD_ADDR_WIDTH - 1:0] current_command_address;

//assign input_offset = offset1;

//assign offset0 = 'd0;
//assign offset1 = period_size >> 1;

//reg [C_ADDR_WIDTH-1:0] channel0_pointer;
//reg [C_ADDR_WIDTH-1:0] channel1_pointer;

reg [7:0] period_count;
reg [15:0] byte_count;

always@(posedge axis_clk)
begin
    if(~axis_resetn)
    begin
        start_dma_r <= 1'b0;
    end
    else begin
        start_dma_r <= start_dma;
    end
end

assign s2mm_cmd_tdata = cmd_data;
assign s2mm_cmd_tvalid = cmd_valid;
assign cmd_ready = s2mm_cmd_tready;

reg [7:0] periods_transferred;
reg [15:0] bytes_transferred;
wire [15:0] bytes_per_transaction;
reg [C_ADDR_WIDTH-1 :0] current_address;
wire [C_ADDR_WIDTH-1 :0] next_address;
wire clear_counts = 1'b0; //clear_dma_transfer_count;
reg [24:0] transfer_count;
reg period_interrupt;

reg [7:0] periods;
reg [15:0] bytes;
reg [24:0] transfer_count_read;

assign dma_transfer_count = transfer_count_read;
assign ioc_interrupt = period_interrupt;
assign slave_error = s2mm_status_valid && s2mm_status[6] && s2mm_status_ready;
assign decode_error = s2mm_status_valid && s2mm_status[5] && s2mm_status_ready;
assign current_command_address = (C_ADDR_WIDTH == C_CMD_ADDR_WIDTH) ? current_address : {{(C_CMD_ADDR_WIDTH-C_ADDR_WIDTH){1'b0}},current_address};
assign cmd_data = {4'd0,1'd0,3'd0,current_command_address,1'b0,1'b1,6'd0,1'b1,7'd0,bytes_per_transaction};

genvar j;
generate if(C_PACKAGING_MODE_S2MM == 0) begin: INTERLEAVED


//wire status_valid = cmd_valid;
//wire status_ready = cmd_ready;
assign bytes_per_transaction =  (C_DATAFORMAT == 1 || C_DATAFORMAT == 2) ? ((pcm_data_width == 'd0) ? (no_of_valid_channels << 3) :
									    (pcm_data_width == 'd1) ? (no_of_valid_channels << 4) :  (no_of_valid_channels << 5)) : 
									   (no_of_valid_channels << 5); //32 bytes per channel

//assign cmd_data = {4'd0,1'd0,3'd0,current_address,1'b0,1'b1,6'd0,1'b1,7'd0,bytes_per_transaction};
assign next_address = ((byte_count == 0) && (period_count == no_of_periods)) ? buffer_start_address : (current_address + bytes_per_transaction);

always@(posedge axis_clk )
begin
    if(~axis_resetn || (!start_dma))
    begin
        period_count <= 8'd0;
        byte_count <= 16'd0;
        current_address <= 'd0;
        cmd_valid <= 1'b0;
    end
    else begin
        if(!start_dma_r && start_dma) begin
            period_count <= 8'd1;
            byte_count <= period_size;
            current_address <= buffer_start_address;
        end
        else if (start_command_generation) begin
            byte_count <= byte_count - bytes_per_transaction;
            cmd_valid <= 1'b1;
        end
        else if (cmd_valid && cmd_ready) begin
            cmd_valid <= 1'b0;
            current_address <= next_address;
            if(byte_count == 'd0) begin
                period_count <= (period_count == no_of_periods) ? 8'd1 : (period_count + 1'b1);
                byte_count <= period_size;
            end 
        end
    end
end

end
else 
begin: NON_INTERLEAVED
reg [3:0] channel;
wire [C_ADDR_WIDTH-1 :0] next_period_address;
reg [C_ADDR_WIDTH - 1:0] channel_pointer [C_NUM_CHANNELS-1:0];
wire [C_ADDR_WIDTH - 1:0] next_channel_pointer [C_NUM_CHANNELS-1:0];
wire [15:0] offset [C_NUM_CHANNELS-1:0];
assign offset[0] = 'd0;
assign offset[1] = input_offset;
integer i;

for(j=0;j < C_NUM_CHANNELS; j=j+1) begin: OFFSET_CALC
    if(j > 1) begin
  assign      offset[j] = offset[j-1] + input_offset;
    end
assign next_channel_pointer[j] = (byte_count == 16'd0) ? (next_period_address + offset[j]) : channel_pointer[j] + bytes_per_transaction;
end

//assign next_channel0_pointer = (byte_count == 16'd0) ? (next_period_address + offset0) : channel0_pointer + bytes_per_transaction;
//assign next_channel1_pointer = (byte_count == 16'd0) ? (next_period_address + offset1) : channel1_pointer + bytes_per_transaction;
assign next_period_address = (period_count == no_of_periods) ? buffer_start_address : (current_period_address + period_size);

assign bytes_per_transaction = ((C_DATAFORMAT == 1) || (C_DATAFORMAT == 2)) ? ((pcm_data_width == 'd0) ? 16'd8 :
							                      (pcm_data_width == 'd1) ? 16'd16 : 16'd32) : 16'd32;  

//assign cmd_data = {4'd0,1'd0,3'd0,current_address,1'b0,1'b1,6'd0,1'b1,7'd0,bytes_per_transaction};
always@(posedge axis_clk )
begin
    if(~axis_resetn || (~start_dma))
    begin
       current_period_address <= 'd0;
       for(i=0; i<C_NUM_CHANNELS; i= i+1) begin
       channel_pointer[i] <= 'd0;
       end
    //   channel0_pointer <= 'd0;
    //   channel1_pointer <= 'd0;
       cmd_valid <= 1'b0;
       byte_count <= 'd0;
       period_count <= 'd0;
       channel <= 4'd0;
       current_address <= 'd0; 
    end
    else begin
        if(!start_dma_r && start_dma) begin
            current_period_address <= buffer_start_address;
             for(i=0; i<C_NUM_CHANNELS; i= i+1) begin
               channel_pointer[i] <= buffer_start_address + offset[i];
             end           
       //     channel0_pointer <= buffer_start_address + offset0;
       //     channel1_pointer <= buffer_start_address + offset1;
            channel <= 4'd0;
            cmd_valid <= 1'b0;
            byte_count <= period_size;
            period_count <= 8'd1;
            current_address <= 'd0; 
        end
        else if (start_command_generation) begin
           // current_address <= channel0_pointer;
            current_address <= channel_pointer[0];
            channel <= 4'd0;
            byte_count <= byte_count - bytes_per_transaction;
            cmd_valid <= 1'b1;
        end
        else if (cmd_valid && cmd_ready) begin
            if(channel < no_of_valid_channels - 1'b1) begin
             //   current_address <= channel1_pointer;
                current_address <= channel_pointer[channel + 1];
                byte_count <= byte_count - bytes_per_transaction;
                channel <= channel + 1'b1;
                cmd_valid <= 1'b1;
            end
            else begin
                cmd_valid <= 1'b0;
                for(i=0; i<no_of_valid_channels; i= i+1) begin
                     channel_pointer[i] <= next_channel_pointer[i];
                end    
             //   channel0_pointer <= next_channel0_pointer;
             //   channel1_pointer <= next_channel1_pointer;
                if(byte_count == 'd0) begin
                    period_count <= (period_count == no_of_periods) ? 8'd1 : (period_count + 1'b1);
                    byte_count <= period_size;
                    current_period_address <= next_period_address;
                end       
            end
        end
    end
end

end
endgenerate

always@(posedge axis_clk) begin
    if((~axis_resetn))
    begin
	transfer_count_read <= 25'd0;
	periods		    <= 'd0;
	bytes		    <= 'd0;
    end
    else if(!start_dma)
    begin
	transfer_count_read <= 25'd0;
	bytes		    <= 'd0;
	periods		    <= 'd0;
    end
  /*  else if(clear_counts)
    begin
	transfer_count_read <= 25'd0;
	periods		    <= 'd0;
	bytes		    <= 'd0;
    end*/
    else begin
        if (s2mm_status_valid && s2mm_status_ready) begin
            if(bytes + bytes_per_transaction == period_size) begin
		bytes	    <= 'd0;
		if(periods == no_of_periods - 1'b1) begin
			periods		    <= 'd0;
			transfer_count_read <= 'd0;
	        end
		else begin
			periods		    <= periods + 1'b1;
			transfer_count_read <= transfer_count_read + bytes_per_transaction;
		end
            end else begin
                bytes <= bytes + bytes_per_transaction;
	    	transfer_count_read <= transfer_count_read + bytes_per_transaction;
	    end
    	end
    end
end


always@(posedge axis_clk) begin
    if((~axis_resetn))
    begin
        periods_transferred <= 8'd0;
        bytes_transferred <= 16'd0;
	transfer_count <= 25'd0;
	period_interrupt <= 1'b0;
    end
    else if((!start_dma))
    begin
        periods_transferred <= 8'd0;
        bytes_transferred <= 16'd0;
	transfer_count <= 25'd0;
	period_interrupt <= 1'b0;
    end
    else begin
	period_interrupt <= 1'b0;
        if (s2mm_status_valid && s2mm_status_ready) begin
            if(bytes_transferred + bytes_per_transaction == period_size) begin
                bytes_transferred <= 'd0;
		period_interrupt <= 1'b1;
		if(periods_transferred == no_of_periods - 1'b1) begin
	    		transfer_count <= 'd0;
                	periods_transferred <= 'd0;
		end
		else begin
	    		transfer_count <= transfer_count + bytes_per_transaction;
                	periods_transferred <= periods_transferred + 1'b1;
		end
            end else begin
                bytes_transferred <= bytes_transferred + bytes_per_transaction;
	    	transfer_count <= transfer_count + bytes_per_transaction;
            end
        end
    end
end


endmodule




// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps

module audio_formatter_v1_0_11_mm2s_top #(
	parameter C_FAMILY = "virtex7" , 
	parameter integer C_MAX_NUM_CHANNELS_MM2S = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_MM2S = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_MM2S_DATAFORMAT = 1, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
	parameter integer C_MM2S_ADDR_WIDTH = 64, //32,64
 	parameter integer C_MM2S_ASYNC_CLOCK = 1
)(
	input 		aud_clk,
	input 		aud_resetn,

	input 		s_axi_lite_aclk,
	input 		s_axi_lite_aresetn,

	input  		s_axi_lite_awvalid, 
	output 		s_axi_lite_awready, 
	input  	[11:0]	s_axi_lite_awaddr , 

	input  		s_axi_lite_wvalid , 
	output 		s_axi_lite_wready , 
	input  	[31:0]	s_axi_lite_wdata  , 
	                    
	output  [1:0] 	s_axi_lite_bresp  , 
	output   	s_axi_lite_bvalid , 
	input 		s_axi_lite_bready , 
	                    
/*	input 		s_axi_lite_arvalid, 
	output		s_axi_lite_arready, 
	input 	[11:0]	s_axi_lite_araddr , 
	                    
	output 		s_axi_lite_rvalid , 
	input 		s_axi_lite_rready , 
	output	[31:0]	s_axi_lite_rdata  , 
	                    
	output  [1:0] 	s_axi_lite_rresp  ,*/ 

	input 	[7:0]	s_axi_lite_araddr , 
	output	[31:0]	s_axi_lite_rdata  , 
	output	      	s_axi_lite_rNOK   ,
	input 		read_trans, 

//------MM2S-----------------
	input 		m_axis_mm2s_aclk,
	input 		m_axis_mm2s_aresetn,
	output 		Irq_mm2s,
                         
// AXIS MM2S Channel                                                             
	output 		m_axis_mm2s_tvalid,
	input 		m_axis_mm2s_tready,
	output	[31:0]	m_axis_mm2s_tdata,
	output 	[7:0]	m_axis_mm2s_tid,

//AXI MM2S Channel
        output 	[C_MM2S_ADDR_WIDTH-1:0]	m_axi_mm2s_araddr,
        output 	[7:0]	m_axi_mm2s_arlen,
        output 	[2:0]	m_axi_mm2s_arsize,
        output 	[1:0]	m_axi_mm2s_arburst,
        output 	[2:0]	m_axi_mm2s_arprot,
        output 	[3:0]	m_axi_mm2s_arcache,
        output 	[3:0]	m_axi_mm2s_aruser,
        output 		m_axi_mm2s_arvalid,
        input 		m_axi_mm2s_arready,
         		                       
        input 	[31:0]	m_axi_mm2s_rdata,
        input 	[1:0]	m_axi_mm2s_rresp,
        input 		m_axi_mm2s_rlast       ,
        input 		m_axi_mm2s_rvalid,
        output 		m_axi_mm2s_rready
  		                
);

wire [3:0] no_of_valid_channels_lite;
wire [2:0] pcm_data_width_lite;
wire [7:0] ioc_threshold_lite;
wire soft_reset_lite;
wire start_dma_lite;

wire mm2s_decode_error_lite;
wire mm2s_slave_error_lite;
wire ioc_irq_pulse_lite;
//wire aes_channel_status_changed_lite;

wire [15:0] fs_multiplier_value_lite;
wire [7:0] no_of_periods_lite;
wire [15:0] period_size_lite;

wire [C_MM2S_ADDR_WIDTH-1 :0] buffer_start_address_lite;
//wire [7:0] no_of_periods_transferred_lite;
wire [24:0] dma_transfer_count_lite;
wire [191:0] aes_channel_status_lite;
wire clear_dma_transfer_count_lite;
wire [15:0] channel_offset_lite;

wire [3:0] no_of_valid_channels;
wire [2:0] pcm_data_width;
wire [7:0] ioc_threshold;
wire soft_reset;
wire start_dma;

wire mm2s_decode_error;
wire mm2s_slave_error;
wire ioc_irq_pulse;
//wire aes_channel_status_changed = 1'b0;

wire [15:0] fs_multiplier_value;
wire [7:0] no_of_periods;
wire [15:0] period_size;
wire [15:0] channel_offset;

wire [C_MM2S_ADDR_WIDTH-1 :0] buffer_start_address;
//wire [7:0] no_of_periods_transferred;
wire [24:0] dma_transfer_count;
wire [191:0] aes_channel_status;
wire clear_dma_transfer_count;

wire halted;
wire soft_reset_clr;
wire soft_reset_core;
wire halt_dm;
wire halt_complete_dm;

wire reset_gen;
assign reset_gen = m_axis_mm2s_aresetn && (~soft_reset_core);

assign soft_reset = soft_reset_lite;

audio_formatter_v1_0_11_mm2s_registers #(
 .C_ADDR_WIDTH(C_MM2S_ADDR_WIDTH)
) mm2s_registers_1
(
  // AXI4-Lite bus (cpu control)
  .iAxiClk               (s_axi_lite_aclk),
  .iAxiResetn            (s_axi_lite_aresetn),
  // - Write address
  .iAxi_AWValid          (s_axi_lite_awvalid),
  .oAxi_AWReady          (s_axi_lite_awready),
  .iAxi_AWAddr           (s_axi_lite_awaddr),
  // - Write data
  .iAxi_WValid           (s_axi_lite_wvalid),
  .oAxi_WReady           (s_axi_lite_wready),
  .iAxi_WData            (s_axi_lite_wdata),
  // - Write response
  .oAxi_BValid           (s_axi_lite_bvalid),
  .iAxi_BReady           (s_axi_lite_bready),
  .oAxi_BResp            (s_axi_lite_bresp),
  // - Read address   
/*  .iAxi_ARValid          (s_axi_lite_arvalid),
  .oAxi_ARReady          (s_axi_lite_arready),
  .iAxi_ARAddr           (s_axi_lite_araddr),
  // - Read data/response
  .oAxi_RValid           (s_axi_lite_rvalid),
  .iAxi_RReady           (s_axi_lite_rready), 
  .oAxi_RData            (s_axi_lite_rdata),
  .oAxi_RResp            (s_axi_lite_rresp),*/
  
  .rReadAddr		 (s_axi_lite_araddr),
  .nReadData		 (s_axi_lite_rdata ),
  .ReadAddrNOK		 (s_axi_lite_rNOK  ),

  // IRQ
  .Irq_mm2s                  (Irq_mm2s),
  
  // In/Out signals
.no_of_valid_channels	(no_of_valid_channels_lite),
.pcm_data_width		(pcm_data_width_lite),
.ioc_threshold		(ioc_threshold_lite),
.reset			(soft_reset_lite),
.run_stop		(start_dma_lite),

.imm2s_decode_error	(mm2s_decode_error_lite),
.imm2s_slave_error	(mm2s_slave_error_lite),
.ioc_irq_pulse		(ioc_irq_pulse_lite),
//.aes_channel_status_changed	(aes_channel_status_changed_lite),

.fs_multiplier_value	(fs_multiplier_value_lite),
.no_of_periods		(no_of_periods_lite),
.period_size		(period_size_lite),
.channel_offset		(channel_offset_lite),

.buffer_start_address	(buffer_start_address_lite),

//.no_of_periods_transferred	(no_of_periods_transferred_lite),
.clear_dma_transfer_count	(clear_dma_transfer_count_lite),
.dma_transfer_count	(dma_transfer_count_lite),
.aes_channel_status_axi	(aes_channel_status_lite),

.halted			(halted),
.soft_reset_clr		(soft_reset_clr)
);

audio_formatter_v1_0_11_reset #(
	.C_ASYNC_CLOCK(C_MM2S_ASYNC_CLOCK)
) reset_mm2s 
(
 .lite_clk(s_axi_lite_aclk),
 .lite_resetn(s_axi_lite_aresetn),

 .axi4_clk(m_axis_mm2s_aclk),
 .axi4_resetn(m_axis_mm2s_aresetn),

 .start_dma_lite	(start_dma_lite),
 .soft_reset_lite	(soft_reset_lite),
 .halt_complete_dm	(halt_complete_dm),
 
 .soft_reset_clear	(soft_reset_clr),
 .soft_reset_core	(soft_reset_core),
 .halt_dm		(halt_dm),
 .halted		(halted)
);

audio_formatter_v1_0_11_mm2s_sync #(
	.C_ADDR_WIDTH(C_MM2S_ADDR_WIDTH),
	.C_ASYNC_CLOCK(C_MM2S_ASYNC_CLOCK)
) mm2s_cdc_1
(
	.aud_clk(aud_clk),
	.aud_resetn(aud_resetn),

 .lite_clk(s_axi_lite_aclk),
 .lite_resetn(s_axi_lite_aresetn),

 .axi4_clk(m_axis_mm2s_aclk),
 .axi4_resetn(reset_gen),

 .no_of_valid_channels_lite	(no_of_valid_channels_lite),
 .pcm_data_width_lite		(pcm_data_width_lite),
 .ioc_threshold_lite		(ioc_threshold_lite),
 .start_dma_lite		(start_dma_lite),
 
 .fs_multiplier_value_lite 		(fs_multiplier_value_lite),
 .no_of_periods_lite			(no_of_periods_lite),
 .period_size_lite			(period_size_lite),
 .channel_offset_lite			(channel_offset_lite),
 .buffer_start_address_lite		(buffer_start_address_lite),
 .aes_channel_status_lite		(aes_channel_status_lite),
 .clear_dma_transfer_count_lite		(clear_dma_transfer_count_lite),

 .mm2s_decode_error_lite	(mm2s_decode_error_lite),
 .mm2s_slave_error_lite		(mm2s_slave_error_lite),
 .ioc_irq_pulse_lite		(ioc_irq_pulse_lite),
 .dma_transfer_count_lite	(dma_transfer_count_lite),
//-----------------------------------------
 .no_of_valid_channels	(no_of_valid_channels),
 .pcm_data_width	(pcm_data_width),
 .ioc_threshold		(ioc_threshold),
 .start_dma		(start_dma),
 
 .fs_multiplier_value 		(fs_multiplier_value),
 .no_of_periods			(no_of_periods),
 .period_size			(period_size),
 .channel_offset		(channel_offset),
 .buffer_start_address		(buffer_start_address),
 .aes_channel_status		(aes_channel_status),
 .clear_dma_transfer_count	(clear_dma_transfer_count),

 .mm2s_decode_error	(mm2s_decode_error),
 .mm2s_slave_error	(mm2s_slave_error),
 .ioc_irq_pulse		(ioc_irq_pulse),
 .dma_transfer_count	(dma_transfer_count)
);


audio_formatter_v1_0_11_mm2s #(
	.C_FAMILY(C_FAMILY),
	.C_MAX_NUM_CHANNELS_MM2S(C_MAX_NUM_CHANNELS_MM2S),
	.C_PACKING_MODE_MM2S(C_PACKING_MODE_MM2S),
	.C_MM2S_DATAFORMAT(C_MM2S_DATAFORMAT),
	.C_MM2S_ADDR_WIDTH(C_MM2S_ADDR_WIDTH)
) mm2s_logic_1
(
	.aud_clk(aud_clk),
	.aud_resetn(aud_resetn),

	.mm2s_clk	(m_axis_mm2s_aclk),
	.mm2s_resetn	(reset_gen),

	.start_dma		(start_dma),
	.buffer_start_address	(buffer_start_address),
	.period_size		(period_size),
	.no_of_periods		(no_of_periods),
	.no_of_valid_channels	(no_of_valid_channels),
	.pcm_data_width		(pcm_data_width),
	.channel_offset		(channel_offset),
	.fs_multiplier_value	(fs_multiplier_value),
	.aes_channel_status	(aes_channel_status),

	.clear_dma_transfer_count	(clear_dma_transfer_count),

	.dma_transfer_count		(dma_transfer_count),
	.ioc_interrupt			(ioc_irq_pulse),
	.slave_error			(mm2s_slave_error),
	.decode_error			(mm2s_decode_error),

 	.halt_complete_dm	(halt_complete_dm),
 	.halt_dm		(halt_dm),

      .m_axis_tdata           (m_axis_mm2s_tdata        ),
      .m_axis_tid             (m_axis_mm2s_tid        ),
      .m_axis_tvalid          (m_axis_mm2s_tvalid       ),
      .m_axis_tready          (m_axis_mm2s_tready       ),

      .m_axi_mm2s_araddr           (m_axi_mm2s_araddr              ),
      .m_axi_mm2s_arlen            (m_axi_mm2s_arlen               ),
      .m_axi_mm2s_arsize           (m_axi_mm2s_arsize              ),
      .m_axi_mm2s_arburst          (m_axi_mm2s_arburst             ),
      .m_axi_mm2s_arprot           (m_axi_mm2s_arprot              ),
      .m_axi_mm2s_arcache          (m_axi_mm2s_arcache             ),
      .m_axi_mm2s_aruser           (m_axi_mm2s_aruser             ),
      .m_axi_mm2s_arvalid          (m_axi_mm2s_arvalid             ),
      .m_axi_mm2s_arready          (m_axi_mm2s_arready             ),

      .m_axi_mm2s_rdata            (m_axi_mm2s_rdata               ),
      .m_axi_mm2s_rresp            (m_axi_mm2s_rresp               ),
      .m_axi_mm2s_rlast            (m_axi_mm2s_rlast               ),
      .m_axi_mm2s_rvalid           (m_axi_mm2s_rvalid              ),
      .m_axi_mm2s_rready           (m_axi_mm2s_rready              )

);


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps


module audio_formatter_v1_0_11_mm2s_registers 
#(
 parameter C_ADDR_WIDTH = 64

)
(

  // AXI4-Lite bus (cpu control)
  input             iAxiClk,
  input             iAxiResetn,
  // - Write address
  input             iAxi_AWValid,
  output reg        oAxi_AWReady,
  input      [11:0] iAxi_AWAddr,
  // - Write data
  input             iAxi_WValid,
  output reg        oAxi_WReady,
  input      [31:0] iAxi_WData,
  // - Write response
  output reg        oAxi_BValid,
  input             iAxi_BReady,
  output reg [ 1:0] oAxi_BResp,

/*  // - Read address   
  input             iAxi_ARValid,
  output reg        oAxi_ARReady,
  input      [ 7:0] iAxi_ARAddr,
  // - Read data/response
  output reg        oAxi_RValid,
  input             iAxi_RReady, 
  output reg [31:0] oAxi_RData,
  output reg [ 1:0] oAxi_RResp,
*/

input      [ 7:0] rReadAddr,
output reg [31:0] nReadData,
output reg        ReadAddrNOK,

output reg [3:0] no_of_valid_channels,
output reg [2:0] pcm_data_width,
output reg [7:0] ioc_threshold,
output reg reset,
output reg run_stop,

input wire imm2s_decode_error,
input wire imm2s_slave_error,
input ioc_irq_pulse,
//input aes_channel_status_changed,

output reg [15:0] fs_multiplier_value,
output reg [7:0] no_of_periods,
output reg [15:0] period_size,
output wire [15:0] channel_offset,

output wire [C_ADDR_WIDTH-1:0] buffer_start_address,

input halted,
input soft_reset_clr,
//input [7:0] no_of_periods_transferred,
input [24:0] dma_transfer_count,
output [191:0] aes_channel_status_axi,
output clear_dma_transfer_count,
output Irq_mm2s
);

localparam cAXI4_RESP_OKAY   = 2'b00; // Okay
localparam cAXI4_RESP_SLVERR = 2'b10; // Slave error
localparam cADDR_CTRL 	      = 'h110; //Control register
localparam cADDR_STS  	      = 'h114; //Status register
localparam cADDR_MULTIPLIER   = 'h118; //Multiplier register
localparam cADDR_PERIOD_CFG   = 'h11C; //Period configuration
localparam cADDR_BUFFER_LSB   = 'h120; //LSB Buffer Address
localparam cADDR_BUFFER_MSB   = 'h124; //MSB Buffer Address
localparam cADDR_DMA_COUNT    = 'h128; //DMA MM2S transfer count

localparam cADDR_AES_CHSTS_1  = 'h12C; // AES Channel Status 1
localparam cADDR_AES_CHSTS_2  = 'h130; // AES Channel Status 2
localparam cADDR_AES_CHSTS_3  = 'h134; // AES Channel Status 3
localparam cADDR_AES_CHSTS_4  = 'h138; // AES Channel Status 4
localparam cADDR_AES_CHSTS_5  = 'h13C; // AES Channel Status 5
localparam cADDR_AES_CHSTS_6  = 'h140; // AES Channel Status 6

localparam cADDR_SIZE_PER_CHANNEL    = 'h144; //DMA MM2S size per channel

// Irq Generation
reg ioc_irq;
wire err_irq;
//reg halted;
//wire halted = 1'b0;
reg ioc_irq_en;
reg err_irq_en;
//reg timeout_irq_en;
//reg timeout_err;
reg clear_ioc_irq;
reg clear_channel_status;

reg mm2s_decode_error;
reg mm2s_slave_error;
reg clear_transfer_count;
reg [15:0] size_per_channel;

reg clear_transfer_count_r;

always@(posedge iAxiClk) 
begin
 if (!iAxiResetn) begin
   clear_transfer_count_r <= 1'b0;
 end
 else begin
   clear_transfer_count_r <= clear_transfer_count;
 end
end

assign channel_offset = size_per_channel;
assign clear_dma_transfer_count = (!clear_transfer_count) && clear_transfer_count_r;
always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	ioc_irq <= 1'b0;
  end
  else begin
	if(ioc_irq_pulse) begin
		ioc_irq <= 1'b1;
	end
	else if(clear_ioc_irq) begin
		ioc_irq <= 1'b0;
	end
  end
end

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	mm2s_decode_error <= 1'b0;
  end
  else begin
	if(imm2s_decode_error) begin
		mm2s_decode_error <= 1'b1;
	end
  end
end

always@(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr) begin
	mm2s_slave_error <= 1'b0;
  end
  else begin
	if(imm2s_slave_error) begin
		mm2s_slave_error <= 1'b1;
	end
  end
end

/*always@(posedge iAxiClk)
begin
  if (!iAxiResetn) begin
	timeout_err <= 1'b0;
  end
  else begin
	if(itimeout_err) begin
		timeout_err <= 1'b1;
	end
  end
end
*/
assign err_irq = err_irq_en && (mm2s_slave_error || mm2s_decode_error);
assign Irq_mm2s = ioc_irq || err_irq; // || (timeout_err && timeout_irq_en);

// Input Capture
reg [191:0] aes_channel_status;
assign aes_channel_status_axi = aes_channel_status;
/*always@(posedge iAxiClk)
begin
  if (!iAxiResetn) begin
    aes_channel_status   <= 192'h0;
  end
  else begin
    if (aes_channel_status_changed) begin
      aes_channel_status <= aes_channel_status_axi;
    end
  end
end
*/

////////////////////////////////////////////////////////
// Write channel

localparam sWriteReset = 3'd0;
localparam sWriteAddr = 3'd1;
localparam sWriteData = 3'd2;
localparam sWriteResp = 3'd3;
             
reg [2:0] stmWrite;

reg [11:0] rWriteAddr;

// Statemachine for taking care of the write signals
always @(posedge iAxiClk)
begin
  if (!iAxiResetn)
  begin
    oAxi_AWReady        <= 1'b0;
    oAxi_WReady         <= 1'b0;
    oAxi_BValid         <= 1'b0;
    rWriteAddr          <=  'h0;
    stmWrite            <= sWriteReset;
  end
  else
  begin
    case (stmWrite) 
      sWriteReset :
      begin
        oAxi_AWReady    <= 1'b1;
        oAxi_WReady     <= 1'b0;
        oAxi_BValid     <= 1'b0;
        stmWrite        <= sWriteAddr;
      end
      
      sWriteAddr :
      begin
        oAxi_AWReady    <= 1'b1;
        if (iAxi_AWValid)
        begin
          oAxi_AWReady  <= 1'b0;
          oAxi_WReady   <= 1'b1;
          rWriteAddr    <= iAxi_AWAddr;
          stmWrite      <= sWriteData;
        end
      end
      
      sWriteData :
      begin
        oAxi_WReady     <= 1'b1;
        
        if (iAxi_WValid)
        begin
          oAxi_WReady   <= 1'b0;
          oAxi_BValid   <= 1'b1;
          stmWrite      <= sWriteResp;
        end
      end
      
      sWriteResp :
      begin
        oAxi_BValid     <= 1'b1;
        if (iAxi_BReady)
        begin
          oAxi_BValid   <= 1'b0;
          stmWrite      <= sWriteReset;
        end
      end 
      
      default :
        stmWrite        <= sWriteReset;
    endcase
  end
end

reg [63:0] buffer_address;
assign buffer_start_address = buffer_address[C_ADDR_WIDTH - 1:0];


wire [31:0] ctrl_reg = {
	9'd0,
	no_of_valid_channels,
	pcm_data_width,
	2'd0,
	ioc_irq_en,
	err_irq_en,
//	ioc_threshold,
	8'd0,
	2'd0,
	reset,
	run_stop
};

wire [31:0] status_reg = {
	ioc_irq,
	err_irq,
	11'd0,
	mm2s_decode_error,
	mm2s_slave_error,
	16'd0,
	halted
};

wire [31:0] multiplier_reg = {
 16'd0,
 fs_multiplier_value
};

wire [31:0] period_confg_reg = {
	8'd0,
	no_of_periods,
	period_size
};

wire [31:0] buffer_addr_lsb_reg = {
	buffer_address[31:0]
};

wire [31:0] buffer_addr_msb_reg = {
	buffer_address[63:32]
};

wire [31:0] mm2s_transfer_count_reg = {
	7'd0,
//	no_of_periods_transferred,
	dma_transfer_count
};

wire [31:0] mm2s_size_per_channel_reg = {
	16'd0,
	size_per_channel
};

// Write address decoder
always @(posedge iAxiClk)
begin
  if (!iAxiResetn || soft_reset_clr)
  begin
    oAxi_BResp        <= cAXI4_RESP_OKAY;
    clear_ioc_irq   <= 1'b0; 
  //  clear_err_irq   <= 1'b0; 
  //  clear_channel_status <= 1'b0;

		no_of_valid_channels 	<= 4'd2;
		pcm_data_width 		<= 3'd2;
		ioc_irq_en		<= 1'b1;
		err_irq_en		<= 1'b0;
		ioc_threshold 		<= 8'd1;
		reset			<= 1'b0;
		run_stop		<= 1'b0;
          	fs_multiplier_value    	<= 'h180;

		no_of_periods		<= 'd1;
		period_size 		<= 'd0;

		buffer_address		<= 'd0;
		size_per_channel	<= 16'd0;
		aes_channel_status	<= 'd0;
  end
  else
  begin
    // Defaults
      clear_ioc_irq   <= 1'b0; 
    if (oAxi_WReady && iAxi_WValid)
    begin
      oAxi_BResp      <= cAXI4_RESP_OKAY;
    //  clear_err_irq   <= 1'b0; 
    //  clear_channel_status <= 1'b0;
      case (rWriteAddr)
        cADDR_CTRL :
        begin
		no_of_valid_channels 	<= iAxi_WData[22:19];
		pcm_data_width 		<= iAxi_WData[18:16];
		ioc_irq_en		<= iAxi_WData[13];
		err_irq_en		<= iAxi_WData[12];
		ioc_threshold 		<= iAxi_WData[11:4];
		reset			<= iAxi_WData[1];
		run_stop		<= iAxi_WData[0];
        end
        cADDR_STS :
        begin
		clear_ioc_irq 		<= iAxi_WData[31];
//		clear_err_irq		<= iAxi_WData[30];
        end
        cADDR_MULTIPLIER :
        begin
          	fs_multiplier_value    	<= iAxi_WData[15:0];
        end
	cADDR_PERIOD_CFG :
	begin
		no_of_periods		<= iAxi_WData[23:16];
		period_size		<= iAxi_WData[15:0];
	end
	cADDR_BUFFER_LSB :
	begin
		buffer_address[31:0] 	<= iAxi_WData[31:0];
	end
	cADDR_BUFFER_MSB :
	begin
		buffer_address[63:32] 	<= iAxi_WData[31:0];
	end
	cADDR_DMA_COUNT :
	begin
	end
	cADDR_AES_CHSTS_1 :
	begin
		aes_channel_status[31:0]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_2 :
	begin
		aes_channel_status[63:32]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_3 :
	begin
		aes_channel_status[95:64]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_4 :
	begin
		aes_channel_status[127:96]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_5 :
	begin
		aes_channel_status[159:128]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_AES_CHSTS_6 :
	begin
		aes_channel_status[191:160]	<= iAxi_WData[31:0];
		//clear_channel_status 	<= 1'b1;
	end
	cADDR_SIZE_PER_CHANNEL :
	begin
		size_per_channel	<= iAxi_WData[15:0];
	end	
        default :
	begin
	    if((rWriteAddr[1:0] == 'd0) && (rWriteAddr[8] == 'b0) && (rWriteAddr[7:0] >= 'h10) && (rWriteAddr[7:0] <= 'h44))
            begin
		oAxi_BResp <= cAXI4_RESP_OKAY; //For S2MM registers	
	    end
	    else
	    begin
	        oAxi_BResp <= cAXI4_RESP_SLVERR;
	    end
	end
      endcase
    end
  end
end

/*
localparam sReadReset = 2'd0;
localparam sReadAddr = 2'd1;
localparam sDecodeAddr = 2'd2;
localparam sReadData = 2'd3;
             
reg [1:0] stmRead;

reg        ReadAddrNOK;
reg [ 7:0] rReadAddr;
reg [31:0] nReadData;

// Statemachine for taking care of the read signals
always @(posedge iAxiClk)
begin
  if (!iAxiResetn)
  begin
    oAxi_ARReady        <= 1'b0;    
    oAxi_RResp          <= cAXI4_RESP_OKAY;
    oAxi_RValid         <= 1'b0;
    oAxi_RData          <=  'h0;
    rReadAddr           <=  'h0;
    stmRead             <= sReadReset;
  end
  else
  begin
    case (stmRead) 
      sReadReset :
      begin
        oAxi_ARReady    <= 1'b1;
        oAxi_RResp      <= cAXI4_RESP_OKAY;
        oAxi_RValid     <= 1'b0;
        oAxi_RData      <=  'h0;
        rReadAddr       <=  'h0;
        stmRead         <= sReadAddr;
      end
      
      sReadAddr :
      begin
        oAxi_ARReady    <= 1'b1;
        if (iAxi_ARValid)
        begin
          oAxi_ARReady  <= 1'b0;
          rReadAddr     <= iAxi_ARAddr;
          stmRead       <= sDecodeAddr;
        end
      end
      
      sDecodeAddr :
      begin
        if (ReadAddrNOK)
          oAxi_RResp    <= cAXI4_RESP_SLVERR;
        else
          oAxi_RResp    <= cAXI4_RESP_OKAY;
          
        oAxi_RData      <= nReadData;
        oAxi_RValid     <= 1'b1;
        stmRead         <= sReadData;
      end
      
      sReadData :
      begin
        oAxi_RValid     <= 1'b1;
        if (iAxi_RReady)
        begin
          oAxi_RValid   <= 1'b0;
          stmRead       <= sReadReset;
        end
      end
      
      default :
        stmRead         <= sReadReset;
    endcase
  end
end
*/

// Read address decoder
always@(*)
begin
  ReadAddrNOK        = 1'b0;
  nReadData          =  'h0;
  clear_transfer_count = 1'b0;
  case (rReadAddr)
    cADDR_CTRL[7:0] :
    begin
      nReadData   	= ctrl_reg;
    end
    cADDR_STS[7:0] :
    begin
      nReadData		= status_reg;
    end
    cADDR_MULTIPLIER[7:0] :
    begin
      nReadData		= multiplier_reg;
    end
    cADDR_PERIOD_CFG[7:0] :
    begin
      nReadData		= period_confg_reg;
    end
    cADDR_BUFFER_LSB[7:0] :
    begin
      nReadData		= buffer_addr_lsb_reg;
    end
    cADDR_BUFFER_MSB[7:0] :
    begin
      nReadData		= buffer_addr_msb_reg;
    end
    cADDR_DMA_COUNT[7:0] :
    begin
      nReadData		= mm2s_transfer_count_reg;
	clear_transfer_count = 1'b1;
    end
    cADDR_AES_CHSTS_1[7:0] :
    begin
      nReadData		= aes_channel_status[31:0];
    end
    cADDR_AES_CHSTS_2[7:0] :
    begin
      nReadData      = aes_channel_status[63:32];
    end
    cADDR_AES_CHSTS_3[7:0] :
    begin
      nReadData      = aes_channel_status[95:64];
    end
    cADDR_AES_CHSTS_4[7:0] :
    begin
      nReadData      = aes_channel_status[127:96];
    end
    cADDR_AES_CHSTS_5[7:0] :
    begin
      nReadData      = aes_channel_status[159:128];
    end
    cADDR_AES_CHSTS_6[7:0] :
    begin
      nReadData      = aes_channel_status[191:160];
    end
    cADDR_SIZE_PER_CHANNEL[7:0] :
    begin
      nReadData	     = mm2s_size_per_channel_reg;
    end
    default : 
      ReadAddrNOK    = 1'b1;
  endcase  
end

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps

module audio_formatter_v1_0_11_mm2s #(
	parameter C_FAMILY = "virtex7" , 
	parameter integer C_MAX_NUM_CHANNELS_MM2S = 2, //2,4,6,8
	parameter integer C_PACKING_MODE_MM2S = 0, // 0: Interleaved, 1: Non-interleaved
	parameter integer C_MM2S_DATAFORMAT = 3, // 0: AES -> AES
						 // 1: AES -> PCM
						 // 2: PCM -> PCM
						 // 3: PCM -> AES
	parameter integer C_MM2S_ADDR_WIDTH = 64 //32,64
)(
	input 		aud_clk,
	input 		aud_resetn,

	input mm2s_clk,
	input mm2s_resetn,

	input start_dma,
	input [C_MM2S_ADDR_WIDTH-1:0] buffer_start_address,
	input [15:0] period_size,
	input [7:0] no_of_periods,
	input [3:0] no_of_valid_channels,
	input [2:0] pcm_data_width,
	input clear_dma_transfer_count,
	input [15:0] channel_offset,
	input [15:0] fs_multiplier_value,
	input [191:0] aes_channel_status,
	
	output [24:0] dma_transfer_count,
	output ioc_interrupt,
	output slave_error,
	output decode_error,
	
	input halt_dm,
	output halt_complete_dm,
	
	output 		m_axis_tvalid,
	input 		m_axis_tready,
	output	[31:0]	m_axis_tdata,
	output 	[7:0]	m_axis_tid,

        output 	[C_MM2S_ADDR_WIDTH-1:0]	m_axi_mm2s_araddr,
        output 	[7:0]	m_axi_mm2s_arlen,
        output 	[2:0]	m_axi_mm2s_arsize,
        output 	[1:0]	m_axi_mm2s_arburst,
        output 	[2:0]	m_axi_mm2s_arprot,
        output 	[3:0]	m_axi_mm2s_arcache,
        output 	[3:0]	m_axi_mm2s_aruser,
        output 		m_axi_mm2s_arvalid,
        input 		m_axi_mm2s_arready,
         		                       
        input 	[31:0]	m_axi_mm2s_rdata,
        input 	[1:0]	m_axi_mm2s_rresp,
        input 		m_axi_mm2s_rlast       ,
        input 		m_axi_mm2s_rvalid,
        output 		m_axi_mm2s_rready
);
localparam C_CMD_ADDR_WIDTH = 64;
wire [C_CMD_ADDR_WIDTH-1:0] m_axi_mm2s_araddr_temp;
assign m_axi_mm2s_araddr =  m_axi_mm2s_araddr_temp[C_MM2S_ADDR_WIDTH-1 :0];
wire data_ready;
//wire s_axis_tlast;
wire [31:0] s_axis_tdata;
wire s_axis_tvalid;
wire s_axis_tready;

wire [31:0] s_axis_tdata_dm;
wire s_axis_tvalid_dm;
wire s_axis_tready_dm;

wire [40+C_CMD_ADDR_WIDTH-1:0] mm2s_cmd_data;
wire mm2s_cmd_valid;
wire mm2s_cmd_ready;

wire mm2s_status_valid;
wire [7:0] mm2s_status;
wire mm2s_status_ready;

wire [31:0] m_axis_tdata_mem;

audio_formatter_v1_0_11_mm2s_buffer #(
	.C_NUM_CHANNELS(C_MAX_NUM_CHANNELS_MM2S),
	.C_PACKAGING_MODE_MM2S(C_PACKING_MODE_MM2S),
	.C_MM2S_DATAFORMAT(C_MM2S_DATAFORMAT)
) buffering_1 (
	.axis_clk(mm2s_clk),
	.axis_resetn(mm2s_resetn),
	.aud_clk(aud_clk),
	.aud_resetn(aud_resetn),

	.data_ready(data_ready),
	.start_dma(start_dma),
	.no_of_valid_channels(no_of_valid_channels),
	.pcm_data_width(pcm_data_width),
	.multiplier_value(fs_multiplier_value),
	
	.s_axis_tvalid(s_axis_tvalid),
//	.s_axis_tlast(s_axis_tlast),
	.s_axis_wdata(s_axis_tdata),
	.s_axis_tready(s_axis_tready),

	.m_axis_tvalid(m_axis_tvalid),
	.m_axis_tdata(m_axis_tdata_mem),
	.m_axis_tid(m_axis_tid),
	.m_axis_tready(m_axis_tready)
);

generate if(C_MM2S_DATAFORMAT == 3) begin : PCM_TO_AES

reg start_dma_r;
wire capture;
assign capture = (~start_dma_r) && start_dma;
wire [23:0] pcm_data;
wire [31:0] aes_enc_data;
assign pcm_data = (pcm_data_width == 'd0) ? {m_axis_tdata_mem[7:0],16'd0} :
		  (pcm_data_width == 'd1) ? {m_axis_tdata_mem[15:0],8'd0} :
		  (pcm_data_width == 'd2) ? {m_axis_tdata_mem[19:0],4'd0} : {m_axis_tdata_mem[23:0]};
assign m_axis_tdata = (pcm_data_width == 'd4) ? m_axis_tdata_mem : aes_enc_data;

always@(posedge mm2s_clk) begin
	if(!mm2s_resetn) begin
		start_dma_r <= 1'b0;
	end
	else begin
		start_dma_r <= start_dma;
	end
end

audio_formatter_v1_0_11_aes_enc aes_enc1 
(
	.axis_clk(mm2s_clk),
	.axis_resetn(mm2s_resetn),

	.in_data(pcm_data),
	.in_valid(m_axis_tvalid),
	.in_ready(m_axis_tready),
	.in_id(m_axis_tid[3:0]),

	.no_of_valid_channels(no_of_valid_channels),
	.validity(1'b0),
	.out_data(aes_enc_data),

	.capt_channel_status(capture),
	.aes_channel_status(aes_channel_status)
);


end
else if(C_MM2S_DATAFORMAT == 1) begin : AES_TO_PCM

assign m_axis_tdata = (pcm_data_width == 'd0) ? {24'd0,m_axis_tdata_mem[27:20]} :
		      (pcm_data_width == 'd1) ? {16'd0,m_axis_tdata_mem[27:12]} :
		      (pcm_data_width == 'd2) ? {12'd0,m_axis_tdata_mem[27:8]}  :
		      (pcm_data_width == 'd4) ? m_axis_tdata_mem 		: {8'd0, m_axis_tdata_mem[27:4]};

end
else if(C_MM2S_DATAFORMAT == 2) begin : PCM_TO_PCM

assign m_axis_tdata = (pcm_data_width == 'd0) ? {24'd0,m_axis_tdata_mem[7:0]} :
		      (pcm_data_width == 'd1) ? {16'd0,m_axis_tdata_mem[15:0]} :
		      (pcm_data_width == 'd2) ? {12'd0,m_axis_tdata_mem[19:0]}  :
		      (pcm_data_width == 'd4) ? m_axis_tdata_mem 		: {8'd0, m_axis_tdata_mem[23:0]};

end
else begin : SEND_FULL_DATA

assign m_axis_tdata = m_axis_tdata_mem;

end
endgenerate


generate if((C_MM2S_DATAFORMAT == 3) || (C_MM2S_DATAFORMAT == 2)) begin : PCM_MEM_TO_BUFFER

wire [1:0] MAX;
assign MAX = (pcm_data_width == 'd0) ? 2'd3 : 2'd1; 
reg [1:0] count;
reg [31:0] data;
reg valid;
wire [31:0] split_data;
wire ready;
assign ready = ((count == 'd0) && (valid == 'd0) && mm2s_resetn);
assign split_data = (pcm_data_width == 'd0) ? ((count == 'd3) ? {24'd0, data[7:0]}  :
		    			       (count == 'd2) ? {24'd0, data[15:8]} :
					       (count == 'd1) ? {24'd0, data[23:16]}: {24'd0,data[31:24]}) :
		    (pcm_data_width == 'd1) ? ((count == 'd1) ? {16'd0,data[15:0]}  : {16'd0,data[31:16]}) : data;

always@(posedge mm2s_clk) begin
	if(!mm2s_resetn) begin
		count <= 'd0;
		data  <= 'd0;
		valid <= 'b0;
	end
	else if(s_axis_tvalid_dm && s_axis_tready_dm) begin
		count <= MAX;
		data  <= s_axis_tdata_dm;
		valid <= 1'b1;
	end
	else if(s_axis_tvalid && s_axis_tready) begin
		if(count == 'd0) begin
			valid <= 1'b0;
		end
		else begin
			count <= count - 1'b1;
		end
	end
end

assign s_axis_tvalid = (pcm_data_width <= 'd1) ? valid : s_axis_tvalid_dm;
assign s_axis_tdata  = (pcm_data_width <= 'd1) ? split_data : s_axis_tdata_dm;
assign s_axis_tready_dm = (pcm_data_width <= 'd1) ? ready : s_axis_tready;

end
else begin: FULL_DATA

assign s_axis_tvalid =  s_axis_tvalid_dm;
assign s_axis_tdata  =  s_axis_tdata_dm;
assign s_axis_tready_dm =  s_axis_tready;


end
endgenerate

audio_formatter_v1_0_11_mm2s_command_gen #(
	.C_NUM_CHANNELS(C_MAX_NUM_CHANNELS_MM2S),
	.C_PACKAGING_MODE_MM2S(C_PACKING_MODE_MM2S),
	.C_ADDR_WIDTH(C_MM2S_ADDR_WIDTH),
	.C_DATAFORMAT(C_MM2S_DATAFORMAT),
	.C_CMD_ADDR_WIDTH(C_CMD_ADDR_WIDTH)
) command_generator_1 (
	.axis_clk(mm2s_clk),
	.axis_resetn(mm2s_resetn),
	
	.no_of_valid_channels(no_of_valid_channels),
	.buffer_start_address(buffer_start_address),
	.period_size(period_size),
	.no_of_periods(no_of_periods),
	.input_offset(channel_offset),
	.pcm_data_width(pcm_data_width),

	.data_ready(data_ready),
	.start_dma(start_dma),
	.clear_dma_transfer_count(clear_dma_transfer_count),
	.dma_transfer_count(dma_transfer_count),
	.slave_error(slave_error),
	.decode_error(decode_error),
	.ioc_interrupt(ioc_interrupt),

	.mm2s_status_valid(mm2s_status_valid),
	.mm2s_status_ready(mm2s_status_ready),
	.mm2s_status(mm2s_status),
	
	.mm2s_cmd_tdata(mm2s_cmd_data),
	.mm2s_cmd_tvalid(mm2s_cmd_valid),
	.mm2s_cmd_tready(mm2s_cmd_ready)
);

axi_datamover
   #(
      .C_INCLUDE_MM2S              ( 1'b1         	),
      .C_M_AXI_MM2S_ADDR_WIDTH     ( C_CMD_ADDR_WIDTH      ),
      .C_M_AXI_MM2S_DATA_WIDTH     ( 32          ),
      .C_M_AXIS_MM2S_TDATA_WIDTH   ( 32          ),
      .C_INCLUDE_MM2S_STSFIFO      ( 1     		   ),
      .C_MM2S_STSCMD_FIFO_DEPTH    ( 1	   ),
      .C_MM2S_STSCMD_IS_ASYNC      ( 0   ),
      .C_INCLUDE_MM2S_DRE          ( 0           ),
      .C_MM2S_BURST_SIZE           ( 256   ),
      .C_MM2S_BTT_USED             ( 23            ),
      .C_MM2S_ADDR_PIPE_DEPTH      ( 4 ),
      .C_MM2S_INCLUDE_SF           ( 0                      ),
      .C_ENABLE_MM2S_TKEEP 	   (0) ,

 
      .C_FAMILY                    ( C_FAMILY 		     ),
      .C_ENABLE_CACHE_USER         ( 0                       ),
      .C_ENABLE_SKID_BUF           ( "11000"                 ),
      .C_CMD_WIDTH                 ( 40+C_CMD_ADDR_WIDTH    ),

      .C_INCLUDE_S2MM              ( 1'b0	   	     ),
      .C_M_AXI_S2MM_ADDR_WIDTH     ( C_CMD_ADDR_WIDTH ),
      .C_M_AXI_S2MM_DATA_WIDTH     ( 32      		),
      .C_S_AXIS_S2MM_TDATA_WIDTH   ( 32      		),
      .C_INCLUDE_S2MM_STSFIFO      ( 1     		),
      .C_S2MM_STSCMD_FIFO_DEPTH    ( 1 			),
      .C_S2MM_STSCMD_IS_ASYNC      ( 0      		),
      .C_INCLUDE_S2MM_DRE          ( 0    		),
      .C_S2MM_BURST_SIZE           ( 256   		),
      .C_S2MM_BTT_USED             ( 23         	),
      .C_S2MM_SUPPORT_INDET_BTT    ( 0      		),
      .C_S2MM_ADDR_PIPE_DEPTH      ( 4			),
      .C_S2MM_INCLUDE_SF           ( 0                  ),
      .C_ENABLE_S2MM_TKEEP  	   ( 0 			)
    ) I_DATAMOVER_MM2S 
  (
      // MM2S Primary Clock / Reset input
      .m_axi_mm2s_aclk             (mm2s_clk),
      .m_axi_mm2s_aresetn          (mm2s_resetn),

      // MM2S Soft Shutdown
      .mm2s_halt                   (halt_dm          ),
      .mm2s_halt_cmplt             (halt_complete_dm ),

      // MM2S Error output discrete
      .mm2s_err                    (           ),

      // Memory Map to Stream Command FIFO and Status FIFO Async CLK/RST //////////////
      .m_axis_mm2s_cmdsts_aclk     (mm2s_clk                 ),
      .m_axis_mm2s_cmdsts_aresetn  (mm2s_resetn            ),

      // User Command Interface Ports (AXI Stream)
      .s_axis_mm2s_cmd_tvalid      (mm2s_cmd_valid    ),
      .s_axis_mm2s_cmd_tready      (mm2s_cmd_ready    ),
      .s_axis_mm2s_cmd_tdata       (mm2s_cmd_data     ),

      // User Status Interface Ports (AXI Stream)
      .m_axis_mm2s_sts_tvalid      (mm2s_status_valid    ),
      .m_axis_mm2s_sts_tready      (mm2s_status_ready    ),
      .m_axis_mm2s_sts_tdata       (mm2s_status    ),
      .m_axis_mm2s_sts_tkeep       (    ),
      .m_axis_mm2s_sts_tlast       (    ),

   
      // Address Posting contols
      .mm2s_allow_addr_req         (1'b1   ),
      .mm2s_addr_req_posted        (  ),
      .mm2s_rd_xfer_cmplt          (  ),
      
   
      // MM2S AXI Address Channel I/O  //////////////////////////////////////
   //   m_axi_mm2s_arid             (                     ),
      .m_axi_mm2s_araddr           (m_axi_mm2s_araddr_temp         ),
      .m_axi_mm2s_arlen            (m_axi_mm2s_arlen               ),
      .m_axi_mm2s_arsize           (m_axi_mm2s_arsize              ),
      .m_axi_mm2s_arburst          (m_axi_mm2s_arburst             ),
      .m_axi_mm2s_arprot           (m_axi_mm2s_arprot              ),
      .m_axi_mm2s_arcache          (m_axi_mm2s_arcache             ),
      .m_axi_mm2s_aruser           (m_axi_mm2s_aruser              ),
      .m_axi_mm2s_arvalid          (m_axi_mm2s_arvalid             ),
      .m_axi_mm2s_arready          (m_axi_mm2s_arready             ),

      // MM2S AXI MMap Read Data Channel I/O  //////////////////////////////-
      .m_axi_mm2s_rdata            (m_axi_mm2s_rdata               ),
      .m_axi_mm2s_rresp            (m_axi_mm2s_rresp               ),
      .m_axi_mm2s_rlast            (m_axi_mm2s_rlast               ),
      .m_axi_mm2s_rvalid           (m_axi_mm2s_rvalid              ),
      .m_axi_mm2s_rready           (m_axi_mm2s_rready              ),

      // MM2S AXI Master Stream Channel I/O  ////////////////////////////////
      .m_axis_mm2s_tdata           (s_axis_tdata_dm       ),
      .m_axis_mm2s_tkeep           (       ),
      .m_axis_mm2s_tlast           (), //s_axis_tlast,
      .m_axis_mm2s_tvalid          (s_axis_tvalid_dm      ),
      .m_axis_mm2s_tready          (s_axis_tready_dm      ),

      // Testing Support I/O
      .mm2s_dbg_sel                (4'd0        ),
      .mm2s_dbg_data               ()
    );


endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps


module audio_formatter_v1_0_11_mm2s_sync 
#(
 	parameter integer C_ADDR_WIDTH = 64,
	parameter integer C_ASYNC_CLOCK = 1

)
(
input 		aud_clk,
input 		aud_resetn,

input lite_clk,
input lite_resetn,

input axi4_clk,
input axi4_resetn,

input wire [3:0] no_of_valid_channels_lite,
input wire [2:0] pcm_data_width_lite,
input wire [7:0] ioc_threshold_lite,
input wire start_dma_lite,

input wire [15:0] fs_multiplier_value_lite,
input wire [7:0] no_of_periods_lite,
input wire [15:0] period_size_lite,
input wire [15:0] channel_offset_lite,
input wire [C_ADDR_WIDTH-1:0] buffer_start_address_lite,
input wire [191:0] aes_channel_status_lite,
input wire clear_dma_transfer_count_lite,

output wire mm2s_decode_error_lite,
output wire mm2s_slave_error_lite,
output wire ioc_irq_pulse_lite,
output wire [24:0] dma_transfer_count_lite,
//------------------
output reg [3:0] no_of_valid_channels,
output reg [2:0] pcm_data_width,
output reg [7:0] ioc_threshold,
output wire start_dma,

output wire [15:0] fs_multiplier_value,
output reg [7:0] no_of_periods,
output reg [15:0] period_size,
output reg [15:0] channel_offset,
output reg [191:0] aes_channel_status,
output wire [C_ADDR_WIDTH-1:0] buffer_start_address,
output wire clear_dma_transfer_count, //pulse

input wire mm2s_decode_error,
input wire mm2s_slave_error,
input wire ioc_irq_pulse,
input wire [24:0] dma_transfer_count

);

wire start_dma_1;
wire start_dma_pos;
reg start_dma_r;

wire start_dma_1_aud;
wire start_dma_pos_aud;
reg start_dma_r_aud;

parameter ONE = 1'b1;


generate if (C_ASYNC_CLOCK)
begin: ASYNC_CLOCKS

xpm_cdc_array_single #(

  //Common module parameters
  .DEST_SYNC_FF   (2), 
  .INIT_SYNC_FF   (0), 
  .SIM_ASSERT_CHK (0), 
  .SRC_INPUT_REG  (0), 
  .WIDTH          (25) 
  ) xpm_cdc_array_single_dma_count (
  .src_clk  (axi4_clk),  
  .src_in   (dma_transfer_count),
  .dest_clk (lite_clk),
  .dest_out (dma_transfer_count_lite)
  );

xpm_cdc_array_single #(

  //Common module parameters
  .DEST_SYNC_FF   (2), 
  .INIT_SYNC_FF   (0), 
  .SIM_ASSERT_CHK (0), 
  .SRC_INPUT_REG  (0), 
  .WIDTH          (C_ADDR_WIDTH) 
  ) xpm_cdc_array_singe_bsa (
  .src_clk  (lite_clk),  
  .src_in   (buffer_start_address_lite),
  .dest_clk (axi4_clk),
  .dest_out (buffer_start_address)
  );

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_CLEAR_COUNT_INST (
  .src_clk    (lite_clk),
  .src_rst    (~lite_resetn),
  .src_pulse  (clear_dma_transfer_count_lite),
  
  .dest_clk   (axi4_clk),
  .dest_rst   (~axi4_resetn),
  .dest_pulse (clear_dma_transfer_count)
);

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_PERIOD_INT_INST (
  .src_clk    (axi4_clk),
  .src_rst    (~axi4_resetn),
  .src_pulse  (ioc_irq_pulse),
  
  .dest_clk   (lite_clk),
  .dest_rst   (~lite_resetn),
  .dest_pulse (ioc_irq_pulse_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_SLAVE_ERROR_INST (
  .src_clk   (axi4_clk),
  .src_in    (mm2s_slave_error),
  
  .dest_clk  (lite_clk),
  .dest_out  (mm2s_slave_error_lite)
);

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_DECODE_ERROR_INST (
  .src_clk   (axi4_clk),
  .src_in    (mm2s_decode_error),
  
  .dest_clk  (lite_clk),
  .dest_out  (mm2s_decode_error_lite)
);



xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_START_DMA_INST (
  .src_clk   (lite_clk),
  .src_in    (start_dma_lite),
  
  .dest_clk  (axi4_clk),
  .dest_out  (start_dma_1)
);

end else
begin: SYNC_CLOCKS

assign start_dma_1 = start_dma_lite;
assign mm2s_decode_error_lite = mm2s_decode_error;
assign mm2s_slave_error_lite = mm2s_slave_error;
assign ioc_irq_pulse_lite = ioc_irq_pulse;
assign clear_dma_transfer_count = clear_dma_transfer_count_lite;
assign buffer_start_address = buffer_start_address_lite;
assign dma_transfer_count_lite = dma_transfer_count;

end
endgenerate

xpm_cdc_single #(
  .DEST_SYNC_FF   (2),
  .SIM_ASSERT_CHK (0),
  .SRC_INPUT_REG  (1)
)
CDC_START_DMA_AUD_INST (
  .src_clk   (lite_clk),
  .src_in    (start_dma_lite),
  
  .dest_clk  (aud_clk),
  .dest_out  (start_dma_1_aud)
);

assign start_dma = start_dma_r;
assign start_dma_pos = !start_dma_r && start_dma_1;
always@(posedge axi4_clk) begin
	if(!axi4_resetn) begin
		start_dma_r	<= 1'b0;
	end
	else begin
		start_dma_r	<= start_dma_1;
	end
end

assign start_dma_pos_aud = !start_dma_r_aud && start_dma_1_aud;
always@(posedge aud_clk) begin
	if(~aud_resetn) begin
		start_dma_r_aud	<= 1'b0;
	end
	else begin
		start_dma_r_aud	<= start_dma_1_aud;
	end
end

/*always@(posedge aud_clk) begin
	if(~aud_resetn) begin
		fs_multiplier_value	<= 'd0;
	end
	else if(start_dma_pos_aud) begin
		fs_multiplier_value	<= fs_multiplier_value_lite;
	end
end*/
xpm_cdc_array_single #(

  //Common module parameters
  .DEST_SYNC_FF   (2), 
  .INIT_SYNC_FF   (0), 
  .SIM_ASSERT_CHK (0), 
  .SRC_INPUT_REG  (0), 
  .WIDTH          (16) 
  ) xpm_cdc_array_multiplier (
  .src_clk  (lite_clk),  
  .src_in   (fs_multiplier_value_lite),
  .dest_clk (aud_clk),
  .dest_out (fs_multiplier_value)
  );


generate if (ONE) begin: CAPTURE_MM2S  
always@(posedge axi4_clk) begin
	if(!axi4_resetn) begin
		no_of_valid_channels	<= 'd0;
		pcm_data_width		<= 'd0;
		ioc_threshold		<= 'd0;
		no_of_periods		<= 'd0;
		period_size		<= 'd0;
		channel_offset		<= 'd0;
		aes_channel_status	<= 'd0;
	end
	else if(start_dma_pos) begin
		no_of_valid_channels	<= no_of_valid_channels_lite;
		pcm_data_width		<= pcm_data_width_lite;
		ioc_threshold		<= ioc_threshold_lite;
		no_of_periods		<= no_of_periods_lite;
		period_size		<= period_size_lite;
		channel_offset		<= channel_offset_lite;
		aes_channel_status	<= aes_channel_status_lite;
	end
end

end
endgenerate

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////

`timescale 1ps / 1ps
module audio_formatter_v1_0_11_mm2s_buffer #(
 parameter C_NUM_CHANNELS = 2,
 parameter C_DATAWIDTH = 32,
 parameter integer C_MM2S_DATAFORMAT = 3, // 0: AES -> AES
					  // 1: AES -> PCM
					  // 2: PCM -> PCM
					  // 3: PCM -> AES
 parameter C_PACKAGING_MODE_MM2S = 0 //0: Interleaved; 1: non-interleaved
)(
input axis_clk,
input axis_resetn,
input aud_clk,
input aud_resetn,

input start_dma,
input [3:0] no_of_valid_channels,
input [2:0] pcm_data_width,
input [15:0] multiplier_value,
//from datamover
input s_axis_tvalid,
input [31:0] s_axis_wdata,
//input s_axis_tlast,
output s_axis_tready,
//to command generator
output data_ready,

//external
output m_axis_tvalid,
output [7:0] m_axis_tid,
output [31:0] m_axis_tdata,
input m_axis_tready
);

reg aud_pulse;
wire sample_pulse; //axis_clk_domain

reg ready_to_send;
reg [3:0] count;
reg [ 15:0] sample_count;

always@(posedge aud_clk ) begin
  if(~aud_resetn) begin
	sample_count <= 'd1;
	aud_pulse    <= 1'b0;
  end
  else begin
	if(sample_count >= multiplier_value) begin
		sample_count <= 'd1;
		aud_pulse	<= 1'b1;
	end
	else begin
		sample_count <= sample_count + 1'b1;
		aud_pulse	<= 1'b0;
	end
  end
end
//assign sample_pulse = aud_pulse; // Add CDC

xpm_cdc_pulse #(
  .DEST_SYNC_FF   (4),
  .REG_OUTPUT     (1),
  .RST_USED       (1),
  .SIM_ASSERT_CHK (0)
)
CDC_SAMPLE_PULSE_INST (
  .src_clk    (aud_clk),
  .src_rst    (~aud_resetn),
  .src_pulse  (aud_pulse),
  
  .dest_clk   (axis_clk),
  .dest_rst   (~axis_resetn),
  .dest_pulse (sample_pulse)
);

/*reg [7:0] tid;
always@(posedge axis_clk or negedge axis_resetn) begin
  if((!axis_resetn) || (!start_dma)) begin
	tid <= 'd0;
  end
  else begin
	if(tvalid && tready) begin
		if(tid < no_of_valid_channels-1'b1) begin
			tid <= tid + 1'b1;
		else begin
			tid <= 'd0;
		end
	end
  end
end
*/
genvar i;

generate if (C_PACKAGING_MODE_MM2S == 0)
begin: INTERLEAVED
//localparam FIFO_DEPTH = 16;
localparam FIFO_DEPTH = (C_NUM_CHANNELS == 6) ? (128) : (C_NUM_CHANNELS << 4);
localparam CNT_WIDTH = (FIFO_DEPTH == 32) ? 6 : ((FIFO_DEPTH == 64) ? 7 : 8);
wire full;
wire data_valid;
wire rst_bsy;
wire tready;
wire tvalid;
wire [31:0] data;
wire [CNT_WIDTH-1:0] fifo_count;
wire fifo_empty;

assign data_ready = fifo_empty;

xpm_fifo_sync #(
      .FIFO_WRITE_DEPTH(FIFO_DEPTH),   // DECIMAL
      .PROG_EMPTY_THRESH(FIFO_DEPTH >> 2),    // DECIMAL
      .PROG_FULL_THRESH(FIFO_DEPTH >> 1),     // DECIMAL
      .RD_DATA_COUNT_WIDTH(CNT_WIDTH),   // DECIMAL
      .WR_DATA_COUNT_WIDTH(CNT_WIDTH),    // DECIMAL
      .WRITE_DATA_WIDTH(32),     // DECIMAL
      .READ_DATA_WIDTH(32),      // DECIMAL

      .FIFO_READ_LATENCY(0),     // DECIMAL
      .DOUT_RESET_VALUE("0"),    // String
      .ECC_MODE("no_ecc"),       // String
      .FIFO_MEMORY_TYPE("auto"), // String
      .FULL_RESET_VALUE(0),      // DECIMAL
      .READ_MODE("FWFT"),         // String
      .USE_ADV_FEATURES("1F1F"), // String
      .WAKEUP_TIME(0)           // DECIMAL
   )
   interleaved_xpm_fifo_buffer (
      .almost_empty(),   
      .almost_full(full),     
      .dbiterr(),             
      //.empty(fifo_empty),                 
      .empty(),                 
      .full(),                   
      .overflow(),           
      .rd_rst_busy(),     
      .sbiterr(),             
      .underflow(),         
      .wr_rst_busy(rst_bsy),     
      .injectdbiterr(1'b0), 
      .injectsbiterr(1'b0), 
      .sleep(1'b0),                 

      .prog_empty(fifo_empty),       
      //.prog_full(full),         
      .prog_full(),         
      .data_valid(data_valid),       
      .rd_data_count(fifo_count), 
      .rd_en(tready && tvalid),                 
      .dout(data),                   

      .rst(!axis_resetn),                     
      .wr_clk(axis_clk),               
      .wr_data_count(), 
      .wr_ack(),               
      .din(s_axis_wdata),                     
      .wr_en(s_axis_tvalid && s_axis_tready)                  
   );


always@(posedge axis_clk) begin
	if(!axis_resetn || (!start_dma)) begin
		ready_to_send <= 1'b0;
		count	<= 'd0;
	end
	else begin
		if(!ready_to_send && sample_pulse && (fifo_count >= no_of_valid_channels ) && (!rst_bsy)) begin
			ready_to_send <= 1'b1;
			count		<= 'd0;
		end
		else if(tready && tvalid) begin
			if (count == no_of_valid_channels - 1'b1) begin
				ready_to_send <= 1'b0;
			end
			else begin
				ready_to_send <= 1'b1;
				count <= count + 1'b1;
			end
		end
	end
end

assign s_axis_tready = start_dma && (!full) && (!rst_bsy);
assign tvalid = ready_to_send && data_valid;
assign tready = m_axis_tready;
assign m_axis_tvalid = tvalid;
assign m_axis_tid = {4'd0,count};
assign m_axis_tdata = data;

end
else begin: NON_INTERLEAVED

wire [C_NUM_CHANNELS-1:0] wr_en;
wire [C_NUM_CHANNELS-1:0] rd_en;
wire [C_NUM_CHANNELS-1:0] full;
wire [C_NUM_CHANNELS-1:0] rst_bsy;
wire [C_NUM_CHANNELS-1:0] data_valid;
wire [C_NUM_CHANNELS-1:0] data_valid_dummy;
wire [31:0] data [C_NUM_CHANNELS -1:0];
wire [C_NUM_CHANNELS-1:0] fifo_empty;

reg [7:0] tid_in;
reg [2:0] count_tid;
always@(posedge axis_clk) begin
  if((!axis_resetn) || (!start_dma)) begin
	tid_in <= 'd0;
	count_tid	<= 3'd0;
  end
  else begin
	if(s_axis_tvalid && s_axis_tready) begin
		count_tid	<= count_tid + 1'b1;
		if(count_tid == 3'd7) begin //Assuming 8 samples of each channel are read at a time
			if(tid_in < no_of_valid_channels-1'b1) begin
				tid_in <= tid_in + 1'b1;
			end
			else begin
				tid_in <= 'd0;
			end
		end
	end
  end
end

assign s_axis_tready = s_axis_tvalid && (!full[tid_in]) && start_dma && (!(|rst_bsy));
assign m_axis_tvalid = ready_to_send;
assign m_axis_tid = {4'd0, count};
assign m_axis_tdata = data[count];
assign data_ready = (&fifo_empty);
//genvar i;
//generate
for(i=0; i < C_NUM_CHANNELS; i= i+1) begin: SEPERATING_CHANNELS


assign rd_en[i] = m_axis_tvalid && m_axis_tready && (count == i) && data_valid[i];
assign wr_en[i] = s_axis_tvalid && s_axis_tready && (tid_in == i) ;
assign data_valid_dummy[i] = (i < no_of_valid_channels) ? data_valid[i] : 1'b1;

xpm_fifo_sync #(
      .FIFO_WRITE_DEPTH(32),   // DECIMAL
      .PROG_EMPTY_THRESH(16),    // DECIMAL
      .PROG_FULL_THRESH(24),     // DECIMAL
      .RD_DATA_COUNT_WIDTH(5),   // DECIMAL
      .WR_DATA_COUNT_WIDTH(5),    // DECIMAL
      .WRITE_DATA_WIDTH(32),     // DECIMAL
      .READ_DATA_WIDTH(32),      // DECIMAL

      .FIFO_READ_LATENCY(0),     // DECIMAL
      .DOUT_RESET_VALUE("0"),    // String
      .ECC_MODE("no_ecc"),       // String
      .FIFO_MEMORY_TYPE("auto"), // String
      .FULL_RESET_VALUE(0),      // DECIMAL
      .READ_MODE("FWFT"),         // String
      .USE_ADV_FEATURES("1717"), // String
      .WAKEUP_TIME(0)           // DECIMAL
   )
   interleaved_xpm_fifo_buffer (
      .almost_empty(),   
      .almost_full(),     
      .dbiterr(),             
      //.empty(fifo_empty[i]),                 
      .empty(),                 
      .full(),                   
      .overflow(),           
      .rd_rst_busy(),     
      .sbiterr(),             
      .underflow(),         
      .wr_rst_busy(rst_bsy[i]),     
      .injectdbiterr(1'b0), 
      .injectsbiterr(1'b0), 
      .sleep(1'b0),                 

      .prog_empty(fifo_empty[i]),       
      .prog_full(full[i]),         
      .data_valid(data_valid[i]),       
      .rd_data_count(), 
      .rd_en(rd_en[i]),                 
      .dout(data[i]),                   

      .rst(!axis_resetn ),                     
      .wr_clk(axis_clk),               
      .wr_data_count(), 
      .wr_ack(),               
      .din(s_axis_wdata),                     
      .wr_en( wr_en[i] && start_dma)                  
   );

end
//endgenerate

always@(posedge axis_clk) begin
	if(!axis_resetn || (!start_dma)) begin
		ready_to_send <= 1'b0;
		count	<= 'd0;
	end
	else begin
		if(!ready_to_send && sample_pulse && (&data_valid_dummy) && (!(|rst_bsy))) begin
			ready_to_send <= 1'b1;
			count		<= 'd0;
		end
		else if(m_axis_tready && m_axis_tvalid) begin
			if (count == no_of_valid_channels - 1'b1) begin
				ready_to_send <= 1'b0;
			end
			else begin
				ready_to_send <= 1'b1;
				count <= count + 1'b1;
			end
		end
	end
end

end
endgenerate

endmodule


// (c) Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
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
////////////////////////////////////////////////////////////
`timescale 1ps / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06/08/2018 03:38:39 PM
// Design Name: 
// Module Name: audio_formatter_v1_0_11_command_generator
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////




module audio_formatter_v1_0_11_mm2s_command_gen
#(
 parameter C_NUM_CHANNELS = 2,
 parameter C_PACKAGING_MODE_MM2S = 0,
 parameter C_ADDR_WIDTH = 32,
 parameter C_CMD_ADDR_WIDTH = 64,
 parameter C_DATAFORMAT = 3
)(
input axis_clk,
input axis_resetn,

input [3:0] no_of_valid_channels,
input [C_ADDR_WIDTH-1: 0] buffer_start_address,
input [15:0] period_size,
input [7:0] no_of_periods,
input [2:0] pcm_data_width,

input start_dma,
input clear_dma_transfer_count,
input [15:0] input_offset,
input data_ready,

output [40+C_CMD_ADDR_WIDTH-1:0] mm2s_cmd_tdata,
output mm2s_cmd_tvalid,
input mm2s_cmd_tready,

input [7:0] mm2s_status,
input mm2s_status_valid,
output mm2s_status_ready,

output [24:0] dma_transfer_count,
output slave_error,
output decode_error,
output ioc_interrupt
 );

assign mm2s_status_ready = 1'b1; 
wire [40+C_CMD_ADDR_WIDTH-1:0] cmd_data;
 reg cmd_valid;
 wire cmd_ready;
//wire [C_ADDR_WIDTH-1:0] next_channel0_pointer;
//wire [C_ADDR_WIDTH-1:0] next_channel1_pointer;
//wire [15:0] offset0;
//wire [15:0] offset1;
wire [C_ADDR_WIDTH-1:0] next_period_starting_address;
reg [C_ADDR_WIDTH -1:0] current_period_address;
reg start_dma_r;
reg start_command_generation;
wire [C_CMD_ADDR_WIDTH - 1:0] current_command_address;

//assign input_offset = offset1;

//assign offset0 = 'd0;
//assign offset1 = period_size >> 1;

//reg [C_ADDR_WIDTH-1:0] channel0_pointer;
//reg [C_ADDR_WIDTH-1:0] channel1_pointer;

reg [7:0] period_count;
reg [15:0] byte_count;
reg data_ready_r;

always@(posedge axis_clk)
begin
    if(~axis_resetn)
    begin
        start_dma_r <= 1'b0;
	data_ready_r <= 1'b0;
    end
    else begin
        start_dma_r <= start_dma;
	data_ready_r <= data_ready;
    end
end


assign mm2s_cmd_tdata = cmd_data;
assign mm2s_cmd_tvalid = cmd_valid;
assign cmd_ready = mm2s_cmd_tready;

reg [7:0] periods;
reg [15:0] bytes;
reg [24:0] transfer_count_read;

reg [7:0] periods_transferred;
reg [15:0] bytes_transferred;
wire [15:0] bytes_per_transaction;
reg [C_ADDR_WIDTH-1 :0] current_address;
wire [C_ADDR_WIDTH-1 :0] next_address;
wire clear_counts = 1'b0; //clear_dma_transfer_count;
reg [24:0] transfer_count;
reg period_interrupt;
reg [3:0] channel;
reg start_command;
reg start_command_r;

assign dma_transfer_count = transfer_count_read;
assign ioc_interrupt = period_interrupt;
assign slave_error = mm2s_status_valid && mm2s_status[6] && mm2s_status_ready;
assign decode_error = mm2s_status_valid && mm2s_status[5] && mm2s_status_ready;

assign current_command_address = (C_ADDR_WIDTH == C_CMD_ADDR_WIDTH) ? current_address : {{(C_CMD_ADDR_WIDTH-C_ADDR_WIDTH){1'b0}},current_address};
assign cmd_data = {4'd0,1'd0,channel[2:0],current_command_address,1'b0,1'b1,6'd0,1'b1,7'd0,bytes_per_transaction};

genvar j;
generate if(C_PACKAGING_MODE_MM2S == 0) begin: INTERLEAVED

//assign start_command_generation = (~start_command_r) && start_command;
always@(posedge axis_clk)
begin
    if(~axis_resetn)
    begin
        start_command_generation <= 1'b0;
    end
    else begin
        if(!start_dma_r && start_dma) begin
		start_command_generation	<= 1'b1;
	end
	//else if(cmd_valid && cmd_ready) begin
	else if((~start_command) && data_ready) begin
		start_command_generation	<= 1'b1;
	end
	else begin
		start_command_generation	<= 1'b0;
	end
    end
end

always@(posedge axis_clk) begin
    if(~axis_resetn)
    begin
        start_command <= 1'b0;
        start_command_r <= 1'b0;
    end
    else begin
        start_command_r <= start_command;
        if(!start_dma_r && start_dma) begin
		start_command	<= 1'b1;
	end
	else if(mm2s_status_valid) begin
		start_command  <= 1'b0;
	end
        else if(data_ready) begin
		start_command	<= 1'b1;
	end
    end
end

//wire status_valid = cmd_valid;
//wire status_ready = cmd_ready;

//assign bytes_per_transaction = (no_of_valid_channels) << 5; //32 bytes per channel
/*assign bytes_per_transaction =  (C_DATAFORMAT == 3 || C_DATAFORMAT == 2) ? ((pcm_data_width == 'd0) ? (8) :
									    (pcm_data_width == 'd1) ? (16) :  (32)) : 
									   (32); //32 bytes */

assign bytes_per_transaction =  (C_DATAFORMAT == 3 || C_DATAFORMAT == 2) ? ((pcm_data_width == 'd0) ? (no_of_valid_channels << 3) :
									    (pcm_data_width == 'd1) ? (no_of_valid_channels << 4) :  (no_of_valid_channels << 5)) : 
									   (no_of_valid_channels << 5); //32 bytes per channel

assign next_address = ((byte_count == 0) && (period_count == no_of_periods)) ? buffer_start_address : (current_address + bytes_per_transaction);

always@(posedge axis_clk)
begin
    if(~axis_resetn || (!start_dma))
    begin
	channel <= 'd0;
        period_count <= 8'd0;
        byte_count <= 16'd0;
        current_address <= 'd0;
        cmd_valid <= 1'b0;
    end
    else begin
        if(!start_dma_r && start_dma) begin
            period_count <= 8'd1;
            byte_count <= period_size;
            current_address <= buffer_start_address;
        end
        else if (start_command_generation) begin
            byte_count <= byte_count - bytes_per_transaction;
            cmd_valid <= 1'b1;
        end
        else if (cmd_valid && cmd_ready) begin
            cmd_valid <= 1'b0;
            current_address <= next_address;
            if(byte_count == 'd0) begin
                period_count <= (period_count == no_of_periods) ? 8'd1 : (period_count + 1'b1);
                byte_count <= period_size;
            end 
        end
    end
end

end
else 
begin: NON_INTERLEAVED
wire [C_ADDR_WIDTH-1 :0] next_period_address;
reg [C_ADDR_WIDTH - 1:0] channel_pointer [C_NUM_CHANNELS-1:0];
wire [C_ADDR_WIDTH - 1:0] next_channel_pointer [C_NUM_CHANNELS-1:0];
wire [15:0] offset [C_NUM_CHANNELS-1:0];
assign offset[0] = 'd0;
assign offset[1] = input_offset;
integer i;

for(j=0;j < C_NUM_CHANNELS; j=j+1) begin: OFFSET_CALC
    if(j > 1) begin
  assign      offset[j] = offset[j-1] + input_offset;
    end
assign next_channel_pointer[j] = (byte_count == 16'd0) ? (next_period_address + offset[j]) : channel_pointer[j] + bytes_per_transaction;
end

//assign start_command_generation = (~start_command_r) && start_command;
always@(posedge axis_clk )
begin
    if(~axis_resetn)
    begin
        start_command_generation <= 1'b0;
    end
    else begin
        if(!start_dma_r && start_dma) begin
		start_command_generation	<= 1'b1;
	end
	//else if(cmd_valid && cmd_ready && (channel == no_of_valid_channels -1'b1)) begin
	else if((~start_command) && data_ready) begin
		start_command_generation	<= 1'b1;
	end
	else begin
		start_command_generation	<= 1'b0;
	end
    end
end


always@(posedge axis_clk) begin
    if(~axis_resetn)
    begin
        start_command <= 1'b0;
        start_command_r <= 1'b0;
    end
    else begin
        start_command_r <= start_command;
        if(!start_dma_r && start_dma) begin
		start_command	<= 1'b1;
	end
	else if((mm2s_status_valid && (mm2s_status[2:0] == no_of_valid_channels - 1'b1)) && (channel == no_of_valid_channels -1'b1)) begin
		start_command  <= 1'b0;
	end
        else if(data_ready) begin
		start_command	<= 1'b1;
	end
    end
end

//assign next_channel0_pointer = (byte_count == 16'd0) ? (next_period_address + offset0) : channel0_pointer + bytes_per_transaction;
//assign next_channel1_pointer = (byte_count == 16'd0) ? (next_period_address + offset1) : channel1_pointer + bytes_per_transaction;
assign next_period_address = (period_count == no_of_periods) ? buffer_start_address : (current_period_address + period_size);

//assign bytes_per_transaction = 16'd32; //32 bytes per channel
assign bytes_per_transaction = ((C_DATAFORMAT == 3) || (C_DATAFORMAT == 2)) ? ((pcm_data_width == 'd0) ? 16'd8 :
							                      (pcm_data_width == 'd1) ? 16'd16 : 16'd32) : 16'd32;  

always@(posedge axis_clk )
begin
    if(~axis_resetn || (~start_dma))
    begin
       current_period_address <= 'd0;
       for(i=0; i<C_NUM_CHANNELS; i= i+1) begin
       channel_pointer[i] <= 'd0;
       end
    //   channel0_pointer <= 'd0;
    //   channel1_pointer <= 'd0;
       cmd_valid <= 1'b0;
       byte_count <= 'd0;
       period_count <= 'd0;
       channel <= 4'd0;
       current_address <= 'd0; 
    end
    else begin
        if(!start_dma_r && start_dma) begin
            current_period_address <= buffer_start_address;
             for(i=0; i<C_NUM_CHANNELS; i= i+1) begin
               channel_pointer[i] <= buffer_start_address + offset[i];
             end           
       //     channel0_pointer <= buffer_start_address + offset0;
       //     channel1_pointer <= buffer_start_address + offset1;
            channel <= 4'd0;
            cmd_valid <= 1'b0;
            byte_count <= period_size;
            period_count <= 8'd1;
            current_address <= 'd0; 
        end
        else if (start_command_generation) begin
           // current_address <= channel0_pointer;
            current_address <= channel_pointer[0];
            channel <= 4'd0;
            byte_count <= byte_count - bytes_per_transaction;
            cmd_valid <= 1'b1;
        end
        else if (cmd_valid && cmd_ready) begin
            if(channel < no_of_valid_channels - 1'b1) begin
             //   current_address <= channel1_pointer;
                current_address <= channel_pointer[channel + 1];
                byte_count <= byte_count - bytes_per_transaction;
                channel <= channel + 1'b1;
                cmd_valid <= 1'b1;
            end
            else begin
                cmd_valid <= 1'b0;
                for(i=0; i<no_of_valid_channels; i= i+1) begin
                     channel_pointer[i] <= next_channel_pointer[i];
                end    
             //   channel0_pointer <= next_channel0_pointer;
             //   channel1_pointer <= next_channel1_pointer;
                if(byte_count == 'd0) begin
                    period_count <= (period_count == no_of_periods) ? 8'd1 : (period_count + 1'b1);
                    byte_count <= period_size;
                    current_period_address <= next_period_address;
                end       
            end
        end
    end
end

end
endgenerate

always@(posedge axis_clk) begin
    if((~axis_resetn))
    begin
	transfer_count_read <= 25'd0;
	periods		    <= 'd0;
	bytes		    <= 'd0;
    end
    else if(!start_dma)
    begin
	transfer_count_read <= 25'd0;
	bytes		    <= 'd0;
	periods		    <= 'd0;
    end
 /*   else if(clear_counts)
    begin
	transfer_count_read <= 25'd0;
	periods		    <= 'd0;
	bytes		    <= 'd0;
    end*/
    else begin
        if (mm2s_status_valid && mm2s_status_ready) begin
            if(bytes + bytes_per_transaction == period_size) begin
		bytes	    <= 'd0;
		if(periods == no_of_periods - 1'b1) begin
			periods		    <= 'd0;
			transfer_count_read <= 'd0;
	        end
		else begin
			periods		    <= periods + 1'b1;
			transfer_count_read <= transfer_count_read + bytes_per_transaction;
		end
            end else begin
                bytes <= bytes + bytes_per_transaction;
	    	transfer_count_read <= transfer_count_read + bytes_per_transaction;
	    end
    	end
    end
end


always@(posedge axis_clk) begin
    if((~axis_resetn))
    begin
        periods_transferred <= 8'd0;
        bytes_transferred <= 16'd0;
	transfer_count <= 25'd0;
	period_interrupt <= 1'b0;
    end
    //else if(clear_counts || (!start_dma))
    else if(!start_dma)
    begin
        periods_transferred <= 8'd0;
        bytes_transferred <= 16'd0;
	transfer_count <= 25'd0;
	period_interrupt <= 1'b0;
    end
    else begin
	period_interrupt <= 1'b0;
        if (mm2s_status_valid && mm2s_status_ready) begin
            if(bytes_transferred + bytes_per_transaction == period_size) begin
                bytes_transferred <= 'd0;
		period_interrupt <= 1'b1;
		if(periods_transferred == no_of_periods - 1'b1) begin
	    		transfer_count <= 'd0;
                	periods_transferred <= 'd0;
		end
		else begin
	    		transfer_count <= transfer_count + bytes_per_transaction;
                	periods_transferred <= periods_transferred + 1'b1;
		end
            end else begin
                bytes_transferred <= bytes_transferred + bytes_per_transaction;
	    	transfer_count <= transfer_count + bytes_per_transaction;
            end
        end
    end
end


endmodule




