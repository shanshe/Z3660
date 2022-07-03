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
//(* mark_debug = "true" *)    input clk,
    output interrupt,
    inout [31:0] D040,
    input [31:0] A060,
    input R_W040,
    output reg nTA,
    input nTEA,
    input nTCI,
    input PCLK_clk,
    input BCLK_clk,
//    input CLK90_clk,
//    input CPUCLK_clk,
    input nTS,
    input [1:0] SIZ40,
    output reg NU_1,
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
    input wire [31:0] GPIO_IN,
    output reg [31:0] GPIO_OUT
    );
    reg toggle_bit = 1;

    assign interrupt = 1'b0;
    assign  BP = 1'b0;

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

wire sdr_req_pending;
reg [31:0] data;
reg [31:0] data_burst1;
reg [31:0] data_burst2;
reg [31:0] data_burst3;
reg [31:0] data_burst4;
//assign DDIR=(~RW) & DDIR_no;
//assign DOE = 1'd0;

reg BCLK_rising=0;
reg BCLK_falling=0;
reg [1:0] BCLK_d=0;

localparam RAM_idle = 6'd0;
//localparam RAM_write1 = 6'd1;
localparam RAM_write2 = 6'd2;
localparam RAM_write3 = 6'd3;
//localparam RAM_read1 = 6'd4;
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
localparam RAM_write_burst1d = 6'd29;
localparam RAM_write_burst1b = 6'd30;
localparam RAM_write_burst1c = 6'd31;
localparam RAM_TS0 = 6'd32;
localparam RAM_TS1 = 6'd33;
localparam RAM_TS2 = 6'd34;

(* mark_debug = "true" *) reg [5:0] RAM_state = RAM_idle;

reg [3:0]nTS_d=4'b1111;
reg [31:0]GPIO_c=0;
reg [31:0]GPIO_m=0;
reg [31:0]GPIO_s=0;
reg TScondition=0;
reg ENcondition=0;
reg active_cycle=0;
(* mark_debug = "true" *) reg [1:0] configured = 2'b00;
(* mark_debug = "true" *) reg [1:0] shutup = 2'b00;
(* mark_debug = "true" *) reg [15:0] autoConfigBaseFastRam = 16'h0000;
(* mark_debug = "true" *) reg [15:0] autoConfigBaseRTG = 16'h0000;

