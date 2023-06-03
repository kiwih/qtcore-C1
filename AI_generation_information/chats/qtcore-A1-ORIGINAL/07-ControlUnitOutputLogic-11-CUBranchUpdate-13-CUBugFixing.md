# 07 - CONTROL UNIT OUTPUT LOGIC

## USER (#restarts:1)
This is a subset of the specification for "qtcore-A1", an 8-bit accumulator-based RISC processor.

## The ISA

**Immediate Data Manipulation Instructions**

ADDI: Add immediate to Accumulator
Opcode (4 bits): 1110
Immediate (4 bits): 4-bit Immediate
Register Effects: ACC <- ACC + IMM

**Instructions with Variable-Data Operands**

LDA: Load Accumulator with memory contents
Opcode (3 bits): 000
Operand (5 bits): Memory Address
Register Effects: ACC <- M[Address]

STA: Store Accumulator to memory
Opcode (3 bits): 001
Operand (5 bits): Memory Address
Register Effects: M[Address] <- ACC

ADD: Add memory contents to Accumulator
Opcode (3 bits): 010
Operand (5 bits): Memory Address
Register Effects: ACC <- ACC + M[Address]

SUB: Subtract memory contents from Accumulator
Opcode (3 bits): 011
Operand (5 bits): Memory Address
Register Effects: ACC <- ACC - M[Address]

AND: AND memory contents with Accumulator
Opcode (3 bits): 100
Operand (5 bits): Memory Address
Register Effects: ACC <- ACC & M[Address]

OR: OR memory contents with Accumulator
Opcode (3 bits): 101
Operand (5 bits): Memory Address
Register Effects: ACC <- ACC | M[Address]

XOR: XOR memory contents with Accumulator
Opcode (3 bits): 110
Operand (5 bits): Memory Address
Register Effects: ACC <- ACC ^ M[Address]

**Control and Branching Instructions**

JMP: Jump to memory address
Opcode (8 bits): 11110000
PC Behavior: PC <- ACC (Load the PC with the address stored in the accumulator)

JSR: Jump to Subroutine (save address to ACC)
Opcode (8 bits): 11110001
PC Behavior: ACC <- PC + 1, PC <- ACC (Save the next address in ACC, then jump to the address in ACC)

BEQ_FWD: Branch if equal, forward (branch if ACC == 0)
Opcode (8 bits): 11110010
PC Behavior: If ACC == 0, then PC <- PC + 2 (Jump 1 instruction forward if ACC is zero)

BEQ_BWD: Branch if equal, backward (branch if ACC == 0)
Opcode (8 bits): 11110011
PC Behavior: If ACC == 0, then PC <- PC - 2 (Jump 1 instruction backward if ACC is zero)

BNE_FWD: Branch if not equal, forward (branch if ACC != 0)
Opcode (8 bits): 11110100
PC Behavior: If ACC != 0, then PC <- PC + 2 (Jump 1 instruction forward if ACC is non-zero)

BNE_BWD: Branch if not equal, backward (branch if ACC != 0)
Opcode (8 bits): 11110101
PC Behavior: If ACC != 0, then PC <- PC - 2 (Jump 1 instruction backward if ACC is non-zero)

HLT: Halt the processor until reset
Opcode (8 bits): 11111111
PC Behavior: Stop execution (PC does not change until a reset occurs)

**Data Manipulation Instructions**

SHL: Shift Accumulator left
Opcode (8 bits): 11110110
Register Effects: ACC <- ACC << 1

SHR: Shift Accumulator right
Opcode (8 bits): 11110111
Register Effects: ACC <- ACC >> 1

SHL4: Shift Accumulator left by 4 bits
Opcode (8 bits): 11111000
Register Effects: ACC <- ACC << 4

ROL: Rotate Accumulator left
Opcode (8 bits): 11111001
Register Effects: ACC <- (ACC << 1) OR (ACC >> 7)

ROR: Rotate Accumulator right
Opcode (8 bits): 11111010
Register Effects: ACC <- (ACC >> 1) OR (ACC << 7)

LDAR: Load Accumulator via indirect memory access (ACC as ptr)
Opcode (8 bits): 11111011
Register Effects: ACC <- M[ACC]

DEC: Decrement Accumulator
Opcode (8 bits): 11111100
Register Effects: ACC <- ACC - 1

CLR: Clear (Zero) Accumulator
Opcode (8 bits): 11111101
Register Effects: ACC <- 0

INV: Invert (NOT) Accumulator
Opcode (8 bits): 11111110
Register Effects: ACC <- ~ACC

## Internal datapath components

Internally, qtcore-A1 will use the following registers:

1. Program Counter (PC): 5 bits - Stores the address of the next instruction to be executed. The reduced size is sufficient for addressing the 32-byte register memory (2^5 = 32).
2. Accumulator (ACC): 8 bits - Stores the result of arithmetic and logical operations, as well as acts as a temporary storage for addresses during certain operations (e.g., JSR instruction).
3. Instruction Register (IR): 8 bits - Stores the current instruction being executed.

When instantiating the ALU, use the following definition:
```
module isa_to_alu_opcode (
input [7:0] isa_instr,
output reg [3:0] alu_opcode
);
```

