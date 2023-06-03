# 08 - DATAPATH COMPONENTS

## USER
Here are the definition of Verilog components for "qtcore-A1", an 8-bit accumulator-based RISC processor.

Internally, qtcore-A1 will use the following registers:

1. Program Counter (PC): 5 bits - Stores the address of the next instruction to be executed. The reduced size is sufficient for addressing the 32-byte register memory (2^5 = 32).
2. Accumulator (ACC): 8 bits - Stores the result of arithmetic and logical operations, as well as acts as a temporary storage for addresses during certain operations (e.g., JSR instruction).
3. Instruction Register (IR): 8 bits - Stores the current instruction being executed.

Registers PC, ACC, and IR are instantiated using this shift register definition:
```
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
```

Memory is instantiated using this definition:
```
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
```

The ALU is instantiated using this definition:
```
module alu (
    input wire [7:0] A,
    input wire [7:0] B,
    input wire [3:0] opcode,
    output reg [7:0] Y
);
```

The control unit is instantiated using this definition:
```
module control_unit (
input wire clk, // Clock input
input wire rst, // Reset input
input wire processor_enable, // Processor enable signal
output reg processor_halted, // Processor halted signal
input wire [7:0] instruction, // Input from the Instruction Register (IR)
input wire ZF, // Zero Flag input, true when ACC is zero

output reg PC_write_enable, // Enables writing to the PC
output reg [1:0] PC_mux_select, // Selects the input for the PC multiplexer
// 00: PC + 1 (FETCH cycle, BEQ_FWD, BNE_FWD)
// 01: ACC (JMP, JSR)
// 10: PC - 3 (BEQ_BWD, BNE_BWD)

output reg ACC_write_enable, // Enables writing to the ACC
output reg [1:0] ACC_mux_select, // Selects the input for the ACC multiplexer
// 00: ALU output
// 01: Memory contents (LDA, LDAR)
// 10: PC (JSR)

output reg IR_load_enable, // Enables loading new instruction into IR from memory

output wire [3:0] ALU_opcode, // Control signal specifying the ALU operation
output reg ALU_inputB_mux_select, // Selects input B for the ALU multiplexer
// 0: Memory contents (ADD, SUB, AND, OR, XOR)
// 1: Immediate (ADDI)

output reg Memory_write_enable, // Enables writing to memory (STA)
output reg [1:0] Memory_address_mux_select, // Selects input for memory address multiplexer
// 00: IR[4:0] (LDA, STA, ADD, SUB, AND, OR, XOR)
// 01: ACC (LDAR)
// 10: PC (Instruction fetching)

// Scan chain signals
input wire scan_enable, // Scan chain enable signal
input wire scan_in, // Scan chain input
output wire scan_out // Scan chain output
);
```

The top level module is defined as follows:
```
module accumulator_microcontroller
(
    input wire clk,
    input wire rst,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire proc_en,
    output wire halt
);
```

There is an embedded scan chain which connects all the registers in each component together.
The ordering is as as follows:
1. Control Unit
2. Program Counter (PC)
3. Accumulator (ACC)
4. Instruction Register (IR)
5. Memory Bank

Let's finish off the top-level module. It will contain the following components: the ALU, the memory, the PC, ACC, and IR, as well as the control unit. 
Each component will need to be wired together. This will require declaring some wires. What wires will be needed?

## ASSISTANT
To finish off the top-level module, you will need to declare the following wires:

1. Wires to connect the ALU inputs and output:
   - `wire [7:0] alu_A;`
   - `wire [7:0] alu_B;`
   - `wire [7:0] alu_Y;`

2. Wires to connect the memory inputs, address, and output:
   - `wire [4:0] memory_address;`
   - `wire [7:0] memory_data_in;`
   - `wire [7:0] memory_data_out;`

3. Wires to connect the Program Counter (PC) inputs and output:
   - `wire [4:0] pc_data_in;`
   - `wire [4:0] pc_data_out;`

