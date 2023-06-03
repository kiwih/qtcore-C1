# 14 - MEMORY MAPPED COMPONENTS

## USER
I have the following memory bank module written in Verilog:
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
        //data_out = mem_data_out[address];
    end

    // Scan chain output
    assign scan_out = mem_scan_out[MEM_SIZE];

endmodule
```
I want to customize it to add some memory-mapped I/O. Specifically, I want to do the following:
1. Add a 1-bit "btn_in" button input
2. Add a 7-bit "led_out" led output 
3. Have these go to the last shift register in the memory bank - perhaps by implementing the last shift register as two concatenated shift registers, one for the button (which always reads the status of the button input) and one which is for the leds. 
Can you update this for me?

## ASSISTANT
Sure, I've made the modifications to the Verilog code according to your requirements. Here's the updated code:

```
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 31, // Reduce memory size by 1 to accommodate IO
    parameter IO_ADDR = MEM_SIZE
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire btn_in, // Button input
    output wire [6:0] led_out // 7-bit LED output
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

    // IO shift registers
    wire [6:0] led_data_out;
    wire btn_data_out;
    shift_register #(
        .WIDTH(7)
    ) led_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(write_enable && (address == IO_ADDR)),
        .data_in(data_in[6:0]),
        .data_out(led_data_out),
        .scan_enable(scan_enable),
        .scan_in(mem_scan_out[MEM_SIZE-1]),
        .scan_out()
    );

    shift_register #(
        .WIDTH(1)
    ) btn_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(1'b0), // Always read the status of the button input
        .data_in(btn_in),
        .data_out(btn_data_out),
        .scan_enable(scan_enable),
        .scan_in(),
        .scan_out(scan_out)
    );

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
        if (address == IO_ADDR) begin
            data_out = {btn_data_out, led_data_out};
        end
    end

    // Assign LED output
    assign led_out = led_data_out;

endmodule
```

The changes I made are as follows:

1. I reduced `MEM_SIZE` by 1 to accommodate the memory-mapped I/O.
2. Added input "btn_in" and output "led_out" as requested.
3. Created two shift registers for the button and the LEDs at the last memory address `IO_ADDR`.
4. Updated the read operation to return the concatenated value of the button and LEDs when the address is `IO_ADDR`.
5. Assigned the `led

## USER
Looks good! Can you make the following changes for me:
1. declare a wire to take the scan_out from the led_shift_register to the scan_in of the btn_shift_register
2. Make the led_shift_register read from the upper 7 bits of data_in, not the lower bits
3. Make the data_out for IO_ADDR have the btn_data_out at the LSB, not the MSB

## ASSISTANT
Of course! I've made the changes according to your requests. Here's the updated Verilog code:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 31, // Reduce memory size by 1 to accommodate IO
    parameter IO_ADDR = MEM_SIZE
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire btn_in, // Button input
    output wire [6:0] led_out // 7-bit LED output
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

    // IO shift registers
    wire [6:0] led_data_out;
    wire btn_data_out;
    wire io_scan_out; // New wire to connect scan_out of led_shift_register to scan_in of btn_shift_register
    
    shift_register #(
        .WIDTH(7)
    ) led_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(write_enable && (address == IO_ADDR)),
        .data_in(data_in[7:1]), // Read from upper 7 bits of data_in
        .data_out(led_data_out),
        .scan_enable(scan_enable),
        .scan_in(mem_scan_out[MEM_SIZE-1]),
        .scan_out(io_scan_out) // Connect the new wire to the scan_out
    );

    shift_register #(
        .WIDTH(1)
    ) btn_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(1'b0), // Always read the status of the button input
        .data_in(btn_in),
        .data_out(btn_data_out),
        .scan_enable(scan_enable),
        .scan_in(io_scan_out), // Connect the new wire to the scan_in
        .scan_out(scan_out)
    );

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
        if (address == IO_ADDR) begin
            data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        end
    end

    // Assign LED output
    assign led_out = led_data_out;

