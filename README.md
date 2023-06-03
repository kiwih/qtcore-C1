# QTCore-C1

*Author*: Hammond Pearce

*Description*: An accumulator-based 8-bit microarchitecture designed via GPT-4 conversations.

## How it works and design motivation

The QTCore-C1 is a comprehensive accumulator-based 8-bit microarchitecture. It is a Von Neumann design (shared data and instruction memory). 

Probably the most interesting thing about this design is that all functional Verilog beyond the top level wrapper was written by GPT-4, i.e. not a human! 
The author (Hammond Pearce) developed with GPT-4 first the ISA, then the processor, fully conversationally. Hammond wrote the test-benches to validate the design, and then had the
appropriate back-and-forth with GPT-4 to have it fix all bugs.
For your interest, we provide all conversation logs in the project repository under the `/AI_generation_information` directory.

This processor is based on the ideas undertaken in the [Chip-Chat](https://arxiv.org/abs/2305.13243) research project, where Dr. Pearce shephereded GPT-4 to design a basic processor, the QTCore-A1. This was a very constrained design as it needed to fit within the requirements of Tiny Tapeout 3, meaning it had a lot of spatial (less than 20 bytes of instruction/data memory!) and functionality constraints (basically no peripherals). 

The QTCore-C1 is a much more comprehensive design which was made for Efabless's first [AI generated Design Contest](https://efabless.com/ai-generated-design-contest). Targeted to Caravel, it has a lot more space available to it than with Tiny Tapeout. As such, it has a full 256 bytes of instruction and data memory, as well as 8-bit I/O ports, as an internal 16-bit timer, and even memory execution protection across 16-byte segments! Due to these expanded properties, it is a much more complex design with a lot more Verilog code as well as a much more comprehensive ISA, and the two processors, although sharing some similarities, are broadly incompatible.

*What could this be used for?* Practically, you could imagine a little co-processor like this being used for predictable-time I/O state machines, similar to the RP2040's PIO. It helps that this design is quite small, and could be easily replicated many times on a single die. It also has 8-bit I/O, an interrupt output, and a timer, which is quite useful for many applications.

*Why make this design instead of something like RISC-V?*: There are many implementations of open-source processors for ISAs like RISC-V and MIPS. The problem is, that means GPT-4 has seen these designs during training. For Efabless's generative AI contest (and the earlier Chip-Chat work), I didn't want to explore simply the capabilities of GPT-4 to emit data it has trained over - rather, I wanted to see how they performed when making something more novel. As such, I shephereded the models into making wholly new designs, with strange ISAs quite different to what is available in the open-source literature. 

**The QTCore-C1 architecture defines a processor with the following components:**

* Control Unit: 2-cycle FSM for driving the processor (3 bit one-hot encoded state register)
* Program Counter: 8-bit register containing the current address of the program
* Segment Register: 4-bit register containing the current segment used for data memory instructions
* Instruction Register: 8-bit register containing the current instruction to execute
* Accumulator: 8-bit register used for data storage, manipulation, and logic
* Memory Bank: 256 8-bit registers which store instructions and data. 
* Control and Status Registers: 8 8-bit registers which are used for special features including the timer, I/O, memory protection, and sending an interrupt to the larger Caravel processor.

In order to program and debug the processor, all registers are connected via one large scan chain which is interfaced with the internal wishbone bus.
As such, you use the Caravel processor to read and write the status of the processor.
You choose which you are using by asserting the appropriate chip select.
This makes it quite easy to interact with the processor.

The ISA description follows. 
For your convenience, we also provide an assembler in Python (also written by GPT-4) which makes it quite easy to write simple programs.

**Instructions with Variable-Data Operands**

1. LDA (Load Accumulator with memory contents)
   - Opcode: 0000MMMM
   - Registers: ACC <= M[{Segment, MMMM}]
2. STA (Store Accumulator to memory)
   - Opcode: 0001MMMM
   - Registers: M[{Segment, MMMM}] <= ACC
3. ADD (Add memory contents to Accumulator)
   - Opcode: 0010MMMM
   - Registers: ACC <= ACC + M[{Segment, MMMM}]
4. SUB (Subtract memory contents from Accumulator)
   - Opcode: 0011MMMM
   - Registers: ACC <= ACC - M[{Segment, MMMM}]
5. AND (AND memory contents with Accumulator)
   - Opcode: 0100MMMM
   - Registers: ACC <= ACC & M[{Segment, MMMM}]
6. OR (OR memory contents with Accumulator)
   - Opcode: 0101MMMM
   - Registers: ACC <= ACC \| M[{Segment, MMMM}]
7. XOR (XOR memory contents with Accumulator)
   - Opcode: 0110MMMM
   - Registers: ACC <= ACC ^ M[{Segment, MMMM}]

**Immediate Data Manipulation Instructions**

1. ADDI (Add immediate to Accumulator)
   - Opcode: 1000IIII
   - Registers: ACC <= ACC + IMM (Immediate value)
2. LUI (Load Upper Immediate to Accumulator)
   - Opcode: 1001IIII
   - Registers: ACC <= IMM << 4, ACC[0:3] <= 0 (Immediate value)
3. SETSEG (Set Segment Register to Immediate)
   - Opcode: 1010IIII
   - Registers: Segment <= IMM (Immediate value)

**Control/Status Register Manipulation Instructions**

1. CSR (Read from Control/Status Register to Accumulator)
   - Opcode: 10110RRR
   - Registers: ACC <= CSR[RRR]
2. CSW (Write from Accumulator to Control/Status Register)
   - Opcode: 10111RRR
   - Registers: CSR[RRR] <= ACC

**Fixed Control and Branching Instructions**

1. JMP (Jump to memory address)
   - Opcode: 11110000
   - PC Behavior: PC <- ACC (Load the PC with the address stored in the accumulator)
2. JSR (Jump to Subroutine)
   - Opcode: 11110001
   - PC Behavior: ACC <- PC + 1, PC <- ACC (Save the next address in ACC, then jump to the address in ACC)
3. HLT (Halt the processor until reset)
   - Opcode: 11111111
   - PC Behavior: Stop execution (PC does not change until a reset occurs)

**Variable-Operand Branching Instructions**

1. BEQ (Branch if equal - branch if ACC == 0)
   - Opcode: 1100SBBB
   - Operand: Sign + Offset
   - PC Behavior: If ACC == 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1
2. BNE (Branch if not equal - branch if ACC != 0)
   - Opcode: 1101SBBB
   - Operand: Sign + Offset
   - PC Behavior: If ACC != 0, then PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1
3. BRA (Branch always)
   - Opcode: 1110SBBB
   - Operand: Sign + Offset
   - PC Behavior: PC <- PC + Offset if Sign is 0, or PC <- PC - Offset if Sign is 1

**Data Manipulation Instructions**

1. SHL (Shift Accumulator left)
   - Opcode: 11110110
   - Register Effects: ACC <- ACC << 1
2. SHR (Shift Accumulator right)
   - Opcode: 11110111
   - Register Effects: ACC <- ACC >> 1
3. ROL (Rotate Accumulator left)
   - Opcode: 11111000
   - Register Effects: ACC <- (ACC << 1) OR (ACC >> 7)
4. ROR (Rotate Accumulator right)
   - Opcode: 11111001
   - Register Effects: ACC <- (ACC >> 1) OR (ACC << 7)
5. LDAR (Load Accumulator via indirect memory access, using ACC as a pointer)
   - Opcode: 11111010
   - Register Effects: ACC <- M[ACC]
6. SETSEG_ACC (Set Segment Register to the lower 4 bits of the Accumulator)
   - Opcode: 11111011
   - Register Effects: Segment <- ACC[3:0]
7. DEC (Decrement Accumulator)
   - Opcode: 11111100
   - Register Effects: ACC <- ACC - 1
8. CLR (Clear or Zero Accumulator)
   - Opcode: 11111101
   - Register Effects: ACC <- 0
9. INV (Invert or NOT Accumulator)
   - Opcode: 11111110
   - Register Effects: ACC <- ~ACC

## Multi-cycle timing

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

## List of registers and sizes

* Main registers

- **Program Counter (PC)**: 8 bits - Stores the address of the next instruction to be executed.
- **Accumulator (ACC)**: 8 bits - Stores the result of arithmetic and logical operations, as well as acts as a temporary storage for addresses during certain operations (e.g., JSR instruction).
- **Instruction Register (IR)**: 8 bits - Stores the current instruction being executed.
- **Segment Register (SEG)**: 4 bits - Handles the segment for memory operations.

* Control/Status Registers (CSRs) and their addresses

1. **SEGEXE_L (000)**: 8 bits - Represents the lower half of the memory segments that are designated as executable. Each bit in the register corresponds to a segment in the lower half of memory space. If a bit is set to 1, the corresponding segment is marked as executable.

2. **SEGEXE_H (001)**: 8 bits - Represents the higher half of the memory segments that are designated as executable. Each bit in the register corresponds to a segment in the upper half of memory space. If a bit is set to 1, the corresponding segment is marked as executable.

3. **IO_IN (010)**: 8 bits - Input register for UART (or any general I/O device) operations. This is used to read data from external devices.

4. **IO_OUT (011)**: 8 bits - Output register for UART (or any general I/O device) operations. This is used to write data to external devices.

5. **CNT_L (100)**: 8 bits - Lower 8 bits of a 16-bit counter register. This can be used to store the lower half of a count value, which could be used for timing operations, or looping in programming, etc.

6. **CNT_H (101)**: 8 bits - Higher 8 bits of a 16-bit counter register. This can be used to store the upper half of a count value, similar to the CNT_L register.

7. **STATUS_CTRL (110)**: 8 bits - A control register to hold the status of different operations in the CPU. This can contain flags for conditions such as zero result, negative result, overflow, etc.

8. **TEMP (111)**: 8 bits - A temporary register for general use. This could be used by the programmer for temporary storage during calculations, data manipulation, etc.

### Example programming using the assembler

Writing assembly programs for QTCore-C1 is simplified by the assembler produced by GPT-4. First, we define a additional meta-instructions:

### Meta-instructions:

- DATA: Define raw data to be loaded at the current address
- Operand (8 bits): 8-bit data value

### Presenting programs to the assembler:

1. Programs are presented in the format `[address]: [mnemonic] [optional operand]`
2. There is a special meta-instruction called DATA, which is followed by a number. If this is used, just place that number at that address.
3. Programs cannot exceed the size of the memory (in QTCore-C1, this is 256 bytes).
4. The memory contains both instructions and data.

### Example program:

In this program observe how we read and write to the I/O, the timer, and the data memory via SETSEG. We also set the memory segments to be executable via CSW, and then jump to a non-executable segment to crash the processor.

```
; this program tests the IO and counter registers via (CSR/CSW) and the interrupt emitter
; it should crash with a HALT after jumping to a non-executable data segment
; check output: program halted
; check output: IO_OUT = 15
; check output: M[240] = IO_IN
; check output: M[253] = 2
; check output: M[254] = 0b11111101
; check output: M[255] = 0xf1
0: CLR
1: SETSEG 15
2: LDA 0 ; loads value M[16*15 + 0 = 240] = 15
3: CSW 3 ; sets the value 15 to the CSR register 3 (IO_OUT)
4: CSR 2 ; reads from CSR register 2 (IO_IN) to ACC
5: STA 0 ; sends the IO_IN value to M[16*15 + 0 = 240]
6: CLR ;
7: ADDI 15;
8: CSW 0 ; sets SEGEXE_L to 0x0F, making just segments 0-3 executable
9: CSW 1 ; sets SEGEXE_H to 0x0F, making just segments 8-11 executable
10: CLR ;
11: LDAR ; loads the value of M[0] = 0b11111101 into ACC
12: STA 14; stores it in M[16*15 + 14 = 254]
13: CLR ;
14: CSW 4; set 0 to CNT_L
15: CSW 5; set 0 to CNT_H
16: ADDI 2; 
17: CSW 6; set the second bit of STATUS_CTRL (starts the counter)
18: ADDI 0; timer to value 1
19: CSR 4; reads the timer (value = 1), timer to value 2
20: CSR 4; reads the timer (value = 2), timer to value 3  
21: STA 13; stores it in M[16*15 + 13 = 253] (should be value 2)
22: LUI 3;
23: JMP ; jumps to address 48

48: CLR ;
49: LUI 15;
50: ADDI 1; make the constant 0xf1
51: STA 15; store it in M[16*15 + 15 = 255]
52: JMP; jumps to address 0xf1 (241)

240: DATA 15
241: CLR ; sets ACC to 0, but actually we should crash here for being non-executable
242: JMP 
```

## Verification

Two full-design testbenches are provided as `wb_port` and `wb_port_test4`. Additional functional testing is provided in `qtcore_c1_4baddr_scan_test` which is a scan-based testbench, run it with just `make` in the `/verilog/rtl` directory (it's not connected to the global caravel make scripts).


## Forked from the Caravel User Project

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![UPRJ_CI](https://github.com/efabless/caravel_project_example/actions/workflows/user_project_ci.yml/badge.svg)](https://github.com/efabless/caravel_project_example/actions/workflows/user_project_ci.yml) [![Caravel Build](https://github.com/efabless/caravel_project_example/actions/workflows/caravel_build.yml/badge.svg)](https://github.com/efabless/caravel_project_example/actions/workflows/caravel_build.yml)

| :exclamation: Important Note            |
|-----------------------------------------|

### Caravel information follows

Refer to [README](docs/source/index.rst#section-quickstart) for a quickstart of how to use caravel_user_project

Refer to [README](docs/source/index.rst) for this sample project documentation. 