4. Wires to connect the Accumulator (ACC) inputs and output:
   - `wire [7:0] acc_data_in;`
   - `wire [7:0] acc_data_out;`

5. Wires to connect the Instruction Register (IR) inputs and output:
   - `wire [7:0] ir_data_in;`
   - `wire [7:0] ir_data_out;`

6. Wires to connect the control unit signals:
   - `wire cu_PC_write_enable;`
   - `wire [1:0] cu_PC_mux_select;`
   - `wire cu_ACC_write_enable;`
   - `wire [1:0] cu_ACC_mux_select;`
   - `wire cu_IR_load_enable;`
   - `wire [3:0] cu_ALU_opcode;`
   - `wire cu_ALU_inputB_mux_select;`
   - `wire cu_Memory_write_enable;`
   - `wire [1:0] cu_Memory_address_mux_select;`
   - `wire cu_ZF;` // Zero Flag from ALU

7. Wires for the scan chain connections:
   - `wire control_unit_scan_out;`
   - `wire pc_scan_out;`
   - `wire acc_scan_out;`
   - `wire ir_scan_out;`
   - `wire memory_scan_out;`

With these wires declared, you can now connect all the components together within the `accumulator_microcontroller` module. Don't forget to instantiate each module (ALU, memory bank, PC, ACC, IR, and control unit) with the appropriate inputs, outputs, and parameters.

## USER
Can you now write Verilog code to instantiate the ALU and any multiplexers that it needs? Use the wire definitions you just provided to provide the necessary I/O, as well as suitable top-level I/O from the module definition.

## ASSISTANT
Certainly! Here's the Verilog code to instantiate the ALU, along with the required multiplexers:

```verilog
module accumulator_microcontroller
(
    input wire clk,
    input wire rst,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire proc_en,
    output wire halt
);
    // Declare wires for ALU, memory, PC, ACC, and IR connections
    wire [7:0] alu_A;
    wire [7:0] alu_B;
    wire [7:0] alu_Y;
    wire [4:0] memory_address;
    wire [7:0] memory_data_in;
    wire [7:0] memory_data_out;
    wire [4:0] pc_data_in;
    wire [4:0] pc_data_out;
    wire [7:0] acc_data_in;
    wire [7:0] acc_data_out;
    wire [7:0] ir_data_in;
    wire [7:0] ir_data_out;
    wire cu_PC_write_enable;
    wire [1:0] cu_PC_mux_select;
    wire cu_ACC_write_enable;
    wire [1:0] cu_ACC_mux_select;
    wire cu_IR_load_enable;
    wire [3:0] cu_ALU_opcode;
    wire cu_ALU_inputB_mux_select;
    wire cu_Memory_write_enable;
    wire [1:0] cu_Memory_address_mux_select;
    wire cu_ZF;
    wire control_unit_scan_out;
    wire pc_scan_out;
    wire acc_scan_out;
    wire ir_scan_out;
    wire memory_scan_out;

    // Instantiate ALU
    alu ALU_inst (
        .A(alu_A),
        .B(alu_B),
        .opcode(cu_ALU_opcode),
        .Y(alu_Y)
    );

    // Instantiate ALU Input A multiplexer (connected to ACC)
    assign alu_A = acc_data_out;

    // Instantiate ALU Input B multiplexer
    wire [7:0] immediate;
    assign immediate = {1'b0, ir_data_out[2:0]}; // Immediate value is IR[2:0] with zero-extended MSB

    assign alu_B = cu_ALU_inputB_mux_select ? immediate : memory_data_out;

    // Remaining module instantiations and connections go here
endmodule
```

This code instantiates the ALU and connects its inputs and output to the appropriate wires. The ALU input A is directly connected to the accumulator output. The ALU input B is connected to a multiplexer that selects between the memory_data_out and the immediate value, based on the cu_ALU_inputB_mux_select signal. The immediate value is generated by zero-extending the least significant 3 bits of the instruction register's output.

## USER
For the immediate value, it should be the bottom 4 bits of IR extended by 4 zeros, can you update the definition of the immediate wire?

