# 02 - SHIFT REGISTERS AND MEMORY

## USER (#restarts:1)
We are working on a an 8-bit accumulator-based RISC processor. 

Here is the ISA that we have defined:

**Immediate Data Manipulation Instructions**

| Instruction | Description                          | Opcode (4 bits) | Immediate (4 bits) |
|-------------|--------------------------------------|----------------|--------------------|
| ADDI        | Add immediate to Accumulator         | 1110           | 4-bit Immediate    |

**Instructions with Variable-Data Operands**

| Instruction | Description                                 | Opcode (3 bits) | Operand (5 bits) |
|-------------|---------------------------------------------|----------------|-----------------|
| LDA         | Load Accumulator with memory contents       | 000            | Memory Address  |
| STA         | Store Accumulator to memory                 | 001            | Memory Address  |
| ADD         | Add memory contents to Accumulator          | 010            | Memory Address  |
| SUB         | Subtract memory contents from Accumulator   | 011            | Memory Address  |
| AND         | AND memory contents with Accumulator        | 100            | Memory Address  |
| OR          | OR memory contents with Accumulator         | 101            | Memory Address  |
| XOR         | XOR memory contents with Accumulator        | 110            | Memory Address  |

**Control and Branching Instructions**

| Instruction  | Description                                     | Opcode (8 bits) | PC Behavior                                                                   |
|--------------|-------------------------------------------------|----------------|-------------------------------------------------------------------------------|
| JMP          | Jump to memory address                          | 11110000       | PC <- ACC (Load the PC with the address stored in the accumulator)            |
| JSR          | Jump to Subroutine (save address to ACC)        | 11110001       | ACC <- PC + 1, PC <- ACC (Save the next address in ACC, then jump to the address in ACC) |
| BEQ_FWD      | Branch if equal, forward (branch if ACC == 0)   | 11110010       | If ACC == 0, then PC <- PC + 2 (Jump 1 instruction forward if ACC is zero)    |
| BEQ_BWD      | Branch if equal, backward (branch if ACC == 0)  | 11110011       | If ACC == 0, then PC <- PC - 2 (Jump 1 instruction backward if ACC is zero)   |
| BNE_FWD      | Branch if not equal, forward (branch if ACC != 0)| 11110100       | If ACC != 0, then PC <- PC + 2 (Jump 1 instruction forward if ACC is non-zero)|
| BNE_BWD      | Branch if not equal, backward (branch if ACC != 0)| 11110101       | If ACC != 0, then PC <- PC - 2 (Jump 1 instruction backward if ACC is non-zero)|
| HLT          | Halt the processor until reset                  | 11111111       | Stop execution (PC does not change until a reset occurs)                      |

**Data Manipulation Instructions**

| Instruction | Description                                    | Opcode (8 bits) |
|-------------|------------------------------------------------|----------------|
| SHL         | Shift Accumulator left                         | 11110110       |
| SHR         | Shift Accumulator right                        | 11110111       |
| SHL4        | Shift Accumulator left by 4 bits               | 11111000       |
| ROL         | Rotate Accumulator left                        | 11111001       |
| ROR         | Rotate Accumulator right                       | 11111010       |
| INC         | Increment Accumulator                          | 11111011       |
| DEC         | Decrement Accumulator                          | 11111100       |
| CLR         | Clear (Zero) Accumulator                       | 11111101       |
| INV         | Invert (NOT) Accumulator                       | 11111110       |

Internally, it will use the following registers:

1. Program Counter (PC): 5 bits - Stores the address of the next instruction to be executed. The reduced size is sufficient for addressing the 32-byte register memory (2^5 = 32).
2. Accumulator (ACC): 8 bits - Stores the result of arithmetic and logical operations, as well as acts as a temporary storage for addresses during certain operations (e.g., JSR instruction).
3. Instruction Register (IR): 8 bits - Stores the current instruction being executed.

I'd like to now consider the implementation of some components needed to make this design. I want to use shift registers as the register primitive so that we can make a long scan chain for debugging purposes. Can you make a generic shift register in Verilog which could be used to instantiate each register so we can have a scan chain?

