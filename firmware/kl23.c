/*
* Project: Chord keyboard Kladenets-23
* Version: 0.3 (pre alpha)
* Date: 2019-06-14
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
	PORTD |= 0b01100011; // + LED Off

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

uint16_t Chords[2] = {0, 0};

void Keyboard_Scan() {
	Chords[0] = 0;
	Chords[1] = 0;

	PORTB &= ~(1<<0); // B0
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<0; // C2
	if (! (PIND & 1<<4)) Chords[0] |= 1<<1; // D4
	if (! (PINB & 1<<1)) Chords[0] |= 1<<9; // B1
	PORTB |= 1<<0;

	PORTD &= ~(1<<1); // D1
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<2; // C2
	if (! (PIND & 1<<4)) Chords[0] |= 1<<3; // D4
	if (! (PINB & 1<<1)) Chords[0] |= 1<<8; // B1
	PORTD |= 1<<1;

	PORTD &= ~(1<<5); // D5
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<4; // C2
	if (! (PIND & 1<<4)) Chords[0] |= 1<<5; // D4
	PORTD |= 1<<5;

	PORTD &= ~(1<<0); // D0
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<6; // C2
	if (! (PIND & 1<<4)) Chords[0] |= 1<<7; // D4
	PORTD |= 1<<0;

	PORTB &= ~(1<<2); // B2
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[1] |= 1<<0; // C4
	if (! (PINB & 1<<4)) Chords[1] |= 1<<1; // B4
	if (! (PINB & 1<<1)) Chords[1] |= 1<<9; // B1
	PORTB |= 1<<2;

	PORTC &= ~(1<<6); // C6
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[1] |= 1<<2; // C2
	if (! (PINB & 1<<4)) Chords[1] |= 1<<3; // B4
	if (! (PINB & 1<<1)) Chords[1] |= 1<<8; // B1
	PORTC |= 1<<6;

	PORTC &= ~(1<<5); // C5
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[1] |= 1<<4; // C4
	if (! (PINB & 1<<4)) Chords[1] |= 1<<5; // B4
	PORTC |= 1<<5;

	PORTB &= ~(1<<3); // B3
	_delay_us(1);
	if (! (PINC & 1<<4)) Chords[1] |= 1<<6; // C4
	if (! (PINB & 1<<4)) Chords[1] |= 1<<7; // B4
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

#define HID_KEYBOARD_RU_O					HID_KEYBOARD_SC_J
#define HID_KEYBOARD_RU_A					HID_KEYBOARD_SC_F
#define HID_KEYBOARD_RU_N					HID_KEYBOARD_SC_Y
#define HID_KEYBOARD_RU_S					HID_KEYBOARD_SC_C
#define HID_KEYBOARD_RU_L					HID_KEYBOARD_SC_K
#define HID_KEYBOARD_RU_E					HID_KEYBOARD_SC_T
#define HID_KEYBOARD_RU_I					HID_KEYBOARD_SC_B
#define HID_KEYBOARD_RU_D					HID_KEYBOARD_SC_L
#define HID_KEYBOARD_RU_P					HID_KEYBOARD_SC_G
#define HID_KEYBOARD_RU_T					HID_KEYBOARD_SC_N
#define HID_KEYBOARD_RU_R					HID_KEYBOARD_SC_H
#define HID_KEYBOARD_RU_H					HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
#define HID_KEYBOARD_RU_U					HID_KEYBOARD_SC_E
#define HID_KEYBOARD_RU_W					HID_KEYBOARD_SC_D
#define HID_KEYBOARD_RU_J					HID_KEYBOARD_SC_Q
#define HID_KEYBOARD_RU_V					HID_KEYBOARD_SC_M
#define HID_KEYBOARD_RU_B					HID_KEYBOARD_SC_COMMA_AND_LESS_THAN_SIGN
#define HID_KEYBOARD_RU_K					HID_KEYBOARD_SC_R
#define HID_KEYBOARD_RU_Q					HID_KEYBOARD_SC_SEMICOLON_AND_COLON
#define HID_KEYBOARD_RU_C					HID_KEYBOARD_SC_W
#define HID_KEYBOARD_RU_M					HID_KEYBOARD_SC_V
#define HID_KEYBOARD_RU_X					HID_KEYBOARD_SC_OPENING_BRACKET_AND_OPENING_BRACE
#define HID_KEYBOARD_RU_Z					HID_KEYBOARD_SC_P
#define HID_KEYBOARD_RU_F					HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
#define HID_KEYBOARD_RU_G					HID_KEYBOARD_SC_U
#define HID_KEYBOARD_RU_Y					HID_KEYBOARD_SC_S
#define HID_KEYBOARD_RU_SOFT_SIGN		HID_KEYBOARD_SC_M
#define HID_KEYBOARD_RU_HARD_SIGN		HID_KEYBOARD_SC_CLOSING_BRACKET_AND_CLOSING_BRACE
#define HID_KEYBOARD_RU_YU					HID_KEYBOARD_SC_DOT_AND_GREATER_THAN_SIGN
#define HID_KEYBOARD_RU_YA					HID_KEYBOARD_SC_Z
#define HID_KEYBOARD_RU_YI					HID_KEYBOARD_SC_S
#define HID_KEYBOARD_RU_YO					HID_KEYBOARD_SC_GRAVE_ACCENT_AND_TILDE
#define HID_KEYBOARD_RU_SH					HID_KEYBOARD_SC_I
#define HID_KEYBOARD_RU_SCH				HID_KEYBOARD_SC_O
#define HID_KEYBOARD_RU_CH					HID_KEYBOARD_SC_X
#define HID_KEYBOARD_RU_YE					HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE

#define HID_KEYBOARD_FF	0xFF

// Vowels for the right hand, bits 11xx are replaced by bits 10xx
static uint8_t Layer_Vowels[2][11] = {
	{
		HID_KEYBOARD_SC_O,							// 00 01
		HID_KEYBOARD_SC_E,							// 00 10
		HID_KEYBOARD_SC_U,							// 00 11
		HID_KEYBOARD_SC_SPACE,						// 01 00
		HID_KEYBOARD_SC_A,							// 01 01
		HID_KEYBOARD_SC_I,							// 01 10
		0,													// 01 11
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,	// 10 00
		0,													// 10 01
		0,													// 10 10
		0,													// 10 11
	},
	{
		HID_KEYBOARD_RU_O,			// 00 01
		HID_KEYBOARD_RU_E,			// 00 10
		HID_KEYBOARD_RU_U,			// 00 11
		HID_KEYBOARD_SC_SPACE,		// 01 00
		HID_KEYBOARD_RU_A,			// 01 01
		HID_KEYBOARD_RU_I,			// 01 10
		HID_KEYBOARD_RU_YU,			// 01 11
		HID_KEYBOARD_RU_SOFT_SIGN,	// 10 00
		HID_KEYBOARD_RU_YA,			// 10 01
		HID_KEYBOARD_RU_YI,			// 10 10
		HID_KEYBOARD_RU_YO,			// 10 11
	}
};

// Consonants for the left hand
static uint8_t Layer_Consonants[2][31] = {
	{
		HID_KEYBOARD_SC_R,									// 0 0001
		HID_KEYBOARD_SC_N,									// 0 0010
		HID_KEYBOARD_SC_P,									// 0 0011
		HID_KEYBOARD_SC_S,									// 0 0100
		HID_KEYBOARD_SC_C,									// 0 0101
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,			// 0 0110
		HID_KEYBOARD_SC_W,									// 0 0111
		HID_KEYBOARD_SC_T,									// 0 1000
		HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,	// 0 1001
		HID_KEYBOARD_SC_H,									// 0 1010
		HID_KEYBOARD_SC_J,									// 0 1011
		HID_KEYBOARD_SC_K,									// 0 1100
		HID_KEYBOARD_SC_X,									// 0 1101
		HID_KEYBOARD_SC_Y,									// 0 1110
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,			// 0 1111
		0,															// 1 0000 .,
		HID_KEYBOARD_SC_L,									// 1 0001
		HID_KEYBOARD_SC_M,									// 1 0010
		HID_KEYBOARD_SC_B,									// 1 0011
		HID_KEYBOARD_SC_Z,									// 1 0100
		HID_KEYBOARD_SC_LEFT_SHIFT,						// 1 0101
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,			// 1 0110
		HID_KEYBOARD_SC_V,									// 1 0111
		HID_KEYBOARD_SC_D,									// 1 1000
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,				// 1 1001
		HID_KEYBOARD_SC_F,									// 1 1010
		0,															// 1 1011 th
		HID_KEYBOARD_SC_G,									// 1 1100
		HID_KEYBOARD_SC_Q,									// 1 1101
		0,															// 1 1110 ght
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,					// 1 1111
	},
	{
		HID_KEYBOARD_RU_R,									// 0 0001
		HID_KEYBOARD_RU_N,									// 0 0010
		HID_KEYBOARD_RU_P,									// 0 0011
		HID_KEYBOARD_RU_S,									// 0 0100
		HID_KEYBOARD_RU_J,									// 0 0101
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,			// 0 0110
		HID_KEYBOARD_RU_CH,									// 0 0111
		HID_KEYBOARD_RU_T,									// 0 1000
		HID_KEYBOARD_SC_9_AND_OPENING_PARENTHESIS,	// 0 1001
		HID_KEYBOARD_RU_W,									// 0 1010
		HID_KEYBOARD_RU_SH,									// 0 1011
		HID_KEYBOARD_RU_K,									// 0 1100
		HID_KEYBOARD_RU_X,									// 0 1101
		HID_KEYBOARD_RU_YE,									// 0 1110
		HID_KEYBOARD_SC_MINUS_AND_UNDERSCORE,			// 0 1111
		0,															// 1 0000 .,
		HID_KEYBOARD_RU_L,									// 1 0001
		HID_KEYBOARD_RU_M,									// 1 0010
		HID_KEYBOARD_RU_B,									// 1 0011
		HID_KEYBOARD_RU_Z,									// 1 0100
		HID_KEYBOARD_SC_LEFT_SHIFT,						// 1 0101
		HID_KEYBOARD_SC_APOSTROPHE_AND_QUOTE,			// 1 0110
		HID_KEYBOARD_RU_C,									// 1 0111
		HID_KEYBOARD_RU_D,									// 1 1000
		HID_KEYBOARD_SC_1_AND_EXCLAMATION,				// 1 1001
		HID_KEYBOARD_RU_F,									// 1 1010
		HID_KEYBOARD_RU_SCH,									// 1 1011
		HID_KEYBOARD_RU_G,									// 1 1100
		HID_KEYBOARD_RU_Q,									// 1 1101
		HID_KEYBOARD_RU_HARD_SIGN,							// 1 1110
		HID_KEYBOARD_SC_EQUAL_AND_PLUS,					// 1 1111
	}
};

static uint8_t Layer_NavMou[2][31] = {
	{
		0, // 000 01
		0, // 000 10
		0, // 000 11
		0, // 001 00
		0, // 001 01
		0, // 001 10
		0, // 001 11
		0, // 010 00
		0, // 010 01
		0, // 010 10
		0, // 010 11
		0, // 011 00
		0, // 011 01
		0, // 011 10
		0, // 011 11
		0, // 100 00
		0, // 100 01
		0, // 100 10
		0, // 100 11
		0, // 101 00
		0, // 101 01
		0, // 101 10
		0, // 101 11
		0, // 110 00
		0, // 110 01
		0, // 110 10
		0, // 110 11
		0, // 111 00
		0, // 111 01
		0, // 111 10
		0, // 111 11
	},
	{
		0, // 000 01
		0, // 000 10
		0, // 000 11
		0, // 001 00
		0, // 001 01
		0, // 001 10
		0, // 001 11
		0, // 010 00
		0, // 010 01
		0, // 010 10
		0, // 010 11
		0, // 011 00
		0, // 011 01
		0, // 011 10
		0, // 011 11
		0, // 100 00
		0, // 100 01
		0, // 100 10
		0, // 100 11
		0, // 101 00
		0, // 101 01
		0, // 101 10
		0, // 101 11
		0, // 110 00
		0, // 110 01
		0, // 110 10
		0, // 110 11
		0, // 111 00
		0, // 111 01
		0, // 111 10
		0, // 111 11
	}
};

static uint8_t Layer_NumFunc[3][15] = {
	{
		0, // 00 01
		0, // 00 10
		0, // 00 11
		0, // 01 00
		0, // 01 01
		0, // 01 10
		0, // 01 11
		0, // 10 00
		0, // 10 01
		0, // 10 10
		0, // 10 11
		0, // 11 00
		0, // 11 01
		0, // 11 10
		0, // 11 11
	},
	{
		0, // 00 01
		0, // 00 10
		0, // 00 11
		0, // 01 00
		0, // 01 01
		0, // 01 10
		0, // 01 11
		0, // 10 00
		0, // 10 01
		0, // 10 10
		0, // 10 11
		0, // 11 00
		0, // 11 01
		0, // 11 10
		0, // 11 11
	},
	{
		0, // 00 01
		0, // 00 10
		0, // 00 11
		0, // 01 00
		0, // 01 01
		0, // 01 10
		0, // 01 11
		0, // 10 00
		0, // 10 01
		0, // 10 10
		0, // 10 11
		0, // 11 00
		0, // 11 01
		0, // 11 10
		0, // 11 11
	}
};

#define LAYER1 0
#define LAYER2 1

#define OS_LINUX 0
#define OS_WINDOWS 1
#define OS_MAC 2

bool Chord_Growing[2] = {true, true};
uint8_t Q_Mods = 0;
uint8_t Layer_Current = LAYER1;

uint8_t OS_Mode = OS_LINUX;
uint8_t EE_OS_Mode EEMEM; // = OS_LINUX;
void Settings_Get() {
	OS_Mode = eeprom_read_byte(&EE_OS_Mode);
}
void Settings_Set() {
	eeprom_write_byte(&EE_OS_Mode, OS_Mode);
}

uint8_t Macros_Buffer[7] = {0,0,0,0,0,0,0};

/*uint8_t Check_Mods(uint8_t keyCode) {
	uint8_t mods = 0;
	if (keyCode >= HID_KEYBOARD_SC_LEFT_CONTROL && keyCode <=HID_KEYBOARD_SC_RIGHT_GUI) {
		mods = keyCode - HID_KEYBOARD_SC_LEFT_CONTROL + 1;
	}
	return mods;
}*/

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
			KeyboardReport->KeyCode[usedKeyCodes++] = Macros_Buffer[0];
			for (uint8_t i = 0; i < sizeof(Macros_Buffer) - 1; i++) {
				Macros_Buffer[i] = Macros_Buffer[i+1];
				if (! Macros_Buffer[i]) break;
			}
			Macros_Buffer[6] = 0;
		} else {

			bool setShift = false;
			bool clearShift = false;

			uint16_t chords[2] = {Chords[0], Chords[1]};
			Keyboard_Scan();

			for (uint8_t side=0; side<=1; side++) {
				uint16_t chord2 = chords[side]; 
				if (Chords[side] < chord2) {
					if (Chord_Growing[side]) {
						Chord_Growing[side] = false;
						if (chord2 == 0x200 || chord2 == 0x80) { // Layer change
							uint8_t newLayer = LAYER1;
							if (chord2 == 0x80) {
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
							if ((chord & 0xC0) && ! (chord & 0x03)) { // Num

							} else {
								
							}
						} else {
							bool isConsonants = chord2 & 0x07C;
							bool isVowels = chord2 & 0x303;
							bool isCShift = chord2 & 0x080;
							if (isConsonants) { // Consonants
								uint8_t chord = chord2;
								chord = (chord >> 2) & ~0x20;
								uint8_t keyCode = Layer_Consonants[Layer_Current][chord - 1];
								if (keyCode == HID_KEYBOARD_SC_LEFT_SHIFT) {
									mods = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
								} else {
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
									}
									if (keyCode) {
										KeyboardReport->KeyCode[usedKeyCodes++] = keyCode;
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
									if (chord == 0b00000111) { // ou
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_O;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_U;
									} else if (chord == 0b00001001) { // ea
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_E;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_A;
									} else if (chord == 0b00001010) { // io
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_I;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_O;
									} else if (chord == 0b00001011) { // ae
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_A;
										KeyboardReport->KeyCode[usedKeyCodes++] = HID_KEYBOARD_SC_E;
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
		}

		if (usedKeyCodes > 1) {
			uint8_t j = 0;
			for (uint8_t i = 1; i <= usedKeyCodes; i ++) {
				Macros_Buffer[j] = KeyboardReport->KeyCode[i];
				KeyboardReport->KeyCode[i] = 0;
				j ++;
			}
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
	while (true) {
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Mouse_HID_Interface);
		USB_USBTask();
	}
}