## For Multi-cycle execution:

**FETCH cycle (all instructions)**

1. IR <- M[PC]
2. PC <- PC + 1

**EXECUTE cycle**

For **Immediate Data Manipulation Instructions**:

* ADDI: ACC <- ACC + IMM

For **Instructions with Variable-Data Operands**:

* LDA: ACC <- M[Address]
* STA: M[Address] <- ACC
* ADD: ACC <- ACC + M[Address]
* SUB: ACC <- ACC - M[Address]
* AND: ACC <- ACC & M[Address]
* OR: ACC <- ACC | M[Address]
* XOR: ACC <- ACC ^ M[Address]

For **Control and Branching Instructions**:

* JMP: PC <- ACC
* JSR: ACC <- PC, PC <- ACC
* BEQ_FWD: If ACC == 0, then PC <- PC + 1
* BEQ_BWD: If ACC == 0, then PC <- PC - 3
* BNE_FWD: If ACC != 0, then PC <- PC + 1
* BNE_BWD: If ACC != 0, then PC <- PC - 3
* HLT: (No operation, processor halted)

For **Data Manipulation Instructions**:

* SHL: ACC <- ACC << 1
* SHR: ACC <- ACC >> 1
* SHL4: ACC <- ACC << 4
* ROL: ACC <- (ACC << 1) OR (ACC >> 7)
* ROR: ACC <- (ACC >> 1) OR (ACC << 7)
* LDAR: ACC <- M[ACC]
* DEC: ACC <- ACC - 1
* CLR: ACC <- 0
* INV: ACC <- ~ACC

## Top level control unit module definition

```
module control_unit (
input wire clk, // Clock input
input wire rst, // Reset input
input wire processor_enable, // Processor enable signal
output wire processor_halted, // Processor halted signal
input wire [7:0] instruction, // Input from the Instruction Register (IR)
input wire ZF, // Zero Flag input, true when ACC is zero

output wire PC_write_enable, // Enables writing to the PC
output wire [1:0] PC_mux_select, // Selects the input for the PC multiplexer
// 00: PC + 1 (FETCH cycle, BEQ_FWD, BNE_FWD)
// 01: ACC (JMP, JSR)
// 10: PC - 3 (BEQ_BWD, BNE_BWD)

output wire ACC_write_enable, // Enables writing to the ACC
output wire [1:0] ACC_mux_select, // Selects the input for the ACC multiplexer
// 00: ALU output
// 01: Memory contents (LDA, LDAR)
// 10: PC (JSR)

output wire IR_load_enable, // Enables loading new instruction into IR from memory

output wire [3:0] ALU_opcode, // Control signal specifying the ALU operation
output wire ALU_inputB_mux_select, // Selects input B for the ALU multiplexer
// 0: Memory contents (ADD, SUB, AND, OR, XOR)
// 1: Immediate (ADDI)

output wire Memory_write_enable, // Enables writing to memory (STA)
output wire [1:0] Memory_address_mux_select, // Selects input for memory address multiplexer
// 00: IR[4:0] (LDA, STA, ADD, SUB, AND, OR, XOR)
// 01: ACC (LDAR)
// 10: PC (Instruction fetching)

// Scan chain signals
input wire scan_enable, // Scan chain enable signal
input wire scan_in, // Scan chain input
output wire scan_out // Scan chain output
);

// Define state constants
localparam STATE_RESET = 3'b000;
localparam STATE_FETCH = 3'b001;
localparam STATE_EXECUTE = 3'b010;
localparam STATE_HALT = 3'b100;

// Instantiate shift register for state storage (one-hot encoding)
reg [2:0] state_in;
wire [2:0] state_out;
// state register here...
// ...

// Combinational logic for state_in
always @(*) begin
// ...
end

// Control signals go here

endmodule
```

Let's now work on the code that drives the control signals. First, can you provide code to instantiate the ALU ISA decoder inside the control_unit module? This will made the ALU opcode for us.

## ASSISTANT
To instantiate the ALU ISA decoder inside the control_unit module, you can add the following code inside the `control_unit` module definition:

```verilog
// Instantiate ALU ISA decoder
isa_to_alu_opcode isa_decoder (
    .isa_instr(instruction),
    .alu_opcode(ALU_opcode)
);
```

This code connects the `instruction` input wire from the control unit to the `isa_instr` input of the `isa_to_alu_opcode` module and the `ALU_opcode` output wire from the control unit to the `alu_opcode` output of the `isa_to_alu_opcode` module.

Remember to include the `isa_to_alu_opcode` module definition provided in your specification before the `control_unit` module in your Verilog source file. Your entire control_unit module, including the ALU ISA decoder instantiation, should look like this:

```verilog
module control_unit (
input wire clk, //
```

## USER
Looks perfect. Can you now explain and write combinational verilog code to drive the PC control signals?

## ASSISTANT
Sure, let's break down the PC control signals and discuss their behavior based on the instruction set and the internal state of the control unit.