## ASSISTANT
Sure! Here's a generic shift register module in Verilog that can be parameterized with the number of bits and can be used to instantiate each register in your design. This shift register can be used in a scan chain by connecting the `scan_in` and `scan_out` ports accordingly.

```verilog
module shift_register #(
    parameter WIDTH = 8
)(
    input wire clk,
    input wire rst,
    input wire enable,
    input wire [WIDTH-1:0] data_in,
    output reg [WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out
);

    reg [WIDTH-1:0] internal_data;

    // Shift register operation
    always @(posedge clk) begin
        if (rst) begin
            internal_data <= {WIDTH{1'b0}};
        end else if (enable) begin
            internal_data <= data_in;
        end else if (scan_enable) begin
            internal_data <= {internal_data[WIDTH-2:0], scan_in};
        end
    end

    // Output assignment
    assign data_out = internal_data;
    assign scan_out = internal_data[WIDTH-1];

endmodule
```

This shift register module has the following ports:
- `clk`: Clock input.
- `rst`: Reset input. Resets the shift register when high.
- `enable`: Enable input. When high, the register will be loaded with `data_in`.
- `data_in`: Data input. An `WIDTH`-bit input to be loaded into the register when `enable` is high.
- `data_out`: Data output. The current value stored in the register.
- `scan_enable`: Scan enable input. When high, the register shifts its content, loading `scan_in` at the LSB position.
- `scan_in`: Scan input. The data to be loaded into the register's LSB when `scan_enable` is high.
- `scan_out`: Scan output. The data shifted out from the register's MSB.

You can instantiate this shift register module for each of your registers (PC, ACC, and IR) and connect them in a scan chain for debugging purposes.

## USER (#restarts:4)
I'm receiving an error that says that concurrent assignment to a non-net 'data_out' is not permitted on the line `assign scan_out = internal_data[WIDTH-1];`

## ASSISTANT
Apologies for the oversight. The issue is due to the use of `output reg` for the `data_out` port, which should be `output wire`. Here's the corrected shift register module:

```verilog
module shift_register #(
    parameter WIDTH = 8
)(
    input wire clk,
    input wire rst,
    input wire enable,
    input wire [WIDTH-1:0] data_in,
    output wire [WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out
);

    reg [WIDTH-1:0] internal_data;

    // Shift register operation
    always @(posedge clk) begin
        if (rst) begin
            internal_data <= {WIDTH{1'b0}};
        end else if (enable) begin
            internal_data <= data_in;
        end else if (scan_enable) begin
            internal_data <= {internal_data[WIDTH-2:0], scan_in};
        end
    end

    // Output assignment
    assign data_out = internal_data;
    assign scan_out = internal_data[WIDTH-1];

endmodule
```

The change made is replacing `output reg [WIDTH-1:0] data_out` with `output wire [WIDTH-1:0] data_out`. This should resolve the error you encountered.

## USER
Excellent, that is now working. We'll also need a large register bank for memory according to the defined ISA. Can you implement this in Verilog? I'd also like to be able to connect the memory bank to the scan chain.

## ASSISTANT
Sure! Here's a parameterized memory bank module in Verilog that can be instantiated for your processor design. This memory bank module is designed to work with the scan chain by using the previously implemented shift register module.

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 32
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output wire [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out
);

    // Generate an array of shift registers for the memory
    genvar i;
    generate
        for (i = 0; i < MEM_SIZE; i = i + 1) begin : memory
            shift_register #(
                .WIDTH(DATA_WIDTH)
            ) mem_cell (
                .clk(clk),
                .rst(rst),
                .enable(write_enable && (address == i)),
                .data_in(data_in),
                .data_out(data_out[i]),
                .scan_enable(scan_enable),
                .scan_in(i == 0 ? scan_in : memory[i-1].scan_out),
                .scan_out(scan_out)
            );
        end
    endgenerate

    // Read operation
    assign data_out = memory[address].data_out;

