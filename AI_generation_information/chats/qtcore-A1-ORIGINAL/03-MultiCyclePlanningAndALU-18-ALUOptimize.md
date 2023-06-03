# 03 - MULTI-CYCLE PLANNING AND ALU

## USER (#restarts:1)
The full ISA specification for the 8-bit accumulator-based RISC processor.

**Immediate Data Manipulation Instructions**

| Instruction | Description                          | Opcode (4 bits) | Immediate (4 bits) | Register Effects  |
|-------------|--------------------------------------|----------------|--------------------|-------------------|
| ADDI        | Add immediate to Accumulator         | 1110           | 4-bit Immediate    | ACC <- ACC + IMM  |

**Instructions with Variable-Data Operands**

| Instruction | Description                                 | Opcode (3 bits) | Operand (5 bits) | Register Effects          |
|-------------|---------------------------------------------|----------------|-----------------|---------------------------|
| LDA         | Load Accumulator with memory contents       | 000            | Memory Address  | ACC <- M[Address]         |
| STA         | Store Accumulator to memory                 | 001            | Memory Address  | M[Address] <- ACC         |
| ADD         | Add memory contents to Accumulator          | 010            | Memory Address  | ACC <- ACC + M[Address]   |
| SUB         | Subtract memory contents from Accumulator   | 011            | Memory Address  | ACC <- ACC - M[Address]   |
| AND         | AND memory contents with Accumulator        | 100            | Memory Address  | ACC <- ACC & M[Address]   |
| OR          | OR memory contents with Accumulator         | 101            | Memory Address  | ACC <- ACC \| M[Address]  |
| XOR         | XOR memory contents with Accumulator        | 110            | Memory Address  | ACC <- ACC ^ M[Address]   |

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

| Instruction | Description                                              | Opcode (8 bits) | Register Effects          |
|-------------|----------------------------------------------------------|----------------|---------------------------|
| SHL         | Shift Accumulator left                                   | 11110110       | ACC <- ACC << 1           |
| SHR         | Shift Accumulator right                                  | 11110111       | ACC <- ACC >> 1           |
| SHL4        | Shift Accumulator left by 4 bits                         | 11111000       | ACC <- ACC << 4           |
| ROL         | Rotate Accumulator left                                  | 11111001       | ACC <- (ACC << 1) OR (ACC >> 7) |
| ROR         | Rotate Accumulator right                                 | 11111010       | ACC <- (ACC >> 1) OR (ACC << 7) |
| LDAR        | Load Accumulator via indirect memory access (ACC as ptr) | 11111011       | ACC <- M[ACC]             |
| DEC         | Decrement Accumulator                                    | 11111100       | ACC <- ACC - 1            |
| CLR         | Clear (Zero) Accumulator                                 | 11111101       | ACC <- 0                  |
| INV         | Invert (NOT) Accumulator                                 | 11111110       | ACC <- ~ACC               |

Internally, it will use the following registers:

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

I'd like us to start thinking about the datapath. In order to do this, we need to start breaking up operation of the ISA into steps, then identify how the steps should be implemented in the most efficient way possible. This processor will be implemented in just 1000 standard cells of an ASIC, so hardware utilization is the most important thing to be optimizing. 

Bearing this in mind, I believe that a multi-cycle implementation will be most efficient. Would you agree? If so, can you list what should happen to each register in each cycle for each of the instructions?

## ASSISTANT
Yes, I agree that a multi-cycle implementation would be most efficient for this processor, given the hardware utilization constraints. A multi-cycle datapath allows for better hardware sharing and reduces the complexity of the control logic, which should be beneficial in meeting the 1000 standard cell limit.

Here's a proposed breakdown of the register operations for each instruction in a multi-cycle datapath:

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
* OR:  ACC <- ACC | M[Address]
* XOR: ACC <- ACC ^ M[Address]

For **Control and Branching Instructions**:

* JMP: PC <- ACC
* JSR: ACC <- PC, PC <- ACC
* BEQ_FWD: If ACC == 0, then PC <- PC + 1
* BEQ_BWD: If ACC == 0, then PC <- PC - 1
* BNE_FWD: If ACC != 0, then PC <- PC + 1
* BNE_BWD: If ACC != 0, then PC <- PC - 1
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

Note that for some instructions, additional cycles may be required for memory access or complex operations. The control unit will need to be designed to generate the appropriate control signals for each stage of the multi-cycle datapath.

## USER
Can you double check the BEQ_BWD and BNE_BWD PC update math? It doesn't look right to me.

## ASSISTANT
Apologies for the oversight. You are correct that the PC update math for BEQ_BWD and BNE_BWD was not accurate. Here's the corrected version:

For **Control and Branching Instructions**:

* BEQ_FWD: If ACC == 0, then PC <- PC + 2
* BEQ_BWD: If ACC == 0, then PC <- PC - 2
* BNE_FWD: If ACC != 0, then PC <- PC + 2
* BNE_BWD: If ACC != 0, then PC <- PC - 2