## ASSISTANT
Of course, I'll update the definition of the immediate wire as per your request. Here's the updated Verilog code for the ALU instantiation and the immediate value:

```verilog
module accumulator_microcontroller
(
    input wire clk,
    input wire rst,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire proc_en,
    output wire halt
);
    // Declare wires for ALU, memory, PC, ACC, and IR connections
    wire [7:0] alu_A;
    wire [7:0] alu_B;
    wire [7:0] alu_Y;
    wire [4:0] memory_address;
    wire [7:0] memory_data_in;
    wire [7:0] memory_data_out;
    wire [4:0] pc_data_in;
    wire [4:0] pc_data_out;
    wire [7:0] acc_data_in;
    wire [7:0] acc_data_out;
    wire [7:0] ir_data_in;
    wire [7:0] ir_data_out;
    wire cu_PC_write_enable;
    wire [1:0] cu_PC_mux_select;
    wire cu_ACC_write_enable;
    wire [1:0] cu_ACC_mux_select;
    wire cu_IR_load_enable;
    wire [3:0] cu_ALU_opcode;
    wire cu_ALU_inputB_mux_select;
    wire cu_Memory_write_enable;
    wire [1:0] cu_Memory_address_mux_select;
    wire cu_ZF;
    wire control_unit_scan_out;
    wire pc_scan_out;
    wire acc_scan_out;
    wire ir_scan_out;
    wire memory_scan_out;

    // Instantiate ALU
    alu ALU_inst (
        .A(alu_A),
        .B(alu_B),
        .opcode(cu_ALU_opcode),
        .Y(alu_Y)
    );

    // Instantiate ALU Input A multiplexer (connected to ACC)
    assign alu_A = acc_data_out;

    // Instantiate ALU Input B multiplexer
    wire [7:0] immediate;
    assign immediate = {4'b0000, ir_data_out[3:0]}; // Immediate value is IR[3:0] with four zero-extended MSBs

    assign alu_B = cu_ALU_inputB_mux_select ? immediate : memory_data_out;

    // Remaining module instantiations and connections go here
endmodule
```

In this updated code, the immediate value is generated by taking the least significant 4 bits of the instruction register's output (IR[3:0]) and extending it with 4 zeros in the most significant bits.

## USER
Looks good. Can you now instantiate the PC, IR, and ACC and their multiplexers
? Assume it will be put after the `// Remaining module instantiations and connections go here`, you don't need to rewrite the rest of the file.

## ASSISTANT
Sure, here's the code to instantiate the PC, IR, and ACC along with their respective multiplexers:

```verilog
// Instantiate PC
shift_register #(
    .WIDTH(5)
) PC_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_PC_write_enable),
    .data_in(pc_data_in),
    .data_out(pc_data_out),
    .scan_enable(scan_enable),
    .scan_in(scan_in),
    .scan_out(pc_scan_out)
);

// Instantiate PC multiplexer
wire [4:0] pc_plus_one = pc_data_out + 5'b1;
wire [4:0] pc_minus_three = pc_data_out - 5'b11;
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        default: pc_data_in = 5'b0;
    endcase
end

// Instantiate IR
shift_register #(
    .WIDTH(8)
) IR_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_IR_load_enable),
    .data_in(memory_data_out),
    .data_out(ir_data_out),
    .scan_enable(scan_enable),
    .scan_in(pc_scan_out),
    .scan_out(ir_scan_out)
);

// Instantiate ACC
shift_register #(
    .WIDTH(8)
) ACC_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_ACC_write_enable),
    .data_in(acc_data_in),
    .data_out(acc_data_out),
    .scan_enable(scan_enable),
    .scan_in(ir_scan_out),
    .scan_out(acc_scan_out)
);

// Instantiate ACC multiplexer
always @(*) begin
    case(cu_ACC_mux_select)
        2'b00: acc_data_in = alu_Y;
        2'b01: acc_data_in = memory_data_out;
        2'b10: acc_data_in = pc_data_out[7:0];
        default: acc_data_in = 8'b0;
    endcase
end
```

