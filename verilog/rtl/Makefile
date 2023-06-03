# Makefile
# See https://docs.cocotb.org/en/stable/quickstart.html for more info

# defaults
SIM ?= icarus
TOPLEVEL_LANG ?= verilog

# TOPLEVEL is the name of the toplevel module in your Verilog or VHDL file
TOPLEVEL = qtcore_c1_4baddr_scan_test

run_test: test
	./a.out

# this is the only part you should need to modify:
VERILOG_SOURCES += $(PWD)/accumulator_microcontroller.v \
 $(PWD)/alu.v $(PWD)/shift_register.v \
 $(PWD)/memory_bank.v \
 $(PWD)/control_unit.v \
 $(PWD)/Control_Status_Registers.v

test:
	iverilog -s $(TOPLEVEL) $(VERILOG_SOURCES) $(PWD)/qtcore_c1_4baddr_scan_test.v

clean:
	rm -rf TEST_PASSES.txt
	rm -rf *.out

.PHONY: clean test run_test