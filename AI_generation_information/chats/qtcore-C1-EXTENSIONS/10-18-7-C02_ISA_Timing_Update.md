## USER
This is the ISA for an 8-bit accumulator based architecture:

**Instructions with Variable-Data Operands**

| Instruction | Description                                             | Opcode (4 bits) | Operand (4 bits) | Register Effects                          |
|-------------|---------------------------------------------------------|----------------|-----------------|-------------------------------------------|
| LDA         | Load Accumulator with memory contents                   | 0000           | Memory Offset   | ACC <- M[{Segment, Offset}]               |
| STA         | Store Accumulator to memory                             | 0001           | Memory Offset   | M[{Segment, Offset}] <- ACC               |
| ADD         | Add memory contents to Accumulator                      | 0010           | Memory Offset   | ACC <- ACC + M[{Segment, Offset}]         |
| SUB         | Subtract memory contents from Accumulator               | 0011           | Memory Offset   | ACC <- ACC - M[{Segment, Offset}]         |
| AND         | AND memory contents with Accumulator                    | 0100           | Memory Offset   | ACC <- ACC & M[{Segment, Offset}]         |
| OR          | OR memory contents with Accumulator                     | 0101           | Memory Offset   | ACC <- ACC \| M[{Segment, Offset}]        |
| XOR         | XOR memory contents with Accumulator                    | 0110           | Memory Offset   | ACC <- ACC ^ M[{Segment, Offset}]        |

**Immediate Data Manipulation Instructions**

| Instruction | Description                                   | Opcode (4 bits) | Immediate (4 bits) | Register Effects             |
|-------------|-----------------------------------------------|----------------|--------------------|------------------------------|
| ADDI        | Add immediate to Accumulator                  | 1000           | 4-bit Immediate    | ACC <- ACC + IMM             |
| LUI         | Load Upper Immediate to Accumulator           | 1001           | 4-bit Immediate    | ACC <- IMM << 4, ACC[0:3] <- 0 |
| SETSEG      | Set Segment Register to Immediate             | 1010           | 4-bit Immediate    | Segment <- IMM               |

**Control/Status Register Manipulation Instructions**

| Instruction | Description                                    | Opcode (8 bits) | Register Effects     |
|-------------|------------------------------------------------|----------------|----------------------|
| CSR         | Read from Control/Status Register to Accumulator| 10110RRR       | ACC <- CSR[RRR]      |
| CSW         | Write from Accumulator to Control/Status Register| 10111RRR       | CSR[RRR] <- ACC      |

Here is the list of the Control/Status Registers (CSRs):

| CSR Number (3 bits) | Register Name |
|---------------------|---------------|
| 000                 | SEGEXE_L      |
| 001                 | SEGEXE_H      |
| 010                 | IO_IN         |
| 011                 | IO_OUT        |
| 100                 | CNT_L         |
| 101                 | CNT_H         |
| 110                 | STATUS_CTRL   |
| 111                 | TEMP          |

- **SEGEXE_L** and **SEGEXE_H**: Lower and Higher 4 bits of the Segment Executable Address respectively.
- **IO_IN** and **IO_OUT**: Input/Output Registers for UART (or any general I/O device) operations.
- **CNT_L** and **CNT_H**: Lower and Higher 4 bits of a counter register.
- **STATUS_CTRL**: A control register to hold the status of different operations in the CPU.
- **TEMP**: A temporary register for general use.

**Fixed Control and Branching Instructions**

| Instruction  | Description                                     | Opcode (8 bits) | PC Behavior                                                                   |
|--------------|-------------------------------------------------|----------------|-------------------------------------------------------------------------------|
| JMP          | Jump to memory address                          | 11110000       | PC <- ACC (Load the PC with the address stored in the accumulator)            |
| JSR          | Jump to Subroutine (save address to ACC)        | 11110001       | ACC <- PC + 1, PC <- ACC (Save the next address in ACC, then jump to the address in ACC) |
| HLT          | Halt the processor until reset                  | 11111111       | Stop execution (PC does not change until a reset occurs)   

