// SPDX-FileCopyrightText: 2020 Efabless Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// SPDX-License-Identifier: Apache-2.0

`default_nettype none
/*
 *-------------------------------------------------------------
 *
 * user_proj_example
 *
 * This is an example of a (trivially simple) user project,
 * showing how the user project can connect to the logic
 * analyzer, the wishbone bus, and the I/O pads.
 *
 * This project generates an integer count, which is output
 * on the user area GPIO pads (digital output only).  The
 * wishbone connection allows the project to be controlled
 * (start and stop) from the management SoC program.
 *
 * See the testbenches in directory "mprj_counter" for the
 * example programs that drive this user project.  The three
 * testbenches are "io_ports", "la_test1", and "la_test2".
 *
 *-------------------------------------------------------------
 */

module user_proj_example #(
    parameter BITS = 32
)(
`ifdef USE_POWER_PINS
    inout vccd1,	// User area 1 1.8V supply
    inout vssd1,	// User area 1 digital ground
`endif

    // Wishbone Slave ports (WB MI A)
    input wb_clk_i,
    input wb_rst_i,
    input wbs_stb_i,
    input wbs_cyc_i,
    input wbs_we_i,
    input [3:0] wbs_sel_i,
    input [31:0] wbs_dat_i,
    input [31:0] wbs_adr_i,
    output reg wbs_ack_o,
    output [31:0] wbs_dat_o,

    // Logic Analyzer Signals
    input  [127:0] la_data_in,
    output [127:0] la_data_out,
    input  [127:0] la_oenb,

    // IOs
    input  [15:0] io_in,
    output [15:0] io_out,
    output [15:0] io_oeb,

    // IRQ
    output [2:0] irq
);
    wire clk;
    wire rst;

    wire [15:0] io_in;
    wire [15:0] io_out;
    wire [15:0] io_oeb;

    wire [7:0] wbs_adr_int;

    wire [15:0] rdata; 
    wire [15:0] wdata;
    wire [15:0] count;

    wire valid;
    wire [3:0] wstrb;
    wire [31:0] la_write;

    assign valid = wbs_cyc_i && wbs_stb_i;

    assign wstrb = wbs_sel_i & {4{wbs_we_i}};
    //assign wbs_dat_o = rdata;
    //assign wdata = wbs_dat_i[15:0];

    assign io_oeb = {8'hff, 8'h00}; //top 8 bits are inputs, bottom 8 bits are outputs

    reg[7:0] io_in_reg_pipe;
    reg[7:0] io_in_reg;
    always @(posedge wb_clk_i) begin
        io_in_reg_pipe <= io_in[15:8];
        io_in_reg <= io_in_reg_pipe;
    end

    // IO
    assign io_out = count;
    assign io_oeb = {(15){rst}};

    // IRQ
    assign irq[2:1] = 2'b00;	// Unused

    // LA
    // assign la_data_out = {{(127-BITS){1'b0}}, count};
    // // Assuming LA probes [63:32] are for controlling the count register  
    // assign la_write = ~la_oenb[63:32] & ~{BITS{valid}};
    // // Assuming LA probes [65:64] are for controlling the count clk & reset  
    // assign clk = (~la_oenb[64]) ? la_data_in[64]: wb_clk_i;
    // assign rst = (~la_oenb[65]) ? la_data_in[65]: wb_rst_i;


    wire halt_out;
    reg [31:0] scan_io;
    reg [31:0] scan_wbs;
    reg scan_go;
    reg scan_enable;
    reg [4:0] scan_cnt;
    reg scan_in_progress;
    reg scan_done_strobe;

    wire scan_in = scan_io[31];
    wire scan_out;

    reg proc_go;

    assign clk = wb_clk_i;

    //initial $display("Hello world!");

    always@(posedge clk) begin
        wbs_ack_o <= 0;

        if(wb_rst_i) begin
            scan_wbs <= 0;
            proc_go <= 0;
            scan_go <= 0;
        end else begin
            if(scan_done_strobe) begin
                scan_go <= 0;
                scan_wbs <= scan_io;
                //$display("scan_io unloading, = %b", scan_io);
            end
            if(valid) begin
                wbs_ack_o <= 1;
                if(wbs_adr_i[7:0] == 8'h00) begin

                    
                    if(wstrb[0])
                        //$display("setting scan_wbs[7:0] to %h", wbs_dat_i[7:0]);
                        scan_wbs[7:0] <= wbs_dat_i[7:0];
                    if(wstrb[1])
                        //$display("setting scan_wbs[15:8] to %h", wbs_dat_i[15:8]);
                        scan_wbs[15:8] <= wbs_dat_i[15:8];
                    if(wstrb[2])
                        //$display("setting scan_wbs[23:16] to %h", wbs_dat_i[23:16]);
                        scan_wbs[23:16] <= wbs_dat_i[23:16];
                    if(wstrb[3])
                        //$display("setting scan_wbs[31:24] to %h", wbs_dat_i[31:24]);
                        scan_wbs[31:24] <= wbs_dat_i[31:24];

                end else if(wbs_adr_i[7:0] == 8'h04) begin
                    if(wstrb[0]) begin
                        if(wbs_dat_i[0] == 1'b1)
                            scan_go <= 1;
                        
                        proc_go <= wbs_dat_i[1];
                    end
                end
            end
        end
    end

    reg scan_first_cycle = 0;

    always@(posedge clk) begin
        scan_done_strobe <= 0;
        scan_enable <= 0;
        if(wb_rst_i) begin
            scan_in_progress <= 0;
            scan_first_cycle <= 0;
            scan_cnt <= 0;
            scan_io <= 0;
        end else begin
            if(scan_go==1 && 
                scan_in_progress == 0 &&
                proc_go == 0 &&
                scan_first_cycle == 0 && 
                scan_done_strobe==0) begin
                
                $display("scan_io loading, = %h", scan_wbs);
                scan_io <= scan_wbs;
                scan_cnt <= 5'd31;
                scan_first_cycle <= 1;
            end else if(scan_first_cycle == 1) begin
                scan_in_progress <= 1;
                scan_first_cycle <= 0;
                scan_enable <= 1;
            end else if(scan_in_progress == 1) begin
                scan_io <= (scan_io << 1) | scan_out;
                if(scan_cnt != 0) begin
                    scan_enable <= 1;
                    scan_cnt <= scan_cnt - 1;
                end else begin
                    scan_in_progress <= 0;
                    scan_done_strobe <= 1;
                end
            end else if(scan_done_strobe == 1) begin
                //scan_enable <= 1;
                ; //nothing to do here
            end
        end
    end

    accumulator_microcontroller #(
        .MEM_SIZE(256)
    ) qtcore_C1 (
        .clk(wb_clk_i),
        .rst(wb_rst_i),
        .scan_enable(scan_enable),
        .scan_in(scan_in),
        .scan_out(scan_out),
        .proc_en(proc_go),
        .halt(halt_out),
        .IO_in(io_in[15:8]),    
        .IO_out(io_out[7:0]),   
        .INT_out(irq[0])      
    );
    
    assign wbs_dat_o = (wbs_adr_i[7:0] == 8'h00) ? {scan_wbs} : {29'h0, halt_out, proc_go, scan_enable};

endmodule

`default_nettype wire
