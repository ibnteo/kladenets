/*
* Project: Chord keyboard Kladenets-23
* Version: 0.94 (pre release)
* Date: 2019-06-27
* Author: Vladimir Romanovich <ibnteo@gmail.com>
* License: MIT
* https://github.com/ibnteo/kladenets
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <util/delay.h>
#include "Descriptors.h"
#include <LUFA/Drivers/USB/USB.h>

uint16_t Chords[2] = {0, 0};
// Ports_Init(), LEDs(), Keyboard_Scan()
#include "microsin162.h"
//#include "catboard2.h"
//#include "promicro.h"
//#include "wakizashi.h"

#define LAYER1 0
#define LAYER2 1

#define NAV_MODE 0
#define MOU_MODE 1

#define MACROS_BUFFER_SIZE	40
#define MACROS_BUFFER_MAX	20
uint8_t Macros_Buffer[MACROS_BUFFER_SIZE];
uint8_t Macros_Index = 0;

int8_t Mouse_X;
int8_t Mouse_Y;
int8_t Mouse_W;
uint8_t Mouse_Button;

#define MOU_ROUG	0
#define MOU_PREC	1
uint8_t Mou_Speed = MOU_ROUG;

bool Chord_Growing = true;
uint8_t Q_Mods = 0;
uint8_t Q_Nav = NAV_MODE;
uint8_t Layer_Current = LAYER1;

uint16_t Time_Tick = 0;
uint16_t Chord_Tick = 0;
uint16_t Chords_Last[2] = {0, 0};

// EEPROM Settings
#define LAYOUTS1	0
#define LAYOUTS2	1

uint8_t Layout_Mode = LAYOUTS2;
uint8_t EE_Layout_Mode EEMEM;

#define OS_LINUX	0
#define OS_WINDOWS	1
#define OS_MAC		2
#define OS_ANDROID	3

uint8_t OS_Mode = OS_LINUX;
uint8_t EE_OS_Mode EEMEM;

uint8_t Meta = HID_KEYBOARD_MODIFIER_LEFTCTRL;

uint8_t Settings_Side = 0;

void Settings_Read() {
	Layout_Mode = eeprom_read_byte(&EE_Layout_Mode);
	OS_Mode = eeprom_read_byte(&EE_OS_Mode);
	Meta = (OS_Mode == OS_MAC) ? HID_KEYBOARD_MODIFIER_LEFTGUI : HID_KEYBOARD_MODIFIER_LEFTCTRL;
}
void Settings_Write() {
	eeprom_write_byte(&EE_Layout_Mode, Layout_Mode);
	eeprom_write_byte(&EE_OS_Mode, OS_Mode);
}

void Layout_Switch() {
	if (OS_Mode == OS_WINDOWS) { // Ctrl+Shift
		Macros_Buffer[Macros_Index++] = 0;
		Macros_Buffer[Macros_Index++] = HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_LEFTCTRL;
	} else if (OS_Mode == OS_MAC || OS_Mode == OS_ANDROID) { // Cmd+Space || Win+Space
		Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
		Macros_Buffer[Macros_Index++] = HID_KEYBOARD_MODIFIER_LEFTGUI;
	} else { // Alt+Shift
		Macros_Buffer[Macros_Index++] = 0;
		Macros_Buffer[Macros_Index++] = HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_LEFTALT;
	}
}

/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_t)];

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(USB_MouseReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the keyboard HID
 *  interface within the device.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 0,
				.ReportINEndpoint             =
					{
						.Address              = KEYBOARD_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
			},
	};

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another. This is for the mouse HID
 *  interface within the device.
 */
USB_ClassInfo_HID_Device_t Mouse_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 1,
				.ReportINEndpoint             =
					{
						.Address              = MOUSE_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevMouseHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevMouseHIDReportBuffer),
			},
	};


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void Hardware_Setup()
{
	// Set for 16 MHz clock
	CLKPR = 0x80; CLKPR = 0;

	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	Ports_Init();
	LED_On();
	USB_Init();
	LED_Off();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    //LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    //LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);

	USB_Device_EnableSOFEvents();

	//LED_Switch(! ConfigSuccess);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
	HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
}