wire AUTOCONFIG_RANGE = ({A060[31:16]} == {16'hFF00}) && ~&shutup[1:0] && ~&configured[1:0];

wire FASTRAM_CONFIGURED_RANGE = (A060[31:0] >= {autoConfigBaseFastRam[15:0],16'h0000}) && (A060[31:0] < ({autoConfigBaseFastRam[15:0],16'h0000}+32'h10000000)) && configured[0];
wire RTG_CONFIGURED_RANGE = (A060[31:0] >= {autoConfigBaseRTG[15:0],16'h0000}) && (A060[31:0] < ({autoConfigBaseRTG[15:0],16'h0000}+32'h08000000)) && configured[1];

    always @(posedge m00_axi_aclk) begin
        GPIO_m<=GPIO_IN;
        GPIO_s<=GPIO_m;
//        if(GPIO_s[0]==1) //RAM ENABLE
            nTS_d <= {nTS_d[2:0],nTS};
//        else
//            nTS_d <= 4'b1111;
        
        if(GPIO_s[6:4]==3'b000)
            TScondition<=(nTS_d[2:1]==2'b10)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b001)
            TScondition<=(nTS_d[1:0]==2'b10)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b010)
            TScondition<=(nTS_d[0]==1'b0)/*&&(GPIO_s[0]==1'b1)*/&&(active_cycle==1'b0)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b011)
            TScondition<=(nTS==1'b0)/*&&(GPIO_s[0]==1'b1)*/&&(active_cycle==1'b0)?1'b1:1'b0; // falling edge
        else if(GPIO_s[6:4]==3'b100)
            TScondition<=(nTS_d[2:1]==2'b01)?1'b1:1'b0; // rising edge
        else if(GPIO_s[6:4]==3'b101)
            TScondition<=(nTS_d[1:0]==2'b01)?1'b1:1'b0; // rising edge
    end
    always @(negedge m00_axi_aclk) begin
        if(GPIO_c[9:8]==2'b00)
            ENcondition<=(nCLKEN_clk==1'b0)&&(PCLK_clk==1'b0)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b01)
            ENcondition<=(nCLKEN_clk==1'b0)&&(PCLK_clk==1'b1)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b10)
            ENcondition<=(nCLKEN_clk==1'b0)?1'b1:1'b0;
        else if(GPIO_c[9:8]==2'b11)
            ENcondition<=BCLK_falling==1?1'b1:1'b0;
    end

assign D040[31:0]= (RAM_state == RAM_read2)||(RAM_state == RAM_read3)||(RAM_state == RAM_read31) ||
                   (RAM_state == RAM_read4)||(RAM_state == RAM_read41)||(RAM_state == RAM_read42) ||
                   (RAM_state == RAM_read43)||(RAM_state == RAM_read44)||
                   (RAM_state == RAM_read_burst2)||(RAM_state == RAM_read_burst3)||(RAM_state == RAM_read_burst4)
                   ? data[31:0] : 32'bz;
always @(posedge m00_axi_aclk) begin

    BCLK_d<={BCLK_d[0],BCLK_clk};
    if (BCLK_d==2'b01)
        BCLK_rising<=1;
    else
        BCLK_rising<=0;
    if (BCLK_d==2'b10)
        BCLK_falling<=1;
    else
        BCLK_falling<=0;
end
always @(posedge m00_axi_aclk) begin
    if((RESET_IN==1'b1) || (GPIO_s[31]==1)) begin
        m00_axi_arlen <= 'h0;
        m00_axi_arburst <= 'h0;
//        m00_axi_rready <= 1;
        m00_axi_awlen <= 'h0;
        m00_axi_awburst <= 'h0;
        m00_axi_wlast <= 'h1;
        nTA<= 1'bz;
        nTBI <= 1'b1;
        RAM_state <= RAM_idle;
        active_cycle <= 1'b0;
        configured <= 2'b00;
        shutup <= 2'b00;
        autoConfigBaseFastRam[15:0] = 16'h0000;
        autoConfigBaseRTG[15:0] = 16'h0000;
        GPIO_OUT[31:0] <= 32'h0;
    end else begin
        nTA<= 1'bz;
        case (RAM_state)
            RAM_idle: begin
        active_cycle <= 1'b0;
//                if (nTS_d[2:1]==2'b01) begin // rising edge
//                if (nTS_d[2:1]==2'b10) begin // falling edge
//                if (nTS_d[1:0]==2'b10) begin // falling edge
//                if ((nTS==1'b0)&(GPIO[0]==1'b1)) begin // falling edge
                if (TScondition==1'b1) begin
        active_cycle <= 1'b1;
                    GPIO_c<=GPIO_s;
                    nTBI<=1'b1;
                    if ( (A060[31:27]== 5'b00001)           // CPU-RAM  128Mb
                         ||(FASTRAM_CONFIGURED_RANGE==1'b1) // Z3 RAM   256Mb
                         ||(RTG_CONFIGURED_RANGE==1'b1)     // Z3 RTG
                       )begin
                        if (R_W040==0)                    // Write cycle
                            begin
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
                                        m00_axi_wstrb   <= 4'b1100;
                                    else if (A060[1:0]==2'b10)
                                        m00_axi_wstrb   <= 4'b0011;
                                    else
                                        m00_axi_wstrb   <= 4'b0000; // impossible case?
                                end
                                else begin // byte
                                    if (A060[1:0]==2'b00)
                                        m00_axi_wstrb   <= 4'b1000;
                                    else if (A060[1:0]==2'b01)
                                        m00_axi_wstrb   <= 4'b0100;
                                    else if (A060[1:0]==2'b10)
                                        m00_axi_wstrb   <= 4'b0010;
                                    else //if (A060[1:0]==2'b11)
                                        m00_axi_wstrb   <= 4'b0001;
                                end
                                if(FASTRAM_CONFIGURED_RANGE==1'b1)
                                    m00_axi_awaddr[31:0] <= {A060[31:2],2'b00}-{autoConfigBaseFastRam[15:0],16'h0000}+{32'h1000_0000};
                                else if(RTG_CONFIGURED_RANGE==1'b1) begin
                                    m00_axi_awaddr[31:0] <= {A060[31:2],2'b00}-{autoConfigBaseRTG,16'h0000}+{32'h0000_0000};
                                    if({A060[31:2],2'b00}-{autoConfigBaseRTG,16'h0000}+{32'h0000_0000} <= 32'h0001_0000) begin // register memory zone
                                        GPIO_OUT[31:0] <= {A060[31:2],2'b00}-{autoConfigBaseRTG,16'h0000}+{toggle_bit,31'h0000_0000};
                                        toggle_bit<=~toggle_bit;
                                    end
                                end
                                else
                                    m00_axi_awaddr[31:0] <= {A060[31:2],2'b00};
                                m00_axi_wdata[31:0] <= D040[31:0];
                                m00_axi_awvalid  <= 1;
                                RAM_state <= RAM_write1b;
                            end
                        else  // Read Cycle
                            begin
                            if(FASTRAM_CONFIGURED_RANGE==1'b1)
                                m00_axi_araddr[31:0] <= {A060[31:2],2'b00}-{autoConfigBaseFastRam[15:0],16'h0000}+{32'h1000_0000};
                            else if(RTG_CONFIGURED_RANGE==1'b1)
                                m00_axi_araddr[31:0] <= {A060[31:2],2'b00}-{autoConfigBaseRTG,16'h0000}+{32'h0000_0000};
                            else
                                m00_axi_araddr[31:0]  <= {A060[31:2],2'b00};
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
                            RAM_state <= RAM_read1b;
                        end
                    end

                    else if(AUTOCONFIG_RANGE==1'b1) begin
                        if (R_W040==0) begin                    // Write cycle
                            RAM_state <= RAM_write3;
                            casex (A060[15:0])
                                16'hXX44: begin

                                    if (configured[1:0] == 2'b00) begin
                                        autoConfigBaseFastRam[15:0] <= D040[31:16];     // FastRAM
                                        configured[0] <= 1'b1;
                                    end
                                    else
                                    if (configured[1:0] == 2'b01) begin
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
                        else  begin // Read Cycle
                        RAM_state <= RAM_read3;
                
                            case (A060[15:0])
                                16'h0000: begin
                                    if ({configured[1:0] == 2'b00}) data[31:0] <= {16'b1010_1111_1111_1111,16'hFFFF}; // zorro 3 (10), pool link (1), autoboot ROM no (0)
                                    else 
                                    if ({configured[1:0] == 2'b01}) data[31:0] <= {16'b1000_1111_1111_1111,16'hFFFF}; // zorro 3 (10), no pool link (0), autoboot ROM no (0)
                                end

                                16'h0100: begin
                                    if (configured[1:0] == 2'b00) data[31:0] <= {16'b0100_1111_1111_1111,16'hFFFF}; // next board unrelated (0), 256MB FastRAM
                                    else
                                    if (configured[1:0] == 2'b01) data[31:0] <= {16'b1011_1111_1111_1111,16'hFFFF}; // next board unrelated (0), 128MB RTG
//                                    if ({configured[2:0] == 3'b011}) autoConfigData <= 4'h1;     // (02) IO Port B
                                end
                                16'h0004: begin
                                    data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // product number
                                end
                                16'h0104: begin
                                    if (configured[1:0] == 2'b00) data[31:0] <= {16'b1010_1111_1111_1111,16'hFFFF}; // 5 for the 256MB Z3 Fast
                                    else
                                    if (configured[1:0] == 2'b01) data[31:0] <= {16'b1011_1111_1111_1111,16'hFFFF}; // 4 for the RTG PIC
                                end
                                16'h0008: begin
                                    if (configured[1:0] == 2'b00) data[31:0] <= {16'b1000_1111_1111_1111,16'hFFFF}; // flags inverted 0111 io,shutup,extension,reserved(1)
                                    else
                                    if (configured[1:0] == 2'b01) data[31:0] <= {16'b1000_1111_1111_1111,16'hFFFF}; // flags inverted 0111 io,shutup,extension,reserved(1)
                                end
                                16'h0108: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // inverted zero

                                16'h000c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; // reserved?
                                16'h010c: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF}; //

                                16'h0010: data[31:0] <= {16'b1001_1111_1111_1111,16'hFFFF}; // manufacturer high byte inverted
                                16'h0110: data[31:0] <= {16'b0010_1111_1111_1111,16'hFFFF}; //
                                16'h0014: data[31:0] <= {16'b1001_1111_1111_1111,16'hFFFF}; // manufacturer low byte
                                16'h0114: data[31:0] <= {16'b0001_1111_1111_1111,16'hFFFF};

//                                16'h0028: autoConfigData <= 'b1001_1111_1111_1111; // autoboot rom vector (er_InitDiagVec)
//                                16'h0128: autoConfigData <= 'b1111_1111_1111_1111; // = ~0x6000
//                                16'h002c: autoConfigData <= 'b1111_1111_1111_1111;
//                                16'h012c: autoConfigData <= 'b1111_1111_1111_1111;

                                default: data[31:0] <= {16'b1111_1111_1111_1111,16'hFFFF};

                            endcase
                        end
                    end
					else begin
                    // addres is not used by FPGA code, so send TS to CPLD
                        NU_1 <= 1'b0;
                        RAM_state <= RAM_TS0;
					end
                end
            end
        RAM_TS0: begin
            NU_1 <= 1'b0;
            RAM_state <= RAM_TS1;
        end
        RAM_TS1: begin
            NU_1 <= 1'b0;
            RAM_state <= RAM_TS2;
        end
        RAM_TS2: begin
            NU_1 <= 1'b1;
            RAM_state <= RAM_idle;
        end
        RAM_read1b: begin
            if (m00_axi_arready) begin
                if((SIZ40[1:0]==2'b11)&& (GPIO_c[1]==1))  // BURST READ ENABLE
                    RAM_state <= RAM_read_burst1;
                else
                    RAM_state <= RAM_read2;
            end
        end
        RAM_read2: begin
data[31:0] <= m00_axi_rdata[31:0];
            m00_axi_arvalid <= 1'b0;
            if (m00_axi_rvalid) begin
                RAM_state <= RAM_read3;
                data[31:0] <= m00_axi_rdata[31:0];
            end
        end
        RAM_read3: begin
//            data[31:0] <= m00_axi_rdata[31:0]; //with cache or HP this is wrong
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_read31;
nTA <= 1'b0;
            end
        end
        RAM_read31: begin
            nTA <= 1'b0;
//            data[31:0] <= m00_axi_rdata[31:0]; //with cache or HP this is wrong
            if(ENcondition==1'b1) begin
//            if((nCLKEN==1'b1)&&(PCLK==1'b0)) begin
//            if(BCLK_falling==1) begin
                RAM_state <= RAM_idle;
        active_cycle <= 1'b0;
                nTA <= 1'bz;
             end
        end
        RAM_read_burst1: begin
            m00_axi_arvalid <= 1'b0; 
            if (m00_axi_rvalid) begin
                data_burst1[31:0]<=m00_axi_rdata[31:0];
                RAM_state <= RAM_read_burst2;
            end
        end
        RAM_read_burst2: begin
            data[31:0] <= data_burst1[31:0];
            if (m00_axi_rvalid) begin
                data_burst2[31:0]<=m00_axi_rdata[31:0];
                RAM_state <= RAM_read_burst3;
            end
        end
        RAM_read_burst3: begin
            data[31:0] <= data_burst1[31:0];
            if (m00_axi_rvalid) begin
                data_burst3[31:0]<=m00_axi_rdata[31:0];
                RAM_state <= RAM_read_burst4;
            end
        end
        RAM_read_burst4: begin
            data[31:0] <= data_burst1[31:0];
            m00_axi_arvalid <= 1'b0; 
            if (m00_axi_rvalid) begin
                data_burst4[31:0]<=m00_axi_rdata[31:0];
                RAM_state <= RAM_read4;
            end
        end
        RAM_read4: begin
            data[31:0] <= data_burst1[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_read41;
                nTA <= 1'b0;
            end
        end
        RAM_read41: begin
            nTA <= 1'b0;
            data[31:0] <= data_burst1[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_read42;
            end
        end
        RAM_read42: begin
            nTA <= 1'b0;
            data[31:0] <= data_burst2[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_read43;
            end
        end
        RAM_read43: begin
            nTA <= 1'b0;
            data[31:0] <= data_burst3[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_read44;
            end
       end
        RAM_read44: begin
            nTA <= 1'b0;
            data[31:0] <= data_burst4[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_idle;
        active_cycle <= 1'b0;
                nTA <= 1'bz;
                nTBI<=1'b1;
            end
        end
        RAM_write1b: begin
            m00_axi_wdata[31:0] <= D040[31:0];
            if (m00_axi_awready) begin
                if((SIZ40[1:0]==2'b11) && (GPIO_c[2]==1))  //line // BURST WRITE ENABLE
                    RAM_state <= RAM_write_burst1;
                else
                    RAM_state <= RAM_write2;
            end
        end
        RAM_write_burst1: begin
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_awvalid <= 0;
            if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst1b;
                nTA <= 1'b0;
            end
        end
        RAM_write_burst1b: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            if(ENcondition==1'b1) begin
                RAM_state <=RAM_write_burst1c;
            end
        end
        RAM_write_burst1c: begin
            nTA <= 1'b0;
            m00_axi_wvalid <= 1;
            m00_axi_wdata[31:0] <= D040[31:0];
            if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst1d;
            end
        end
        RAM_write_burst1d: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 0;
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst2;
            end
        end
        RAM_write_burst2: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 1;
            if (m00_axi_wready) begin
                RAM_state <= RAM_write_burst21;
            end
        end
        RAM_write_burst21: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 0;
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst3;
            end
        end
        RAM_write_burst3: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 1;
            if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst31;
            end
        end
        RAM_write_burst31: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 0;
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_write_burst4;
            end
        end
        RAM_write_burst4: begin
            nTA <= 1'b0;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 1;
            m00_axi_wlast <= 1;
            if (m00_axi_wready) begin
                RAM_state <=RAM_write_burst41;
                nTA <= 1'bz;
                nTBI<=1'b1;
            end
        end
        RAM_write_burst41: begin
//            nTA <= 1'b0;
//            nTA <= 1'bz;
//            nTBI<=1'b1;
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_wvalid <= 0;
            m00_axi_wlast <= 0;
//            if((nCLKEN==1'b0)&&(PCLK==1'b0)) begin
//            if(BCLK_rising==1) begin
                RAM_state <= RAM_idle;
        active_cycle <= 1'b0;
//                nTA <= 1'bz;
//                nTBI<=1'b1;
//            end
        end

        RAM_write2: begin
            m00_axi_wdata[31:0] <= D040[31:0];
            m00_axi_awvalid <= 0;
            m00_axi_wvalid <= 1;
            m00_axi_wlast <= 'h1;
            if (m00_axi_wready) begin
//                RAM_state <=RAM_write21;
                RAM_state <=RAM_write3;
            end
        end
        RAM_write21: begin
            m00_axi_wvalid <= 0;
            m00_axi_wlast <= 'h0;
            if (m00_axi_bvalid) begin
                RAM_state <=RAM_write3;
            end
        end
        RAM_write3: begin
            m00_axi_wvalid <= 0;
            m00_axi_wlast <= 'h0;
            if(ENcondition==1'b1) begin
                RAM_state <= RAM_write31;
                nTA <= 1'b0;
            end
        end
        RAM_write31: begin
            nTA <= 1'b0;
            if(ENcondition==1'b1) begin
//            if((nCLKEN==1'b1)&&(PCLK==1'b0)) begin
                RAM_state <= RAM_idle;
                active_cycle <= 1'b0;
                nTA <= 1'bz;
            end
        end

        default: begin
            RAM_state <= RAM_idle;
        active_cycle <= 1'b0;
        end
        endcase
    end
end

endmodule
