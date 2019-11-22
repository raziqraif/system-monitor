system_monitor:
	gcc `pkg-config --cflags gtk+-3.0` -o "System Monitor" main.c gui.c process.c `pkg-config --libs gtk+-3.0`