This code instantiates the PC, IR, and ACC registers using the `shift_register` module with the appropriate width. It also sets up the multiplexers for each register, controlling the input data based on the control signals coming from the control unit.

For the PC multiplexer, it takes care of the cases for PC + 1, ACC[4:0], and PC - 3.

For the ACC multiplexer, it considers the cases for ALU output, memory_data_out, and pc_data_out[7:0].

## USER
If you are going to use always blocks to set pc_data_in and acc_data_in they can't be the wire type - can you fix their definitions?

## ASSISTANT
You are right, I apologize for the oversight. To fix the issue, we need to change the `pc_data_in` and `acc_data_in` from wire to reg type. Here's the updated instantiation of PC, IR, and ACC with the corrected definitions:

```verilog
// Declare wires and regs for PC, ACC, and IR connections
// ...
reg [4:0] pc_data_in;
reg [7:0] acc_data_in;
// ...

// Instantiate PC
shift_register #(
    .WIDTH(5)
) PC_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_PC_write_enable),
    .data_in(pc_data_in),
    .data_out(pc_data_out),
    .scan_enable(scan_enable),
    .scan_in(scan_in),
    .scan_out(pc_scan_out)
);

// Instantiate PC multiplexer
wire [4:0] pc_plus_one = pc_data_out + 5'b1;
wire [4:0] pc_minus_three = pc_data_out - 5'b11;
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        default: pc_data_in = 5'b0;
    endcase
end

// Instantiate IR
shift_register #(
    .WIDTH(8)
) IR_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_IR_load_enable),
    .data_in(memory_data_out),
    .data_out(ir_data_out),
    .scan_enable(scan_enable),
    .scan_in(pc_scan_out),
    .scan_out(ir_scan_out)
);

// Instantiate ACC
shift_register #(
    .WIDTH(8)
) ACC_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_ACC_write_enable),
    .data_in(acc_data_in),
    .data_out(acc_data_out),
    .scan_enable(scan_enable),
    .scan_in(ir_scan_out),
    .scan_out(acc_scan_out)
);

// Instantiate ACC multiplexer
always @(*) begin
    case(cu_ACC_mux_select)
        2'b00: acc_data_in = alu_Y;
        2'b01: acc_data_in = memory_data_out;
        2'b10: acc_data_in = pc_data_out[7:0];
        default: acc_data_in = 8'b0;
    endcase
end
```

Now, `pc_data_in` and `acc_data_in` are defined as reg types and can be properly assigned in the always blocks.

## USER
If your ACC multiplexer, it assumes that pc_data_out is 8 bits, but that's not right - can you rewrite that multiplexer with the correct width for pc_data_out and extend it with zeros to make 8 bits?

## ASSISTANT
You are correct, the `pc_data_out` has a width of 5 bits, not 8 bits. I apologize for the mistake. Here's the updated ACC multiplexer that correctly handles the width of `pc_data_out` by extending it with zeros to make it 8 bits:

```verilog
// Instantiate ACC multiplexer
always @(*) begin
    case(cu_ACC_mux_select)
        2'b00: acc_data_in = alu_Y;
        2'b01: acc_data_in = memory_data_out;
        2'b10: acc_data_in = {3'b000, pc_data_out};
        default: acc_data_in = 8'b0;
    endcase
end
```

In the updated code, the PC value is extended with 3 zero bits in the most significant positions (`{3'b000, pc_data_out}`) to form an 8-bit value for the ACC input.

## USER
The PC's scan_in is coming from the wrong wire, can you fix that?

## ASSISTANT
I apologize for the oversight. Here's the corrected instantiation of the PC with the proper `scan_in` connection:

```verilog
// Instantiate PC
shift_register #(
    .WIDTH(5)
) PC_inst (
    .clk(clk),
    .rst(rst),
    .enable(cu_PC_write_enable),
    .data_in(pc_data_in),
    .data_out(pc_data_out),
    .scan_enable(scan_enable),
    .scan_in(control_unit_scan_out),
    .scan_out(pc_scan_out)
);
```