#define HID_KEYBOARD_RU_O			HID_KEYBOARD_SC_J
#define HID_KEYBOARD_RU_A			HID_KEYBOARD_SC_F
#define HID_KEYBOARD_RU_N			HID_KEYBOARD_SC_Y
#define HID_KEYBOARD_RU_S			HID_KEYBOARD_SC_C
#define HID_KEYBOARD_RU_L			HID_KEYBOARD_SC_K
#define HID_KEYBOARD_RU_E			HID_KEYBOARD_SC_T
#define HID_KEYBOARD_RU_I			HID_KEYBOARD_SC_B
#define HID_KEYBOARD_RU_D			HID_KEYBOARD_SC_L
#define HID_KEYBOARD_RU_P			HID_KEYBOARD_SC_G
#define HID_KEYBOARD_RU_T			HID_KEYBOARD_SC_N
#define HID_KEYBOARD_RU_R			HID_KEYBOARD_SC_H
#define HID_KEYBOARD_RU_H			HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
#define HID_KEYBOARD_RU_U			HID_KEYBOARD_SC_E
#define HID_KEYBOARD_RU_W			HID_KEYBOARD_SC_D
#define HID_KEYBOARD_RU_J			HID_KEYBOARD_SC_Q
#define HID_KEYBOARD_RU_V			HID_KEYBOARD_SC_M
#define HID_KEYBOARD_RU_B			HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
#define HID_KEYBOARD_RU_K			HID_KEYBOARD_SC_R
#define HID_KEYBOARD_RU_ZH			HID_KEYBOARD_SC_SEMICOLON_AND_COLON
#define HID_KEYBOARD_RU_C			HID_KEYBOARD_SC_W
#define HID_KEYBOARD_RU_M			HID_KEYBOARD_SC_V
#define HID_KEYBOARD_RU_X			HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
#define HID_KEYBOARD_RU_Z			HID_KEYBOARD_SC_P
#define HID_KEYBOARD_RU_F			HID_KEYBOARD_SC_A
#define HID_KEYBOARD_RU_G			HID_KEYBOARD_SC_U
#define HID_KEYBOARD_RU_Y			HID_KEYBOARD_SC_S
#define HID_KEYBOARD_RU_SOFT_SIGN	HID_KEYBOARD_SC_M
#define HID_KEYBOARD_RU_HARD_SIGN	HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE
#define HID_KEYBOARD_RU_YU			HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
#define HID_KEYBOARD_RU_YA			HID_KEYBOARD_SC_Z
#define HID_KEYBOARD_RU_YI			HID_KEYBOARD_SC_S
#define HID_KEYBOARD_RU_YO			HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
#define HID_KEYBOARD_RU_SH			HID_KEYBOARD_SC_I
#define HID_KEYBOARD_RU_SCH			HID_KEYBOARD_SC_O
#define HID_KEYBOARD_RU_CH			HID_KEYBOARD_SC_X
#define HID_KEYBOARD_RU_YE			HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE
#define HID_KEYBOARD_RU_DOT			HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK

// Vowels for the right hand, bits 11xx are replaced by bits 10xx
const uint8_t Layer_Vowels[22] PROGMEM = {
	HID_KEYBOARD_SC_I,						// 00 01
	HID_KEYBOARD_RU_I,						// 00 01	И
	HID_KEYBOARD_SC_O,						// 00 10
	HID_KEYBOARD_RU_O,						// 00 10	О
	HID_KEYBOARD_SC_A,						// 00 11
	HID_KEYBOARD_RU_A,						// 00 11	А
	HID_KEYBOARD_SC_SPACE,					// 01 00
	HID_KEYBOARD_SC_SPACE,					// 01 00	_
	HID_KEYBOARD_SC_Y,						// 01 01
	HID_KEYBOARD_RU_SOFT_SIGN,				// 01 01	Ь
	HID_KEYBOARD_SC_U,						// 01 10
	HID_KEYBOARD_RU_U,						// 01 10	У
	0,										// 01 11 ea
	HID_KEYBOARD_RU_YA,						// 01 11	Я
	HID_KEYBOARD_SC_E,						// 10 00 
	HID_KEYBOARD_RU_E,						// 10 00	Е
	0,										// 10 01 io
	HID_KEYBOARD_RU_YI,						// 10 01	Ы
	0,										// 10 10 ou
	HID_KEYBOARD_RU_YO,						// 10 10	Ё
	HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,	// 10 11 
	HID_KEYBOARD_RU_YU,						// 10 11	Ю
};

// Consonants for the left hand
const uint8_t Layer_Consonants[62] PROGMEM = {
	HID_KEYBOARD_SC_T,			// 0 0001
	HID_KEYBOARD_RU_T,			// 0 0001
	HID_KEYBOARD_SC_S,			// 0 0010
	HID_KEYBOARD_RU_S,			// 0 0010
	HID_KEYBOARD_SC_P,			// 0 0011
	HID_KEYBOARD_RU_P,			// 0 0011
	HID_KEYBOARD_SC_R,			// 0 0100
	HID_KEYBOARD_RU_R,			// 0 0100
	HID_KEYBOARD_SC_C,			// 0 0101
	HID_KEYBOARD_RU_K,			// 0 0101
	HID_KEYBOARD_SC_X,			// 0 0110
	HID_KEYBOARD_RU_X,			// 0 0110
	HID_KEYBOARD_SC_G,			// 0 0111
	HID_KEYBOARD_RU_G,			// 0 0111
	HID_KEYBOARD_SC_N,			// 0 1000
	HID_KEYBOARD_RU_N,			// 0 1000
	HID_KEYBOARD_SC_K,			// 0 1001
	HID_KEYBOARD_RU_CH,			// 0 1001
	HID_KEYBOARD_SC_L,			// 0 1010
	HID_KEYBOARD_RU_L,			// 0 1010
	HID_KEYBOARD_SC_Z,			// 0 1011
	HID_KEYBOARD_RU_Z,			// 0 1011
	HID_KEYBOARD_SC_D,			// 0 1100
	HID_KEYBOARD_RU_D,			// 0 1100
	HID_KEYBOARD_SC_J,			// 0 1101
	HID_KEYBOARD_RU_J,			// 0 1101
	HID_KEYBOARD_SC_B,			// 0 1110
	HID_KEYBOARD_RU_B,			// 0 1110
	HID_KEYBOARD_SC_M,			// 0 1111
	HID_KEYBOARD_RU_M,			// 0 1111
	HID_KEYBOARD_SC_H,			// 1 0000
	HID_KEYBOARD_RU_W,			// 1 0000
	HID_KEYBOARD_SC_W,			// 1 0001
	HID_KEYBOARD_RU_ZH,			// 1 0001
	HID_KEYBOARD_SC_Q,			// 1 0010
	HID_KEYBOARD_RU_C,			// 1 0010
	0,							// 1 0011 ()
	0,							// 1 0011 ()
	HID_KEYBOARD_SC_V,			// 1 0100
	HID_KEYBOARD_RU_SH,			// 1 0100
	HID_KEYBOARD_SC_LEFT_SHIFT,	// 1 0101
	HID_KEYBOARD_SC_LEFT_SHIFT,	// 1 0101
	0,							// 1 0110 ???
	HID_KEYBOARD_SC_3_AND_HASHMARK,	// 1 0110 №
	0,							// 1 0111 ???
	0,							// 1 0111 ???
	HID_KEYBOARD_SC_F,			// 1 1000
	HID_KEYBOARD_RU_SCH,		// 1 1000
	0,							// 1 1001 ???
	0,							// 1 1001 ???
	HID_KEYBOARD_SC_Y,			// 1 1010
	HID_KEYBOARD_RU_F,			// 1 1010
	0,							// 1 1011
	HID_KEYBOARD_RU_YE,			// 1 1011
	0,							// 1 1100 .,
	0,							// 1 1100 .,
	0,							// 1 1101 ???
	0,							// 1 1101 ???
	0,							// 1 1110 ???
	HID_KEYBOARD_RU_HARD_SIGN,	// 1 1110
	0,							// 1 1111 $#
	0,							// 1 1111 $#
};