In this corrected version, the PC is updated by 2 for both forward and backward branching instructions. This is because each instruction takes up 8 bits or 1 byte, and we need to jump 1 instruction forward or backward. Since the PC increments by 1 in the FETCH cycle, we need to add or subtract 2 to make a full jump.

## USER
But if PC is updated in the FETCH cycle, then the math is still off? Because in FETCH the PC would be PC+1, and then in execute then it might be +2, making it overall +3 which is wrong. Perhaps we could just update PC in the Execute stage? What else would that change in your list of instructions?

## ASSISTANT
You are correct, and I apologize for the confusion. We should indeed update the PC in the Execute stage for the branching instructions. This would ensure that we jump the correct number of instructions. Here's the updated list of instructions:

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
* OR:  ACC <- ACC | M[Address]
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

By updating the PC in the Execute stage for BEQ_FWD, BEQ_BWD, BNE_FWD, and BNE_BWD, we ensure the correct number of instruction jumps. Since the PC already increments by 1 in the FETCH cycle, we only need to add 1 for forward branches and subtract 3 for backward branches during the Execute stage.

## USER
That isn't quite was I was suggesting, but the results look good to me! Can you suggest an ALU component which could perform the needed operations for ACC? Write it in Verilog

## ASSISTANT
Sure! Here's a simple ALU component in Verilog that performs the necessary operations for the accumulator. This ALU takes two 8-bit inputs `A` and `B`, and an `opcode` input to select the operation. The output `Y` is an 8-bit signal that represents the result of the selected operation.

```verilog
module alu (
    input wire [7:0] A,
    input wire [7:0] B,
    input wire [2:0] opcode,
    output reg [7:0] Y
);

always @(*) begin
    case (opcode)
        3'b000: Y = A + B;          // ADD
        3'b001: Y = A - B;          // SUB
        3'b010: Y = A & B;          // AND
        3'b011: Y = A | B;          // OR
        3'b100: Y = A ^ B;          // XOR
        3'b101: Y = A << 1;         // SHL
        3'b110: Y = A >> 1;         // SHR
        default: Y = 8'b00000000;   // Default case (undefined operation)
    endcase
end

endmodule
```

This ALU can perform ADD, SUB, AND, OR, XOR, SHL, and SHR operations. However, note that the ALU doesn't handle other operations like ROL, ROR, SHL4, LDAR, DEC, CLR, and INV directly. These operations can be implemented outside the ALU or by extending the ALU with additional opcodes and logic as needed.

## USER
Makes sense to just have one comprehensive ALU, so can you add support for the missing instructions e.g. ROL/ROR etc?

## ASSISTANT
Certainly! Here's an updated ALU component in Verilog that includes support for all the necessary operations for the accumulator, including ROL, ROR, SHL4, DEC, CLR, and INV. The `opcode` input has been extended to 4 bits to accommodate the additional operations.

```verilog
module alu (
    input wire [7:0] A,
    input wire [7:0] B,
    input wire [3:0] opcode,
    output reg [7:0] Y
);

always @(*) begin
    case (opcode)
        4'b0000: Y = A + B;               // ADD
        4'b0001: Y = A - B;               // SUB
        4'b0010: Y = A & B;               // AND
        4'b0011: Y = A | B;               // OR
        4'b0100: Y = A ^ B;               // XOR
        4'b0101: Y = A << 1;              // SHL
        4'b0110: Y = A >> 1;              // SHR
        4'b0111: Y = A << 4;              // SHL4
        4'b1000: Y = {A[6:0], A[7]};      // ROL
        4'b1001: Y = {A[0], A[7:1]};      // ROR
        4'b1010: Y = A - 1;               // DEC
        4'b1011: Y = 8'b00000000;         // CLR
        4'b1100: Y = ~A;                  // INV
        default: Y = 8'b00000000;         // Default case (undefined operation)
    endcase
end

endmodule
```

With this comprehensive ALU, you can perform all the required operations for the accumulator using the appropriate opcode. The ALU can now handle ADD, SUB, AND, OR, XOR, SHL, SHR, SHL4, ROL, ROR, DEC, CLR, and INV operations.

## USER (#restarts:1)
This ALU is good. Can you print the ALU opcodes as a table?

## ASSISTANT
Here's the table of ALU opcodes and their corresponding operations:

