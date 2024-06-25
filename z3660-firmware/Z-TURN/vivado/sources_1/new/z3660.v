`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 24.06.2021 22:09:19
// Design Name: 
// Module Name: z3660
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


module z3660(
//    input clk,
    output interrupt,
    inout [31:0] D040,
    inout [31:0] A060,
    inout R_W040,
    inout nTA,
    input nTEA,
    input nTCI,
    input PCLK_clk,
//    input BCLK_clk,
//    input CLK90_clk,
//    input CPUCLK_clk,
    input nTS,
    inout [1:0] SIZ40,
    output nTS_FPGA_out,
    input nCLKEN_clk,
    output reg nTBI,
    output BP,
//  HP master interface to write to PS memory directly
    input wire m00_axi_aclk,
    input wire m00_axi_aresetn,
    // write address channel
    input wire m00_axi_awready,
    output reg [31:0] m00_axi_awaddr,
    output reg [3:0] m00_axi_awlen,
    output reg [2:0] m00_axi_awsize,
    output reg [1:0] m00_axi_awburst,
    output reg m00_axi_awlock,
    output reg [3:0] m00_axi_awcache,
    output reg [2:0] m00_axi_awprot,
    //output reg [3:0] m00_axi_awqos,
    output reg m00_axi_awvalid,
   
    // write channel
    input wire m00_axi_wready,
    output reg [31:0] m00_axi_wdata,
    output reg [3:0] m00_axi_wstrb,
    output reg m00_axi_wlast,
    output reg m00_axi_wvalid,
   
    // buffered write response channel
    input wire [1:0] m00_axi_bresp,
    input wire m00_axi_bvalid,
    output reg m00_axi_bready,
   
    // read address channel
    input wire m00_axi_arready,
    output reg [31:0] m00_axi_araddr,
    output reg [3:0] m00_axi_arlen,
    output reg [2:0] m00_axi_arsize,
    output reg [1:0] m00_axi_arburst,
    output reg m00_axi_arlock,
    output reg [3:0] m00_axi_arcache,
    output reg [2:0] m00_axi_arprot,
    //output reg [3 : 0] m00_axi_arqos,
    output reg m00_axi_arvalid,
   
    output reg m00_axi_rready,
    input wire [31:0] m00_axi_rdata,
    input wire [1:0] m00_axi_rresp,
    input wire m00_axi_rlast,
    input wire m00_axi_rvalid,
    input wire RESET_IN,
    input wire [1:0] control_vblank,
      // Xilinx AXI4-Lite implementation starts here ==============================

   input wire  S00_AXI_ACLK,
   input wire  S00_AXI_ARESETN,
   input wire [31 : 0] S00_AXI_AWADDR,
   input wire [2 : 0] S00_AXI_AWPROT,
   input wire  S00_AXI_AWVALID,
   output wire  S00_AXI_AWREADY,
   input wire [31 : 0] S00_AXI_WDATA,
   input wire [3 : 0] S00_AXI_WSTRB,
   input wire  S00_AXI_WVALID,
   output wire  S00_AXI_WREADY,
   output wire [1 : 0] S00_AXI_BRESP,
   output wire  S00_AXI_BVALID,
   input wire  S00_AXI_BREADY,
   input wire [31 : 0] S00_AXI_ARADDR,
   input wire [2 : 0] S00_AXI_ARPROT,
   input wire  S00_AXI_ARVALID,
   output wire  S00_AXI_ARREADY,
   output wire [31 : 0] S00_AXI_RDATA,
   output wire [1 : 0] S00_AXI_RRESP,
   output wire  S00_AXI_RVALID,
   input wire  S00_AXI_RREADY,

   input wire  S01_AXI_ACLK,
   input wire  S01_AXI_ARESETN,
   input wire [4 : 0] S01_AXI_AWADDR,
   input wire [2 : 0] S01_AXI_AWPROT,
   input wire  S01_AXI_AWVALID,
   output wire  S01_AXI_AWREADY,
   input wire [31 : 0] S01_AXI_WDATA,
   input wire [3 : 0] S01_AXI_WSTRB,
   input wire  S01_AXI_WVALID,
   output wire  S01_AXI_WREADY,
   output wire [1 : 0] S01_AXI_BRESP,
   output wire  S01_AXI_BVALID,
   input wire  S01_AXI_BREADY,
   input wire [4 : 0] S01_AXI_ARADDR,
   input wire [2 : 0] S01_AXI_ARPROT,
   input wire  S01_AXI_ARVALID,
   output wire  S01_AXI_ARREADY,
   output wire [31 : 0] S01_AXI_RDATA,
   output wire [1 : 0] S01_AXI_RRESP,
   output wire  S01_AXI_RVALID,
   input wire  S01_AXI_RREADY
    );

    always @(posedge m00_axi_aclk) begin
//      m00_axi_awlen <= 'h0; // 1 burst (1 write)
//      m00_axi_awburst <= 'h0; // FIXED (non incrementing)
//      m00_axi_wlast <= 'h1;
      m00_axi_awsize <= 'h2; // 2^2 == 4 bytes
      m00_axi_awcache <= 'hF; //was 3
//      m00_axi_awcache <= 'h3; //was 3
      m00_axi_awlock <= 'h0;
      m00_axi_awprot <= 'h0;
      //m00_axi_awqos <= 'h0;
      m00_axi_bready <= 'h1;

//      m00_axi_arlen <= 'h0;
//      m00_axi_arburst <= 'h0;
      m00_axi_arsize <= 'h2;
      m00_axi_arcache <= 'hF; //was 3
//      m00_axi_arcache <= 'h3; //was 3
      m00_axi_arlock <= 'h0;
      m00_axi_arprot <= 'h0;
      //m00_axi_arqos <= 'h0;
      m00_axi_rready <= 1;      
   end


wire ARM_BG;
wire ARM_RnW;
wire [1:0] ARM_SIZ;
wire [7:0] ARM_BANK;
wire ARM_COMMAND;
wire [31:0] ARM_ADDRESS;
wire [31:0] ARM_DATA;
reg [31:0] A060_out;
reg [31:0] D040_out;
reg nTS_out;
reg RW040_out;
reg [1:0] SIZ40_out;
reg nTA1;

  wire [31:0] GPIO_IN;
  wire [31:0] GPIO_READ_IN;
  reg [31:0] GPIO_OUT;
//  reg [31:0] GPIO_DATA;
//  reg [31:0] GPIO_ADD;
localparam GPIO_DATA  = 32'h0;
localparam GPIO_ADD   = 32'h0;

`include "version.vh"
 
reg [31:0] data;
reg [31:0] data_burst1;
reg [31:0] data_burst2;
reg [31:0] data_burst3;
reg [31:0] data_burst4;
//assign DDIR=(~RW) & DDIR_no;
//assign DOE = 1'd0;

localparam REG_ZZ_VBLANK_STATUS  = 21'h00017C;
localparam REG_ZZ_INT_STATUS  = 21'h0001A8;
localparam Z3_RAM_DDR_OFFSET  = 32'h2000_0000;
localparam CPU_RAM_DDR_OFFSET = 32'h0000_0000;
localparam MAPROM_DDR_OFFSET  = 32'h0000_0000;
localparam RTG_RAM_DDR_OFFSET = 32'h1800_0000;

localparam nTS_idle = 6'd0;
localparam nTS_TS0 = 6'd1;
localparam nTS_TS1 = 6'd2;
localparam nTS_TS2 = 6'd3;

localparam RAM_idle = 6'd0;
localparam RAM_write1 = 6'd1;
localparam RAM_write2 = 6'd2;
localparam RAM_write3 = 6'd3;

localparam RAM_read2 = 6'd5;
localparam RAM_read3 = 6'd6;
localparam RAM_write1b = 6'd7;
localparam RAM_read1b = 6'd8;
localparam RAM_read31 = 6'd9;
localparam RAM_write31 = 6'd10;
localparam RAM_write21 = 6'd11;
localparam RAM_read_burst1 = 6'd12;
localparam RAM_read_burst2 = 6'd13;
localparam RAM_read_burst3 = 6'd14;
localparam RAM_read_burst4 = 6'd15;
localparam RAM_read4 = 6'd16;
localparam RAM_read41 = 6'd17;
localparam RAM_read42 = 6'd18;
localparam RAM_read43 = 6'd19;
localparam RAM_read44 = 6'd20;
localparam RAM_write_burst1 = 6'd21;
localparam RAM_write_burst2 = 6'd22;
localparam RAM_write_burst3 = 6'd23;
localparam RAM_write_burst4 = 6'd24;
localparam RAM_write_burst11 = 6'd25;
localparam RAM_write_burst21 = 6'd26;
localparam RAM_write_burst31 = 6'd27;
localparam RAM_write_burst41 = 6'd28;
localparam RAM_write_burst1a = 6'd29;
localparam RAM_write_burst1b = 6'd30;
localparam RAM_write_burst1c = 6'd31;
localparam RAM_write_burst1d = 6'd32;

