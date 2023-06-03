# 15 - SHIFT REGISTER BUG FIX

## USER
I have the following shift_register written in Verilog:
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

    reg [WIDTH-1:0] internal_data;

    // Shift register operation
    always @(posedge clk) begin
        if (rst) begin
            internal_data <= {WIDTH{1'b0}};
        end else if (enable) begin
            internal_data <= data_in;
        end else if (scan_enable) begin
            internal_data <= {internal_data[WIDTH-2:0], scan_in};
        end
    end

    // Output assignment
    assign data_out = internal_data;
    assign scan_out = internal_data[WIDTH-1];

endmodule
```
Unfortunately, it doesn't work if the WIDTH is set to 1. Can you fix it?

## ASSISTANT
Sure! The issue arises from the part where you define the concatenation of the internal_data register when `scan_enable` is high. When the WIDTH is set to 1, the range `internal_data[WIDTH-2:0]` will cause an out-of-bounds access to the internal_data register.

To fix this, you can use a conditional expression to handle the case where WIDTH is set to 1. Here's the updated code:

```verilog
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

    reg [WIDTH-1:0] internal_data;

    // Shift register operation
    always @(posedge clk) begin
        if (rst) begin
            internal_data <= {WIDTH{1'b0}};
        end else if (enable) begin
            internal_data <= data_in;
        end else if (scan_enable) begin
            if (WIDTH == 1) begin
                internal_data <= scan_in;
            end else begin
                internal_data <= {internal_data[WIDTH-2:0], scan_in};
            end
        end
    end

    // Output assignment
    assign data_out = internal_data;
    assign scan_out = internal_data[WIDTH-1];

endmodule
```

Now, the code will properly handle the case where WIDTH is set to 1.

## USER
Thanks! Can you also make it so scan_enable has a higher priority than the normal enable?

## ASSISTANT
Sure! To give the `scan_enable` signal a higher priority over the `enable` signal, you can change the order of the conditions in the always block. Here's the updated code:

```verilog
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

    reg [WIDTH-1:0] internal_data;

    // Shift register operation
    always @(posedge clk) begin
        if (rst) begin
            internal_data <= {WIDTH{1'b0}};
        end else if (scan_enable) begin
            if (WIDTH == 1) begin
                internal_data <= scan_in;
            end else begin
                internal_data <= {internal_data[WIDTH-2:0], scan_in};
            end
        end else if (enable) begin
            internal_data <= data_in;
        end
    end

    // Output assignment
    assign data_out = internal_data;
    assign scan_out = internal_data[WIDTH-1];

endmodule
```

Now, the `scan_enable` signal has a higher priority over the `enable` signal. When both signals are high, the shift register will operate in scan mode instead of the normal mode.