| Opcode (4 bits) | Operation  | Description                          |
|-----------------|------------|--------------------------------------|
| 0000            | ADD        | Add: Y = A + B                       |
| 0001            | SUB        | Subtract: Y = A - B                  |
| 0010            | AND        | Bitwise AND: Y = A & B               |
| 0011            | OR         | Bitwise OR: Y = A \| B               |
| 0100            | XOR        | Bitwise XOR: Y = A ^ B               |
| 0101            | SHL        | Shift Left: Y = A << 1               |
| 0110            | SHR        | Shift Right: Y = A >> 1              |
| 0111            | SHL4       | Shift Left by 4: Y = A << 4          |
| 1000            | ROL        | Rotate Left: Y = {A[6:0], A[7]}      |
| 1001            | ROR        | Rotate Right: Y = {A[0], A[7:1]}     |
| 1010            | DEC        | Decrement: Y = A - 1                 |
| 1011            | CLR        | Clear: Y = 0                         |
| 1100            | INV        | Invert: Y = ~A                       |
| Others          | Undefined  | Default case (undefined operation)   |

These opcodes and their corresponding operations are implemented in the comprehensive ALU provided earlier.

## USER
For efficiency, can we swap the CLR and INV opcodes? That way, 11XX can be used for CLR. Re-write the Verilog too?

## ASSISTANT
Sure! Here's the updated table of ALU opcodes with CLR and INV swapped, and the corresponding Verilog code for the ALU:

| Opcode (4 bits) | Operation  | Description                          |
|-----------------|------------|--------------------------------------|
| 0000            | ADD        | Add: Y = A + B                       |
| 0001            | SUB        | Subtract: Y = A - B                  |
| 0010            | AND        | Bitwise AND: Y = A & B               |
| 0011            | OR         | Bitwise OR: Y = A \| B               |
| 0100            | XOR        | Bitwise XOR: Y = A ^ B               |
| 0101            | SHL        | Shift Left: Y = A << 1               |
| 0110            | SHR        | Shift Right: Y = A >> 1              |
| 0111            | SHL4       | Shift Left by 4: Y = A << 4          |
| 1000            | ROL        | Rotate Left: Y = {A[6:0], A[7]}      |
| 1001            | ROR        | Rotate Right: Y = {A[0], A[7:1]}     |
| 1010            | DEC        | Decrement: Y = A - 1                 |
| 1011            | INV        | Invert: Y = ~A                       |
| 1100            | CLR        | Clear: Y = 0                         |
| Others          | Undefined  | Default case (undefined operation)   |

Here's the updated ALU component in Verilog with the CLR and INV opcodes swapped:

```verilog
module alu (
    input wire [7:0] A,
    input wire [7:0] B,
    input wire [3:0] opcode,
    output reg [7:0] Y
);

always @(*) begin
    case (opcode)
        4'b0000: Y = A + B;               // ADD
        4'b0001: Y = A - B;               // SUB
        4'b0010: Y = A & B;               // AND
        4'b0011: Y = A | B;               // OR
        4'b0100: Y = A ^ B;               // XOR
        4'b0101: Y = A << 1;              // SHL
        4'b0110: Y = A >> 1;              // SHR
        4'b0111: Y = A << 4;              // SHL4
        4'b1000: Y = {A[6:0], A[7]};      // ROL
        4'b1001: Y = {A[0], A[7:1]};      // ROR
        4'b1010: Y = A - 1;               // DEC
        4'b1011: Y = ~A;                  // INV
        4'b1100: Y = 8'b00000000;         // CLR
        default: Y = 8'b00000000;         // Default case (undefined operation)
    endcase
end

endmodule
```

With this change, any opcode starting with `11` will be interpreted as a CLR operation, which should help improve efficiency.

# 18 - ALU OPTIMIZE (return to conversation 03)

## USER
To optimize this further, can you combine the default case and 4'b11 so the default case is CLR?

## ASSISTANT
Absolutely, by making the default case for the ALU the CLR operation, we can save some complexity in the implementation. This way, any opcode not specifically defined will be treated as a CLR operation. Here's the updated Verilog module:

```verilog
module alu (
    input wire [7:0] A,
    input wire [7:0] B,
    input wire [3:0] opcode,
    output reg [7:0] Y
);

always @(*) begin
    case (opcode)
        4'b0000: Y = A + B;               // ADD
        4'b0001: Y = A - B;               // SUB
        4'b0010: Y = A & B;               // AND
        4'b0011: Y = A | B;               // OR
        4'b0100: Y = A ^ B;               // XOR
        4'b0101: Y = A << 1;              // SHL
        4'b0110: Y = A >> 1;              // SHR
        4'b0111: Y = A << 4;              // SHL4
        4'b1000: Y = {A[6:0], A[7]};      // ROL
        4'b1001: Y = {A[0], A[7:1]};      // ROR
        4'b1010: Y = A - 1;               // DEC
        4'b1011: Y = ~A;                  // INV
        default: Y = 8'b00000000;         // CLR, including 4'b1100, 4'b1101, 4'b1110, 4'b1111
    endcase
end

endmodule
```

Now, if the opcode does not match any defined operations, the ALU will default to the CLR operation, which sets Y to zero. This includes any opcode starting with `11` (4'b1100, 4'b1101, 4'b1110, 4'b1111), which were previously not explicitly defined.