// Navigation for the left hand
const uint8_t Layer_NavMou[63] PROGMEM = {
	0,								// 000001 Mou
	HID_KEYBOARD_SC_LEFT_ARROW,		// 000010
	HID_KEYBOARD_SC_HOME,			// 000011
	HID_KEYBOARD_SC_UP_ARROW,		// 000100
	HID_KEYBOARD_SC_RIGHT_CONTROL,	// 000101
	HID_KEYBOARD_SC_PAGE_UP,		// 000110
	0,								// 000111
	HID_KEYBOARD_SC_DOWN_ARROW,		// 001000
	HID_KEYBOARD_SC_PAGE_DOWN,		// 001001
	HID_KEYBOARD_SC_LEFT_CONTROL,	// 001010
	0,								// 001011
	HID_KEYBOARD_SC_TAB,			// 001100
	0,								// 001101
	0,								// 001110
	0,								// 001111
	HID_KEYBOARD_SC_DELETE,			// 010000
	HID_KEYBOARD_SC_RIGHT_GUI,		// 010001
	0,								// 010010
	0,								// 010011
	HID_KEYBOARD_SC_RIGHT_ALT,		// 010100
	HID_KEYBOARD_SC_LEFT_SHIFT,		// 010101
	0,								// 010110
	0,								// 010111
	HID_KEYBOARD_SC_INSERT,			// 011000
	0,								// 011001
	0,								// 011010
	0,								// 011011
	0,								// 011100
	0,								// 011101
	0,								// 011110
	0,								// 011111
	HID_KEYBOARD_SC_RIGHT_ARROW,	// 100000
	0,								// 100001
	HID_KEYBOARD_SC_LEFT_GUI,		// 100010
	0,								// 100011
	0,								// 100100
	0,								// 100101
	0,								// 100110
	0,								// 100111
	HID_KEYBOARD_SC_LEFT_ALT,		// 101000
	0,								// 101001
	HID_KEYBOARD_SC_LEFT_SHIFT,		// 101010
	0,								// 101011
	0,								// 101100
	0,								// 101101
	0,								// 101110
	0,								// 101111
	HID_KEYBOARD_SC_END,			// 110000
	0,								// 110001
	0,								// 110010
	0,								// 110011
	0,								// 110100
	0,								// 110101
	0,								// 110110
	0,								// 110111
	0,								// 111000
	0,								// 111001
	0,								// 111010
	0,								// 111011
	0,								// 111100
	0,								// 111101
	0,								// 111110
	0,								// 111111
};

const uint8_t Layer_Num[2][15] PROGMEM = {
	{
	HID_KEYBOARD_SC_1_AND_EXCLAMATION,			// 000001
	HID_KEYBOARD_SC_2_AND_AT,					// 000010
	HID_KEYBOARD_SC_3_AND_HASHMARK,				// 000011
	HID_KEYBOARD_SC_4_AND_DOLLAR,				// 000100
	HID_KEYBOARD_SC_5_AND_PERCENTAGE,			// 000101
	HID_KEYBOARD_SC_6_AND_CARET,				// 000110
	HID_KEYBOARD_SC_7_AND_AND_AMPERSAND,		// 000111
	HID_KEYBOARD_SC_8_AND_ASTERISK,				// 001000
	HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,	// 001001
	HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,	// 001010
	HID_KEYBOARD_SC_EQUAL_AND_PLUS,				// 001011
	HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,		// 001100
	HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,			// 001101  * (Shift+8)
	HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	// 001110
	HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,	// 001111
	},
	{
	HID_KEYBOARD_SC_KEYPAD_1_AND_END,			// 010001
	HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,	// 010010
	HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,		// 010011
	HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,	// 010100
	HID_KEYBOARD_SC_KEYPAD_5,					// 010101
	HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,	// 010110
	HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,			// 010111
	HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,		// 011000
	HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,		// 011001
	HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,		// 011010
	HID_KEYBOARD_SC_KEYPAD_PLUS,				// 011011
	HID_KEYBOARD_SC_KEYPAD_MINUS,				// 011100
	HID_KEYBOARD_SC_KEYPAD_ASTERISK,			// 011101
	HID_KEYBOARD_SC_KEYPAD_SLASH,				// 011110
	HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE,		// 011111
	}
};