1. `PC_write_enable`: This signal is used to enable writing to the Program Counter (PC). We need to enable this signal whenever we want to change the PC, which happens in the following cases:

   - FETCH cycle (PC <- PC + 1)
   - JMP (PC <- ACC)
   - JSR (PC <- ACC)
   - BEQ_FWD (If ACC == 0, then PC <- PC + 1)
   - BEQ_BWD (If ACC == 0, then PC <- PC - 3)
   - BNE_FWD (If ACC != 0, then PC <- PC + 1)
   - BNE_BWD (If ACC != 0, then PC <- PC - 3)

2. `PC_mux_select`: This 2-bit signal selects the input for the PC multiplexer. Based on the instruction set, we have the following cases:

   - `2'b00`: PC + 1 (FETCH cycle, BEQ_FWD, BNE_FWD)
   - `2'b01`: ACC (JMP, JSR)
   - `2'b10`: PC - 3 (BEQ_BWD, BNE_BWD)

Now let's write the combinational Verilog code to drive these PC control signals. We'll use an `always @(*)` block to describe the combinational logic:

```verilog
always @(*) begin
    // Default values
    PC_write_enable = 1'b0;
    PC_mux_select = 2'b00;

    case (state_out)
        STATE_FETCH: begin
            PC_write_enable = 1'b1;
            PC_mux_select = 2'b00;
        end
        STATE_EXECUTE: begin
            case (instruction[7:0])
                8'b11110000: begin // JMP
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110001: begin // JSR
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110010: begin // BEQ_FWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b00;
                    end
                end
                8'b11110011: begin // BEQ_BWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
                8'b11110100: begin // BNE_FWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b00;
                    end
                end
                8'b11110101: begin // BNE_BWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
            endcase
        end
    endcase
end
```

This code sets the default values for the PC control signals, then changes them based on the current state and the instruction being executed. The `case` statements are used to match the specific instructions that require changes to the PC control signals.

## USER
Looks good. Since you're using an always block, can you rewrite the control_unit module definition 
I/O to use "reg" on the relevant control signal outputs? Keep the comments though

## ASSISTANT
Certainly! I have updated the `control_unit` module definition with "reg" on the relevant control signal outputs. The comments have been preserved as well:

```verilog
module control_unit (
input wire clk, // Clock input
input wire rst, // Reset input
input wire processor_enable, // Processor enable signal
output wire processor_halted, // Processor halted signal
input wire [7:0] instruction, // Input from the Instruction Register (IR)
input wire ZF, // Zero Flag input, true when ACC is zero

output reg PC_write_enable, // Enables writing to the PC
output reg [1:0] PC_mux_select, // Selects the input for the PC multiplexer
// 00: PC + 1 (FETCH cycle, BEQ_FWD, BNE_FWD)
// 01: ACC (JMP, JSR)
// 10: PC - 3 (BEQ_BWD, BNE_BWD)

output wire ACC_write_enable, // Enables writing to the ACC
output wire [1:0] ACC_mux_select, // Selects the input for the ACC multiplexer
// 00: ALU output
// 01: Memory contents (LDA, LDAR)
// 10: PC (JSR)

output wire IR_load_enable, // Enables loading new instruction into IR from memory

output reg [3:0] ALU_opcode, // Control signal specifying the ALU operation
output wire ALU_inputB_mux_select, // Selects input B for the ALU multiplexer
// 0: Memory contents (ADD, SUB, AND, OR, XOR)
// 1: Immediate (ADDI)

output wire Memory_write_enable, // Enables writing to memory (STA)
output wire [1:0] Memory_address_mux_select, // Selects input for memory address multiplexer
// 00: IR[4:0] (LDA, STA, ADD, SUB, AND, OR, XOR)
// 01: ACC (LDAR)
// 10: PC (Instruction fetching)

// Scan chain signals
input wire scan_enable, // Scan chain enable signal
input wire scan_in, // Scan chain input
output wire scan_out // Scan chain output
);

// Define state constants
localparam STATE_RESET = 3'b000;
localparam STATE_FETCH = 3'b001;
localparam STATE_EXECUTE = 3'b010;
localparam STATE_HALT = 3'b100;

// Instantiate shift register for state storage (one-hot encoding)
reg [2:0] state_in;
wire [2:0] state_out;
// state register here...
// ...

// Combinational logic for state_in
always @(*) begin
// ...
end

// Control signals go here

endmodule
```

With these changes, the `control_unit` module definition now uses "reg" for the PC control signal outputs, as well as for the ALU_opcode output.

## USER
The ALU_opcode should stay as a wire though - it's coming from the ALU ISA decoder instance?

## ASSISTANT
Apologies for the confusion. You are correct, the `ALU_opcode` should remain as a wire since it is coming from the ALU ISA decoder instance. Here's the corrected `control_unit` module definition:

```verilog
module control_unit (
input wire clk, // Clock input
input wire rst, // Reset input
input wire processor_enable, // Processor enable signal
output wire processor_halted, // Processor halted signal
input wire [7:0] instruction, // Input from the Instruction Register (IR)
input wire ZF, // Zero Flag input, true when ACC is zero

output reg PC_write_enable, // Enables writing to the PC
output reg [1:0] PC_mux_select, // Selects the input for the PC multiplexer
// 00: PC + 1 (FETCH cycle, BEQ_FWD, BNE_FWD)
// 01: ACC (JMP, JSR)
// 10: PC - 3 (BEQ_BWD, BNE_BWD)

output wire ACC_write_enable, // Enables writing to the ACC
output wire [1:0] ACC_mux_select, // Selects the input for the ACC multiplexer
// 00: ALU output
// 01: Memory contents (LDA, LDAR)
// 10: PC (JSR)

output wire IR_load_enable, // Enables loading new instruction into IR from memory

output wire [3:0] ALU_opcode, // Control signal specifying the ALU operation
output wire ALU_inputB_mux_select, // Selects input B for the ALU multiplexer
// 0: Memory contents (ADD, SUB, AND, OR, XOR)
// 1: Immediate (ADDI)

output wire Memory_write_enable, // Enables writing to memory (STA)
output wire [1:0] Memory_address_mux_select, // Selects input for memory address multiplexer
// 00: IR[4:0] (LDA, STA, ADD, SUB, AND, OR, XOR)
// 01: ACC (LDAR)
// 10: PC (Instruction fetching)

// Scan chain signals
input wire scan_enable, // Scan chain enable signal
input wire scan_in, // Scan chain input
output wire scan_out // Scan chain output
);

// Define state constants
localparam STATE_RESET = 3'b000;
localparam STATE_FETCH = 3'b001;
localparam STATE_EXECUTE = 3'b010;
localparam STATE_HALT = 3'b100;

// Instantiate shift register for state storage (one-hot encoding)
reg [2:0] state_in;
wire [2:0] state_out;
// state register here...
// ...

// Combinational logic for state_in
always @(*) begin
// ...
end

// Control signals go here

endmodule
```

Now the `ALU_opcode` is set as a wire, while the other relevant control signal outputs remain as "reg".

## USER (#restarts:2)
Looks perfect. Can you now explain and write combinational verilog code to drive the ACC
control signals? There are more classes of instruction that effect ACC.

## ASSISTANT
Certainly! The ACC control signals include `ACC_write_enable` and `ACC_mux_select`. We need to drive these signals based on the current instruction and its associated operation. Here is the combinational logic for the ACC control signals:

```verilog
always @(*) begin
  // Default values
  ACC_write_enable = 1'b0;
  ACC_mux_select = 2'b00;

  // Check if the current state is EXECUTE
  if (state_out == STATE_EXECUTE) begin
    // Immediate Data Manipulation Instructions
    if (instruction[7:4] == 4'b1110) begin // ADDI
      ACC_write_enable = 1'b1;
      ACC_mux_select = 2'b00; // ALU output
    end

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
      3'b000: begin // LDA
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
      3'b001: ; // STA (No effect on ACC)
      3'b010: ; // ADD (handled by default values)
      3'b011: ; // SUB (handled by default values)
      3'b100: ; // AND (handled by default values)
      3'b101: ; // OR  (handled by default values)
      3'b110: ; // XOR (handled by default values)
    endcase

    // Control and Branching Instructions
    // (No direct effect on ACC)

    // Data Manipulation Instructions
    case (instruction)
      8'b11110110: ; // SHL (handled by default values)
      8'b11110111: ; // SHR (handled by default values)
      8'b11111000: ; // SHL4 (handled by default values)
      8'b11111001: ; // ROL (handled by default values)
      8'b11111010: ; // ROR (handled by default values)
      8'b11111011: begin // LDAR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
      8'b11111100: ; // DEC (handled by default values)
      8'b11111101: ; // CLR (handled by default values)
      8'b11111110: ; // INV (handled by default values)
    endcase

  end // if (state_out == STATE_EXECUTE)
end
```

This code sets the default values for `ACC_write_enable` and `ACC_mux_select` and then updates them based on the current instruction during the EXECUTE state. The ALU output is the default source for the accumulator (ADD, SUB, AND, OR, XOR, SHL, SHR, SHL4, ROL, ROR, DEC, CLR, INV). For LDA and LDAR, the ACC mux select is changed to select memory contents as the source. STA has no effect on the ACC, and control and branching instructions do not have a direct effect on the ACC.

## USER (#restarts:1)
The default values for ACC is to have write disabled, but many of these instructions do need to write to ACC? They should enable ACC write in those cases.

## ASSISTANT
You are correct. I apologize for the oversight. I'll update the code to enable ACC write for the relevant instructions.

```verilog
always @(*) begin
  // Default values
  ACC_write_enable = 1'b0;
  ACC_mux_select = 2'b00;

  // Check if the current state is EXECUTE
  if (state_out == STATE_EXECUTE) begin
    // Immediate Data Manipulation Instructions
    if (instruction[7:4] == 4'b1110) begin // ADDI
      ACC_write_enable = 1'b1;
      ACC_mux_select = 2'b00; // ALU output
    end

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
      3'b000: begin // LDA
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
      3'b001: ; // STA (No effect on ACC)
      3'b010,
      3'b011,
      3'b100,
      3'b101,
      3'b110: ACC_write_enable = 1'b1; // ADD, SUB, AND, OR, XOR
    endcase

    // Control and Branching Instructions
    // (No direct effect on ACC)

    // Data Manipulation Instructions
    case (instruction)
      8'b11110110,
      8'b11110111,
      8'b11111000,
      8'b11111001,
      8'b11111010,
      8'b11111100,
      8'b11111101,
      8'b11111110: ACC_write_enable = 1'b1; // SHL, SHR, SHL4, ROL, ROR, DEC, CLR, INV
      8'b11111011: begin // LDAR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
    endcase

  end // if (state_out == STATE_EXECUTE)
end
```

