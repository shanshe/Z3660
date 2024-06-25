`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07.12.2023 09:28:34
// Design Name: 
// Module Name: clock_gen
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


module clock_gen(
    input in_clk,
    input clk_select,
    input reset,
    output PCLK_clk,
    output nCLKEN_clk,
    output BCLK_clk,
    output CLK90_clk,
    output CPUCLK_clk,
    output PCLK_reg,
    output nCLKEN_reg
    );

reg [2:0] counter=0;
reg PCLK;
reg nCLKEN=0;
reg BCLK=1;
reg CPUCLK=0;
reg CLK90=1;

assign PCLK_reg = PCLK;
assign nCLKEN_reg = nCLKEN;
/*
ODDR #(
   .DDR_CLK_EDGE("OPPOSITE_EDGE"), // or "SAME_EDGE"
   .INIT(1'b0),
   .SRTYPE("SYNC") // or "ASYNC"
) ODDR_PCLK (
   .Q(PCLK_clk),
   .C(PCLK),
   .CE(1),.D1(1),.D2(0),.R(0),.S(0)
);

ODDR #(
   .DDR_CLK_EDGE("OPPOSITE_EDGE"), // or "SAME_EDGE"
   .INIT(1'b0),
   .SRTYPE("SYNC") // or "ASYNC"
) ODDR_nCLKEN (
   .Q(nCLKEN_clk),
   .C(nCLKEN),
   .CE(1),.D1(1),.D2(0),.R(0),.S(0)
);

ODDR #(
   .DDR_CLK_EDGE("OPPOSITE_EDGE"), // or "SAME_EDGE"
   .INIT(1'b0),
   .SRTYPE("SYNC") // or "ASYNC"
) ODDR_CLK90 (
   .Q(CLK90_clk),
   .C(CLK90),
   .CE(1),.D1(1),.D2(0),.R(0),.S(0)
);

ODDR #(
   .DDR_CLK_EDGE("OPPOSITE_EDGE"), // or "SAME_EDGE"
   .INIT(1'b0),
   .SRTYPE("SYNC") // or "ASYNC"
) ODDR_CPUCLK (
   .Q(CPUCLK_clk),
   .C(CPUCLK),
   .CE(1),.D1(1),.D2(0),.R(0),.S(0)
);

ODDR #(
   .DDR_CLK_EDGE("OPPOSITE_EDGE"), // or "SAME_EDGE"
   .INIT(1'b0),
   .SRTYPE("SYNC") // or "ASYNC"
) ODDR_BCLK (
   .Q(BCLK_clk),
   .C(BCLK),
   .CE(1),.D1(1),.D2(0),.R(0),.S(0)
);
*/
assign PCLK_clk=PCLK;
assign nCLKEN_clk=nCLKEN;
assign CLK90_clk=CLK90;
assign CPUCLK_clk=CPUCLK;
assign BCLK_clk=BCLK;

always @(posedge in_clk) begin
   if(reset==0) begin
      counter=0;
      PCLK=1; nCLKEN=0; BCLK=1; CPUCLK=0; CLK90=1;
   end else begin
      counter = counter +1;
      if(clk_select==0) begin
         case(counter[2:0])
            3'b000: begin
               PCLK=1; nCLKEN=0; BCLK=1; CPUCLK=0; CLK90=1;
            end
            3'b001: begin
               PCLK=0; nCLKEN=1; BCLK=1; CPUCLK=0; CLK90=0;
            end
            3'b010: begin
               PCLK=1; nCLKEN=1; BCLK=0; CPUCLK=1; CLK90=0;
            end
            3'b011: begin
               PCLK=0; nCLKEN=0; BCLK=0; CPUCLK=1; CLK90=1;
            end
            3'b100: begin
               PCLK=1; nCLKEN=0; BCLK=1; CPUCLK=0; CLK90=1;
            end
            3'b101: begin
               PCLK=0; nCLKEN=1; BCLK=1; CPUCLK=0; CLK90=0;
            end
            3'b110: begin
               PCLK=1; nCLKEN=1; BCLK=0; CPUCLK=1; CLK90=0;
            end
            3'b111: begin
               PCLK=0; nCLKEN=0; BCLK=0; CPUCLK=1; CLK90=1;
            end
         endcase
      end else begin
         case(counter[2:0])
            3'b000: begin
               PCLK=1; nCLKEN=0; BCLK=1; CPUCLK=0; CLK90=1;
            end
            3'b001: begin
               PCLK=0; nCLKEN=1; BCLK=1; CPUCLK=0; CLK90=1;
            end
            3'b010: begin
               PCLK=1; nCLKEN=1; BCLK=1; CPUCLK=0; CLK90=0;
            end
            3'b011: begin
               PCLK=0; nCLKEN=0; BCLK=1; CPUCLK=0; CLK90=0;
            end
            3'b100: begin
               PCLK=1; nCLKEN=0; BCLK=0; CPUCLK=1; CLK90=0;
            end
            3'b101: begin
               PCLK=0; nCLKEN=1; BCLK=0; CPUCLK=1; CLK90=0;
            end
            3'b110: begin
               PCLK=1; nCLKEN=1; BCLK=0; CPUCLK=1; CLK90=1;
            end
            3'b111: begin
               PCLK=0; nCLKEN=0; BCLK=0; CPUCLK=1; CLK90=1;
            end
         endcase
      end
   end
end 

endmodule