const uint8_t Layer_Func[15] PROGMEM = {
	HID_KEYBOARD_SC_F1,				// 110001
	HID_KEYBOARD_SC_F2,				// 110010
	HID_KEYBOARD_SC_F3,				// 110011
	HID_KEYBOARD_SC_F4,				// 110100
	HID_KEYBOARD_SC_F5,				// 110101
	HID_KEYBOARD_SC_F6,				// 110110
	HID_KEYBOARD_SC_F7,				// 110111
	HID_KEYBOARD_SC_F8,				// 111000
	HID_KEYBOARD_SC_F9,				// 111001
	HID_KEYBOARD_SC_F10,			// 111010
	HID_KEYBOARD_SC_F11,			// 111011
	HID_KEYBOARD_SC_F12,			// 111100
	HID_KEYBOARD_SC_SCROLL_LOCK,	// 111101
	HID_KEYBOARD_SC_CAPS_LOCK,		// 111110
	HID_KEYBOARD_SC_NUM_LOCK,		// 111111
};

#define KM_MACROS		1
#define KM_SHIFT0		2
#define KM_SHIFT1		3
#define KM_LAY_SHIFT0	4
#define KM_LAY_SHIFT1	5

// Symbols ., () %# Lay1 Lay2
const uint8_t Layer_Sym[3][36] PROGMEM = {
	{
		HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,	KM_SHIFT0,			// 00 00 0 .
		HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	KM_SHIFT0,			// 00 00 0 .
		HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,	KM_SHIFT0,			// 00 00 1 ,
		HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	KM_SHIFT1,			// 00 00 1 ,
		HID_KEYBOARD_SC_SEMICOLON_AND_COLON,		KM_SHIFT1,			// 00 01 2 :
		HID_KEYBOARD_SC_6_AND_CARET,				KM_SHIFT1,			// 00 01 2 :
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,			KM_SHIFT1,			// 00 10 3 !
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,			KM_SHIFT1,			// 00 10 3 !
		HID_KEYBOARD_SC_SEMICOLON_AND_COLON,		KM_SHIFT0,			// 01 01 4 ; 
		HID_KEYBOARD_SC_4_AND_DOLLAR,				KM_SHIFT1,			// 01 01 4 ; 
		HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	KM_SHIFT1,			// 01 10 5 ?
		HID_KEYBOARD_SC_7_AND_AND_AMPERSAND,		KM_SHIFT1,			// 01 10 5 ?
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,		KM_SHIFT1,			// 11 00 6 "
		HID_KEYBOARD_SC_2_AND_AT,					KM_SHIFT1,			// 11 00 6 "
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,		KM_SHIFT0,			// 11 01 7 '
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,		KM_LAY_SHIFT0,		// 11 01 7 '
		HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,		KM_SHIFT0,			// 11 10 8 `
		HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,		KM_LAY_SHIFT0,		// 11 10 8 `
	},
	{
		HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,			KM_SHIFT1,		// 00 00 0 (
		HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,			KM_SHIFT1,		// 00 00 0 (
		HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,			KM_SHIFT1,		// 00 00 1 )
		HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,			KM_SHIFT1,		// 00 00 1 )
		HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,	KM_SHIFT0,		// 00 01 [
		HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,	KM_LAY_SHIFT0,	// 00 01 [
		HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,	KM_SHIFT0,		// 00 10 ]
		HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,	KM_LAY_SHIFT0,	// 00 10 ]
		HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,			KM_SHIFT1,		// 01 01 <
		HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,			KM_LAY_SHIFT1,	// 01 01 <
		HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,			KM_SHIFT1,		// 01 10 >
		HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,			KM_LAY_SHIFT1,	// 01 10 >
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,				KM_SHIFT1,		// 11 00 _
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,				KM_SHIFT1,		// 11 00 _
		HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,	KM_SHIFT1,		// 11 01 {
		HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE,	KM_LAY_SHIFT1,	// 11 01 {
		HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,	KM_SHIFT1,		// 11 10 }
		HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE,	KM_LAY_SHIFT1,	// 11 10 }
	},
	{
		HID_KEYBOARD_SC_4_AND_DOLLAR,		KM_SHIFT1,			// 00 00 0 $
		HID_KEYBOARD_SC_4_AND_DOLLAR,		KM_LAY_SHIFT1,		// 00 00 0 $
		HID_KEYBOARD_SC_3_AND_HASHMARK,		KM_SHIFT1,			// 00 00 1 #
		HID_KEYBOARD_SC_3_AND_HASHMARK,		KM_LAY_SHIFT1,		// 00 00 1 #
		HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,	KM_SHIFT1,		// 00 01 ~
		HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE,	KM_LAY_SHIFT1,	// 00 01 ~
		HID_KEYBOARD_SC_5_AND_PERCENTAGE,	KM_SHIFT1,			// 00 10 %
		HID_KEYBOARD_SC_5_AND_PERCENTAGE,	KM_LAY_SHIFT1,		// 00 10 %
		HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,	KM_SHIFT1,			// 01 01 |
		HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,	KM_LAY_SHIFT1,		// 01 01 |
		HID_KEYBOARD_SC_6_AND_CARET,		KM_SHIFT1,			// 01 10 ^
		HID_KEYBOARD_SC_6_AND_CARET, 		KM_LAY_SHIFT1,		// 01 10 ^
		HID_KEYBOARD_SC_2_AND_AT,			KM_SHIFT1,			// 11 00 @
		HID_KEYBOARD_SC_2_AND_AT,			KM_LAY_SHIFT1,		// 11 00 @
		HID_KEYBOARD_SC_BACKSLASH_AND_PIPE,	KM_SHIFT0,			// 11 01 /b
		HID_KEYBOARD_SC_BACKSLASH_AND_PIPE, KM_SHIFT0,			// 11 01 /b
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,		KM_SHIFT0,			// 11 10 =
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,		KM_SHIFT0,			// 11 10 =
	}
};

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		        uint8_t* const ReportID,
		        const uint8_t ReportType,
		        void* ReportData,
		        uint16_t* const ReportSize)
{

	if (Time_Tick > 0) {
		Time_Tick++;
		if (Time_Tick >= 500) {
			Time_Tick = 0;
		}
	}

	if (Macros_Index < (MACROS_BUFFER_SIZE - MACROS_BUFFER_MAX)) {


		uint8_t mods = Q_Mods;

		uint16_t chords[2] = {Chords[0], Chords[1]};
		Keyboard_Scan();

		if ((Chords[0] & 0x300) != 0x200 && (Chords[1] & 0x300) != 0x200) {
			Q_Mods = 0;
			Q_Nav = NAV_MODE;
			Mouse_Button = 0;
		}
		bool isRelease = Chords[0] < chords[0] || Chords[1] < chords[1];
		bool isPress = Chords[0] > chords[0] || Chords[1] > chords[1];
		bool isTick = Time_Tick > Chord_Tick && Chords[0] == Chords_Last[0] && Chords[1] == Chords_Last[1];
		if (isTick) {
			Time_Tick--;
		}
		for (uint8_t side=0; side<=1; side++) {
			uint16_t chord2 = chords[side];
			uint16_t chord21 = chords[side ? 0 : 1];
			if (isPress) {
				if (side) Chord_Growing = true;
				Time_Tick = 1;
			} else if (isRelease || isTick) {
				if (Chord_Growing) {

					if (Settings_Side && (Settings_Side - 1) == side) { // Settings
						Settings_Side = 0;
						uint8_t chord = chord2;
						if (! (chord2 & ~0x13F)) {
							if (chord & 0x3) {
								Layout_Mode = (chord & 0x3) - 1;
							}
							if (chord & 0x3C) {
								OS_Mode = ((chord & 0x3C) >> 2) - 1;
								Meta = (OS_Mode == OS_MAC) ? HID_KEYBOARD_MODIFIER_LEFTGUI : HID_KEYBOARD_MODIFIER_LEFTCTRL;
							}
							if ((chord2 & 0x100) && (chord2 & 0x37)) {
								Settings_Write();
							}
						}
						LED_Toggle();
					} else if ((Chords_Last[side] == 0x299 && chords[side] == 0x266) || (Chords_Last[side] == 0x266 && chords[side] == 0x299)) { // Settings
						Settings_Side = side + 1;
						LED_Toggle();
					} else if (chord2 == 0x200 || chord2 == 0x80) { // Layer change
						if (Layout_Mode == LAYOUTS2) {
							uint8_t newLayer = LAYER1;
							if (chord2 == 0x80) {
								newLayer = LAYER2;
							}
							if (Layer_Current != newLayer) {
								Layout_Switch();
								Layer_Current = newLayer;
								LED_Switch(newLayer == LAYER2);
							}
						}
					} else if ((chord2 & 0x300) == 0x200) { // Quasi
						uint8_t chord = chord2;
						if (chord == 0x80) { // Enter
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_ENTER;
							Macros_Buffer[Macros_Index++] = mods;
						} else if (chord == 0x40 || chord == 0x43) { // Backspace
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
							Macros_Buffer[Macros_Index++] = mods;
						} else if (chord == 0x83) { // Space
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = mods;
						} else if (chord == 0xC3) { // Enter Numpad
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_KEYPAD_ENTER;
							Macros_Buffer[Macros_Index++] = mods;
						} else if (chord == 0xC0) { // Escape
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_ESCAPE;
							Macros_Buffer[Macros_Index++] = mods;
						} else if ((chord & 0x40) && (chord & 0x1) && (chord & 0x14) && ! (chord & 0xAA)) { // Backspaces
							if (chord == 0x51) { // 2 Backspaces
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
							} else if (chord == 0x45) { // Ctrl+Backspace
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = Meta;
							} else if (chord == 0x55) { // 4 Backspaces
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_BACKSPACE;
								Macros_Buffer[Macros_Index++] = 0;
							}
						} else if ((chord & 0xC0) == 0xC0 && (chord & 0x3C) && (chord & 0x3) != 0x3) { // Func
							uint8_t chord1 = (chord & 0x3C) >> 2;
							uint8_t keyCode = pgm_read_byte(&Layer_Func[(chord1 & 0xF) - 1]);
							if (keyCode) {
								if (chord == 0xFD) { // Pause/Break
									keyCode = HID_KEYBOARD_SC_PAUSE;
								} else if (chord == 0xFE) { // Print Screen
									keyCode = HID_KEYBOARD_SC_PRINT_SCREEN;
								} else if ((chord & 0x3) == 0x1) { // Ctrl/Cmd+Func
									mods |= Meta;
									Q_Mods = 0;
								} else if ((chord & 0x3) == 0x2) { // Alt+Func
									mods |= HID_KEYBOARD_MODIFIER_LEFTALT;
									Q_Mods = 0;
								}
								Macros_Buffer[Macros_Index++] = keyCode;
								Macros_Buffer[Macros_Index++] = mods;
							}
						} else if ((chord & 0x3) == 0x3 && (chord & 0x3C)) { // Num
							uint8_t chord1 = (chord & 0x3C) >> 2;
							uint8_t numLayer = 0;
							if ((chord & 0xC0) == 0xC0) { // NumBlock
								numLayer = 1;
							} else if ((chord & 0xC0) == 0x40) { // Ctrl+Num
								if (! (mods & ~HID_KEYBOARD_MODIFIER_LEFTSHIFT)) {
									mods = Meta;
									Q_Mods = 0;
								}
							}
							uint8_t keyCode = pgm_read_byte(&Layer_Num[numLayer][(chord1 & 0xF) - 1]);
							if (keyCode) {
								if (! mods) {
									if (keyCode == HID_KEYBOARD_SC_EQUAL_AND_PLUS) { // = +
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									} else if (keyCode == HID_KEYBOARD_SC_BACKSLASH_AND_PIPE) { // * 8
										keyCode = HID_KEYBOARD_SC_8_AND_ASTERISK;
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									} else if (keyCode == HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK && Layer_Current == LAYER2) { // / Rus
										keyCode = HID_KEYBOARD_SC_BACKSLASH_AND_PIPE;
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									} else if (keyCode == HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN && Layer_Current == LAYER2) { // , Rus
										keyCode = HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK;
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									}
								}
								Macros_Buffer[Macros_Index++] = keyCode;
								Macros_Buffer[Macros_Index++] = mods;
							}
							if ((chord & 0xC0) == 0x80) { // +Space
								Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
								Macros_Buffer[Macros_Index++] = 0;
							}
						} else if ((chord & 0x3) && (chord & 0x3) != 0x3 && ! (chord & 0x3C) && (chord & 0xC0)) { // Mods+Vowels
							uint8_t keyCode;
							if ((chord & 0x3) == 0x2) { // Ctrl+[IOA]
								keyCode = pgm_read_byte(&Layer_Vowels[((chord & 0xC0) >> 5) - 2]);
							} else if (chord == 0x41) { // Ctrl+H
								keyCode = pgm_read_byte(&Layer_Consonants[(0b10000 - 1) << 1]);
							} else if (chord == 0xC1) { // Ctrl+U
								keyCode = pgm_read_byte(&Layer_Vowels[(0b0110 - 1) << 1]);
							} else if (chord == 0x81) { // Ctrl+E
								keyCode = pgm_read_byte(&Layer_Vowels[(0b1000 - 1) << 1]);
							}
							if (keyCode) {
								if (! (mods & ~HID_KEYBOARD_MODIFIER_LEFTSHIFT)) {
									mods = Meta;
								}
								Q_Mods = 0;
								Macros_Buffer[Macros_Index++] = keyCode;
								Macros_Buffer[Macros_Index++] = mods;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = mods;
							}
						} else if ((chord & 0xC0) && (chord & 0x3C)) { // Mods+Consonants
							uint8_t keyCode = pgm_read_byte(&Layer_Consonants[(((chord >> 2) & 0x1F) - 1) << 1]);
							if (! (chord & 0x3) && keyCode) {
								if (! (mods & ~HID_KEYBOARD_MODIFIER_LEFTSHIFT)) {
									mods = Meta;
								}
								Q_Mods = 0;
								Macros_Buffer[Macros_Index++] = keyCode;
								Macros_Buffer[Macros_Index++] = mods;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = mods;
							} else { // Mods+Symbols
								if (! (mods & ~HID_KEYBOARD_MODIFIER_LEFTSHIFT)) {
									mods = Meta;
								}
								Q_Mods = 0;
								if (chord == 0x70) { // Mod+,
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN;
									Macros_Buffer[Macros_Index++] = mods;
								} else if (chord == 0x71) { // Mod+;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SEMICOLON_AND_COLON;
									Macros_Buffer[Macros_Index++] = mods;
								} else if (chord == 0x4C) { // Mod+[
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE;
									Macros_Buffer[Macros_Index++] = mods;
								} else if (chord == 0x4D) { // Mod+]
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE;
									Macros_Buffer[Macros_Index++] = mods;
								} else if (chord == 0x7C) { // Mod+'
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE;
									Macros_Buffer[Macros_Index++] = mods;
								} else if (chord == 0x7D) { // Mod+`
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE;
									Macros_Buffer[Macros_Index++] = mods;
								}
							}
						} else if (Q_Nav == MOU_MODE) { // Mou
							uint8_t keyCode = pgm_read_byte(&Layer_NavMou[(chord & 0x3F) - 1]);
							int8_t step = isTick ? 2 : 20;
							if (Mou_Speed == MOU_PREC) step = 1;
							if (keyCode >= HID_KEYBOARD_SC_LEFT_CONTROL && keyCode <= HID_KEYBOARD_SC_RIGHT_GUI) {
								mods ^= 1 << (keyCode - HID_KEYBOARD_SC_LEFT_CONTROL);
								if (! chord21) Q_Mods = mods;
								Macros_Buffer[Macros_Index++] = 0;
								Macros_Buffer[Macros_Index++] = mods;
							} else if (chord == 0x1) {
								Q_Nav = NAV_MODE;
							} else if (chord == 0x10) {
								Mou_Speed = (Mou_Speed == MOU_PREC) ? MOU_ROUG : MOU_PREC;
							} else if ((chord == 0x2 && side) || (chord == 0x20 && ! side)) {
								Mouse_X = -step;
							} else if ((chord == 0x20 && side) || (chord == 0x2 && ! side)) {
								Mouse_X = step;
							} else if (chord == 0x4) {
								Mouse_Y = -step;
							} else if (chord == 0x8) {
								Mouse_Y = step;
							} else if (chord == 0x9) {
								Mouse_W = -1;
							} else if (chord == 0x6) {
								Mouse_W = 1;
							} else if (chord == 0x3) {
								Mouse_Button = Mouse_Button & 0x1 ? 0 : 0x1;
							} else if (chord == 0xC) {
								Mouse_Button = Mouse_Button & 0x4 ? 0 : 0x4;
							} else if (chord == 0x30) {
								Mouse_Button = Mouse_Button & 0x2 ? 0 : 0x2;
							}
						} else if (Q_Nav == NAV_MODE) { // Nav
							uint8_t keyCode = pgm_read_byte(&Layer_NavMou[(chord & 0x3F) - 1]);
							if (! keyCode) {
								if (chord == 0x1) {
									Q_Nav = MOU_MODE;
								} else if (chord == 0x1E) { // Ctrl+Alt
									mods = HID_KEYBOARD_MODIFIER_LEFTALT | HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = 0;
									Macros_Buffer[Macros_Index++] = mods;
									if (! chord21) Q_Mods = mods;
								} else if (chord == 0x2C) { // Alt+Tab
									mods = HID_KEYBOARD_MODIFIER_LEFTALT;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_TAB;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = mods;
								} else if (chord == 0x1C) { // Ctrl+Tab
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_TAB;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = mods;
								} else if (chord == 0xE) { // Ctrl+PgUp
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_PAGE_UP;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								} else if (chord == 0xD) { // Ctrl+PgDn
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_PAGE_DOWN;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								} else if (chord == 0x1A) { // Ctrl+Ins
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_INSERT;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								} else if (chord == 0x3A) { // Shift+Ins
									mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_INSERT;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								} else if (chord == 0x25) { // Ctrl+Del
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_DELETE;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								} else if (chord == 0x35) { // Shift+Del
									mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_DELETE;
									Macros_Buffer[Macros_Index++] = mods;
									Q_Mods = 0;
								}
							} else {
								if (keyCode >= HID_KEYBOARD_SC_LEFT_CONTROL && keyCode <= HID_KEYBOARD_SC_RIGHT_GUI) {
									mods ^= 1 << (keyCode - HID_KEYBOARD_SC_LEFT_CONTROL);
									if (! chord21) Q_Mods = mods;
								} else {
									if (side == 0 && keyCode == HID_KEYBOARD_SC_LEFT_ARROW) {
										keyCode = HID_KEYBOARD_SC_RIGHT_ARROW;
									} else if (side == 0 && keyCode == HID_KEYBOARD_SC_RIGHT_ARROW) {
										keyCode = HID_KEYBOARD_SC_LEFT_ARROW;
									} else if (side == 0 && keyCode == HID_KEYBOARD_SC_HOME) {
										keyCode = HID_KEYBOARD_SC_END;
									} else if (side == 0 && keyCode == HID_KEYBOARD_SC_END) {
										keyCode = HID_KEYBOARD_SC_HOME;
									}
									Macros_Buffer[Macros_Index++] = keyCode;
									Macros_Buffer[Macros_Index++] = mods;
								}
							}
						}
					} else {
						uint16_t isConsonants = chord2 & 0x07C;
						uint16_t isVowels = chord2 & 0x303;
						uint16_t isCShift = chord2 & 0x080;
						if (chord2 == 0xD4) { // Tab
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_TAB;
							Macros_Buffer[Macros_Index++] = mods;
						} else if (chord2 == 0x1D4) { // 4 Spaces
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
						} else if (chord2 == 0x180) { // 2 Spaces
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
							Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
							Macros_Buffer[Macros_Index++] = 0;
						} else { // Letters
							uint8_t modsV = 0;
							uint8_t modsC = 0;
							if (isConsonants) { // Consonants
								uint8_t chord1 = chord2;
								chord1 = (chord1 >> 2) & ~0x20;
								uint8_t keyCode = pgm_read_byte(&Layer_Consonants[Layer_Current ? (chord1 << 1) - 1 : (chord1 << 1) - 2]);
								if (keyCode) {
									if (keyCode == HID_KEYBOARD_SC_LEFT_SHIFT) {
										modsV = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									} else {
										if (isCShift && ! (chord2 & 0x203)) { // CShift
											modsC = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
										}
										if (keyCode == HID_KEYBOARD_SC_3_AND_HASHMARK) { // №
											mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
										}
										Macros_Buffer[Macros_Index++] = keyCode;
										Macros_Buffer[Macros_Index++] = mods | modsC;
									}
								} else { // Symbols
									uint8_t symLayer = 0;
									if (chord1 == 0x1C) {
										symLayer = 1;
									} else if (chord1 == 0x13) {
										symLayer = 2;
									} else if (chord1 == 0x1F) {
										symLayer = 3;
									}
									if (symLayer) {
										uint8_t sym = 0;
										const uint16_t chord12 = chord2 & 0x303;
										if ((chord12 == 0 || chord12 == 0x100) && isCShift) sym = 1;
										else if (chord12 == 0x1) sym = 2;
										else if (chord12 == 0x2) sym = 3;
										else if (chord12 == 0x101) sym = 4;
										else if (chord12 == 0x102) sym = 5;
										else if (chord12 == 0x300) sym = 6;
										else if (chord12 == 0x301) sym = 7;
										else if (chord12 == 0x302) sym = 8;
										sym = sym << 2; // * 4
										if (Layer_Current == LAYER2) {
											sym += 2;
										}
										uint8_t keyCode = pgm_read_byte(&Layer_Sym[symLayer - 1][sym]);
										uint8_t keyMode = pgm_read_byte(&Layer_Sym[symLayer - 1][sym + 1]);

										if (keyMode == KM_SHIFT0) {
											mods = 0;
											Macros_Buffer[Macros_Index++] = keyCode;
											Macros_Buffer[Macros_Index++] = mods;
										} else if (keyMode == KM_SHIFT1) {
											mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
											Macros_Buffer[Macros_Index++] = keyCode;
											Macros_Buffer[Macros_Index++] = mods;
										} else if (keyMode == KM_LAY_SHIFT0 || keyMode == KM_LAY_SHIFT1) {
											mods = 0;
											Macros_Buffer[Macros_Index++] = 0xFF;
											Macros_Buffer[Macros_Index++] = 0;

											Layout_Switch();

											Macros_Buffer[Macros_Index++] = 0xFF;
											Macros_Buffer[Macros_Index++] = 0;

											Macros_Buffer[Macros_Index++] = keyCode;
											Macros_Buffer[Macros_Index++] = keyMode == KM_LAY_SHIFT1 ? HID_KEYBOARD_MODIFIER_LEFTSHIFT : 0;

											Macros_Buffer[Macros_Index++] = 0xFF;
											Macros_Buffer[Macros_Index++] = 0;

											Layout_Switch();

											Macros_Buffer[Macros_Index++] = 0xFF;
											Macros_Buffer[Macros_Index++] = 0;
										} else if (keyMode == KM_MACROS) {
										}
										if (isVowels == 0x100 || (isCShift && isVowels)) {
											Macros_Buffer[Macros_Index++] = 0xFF;
											Macros_Buffer[Macros_Index++] = 0;
											Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
											Macros_Buffer[Macros_Index++] = 0;
										}
										isVowels = 0;
										isCShift = 0;
									}
								}
							}
							if (isVowels) { // Vowels
								uint8_t chord1 = (chord2 & 0x3) | ((chord2 & 0x300) >> 6);
								if ((chord1 & 0xC) == 0xC) {
									chord1 = chord1 & 0xB;
								}
								uint8_t keyCode = pgm_read_byte(&Layer_Vowels[Layer_Current ? (chord1 << 1) - 1 : (chord1 << 1) - 2]);
								if (! keyCode && Layer_Current == LAYER1) {
									if (chord1 == 0b00001010) { // ou
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_O;
										Macros_Buffer[Macros_Index++] = 0;
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_U;
										Macros_Buffer[Macros_Index++] = 0;
									} else if (chord1 == 0b00000111) { // ea
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_E;
										Macros_Buffer[Macros_Index++] = 0;
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_A;
										Macros_Buffer[Macros_Index++] = 0;
									} else if (chord1 == 0b00001001) { // io
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_I;
										Macros_Buffer[Macros_Index++] = 0;
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_O;
										Macros_Buffer[Macros_Index++] = 0;
									} else if (chord1 == 0b00001010) { // ae
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_A;
										Macros_Buffer[Macros_Index++] = 0;
										Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_E;
										Macros_Buffer[Macros_Index++] = 0;
									}
								}
								if (keyCode) {
									Macros_Buffer[Macros_Index++] = keyCode;
									Macros_Buffer[Macros_Index++] = mods | modsV;
								}
								if (isCShift) { // VSpace
									Macros_Buffer[Macros_Index++] = 0xFF;
									Macros_Buffer[Macros_Index++] = 0;
									Macros_Buffer[Macros_Index++] = HID_KEYBOARD_SC_SPACE;
									Macros_Buffer[Macros_Index++] = 0;
								}
							}
						}
					}
					if (side && isRelease) {
						Chord_Tick = Time_Tick + Time_Tick / 2;
						Chords_Last[0] = chords[0];
						Chords_Last[1] = chords[1];
						Time_Tick = 0;
					}
					if (side && ! isTick) Chord_Growing = false;
				}
			}
		}
	}

	/* Determine which interface must have its report generated */
	if (HIDInterfaceInfo == &Keyboard_HID_Interface) {
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

		if (Macros_Index) {
			if (Time_Tick >= Chord_Tick) {
				Time_Tick = Time_Tick - Time_Tick / 8;
			}
			KeyboardReport->KeyCode[0] = (Macros_Buffer[0] == 0xFF ? 0 : Macros_Buffer[0]);
			KeyboardReport->Modifier = Macros_Buffer[1];
			if (Macros_Buffer[0] == Macros_Buffer[2] && Macros_Buffer[2] != 0xFF) {
				Macros_Buffer[0] = 0xFF;
			} else {
				for (uint8_t i = 0; i <= (Macros_Index - 2); i+=2) {
					Macros_Buffer[i + 0] = Macros_Buffer[i + 2];
					Macros_Buffer[i + 1] = Macros_Buffer[i + 3];
					if (! Macros_Buffer[i + 0] && ! Macros_Buffer[i + 1]) break;
				}
				Macros_Buffer[MACROS_BUFFER_SIZE - 1] = 0;
				Macros_Buffer[MACROS_BUFFER_SIZE - 2] = 0;
				Macros_Index -= 2;
			}
		} else {
			KeyboardReport->Modifier = Q_Mods;
		}

		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		return true;
	} else {
		USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

		MouseReport->X = Mouse_X;
		MouseReport->Y = Mouse_Y;
		MouseReport->W = Mouse_W;
		Mouse_X = 0;
		Mouse_Y = 0;
		Mouse_W = 0;
		MouseReport->Button = Mouse_Button;

		*ReportSize = sizeof(USB_MouseReport_Data_t);
		return true;
	}
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		         const uint8_t ReportID,
		         const uint8_t ReportType,
		         const void* ReportData,
		         const uint16_t ReportSize)
{
	if (HIDInterfaceInfo == &Keyboard_HID_Interface)
	{
		uint8_t* LEDReport = (uint8_t*)ReportData;

		if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK) {
		  //LED_On();
		}

		if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK) {
		  //LED_On();
		}

		/*uint8_t  LEDMask   = LEDS_NO_LEDS;

		if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
		  LEDMask |= LEDS_LED1;

		LEDs_SetAllLEDs(LEDMask);*/
	}
}

int main(void) {
	Hardware_Setup();
	GlobalInterruptEnable();
	Settings_Read();
	while (true) {
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
	}
}
