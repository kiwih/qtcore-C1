# 09 - PYTHON ASSEMBLER

## USER (#restarts:2)
Here is the ISA for "qtcore-A1", an 8-bit accumulator-based RISC processor.

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

Can you write a simple Python script which can convert programs in assembly to an array of binary values which could be executed by this program? Assume the following:
1. Programs are presented in the format [address]: [mnemonic] [optional arguments]
2. There is a special meta-instruction called DATA, which is followed by a number. If this is used, just place that number at that address.
3. Programs are always provided as a complete listing of 30 bytes. If a program is less than 30 bytes, assume that there are DATA 0 commands for the remaining bytes to make up to 30.

In the assembled version of the program, emit it as the following format [address]: [binary value] for all 30 lines.

## ASSISTANT
Here's a simple Python script that assembles the given assembly code into an array of binary values according to the qtcore-A1 ISA:

```python
def assemble(asm_lines):
    opcode_dict = {
        'ADDI': '1110',
        'LDA': '000',
        'STA': '001',
        'ADD': '010',
        'SUB': '011',
        'AND': '100',
        'OR': '101',
        'XOR': '110',
        'JMP': '11110000',
        'JSR': '11110001',
        'BEQ_FWD': '11110010',
        'BEQ_BWD': '11110011',
        'BNE_FWD': '11110100',
        'BNE_BWD': '11110101',
        'HLT': '11111111',
        'SHL': '11110110',
        'SHR': '11110111',
        'SHL4': '11111000',
        'ROL': '11111001',
        'ROR': '11111010',
        'LDAR': '11111011',
        'DEC': '11111100',
        'CLR': '11111101',
        'INV': '11111110',
    }

    memory = ['00000000'] * 30

    for line in asm_lines:
        address, instruction = line.split(': ')
        address = int(address)
        tokens = instruction.split(' ')
        mnemonic = tokens[0]

        if mnemonic == 'DATA':
            memory[address] = format(int(tokens[1]), '08b')
        elif mnemonic in opcode_dict:
            opcode = opcode_dict[mnemonic]
            if len(tokens) > 1:
                operand = format(int(tokens[1]), '05b')
                memory[address] = opcode + operand
            else:
                memory[address] = opcode

    binary_program = [(i, mem) for i, mem in enumerate(memory)]
    return binary_program


# Example usage:
asm_program = [
    '0: LDA 10',
    '1: ADDI 2',
    '2: STA 11',
    '3: HLT',
    '4: DATA 0',
    '10: DATA 5',
    '11: DATA 0',
]
binary_program = assemble(asm_program)
for addr, value in binary_program:
    print(f'{addr}: {value}')
```

This script defines an `assemble` function that takes a list of assembly lines as input and returns the assembled binary program as a list of tuples containing the address and binary value. The example usage demonstrates how to use the `assemble` function and print the assembled binary program in the required format. Note that this script assumes that the provided assembly program is well-formed and error-free.

## USER (#restarts:1)
One issue - the ADDI instruction has a 4-bit operand, not a 5-bit one.

## ASSISTANT
Apologies for the oversight. I have corrected the script to handle the 4-bit operand for the ADDI instruction:

```python
def assemble(asm_lines):
    opcode_dict = {
        'ADDI': '1110',
        'LDA': '000',
        'STA': '001',
        'ADD': '010',
        'SUB': '011',
        'AND': '100',
        'OR': '101',
        'XOR': '110',
        'JMP': '11110000',
        'JSR': '11110001',
        'BEQ_FWD': '11110010',
        'BEQ_BWD': '11110011',
        'BNE_FWD': '11110100',
        'BNE_BWD': '11110101',
        'HLT': '11111111',
        'SHL': '11110110',
        'SHR': '11110111',
        'SHL4': '11111000',
        'ROL': '11111001',
        'ROR': '11111010',
        'LDAR': '11111011',
        'DEC': '11111100',
        'CLR': '11111101',
        'INV': '11111110',
    }

    memory = ['00000000'] * 30

    for line in asm_lines:
        address, instruction = line.split(': ')
        address = int(address)
        tokens = instruction.split(' ')
        mnemonic = tokens[0]

        if mnemonic == 'DATA':
            memory[address] = format(int(tokens[1]), '08b')
        elif mnemonic in opcode_dict:
            opcode = opcode_dict[mnemonic]
            if len(tokens) > 1:
                if mnemonic == 'ADDI':
                    operand = format(int(tokens[1]), '04b')
                    memory[address] = opcode + operand
                else:
                    operand = format(int(tokens[1]), '05b')
                    memory[address] = opcode + operand
            else:
                memory[address] = opcode

    binary_program = [(i, mem) for i, mem in enumerate(memory)]
    return binary_program


# Example usage:
asm_program = [
    '0: LDA 10',
    '1: ADDI 2',
    '2: STA 11',
    '3: HLT',
    '4: DATA 0',
    '10: DATA 5',
    '11: DATA 0',
]
binary_program = assemble(asm_program)
for addr, value in binary_program:
    print(f'{addr}: {value}')
```

Now, the script correctly handles the 4-bit operand for the ADDI instruction.

## USER (#restarts:1)
Great. Can you add one meta-instruction, NOP, which can be represented by ADDI with all zeros as the operand?

## ASSISTANT
Sure, I've added the NOP meta-instruction to the script. It will be represented by an ADDI instruction with a 4-bit zero operand:

```python
def assemble(asm_lines):
    opcode_dict = {
        'ADDI': '1110',
        'LDA': '000',
        'STA': '001',
        'ADD': '010',
        'SUB': '011',
        'AND': '100',
        'OR': '101',
        'XOR': '110',
        'JMP': '11110000',
        'JSR': '11110001',
        'BEQ_FWD': '11110010',
        'BEQ_BWD': '11110011',
        'BNE_FWD': '11110100',
        'BNE_BWD': '11110101',
        'HLT': '11111111',
        'SHL': '11110110',
        'SHR': '11110111',
        'SHL4': '11111000',
        'ROL': '11111001',
        'ROR': '11111010',
        'LDAR': '11111011',
        'DEC': '11111100',
        'CLR': '11111101',
        'INV': '11111110',
        'NOP': '11100000',
    }

    memory = ['00000000'] * 30

    for line in asm_lines:
        address, instruction = line.split(': ')
        address = int(address)
        tokens = instruction.split(' ')
        mnemonic = tokens[0]

        if mnemonic == 'DATA':
            memory[address] = format(int(tokens[1]), '08b')
        elif mnemonic in opcode_dict:
            opcode = opcode_dict[mnemonic]
            if len(tokens) > 1:
                if mnemonic == 'ADDI':
                    operand = format(int(tokens[1]), '04b')
                    memory[address] = opcode + operand
                else:
                    operand = format(int(tokens[1]), '05b')
                    memory[address] = opcode + operand
            else:
                memory[address] = opcode

    binary_program = [(i, mem) for i, mem in enumerate(memory)]
    return binary_program


# Example usage:
asm_program = [
    '0: LDA 10',
    '1: ADDI 2',
    '2: STA 11',
    '3: HLT',
    '4: NOP',
    '10: DATA 5',
    '11: DATA 0',
]
binary_program = assemble(asm_program)
for addr, value in binary_program:
    print(f'{addr}: {value}')
```

The script now includes the NOP meta-instruction, which is represented by an ADDI instruction with a 4-bit zero operand (11100000 in binary).