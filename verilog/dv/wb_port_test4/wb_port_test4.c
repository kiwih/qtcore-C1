/*
 * SPDX-FileCopyrightText: 2020 Efabless Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 */

// This include is relative to $CARAVEL_PATH (see Makefile)
#include <defs.h>
#include <stub.c>

#define reg_mprj_slave_ioscan (*(volatile uint32_t*)0x30000000)
#define reg_mprj_slave_ctrl (*(volatile uint32_t*)0x30000004)
/*
	Wishbone Test:
		- Configures MPRJ lower 8-IO pins as outputs
		- Checks counter value through the wishbone port
*/

const uint32_t program[67] = { //this program has a CLR instead of loading from IO_IN
    0x00000000,//MEM[255:252]
	0x00000000,//MEM[251:248]
	0x00000000,//MEM[247:244]
	0x00f0fd0f,//MEM[243:240]
	0x00000000,//MEM[239:236]
	0x00000000,//MEM[235:232]
	0x00000000,//MEM[231:228]
	0x00000000,//MEM[227:224]
	0x00000000,//MEM[223:220]
	0x00000000,//MEM[219:216]
	0x00000000,//MEM[215:212]
	0x00000000,//MEM[211:208]
	0x00000000,//MEM[207:204]
	0x00000000,//MEM[203:200]
	0x00000000,//MEM[199:196]
	0x00000000,//MEM[195:192]
	0x00000000,//MEM[191:188]
	0x00000000,//MEM[187:184]
	0x00000000,//MEM[183:180]
	0x00000000,//MEM[179:176]
	0x00000000,//MEM[175:172]
	0x00000000,//MEM[171:168]
	0x00000000,//MEM[167:164]
	0x00000000,//MEM[163:160]
	0x00000000,//MEM[159:156]
	0x00000000,//MEM[155:152]
	0x00000000,//MEM[151:148]
	0x00000000,//MEM[147:144]
	0x00000000,//MEM[143:140]
	0x00000000,//MEM[139:136]
	0x00000000,//MEM[135:132]
	0x00000000,//MEM[131:128]
	0x00000000,//MEM[127:124]
	0x00000000,//MEM[123:120]
	0x00000000,//MEM[119:116]
	0x00000000,//MEM[115:112]
	0x00000000,//MEM[111:108]
	0x00000000,//MEM[107:104]
	0x00000000,//MEM[103:100]
	0x00000000,//MEM[99:96]
	0x00000000,//MEM[95:92]
	0x00000000,//MEM[91:88]
	0x00000000,//MEM[87:84]
	0x00000000,//MEM[83:80]
	0x00000000,//MEM[79:76]
	0x00000000,//MEM[75:72]
	0x00000000,//MEM[71:68]
	0x00000000,//MEM[67:64]
	0x00000000,//MEM[63:60]
	0x00000000,//MEM[59:56]
	0x000000f0,//MEM[55:52]
	0x1f819ffd,//MEM[51:48]
	0x00000000,//MEM[47:44]
	0x00000000,//MEM[43:40]
	0x00000000,//MEM[39:36]
	0x00000000,//MEM[35:32]
	0x00000000,//MEM[31:28]
	0x00000000,//MEM[27:24]
	0xf0931db4,//MEM[23:20]
	0xb480be82,//MEM[19:16]
	0xbdbcfd1e,//MEM[15:12]
	0xfafdb9b8,//MEM[11:8]
	0x8ffd10fd,//MEM[7:4]
	0xbb00affd,//MEM[3:0]
	0b00000000000000000000000000000000, //TEMP, STATUS_CTRL, CNT_H, CNT_L
	0b00000000000000000000000011111111, //IO_OUT, IO_IN, SEG_EXE_H, SEG_EXE_L
	0b00000000000000000000000000000010, //ACC, IR, PC, SEG[4 bit], CU[3 bit]
};

uint32_t scan_in[67] = {0};

