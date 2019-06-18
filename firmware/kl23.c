/*
* Project: Chord keyboard Kladenets-23
* Version: 0.5 (beta)
* Date: 2019-06-18
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
#include "microsin162.c" // Ports_Init(), LEDs(), Keyboard_Scan()
//#include "catboard2.c" // Ports_Init(), LEDs(), Keyboard_Scan()

#define LAYER1 0
#define LAYER2 1

#define NAV_MODE 0
#define MOU_MODE 1

#define MACROS_BUFFER_SIZE 20
uint8_t Macros_Buffer[MACROS_BUFFER_SIZE];
uint8_t Buffer_Last = 0;

bool Chord_Growing[2] = {true, true};
uint8_t Q_Mods = 0;
uint8_t Q_Nav = NAV_MODE;
uint8_t Layer_Current = LAYER1;

// EEPROM Settings
#define LAYOUTS_ENG 0
#define LAYOUTS_ENG_RUS 1

#define OS_LINUX 0
#define OS_WINDOWS 1
#define OS_MAC 2

uint8_t Layout_Mode = LAYOUTS_ENG;
uint8_t EE_Layout_Mode EEMEM;

uint8_t OS_Mode = OS_LINUX;
uint8_t EE_OS_Mode EEMEM;

void Settings_Get() {
	OS_Mode = eeprom_read_byte(&EE_OS_Mode);
	Layout_Mode = eeprom_read_byte(&EE_Layout_Mode);
}
void Settings_Set() {
	eeprom_write_byte(&EE_OS_Mode, OS_Mode);
	eeprom_write_byte(&EE_Layout_Mode, Layout_Mode);
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

	LED_Switch(! ConfigSuccess);
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

#define HID_KEYBOARD_FF	0xFF

// Vowels for the right hand, bits 11xx are replaced by bits 10xx
static uint8_t Layer_Vowels[2][11] = {
	{
		HID_KEYBOARD_SC_I,						// 00 01
		HID_KEYBOARD_SC_O,						// 00 10
		HID_KEYBOARD_SC_A,						// 00 11
		HID_KEYBOARD_SC_SPACE,					// 01 00
		HID_KEYBOARD_SC_Y,						// 01 01
		HID_KEYBOARD_SC_U,						// 01 10
		0,										// 01 11 ea
		HID_KEYBOARD_SC_E,						// 10 00 
		0,										// 10 01 io
		0,										// 10 10 ou
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,	// 10 11 
	},
	{
		HID_KEYBOARD_RU_I,			// 00 01
		HID_KEYBOARD_RU_O,			// 00 10
		HID_KEYBOARD_RU_A,			// 00 11
		HID_KEYBOARD_SC_SPACE,		// 01 00
		HID_KEYBOARD_RU_YI,			// 01 01
		HID_KEYBOARD_RU_U,			// 01 10
		HID_KEYBOARD_RU_YA,			// 01 11
		HID_KEYBOARD_RU_E,			// 10 00
		HID_KEYBOARD_RU_SOFT_SIGN,	// 10 01
		HID_KEYBOARD_RU_YO,			// 10 10
		HID_KEYBOARD_RU_YU,			// 10 11
	}
};

// Consonants for the left hand
static uint8_t Layer_Consonants[2][31] = {
	{
		HID_KEYBOARD_SC_T,							// 0 0001
		HID_KEYBOARD_SC_S,							// 0 0010
		HID_KEYBOARD_SC_P,							// 0 0011
		HID_KEYBOARD_SC_R,							// 0 0100
		HID_KEYBOARD_SC_C,							// 0 0101
		HID_KEYBOARD_SC_X,							// 0 0110
		HID_KEYBOARD_SC_G,							// 0 0111
		HID_KEYBOARD_SC_N,							// 0 1000
		HID_KEYBOARD_SC_K,							// 0 1001
		HID_KEYBOARD_SC_L,							// 0 1010
		HID_KEYBOARD_SC_Z,							// 0 1011
		HID_KEYBOARD_SC_D,							// 0 1100
		HID_KEYBOARD_SC_J,							// 0 1101
		HID_KEYBOARD_SC_B,							// 0 1110
		HID_KEYBOARD_SC_M,							// 0 1111
		HID_KEYBOARD_SC_H,							// 1 0000
		HID_KEYBOARD_SC_Q,							// 1 0001
		HID_KEYBOARD_SC_W,							// 1 0010
		0,											// 1 0011 ???
		HID_KEYBOARD_SC_V,							// 1 0100
		HID_KEYBOARD_SC_LEFT_SHIFT,					// 1 0101
		0,											// 1 0110 ()
		0,											// 1 0111 ???
		HID_KEYBOARD_SC_F,							// 1 1000
		0,											// 1 1001 %#
		HID_KEYBOARD_SC_Y,							// 1 1010
		0,											// 1 1011 ???
		0,											// 1 1100 ???
		0,											// 1 1101 ???
		0,											// 1 1110 ???
		0,											// 1 1111 .,
	},
	{
		HID_KEYBOARD_RU_T,							// 0 0001
		HID_KEYBOARD_RU_S,							// 0 0010
		HID_KEYBOARD_RU_P,							// 0 0011
		HID_KEYBOARD_RU_R,							// 0 0100
		HID_KEYBOARD_RU_K,							// 0 0101
		HID_KEYBOARD_RU_X,							// 0 0110
		HID_KEYBOARD_RU_G,							// 0 0111
		HID_KEYBOARD_RU_N,							// 0 1000
		HID_KEYBOARD_RU_CH,							// 0 1001
		HID_KEYBOARD_RU_L,							// 0 1010
		HID_KEYBOARD_RU_Z,							// 0 1011
		HID_KEYBOARD_RU_D,							// 0 1100
		HID_KEYBOARD_RU_J,							// 0 1101
		HID_KEYBOARD_RU_B,							// 0 1110
		HID_KEYBOARD_RU_M,							// 0 1111
		HID_KEYBOARD_RU_W,							// 1 0000
		HID_KEYBOARD_RU_ZH,							// 1 0001
		HID_KEYBOARD_RU_C,							// 1 0010
		0,											// 1 0011 ???
		HID_KEYBOARD_RU_SH,							// 1 0100
		HID_KEYBOARD_SC_LEFT_SHIFT,					// 1 0101
		0,											// 1 0110 ()
		0,											// 1 0111 ???
		HID_KEYBOARD_RU_SCH,						// 1 1000
		0,											// 1 1001 %№
		HID_KEYBOARD_RU_F,							// 1 1010
		HID_KEYBOARD_RU_YE,							// 1 1011
		0,											// 1 1100 ???
		0,											// 1 1101 ???
		HID_KEYBOARD_RU_HARD_SIGN,					// 1 1110
		0,											// 1 1111 .,
	}
};

// Symbols ., () %# symCode and isShift
static uint8_t Layer_Sym[3][24] = {
	{
		HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,	0, // 00 00 .
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,			1, // 00 01 !
		HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	1, // 00 10 ?
		HID_KEYBOARD_SC_SEMICOLON_AND_COLON,		1, // 00 11 :
		HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN,	0, // 01 00 ,
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,		0, // 01 01 -
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,		0, // 01 10 '
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,		1, // 01 11 _
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,		1, // 10 00 "
		HID_KEYBOARD_SC_SEMICOLON_AND_COLON,		0, // 10 01 ;
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,				1, // 10 10 +
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,				0, // 10 11 =
	},
	{
		0,	0, // 00 00 (
		0,	0, // 00 01 [
		0,	0, // 00 10 {
		0,	0, // 00 11 ]
		0,	0, // 01 00 )
		0,	0, // 01 01 <
		0,	0, // 01 10 }
		0,	0, // 01 11 >
		0,	0, // 10 00 /
		0,	0, // 10 01 \
		0,	0, // 10 10 |
		0,	0, // 10 11 *
	},
	{
		0,	0, // 00 00 %
		0,	0, // 00 01 `
		0,	0, // 00 10 ~
		0,	0, // 00 11 @
		0,	0, // 01 00 #
		0,	0, // 01 01 $
		0,	0, // 01 10 ^
		0,	0, // 01 11 &
		0,	0, // 10 00 
		0,	0, // 10 01 
		0,	0, // 10 10 
		0,	0, // 10 11 
	}
};

// Navigation for the left hand
static uint8_t Layer_NavMou[63] = {
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

static uint8_t Layer_Num[48] = {
	HID_KEYBOARD_SC_BACKSPACE,		// 010000 Backspace
	HID_KEYBOARD_SC_F4,				// 010001
	HID_KEYBOARD_SC_F8,				// 010010
	HID_KEYBOARD_SC_F12,			// 010011
	HID_KEYBOARD_SC_F1,				// 010100
	HID_KEYBOARD_SC_F5,				// 010101
	HID_KEYBOARD_SC_F9,				// 010110
	HID_KEYBOARD_SC_CAPS_LOCK,		// 010111
	HID_KEYBOARD_SC_F2,				// 011000
	HID_KEYBOARD_SC_F6,				// 011001
	HID_KEYBOARD_SC_F10,			// 011010
	HID_KEYBOARD_SC_SCROLL_LOCK,	// 011011
	HID_KEYBOARD_SC_F3,				// 011100
	HID_KEYBOARD_SC_F7,				// 011101
	HID_KEYBOARD_SC_F11,			// 011110
	HID_KEYBOARD_SC_NUM_LOCK,		// 011111

	HID_KEYBOARD_SC_ENTER,						// 100000 Enter
	HID_KEYBOARD_SC_4_AND_DOLLAR,				// 100001
	HID_KEYBOARD_SC_8_AND_ASTERISK,				// 100010
	HID_KEYBOARD_SC_EQUAL_AND_PLUS,				// 100011
	HID_KEYBOARD_SC_1_AND_EXCLAMATION,			// 100100
	HID_KEYBOARD_SC_5_AND_PERCENTAGE,			// 100101
	HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,	// 100110
	HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK,	// 100111
	HID_KEYBOARD_SC_2_AND_AT,					// 101000
	HID_KEYBOARD_SC_6_AND_CARET,				// 101001
	HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,	// 101010
	0,											// 101011 * (Shift+8)
	HID_KEYBOARD_SC_3_AND_HASHMARK,				// 101100
	HID_KEYBOARD_SC_7_AND_AND_AMPERSAND,		// 101101
	HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,		// 101110
	HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,	// 101111

	HID_KEYBOARD_SC_ESCAPE,						// 110000 Escape
	HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,	// 110001
	HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,		// 110010
	HID_KEYBOARD_SC_KEYPAD_PLUS,				// 110011
	HID_KEYBOARD_SC_KEYPAD_1_AND_END,			// 110100
	HID_KEYBOARD_SC_KEYPAD_5,					// 110101
	HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,		// 110110
	HID_KEYBOARD_SC_KEYPAD_SLASH,				// 110111
	HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,	// 111000
	HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,	// 111001
	HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,		// 111010
	HID_KEYBOARD_SC_KEYPAD_ASTERISK,			// 111011
	HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,		// 111100
	HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,			// 111101
	HID_KEYBOARD_SC_KEYPAD_MINUS,				// 111110
	HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE		// 111111
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
	/* Determine which interface must have its report generated */
	if (HIDInterfaceInfo == &Keyboard_HID_Interface) {
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

		uint8_t usedKeyCodes = 0;
		uint8_t mods = Q_Mods;

		if (Macros_Buffer[0]) {
			KeyboardReport->Modifier |= mods;
			if (Macros_Buffer[0] != Buffer_Last) {
				KeyboardReport->KeyCode[usedKeyCodes++] = Macros_Buffer[0];
				Buffer_Last = Macros_Buffer[0];
				for (uint8_t i = 0; i < (MACROS_BUFFER_SIZE - 2); i++) {
					Macros_Buffer[i] = Macros_Buffer[i+1];
					if (! Macros_Buffer[i]) break;
				}
				Macros_Buffer[MACROS_BUFFER_SIZE - 1] = 0;
			} else {
				Buffer_Last = 0;
			}
		} else {

			bool setShift = false;
			bool clearShift = false;

			uint16_t chords[2] = {Chords[0], Chords[1]};
			Keyboard_Scan();

			for (uint8_t side=0; side<=1; side++) {
				uint16_t chord2 = chords[side]; 
				if (Chords[side] < chord2) {
					if ((Chords[side] & 0x300) != 0x200) {
						Q_Mods = 0;
					}
					if (Chord_Growing[side]) {
						Chord_Growing[side] = false;
						if (chord2 == 0x200 || chord2 == 0x80) { // Layer change
						//if (chord2 == 0x200 || chord2 == 0x40) { // Layer change
							uint8_t newLayer = LAYER1;
							if (chord2 == 0x80) {
							//if (chord2 == 0x40) {
								newLayer = LAYER2;
							}
							if (Layer_Current != newLayer) {
								if (OS_Mode == OS_WINDOWS) { // Ctrl+Shift
									mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_LEFTCTRL;
								} else if (OS_Mode == OS_MAC) { // Cmd+Space
									mods = HID_KEYBOARD_MODIFIER_LEFTGUI;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
								} else { // Alt+Shift
									mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_LEFTALT;
								}
								Layer_Current = newLayer;
								LED_Switch(newLayer == LAYER2);
							}
						} else if ((chord2 & 0x300) == 0x200) { // Quasi
							uint8_t chord = chord2;
							if ((chord & 0x41) == 0x41 && ! (chord & 0xAA)) { // Backspaces
								if (chord == 0x41) {
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
								} else if (chord == 0x45) {
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
								} else if (chord == 0x51) {
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
								} else if (chord == 0x55) {
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
									KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_BACKSPACE;
								}
							} else if ((chord & 0xC0) && ! (chord & 0x03)) { // Num
								chord = (chord & 0xFC) >> 2;
								uint8_t keyCode = Layer_Num[(chord & 0x3F) - 0x10];
								if (! keyCode) {
									if (chord == 0b00101011) { // * (Shift+8)
										keyCode = HID_KEYBOARD_SC_8_AND_ASTERISK;
										setShift = true;
										KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
									}
								} else {
									if (keyCode == HID_KEYBOARD_SC_EQUAL_AND_PLUS) { // = +
										setShift = true;
									} else if (keyCode == HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN && Layer_Current == LAYER2) { // , Rus
										keyCode = HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK;
										setShift = true;
									}
									KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
								}
							/*} else if ((chord & 0x3) && ! (chord & 0x3C) && (chord & 0x80)) { // Mods+Vowels
								uint8_t keyCode;
								if ((chord & 0xC0) == 0x80) {
									keyCode = Layer_Vowels[0][(chord & 0x3) - 1];
								} else if (chord == 0xC3) {
									keyCode = Layer_Vowels[0][0x8 - 1];
								} else if ((chord & 0xC0) == 0xC0) {
									keyCode = Layer_Vowels[0][((chord & 0x3) | 0x4) - 1];
								}
								if (keyCode) {
									if (! (mods & HID_KEYBOARD_MODIFIER_LEFTSHIFT & HID_KEYBOARD_MODIFIER_RIGHTSHIFT)) {
										mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									}
									KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
								}
							} else if ((chord & 0x3) == 0x3 && ! (chord & 0x80) && (chord & 0xC7)) { // Mods+Consonants
								uint8_t keyCode = Layer_Consonants[0][((chord >> 2) & 0x1F) - 1];
								if (keyCode) {
									if (! (mods & HID_KEYBOARD_MODIFIER_LEFTSHIFT & HID_KEYBOARD_MODIFIER_RIGHTSHIFT)) {
										mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
									}
									KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
								}*/
							} else if (Q_Nav == NAV_MODE) { // Nav
								uint8_t keyCode = Layer_NavMou[(chord & 0x3F) - 1];
								if (! keyCode) {
									// TODO: Mou, Mods
									if (chord == 0x2C) { // Alt+Tab
										mods = HID_KEYBOARD_MODIFIER_LEFTALT;
										Q_Mods = mods;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_TAB;
									} else if (chord == 0xE) { // Ctrl+Tab
										mods = HID_KEYBOARD_MODIFIER_LEFTCTRL;
										Q_Mods = mods;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_TAB;
									}
								} else {
									if (keyCode >= HID_KEYBOARD_SC_LEFT_CONTROL && keyCode <= HID_KEYBOARD_SC_RIGHT_GUI) {
										mods ^= 1 << (keyCode - HID_KEYBOARD_SC_LEFT_CONTROL);
										Q_Mods = mods;
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
										KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
									}
								}
							}
						} else {
							//chord2 = (chord2 & 0x33F) | ((chord2 & 0x40) << 1) | ((chord2 & 0x80) >> 1); // change little fingers (search for a good layout)
							bool isConsonants = chord2 & 0x07C;
							bool isVowels = chord2 & 0x303;
							bool isCShift = chord2 & 0x080;
							if (chord2 == 0xD4) { // Tab
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_TAB;
							} else if (chord2 == 0x1D4) { // 4 Spaces
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
							} else if (chord2 == 0x180) { // 2 Spaces
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
								KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
							} else { // Letters
								if (isConsonants) { // Consonants
									uint8_t chord = chord2;
									chord = (chord >> 2) & ~0x20;
									uint8_t keyCode = Layer_Consonants[Layer_Current][chord - 1];
									if (keyCode == HID_KEYBOARD_SC_LEFT_SHIFT) {
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									} else {
										/*
										if (keyCode == HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS) { // ()
											if (isCShift) {
												keyCode = HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS;
											}
											setShift = true;
										} else if (keyCode == HID_KEYBOARD_SC_1_AND_EXCLAMATION) { // !?
											if (isCShift) {
												if (Layer_Current == LAYER2) {
													keyCode = HID_KEYBOARD_SC_7_AND_AND_AMPERSAND;
												} else {
													keyCode = HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK;
												}
											}
											setShift = true;
										} else if (! keyCode) {
											if (chord == 0x10) { // .,
												if (Layer_Current == LAYER2) {
													keyCode = HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK;
												} else {
													if (isCShift) {
														keyCode = HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN;
														clearShift = true;
													} else {
														keyCode = HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN;
													}
												}
											} else if (chord == 0x1B) { // th
												KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_T;
												KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_H;
											} else if (chord == 0x1E) { // ght
												KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_G;
												KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_H;
												KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_T;
											}
										}*/
										if (keyCode) {
											KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
										} else {

										}
									}
									if (isCShift && ! (chord2 & 0x203)) { // CShift
										mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
									}
								}
								if (isVowels) { // Vowels
									uint8_t chord = (chord2 & 0x3) | ((chord2 & 0x300) >> 6);
									if ((chord & 0xC) == 0xC) {
										chord = chord & 0xB;
									}
									uint8_t keyCode = Layer_Vowels[Layer_Current][chord - 1];
									if (! keyCode && Layer_Current == LAYER1) {
										if (chord == 0b00001010) { // ou
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_O;
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_U;
										} else if (chord == 0b00000111) { // ea
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_E;
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_A;
										} else if (chord == 0b00001001) { // io
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_I;
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_O;
										/*} else if (chord == 0b00001010) { // ae
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_A;
											KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_E;*/
										}
									}
									if (keyCode) {
										KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
									}
									if (isCShift) { // VSpace
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_SPACE;
									}
								}
							}
						}
					}
				} else if (Chords[side] > chord2) {
					Chord_Growing[side] = true;
				}
			}

			KeyboardReport->Modifier |= mods;
			if (clearShift) {
				KeyboardReport->Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_RIGHTSHIFT);
			}
			if (setShift && !(KeyboardReport->Modifier & (HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_RIGHTSHIFT))) {
				KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;
			}

			if (usedKeyCodes > 1) {
				uint8_t j = 0;
				for (uint8_t i = 1; i < usedKeyCodes; i++) {
					if (j < (MACROS_BUFFER_SIZE - 1)) {
						if (KeyboardReport->KeyCode[i] == KeyboardReport->KeyCode[i - 1]) {
							Macros_Buffer[j++] = 0xFF;
						}
						Macros_Buffer[j++] = KeyboardReport->KeyCode[i];
					}
				}
				for (uint8_t i = 1; i < usedKeyCodes; i++) {
					KeyboardReport->KeyCode[i] = 0;
				}
			}
		}

		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		return true;
	} else {
		USB_MouseReport_Data_t* MouseReport = (USB_MouseReport_Data_t*)ReportData;

		/* If first board button being held down, no mouse report */
		/*if (ButtonStatus_LCL & BUTTONS_BUTTON1)
		return 0;

		if (JoyStatus_LCL & JOY_UP)
		  MouseReport->Y = -1;
		else if (JoyStatus_LCL & JOY_DOWN)
		  MouseReport->Y =  1;

		if (JoyStatus_LCL & JOY_LEFT)
		  MouseReport->X = -1;
		else if (JoyStatus_LCL & JOY_RIGHT)
		  MouseReport->X =  1;

		if (JoyStatus_LCL & JOY_PRESS)
		  MouseReport->Button |= (1 << 0);*/

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
		  LED_On();
		}

		if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK) {
		  LED_On();
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
	Settings_Get();
	while (true) {
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
	}
}
