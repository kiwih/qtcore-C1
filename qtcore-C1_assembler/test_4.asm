; this program tests the IO registers via (CSR) and the interrupt emitter
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