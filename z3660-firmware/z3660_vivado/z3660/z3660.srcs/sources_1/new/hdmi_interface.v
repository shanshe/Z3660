`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 30.05.2022 15:51:42
// Design Name: 
// Module Name: hdmi_interface
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

module hdmi_interface(
    input vid_de,
    input [23:0] vid_data,
    input vid_hs,
    input vid_vs,
    input vid_clk,
    output hdmi_de,
    output [15:0] hdmi_data,
    output hdmi_hs,
    output hdmi_vs,
    output hdmi_clk
    );
// color 24 bit -> color 16 bit 565
    assign    hdmi_data   =   {vid_data[23:19],vid_data[15:10],vid_data[7:3]};
    assign    hdmi_clk    =   vid_clk;
    assign    hdmi_de     =   vid_de;
    assign    hdmi_hs     =   ~vid_hs;
    assign    hdmi_vs     =   ~vid_vs;  

endmodule