Now, the `scan_in` of the PC is correctly connected to the `control_unit_scan_out` wire.

## USER
Looking good! Can you now instantiate the memory unit and the multiplexers that it requires?

## ASSISTANT
Certainly! Here's the code to instantiate the memory unit and its required multiplexers:

```verilog
// Instantiate Memory Bank
memory_bank #(
    .ADDR_WIDTH(5),
    .DATA_WIDTH(8),
    .MEM_SIZE(32)
) memory_inst (
    .clk(clk),
    .rst(rst),
    .address(memory_address),
    .data_in(acc_data_out),
    .write_enable(cu_Memory_write_enable),
    .data_out(memory_data_out),
    .scan_enable(scan_enable),
    .scan_in(acc_scan_out),
    .scan_out(memory_scan_out)
);

// Instantiate Memory Address Multiplexer
always @(*) begin
    case(cu_Memory_address_mux_select)
        2'b00: memory_address = ir_data_out[4:0];
        2'b01: memory_address = acc_data_out[4:0];
        2'b10: memory_address = pc_data_out;
        default: memory_address = 5'b0;
    endcase
end
```

This code instantiates the memory unit using the `memory_bank` module with the appropriate parameters for address width, data width, and memory size. It also sets up the memory address multiplexer, controlling the address input based on the control signals coming from the control unit.

For the memory address multiplexer, it takes care of the cases for IR[4:0], ACC[4:0], and PC.

## USER
If you are going to use an always block to set memory_address it can't be the wire type - can you fix the
 definitions?

## ASSISTANT
You are correct, and I apologize for the mistake. To fix the issue, we need to change the `memory_address` from a wire to a reg type. Here's the updated instantiation of the memory unit with the corrected definition:

```verilog
// Declare wires and regs for Memory connections
// ...
reg [4:0] memory_address;
// ...

// Instantiate Memory Bank
memory_bank #(
    .ADDR_WIDTH(5),
    .DATA_WIDTH(8),
    .MEM_SIZE(32)
) memory_inst (
    .clk(clk),
    .rst(rst),
    .address(memory_address),
    .data_in(acc_data_out),
    .write_enable(cu_Memory_write_enable),
    .data_out(memory_data_out),
    .scan_enable(scan_enable),
    .scan_in(acc_scan_out),
    .scan_out(memory_scan_out)
);

// Instantiate Memory Address Multiplexer
always @(*) begin
    case(cu_Memory_address_mux_select)
        2'b00: memory_address = ir_data_out[4:0];
        2'b01: memory_address = acc_data_out[4:0];
        2'b10: memory_address = pc_data_out;
        default: memory_address = 5'b0;
    endcase
end
```

Now, `memory_address` is defined as a reg type and can be properly assigned in the always block.

## USER
Looks good. Can you now instantiate the control unit?

## ASSISTANT
Certainly! Here's the instantiation of the control unit:

```verilog
// Instantiate Control Unit
control_unit cu_inst (
    .clk(clk),
    .rst(rst),
    .processor_enable(proc_en),
    .processor_halted(halt),
    .instruction(ir_data_out),
    .ZF(zero_flag),

    .PC_write_enable(cu_PC_write_enable),
    .PC_mux_select(cu_PC_mux_select),

    .ACC_write_enable(cu_ACC_write_enable),
    .ACC_mux_select(cu_ACC_mux_select),

    .IR_load_enable(cu_IR_load_enable),

    .ALU_opcode(alu_opcode),
    .ALU_inputB_mux_select(cu_ALU_inputB_mux_select),

    .Memory_write_enable(cu_Memory_write_enable),
    .Memory_address_mux_select(cu_Memory_address_mux_select),

    .scan_enable(scan_enable),
    .scan_in(scan_enable),
    .scan_out(control_unit_scan_out)
);
```

