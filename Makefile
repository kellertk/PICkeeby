.PHONY: all clean configure flash

all:
	@cmake -B build && cmake --build build

configure:
	@cmake -B build

clean:
	rm -rf build out

flash:
	pkcmd-lx -w -f out/keeby.hex -p PIC16F716 -mpcs