endmodule
```

The changes made are:

1. Declared a new wire `io_scan_out` to connect the `scan_out` of

## USER
Looks very good - one last change, the btn_shift_register enable should be 1'b1 (i.e. enabled), it is active high not active low

## ASSISTANT
My apologies for the confusion. I've made the change to the `btn_shift_register` enable as requested. Here's the updated Verilog code:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 31, // Reduce memory size by 1 to accommodate IO
    parameter IO_ADDR = MEM_SIZE
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire btn_in, // Button input
    output wire [6:0] led_out // 7-bit LED output
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

    // IO shift registers
    wire [6:0] led_data_out;
    wire btn_data_out;
    wire io_scan_out; // New wire to connect scan_out of led_shift_register to scan_in of btn_shift_register
    
    shift_register #(
        .WIDTH(7)
    ) led_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(write_enable && (address == IO_ADDR)),
        .data_in(data_in[7:1]), // Read from upper 7 bits of data_in
        .data_out(led_data_out),
        .scan_enable(scan_enable),
        .scan_in(mem_scan_out[MEM_SIZE-1]),
        .scan_out(io_scan_out) // Connect the new wire to the scan_out
    );

    shift_register #(
        .WIDTH(1)
    ) btn_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(1'b1), // Enable the btn_shift_register, always read the status of the button input
        .data_in(btn_in),
        .data_out(btn_data_out),
        .scan_enable(scan_enable),
        .scan_in(io_scan_out), // Connect the new wire to the scan_in
        .scan_out(scan_out)
    );

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
        if (address == IO_ADDR) begin
            data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        end
    end

    // Assign LED output
    assign led_out = led_data_out;

endmodule
```

I've updated the `btn_shift_register` enable

## USER
One other change - can you reverse the order of the btn_shift_register and the led_shift_register in the scan chain, so that the btn_shift_register comes first and the led_shift_register is the one connected to the top-level module scan_out?

## ASSISTANT
Sure! I've updated the order of the `btn_shift_register` and `led_shift_register` in the scan chain as requested. Here's the updated Verilog code:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 31, // Reduce memory size by 1 to accommodate IO
    parameter IO_ADDR = MEM_SIZE
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire btn_in, // Button input
    output wire [6:0] led_out // 7-bit LED output
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

    // IO shift registers
    wire [6:0] led_data_out;
    wire btn_data_out;
    wire io_scan_out; // New wire to connect scan_out of btn_shift_register to scan_in of led_shift_register
    
    shift_register #(
        .WIDTH(1)
    ) btn_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(1'b1), // Enable the btn_shift_register, always read the status of the button input
        .data_in(btn_in),
        .data_out(btn_data_out),
        .scan_enable(scan_enable),
        .scan_in(mem_scan_out[MEM_SIZE-1]), // Connect the scan_in to the last memory cell scan_out
        .scan_out(io_scan_out) // Connect the new wire to the scan_out
    );

    shift_register #(
        .WIDTH(7)
    ) led_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(write_enable && (address == IO_ADDR)),
        .data_in(data_in[7:1]), // Read from upper 7 bits of data_in
        .data_out(led_data_out),
        .scan_enable(scan_enable),
        .scan_in(io_scan_out), // Connect the new wire to the scan_in
        .scan_out(scan_out) // Connect the scan_out to the top-level module
    );

    // Read operation
    integer idx;
    always @(*) begin
        data_out = {DATA_WIDTH{1'b0}};
        for (idx = 0; idx < MEM_SIZE; idx = idx + 1) begin
            if (address == idx) begin
                data_out = mem_data_out[idx];
            end
        end
        if (address == IO_ADDR) begin
            data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        end
    end
```
   

## USER
I think it would be more efficient if instead of using a for loop to index the mem_data_out array, you just indexed it directly. Can you rewrite just the //Read operation code?

## ASSISTANT
You're right. Directly indexing the `mem_data_out` array would be more efficient. I've updated the "// Read operation" section accordingly. Here's the updated Verilog code:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else if (address == IO_ADDR) begin
        data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
    end else begin
        data_out = {DATA_WIDTH{1'b0}};
    end
end
```

This code will now directly index the `mem_data_out` array when the `address` is within the memory size, assign the IO data when the `address` matches `IO_ADDR`, and set the `data_out` to all zeros for any other address.

## USER
Can we actually make it return "00000001" for all memory addresses outside the range?

## ASSISTANT
Certainly! I've updated the "// Read operation" section to return "00000001" for all memory addresses outside the valid range. Here's the updated Verilog code:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else if (address == IO_ADDR) begin
        data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
    end else begin
        data_out = 8'b00000001; // Return "00000001" for all memory addresses outside the range
    end
end
```

Now the `data_out` will be set to "00000001" for any address outside the valid memory range.

## USER
Can you re-write the whole module now? Remember to include the led_out signal.

## ASSISTANT
Sure, here is the entire updated `memory_bank` module with all the changes you've requested:

```verilog
module memory_bank #(
    parameter ADDR_WIDTH = 5,
    parameter DATA_WIDTH = 8,
    parameter MEM_SIZE = 31, // Reduce memory size by 1 to accommodate IO
    parameter IO_ADDR = MEM_SIZE
)(
    input wire clk,
    input wire rst,
    input wire [ADDR_WIDTH-1:0] address,
    input wire [DATA_WIDTH-1:0] data_in,
    input wire write_enable,
    output reg [DATA_WIDTH-1:0] data_out,
    input wire scan_enable,
    input wire scan_in,
    output wire scan_out,
    input wire btn_in, // Button input
    output wire [6:0] led_out // 7-bit LED output
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

    // IO shift registers
    wire [6:0] led_data_out;
    wire btn_data_out;
    wire io_scan_out; // New wire to connect scan_out of btn_shift_register to scan_in of led_shift_register
    
    shift_register #(
        .WIDTH(1)
    ) btn_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(1'b1), // Enable the btn_shift_register, always read the status of the button input
        .data_in(btn_in),
        .data_out(btn_data_out),
        .scan_enable(scan_enable),
        .scan_in(mem_scan_out[MEM_SIZE-1]), // Connect the scan_in to the last memory cell scan_out
        .scan_out(io_scan_out) // Connect the new wire to the scan_out
    );

    shift_register #(
        .WIDTH(7)
    ) led_shift_register (
        .clk(clk),
        .rst(rst),
        .enable(write_enable && (address == IO_ADDR)),
        .data_in(data_in[7:1]), // Read from upper 7 bits of data_in
        .data_out(led_data_out),
        .scan_enable(scan_enable),
        .scan_in(io_scan_out), // Connect the new wire to the scan_in
        .scan_out(scan_out) // Connect the scan_out to the top-level module
    );

    // Read operation
    always @(*) begin
        if (address < MEM_SIZE) begin
            data_out = mem_data_out[address];
        end else if (address == IO_ADDR) begin
            data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        end else begin
            data_out = 8'b00000001; // Return "00000001" for all memory addresses outside the range
        end
    end

    // Assign LED output
    assign led_out = led_data_out;