Now, the ACC write is enabled for the relevant instructions, including ADD, SUB, AND, OR, XOR, SHL, SHR, SHL4, ROL, ROR, DEC, CLR, and INV.

## USER (#restarts:1)
Looks good - I can see one oversight however, which is there is one control and branching instruction which does write to ACC

## ASSISTANT
You're right, I missed the JSR instruction. I'll update the code to include it.

```verilog
always @(*) begin
  // Default values
  ACC_write_enable = 1'b0;
  ACC_mux_select = 2'b00;

  // Check if the current state is EXECUTE
  if (state_out == STATE_EXECUTE) begin
    // Immediate Data Manipulation Instructions
    if (instruction[7:4] == 4'b1110) begin // ADDI
      ACC_write_enable = 1'b1;
      ACC_mux_select = 2'b00; // ALU output
    end

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
      3'b000: begin // LDA
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
      3'b001: ; // STA (No effect on ACC)
      3'b010,
      3'b011,
      3'b100,
      3'b101,
      3'b110: ACC_write_enable = 1'b1; // ADD, SUB, AND, OR, XOR
    endcase

    // Control and Branching Instructions
    case (instruction)
      8'b11110001: begin // JSR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b10; // PC
      end
    endcase

    // Data Manipulation Instructions
    case (instruction)
      8'b11110110,
      8'b11110111,
      8'b11111000,
      8'b11111001,
      8'b11111010,
      8'b11111100,
      8'b11111101,
      8'b11111110: ACC_write_enable = 1'b1; // SHL, SHR, SHL4, ROL, ROR, DEC, CLR, INV
      8'b11111011: begin // LDAR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
    endcase

  end // if (state_out == STATE_EXECUTE)
end
```

Now the JSR instruction is accounted for, and ACC write is enabled for it.

## USER
Looks good. Since we're using always blocks for the combinational outputs, can you rewrite the control_unit module definition I/O to use "reg" on the PC, ACC, IR, and Memory control signal outputs? Keep the comments though

## ASSISTANT
Certainly! I've updated the control_unit module definition with "reg" for the PC, ACC, IR, and Memory control signal outputs while keeping the comments.

```verilog
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

Now the control_unit module definition has been updated with "reg" for the PC, ACC, IR, and Memory control signal outputs.

## USER (#restarts:1)
Looks perfect. Can you now explain and write combinational verilog code to drive the IR control signal?

## ASSISTANT
Sure! The IR control signal is responsible for enabling the loading of a new instruction into the Instruction Register (IR). This happens during the FETCH cycle, and the IR_load_enable signal should be set to 1 during this cycle.

Here's the combinational Verilog code for the IR control signal:

```verilog
always @(*) begin
    // Default: IR_load_enable is 0 (disabled)
    IR_load_enable = 1'b0;

    // During FETCH cycle, set IR_load_enable to 1 (enabled)
    if (state_out == STATE_FETCH) begin
        IR_load_enable = 1'b1;
    end