void main()
{

	/* 
	IO Control Registers
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 3-bits | 1-bit | 1-bit | 1-bit  | 1-bit  | 1-bit | 1-bit   | 1-bit   | 1-bit | 1-bit | 1-bit   |
	Output: 0000_0110_0000_1110  (0x1808) = GPIO_MODE_USER_STD_OUTPUT
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 110    | 0     | 0     | 0      | 0      | 0     | 0       | 1       | 0     | 0     | 0       |
	
	 
	Input: 0000_0001_0000_1111 (0x0402) = GPIO_MODE_USER_STD_INPUT_NOPULL
	| DM     | VTRIP | SLOW  | AN_POL | AN_SEL | AN_EN | MOD_SEL | INP_DIS | HOLDH | OEB_N | MGMT_EN |
	| 001    | 0     | 0     | 0      | 0      | 0     | 0       | 0       | 0     | 1     | 0       |
	*/

	// Configure lower 8-IOs as user output
	// Observe output value in the testbench
	reg_mprj_io_0 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_1 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_2 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_3 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_4 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_5 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_6 =  GPIO_MODE_USER_STD_OUTPUT;
	reg_mprj_io_7 =  GPIO_MODE_USER_STD_OUTPUT;

	/* Set up the housekeeping SPI to be connected internally so	*/
	/* that external pin changes don't affect it.			*/

    reg_spi_enable = 1;
    reg_wb_enable = 1;
	// reg_spimaster_config = 0xa002;	// Enable, prescaler = 2,
                                        // connect to housekeeping SPI

	// Connect the housekeeping SPI to the SPI master
	// so that the CSB line is not left floating.  This allows
	// all of the GPIO pins to be used for user functions.

    reg_mprj_io_31 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_30 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_29 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_28 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_27 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_26 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_25 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_24 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_23 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_22 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_21 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_20 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_19 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_18 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_17 = GPIO_MODE_MGMT_STD_OUTPUT;
    reg_mprj_io_16 = GPIO_MODE_MGMT_STD_OUTPUT;

     /* Apply configuration */
    reg_mprj_xfer = 1;
    while (reg_mprj_xfer == 1);

	reg_la2_oenb = reg_la2_iena = 0x00000000;    // [95:64]

    // Flag start of the test
	reg_mprj_datal = 0xAB600000;
    int i = 0;

    int cnt = 0;

    for(i = 0; i < 67; i++) {
        reg_mprj_slave_ioscan = program[i];
        if(reg_mprj_slave_ioscan != program[i]) {
            reg_mprj_datal = 0xAB6F0000; //early fail
            while(1);
        }
        reg_mprj_slave_ctrl = 0b01;
        //wait until slave is ready
        cnt = 0;
        while(reg_mprj_slave_ctrl & 0b11 != 0b00) {
            cnt++;
            if(cnt > 5) {
                reg_mprj_datal = 0xAB8F0000; //read-in timeout fail
                while(1);
            }
        }
    }

    reg_mprj_slave_ctrl = 0b10;
    //wait until slave halts
    while(reg_mprj_slave_ctrl & 0b100 == 0b000); //wait until the halt_out bit is set
    reg_mprj_datal = 0xAB620000; //halt detected flag
    reg_mprj_slave_ctrl = 0b00; //turn off the processor
	
	//in test bench, the I/O output will have been checked

    for(i = 0; i < 2; i++) {
        reg_mprj_datal = 0xF0000000 | (i << 16);
        reg_mprj_slave_ioscan = program[i];
        if(reg_mprj_slave_ioscan != program[i]) {
            reg_mprj_datal = 0xAB6F0000; //early fail
            while(1);
        }
        reg_mprj_slave_ctrl = 0b01;
        //wait until slave is ready
        cnt = 0;
        while(reg_mprj_slave_ctrl & 0b11 != 0b00) {
            cnt++;
            if(cnt > 10) {
                reg_mprj_datal = 0xAB9F0000; //read-out timeout fail
                while(1);
            }
        }
        //scan out the result
        scan_in[i] = reg_mprj_slave_ioscan;
    }

    if(scan_in[0] != 0xf1fd0200) {
        reg_mprj_datal = 0xAB7F0000; //readout fail
        while(1);
    }

    reg_mprj_datal = 0xAB610000; //finish

    while(1);
    //reg_mprj_datal = 0xAB610000;
    // if (reg_mprj_slave == 0x2B3D) {
    //     reg_mprj_datal = 0xAB610000;
    // }
}