endmodule
```

This updated module now includes

# 17 - MEMORY MAPPED CONSTANTS (return to conversation 14)

## USER
I want to add a few more constant memory-mapped values in the //Read operation outputs.
At IO_ADDR + 1, can you return the value IO_ADDR + 2? This will be useful to load as an offset address later.

## ASSISTANT
Sure, I've added the requested functionality to the "// Read operation" section. When the address is `IO_ADDR + 1`, it will return `IO_ADDR + 2`. Here's the updated Verilog code:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else if (address == IO_ADDR) begin
        data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
    end else if (address == IO_ADDR + 1) begin
        data_out = IO_ADDR + 2; // Return IO_ADDR + 2 for address IO_ADDR + 1
    end else begin
        data_out = 8'b00000001; // Return "00000001" for all other memory addresses outside the range
    end
end
```

This updated "// Read operation" code will now return the value `IO_ADDR + 2` when the input address is `IO_ADDR + 1`.

## USER
Now I want to add, starting at IO_ADDR + 2, the cases for a 7segment display driver. These are the following values for each digit:
0:  segments = 7'b0111111;
1:  segments = 7'b0000110;
2:  segments = 7'b1011011;
3:  segments = 7'b1001111;
4:  segments = 7'b1100110;
5:  segments = 7'b1101101;
6:  segments = 7'b1111100;
7:  segments = 7'b0000111;
8:  segments = 7'b1111111;
9:  segments = 7'b1100111;

Put the digit 0 at IO_ADDR + 2 and increment appropriately.