end
```

In this code, we use an always block to assign the IR_load_enable signal. By default, the signal is set to 0 (disabled). We then check if the current state is the FETCH state (state_out == STATE_FETCH). If so, we enable the IR_load_enable signal by setting it to 1.

## USER
Looks perfect. Can you now explain and write combinational verilog code to drive the ALU mux
 control signal?

## ASSISTANT
Sure! The ALU mux control signal, ALU_inputB_mux_select, determines the source of input B for the ALU. Depending on the instruction type, input B can be either the memory contents (e.g., ADD, SUB, AND, OR, XOR) or an immediate value (e.g., ADDI).

Here's the combinational Verilog code for the ALU mux control signal:

```verilog
always @(*) begin
    // Default: ALU_inputB_mux_select is 0 (memory contents)
    ALU_inputB_mux_select = 1'b0;

    // Immediate Data Manipulation Instructions
    if (instruction[7:4] == 4'b1110) begin // ADDI
        ALU_inputB_mux_select = 1'b1; // Select immediate value as input B
    end
end
```

In this code, we use an always block to assign the ALU_inputB_mux_select signal. By default, the signal is set to 0, which selects memory contents as input B. We then check if the instruction type is ADDI by comparing the upper 4 bits of the instruction with the ADDI opcode (4'b1110). If the condition is true, we set ALU_inputB_mux_select to 1 to select the immediate value as input B for the ALU.

## USER
Looks perfect. Can you now explain and write combinational verilog code to drive the memory control signals?

## ASSISTANT
Sure! The memory control signals, Memory_write_enable and Memory_address_mux_select, control the memory access during instruction execution. Memory_write_enable determines if the current operation should write to memory, while Memory_address_mux_select selects the appropriate address input for memory access.

Here's the combinational Verilog code for the memory control signals:

```verilog
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
        3'b001: begin // STA
            Memory_write_enable = 1'b1; // Enable writing to memory
        end
    endcase

    // Data Manipulation Instructions
    if (instruction[7] == 1'b1) begin
        case (instruction[6:3])
            4'b1011: begin // LDAR
                Memory_address_mux_select = 2'b01; // Select ACC as memory address
            end
        endcase
    end

    // Instruction fetching
    if (state_out == STATE_FETCH) begin
        Memory_address_mux_select = 2'b10; // Select PC as memory address
    end
end
```

In this code, we use an always block to assign the memory control signals. By default, Memory_write_enable is set to 0 (disabled), and Memory_address_mux_select is set to 00 (IR[4:0]).

We use a case statement to check for the STA instruction (opcode 3'b001). If the instruction is STA, we enable the Memory_write_enable signal by setting it to 1.

Next, we check for the LDAR instruction (opcode 4'b1011) under Data Manipulation Instructions. If the instruction is LDAR, we set Memory_address_mux_select to 01, which selects ACC as the memory address.

Finally, during the FETCH cycle (state_out == STATE_FETCH), we set Memory_address_mux_select to 10, which selects PC as the memory address for instruction fetching.

# 11 - CU BRANCH UPDATE (return to conversation 07)

## USER (#restarts:1)
I've updated the forward branch instructions to jump an extra instruction - this means that we'll need an extra type of PC update, since the existing PC+1 used for BEQ_FWD and BNE_FWD won't work (we'll need a PC+2). Can you update the control signal multiplexer comments to suit in the top-level I/O? Fortunately, we don't need to increase the width of the signal as there was an unused encoding in our multiplexer control signals.

Here's the existing I/O for reference:
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

## ASSISTANT
I understand that you've updated the forward branch instructions to jump an extra instruction. We can update the control signal multiplexer comments to reflect this change, and luckily, we don't need to increase the width of the PC_mux_select signal. Here's the updated I/O for the control_unit module:

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
// 00: PC + 1 (FETCH cycle)
// 01: ACC (JMP, JSR)
// 10: PC - 3 (BEQ_BWD, BNE_BWD)
// 11: PC + 2 (BEQ_FWD, BNE_FWD)

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

I have added a new encoding (11) for the PC_mux_select signal that represents PC + 2 for the updated forward branch instructions (BEQ_FWD and BNE_FWD). The comments have been updated to reflect these changes.

## USER (#restarts:1)
Looks perfect. Can you now explain and write and updated version of the combinational verilog code to drive the PC control signals, taking into account the new multiplexer control signal options? Here's your existing code for reference:

```
always @(*) begin
    // Default values
    PC_write_enable = 1'b0;
    PC_mux_select = 2'b00;

    case (state_out)
        STATE_FETCH: begin
            PC_write_enable = 1'b1;
            PC_mux_select = 2'b00;
        end
        STATE_EXECUTE: begin
            case (instruction[7:0])
                8'b11110000: begin // JMP
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110001: begin // JSR
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110010: begin // BEQ_FWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b00;
                    end
                end
                8'b11110011: begin // BEQ_BWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
                8'b11110100: begin // BNE_FWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b00;
                    end
                end
                8'b11110101: begin // BNE_BWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
            endcase
        end
    endcase
end

```

## ASSISTANT
I understand that you want to update the combinational Verilog code for driving the PC control signals to take into account the new multiplexer control signal options (PC + 2 for BEQ_FWD and BNE_FWD). Here's the updated code with the changes:

```
always @(*) begin
    // Default values
    PC_write_enable = 1'b0;
    PC_mux_select = 2'b00;

    case (state_out)
        STATE_FETCH: begin
            PC_write_enable = 1'b1;
            PC_mux_select = 2'b00;
        end
        STATE_EXECUTE: begin
            case (instruction[7:0])
                8'b11110000: begin // JMP
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110001: begin // JSR
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110010: begin // BEQ_FWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b11;
                    end
                end
                8'b11110011: begin // BEQ_BWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
                8'b11110100: begin // BNE_FWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b11;
                    end
                end
                8'b11110101: begin // BNE_BWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
            endcase
        end
    endcase
end
```

I've updated the PC_mux_select value for BEQ_FWD and BNE_FWD cases to be 2'b11, which corresponds to the new PC + 2 encoding in the PC_mux_select signal comments.

# 13 - CU BUG FIXING (return to conversation 11)

## USER (#restarts:1)
I don't think the opcode for LDAR is being checked for correctly in this code (the correct opcode is 11111011)
```
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
        3'b001: begin // STA
            Memory_write_enable = 1'b1; // Enable writing to memory
        end
    endcase

    // Data Manipulation Instructions
    if (instruction[7] == 1'b1) begin
        case (instruction[6:3])
            4'b1011: begin // LDAR
                Memory_address_mux_select = 2'b01; // Select ACC as memory address
            end
        endcase
    end

    // Instruction fetching
    if (state_out == STATE_FETCH) begin
        Memory_address_mux_select = 2'b10; // Select PC as memory address
    end
end
```

## ASSISTANT
You're right. The code you provided is checking the opcode for LDAR incorrectly. The correct opcode for LDAR is 8'b11111011, but the code is checking only the [6:3] bits. We should check all 8 bits of the instruction. Here's the updated code:

```
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
        3'b001: begin // STA
            Memory_write_enable = 1'b1; // Enable writing to memory
        end
    endcase

    // Data Manipulation Instructions
    case (instruction[7:0])
        8'b11111011: begin // LDAR
            Memory_address_mux_select = 2'b01; // Select ACC as memory address
        end
    endcase

    // Instruction fetching
    if (state_out == STATE_FETCH) begin
        Memory_address_mux_select = 2'b10; // Select PC as memory address
    end
end
```

Now, the code checks all 8 bits of the instruction using a case statement, which ensures that the opcode for LDAR is checked correctly.

## USER
There's another problem with this code - the write enable should only be high in the execute state?

## ASSISTANT
You're right, the memory write enable should only be set high in the execute state for the instructions that require it. I'll update the code to account for this:

```verilog
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    if (state_out == STATE_EXECUTE) begin
        // Instructions with Variable-Data Operands
        case (instruction[7:5])
            3'b001: begin // STA
                Memory_write_enable = 1'b1; // Enable writing to memory
            end
        endcase

        // Data Manipulation Instructions
        case (instruction[7:0])
            8'b11111011: begin // LDAR
                Memory_address_mux_select = 2'b01; // Select ACC as memory address
            end
        endcase
    end

    // Instruction fetching
    if (state_out == STATE_FETCH) begin
        Memory_address_mux_select = 2'b10; // Select PC as memory address
    end
end
```

Now, the memory write enable is set high only during the execute state for the STA instruction, and the memory address selection is also updated only during the execute state for the LDAR instruction.

## USER
Can we update the PC control signals
 code so that the outputs are only emitted if the processor_enable signal is true?
```
always @(*) begin
    // Default values
    PC_write_enable = 1'b0;
    PC_mux_select = 2'b00;

    case (state_out)
        STATE_FETCH: begin
            PC_write_enable = 1'b1;
            PC_mux_select = 2'b00;
        end
        STATE_EXECUTE: begin
            case (instruction[7:0])
                8'b11110000: begin // JMP
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110001: begin // JSR
                    PC_write_enable = 1'b1;
                    PC_mux_select = 2'b01;
                end
                8'b11110010: begin // BEQ_FWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b11;
                    end
                end
                8'b11110011: begin // BEQ_BWD
                    if (ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
                8'b11110100: begin // BNE_FWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b11;
                    end
                end
                8'b11110101: begin // BNE_BWD
                    if (!ZF) begin
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b10;
                    end
                end
            endcase
        end
    endcase
end
```

## ASSISTANT
Sure, I can update the code so that the outputs are only emitted if the `processor_enable` signal is true. I will add an `if (processor_enable)` condition around the entire state machine:

```verilog
always @(*) begin
    // Default values
    PC_write_enable = 1'b0;
    PC_mux_select = 2'b00;

    if (processor_enable) begin
        case (state_out)
            STATE_FETCH: begin
                PC_write_enable = 1'b1;
                PC_mux_select = 2'b00;
            end
            STATE_EXECUTE: begin
                case (instruction[7:0])
                    8'b11110000: begin // JMP
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b01;
                    end
                    8'b11110001: begin // JSR
                        PC_write_enable = 1'b1;
                        PC_mux_select = 2'b01;
                    end
                    8'b11110010: begin // BEQ_FWD
                        if (ZF) begin
                            PC_write_enable = 1'b1;
                            PC_mux_select = 2'b11;
                        end
                    end
                    8'b11110011: begin // BEQ_BWD
                        if (ZF) begin
                            PC_write_enable = 1'b1;
                            PC_mux_select = 2'b10;
                        end
                    end
                    8'b11110100: begin // BNE_FWD
                        if (!ZF) begin
                            PC_write_enable = 1'b1;
                            PC_mux_select = 2'b11;
                        end
                    end
                    8'b11110101: begin // BNE_BWD
                        if (!ZF) begin
                            PC_write_enable = 1'b1;
                            PC_mux_select = 2'b10;
                        end
                    end
                endcase
            end
        endcase
    end
end
```

Now, the PC control signals will only be emitted when the `processor_enable` signal is true.

## USER
Can we similarly add the processor_enable check to the ACC control signals?

```
always @(*) begin
  // Default values
  ACC_write_enable = 1'b0;
  ACC_mux_select = 2'b00;

  // Check if the current state is EXECUTE
  if (state_out == STATE_EXECUTE) begin
    // Immediate Data Manipulation Instructions
    if (instruction[7:4] == 4'b1110) begin // ADDI
      ACC_write_enable = 1'b1;
      ACC_mux_select = 2'b00; // ALU output
    end

    // Instructions with Variable-Data Operands
    case (instruction[7:5])
      3'b000: begin // LDA
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
      3'b001: ; // STA (No effect on ACC)
      3'b010,
      3'b011,
      3'b100,
      3'b101,
      3'b110: ACC_write_enable = 1'b1; // ADD, SUB, AND, OR, XOR
    endcase

    // Control and Branching Instructions
    case (instruction)
      8'b11110001: begin // JSR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b10; // PC
      end
    endcase

    // Data Manipulation Instructions
    case (instruction)
      8'b11110110,
      8'b11110111,
      8'b11111000,
      8'b11111001,
      8'b11111010,
      8'b11111100,
      8'b11111101,
      8'b11111110: ACC_write_enable = 1'b1; // SHL, SHR, SHL4, ROL, ROR, DEC, CLR, INV
      8'b11111011: begin // LDAR
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b01; // Memory contents
      end
    endcase

  end // if (state_out == STATE_EXECUTE)
end
```

## ASSISTANT
Sure, I can add the `processor_enable` check to the ACC control signals. I will add an `if (processor_enable)` condition around the entire state machine:

```verilog
always @(*) begin
  // Default values
  ACC_write_enable = 1'b0;
  ACC_mux_select = 2'b00;

  if (processor_enable) begin
    // Check if the current state is EXECUTE
    if (state_out == STATE_EXECUTE) begin
      // Immediate Data Manipulation Instructions
      if (instruction[7:4] == 4'b1110) begin // ADDI
        ACC_write_enable = 1'b1;
        ACC_mux_select = 2'b00; // ALU output
      end

      // Instructions with Variable-Data Operands
      case (instruction[7:5])
        3'b000: begin // LDA
          ACC_write_enable = 1'b1;
          ACC_mux_select = 2'b01; // Memory contents
        end
        3'b001: ; // STA (No effect on ACC)
        3'b010,
        3'b011,
        3'b100,
        3'b101,
        3'b110: ACC_write_enable = 1'b1; // ADD, SUB, AND, OR, XOR
      endcase

      // Control and Branching Instructions
      case (instruction)
        8'b11110001: begin // JSR
          ACC_write_enable = 1'b1;
          ACC_mux_select = 2'b10; // PC
        end
      endcase

      // Data Manipulation Instructions
      case (instruction)
        8'b11110110,
        8'b11110111,
        8'b11111000,
        8'b11111001,
        8'b11111010,
        8'b11111100,
        8'b11111101,
        8'b11111110: ACC_write_enable = 1'b1; // SHL, SHR, SHL4, ROL, ROR, DEC, CLR, INV
        8'b11111011: begin // LDAR
          ACC_write_enable = 1'b1;
          ACC_mux_select = 2'b01; // Memory contents
        end
      endcase

    end // if (state_out == STATE_EXECUTE)
  end // if (processor_enable)
end
```

Now, the ACC control signals will only be emitted when the `processor_enable` signal is true.

## USER
Can we similarly add the processor_enable check to the IR control signals?
```
always @(*) begin
    // Default: IR_load_enable is 0 (disabled)
    IR_load_enable = 1'b0;

    // During FETCH cycle, set IR_load_enable to 1 (enabled)
    if (state_out == STATE_FETCH) begin
        IR_load_enable = 1'b1;
    end
end
```

## ASSISTANT
Of course, I can add the `processor_enable` check to the IR control signals. I will wrap the entire state machine with an `if (processor_enable)` condition:

```verilog
always @(*) begin
    // Default: IR_load_enable is 0 (disabled)
    IR_load_enable = 1'b0;

    if (processor_enable) begin
        // During FETCH cycle, set IR_load_enable to 1 (enabled)
        if (state_out == STATE_FETCH) begin
            IR_load_enable = 1'b1;
        end
    end
end
```

Now, the IR control signals will only be emitted when the `processor_enable` signal is true.

## USER
Can we also add the processor_enable check for the memory signals?
```
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    if (state_out == STATE_EXECUTE) begin
        // Instructions with Variable-Data Operands
        case (instruction[7:5])
            3'b001: begin // STA
                Memory_write_enable = 1'b1; // Enable writing to memory
            end
        endcase

        // Data Manipulation Instructions
        case (instruction[7:0])
            8'b11111011: begin // LDAR
                Memory_address_mux_select = 2'b01; // Select ACC as memory address
            end
        endcase
    end

    // Instruction fetching
    if (state_out == STATE_FETCH) begin
        Memory_address_mux_select = 2'b10; // Select PC as memory address
    end
end
```

## ASSISTANT
Sure, I can add the `processor_enable` check for the memory signals. I will wrap the entire state machine with an `if (processor_enable)` condition:

```verilog
always @(*) begin
    // Default: Memory_write_enable is 0 (disabled)
    Memory_write_enable = 1'b0;

    // Default: Memory_address_mux_select is 00 (IR[4:0])
    Memory_address_mux_select = 2'b00;

    if (processor_enable) begin
        if (state_out == STATE_EXECUTE) begin
            // Instructions with Variable-Data Operands
            case (instruction[7:5])
                3'b001: begin // STA
                    Memory_write_enable = 1'b1; // Enable writing to memory
                end
            endcase

            // Data Manipulation Instructions
            case (instruction[7:0])
                8'b11111011: begin // LDAR
                    Memory_address_mux_select = 2'b01; // Select ACC as memory address
                end
            endcase
        end

        // Instruction fetching
        if (state_out == STATE_FETCH) begin
            Memory_address_mux_select = 2'b10; // Select PC as memory address
        end
    end
end
```

Now, the memory control signals will only be emitted when the `processor_enable` signal is true.