all: src/ut61b_cli.cc src/fs9922_dmm3.cc src/wch_ch9325.cc
	mkdir -p build
	g++ $^ -Wall -Wextra -pedantic -pipe -O2 -std=c++11 `pkg-config libusb-1.0 libudev --libs --cflags` -o build/ut61b_cli