**Variable-Operand Branching Instructions**

| Instruction  | Description                                                 | Opcode (4 bits) | Operand (4 bits) | PC Behavior                                                                                               |
|--------------|-------------------------------------------------------------|----------------|-----------------|-----------------------------------------------------------------------------------------------------------|
| BEQ          | Branch if equal (branch if ACC == 0)                       | 1100           | Sign + Offset   | If ACC == 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1                      |
| BNE          | Branch if not equal (branch if ACC != 0)                   | 1101           | Sign + Offset   | If ACC != 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1                      |
| BRA          | Branch always                                               | 1110           | Sign + Offset   | PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1                                       |

**Data Manipulation Instructions**

| Instruction | Description                                              | Opcode (8 bits) | Register Effects          |
|-------------|----------------------------------------------------------|----------------|---------------------------|
| SHL         | Shift Accumulator left                                   | 11110110       | ACC <- ACC << 1           |
| SHR         | Shift Accumulator right                                  | 11110111       | ACC <- ACC >> 1           |
| ROL         | Rotate Accumulator left                                  | 11111000       | ACC <- (ACC << 1) OR (ACC >> 7) |
| ROR         | Rotate Accumulator right                                 | 11111001       | ACC <- (ACC >> 1) OR (ACC << 7) |
| LDAR        | Load Accumulator via indirect memory access (ACC as ptr) | 11111010       | ACC <- M[ACC]             |
| SETSEG_ACC  | Set Segment Register to lower 4 bits of Accumulator      | 11111011       | Segment <- ACC[3:0]       |
| DEC         | Decrement Accumulator                                    | 11111100       | ACC <- ACC - 1            |
| CLR         | Clear (Zero) Accumulator                                 | 11111101       | ACC <- 0                  |
| INV         | Invert (NOT) Accumulator                                 | 11111110       | ACC <- ~ACC               |

We previously had a multi-cycle implementation for a variation of this processor, but we have since changed the ISA to the above. Can you update the following timing instructions to suit the new ISA?

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

## ASSISTANT
The timing instructions should be updated to reflect the new ISA as follows:

**FETCH cycle (all instructions)**

1. IR <- M[PC]
2. PC <- PC + 1

**EXECUTE cycle**

For **Immediate Data Manipulation Instructions**:

* ADDI: ACC <- ACC + IMM
* LUI: ACC <- IMM << 4, ACC[0:3] <- 0
* SETSEG: Segment <- IMM

For **Instructions with Variable-Data Operands**:

* LDA: ACC <- M[{Segment, Offset}]
* STA: M[{Segment, Offset}] <- ACC
* ADD: ACC <- ACC + M[{Segment, Offset}]
* SUB: ACC <- ACC - M[{Segment, Offset}]
* AND: ACC <- ACC & M[{Segment, Offset}]
* OR:  ACC <- ACC | M[{Segment, Offset}]
* XOR: ACC <- ACC ^ M[{Segment, Offset}]

For **Control/Status Register Manipulation Instructions**:

* CSR: ACC <- CSR[RRR]
* CSW: CSR[RRR] <- ACC

For **Control and Branching Instructions**:

* JMP: PC <- ACC
* JSR: ACC <- PC + 1, PC <- ACC
* HLT: (No operation, processor halted)

For **Variable-Operand Branching Instructions**:

* BEQ: If ACC == 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1
* BNE: If ACC != 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1
* BRA: PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1

For **Data Manipulation Instructions**:

* SHL: ACC <- ACC << 1
* SHR: ACC <- ACC >> 1
* ROL: ACC <- (ACC << 1) OR (ACC >> 7)
* ROR: ACC <- (ACC >> 1) OR (ACC << 7)
* LDAR: ACC <- M[ACC]
* SETSEG_ACC: Segment <- ACC[3:0]
* DEC: ACC <- ACC - 1
* CLR: ACC <- 0
* INV: ACC <- ~ACC
