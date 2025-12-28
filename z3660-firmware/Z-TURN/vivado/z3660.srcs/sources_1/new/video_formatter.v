`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12.08.2022 12:52:25
// Design Name: 
// Module Name: video_formatter
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

/*
 * MNT ZZ9000 Amiga Graphics and Coprocessor Card Firmware
 * Video Stream Formatter
 *
 * Copyright (C) 2019-2021, Lukas F. Hartmann <lukas@mntre.com>
 *                          MNT Research GmbH, Berlin
 *                          https://mntre.com
 *
 * More Info: https://mntre.com/zz9000
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * GNU General Public License v3.0 or later
 *
 * https://spdx.org/licenses/GPL-3.0-or-later.html
 *
*/
`define C_S_AXI_DATA_WIDTH 32
`define C_S_AXI_ADDR_WIDTH 8

module video_formatter(
  input [31:0] m_axis_vid_tdata,
  input m_axis_vid_tlast,
  output m_axis_vid_tready,
  input [0:0]  m_axis_vid_tuser,
  input m_axis_vid_tvalid,
  input m_axis_vid_aclk,
  input aresetn,

  input dvi_clk,
  output reg dvi_hsync,
  output reg dvi_vsync,
  output reg dvi_active_video,
  output [15:0] hdmi_data,

  // control inputs for setting palette, width/height, scaling
//  input [31:0] control_data,
//  input [7:0] control_op,
  output wire [1:0]control_vblank_out,


   // Xilinx AXI4-Lite implementation starts here ==============================

   // Global Clock Signal
   input wire  S_AXI_ACLK,
   // Global Reset Signal. This Signal is Active LOW
   input wire  S_AXI_ARESETN,
   // Write address (issued by master, acceped by Slave)
   input wire [`C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
   // Write channel Protection type. This signal indicates the
   // privilege and security level of the transaction, and whether
   // the transaction is a data access or an instruction access.
   input wire [2 : 0] S_AXI_AWPROT,
   // Write address valid. This signal indicates that the master signaling
   // valid write address and control information.
   input wire  S_AXI_AWVALID,
   // Write address ready. This signal indicates that the slave is ready
   // to accept an address and associated control signals.
   output wire  S_AXI_AWREADY,
   // Write data (issued by master, acceped by Slave)
   input wire [`C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
   // Write strobes. This signal indicates which byte lanes hold
   // valid data. There is one write strobe bit for each eight
   // bits of the write data bus.
   input wire [(`C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
   // Write valid. This signal indicates that valid write
   // data and strobes are available.
   input wire  S_AXI_WVALID,
   // Write ready. This signal indicates that the slave
   // can accept the write data.
   output wire  S_AXI_WREADY,
   // Write response. This signal indicates the status
   // of the write transaction.
   output wire [1 : 0] S_AXI_BRESP,
   // Write response valid. This signal indicates that the channel
   // is signaling a valid write response.
   output wire  S_AXI_BVALID,
   // Response ready. This signal indicates that the master
   // can accept a write response.
   input wire  S_AXI_BREADY,
   // Read address (issued by master, acceped by Slave)
   input wire [`C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
   // Protection type. This signal indicates the privilege
   // and security level of the transaction, and whether the
   // transaction is a data access or an instruction access.
   input wire [2 : 0] S_AXI_ARPROT,
   // Read address valid. This signal indicates that the channel
   // is signaling valid read address and control information.
   input wire  S_AXI_ARVALID,
   // Read address ready. This signal indicates that the slave is
   // ready to accept an address and associated control signals.
   output wire  S_AXI_ARREADY,
   // Read data (issued by slave)
   output wire [`C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
   // Read response. This signal indicates the status of the
   // read transfer.
   output wire [1 : 0] S_AXI_RRESP,
   // Read valid. This signal indicates that the channel is
   // signaling the required read data.
   output wire  S_AXI_RVALID,
   // Read ready. This signal indicates that the master can
   // accept the read data and response information.
   input wire  S_AXI_RREADY
   );

localparam OP_COLORMODE=1;
localparam OP_DIMENSIONS=2;
localparam OP_PALETTE=3;
localparam OP_SCALE=4;
localparam OP_VSYNC=5;
localparam OP_MAX=6;
localparam OP_HS=7;
localparam OP_VS=8;
localparam OP_THRESH=9;
localparam OP_POLARITY=10;
localparam OP_RESET=11;
localparam OP_UNUSED1=12;
localparam OP_SPRITEXY=13;
localparam OP_SPRITE_ADDR=14;
localparam OP_SPRITE_DATA=15;
localparam OP_VIDEOCAP=16; // we ignore this here, it's snooped by MNTZorro
localparam OP_REPORT_LINE=17;
localparam OP_PALETTE_SEL=18; // switch display to secondary 256 color palette for screen split
localparam OP_PALETTE_HI=19; // set values in secondary 256 color palette for screen split

localparam CMODE_8BIT=0;
localparam CMODE_16BIT=1;
localparam CMODE_32BIT=2;
localparam CMODE_15BIT=3;

reg [31:0] dvi_rgb;

assign hdmi_data = {dvi_rgb[23:19],dvi_rgb[15:10],dvi_rgb[7:3]};

reg [11:0] screen_width;
reg [11:0] screen_height;
reg [11:0] screen_height_crop;
reg scale_x = 0;
reg scale_y = 0;
reg [23:0] palette[511:0];
reg [2:0] colormode = CMODE_32BIT;
reg vsync_request;
reg vsync_request_axi;
reg sync_polarity = 1; // negative polarity
reg selected_palette = 0;

reg [15:0] screen_h_max;
reg [15:0] screen_v_max;
reg [15:0] screen_h_sync_start;
reg [15:0] screen_h_sync_end;
reg [15:0] screen_v_sync_start;
reg [15:0] screen_v_sync_end;

localparam MAXWIDTH=1280;
reg [31:0] line_buffer[MAXWIDTH-1:0];

// (input) vdma state
reg [3:0] next_input_state;
reg [11:0] inptr;
reg ready_for_vdma;
reg [1:0] control_vblank;

assign control_vblank_out = control_vblank;
assign m_axis_vid_tready = ready_for_vdma;

reg [11:0] counter_x; // vga domain
reg [11:0] counter_y; // vga domain
reg [11:0] need_line_fetch; // vga domain

reg [11:0] need_line_fetch_reg;
reg [11:0] need_line_fetch_reg2;
reg [11:0] need_line_fetch_reg3;
reg [11:0] last_line_fetch;

wire [31:0] pixin = m_axis_vid_tdata;
wire pixin_valid = m_axis_vid_tvalid;
wire pixin_end_of_line = m_axis_vid_tlast;
wire pixin_framestart = m_axis_vid_tuser[0];

reg need_frame_sync; // vga domain
reg need_frame_sync_reg; // fetch domain

// sprite
localparam SPRITE_W = 32;
localparam SPRITE_H = 48;
localparam SPRITE_SIZE = SPRITE_W*SPRITE_H;
reg [23:0] sprite_buffer[SPRITE_SIZE-1:0];
reg [11:0] sprite_addr_in;
reg [11:0] sprite_x;
reg [11:0] sprite_y;
reg sprite_dbl;
reg [11:0] report_y = 0;
reg vga_sprite_dbl; // vga_domain
reg [11:0] vga_sprite_x; // vga domain
reg [11:0] vga_sprite_y; // vga domain
reg [11:0] vga_sprite_x2; // vga domain
reg [11:0] vga_sprite_y2; // vga domain
reg [11:0] sprite_px; // vga domain
reg [11:0] sprite_py; // vga domain
reg [23:0] sprite_pix; // vga domain
reg sprite_on; // vga domain
reg [11:0] vga_report_y; // vga domain
reg [11:0] vga_report_y_next; // vga domain
reg vga_selected_palette; // vga domain


  // AXI4LITE signals
  reg [`C_S_AXI_ADDR_WIDTH-1 : 0]   axi_awaddr;
  reg   axi_awready;
  reg   axi_wready;
  reg [1 : 0]   axi_bresp;
  reg   axi_bvalid;
  reg [`C_S_AXI_ADDR_WIDTH-1 : 0]   axi_araddr;
  reg   axi_arready;
  reg [`C_S_AXI_DATA_WIDTH-1 : 0]   axi_rdata;
  reg [1 : 0]   axi_rresp;
  reg   axi_rvalid;

  // Example-specific design signals
  // local localparam for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
  // ADDR_LSB is used for addressing 32/64 bit registers/memories
  // ADDR_LSB = 2 for 32 bits (n downto 2)
  // ADDR_LSB = 3 for 64 bits (n downto 3)
  localparam integer ADDR_LSB = (`C_S_AXI_DATA_WIDTH/32) + 1;
  localparam integer OPT_MEM_ADDR_BITS = 5;
  //----------------------------------------------
  //-- Signals for user logic register space example
  //------------------------------------------------
  //-- Number of Slave Registers 4
  wire   slv_reg_rden;
  wire   slv_reg_wren;
  reg [`C_S_AXI_DATA_WIDTH-1:0]  reg_data_out;
  integer  byte_index;
  reg  aw_en;

  // I/O Connections assignments

  assign S_AXI_AWREADY  = axi_awready;
  assign S_AXI_WREADY = axi_wready;
  assign S_AXI_BRESP  = axi_bresp;
  assign S_AXI_BVALID = axi_bvalid;
  assign S_AXI_ARREADY  = axi_arready;
  assign S_AXI_RDATA  = axi_rdata;
  assign S_AXI_RRESP  = axi_rresp;
  assign S_AXI_RVALID = axi_rvalid;
  // Implement axi_awready generation
  // axi_awready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
  // de-asserted when reset is low.

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_awready <= 1'b0;
          aw_en <= 1'b1;
        end
      else
        begin
          if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
            begin
              // slave is ready to accept write address when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              axi_awready <= 1'b1;
              aw_en <= 1'b0;
            end
          else if (S_AXI_BREADY && axi_bvalid)
            begin
              aw_en <= 1'b1;
              axi_awready <= 1'b0;
            end
               else
                 begin
                   axi_awready <= 1'b0;
                 end
        end
    end

  // Implement axi_awaddr latching
  // This process is used to latch the address when both
  // S_AXI_AWVALID and S_AXI_WVALID are valid.

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_awaddr <= 0;
        end
      else
        begin
          if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
            begin
              // Write Address latching
              axi_awaddr <= S_AXI_AWADDR;
            end
        end
    end

  // Implement axi_wready generation
  // axi_wready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is
  // de-asserted when reset is low.

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_wready <= 1'b0;
        end
      else
        begin
          if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
            begin
              // slave is ready to accept write data when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              axi_wready <= 1'b1;
            end
          else
            begin
              axi_wready <= 1'b0;
            end
        end
    end

  // Implement memory mapped register select and write logic generation
  // The write data is accepted and written to memory mapped registers when
  // axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
  // select byte enables of slave registers while writing.
  // These registers are cleared when reset (active low) is applied.
  // Slave register write enable is asserted when valid address and data are available
  // and the slave is ready to accept the write address and write data.
  assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          vsync_request_axi <= 0;
        end
      else begin
        vsync_request_axi <= 0;
        if (slv_reg_wren)
          begin
            case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
                OP_PALETTE: palette[{1'b0, S_AXI_WDATA[31:24]}] <= S_AXI_WDATA[23:0];
                OP_PALETTE_HI: palette[{1'b1, S_AXI_WDATA[31:24]}] <= S_AXI_WDATA[23:0];
                OP_PALETTE_SEL: selected_palette <= S_AXI_WDATA[0];
                OP_DIMENSIONS: begin
                    screen_height <= S_AXI_WDATA[31:16];
                    screen_width  <= S_AXI_WDATA[15:0];
                end
                OP_SCALE: begin
                    scale_x  <= S_AXI_WDATA[0];
                    scale_y  <= S_AXI_WDATA[1];
                    sprite_dbl <= S_AXI_WDATA[1];
                    screen_height_crop <= S_AXI_WDATA[31:16];
                end
                OP_COLORMODE: colormode  <= S_AXI_WDATA[1:0]; // FIXME
                OP_VSYNC: vsync_request_axi <= 1; //control_data[0];
                OP_MAX: begin
                    screen_v_max <= S_AXI_WDATA[31:16];
                    screen_h_max <= S_AXI_WDATA[15:0];
                end
                OP_HS: begin
                    screen_h_sync_start <= S_AXI_WDATA[31:16];
                    screen_h_sync_end <= S_AXI_WDATA[15:0];
                end
                OP_VS: begin
                    screen_v_sync_start <= S_AXI_WDATA[31:16];
                    screen_v_sync_end <= S_AXI_WDATA[15:0];
                end
                OP_THRESH: begin
                end
                OP_POLARITY: begin
                    sync_polarity <= S_AXI_WDATA[0];
                end
                OP_RESET: begin
                /* currently a NOP */
                end
                OP_SPRITEXY: begin
                    sprite_y <= S_AXI_WDATA[31:16];
                    sprite_x <= S_AXI_WDATA[15:0];
                end
                OP_SPRITE_ADDR: begin
                    sprite_addr_in <= S_AXI_WDATA[11:0];
                end
                OP_SPRITE_DATA: begin
                    sprite_buffer[sprite_addr_in] <= S_AXI_WDATA[23:0];
                end
                OP_REPORT_LINE: begin
                    report_y <= S_AXI_WDATA[11:0];
                end
            endcase
          end
      end
    end

  // Implement write response logic generation
  // The write response and response valid signals are asserted by the slave
  // when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.
  // This marks the acceptance of address and indicates the status of
  // write transaction.

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_bvalid  <= 0;
          axi_bresp   <= 2'b0;
        end
      else
        begin
          if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
            begin
              // indicates a valid write response is available
              axi_bvalid <= 1'b1;
              axi_bresp  <= 2'b0; // 'OKAY' response
            end                   // work error responses in future
          else
            begin
              if (S_AXI_BREADY && axi_bvalid)
                //check if bready is asserted while bvalid is high)
                //(there is a possibility that bready is always asserted high)
                begin
                  axi_bvalid <= 1'b0;
                end
            end
        end
    end

  // Implement axi_arready generation
  // axi_arready is asserted for one S_AXI_ACLK clock cycle when
  // S_AXI_ARVALID is asserted. axi_awready is
  // de-asserted when reset (active low) is asserted.
  // The read address is also latched when S_AXI_ARVALID is
  // asserted. axi_araddr is reset to zero on reset assertion.

  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_arready <= 1'b0;
          axi_araddr  <= 32'b0;
        end
      else
        begin
          if (~axi_arready && S_AXI_ARVALID)
            begin
              // indicates that the slave has acceped the valid read address
              axi_arready <= 1'b1;
              // Read address latching
              axi_araddr  <= S_AXI_ARADDR;
            end
          else
            begin
              axi_arready <= 1'b0;
            end
        end
    end

  // Implement axi_arvalid generation
  // axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_ARVALID and axi_arready are asserted. The slave registers
  // data are available on the axi_rdata bus at this instance. The
  // assertion of axi_rvalid marks the validity of read data on the
  // bus and axi_rresp indicates the status of read transaction.axi_rvalid
  // is deasserted on reset (active low). axi_rresp and axi_rdata are
  // cleared to zero on reset (active low).
  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_rvalid <= 0;
          axi_rresp  <= 0;
        end
      else
        begin
          if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
            begin
              // Valid read data is available at the read data bus
              axi_rvalid <= 1'b1;
              axi_rresp  <= 2'b0; // 'OKAY' response
            end
          else if (axi_rvalid && S_AXI_RREADY)
            begin
              // Read data is accepted by the master
              axi_rvalid <= 1'b0;
            end
        end
    end

  // Output register or memory read data
  always @( posedge S_AXI_ACLK )
    begin
      if ( S_AXI_ARESETN == 1'b0 )
        begin
          axi_rdata  <= 0;
        end
      else
        begin
          // When there is a valid read address (S_AXI_ARVALID) with
          // acceptance of read address by the slave (axi_arready),
          // output the read dada
          if (slv_reg_rden)
            begin
//              axi_rdata <= reg_data_out;//{30'h0,control_vblank[1:0]};
              axi_rdata <= {30'h0,control_vblank[1:0]};
            end
        end
    end
assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
  always @(*)
    begin
      // Address decoding for reading registers
      case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
        5'h0   : reg_data_out <= {30'h0,control_vblank[1:0]};
//        3'h1   : reg_data_out <= out_reg1;
//        3'h2   : reg_data_out <= out_reg2;
//        3'h3   : reg_data_out <= out_reg3;
        default : reg_data_out <= 'h0;
      endcase
    end

  // end of AXI-Lite interface ==================================================


always @(posedge m_axis_vid_aclk)
  begin
    if (~aresetn) begin
      ready_for_vdma <= 0;
      next_input_state <= 0;
      inptr <= 0;
      need_frame_sync_reg <= 0;
      need_line_fetch_reg <= 0;
      need_line_fetch_reg2 <= 0;
      last_line_fetch <= 0;
    end
  else begin
    need_frame_sync_reg <= need_frame_sync;
    need_line_fetch_reg  <= need_line_fetch; // sync to clock domain
//    need_line_fetch_reg2 <= need_line_fetch_reg>>scale_y; // line duplication
    if(scale_y==0)
        need_line_fetch_reg2 <= need_line_fetch_reg;
    else
        need_line_fetch_reg2[11:0] <= {1'b0,need_line_fetch_reg[11:1]}; // line duplication

    if (pixin_valid && ready_for_vdma) begin
      line_buffer[inptr] <= pixin;
      // disabling this makes the picture go wild
      if (pixin_framestart) // we might have missed the frame start
        inptr <= 1;
      else if (pixin_end_of_line) // next after this is the first pixel of the line (0)
        inptr <= 0;
      else
        inptr <= inptr + 1'b1;
    end

    // one-hot encoded
    case (next_input_state)
      4'h0: begin
          // wait for start of frame
          ready_for_vdma <= 1;
          if (pixin_framestart)
            next_input_state <= 4'h4;
        end
      4'h1: begin
          // reading from vdma
          last_line_fetch <= need_line_fetch_reg2;

          if (pixin_valid && pixin_end_of_line) begin
            ready_for_vdma <= 0;
            next_input_state <= 4'h2;
          end else
            ready_for_vdma <= 1; // moved here
        end
      4'h2: begin
          // we've read more than enough of this line, wait until it's time for the next

          if (vsync_request) begin
            next_input_state <= 4'h0;
          end
          else if (need_line_fetch_reg2!=last_line_fetch) begin
            // time to read the next line
            next_input_state <= 4'h1;
            //ready_for_vdma <= 1; // from here
          end
        end
      4'h4: begin
          // we are at frame start, wait for the first line of video output
          ready_for_vdma <= 0;

          // line_fetch_reg2 == 0
          if (need_frame_sync_reg==1) begin
            next_input_state <= 4'h2;
          end
        end
    endcase
    end
  end

// control input
always @(posedge m_axis_vid_aclk)
begin
  if(vsync_request_axi) begin
    vsync_request <= 1;
  end
  else if (next_input_state==0) begin
    vsync_request <= 0;
  end
end

localparam PIPE_DELAY = 4;

reg [31:0] palout;
reg [11:0] vga_v_rez;
reg [11:0] vga_v_crop;
reg [11:0] vga_h_rez;
reg [11:0] vga_h_rez_delayed;
reg [11:0] vga_v_max;
reg [11:0] vga_h_max;
reg [11:0] vga_h_sync_start;
reg [11:0] vga_h_sync_end;
reg [11:0] vga_h_sync_start_delayed;
reg [11:0] vga_h_sync_end_delayed;
reg [11:0] vga_v_sync_start;
reg [11:0] vga_v_sync_end;
reg [11:0] counter_scanout;
reg [2:0] vga_colormode;

reg vga_scale_x = 0;
reg vga_scale_y = 0;
reg [31:0] pixout;
reg [7:0]  pixout8;
reg [15:0] pixout16;
reg [31:0] pixout32;
reg [31:0] pixout32_dly;
reg [31:0] pixout32_dly2;
wire [7:0] red16   = {pixout16[4:0],   pixout16[4:2]};
wire [7:0] green16 = {pixout16[10:5],  pixout16[10:9]};
wire [7:0] blue16  = {pixout16[15:11], pixout16[15:13]};
wire [7:0] red15   = {pixout16[4:0],   pixout16[4:2]};
wire [7:0] green15 = {pixout16[9:5],   pixout16[9:7]};
wire [7:0] blue15  = {pixout16[14:10], pixout16[14:12]};

reg [3:0] counter_scanout_step;
reg [3:0] counter_subpixel = 0;

reg vga_sync_polarity = 0;

always @(posedge dvi_clk) begin
  vga_h_rez <= screen_width;
  vga_v_rez <= screen_height;
  vga_v_crop <= screen_height_crop;
  vga_h_max <= screen_h_max - 1'b1;
  vga_v_max <= screen_v_max - 1'b1;
  vga_h_sync_start <= screen_h_sync_start;
  vga_h_sync_end <= screen_h_sync_end;

  vga_h_sync_start_delayed <= vga_h_sync_start+PIPE_DELAY;
  vga_h_sync_end_delayed <= vga_h_sync_end+PIPE_DELAY;
  vga_h_rez_delayed <= vga_h_rez+PIPE_DELAY;

  vga_v_sync_start <= screen_v_sync_start;
  vga_v_sync_end <= screen_v_sync_end;
  vga_scale_x <= scale_x;
  vga_scale_y <= scale_y;
  vga_colormode <= colormode;
  vga_sync_polarity <= sync_polarity;
  if (counter_y == 0) begin
    vga_sprite_x <= sprite_x;
    vga_sprite_y <= sprite_y;
  end
  vga_sprite_x2 <= vga_sprite_x+(SPRITE_W<<sprite_dbl);
  vga_sprite_y2 <= vga_sprite_y+(SPRITE_H<<sprite_dbl);
  vga_sprite_dbl <= sprite_dbl;
  vga_report_y_next <= report_y;
  vga_selected_palette <= selected_palette;

  /*
    pipelines (4 clocks):

    linebuf   pixout32    pixout32_dly  pixout32_dly2 pixout
    linebuf   pixout32    pixout16      pixout32_dly  pixout
    linebuf   pixout32    pixout8       palout        pixout
  */

  case ({vga_scale_x,counter_subpixel[2:0]})
    4'b0011: pixout8 <= pixout32[31:24];
    4'b0000: pixout8 <= pixout32[23:16];
    4'b0001: pixout8 <= pixout32[15:8];
    4'b0010: pixout8 <= pixout32[7:0];

    4'b1111: pixout8 <= pixout32[31:24];
    4'b1000: pixout8 <= pixout32[31:24];
    4'b1001: pixout8 <= pixout32[23:16];
    4'b1010: pixout8 <= pixout32[23:16];
    4'b1011: pixout8 <= pixout32[15:8];
    4'b1100: pixout8 <= pixout32[15:8];
    4'b1101: pixout8 <= pixout32[7:0];
    4'b1110: pixout8 <= pixout32[7:0];
  endcase

  case ({vga_scale_x,counter_subpixel[1:0]})
    3'b001: pixout16 <= {pixout32[23:16],pixout32[31:24]};
    3'b000: pixout16 <= {pixout32[7:0]  ,pixout32[15:8] };

    3'b100: pixout16 <= {pixout32[23:16],pixout32[31:24]};
    3'b111: pixout16 <= {pixout32[23:16],pixout32[31:24]};
    3'b110: pixout16 <= {pixout32[7:0]  ,pixout32[15:8] };
    3'b101: pixout16 <= {pixout32[7:0]  ,pixout32[15:8] };
  endcase

  case ({vga_scale_x,vga_colormode})
    4'b0000: counter_scanout_step <= 3; // 8 bit
    4'b1000: counter_scanout_step <= 7;
    4'b0001: counter_scanout_step <= 1; // 16 bit
    4'b1001: counter_scanout_step <= 3;
    4'b0010: counter_scanout_step <= 0; // 32 bit
    4'b1010: counter_scanout_step <= 1;
    4'b0011: counter_scanout_step <= 1; // 15 bit
    4'b1011: counter_scanout_step <= 3;
  endcase

  if (counter_x>vga_h_rez) begin
    counter_scanout  <= 0;
    counter_subpixel <= counter_scanout_step;
  end else begin
    if (counter_subpixel == 0) begin
      counter_subpixel <= counter_scanout_step;
      counter_scanout  <= counter_scanout + 1'b1;
    end else
      counter_subpixel <= counter_subpixel - 1'b1;
  end

  pixout32 <= counter_y < vga_v_crop? line_buffer[counter_scanout]:32'h0;

  if (vga_colormode==CMODE_16BIT)
    // 16 bit 5r6g5b
    pixout32_dly <= {8'b0,blue16,green16,red16};
  else if (vga_colormode==CMODE_15BIT)
    // 15 bit 5r5g5b for shapeshifter
    pixout32_dly <= {8'b0,blue15,green15,red15};
  else
    pixout32_dly <= pixout32;
  pixout32_dly2 <= pixout32_dly;

  palout <= palette[{vga_selected_palette, pixout8}];

  case (vga_colormode)
    CMODE_8BIT:  pixout <= palout;
    CMODE_16BIT: pixout <= pixout32_dly;
    CMODE_15BIT: pixout <= pixout32_dly;
    CMODE_32BIT: pixout <= pixout32_dly2;
  endcase

  sprite_pix <= sprite_buffer[((sprite_py>>sprite_dbl)<<5)+(sprite_px>>sprite_dbl)];
  if (counter_y >= vga_sprite_y && counter_y < vga_sprite_y2
      && counter_x >= vga_sprite_x && counter_x < vga_sprite_x2) begin
    sprite_on <= 1;
    if (sprite_px < (SPRITE_W<<sprite_dbl)-1'b1)
      sprite_px <= sprite_px + 1'b1;
    else begin
      sprite_px <= 0;
      sprite_py <= sprite_py + 1'b1;
    end
  end else begin
    sprite_on <= 0;
  end

  dvi_rgb <= (sprite_on && sprite_pix!='hff00ff) ? sprite_pix : pixout;

  if (counter_x >= vga_h_max) begin
    counter_x <= 0;
    if (counter_y >= vga_v_max) begin
      counter_y <= 0;
      sprite_px <= 0;
      sprite_py <= 0;
    end else begin
      counter_y <= counter_y + 1'b1;
    end
  end else begin
    counter_x <= counter_x + 1'b1;
  end

  if (counter_x==vga_h_rez) begin
    if (counter_y<vga_v_rez-1'b1)
      need_line_fetch <= counter_y + 1'b1;
    else
      need_line_fetch <= 0;
  end

  // signal synchronization point to fetch process
  if (counter_x<8 && counter_y==vga_v_sync_start)
    need_frame_sync <= 1;
  else
    need_frame_sync <= 0;

  // rasterline interrupt:
  // - first time on vblank start (1 pixel long)
  // - second time on report_y (1 pixel long)
  if (counter_y == vga_v_sync_start || (vga_report_y != 0 && (counter_y == vga_report_y - 1'b1))) begin
    // i tested the position of the interrupt relative to vdma_init,
    // there's a wide window where a buffer switch is ok, and
    // another window in which we get a line that flickers in the middle.
    if (counter_x == vga_h_rez)
      control_vblank[1] <= 1;
    else
      control_vblank[1] <= 0;
  end

  // internal vblank signal
  if (counter_y >= vga_v_rez && counter_y < vga_v_max) begin
    control_vblank[0] <= 1;
    // propagate report (interrupt) line position in vblank
    // to avoid glitches
    vga_report_y <= vga_report_y_next;
  end else begin
    control_vblank[0] <= 0;
  end

  // 4 clocks pipeline delay
  if (counter_x >= vga_h_sync_start_delayed && counter_x < vga_h_sync_end_delayed)
    dvi_hsync <= 1^vga_sync_polarity;
  else
    dvi_hsync <= 0^vga_sync_polarity;

  if (counter_x >= vga_h_sync_start_delayed)
    if (counter_y >= vga_v_sync_start && counter_y < vga_v_sync_end)
      dvi_vsync <= 1^vga_sync_polarity;
    else
      dvi_vsync <= 0^vga_sync_polarity;

  // account for 1 line of vdma wrap-around
  if (counter_y>vga_scale_y && counter_y<=(vga_v_rez + vga_scale_y) && counter_x==PIPE_DELAY)
    dvi_active_video <= 1;

  if (counter_x==vga_h_rez_delayed)
    dvi_active_video <= 0;
end

endmodule

