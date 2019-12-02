CC = gcc -g -Wall -Werror
SRC = main.c gui.c process.c usage.c sysinfo.c callbacks.c
BIN = "System Monitor"

system_monitor:
	$(CC) `pkg-config --cflags gtk+-3.0` -o $(BIN) $(SRC) `pkg-config --libs gtk+-3.0`