## ASSISTANT
I've added the requested 7-segment display driver values starting at `IO_ADDR + 2`. Here's the updated "// Read operation" section with the added cases:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else if (address == IO_ADDR) begin
        data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
    end else if (address == IO_ADDR + 1) begin
        data_out = IO_ADDR + 2; // Return IO_ADDR + 2 for address IO_ADDR + 1
    end else case (address)
        IO_ADDR + 2: data_out = 7'b0111111; // 0
        IO_ADDR + 3: data_out = 7'b0000110; // 1
        IO_ADDR + 4: data_out = 7'b1011011; // 2
        IO_ADDR + 5: data_out = 7'b1001111; // 3
        IO_ADDR + 6: data_out = 7'b1100110; // 4
        IO_ADDR + 7: data_out = 7'b1101101; // 5
        IO_ADDR + 8: data_out = 7'b1111100; // 6
        IO_ADDR + 9: data_out = 7'b0000111; // 7
        IO_ADDR + 10: data_out = 7'b1111111; // 8
        IO_ADDR + 11: data_out = 7'b1100111; // 9
        default: data_out = 8'b00000001; // Return "00000001" for all other memory addresses outside the range
    endcase
end
```

This updated code will now return the appropriate 7-segment display values for digits 0-9 starting at `IO_ADDR + 2`.

## USER
I like that! Can you merge into the case statement the if statement for IO_ADDR and IO_ADDR + 1?

## ASSISTANT
Sure, I've merged the `if` statements for `IO_ADDR` and `IO_ADDR + 1` into the `case` statement. Here's the updated "// Read operation" section:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else case (address)
        IO_ADDR: data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        IO_ADDR + 1: data_out = IO_ADDR + 2; // Return IO_ADDR + 2 for address IO_ADDR + 1
        IO_ADDR + 2: data_out = 7'b0111111; // 0
        IO_ADDR + 3: data_out = 7'b0000110; // 1
        IO_ADDR + 4: data_out = 7'b1011011; // 2
        IO_ADDR + 5: data_out = 7'b1001111; // 3
        IO_ADDR + 6: data_out = 7'b1100110; // 4
        IO_ADDR + 7: data_out = 7'b1101101; // 5
        IO_ADDR + 8: data_out = 7'b1111100; // 6
        IO_ADDR + 9: data_out = 7'b0000111; // 7
        IO_ADDR + 10: data_out = 7'b1111111; // 8
        IO_ADDR + 11: data_out = 7'b1100111; // 9
        default: data_out = 8'b00000001; // Return "00000001" for all other memory addresses outside the range
    endcase
end
```

Now the "// Read operation" code is more concise, with all the conditions for addresses `IO_ADDR`, `IO_ADDR + 1`, and the 7-segment display values handled within the same `case` statement.

## USER
Last change - can all the 7 segment values be concatenated with a zero bit at the LSB?

## ASSISTANT
Sure, I've updated the 7-segment display values in the `case` statement to have a zero bit at the LSB. Here's the updated "// Read operation" section:

```verilog
// Read operation
always @(*) begin
    if (address < MEM_SIZE) begin
        data_out = mem_data_out[address];
    end else case (address)
        IO_ADDR: data_out = {led_data_out, btn_data_out}; // Place btn_data_out at the LSB
        IO_ADDR + 1: data_out = IO_ADDR + 2; // Return IO_ADDR + 2 for address IO_ADDR + 1
        IO_ADDR + 2: data_out = {7'b0111111, 1'b0}; // 0
        IO_ADDR + 3: data_out = {7'b0000110, 1'b0}; // 1
        IO_ADDR + 4: data_out = {7'b1011011, 1'b0}; // 2
        IO_ADDR + 5: data_out = {7'b1001111, 1'b0}; // 3
        IO_ADDR + 6: data_out = {7'b1100110, 1'b0}; // 4
        IO_ADDR + 7: data_out = {7'b1101101, 1'b0}; // 5
        IO_ADDR + 8: data_out = {7'b1111100, 1'b0}; // 6
        IO_ADDR + 9: data_out = {7'b0000111, 1'b0}; // 7
        IO_ADDR + 10: data_out = {7'b1111111, 1'b0}; // 8
        IO_ADDR + 11: data_out = {7'b1100111, 1'b0}; // 9
        default: data_out = 8'b00000001; // Return "00000001" for all other memory addresses outside the range
    endcase
end
```

Now, all the 7-segment display values have a zero bit at the LSB.