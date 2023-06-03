; this program tests subroutines and jumping
; check correct operation by ensuring it halts and that at conclusion:
; MEM[3] should be 255
; MEM[4] should be 0
0: CLR
1: ADDI 5      ; set accumulator to 5
2: JSR          ; jump to address 5
3: HLT          ; stop the program

4: DATA 10    ; put the data in the middle of the program because we're cool that way

5: STA 0       ; store the return address to address 0
6: LDA 4       ; load value from address 4 to accumulator
7: BNE 3      ; if accumulator is not zero, skip to 10
8: LDA 0       ; load the return address from address 0
9: JMP          ; return
10: DEC          ; if we are here, accumulator is not zero, decrement it
11: STA 4       ; store the accumulator in address 4
12: CLR          ; reset the accumulator
13: ADDI 6      ; set it to 6
14: JMP          ; jump to 6