reg [5:0] RAM_state = RAM_idle;
reg [5:0] nTS_state = nTS_idle;

reg [3:0]nTS_d=4'b1111;
reg [31:0]GPIO_c=0;
reg [31:0]GPIO_m=0;
reg [31:0]GPIO_s=0;
reg TScondition=0;
reg ENcondition=0;

    assign interrupt = GPIO_s[29];

reg wait_finish_ARM_cycle=0;
reg ack=0;
reg [1:0] configured = 2'b00;
reg [1:0] shutup = 2'b00;
reg [15:0] autoConfigBaseFastRam = 16'h0000;
reg [15:0] autoConfigBaseRTG = 16'h0000;

reg [1:0] enabled = 2'b11;
reg cpu_ram_enable = 1'b1;
reg maprom_enable = 1'b1;
reg mapromext_enable = 1'b1;
//reg ovl = 1'b1;
wire AUTOCONFIG_RANGE = ({A060[31:16]} == {16'hFF00}) && ~&shutup[1:0] && ~&({configured[1]|~enabled[1],configured[0]|~enabled[0]});
wire MAPROMEXT_RANGE = (A060[31:19] == 13'b0000000011110) && mapromext_enable; // 0xF00000
wire MAPROM_RANGE = (A060[31:19] == 13'b0000000011111) && maprom_enable; // 0xF80000
wire CPU_RAM_RANGE = (A060[31:27] == 5'b00001) && cpu_ram_enable;
wire FASTRAM_CONFIGURED_RANGE = (A060[31:28] == autoConfigBaseFastRam[15:12]) && configured[0] && enabled[0];
wire RTG_CONFIGURED_RANGE = (A060[31:27] == {autoConfigBaseRTG[15:12],1'b0}) && configured[1] && enabled[1];

wire PCLK2_clk = PCLK_clk;
/*
reg PCLK2_clk = 1;
    always @(negedge m00_axi_aclk) begin
        if((RESET_IN==1'b1) || (GPIO_s[31]==1)) begin
            PCLK2_clk <= 1'b0;
        end
        else begin
            PCLK2_clk <= !PCLK2_clk;
        end
    end
 */
    always @(posedge m00_axi_aclk) begin
        nTS_d <= {nTS_d[2:0],nTS};
        GPIO_m<=GPIO_IN;
        GPIO_s<=GPIO_m;

        if(GPIO_s[6:4]==3'b000)
            TScondition<=(nTS_d[2:1]==2'b10)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b001)
            TScondition<=(nTS_d[1:0]==2'b10)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b010)
            TScondition<=(nTS_d[0]==1'b0)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b011)
            TScondition<=(nTS==1'b0)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b100)
            TScondition<=(nTS_d[2:1]==2'b01)?1'b1:1'b0; // rising edge
        else if(GPIO_s[6:4]==3'b101)
            TScondition<=(nTS_d[1:0]==2'b01)?1'b1:1'b0; // rising edge
    end
    always @(negedge m00_axi_aclk) begin
//    always @(*) begin // this is a bad idea, ENcondition becomes a combinational gated clock...
        if(GPIO_c[9:8]==2'b00)
            ENcondition<=(nCLKEN_clk==1'b0)&&(PCLK2_clk==1'b0)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b01)
            ENcondition<=(nCLKEN_clk==1'b0)&&(PCLK2_clk==1'b1)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b10)
            ENcondition<=(PCLK2_clk==1'b0)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b11)
            ENcondition<=(PCLK2_clk==1'b1)?1'b1:1'b0;
    end

assign D040[31:0]= (ARM_BG==1'b0) ? ( (RAM_state == RAM_read2)||(RAM_state == RAM_read3)||(RAM_state == RAM_read31) ||
                   (RAM_state == RAM_read4)||(RAM_state == RAM_read41)||(RAM_state == RAM_read42) ||
                   (RAM_state == RAM_read43)||(RAM_state == RAM_read44)||
                   (RAM_state == RAM_read_burst2)||(RAM_state == RAM_read_burst3)||(RAM_state == RAM_read_burst4)
                   ? data[31:0] : 32'bz) : ((ARM_RnW==1'b0) ? D040_out[31:0] : 32'bz);

reg CPURAM_start_cycle=0;
reg MAPROM_start_cycle=0;
reg MAPROMEXT_start_cycle=0;
reg FASTRAM_start_cycle=0;
reg RTG_start_cycle=0;
reg AUTOCONFIG_start_cycle=0;
reg nTS_FPGA=1'b1;

assign  BP = GPIO_s[28];
wire  AUTOCONFIG_BOOT_ROM_ENABLE = GPIO_s[27];

// GPIO_IN
// -----------------------------------------------------------------------------------------------
// |  31   | 30  |  29  | 28 |     27     | 26 | 25 | 24 | 23 | 22 | 21 | 20 | 19 | 18 | 17 | 16 |
// ----------------------------------------------------------------------------------------------
// | FPGA  | ACK | INT6 | BP | AUTOCONFIG |    |    |    |    |    |    |    |    |    |    |    |
// | RESET | R-W |      |    |  BOOT ROM  |    |    |    |    |    |    |    |    |    |    |    |
// -----------------------------------------------------------------------------------------------
//
// --------------------------------------------------------------------------------------------------
// | 15 | 14 |   13-12    | 11 | 10 |   9-8     |    7      |   6-5-4    |   3    |  2-1  |   0     |
// -------------------------------------------------------------------------------------------------
// |    |    |  RTG-RAM   |    |    |    EN     | MAPROMEXT |     TS     | MAPROM |  W-R  | CPU RAM |
// |    |    | AUTOC. EN  |    |    | CONDITION |   ENABLE  |  CONDITION | ENABLE | BURST | ENABLE  |
// --------------------------------------------------------------------------------------------------

always @(posedge m00_axi_aclk) begin
    nTA1 <= 1'bz;
    if((RESET_IN==1'b1) || (GPIO_s[31]==1)) begin
        m00_axi_arlen <= 'h0;
        m00_axi_arburst <= 'h0;
//        m00_axi_rready <= 1;
        m00_axi_awlen <= 'h0;
        m00_axi_awburst <= 'h0;
        m00_axi_wlast <= 'h1;
        m00_axi_awvalid <= 1'b0;
        m00_axi_arvalid <= 1'b0;
        m00_axi_wvalid <= 1'b0;
        nTBI <= 1'b1;
        nTS_FPGA <= 1'b1;
        RAM_state <= RAM_idle;
        nTS_state <= nTS_idle;
        configured <= 2'b00;
        shutup <= 2'b00;
        autoConfigBaseFastRam[15:0] = 16'h0000;
        autoConfigBaseRTG[15:0] = 16'h0000;
        GPIO_OUT[31:0] <= 32'h0;
        wait_finish_ARM_cycle <= 0;
        enabled[1:0] <={GPIO_s[13],GPIO_s[12]};
        cpu_ram_enable<=GPIO_s[0];
        maprom_enable<=GPIO_s[3];
        mapromext_enable<=GPIO_s[7];
//        ovl<=1'b1;
        CPURAM_start_cycle<=0;
        MAPROM_start_cycle<=0;
        MAPROMEXT_start_cycle<=0;
        FASTRAM_start_cycle<=0;
        RTG_start_cycle<=0;
        AUTOCONFIG_start_cycle<=0;
    end else begin
        case (nTS_state)
            nTS_idle: begin
                nTS_FPGA <= 1'b1;
                if (TScondition==1'b1) begin
                    GPIO_c<=GPIO_s;
                    if (CPU_RAM_RANGE==1'b1 ) begin                 // CPU-RAM  128Mb
                        CPURAM_start_cycle<=1;
                    end
                    else if (MAPROM_RANGE==1'b1) begin             // MAPROM   512kb
                        MAPROM_start_cycle<=1;
                    end
                    else if (MAPROMEXT_RANGE==1'b1) begin           // MAPROMEXT   512kb
                        MAPROMEXT_start_cycle<=1;
                    end
                    else if (FASTRAM_CONFIGURED_RANGE==1'b1) begin  // Z3 RAM   256Mb
                        FASTRAM_start_cycle<=1;
                    end
                    else if (RTG_CONFIGURED_RANGE==1'b1) begin      // Z3 RTG
                        RTG_start_cycle<=1;
                    end
                    else if(AUTOCONFIG_RANGE==1'b1) begin
                        AUTOCONFIG_start_cycle<=1;
                    end
                    else begin
                    //  OVL write detection is not working...
//                        if(A060[31:0]==32'h00BFE001
//                        && SIZ40[1:0]==2'b01 
//                        && R_W040==0) begin // CIA byte write
//                            ovl<=D040[16];
//                        end
                        // address is not used by FPGA code, so send TS to CPLD
                        nTS_FPGA <= 1'b0;
                        nTS_state <= nTS_TS0;
                    end
//                    GPIO_DATA <= D040;
//                    GPIO_ADD  <= {1'b1,A060[30:0]};
                end
            end
            nTS_TS0: begin
                nTS_FPGA <= 1'b0;
                nTS_state <= nTS_TS1;
            end
            nTS_TS1: begin
                nTS_FPGA <= 1'b0;
                nTS_state <= nTS_TS2;
            end
            nTS_TS2: begin
                nTS_FPGA <= 1'b1;
                nTS_state <= nTS_idle;
            end
            default: begin
                nTS_state <= nTS_idle;
            end
        endcase
        case (RAM_state)
            RAM_idle: begin
                m00_axi_awvalid  <= 1'b0;
                m00_axi_arvalid  <= 1'b0;
                if (CPURAM_start_cycle==1) begin
                nTBI<=1'b1;
                    CPURAM_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    if (R_W040==0) begin  // CPURAM Write cycle
                        RAM_state <= RAM_write1b;
                        m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; //BS
                        m00_axi_awvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[2]==1)) begin //line // BURST WRITE ENABLE
                            nTBI<=1'b0;
                            m00_axi_awlen <= 'h3;
                            m00_axi_awburst <= 'h2; // wrap
                            m00_axi_wlast <= 'h0;
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_awlen <= 'h0;
                            m00_axi_awburst <= 'h0;
                            m00_axi_wlast <= 'h0;
                        end
                        if(SIZ40[1:0]==2'b00) // long
                            m00_axi_wstrb   <= 4'b1111;
                        else if(SIZ40[1:0]==2'b11) // line
                            m00_axi_wstrb   <= 4'b1111;
                        else if (SIZ40[1:0]==2'b10) begin // word
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1100;
                                m00_axi_wstrb   <= 4'b0011; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0011;
                                m00_axi_wstrb   <= 4'b1100; //BS
                            else
                                m00_axi_wstrb   <= 4'b0000; // impossible case?
                            end
                        else begin // byte
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1000;
                                m00_axi_wstrb   <= 4'b0001; //BS
                            else if (A060[1:0]==2'b01)
//                                m00_axi_wstrb   <= 4'b0100;
                                m00_axi_wstrb   <= 4'b0010; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0010;
                                m00_axi_wstrb   <= 4'b0100; //BS
                            else //if (A060[1:0]==2'b11)
//                                m00_axi_wstrb   <= 4'b0001;
                                m00_axi_wstrb   <= 4'b1000; //BS
                        end
                        m00_axi_awaddr[31:0] <= {A060[31:2],2'b00};// + {CPU_RAM_DDR_OFFSET};
//                        m00_axi_wdata[31:0] <= D040[31:0];
                    end
                    else begin  // CPURAM Read Cycle
                        RAM_state <= RAM_read1b;
                        m00_axi_araddr[31:0]  <= {A060[31:2],2'b00};// + {CPU_RAM_DDR_OFFSET};
                        m00_axi_arvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[1]==1)) begin // BURST READ ENABLE
                            nTBI<=1'b0;
                            m00_axi_arlen <= 'h3;
                            m00_axi_arburst <= 'h2; // 0=fixed, 1=inc, 2=wrap (060 makes address wrap)
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_arlen <= 'h0;
                            m00_axi_arburst <= 'h0;
                        end
                    end
                end
                else if (MAPROM_start_cycle==1) begin
                    nTBI<=1'b1;
                    MAPROM_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    if (R_W040==0) begin  // MAPROM Write cycle
                        wait_finish_ARM_cycle<=1;
                        GPIO_OUT[31:0] <= {2'b10,SIZ40[1:0],A060[27:0]};
                        RAM_state <= RAM_write3;
                    end
                    else begin  // MAPROM Read Cycle
                        RAM_state <= RAM_read1b;
                        m00_axi_araddr[31:0]  <= {A060[31:2],2'b00};// + {MAPROM_DDR_OFFSET};
                        m00_axi_arvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[1]==1)) begin // BURST READ ENABLE
                            nTBI<=1'b0;
                            m00_axi_arlen <= 'h3;
                            m00_axi_arburst <= 'h2; // 0=fixed, 1=inc, 2=wrap (060 makes address wrap)
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_arlen <= 'h0;
                            m00_axi_arburst <= 'h0;
                        end
//                        wait_finish_ARM_cycle<=1;
//                        GPIO_OUT[31:0] <= {2'b01,SIZ40[1:0],A060[27:0]};
//                        RAM_state <= RAM_read3;
                    end
                end
                else if (MAPROMEXT_start_cycle==1) begin
                    nTBI<=1'b1;
                    MAPROMEXT_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    if (R_W040==0) begin  // MAPROMEXT Write cycle
                        wait_finish_ARM_cycle<=1;
                        GPIO_OUT[31:0] <= {2'b10,SIZ40[1:0],A060[27:0]};
                        RAM_state <= RAM_write3;
                    end
                    else begin  // MAPROMEXT Read Cycle
                        RAM_state <= RAM_read1b;
                        m00_axi_araddr[31:0]  <= {A060[31:2],2'b00};// + {MAPROM_DDR_OFFSET};
                        m00_axi_arvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[1]==1)) begin // BURST READ ENABLE
                            nTBI<=1'b0;
                            m00_axi_arlen <= 'h3;
                            m00_axi_arburst <= 'h2; // 0=fixed, 1=inc, 2=wrap (060 makes address wrap)
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_arlen <= 'h0;
                            m00_axi_arburst <= 'h0;
                        end
//                        wait_finish_ARM_cycle<=1;
//                        GPIO_OUT[31:0] <= {2'b01,SIZ40[1:0],A060[27:0]};
//                        RAM_state <= RAM_read3;
                    end
                end
                else if (FASTRAM_start_cycle==1) begin
                    nTBI<=1'b1;
                    FASTRAM_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    if (R_W040==0) begin  // FASTRAM Write cycle
                        RAM_state <= RAM_write1b;
                        m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; //BS
                        m00_axi_awvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[2]==1)) begin //line // BURST WRITE ENABLE
                            nTBI<=1'b0;
                            m00_axi_awlen <= 'h3;
                            m00_axi_awburst <= 'h2; // wrap
                            m00_axi_wlast <= 'h0;
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_awlen <= 'h0;
                            m00_axi_awburst <= 'h0;
                            m00_axi_wlast <= 'h0;
                        end
                        if(SIZ40[1:0]==2'b00) // long
                            m00_axi_wstrb   <= 4'b1111;
                        else if(SIZ40[1:0]==2'b11) // line
                            m00_axi_wstrb   <= 4'b1111;
                        else if (SIZ40[1:0]==2'b10) begin // word
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1100;
                                m00_axi_wstrb   <= 4'b0011; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0011;
                                m00_axi_wstrb   <= 4'b1100; //BS
                            else
                                m00_axi_wstrb   <= 4'b0000; // impossible case?
                            end
                            else begin // byte
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1000;
                                m00_axi_wstrb   <= 4'b0001; //BS
                            else if (A060[1:0]==2'b01)
//                                m00_axi_wstrb   <= 4'b0100;
                                m00_axi_wstrb   <= 4'b0010; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0010;
                                m00_axi_wstrb   <= 4'b0100; //BS
                            else //if (A060[1:0]==2'b11)
//                                m00_axi_wstrb   <= 4'b0001;
                                m00_axi_wstrb   <= 4'b1000; //BS
                        end
                        m00_axi_awaddr[31:0] <= {Z3_RAM_DDR_OFFSET[31:28],A060[27:2],2'b00};
                    end
                    else begin  // FASTRAM Read Cycle
                        RAM_state <= RAM_read1b;
                        m00_axi_araddr[31:0] <= {Z3_RAM_DDR_OFFSET[31:28],A060[27:2],2'b00};
                        m00_axi_arvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[1]==1)) begin // BURST READ ENABLE
                            nTBI<=1'b0;
                            m00_axi_arlen <= 'h3;
                            m00_axi_arburst <= 'h2; // 0=fixed, 1=inc, 2=wrap (060 makes address wrap)
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_arlen <= 'h0;
                            m00_axi_arburst <= 'h0;
                        end
                    end
                end
                else if (RTG_start_cycle==1) begin
                    nTBI<=1'b1;
                    RTG_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    if (R_W040==0) begin  // RTG Write cycle
                        RAM_state <= RAM_write1b;
                        m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; //BS
                        m00_axi_awvalid  <= 1'b1;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[2]==1)) begin //line // BURST WRITE ENABLE
                            nTBI<=1'b0;
                            m00_axi_awlen <= 'h3;
                            m00_axi_awburst <= 'h2; // wrap
                            m00_axi_wlast <= 'h0;
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_awlen <= 'h0;
                            m00_axi_awburst <= 'h0;
                            m00_axi_wlast <= 'h0;
                        end
                        if(SIZ40[1:0]==2'b00) // long
                            m00_axi_wstrb   <= 4'b1111;
                        else if(SIZ40[1:0]==2'b11) // line
                            m00_axi_wstrb   <= 4'b1111;
                        else if (SIZ40[1:0]==2'b10) begin // word
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1100;
                                m00_axi_wstrb   <= 4'b0011; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0011;
                                m00_axi_wstrb   <= 4'b1100; //BS
                            else
                                m00_axi_wstrb   <= 4'b0000; // impossible case?
                            end
                        else begin // byte
                            if (A060[1:0]==2'b00)
//                                m00_axi_wstrb   <= 4'b1000;
                                m00_axi_wstrb   <= 4'b0001; //BS
                            else if (A060[1:0]==2'b01)
//                                m00_axi_wstrb   <= 4'b0100;
                                m00_axi_wstrb   <= 4'b0010; //BS
                            else if (A060[1:0]==2'b10)
//                                m00_axi_wstrb   <= 4'b0010;
                                m00_axi_wstrb   <= 4'b0100; //BS
                            else //if (A060[1:0]==2'b11)
//                                m00_axi_wstrb   <= 4'b0001;
                                m00_axi_wstrb   <= 4'b1000; //BS
                        end
                        if( A060[26:21] == 6'b000000 ) begin // register memory zone: 0 .. 0x1FFFFF
                            wait_finish_ARM_cycle<=1;
                            m00_axi_awaddr[31:0] <= {RTG_RAM_DDR_OFFSET[31:27],A060[26:2],2'b00};
                            GPIO_OUT[31:0] <= {1'b1,1'b0,SIZ40[1:0],7'h00,A060[20:0]};
                            nTBI<=1'b1;
                            m00_axi_awlen <= 'h0;
                            m00_axi_awburst <= 'h0;
                            m00_axi_wlast <= 'h0;
                        end
                        else begin
                            m00_axi_awaddr[31:0] <= {RTG_RAM_DDR_OFFSET[31:27],A060[26:2],2'b00};
                            end
                        end
                    else begin  // RTG Read Cycle
                        RAM_state <= RAM_read1b;
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[1]==1)) begin // BURST READ ENABLE
                            nTBI<=1'b0;
                            m00_axi_arlen <= 'h3;
                            m00_axi_arburst <= 'h2; // 0=fixed, 1=inc, 2=wrap (060 makes address wrap)
                        end else begin
                            nTBI<=1'b1;
                            m00_axi_arlen <= 'h0;
                            m00_axi_arburst <= 'h0;
                        end
                        if( A060[26:21] == 6'b000000 ) begin // register memory zone: 0 .. 0x1FFFFF
                            if( {A060[20:2],2'b00} == REG_ZZ_VBLANK_STATUS ) begin // vblank register is directly read here
                                data[31:0]<={31'h0,control_vblank[0]};
                                ack<=0;
                                nTBI<=1'b1;
                                m00_axi_arlen <= 'h0;
                                m00_axi_arburst <= 'h0;
                                RAM_state <= RAM_read3;
                            end
//                            else if( {A060[15:2],2'b00} == REG_ZZ_INT_STATUS ) begin // vblank register is directly read here
// direct read registers from RAM
//                                m00_axi_araddr[31:0] <= {16'h0000,A060[15:2],2'b00} + {RTG_RAM_DDR_OFFSET};
//                                m00_axi_arvalid  <= 1'b1;
//                            end
/*                            else if({A060[15:2],2'b00} == REG_ZZ_.... another register? ) begin // ???? register is directly read here
                                data[31:0]<={31'h0,control_vblank[0]};
                                ack<=0;
                                RAM_state <= RAM_read3;
                                nTBI<=1'b1;
                            end*/
                            else begin // other registers are read from RTG memory
// direct read registers from RAM
//                                m00_axi_araddr[31:0] <= {16'h0000,A060[15:2],2'b00} + {RTG_RAM_DDR_OFFSET};
//                                m00_axi_arvalid  <= 1'b1;
                                wait_finish_ARM_cycle<=1;
                                m00_axi_araddr[31:0] <= {RTG_RAM_DDR_OFFSET[31:27],A060[26:2],2'b00};
                                GPIO_OUT[31:0] <= {1'b0,1'b1,SIZ40[1:0],7'h00,A060[20:0]};
                                RAM_state <= RAM_read3;
                                nTBI<=1'b1;
                                m00_axi_arlen <= 'h0;
                                m00_axi_arburst <= 'h0;
                                ack<=0;
                            end
                        end else begin
                            m00_axi_araddr[31:0] <= {RTG_RAM_DDR_OFFSET[31:27],A060[26:2],2'b00};
                            m00_axi_arvalid  <= 1'b1;
                        end
                    end
                end

                else if(AUTOCONFIG_start_cycle==1) begin
                    AUTOCONFIG_start_cycle<=0;
                    wait_finish_ARM_cycle<=0;
                    ack<=0;
                    nTBI<=1'b1;
                    if (R_W040==0) begin  // Autoconfig Write cycle
                        RAM_state <= RAM_write3;
                        casex (A060[15:0])
                            16'hXX44: begin

                                if (configured[0] == 1'b0 && enabled[0] == 1'b1) begin
                                    autoConfigBaseFastRam[15:0] <= D040[31:16];     // FastRAM
                                    configured[0] <= 1'b1;
                                end
                                else
                                if (configured[1] == 1'b0 && enabled[1] == 1'b1) begin
                                    autoConfigBaseRTG[15:0] <= D040[31:16];         // RTG
                                    configured[1] <= 1'b1;
                                end
                            end

                            16'hXX4C: begin
                                if ({configured[0] == 1'b1}) shutup[0] <= 1'b1;   // FastRAM
                                if ({configured[1] == 1'b1}) shutup[1] <= 1'b1;   // RTG
                            end
                        endcase
                    end
                    else  begin // Autoconfig Read Cycle
                        RAM_state <= RAM_read3;
            
                        case (A060[15:0])
                            16'h0000: begin
                                if (configured[0] == 1'b0 && enabled[0] == 1'b1) data[31:0] <= {16'b1010_1111_1111_1111,16'hFFFF}; // zorro 3 (10), pool link (1), autoboot ROM no (0)
                                else 
                                if (configured[1] == 1'b0 && enabled[1] == 1'b1) data[31:0] <= {3'b100,AUTOCONFIG_BOOT_ROM_ENABLE,12'b1111_1111_1111,16'hFFFF}; // zorro 3 (10), no pool link (0), autoboot ROM no (0)
                            end

                            16'h0100: begin
                                if (configured[0] == 1'b0 && enabled[0] == 1'b1) data[31:0] <= {16'b0100_1111_1111_1111,16'hFFFF}; // next board unrelated (0), 256MB FastRAM
                                else
                                if (configured[1] == 1'b0 && enabled[1] == 1'b1) data[31:0] <= {16'b1011_1111_1111_1111,16'hFFFF}; // next board unrelated (0), 128MB RTG
                            end
                            16'h0004: begin
                                data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // product number
                            end
                            16'h0104: begin
                                if (configured[0] == 1'b0 && enabled[0] == 1'b1) data[31:0] <= {16'b1101_1111_1111_1111,16'hFFFF}; // 2 for the 256MB Z3 Fast
                                else
                                if (configured[1] == 1'b0 && enabled[1] == 1'b1) data[31:0] <= {16'b1110_1111_1111_1111,16'hFFFF}; // 1 for the RTG PIC
                            end
                            16'h0008: begin
                                if (configured[0] == 1'b0 && enabled[0] == 1'b1) data[31:0] <= {16'b1000_1111_1111_1111,16'hFFFF}; // flags inverted 0111 io,shutup,extension,reserved(1)
                                else
                                if (configured[1] == 1'b0 && enabled[1] == 1'b1) data[31:0] <= {16'b1000_1111_1111_1111,16'hFFFF}; // flags inverted 0111 io,shutup,extension,reserved(1)
                            end
                            16'h0108: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // inverted zero

                            16'h000c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // reserved?
                            16'h010c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; //

                            16'h0010: data[31:0] <= {16'b1110_1111_1111_1111,16'hFFFF}; // 1 manufacturer high byte inverted
                            16'h0110: data[31:0] <= {16'b1011_1111_1111_1111,16'hFFFF}; // 4
                            16'h0014: data[31:0] <= {16'b1011_1111_1111_1111,16'hFFFF}; // 4 manufacturer low byte
                            16'h0114: data[31:0] <= {16'b0100_1111_1111_1111,16'hFFFF}; // B

                            16'h0028: data[31:0] <= {16'b1001_1111_1111_1111,16'hFFFF}; // autoboot rom vector (er_InitDiagVec)
                            16'h0128: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // = ~0x6000
                            16'h002c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF};
                            16'h012c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF};

                            default: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF};

                        endcase
                    end
                end
            end
            RAM_read1b: begin
                if (m00_axi_arready) begin
                    if(wait_finish_ARM_cycle==0) begin
                        if((SIZ40[1:0]==2'b11)&& (GPIO_c[1]==1))  // BURST READ ENABLE
                            RAM_state <= RAM_read_burst1;
                        else
                            RAM_state <= RAM_read2;
                    end else begin
                        RAM_state <= RAM_read2;
                    end
                end
            end
            RAM_read2: begin
//                data[31:0] <= m00_axi_rdata[31:0];
                data[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                m00_axi_arvalid <= 1'b0;
                if (m00_axi_rvalid) begin
                    RAM_state <= RAM_read3;
//                    data[31:0] <= m00_axi_rdata[31:0];
                    data[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                end
                ack<=0;
            end
            RAM_read3: begin
                if(wait_finish_ARM_cycle==0) begin
                    if(ENcondition==1'b1) begin
                        RAM_state <= RAM_read31;
                        nTA1 <= 1'b0;
                    end
                end
                else begin
                    if((GPIO_IN[30]==1) && (ack==0)) begin // ack read/write
                        GPIO_OUT[31:0] <=32'h0000_0000;
                        data[31:0] <= GPIO_READ_IN[31:0];
                        ack<=1;
                    end
                    else begin
                        if((GPIO_IN[30]==0) && (ack==1)) begin // ack read/write
                            GPIO_OUT[31:0] <=32'h0000_0000;
                            data[31:0] <= GPIO_READ_IN[31:0];
                            if(ENcondition==1'b1) begin
                                RAM_state <= RAM_read31;
                                nTA1 <= 1'b0;
                            end
                        end
                    end
                end

            end
            RAM_read31: begin
                nTA1 <= 1'b0;
//                data[31:0] <= m00_axi_rdata[31:0]; //with cache or HP this is wrong
                if(ENcondition==1'b1) begin
                    RAM_state <= RAM_idle;
                    nTA1 <= 1'b1;
                 end
            end
            RAM_read_burst1: begin
                m00_axi_arvalid <= 1'b0;
                    if (m00_axi_rvalid) begin
//                    data_burst1[31:0]<=m00_axi_rdata[31:0];
                        data_burst1[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                    RAM_state <= RAM_read_burst2;
                end
            end
            RAM_read_burst2: begin
                data[31:0] <= data_burst1[31:0];
                if (m00_axi_rvalid) begin
//                    data_burst2[31:0]<=m00_axi_rdata[31:0];
                    data_burst2[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                    RAM_state <= RAM_read_burst3;
                end
            end
            RAM_read_burst3: begin
                data[31:0] <= data_burst1[31:0];
                if (m00_axi_rvalid) begin
//                    data_burst3[31:0]<=m00_axi_rdata[31:0];
                    data_burst3[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                    RAM_state <= RAM_read_burst4;
                end
            end
            RAM_read_burst4: begin
                data[31:0] <= data_burst1[31:0];
                m00_axi_arvalid <= 1'b0; 
                if (m00_axi_rvalid) begin
//                    data_burst4[31:0]<=m00_axi_rdata[31:0];
                    data_burst4[31:0] <= {m00_axi_rdata[7:0],m00_axi_rdata[15:8],m00_axi_rdata[23:16],m00_axi_rdata[31:24]}; // BS
                    RAM_state <= RAM_read4;
                end
            end
            RAM_read4: begin
                data[31:0] <= data_burst1[31:0];
                if(ENcondition==1'b1) begin
                    RAM_state <= RAM_read41;
                    nTA1 <= 1'b0;
                end
            end
            RAM_read41: begin
                nTA1 <= 1'b0;
                data[31:0] <= data_burst1[31:0];
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_read42;
                end
            end
            RAM_read42: begin
                nTA1 <= 1'b0;
                data[31:0] <= data_burst2[31:0];
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_read43;
                end
            end
            RAM_read43: begin
                nTA1 <= 1'b0;
                data[31:0] <= data_burst3[31:0];
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_read44;
            end
           end
            RAM_read44: begin
                nTA1 <= 1'b0;
                data[31:0] <= data_burst4[31:0];
                if(ENcondition==1'b1) begin
                    RAM_state <= RAM_idle;
                    nTA1 <= 1'b1;
                    nTBI <= 1'b1;
                end
            end
            RAM_write1b: begin
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                if (m00_axi_awready) begin
                    if(wait_finish_ARM_cycle==0) begin
                        if((SIZ40[1:0]==2'b11) && (GPIO_c[2]==1))  //line // BURST WRITE ENABLE
                            RAM_state <= RAM_write_burst1;
                        else
                            RAM_state <= RAM_write2;
                    end else begin
                        RAM_state <= RAM_write2;
                    end
                end
            end
            RAM_write_burst1: begin
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_awvalid <= 1'b0;
                if (m00_axi_wready) begin
                    RAM_state <=RAM_write_burst1a;
                end
            end
            RAM_write_burst1a: begin
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_awvalid <= 1'b0;
                if (m00_axi_wready && m00_axi_awready) begin
                    RAM_state <=RAM_write_burst1b;
                    nTA1 <= 1'b0;
                end
            end
            RAM_write_burst1b: begin
                nTA1 <= 1'b0;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                if(ENcondition==1'b1) begin
                    RAM_state <=RAM_write_burst1c;
                end
            end
            RAM_write_burst1c: begin
                nTA1 <= 1'b1;
                m00_axi_wvalid <= 1;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst1d;
                nTA1 <= 1'b0;
                end
            end
            RAM_write_burst1d: begin
                nTA1 <= 1'b0;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 0;
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst2;
            end
            end
            RAM_write_burst2: begin
                nTA1 <= 1'b1;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 1;
                if (m00_axi_wready) begin
                RAM_state <= RAM_write_burst21;
                nTA1 <= 1'b0;
                end
            end
            RAM_write_burst21: begin
                nTA1 <= 1'b0;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 0;
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst3;
            end
            end
            RAM_write_burst3: begin
                nTA1 <= 1'b1;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 1;
                if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst31;
                nTA1 <= 1'b0;
                end
            end
            RAM_write_burst31: begin
                nTA1 <= 1'b0;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 0;
                if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst4;
            end
            end
            RAM_write_burst4: begin
                nTA1 <= 1'b0;
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 1;
                m00_axi_wlast <= 1;
                if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst41;
                    nTA1 <= 1'b1;
                    nTBI<=1'b1;
                end
            end
            RAM_write_burst41: begin
//                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_wvalid <= 0;
                m00_axi_wlast <= 0;
                if (m00_axi_bvalid) begin
                    RAM_state <= RAM_idle;
                end
            end
    
            RAM_write2: begin
//                m00_axi_wdata[31:0] <= D040[31:0];
                m00_axi_wdata[31:0] <= {D040[7:0],D040[15:8],D040[23:16],D040[31:24]}; // BS
                m00_axi_awvalid <= 1'b0;
                m00_axi_wvalid <= 1;
                m00_axi_wlast <= 'h1;
                if (m00_axi_wready) begin
                    RAM_state <= RAM_write21;
//                    RAM_state <= RAM_write3;
                end
                ack<=0;
            end
            RAM_write21: begin
                    m00_axi_wvalid <= 0;
                    m00_axi_wlast <= 'h0;
                if (m00_axi_bvalid) begin
                    RAM_state <= RAM_write3;
                end
            end
            RAM_write3: begin
                m00_axi_wvalid <= 0;
                m00_axi_wlast <= 'h0;
                if(wait_finish_ARM_cycle==0) begin
                    if(ENcondition==1'b1) begin
                        RAM_state <= RAM_write31;
                        nTA1 <= 1'b0;
                    end
                end
                else begin
                    if((GPIO_IN[30]==1) && (ack==0)) begin // ack read/write
                        GPIO_OUT[31:0] <=32'h0000_0000;
                        ack<=1;
                    end
                    else begin
                        if((GPIO_IN[30]==0) && (ack==1)) begin // ack read/write
                            GPIO_OUT[31:0] <=32'h0000_0000;
                            if(ENcondition==1'b1) begin
                                RAM_state <= RAM_write31;
                                nTA1 <= 1'b0;
                            end
                        end
                    end
                end
            end
            RAM_write31: begin
                nTA1 <= 1'b0;
                if(ENcondition==1'b1) begin
                    RAM_state <= RAM_idle;
                    nTA1 <= 1'b1;
                end
            end

            default: begin
                RAM_state <= RAM_idle;
            end
        endcase
    end
end


  // AXI4LITE signals
  reg  [4:0] s01_axi_awaddr;
  reg        s01_axi_awready;
  reg        s01_axi_wready;
  reg  [1:0] s01_axi_bresp;
  reg        s01_axi_bvalid;
  reg  [4:0] s01_axi_araddr;
  reg        s01_axi_arready;
  reg [31:0] s01_axi_rdata;
  reg  [1:0] s01_axi_rresp;
  reg        s01_axi_rvalid;

  //-- Number of Slave Registers 4
  reg [31:0] s01_slv_reg0;
  reg [31:0] s01_slv_reg1;
  reg [31:0] s01_slv_reg2;
  reg [31:0] s01_slv_reg3;
  reg [31:0] s01_slv_reg4;
  reg [31:0] s01_slv_reg5;
  wire       s01_slv_reg_rden;
  wire       s01_slv_reg_wren;
  reg [31:0] s01_reg_data_out;
  reg        s01_aw_en;

  // I/O Connections assignments

  assign S01_AXI_AWREADY = s01_axi_awready;
  assign S01_AXI_WREADY  = s01_axi_wready;
  assign S01_AXI_BRESP   = s01_axi_bresp;
  assign S01_AXI_BVALID  = s01_axi_bvalid;
  assign S01_AXI_ARREADY = s01_axi_arready;
  assign S01_AXI_RDATA   = s01_axi_rdata;
  assign S01_AXI_RRESP   = s01_axi_rresp;
  assign S01_AXI_RVALID  = s01_axi_rvalid;
  // Implement axi_awready generation
  // axi_awready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
  // de-asserted when reset is low.

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_awready <= 1'b0;
          s01_aw_en <= 1'b1;
        end
      else
        begin
          if (~s01_axi_awready && S01_AXI_AWVALID && S01_AXI_WVALID && s01_aw_en)
            begin
              // slave is ready to accept write address when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              s01_axi_awready <= 1'b1;
              s01_aw_en <= 1'b0;
            end
          else if (S01_AXI_BREADY && s01_axi_bvalid)
            begin
              s01_aw_en <= 1'b1;
              s01_axi_awready <= 1'b0;
            end
               else
                 begin
                   s01_axi_awready <= 1'b0;
                 end
        end
    end

  // Implement axi_awaddr latching
  // This process is used to latch the address when both
  // S_AXI_AWVALID and S_AXI_WVALID are valid.

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_awaddr <= 0;
        end
      else
        begin
          if (~s01_axi_awready && S01_AXI_AWVALID && S01_AXI_WVALID && s01_aw_en)
            begin
              // Write Address latching
              s01_axi_awaddr <= S01_AXI_AWADDR;
            end
        end
    end

  // Implement axi_wready generation
  // axi_wready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is
  // de-asserted when reset is low.

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_wready <= 1'b0;
        end
      else
        begin
          if (~s01_axi_wready && S01_AXI_WVALID && S01_AXI_AWVALID && s01_aw_en )
            begin
              // slave is ready to accept write data when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              s01_axi_wready <= 1'b1;
            end
          else
            begin
              s01_axi_wready <= 1'b0;
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
  assign s01_slv_reg_wren = s01_axi_wready && S01_AXI_WVALID && s01_axi_awready && S01_AXI_AWVALID;

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_slv_reg0 <= 0;
          s01_slv_reg1 <= 0;
          s01_slv_reg2 <= 0;
          s01_slv_reg3 <= 0;
          s01_slv_reg4 <= 0;
          s01_slv_reg5 <= 0;
        end
      else begin
        if (s01_slv_reg_wren)
          begin
            case ( s01_axi_awaddr[4:2] )
              3'h0: s01_slv_reg0 <= S01_AXI_WDATA; // GPIO_IN
              3'h1: s01_slv_reg1 <= S01_AXI_WDATA; // GPIO_OUT
              3'h2: s01_slv_reg2 <= S01_AXI_WDATA; // 
              3'h3: s01_slv_reg3 <= S01_AXI_WDATA; // 
              3'h4: s01_slv_reg4 <= S01_AXI_WDATA; // 
              3'h5: s01_slv_reg5 <= S01_AXI_WDATA; // GPIO_READ_IN
              default : begin
                s01_slv_reg0 <= s01_slv_reg0;
                s01_slv_reg1 <= s01_slv_reg1;
                s01_slv_reg2 <= s01_slv_reg2;
                s01_slv_reg3 <= s01_slv_reg3;
                s01_slv_reg4 <= s01_slv_reg4;
                s01_slv_reg5 <= s01_slv_reg5;
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

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_bvalid  <= 0;
          s01_axi_bresp   <= 2'b0;
        end
      else
        begin
          if (s01_axi_awready && S01_AXI_AWVALID && ~s01_axi_bvalid && s01_axi_wready && S01_AXI_WVALID)
            begin
              // indicates a valid write response is available
              s01_axi_bvalid <= 1'b1;
              s01_axi_bresp  <= 2'b0; // 'OKAY' response
            end                   // work error responses in future
          else
            begin
              if (S01_AXI_BREADY && s01_axi_bvalid)
                //check if bready is asserted while bvalid is high)
                //(there is a possibility that bready is always asserted high)
                begin
                  s01_axi_bvalid <= 1'b0;
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

  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_arready <= 1'b0;
          s01_axi_araddr  <= 32'b0;
        end
      else
        begin
          if (~s01_axi_arready && S01_AXI_ARVALID)
            begin
              // indicates that the slave has acceped the valid read address
              s01_axi_arready <= 1'b1;
              // Read address latching
              s01_axi_araddr  <= S01_AXI_ARADDR;
            end
          else
            begin
              s01_axi_arready <= 1'b0;
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
  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_rvalid <= 0;
          s01_axi_rresp  <= 0;
        end
      else
        begin
          if (s01_axi_arready && S01_AXI_ARVALID && ~s01_axi_rvalid)
            begin
              // Valid read data is available at the read data bus
              s01_axi_rvalid <= 1'b1;
              s01_axi_rresp  <= 2'b0; // 'OKAY' response
            end
          else if (s01_axi_rvalid && S01_AXI_RREADY)
            begin
              // Read data is accepted by the master
              s01_axi_rvalid <= 1'b0;
            end
        end
    end

  // Output register or memory read data
  always @( posedge S01_AXI_ACLK )
    begin
      if ( S01_AXI_ARESETN == 1'b0 )
        begin
          s01_axi_rdata  <= 0;
        end
      else
        begin
          // When there is a valid read address (S_AXI_ARVALID) with
          // acceptance of read address by the slave (axi_arready),
          // output the read dada
          if (s01_slv_reg_rden)
            begin
              s01_axi_rdata <= s01_reg_data_out;     // register read data
            end
        end
    end

  // end of AXI-Lite interface ==================================================


  assign GPIO_IN  = s01_slv_reg0;
  assign GPIO_READ_IN  = s01_slv_reg5;

  assign s01_slv_reg_rden = s01_axi_arready & S01_AXI_ARVALID & ~s01_axi_rvalid;
  always @(*)
  begin
      case (s01_axi_araddr[4:2])
        3'h0 : s01_reg_data_out <= GPIO_IN;
        3'h1 : s01_reg_data_out <= GPIO_OUT;
        3'h2 : s01_reg_data_out <= GPIO_VERS;
        3'h3 : s01_reg_data_out <= s01_slv_reg3;
        3'h4 : s01_reg_data_out <= s01_slv_reg4;
        3'h5 : s01_reg_data_out <= GPIO_READ_IN;
        3'h6 : s01_reg_data_out <= GPIO_DATA; // debug 060 data bus
        3'h7 : s01_reg_data_out <= GPIO_ADD;  // debug 060 address bus
        default: s01_reg_data_out <= 'h0;
      endcase
  end






  // AXI4LITE signals
  reg [31:0] s00_axi_awaddr;
  reg        s00_axi_awready;
  reg        s00_axi_wready;
  reg  [1:0] s00_axi_bresp;
  reg        s00_axi_bvalid;
  reg [31:0] s00_axi_araddr;
  reg        s00_axi_arready;
  reg [31:0] s00_axi_rdata;
  reg  [1:0] s00_axi_rresp;
  reg        s00_axi_rvalid;

  //-- Number of Slave Registers 4
  reg [31:0] s00_slv_reg0;
  reg [31:0] s00_slv_reg1;
  reg [31:0] s00_slv_reg2;
  reg [31:0] s00_slv_reg3;
  reg [31:0] s00_slv_reg4;
  reg [31:0] s00_slv_reg5;
  reg [31:0] s00_slv_reg6;
  wire       s00_slv_reg_rden;
  wire       s00_slv_reg_wren;
  wire       clean_arm_command;
  reg [31:0] s00_reg_data_out;
  reg        s00_aw_en;

  reg [31:0] s00_out_reg0;
  reg [31:0] s00_out_reg1;
  reg [31:0] s00_out_reg2;
  reg [31:0] s00_out_reg3;
  reg [31:0] s00_out_reg4;
  reg [31:0] s00_out_reg5;
  reg [31:0] s00_out_reg6;
  reg [31:0] s00_out_reg7;

  // I/O Connections assignments

  assign S00_AXI_AWREADY = s00_axi_awready;
  assign S00_AXI_WREADY  = s00_axi_wready;
  assign S00_AXI_BRESP   = s00_axi_bresp;
  assign S00_AXI_BVALID  = s00_axi_bvalid;
  assign S00_AXI_ARREADY = s00_axi_arready;
  assign S00_AXI_RDATA   = s00_axi_rdata;
  assign S00_AXI_RRESP   = s00_axi_rresp;
  assign S00_AXI_RVALID  = s00_axi_rvalid;
  // Implement axi_awready generation
  // axi_awready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
  // de-asserted when reset is low.

  always @( posedge S00_AXI_ACLK )
    begin
      if ( S00_AXI_ARESETN == 1'b0 )
        begin
          s00_axi_awready <= 1'b0;
          s00_aw_en <= 1'b1;
        end
      else
        begin
          if (~s00_axi_awready && S00_AXI_AWVALID && S00_AXI_WVALID && s00_aw_en)
            begin
              // slave is ready to accept write address when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              s00_axi_awready <= 1'b1;
              s00_aw_en <= 1'b0;
            end
          else if (S00_AXI_BREADY && s00_axi_bvalid)
            begin
              s00_aw_en <= 1'b1;
              s00_axi_awready <= 1'b0;
            end
               else
                 begin
                   s00_axi_awready <= 1'b0;
                 end
        end
    end

  // Implement axi_awaddr latching
  // This process is used to latch the address when both
  // S_AXI_AWVALID and S_AXI_WVALID are valid.

  always @( posedge S00_AXI_ACLK )
    begin
      if ( S00_AXI_ARESETN == 1'b0 )
        begin
          s00_axi_awaddr <= 0;
        end
      else
        begin
          if (~s00_axi_awready && S00_AXI_AWVALID && S00_AXI_WVALID && s00_aw_en)
            begin
              // Write Address latching
              s00_axi_awaddr <= S00_AXI_AWADDR;
            end
        end
    end

  // Implement axi_wready generation
  // axi_wready is asserted for one S_AXI_ACLK clock cycle when both
  // S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is
  // de-asserted when reset is low.

  always @( posedge S00_AXI_ACLK )
    begin
      if ( S00_AXI_ARESETN == 1'b0 )
        begin
          s00_axi_wready <= 1'b0;
        end
      else
        begin
          if (~s00_axi_wready && S00_AXI_WVALID && S00_AXI_AWVALID && s00_aw_en )
            begin
              // slave is ready to accept write data when
              // there is a valid write address and write data
              // on the write address and data bus. This design
              // expects no outstanding transactions.
              s00_axi_wready <= 1'b1;
            end
          else
            begin
              s00_axi_wready <= 1'b0;
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
  assign s00_slv_reg_wren = s00_axi_wready && S00_AXI_WVALID && s00_axi_awready && S00_AXI_AWVALID;

  always @( posedge S00_AXI_ACLK ) begin
      if ( S00_AXI_ARESETN == 1'b0 ) begin
          s00_slv_reg0 <= 0;
          s00_slv_reg1 <= 0;
          s00_slv_reg2 <= 0;
          s00_slv_reg3 <= 0;
          s00_slv_reg4 <= 0;
          s00_slv_reg5 <= 0;
          s00_slv_reg6 <= 0;
      end else begin
          if (s00_slv_reg_wren) begin
              if(s00_axi_awaddr[27:26]==2'b00) begin
                  case ( s00_axi_awaddr[4:2] )
                      3'h0: s00_slv_reg0 <= S00_AXI_WDATA; // was GPIO_IN, not used now
                      3'h1: s00_slv_reg1 <= S00_AXI_WDATA; // was GPIO_OUT, not used now
                      3'h2: s00_slv_reg2 <= S00_AXI_WDATA; // address
                      3'h3: s00_slv_reg3 <= S00_AXI_WDATA; // dataout
                      3'h4: s00_slv_reg4 <= S00_AXI_WDATA; // control
                      3'h5: s00_slv_reg5 <= S00_AXI_WDATA; // command
                      3'h6: s00_slv_reg6 <= S00_AXI_WDATA; // bank
                      default : begin
                          s00_slv_reg0 <= s00_slv_reg0;
                          s00_slv_reg1 <= s00_slv_reg1;
                          s00_slv_reg2 <= s00_slv_reg2;
                          s00_slv_reg3 <= s00_slv_reg3;
                          s00_slv_reg4 <= s00_slv_reg4;
                          s00_slv_reg5 <= s00_slv_reg5;
                          s00_slv_reg6 <= s00_slv_reg6;
                      end
                  endcase
              end else if(s00_axi_awaddr[27:26]==2'b01) begin
                  s00_slv_reg2 <= {ARM_BANK[7:0],s00_axi_awaddr[25:2]};
                  s00_slv_reg3 <= S00_AXI_WDATA;
                  s00_slv_reg4 <= {24'h0,6'b000100,s00_axi_awaddr[28],1'b1}; // long
              end else if(s00_axi_awaddr[27:26]==2'b10) begin
                  s00_slv_reg2 <= {ARM_BANK[7:0],s00_axi_awaddr[25:2]};
                  s00_slv_reg3 <= S00_AXI_WDATA;
                  s00_slv_reg4 <= {24'h0,6'b000110,s00_axi_awaddr[28],1'b1}; // word
              end else /*if(s00_axi_awaddr[27:26]==2'b11)*/ begin
                  s00_slv_reg2 <= {ARM_BANK[7:0],s00_axi_awaddr[25:2]};
                  s00_slv_reg3 <= S00_AXI_WDATA;
                  s00_slv_reg4 <= {24'h0,6'b000101,s00_axi_awaddr[28],1'b1}; // byte
              end
          end
          if(clean_arm_command) begin
              s00_slv_reg4[4]<= 1'b0;
          end
      end
  end

assign ARM_ADDRESS = s00_slv_reg2;
assign ARM_DATA    = s00_slv_reg3;
assign ARM_BG      = s00_slv_reg4[0];
assign ARM_RnW     = s00_slv_reg4[1];
assign ARM_SIZ     = s00_slv_reg4[3:2];
assign ARM_COMMAND = s00_slv_reg4[4];
assign ARM_BANK    = s00_slv_reg6[7:0];

assign A060   = ARM_BG == 1'b1 ? A060_out   : 32'bz;
assign R_W040 = ARM_BG == 1'b1 ? RW040_out  :  1'bz;
assign SIZ40  = ARM_BG == 1'b1 ? SIZ40_out  :  2'bz;
//assign nTS    = ARM_BG == 1'b1 ? nTS_out    :  1'bz;
//assign nTA    = nTA1 == 1'b0 ? 1'b0 : 1'bz;
assign nTA    = nTA1;

assign nTS_FPGA_out = ARM_BG == 1'b1 ? nTS_out : nTS_FPGA;

localparam ARM_STATE_idle = 5'd0;
localparam ARM_STATE_waiting_command = 5'd1;
localparam ARM_STATE_write = 5'd2;
localparam ARM_STATE_write0 = 5'd3;
localparam ARM_STATE_write1 = 5'd4;
localparam ARM_STATE_write2 = 5'd5;
localparam ARM_STATE_read = 5'd6;
localparam ARM_STATE_read0 = 5'd7;
localparam ARM_STATE_read1 = 5'd8;
localparam ARM_STATE_read2 = 5'd9;

  wire reg5_is_read = s00_slv_reg_rden && s00_axi_araddr[4:2]==3'h5;

  reg [5:0] ARM_state=ARM_STATE_idle;
  assign clean_arm_command = ARM_state == ARM_STATE_read || ARM_state == ARM_STATE_write;

  always @( posedge S00_AXI_ACLK )
    begin
        if((RESET_IN==1'b1) ) begin
            ARM_state<= ARM_STATE_idle;
            s00_out_reg5 <= 32'h0;
            nTS_out <= 1'b1;
        end else begin
            case (ARM_state)
                ARM_STATE_idle: begin
                    nTS_out <= 1'b1;
                    if (ARM_BG==1'b1) begin
                        ARM_state<= ARM_STATE_waiting_command;
                    end
                end
                ARM_STATE_waiting_command: begin
                    nTS_out <= 1'b1;
                    A060_out <= ARM_ADDRESS;
                    SIZ40_out <= ARM_SIZ;
                    RW040_out <= ARM_RnW;
                    if (ARM_COMMAND==1'b1) begin
                        s00_out_reg5 <= 32'h0;
                        nTS_out <= 1'b0;
                        if(ARM_RnW==1'b1)
                            ARM_state <= ARM_STATE_read;
                        else
                            ARM_state <= ARM_STATE_write;
                    end
                    if (ARM_BG==1'b0) begin
                        ARM_state<= ARM_STATE_idle;
                    end
                end
                ARM_STATE_read: begin
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                    if(ENcondition==1'b1)
//                        ARM_state <= ARM_STATE_read1;
                    ARM_state <= ARM_STATE_read0;
                end
                ARM_STATE_read0: begin
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                    if(ENcondition==1'b0)
                        ARM_state <= ARM_STATE_read1;
                end
                ARM_STATE_read1: begin
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                    if(ENcondition==1'b1)
                        ARM_state <= ARM_STATE_read2;
                end
                ARM_STATE_read2: begin
                    nTS_out <= 1'b1;
                    s00_out_reg7 <= D040;
                    if(nTA==1'b0 && nTEA==1'b1) begin
                        s00_out_reg5 <= 32'h1;  // read cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else if(nTA==1'b1 && nTEA==1'b0) begin
                        s00_out_reg5 <= 32'h2;  // read cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else if(nTA==1'b0 && nTEA==1'b0) begin
                        s00_out_reg5 <= 32'h3;  // read cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else begin
                        s00_out_reg5 <= 32'h0;
                        ARM_state <= ARM_STATE_read2;
                    end
                end
                ARM_STATE_write: begin
                    D040_out <= ARM_DATA;
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                     if(ENcondition==1'b1)
//                        ARM_state <= ARM_STATE_write1;
                    ARM_state <= ARM_STATE_write0;
                end
                ARM_STATE_write0: begin
                    D040_out <= ARM_DATA;
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                     if(ENcondition==1'b0)
                        ARM_state <= ARM_STATE_write1;
                end
                ARM_STATE_write1: begin
                    D040_out <= ARM_DATA;
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b0;
                    if(ENcondition==1'b1)
                        ARM_state <= ARM_STATE_write2;
                end
                ARM_STATE_write2: begin
                    D040_out <= ARM_DATA;
                    nTS_out <= 1'b1;
                    if(nTA==1'b0 && nTEA==1'b1) begin
                        s00_out_reg5 <= 32'h1; // write cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else if(nTA==1'b1 && nTEA==1'b0) begin
                        s00_out_reg5 <= 32'h2; // write cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else if(nTA==1'b0 && nTEA==1'b0) begin
                        s00_out_reg5 <= 32'h3; // write cycle finished
                        ARM_state <= ARM_STATE_idle;
                    end
                    else begin
                        s00_out_reg5 <= 32'h0;
                        ARM_state  <= ARM_STATE_write2;
                    end
                end
                default: begin
                    ARM_state <= ARM_STATE_idle;
                    s00_out_reg5 <= 32'h0;
                    nTS_out <= 1'b1;
                end
            endcase
        end
    end

  // Implement write response logic generation
  // The write response and response valid signals are asserted by the slave
  // when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.
  // This marks the acceptance of address and indicates the status of
  // write transaction.

  always @( posedge S00_AXI_ACLK ) begin
      if ( S00_AXI_ARESETN == 1'b0 ) begin
          s00_axi_bvalid  <= 0;
          s00_axi_bresp   <= 2'b0;
      end else begin
          if (s00_axi_awready && S00_AXI_AWVALID && ~s00_axi_bvalid && s00_axi_wready && S00_AXI_WVALID) begin
              // indicates a valid write response is available
              s00_axi_bvalid <= 1'b1;
              s00_axi_bresp  <= 2'b0; // 'OKAY' response
          end                   // work error responses in future
          else begin
              if (S00_AXI_BREADY && s00_axi_bvalid) begin
                //check if bready is asserted while bvalid is high)
                //(there is a possibility that bready is always asserted high)
                  s00_axi_bvalid <= 1'b0;
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

  always @( posedge S00_AXI_ACLK ) begin
      if ( S00_AXI_ARESETN == 1'b0 ) begin
          s00_axi_arready <= 1'b0;
          s00_axi_araddr  <= 32'b0;
      end else begin
          if (~s00_axi_arready && S00_AXI_ARVALID) begin
              // indicates that the slave has acceped the valid read address
              s00_axi_arready <= 1'b1;
              // Read address latching
              s00_axi_araddr  <= S00_AXI_ARADDR;
          end else begin
              s00_axi_arready <= 1'b0;
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
  always @( posedge S00_AXI_ACLK )
    begin
      if ( S00_AXI_ARESETN == 1'b0 )
        begin
          s00_axi_rvalid <= 0;
          s00_axi_rresp  <= 0;
        end
      else
        begin
          if (s00_axi_arready && S00_AXI_ARVALID && ~s00_axi_rvalid)
            begin
              // Valid read data is available at the read data bus
              s00_axi_rvalid <= 1'b1;
              s00_axi_rresp  <= 2'b0; // 'OKAY' response
            end
          else if (s00_axi_rvalid && S00_AXI_RREADY)
            begin
              // Read data is accepted by the master
              s00_axi_rvalid <= 1'b0;
            end
        end
    end

  // Output register or memory read data
  always @( posedge S00_AXI_ACLK )
    begin
      if ( S00_AXI_ARESETN == 1'b0 )
        begin
          s00_axi_rdata  <= 0;
        end
      else
        begin
          // When there is a valid read address (S_AXI_ARVALID) with
          // acceptance of read address by the slave (axi_arready),
          // output the read dada
          if (s00_slv_reg_rden)
            begin
              s00_axi_rdata <= s00_reg_data_out;     // register read data
            end
        end
    end

  // end of AXI-Lite interface ==================================================

  assign s00_slv_reg_rden = s00_axi_arready & S00_AXI_ARVALID & ~s00_axi_rvalid;
  always @(*)
  begin
      case (s00_axi_araddr[4:2])
        3'h0 : s00_reg_data_out <= s00_slv_reg0;
        3'h1 : s00_reg_data_out <= s00_slv_reg1;
        3'h2 : s00_reg_data_out <= s00_slv_reg2;
        3'h3 : s00_reg_data_out <= s00_slv_reg3;
        3'h4 : s00_reg_data_out <= s00_slv_reg4;
        3'h5 : s00_reg_data_out <= s00_out_reg5;
        3'h6 : s00_reg_data_out <= s00_slv_reg6;
        3'h7 : s00_reg_data_out <= s00_out_reg7;
        default: s00_reg_data_out <= 'h0;
      endcase
  end



endmodule