This code instantiates the control unit using the `control_unit` module and connects its inputs and outputs to the appropriate wires and signals. The control unit is responsible for managing the processor's control signals based on the current instruction and other input signals.

## USER
The control unit's scan_in should come from the top level scan_in?

## ASSISTANT
You are right, my apologies for the mistake. The control unit's `scan_in` should come from the top-level `scan_in`. Here's the corrected instantiation of the control unit:

```verilog
// Instantiate Control Unit
control_unit cu_inst (
    .clk(clk),
    .rst(rst),
    .processor_enable(proc_en),
    .processor_halted(halt),
    .instruction(ir_data_out),
    .ZF(zero_flag),

    .PC_write_enable(cu_PC_write_enable),
    .PC_mux_select(cu_PC_mux_select),

    .ACC_write_enable(cu_ACC_write_enable),
    .ACC_mux_select(cu_ACC_mux_select),

    .IR_load_enable(cu_IR_load_enable),

    .ALU_opcode(alu_opcode),
    .ALU_inputB_mux_select(cu_ALU_inputB_mux_select),

    .Memory_write_enable(cu_Memory_write_enable),
    .Memory_address_mux_select(cu_Memory_address_mux_select),

    .scan_enable(scan_enable),
    .scan_in(scan_in),
    .scan_out(control_unit_scan_out)
);
```

Now, the `scan_in` of the control unit is correctly connected to the top-level `scan_in` input.

## USER
We have just a few things left! Can you write code to provide the zero_flag, and can you connect the appropriate signal to the top-level scan_out?

## ASSISTANT
Sure! Here's the code to generate the `zero_flag` and connect the `scan_out` signal to the top-level output:

```verilog
// Generate zero_flag
assign zero_flag = (acc_data_out == 8'b0);

// Connect top-level scan_out
assign scan_out = memory_scan_out;
```

