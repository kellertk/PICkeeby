.PHONY: all clean configure

all:
	@cmake -B build && cmake --build build

configure:
	@cmake -B build

clean:
	rm -rf build out
