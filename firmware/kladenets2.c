/*
* Project: Chord keyboard Kladenets-2
* Version: 0.1 (pre-pre alpha)
* Date: 2019-06-08
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


void Ports_Init() {

	// Init cols
	DDRB  |= 0b00001101; // B0,B2,B3
	PORTB |= 0b00001101;
	DDRC  |= 0b01110000; // C4,C5,C6
	PORTC |= 0b01110000;
	DDRD  |= 0b01100011; // D0,D1,D5, D6(Led)
	PORTD |= 0b01100011; // LED Off

	// Init rows (PullUp)
	DDRB &= ~0b00010010; // B1,B4
	PORTB |= 0b00010010;
	DDRC &= ~0b00010100; // C2,C4
	PORTC |= 0b00010100;
	DDRD &= ~0b00010000; // D4
	PORTD |= 0b00010000;
}

void LED_On() {
	PORTD &= ~(1<<6);
}
void LED_Off() {
	PORTD |= 1<<6;
}
void LED_Toggle() {
	PIND ^= 1<<6;
}
void LED_Switch(bool on) {
	if (on) LED_On(); else LED_Off();
}

uint8_t Chords[4] = {0,0,0,0};

void Keyboard_Scan() {
	Chords[0] = 0;
	Chords[1] = 0;
	Chords[2] = 0;
	Chords[3] = 0;

	PORTB &= ~(1<<0); // B0
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<1; // C2
	if (! (PIND & 1<<4)) Chords[1] |= 1<<1; // D4
	if (! (PINB & 1<<1)) Chords[0] |= 1<<0; // B1
	PORTB |= 1<<0;

	PORTD &= ~(1<<1); // D1
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<2; // C2
	if (! (PIND & 1<<4)) Chords[1] |= 1<<2; // D4
	if (! (PINB & 1<<1)) Chords[1] |= 1<<0; // B1
	PORTD |= 1<<1;

	PORTD &= ~(1<<5); // D5
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<3; // C2
	if (! (PIND & 1<<4)) Chords[1] |= 1<<3; // D4
	PORTD |= 1<<5;

	PORTD &= ~(1<<0); // D0
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<4; // C2
	if (! (PIND & 1<<4)) Chords[1] |= 1<<4; // D4
	PORTD |= 1<<0;

	PORTB &= ~(1<<2); // B2
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[2] |= 1<<1; // C4
	if (! (PINB & 1<<4)) Chords[3] |= 1<<1; // B4
	if (! (PINB & 1<<1)) Chords[2] |= 1<<0; // B1
	PORTB |= 1<<2;

	PORTC &= ~(1<<6); // C6
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[2] |= 1<<2; // C2
	if (! (PINB & 1<<4)) Chords[3] |= 1<<2; // B4
	if (! (PINB & 1<<1)) Chords[3] |= 1<<0; // B1
	PORTC |= 1<<6;

	PORTC &= ~(1<<5); // C5
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[2] |= 1<<3; // C4
	if (! (PINB & 1<<4)) Chords[3] |= 1<<3; // B4
	PORTC |= 1<<5;

	PORTB &= ~(1<<3); // B3
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[2] |= 1<<4; // C4
	if (! (PINB & 1<<4)) Chords[3] |= 1<<4; // B4
	PORTB |= 1<<3;

	_delay_ms(5);
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
	USB_Init();
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

#define CHORD_NAV 0b00000001
#define CHORD_NUM 0b00010000
#define CHORD_SYM 0b00010001
#define KEYBOARD_LAYER 0x00
#define KEYBOARD_MOD 0x00

#define LAYER1 0
#define LAYER_NUM1 1
#define LAYER_NUM2 2
#define LAYER_NAV 3
#define LAYER_MOU 4
#define LAYER_SYM1 5
#define LAYER_SYM2 6
#define LAYER2 7

uint8_t Layers[4][31] = {
	{ // Lay1
		KEYBOARD_LAYER,				// 0000 1 Nav
		HID_KEYBOARD_SC_O,			// 0001 0
		HID_KEYBOARD_SC_A,			// 0001 1
		HID_KEYBOARD_SC_SPACE,		// 0010 0
		HID_KEYBOARD_SC_N,			// 0010 1
		HID_KEYBOARD_SC_S,			// 0011 0
		HID_KEYBOARD_SC_L,			// 0011 1
		HID_KEYBOARD_SC_E,			// 0100 0
		HID_KEYBOARD_SC_I,			// 0100 1
		HID_KEYBOARD_SC_D,			// 0101 0
		HID_KEYBOARD_SC_P,			// 0101 1
		HID_KEYBOARD_SC_T,			// 0110 0
		HID_KEYBOARD_SC_R,			// 0110 1
		HID_KEYBOARD_SC_H,			// 0111 0
		HID_KEYBOARD_SC_U,			// 0111 1
		KEYBOARD_LAYER,				// 1000 0 Num
		KEYBOARD_LAYER,				// 1000 1 Sym
		HID_KEYBOARD_SC_W,			// 1001 0
		HID_KEYBOARD_SC_J,			// 1001 1
		HID_KEYBOARD_SC_V,			// 1010 0
		HID_KEYBOARD_SC_B,			// 1010 1
		HID_KEYBOARD_SC_K,			// 1011 0
		HID_KEYBOARD_SC_Q,			// 1011 1
		HID_KEYBOARD_SC_C,			// 1100 0
		HID_KEYBOARD_SC_M,			// 1100 1
		HID_KEYBOARD_SC_X,			// 1101 0
		HID_KEYBOARD_SC_Z,			// 1101 1
		HID_KEYBOARD_SC_F,			// 1110 0
		HID_KEYBOARD_SC_G,			// 1110 1
		HID_KEYBOARD_SC_Y,			// 1111 0
		HID_KEYBOARD_SC_LEFT_SHIFT	// 1111 1 Shift
	},
	{ // Num1
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,				// 0000 1
		HID_KEYBOARD_SC_2_AND_AT,							// 0001 0
		HID_KEYBOARD_SC_3_AND_HASHMARK,					// 0001 1
		HID_KEYBOARD_SC_4_AND_DOLLAR,						// 0010 0
		HID_KEYBOARD_SC_5_AND_PERCENTAGE,				// 0010 1
		HID_KEYBOARD_SC_6_AND_CARET,						// 0011 0
		HID_KEYBOARD_SC_7_AND_AND_AMPERSAND,			// 0011 1
		HID_KEYBOARD_SC_8_AND_ASTERISK,					// 0100 0
		HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,	// 0100 1
		HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,	// 0101 0
		HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE,			// 0101 1
		HID_KEYBOARD_SC_KEYPAD_MINUS,						// 0110 0
		HID_KEYBOARD_SC_KEYPAD_SLASH,						// 0110 1
		HID_KEYBOARD_SC_KEYPAD_PLUS,						// 0111 0
		HID_KEYBOARD_SC_KEYPAD_ASTERISK,					// 0111 1
		HID_KEYBOARD_SC_KEYPAD_ENTER,						// 1000 0
		HID_KEYBOARD_SC_F1,				// 1000 1
		HID_KEYBOARD_SC_F2,				// 1001 0
		HID_KEYBOARD_SC_F3,				// 1001 1
		HID_KEYBOARD_SC_F4,				// 1010 0
		HID_KEYBOARD_SC_F5,				// 1010 1
		HID_KEYBOARD_SC_F6,				// 1011 0
		HID_KEYBOARD_SC_F7,				// 1011 1
		HID_KEYBOARD_SC_F8,				// 1100 0
		HID_KEYBOARD_SC_F9,				// 1100 1
		HID_KEYBOARD_SC_F10,				// 1101 0
		HID_KEYBOARD_SC_F11,				// 1101 1
		HID_KEYBOARD_SC_F12,				// 1110 0
		HID_KEYBOARD_SC_NUM_LOCK,		// 1110 1
		HID_KEYBOARD_SC_SCROLL_LOCK,	// 1111 0
		HID_KEYBOARD_SC_CAPS_LOCK		// 1111 1
	},
	{ // Num2
		HID_KEYBOARD_SC_KEYPAD_1_AND_END,				// 0000 1
		HID_KEYBOARD_SC_KEYPAD_2_AND_DOWN_ARROW,		// 0001 0
		HID_KEYBOARD_SC_KEYPAD_3_AND_PAGE_DOWN,		// 0001 1
		HID_KEYBOARD_SC_KEYPAD_4_AND_LEFT_ARROW,		// 0010 0
		HID_KEYBOARD_SC_KEYPAD_5,							// 0010 1
		HID_KEYBOARD_SC_KEYPAD_6_AND_RIGHT_ARROW,		// 0011 0
		HID_KEYBOARD_SC_KEYPAD_7_AND_HOME,				// 0011 1
		HID_KEYBOARD_SC_KEYPAD_8_AND_UP_ARROW,			// 0100 0
		HID_KEYBOARD_SC_KEYPAD_9_AND_PAGE_UP,			// 0100 1
		HID_KEYBOARD_SC_KEYPAD_0_AND_INSERT,			// 0101 0
		HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE,			// 0101 1
		HID_KEYBOARD_SC_KEYPAD_MINUS,						// 0110 0
		HID_KEYBOARD_SC_KEYPAD_SLASH,						// 0110 1
		HID_KEYBOARD_SC_KEYPAD_PLUS,						// 0111 0
		HID_KEYBOARD_SC_KEYPAD_ASTERISK,					// 0111 1
		HID_KEYBOARD_SC_BACKSPACE,							// 1000 0
		HID_KEYBOARD_SC_F1,				// 1000 1 	*** TODO: mods
		HID_KEYBOARD_SC_F2,				// 1001 0
		HID_KEYBOARD_SC_F3,				// 1001 1
		HID_KEYBOARD_SC_F4,				// 1010 0
		HID_KEYBOARD_SC_F5,				// 1010 1
		HID_KEYBOARD_SC_F6,				// 1011 0
		HID_KEYBOARD_SC_F7,				// 1011 1
		HID_KEYBOARD_SC_F8,				// 1100 0
		HID_KEYBOARD_SC_F9,				// 1100 1
		HID_KEYBOARD_SC_F10,				// 1101 0
		HID_KEYBOARD_SC_F11,				// 1101 1
		HID_KEYBOARD_SC_F12,				// 1110 0
		HID_KEYBOARD_SC_NUM_LOCK,		// 1110 1
		HID_KEYBOARD_SC_SCROLL_LOCK,	// 1111 0
		HID_KEYBOARD_SC_CAPS_LOCK		// 1111 1
	},
	{ // Nav
		HID_KEYBOARD_SC_ESCAPE,								// 0000 1
		HID_KEYBOARD_SC_RIGHT_ARROW,						// 0001 0
		HID_KEYBOARD_SC_Q,									// 0001 1
		HID_KEYBOARD_SC_DOWN_ARROW,						// 0010 0
		HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN,	// 0010 1
		HID_KEYBOARD_SC_END,									// 0011 0
		HID_KEYBOARD_SC_PAUSE,								// 0011 1
		HID_KEYBOARD_SC_UP_ARROW,							// 0100 0
		HID_KEYBOARD_SC_I,									// 0100 1
		HID_KEYBOARD_SC_BACKSPACE,							// 0101 0
		HID_KEYBOARD_SC_A,									// 0101 1
		HID_KEYBOARD_SC_PAGE_DOWN,							// 0110 0
		HID_KEYBOARD_SC_TAB,									// 0110 1
		HID_KEYBOARD_SC_LEFT_CONTROL,						// 0111 0
		HID_KEYBOARD_SC_RIGHT_CONTROL,					// 0111 1
		HID_KEYBOARD_SC_LEFT_ARROW,						// 1000 0
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,			// 1000 1
		HID_KEYBOARD_SC_PAGE_UP,							// 1001 0
		HID_KEYBOARD_SC_W,									// 1001 1
		HID_KEYBOARD_SC_DELETE,								// 1010 0
		HID_KEYBOARD_SC_INSERT,								// 1010 1
		HID_KEYBOARD_SC_LEFT_GUI,							// 1011 0
		HID_KEYBOARD_SC_RIGHT_GUI,							// 1011 1
		HID_KEYBOARD_SC_HOME,								// 1100 0
		HID_KEYBOARD_SC_PRINT_SCREEN,						// 1100 1
		HID_KEYBOARD_SC_SPACE,								// 1101 0
		HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS,	// 1101 1
		HID_KEYBOARD_SC_LEFT_ALT,							// 1110 0
		HID_KEYBOARD_SC_RIGHT_ALT,							// 1110 1
		HID_KEYBOARD_SC_LEFT_SHIFT,						// 1111 0
		HID_KEYBOARD_SC_RIGHT_SHIFT						// 1111 1
	}
};

bool Chord_Growing[2] = {true, true};
bool Chord_UpFirst[2] = {true, true};
uint8_t Mods = 0;

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
	//uint8_t JoyStatus_LCL    = Joystick_GetStatus();
	//uint8_t ButtonStatus_LCL = Buttons_GetStatus();

	/* Determine which interface must have its report generated */
	if (HIDInterfaceInfo == &Keyboard_HID_Interface) {
		USB_KeyboardReport_Data_t* KeyboardReport = (USB_KeyboardReport_Data_t*)ReportData;

		KeyboardReport->Modifier = Mods;

		uint8_t chords[4] = {Chords[0], Chords[1], Chords[2], Chords[3]};
	
		uint8_t usedKeyCodes = 0;

		Keyboard_Scan();
		uint8_t mods = 0;
		bool setShift = false;
		bool clearShift = false;

		for (uint8_t side=0; side<=1; side++) {
			uint8_t s1 = side * 2;
			uint8_t s2 = s1 + 1;
			if (Chords[s1] < chords[s1] || Chords[s2] < chords[s2]) {
				if (Chord_Growing[side]) {
					Chord_Growing[side] = false;
					uint8_t layer = LAYER1;
					if (chords[s2] == CHORD_NUM && (chords[s1] != CHORD_NAV || Chord_UpFirst[side])) {
						layer = LAYER_NUM1;
						if (chords[s1]) {
							uint8_t keyCode = Layers[layer][chords[s1]-1];
							if (keyCode == HID_KEYBOARD_SC_KEYPAD_DOT_AND_DELETE) {
								keyCode = HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN;
								clearShift = true;
							} else if (keyCode == HID_KEYBOARD_SC_KEYPAD_MINUS) {
								keyCode = HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE;
								clearShift = true;
							} else if (keyCode == HID_KEYBOARD_SC_KEYPAD_SLASH) {
								keyCode = HID_KEYBOARD_SC_SLASH_AND_QUESTION_MARK;
								clearShift = true;
							} else if (keyCode == HID_KEYBOARD_SC_KEYPAD_PLUS) {
								keyCode = HID_KEYBOARD_SC_EQUAL_AND_PLUS;
								setShift = true;
							} else if (keyCode == HID_KEYBOARD_SC_KEYPAD_ASTERISK) {
								keyCode = HID_KEYBOARD_SC_8_AND_ASTERISK;
								setShift = true;
							}
							KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
						} else {
							KeyboardReport->KeyCode[usedKeyCodes++] = Layers[layer][15];
						}
					} else if (chords[s1] == CHORD_NUM) {
						layer = LAYER_NUM2;
						if (chords[s2]) {
							uint8_t keyCode = Layers[layer][chords[s2]-1];
							KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
						} else {
							KeyboardReport->KeyCode[usedKeyCodes++] = Layers[layer][15];
						}
					} else if (chords[s2] == CHORD_NAV) {
						// Mouse
					} else if (chords[s1] == CHORD_NAV) {
						layer = LAYER_NAV;
						if (chords[s2]) {
							uint8_t keyCode = Layers[layer][chords[s2]-1];
							if (keyCode == HID_KEYBOARD_SC_LEFT_CONTROL) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTCTRL;
							} else if (keyCode == HID_KEYBOARD_SC_RIGHT_CONTROL) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTCTRL;
							} else if (keyCode == HID_KEYBOARD_SC_LEFT_SHIFT) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;
							} else if (keyCode == HID_KEYBOARD_SC_RIGHT_SHIFT) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTSHIFT;
							} else if (keyCode == HID_KEYBOARD_SC_LEFT_ALT) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTALT;
							} else if (keyCode == HID_KEYBOARD_SC_RIGHT_ALT) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTALT;
							} else if (keyCode == HID_KEYBOARD_SC_LEFT_GUI) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTGUI;
							} else if (keyCode == HID_KEYBOARD_SC_RIGHT_GUI) {
								KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_RIGHTGUI;
							} else {
								if (keyCode == HID_KEYBOARD_SC_RIGHT_ARROW && s1) {
									keyCode = HID_KEYBOARD_SC_LEFT_ARROW;
								} else if (keyCode == HID_KEYBOARD_SC_LEFT_ARROW && s1) {
									keyCode = HID_KEYBOARD_SC_RIGHT_ARROW;
								} else if (keyCode == HID_KEYBOARD_SC_DOWN_ARROW && s1) {
									keyCode = HID_KEYBOARD_SC_UP_ARROW;
								} else if (keyCode == HID_KEYBOARD_SC_UP_ARROW && s1) {
									keyCode = HID_KEYBOARD_SC_DOWN_ARROW;
								}
								KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
							}
						} else {
							// TODO: set Rus Layer
						}
					} else {
						if (chords[s1]) {
							uint8_t keyCode = Layers[layer][chords[s1]-1];
							KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
						}
						if (chords[s2]) {
							uint8_t keyCode = Layers[layer][chords[s2]-1];
							KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
 						}
					}
				}
			} else if (Chords[s1] > chords[s1] || Chords[s2] > chords[s2]) {
				Chord_Growing[side] = true;
				if (chords[s1]) Chord_UpFirst[side] = false;
				if (chords[s2]) Chord_UpFirst[side] = true;
			}
		}

		if (clearShift) {
			KeyboardReport->Modifier &= ~(HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_RIGHTSHIFT);
		}
		if (setShift && !(KeyboardReport->Modifier & (HID_KEYBOARD_MODIFIER_LEFTSHIFT | HID_KEYBOARD_MODIFIER_RIGHTSHIFT))) {
			KeyboardReport->Modifier |= HID_KEYBOARD_MODIFIER_LEFTSHIFT;
		}

		*ReportSize = sizeof(USB_KeyboardReport_Data_t);
		return false;
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
		/*uint8_t  LEDMask   = LEDS_NO_LEDS;
		uint8_t* LEDReport = (uint8_t*)ReportData;

		if (*LEDReport & HID_KEYBOARD_LED_NUMLOCK)
		  LEDMask |= LEDS_LED1;

		if (*LEDReport & HID_KEYBOARD_LED_CAPSLOCK)
		  LEDMask |= LEDS_LED3;

		if (*LEDReport & HID_KEYBOARD_LED_SCROLLLOCK)
		  LEDMask |= LEDS_LED4;

		LEDs_SetAllLEDs(LEDMask);*/
	}
}

int main(void) {
	Hardware_Setup();
	GlobalInterruptEnable();
	while (true) {
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
	}
}