The `zero_flag` is generated by comparing the `acc_data_out` to zero (8'b0). If the accumulator output is zero, the `zero_flag` will be true (1), otherwise, it will be false (0).

The top-level `scan_out` is connected to the `memory_scan_out` signal, as the memory bank is the last component in the scan chain.

# 12 - DATAPATH BRANCH UPDATE (return to conversation 08)

## USER
Can we update the PC multiplexer? The BEQ_FWD and BNE_FWD instructions have been updated to jump an extra instruction. Here's the definition of the PC control signals in the control unit documentation:
```
output reg PC_write_enable, // Enables writing to the PC
    output reg [1:0] PC_mux_select, // Selects the input for the PC multiplexer
    // 00: PC + 1 (FETCH cycle)
    // 01: ACC (JMP, JSR)
    // 10: PC - 3 (BEQ_BWD, BNE_BWD)
    // 11: PC + 2 (BEQ_FWD, BNE_FWD)
    
```
Here's your existing code, for reference:
```
// Instantiate PC multiplexer
wire [4:0] pc_plus_one = pc_data_out + 5'b1;
wire [4:0] pc_minus_three = pc_data_out - 5'b11;
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        default: pc_data_in = 5'b0;
    endcase
end
```

## ASSISTANT
Sure! I'll update the PC multiplexer to accommodate the new control signals for BEQ_FWD and BNE_FWD instructions. Here's the updated code:

```verilog
// Declare additional wires for PC mux
wire [4:0] pc_plus_two = pc_data_out + 5'b10;

// Instantiate PC multiplexer
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        2'b11: pc_data_in = pc_plus_two;
        default: pc_data_in = 5'b0;
    endcase
end
```

The updated PC multiplexer now handles the new control signal (2'b11) for BEQ_FWD and BNE_FWD instructions by using the `pc_plus_two` wire, which adds 2 to the current `pc_data_out`.

# 16 - DATAPATH BUG FIXING AND SMALL CHANGES (return to conversation 12)

## USER
In the control unit instantiation, I think the signal connected to ALU_opcode is wrong:
```
// Instantiate Control Unit
control_unit cu_inst (
    .clk(clk),

    .rst(rst),
    .processor_enable(proc_en),
    .processor_halted(halt),
    .instruction(ir_data_out),
    .ZF(zero_flag),

    .PC_write_enable(cu_PC_write_enable),
    .PC_mux_select(cu_PC_mux_select),

    .ACC_write_enable(cu_ACC_write_enable),
    .ACC_mux_select(cu_ACC_mux_select),

    .IR_load_enable(cu_IR_load_enable),

    .ALU_opcode(alu_opcode),
    .ALU_inputB_mux_select(cu_ALU_inputB_mux_select),

    .Memory_write_enable(cu_Memory_write_enable),
    .Memory_address_mux_select(cu_Memory_address_mux_select),

    .scan_enable(scan_enable),
    .scan_in(scan_in),
    .scan_out(control_unit_scan_out)
);
```

## ASSISTANT
You are correct. In the control unit instantiation, the `ALU_opcode` signal should be an output from the control unit, not an input. I apologize for the oversight. Here's the corrected instantiation of the control unit:

```verilog
// Instantiate Control Unit
control_unit cu_inst (
    .clk(clk),
    .rst(rst),
    .processor_enable(proc_en),
    .processor_halted(halt),
    .instruction(ir_data_out),
    .ZF(zero_flag),

    .PC_write_enable(cu_PC_write_enable),
    .PC_mux_select(cu_PC_mux_select),

    .ACC_write_enable(cu_ACC_write_enable),
    .ACC_mux_select(cu_ACC_mux_select),

    .IR_load_enable(cu_IR_load_enable),

    .ALU_opcode(cu_ALU_opcode),
    .ALU_inputB_mux_select(cu_ALU_inputB_mux_select),

    .Memory_write_enable(cu_Memory_write_enable),
    .Memory_address_mux_select(cu_Memory_address_mux_select),

    .scan_enable(scan_enable),
    .scan_in(scan_in),
    .scan_out(control_unit_scan_out)
);
```

Now, the `ALU_opcode` signal is correctly connected to the output from the control unit (`cu_ALU_opcode`).

## USER
I want to change the top level definition of the accumulator_microcontroller to the following:
```
module accumulator_microcontroller #(
    parameter MEM_SIZE = 32
) 
(
    input wire clk,
    input wire rst,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire proc_en,
    output wire halt,

    input wire btn_in,
    output wire [6:0] led_out
);
```
The MEM_SIZE, btn_in and led_out should be appropriately connected to the memory. Can you update the previous code:
```
// Instantiate Memory Bank
memory_bank #(
    .ADDR_WIDTH(5),
    .DATA_WIDTH(8),
    .MEM_SIZE(32)
) memory_inst (
    .clk(clk),
    .rst(rst),
    .address(memory_address),
    .data_in(acc_data_out),
    .write_enable(cu_Memory_write_enable),
    .data_out(memory_data_out),
    .scan_enable(scan_enable),
    .scan_in(acc_scan_out),
    .scan_out(memory_scan_out)
);
```

## ASSISTANT
Sure! I can update the previous code to accommodate the new top-level definition of the `accumulator_microcontroller` and connect the `MEM_SIZE`, `btn_in`, and `led_out` signals to the memory. Here's the updated instantiation of the Memory Bank:

```verilog
// Instantiate Memory Bank
memory_bank #(
    .ADDR_WIDTH(5),
    .DATA_WIDTH(8),
    .MEM_SIZE(MEM_SIZE)
) memory_inst (
    .clk(clk),
    .rst(rst),
    .address(memory_address),
    .data_in(acc_data_out),
    .write_enable(cu_Memory_write_enable),
    .data_out(memory_data_out),
    .scan_enable(scan_enable),
    .scan_in(acc_scan_out),
    .scan_out(memory_scan_out),

    // Connect btn_in and led_out
    .btn_in(btn_in),
    .led_out(led_out)
);
```

Now, the `MEM_SIZE` parameter is passed from the top-level module, and the `btn_in` and `led_out` signals are connected to the memory as inputs and outputs, respectively. Make sure to update the memory_bank module definition to include the `btn_in` and `led_out` ports.

## USER
Can we simplify the PC mux 
to just make the default case be the last case? 
```
// Declare additional wires for PC mux
wire [4:0] pc_plus_two = pc_data_out + 5'b10;

// Instantiate PC multiplexer
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        2'b11: pc_data_in = pc_plus_two;
        default: pc_data_in = 5'b0;
    endcase
end
```

## ASSISTANT
Certainly! You can merge the last case with the default case by simply removing the `default` keyword. Here's the simplified PC multiplexer:

```verilog
// Declare additional wires for PC mux
wire [4:0] pc_plus_two = pc_data_out + 5'b10;

// Instantiate PC multiplexer
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        2'b11: pc_data_in = pc_plus_two;
    endcase
end
```

Now, if none of the specified cases match `cu_PC_mux_select`, the mux will automatically select the `pc_plus_two` case (2'b11) as the default.

## USER
Can you make the last case be the default case with the default keyword?

## ASSISTANT
Certainly! To make the last case (2'b11) the default case using the `default` keyword, you can remove the explicit 2'b11 case and add the `default` keyword. Here's the updated PC multiplexer:

```verilog
// Declare additional wires for PC mux
wire [4:0] pc_plus_two = pc_data_out + 5'b10;

// Instantiate PC multiplexer
always @(*) begin
    case(cu_PC_mux_select)
        2'b00: pc_data_in = pc_plus_one;
        2'b01: pc_data_in = acc_data_out[4:0];
        2'b10: pc_data_in = pc_minus_three;
        default: pc_data_in = pc_plus_two;
    endcase
end
```

Now, if none of the specified cases match `cu_PC_mux_select`, the mux will automatically select the `pc_plus_two` case using the `default` keyword.

## USER
Can we update the Memory Address Multiplexer to use the pc_data_out as the default case?
```
// Instantiate Memory Address Multiplexer
always @(*) begin
    case(cu_Memory_address_mux_select)
        2'b00: memory_address = ir_data_out[4:0];
        2'b01: memory_address = acc_data_out[4:0];
        2'b10: memory_address = pc_data_out;
        default: memory_address = 5'b0;
    endcase
end
```

## ASSISTANT
Of course! To use `pc_data_out` as the default case for the Memory Address Multiplexer, you can remove the explicit 2'b10 case and add the `default` keyword. Here's the updated Memory Address Multiplexer:

```verilog
// Instantiate Memory Address Multiplexer
always @(*) begin
    case(cu_Memory_address_mux_select)
        2'b00: memory_address = ir_data_out[4:0];
        2'b01: memory_address = acc_data_out[4:0];
        default: memory_address = pc_data_out;
    endcase
end
```

Now, if none of the specified cases match `cu_Memory_address_mux_select`, the mux will automatically select the `pc_data_out` case using the `default` keyword.

## USER
Can we update the ACC multiplexer to make the PC data out be the default case?
```
// Instantiate ACC multiplexer
always @(*) begin
    case(cu_ACC_mux_select)
        2'b00: acc_data_in = alu_Y;
        2'b01: acc_data_in = memory_data_out;
        2'b10: acc_data_in = {3'b000, pc_data_out};
        default: acc_data_in = 8'b0;
    endcase
end
```

## ASSISTANT
Sure! To use the PC data out as the default case for the ACC multiplexer, you can remove the explicit 2'b10 case and add the `default` keyword. Here's the updated ACC multiplexer:

```verilog
// Instantiate ACC multiplexer
always @(*) begin
    case(cu_ACC_mux_select)
        2'b00: acc_data_in = alu_Y;
        2'b01: acc_data_in = memory_data_out;
        default: acc_data_in = {3'b000, pc_data_out};
    endcase
end
```

Now, if none of the specified cases match `cu_ACC_mux_select`, the mux will automatically select the `pc_data_out` case using the `default` keyword.