endmodule
```

This memory bank module has the following ports:
- `clk`: Clock input.
- `rst`: Reset input. Resets all memory cells when high.
- `address`: Address input. Selects the memory cell to read or write.
- `data_in`: Data input. Data to be written to the selected memory cell when `write_enable` is high.
- `write_enable`: Write enable input. When high, the selected memory cell will be written with `data_in`.
- `data_out`: Data output. Outputs the data stored in the selected memory cell.
- `scan_enable`: Scan enable input. When high, the memory cells are shifted for scan chain operation.
- `scan_in`: Scan input. The data to be loaded into the first memory cell's LSB when `scan_enable` is high.
- `scan_out`: Scan output. The data shifted out from the last memory cell's MSB.

You can instantiate this memory bank module for your processor's memory and connect it to the scan chain. Just be aware that the size of the scan chain will increase significantly when including the memory bank, which may have an impact on the time it takes to shift data through the chain for debugging.

## USER
I think this design has some bugs. It looks like the use of data_out in the generate isn't correct, the synthesis tool
 says there are width mismatches. It's also saying that it can't resolve the hierarchical name for the item 'memory'.

## ASSISTANT
Apologies for the confusion. I made a mistake in the previous code. Here's the corrected memory bank module:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 32
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output wire [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out
);

    // Generate an array of shift registers for the memory
    wire [DATA_WIDTH-1:0] mem_data_out [0:MEM_SIZE-1];
    wire mem_scan_out [0:MEM_SIZE-1];

    genvar i;
    generate
        for (i = 0; i < MEM_SIZE; i = i + 1) begin : memory
            shift_register #(
                .WIDTH(DATA_WIDTH)
            ) mem_cell (
                .clk(clk),
                .rst(rst),
                .enable(write_enable && (address == i)),
                .data_in(data_in),
                .data_out(mem_data_out[i]),
                .scan_enable(scan_enable),
                .scan_in(i == 0 ? scan_in : mem_scan_out[i-1]),
                .scan_out(mem_scan_out[i])
            );
        end
    endgenerate

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
    end

    // Scan chain output
    assign scan_out = mem_scan_out[MEM_SIZE-1];

endmodule
```

In this corrected code, I've made the following changes:
1. Declared an array `mem_data_out` of wires with the size `[DATA_WIDTH-1:0]` for each memory cell's data output.
2. Declared an array `mem_scan_out` of wires for each memory cell's scan output.
3. Updated the `memory` generate block to use `mem_data_out[i]` and `mem_scan_out[i]` for connecting to the shift register module.
4. Replaced the `assign data_out` line with an always block that selects the data output based on the address.
5. Assigned the `scan_out` output to `mem_scan_out[MEM_SIZE-1]`.

This should resolve the width mismatch issues and the inability to resolve the hierarchical name for the `memory` item.

## USER
I'm getting just one error this time - it says that procedural assignment to a non-register data_out is not permitted on line "data_out = {DATA_WIDTH{1'b0}};"

## ASSISTANT
Apologies for the oversight. You should declare `data_out` as a `reg` instead of a `wire` to use it within the always block. Here's the corrected memory bank module:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 32
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out
);

    // Generate an array of shift registers for the memory
    wire [DATA_WIDTH-1:0] mem_data_out [0:MEM_SIZE-1];
    wire mem_scan_out [0:MEM_SIZE-1];

    genvar i;
    generate
        for (i = 0; i < MEM_SIZE; i = i + 1) begin : memory
            shift_register #(
                .WIDTH(DATA_WIDTH)
            ) mem_cell (
                .clk(clk),
                .rst(rst),
                .enable(write_enable && (address == i)),
                .data_in(data_in),
                .data_out(mem_data_out[i]),
                .scan_enable(scan_enable),
                .scan_in(i == 0 ? scan_in : mem_scan_out[i-1]),
                .scan_out(mem_scan_out[i])
            );
        end
    endgenerate

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
    end

    // Scan chain output
    assign scan_out = mem_scan_out[MEM_SIZE-1];

endmodule
```

The change made is replacing `output wire [DATA_WIDTH-1:0] data_out` with `output reg [DATA_WIDTH-1:0] data_out`. This should resolve the error you encountered.