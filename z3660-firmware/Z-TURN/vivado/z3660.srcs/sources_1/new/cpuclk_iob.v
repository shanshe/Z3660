`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 01.07.2025 18:39:20
// Design Name: 
// Module Name: cpuclk_iob
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


module cpuclk_iob(
    input wire fast_clk,
    inout wire CPUCLK_pin,
    input wire CPUCLK_in,       // MMCM output
    inout wire CLK90_pin,
    input wire CLK90_in,        // MMCM output
    input wire enable_output,
    output reg cpuclk_detected,
    output reg clk90_detected
);

    reg CPUCLK_pin_out;
    wire CPUCLK_pin_in;
    wire CPUCLK_pin_t;  // Tri-state control: 0 = enabled output , 1 = tri-state (input)
    reg CLK90_pin_out;
    wire CLK90_pin_in;
    wire CLK90_pin_t;  // Tri-state control: 0 = enabled output , 1 = tri-state (input)

    assign CPUCLK_pin_t = ~enable_output;
    assign CLK90_pin_t = ~enable_output;

    IOBUF iobuf_cpuclk (
        .O(CPUCLK_pin_in),    // pin input
        .IO(CPUCLK_pin),      // physical pin inout
        .I(CPUCLK_pin_out),   // pin output
        .T(CPUCLK_pin_t)      // tri-state control (1 = Hi-Z)
    );

    IOBUF iobuf_clk90 (
        .O(CLK90_pin_in),    // pin input
        .IO(CLK90_pin),      // physical pin inout
        .I(CLK90_pin_out),   // pin output
        .T(CLK90_pin_t)      // tri-state control (1 = Hi-Z)
    );

    always @(*) begin
        if (enable_output) begin
            CPUCLK_pin_out = CPUCLK_in;
            CLK90_pin_out = CLK90_in;
        end else begin
            CPUCLK_pin_out = 1'bz;  // no drive
            CLK90_pin_out = 1'bz;  // no drive
        end
    end

    reg [7:0] count_cpuclk = 0;
    reg [7:0] count_clk90 = 0;
    reg prev_cpuclk_in = 0;
    reg prev_clk90_in = 0;

    always @(posedge fast_clk) begin
        if (enable_output) begin
            count_cpuclk <= 0;
            count_clk90 <= 0;
            cpuclk_detected <= 0;
            clk90_detected <= 0;
        end else begin
            prev_cpuclk_in <= CPUCLK_pin_in;
            prev_clk90_in <= CLK90_pin_in;
            if (~prev_cpuclk_in & CPUCLK_pin_in)
                count_cpuclk <= count_cpuclk + 1;
            if (~prev_clk90_in & CLK90_pin_in)
                count_clk90 <= count_clk90 + 1;

            if (count_cpuclk > 100)
                cpuclk_detected <= 1;
            if (count_clk90 > 100)
                clk90_detected <= 1;
        end
    end

endmodule