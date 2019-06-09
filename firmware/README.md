# Firmware

Keyboard port settings and port scan are in the `kladenets2.c` file in the functions `Ports_Init()` and `Keyboard_Scan()`, can customize the scan of any keyboard with any matrix.

The old version of the LUFA library is used, because the new one for some reason does not work in the controllers I have (Microsin162).

## Compile

	Compile to .hex file:

	$ make

## Flash

	Press the button on the controller and run the command:

	$ make dfu
