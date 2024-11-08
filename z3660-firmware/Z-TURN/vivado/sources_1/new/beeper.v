`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 14.09.2024 10:59:39
// Design Name: 
// Module Name: beeper
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


module beeper(
    input clk,
    input BP_IN,
    output BP_OUT
    );
reg bp_out;
assign BP_OUT = bp_out;
reg [31:0] counter=0;
localparam BP_STATE_IDLE = 0;
localparam BP_STATE_ON = 1;
reg BP_STATE=BP_STATE_IDLE;

always @(posedge clk) begin
    bp_out <= 1'b0;
    case (BP_STATE)
        BP_STATE_IDLE: begin
            counter = 0;
            if(BP_IN==1'b1) begin
                BP_STATE=BP_STATE_ON;
            end
        end
        BP_STATE_ON: begin
            counter = counter+1;
            if(counter<1000) // 50 us aprox
                bp_out <= 1'b1;
            else if (counter==184320) begin // 10 ms
                BP_STATE=BP_STATE_IDLE;
            end
        end
    endcase
end   
endmodule
