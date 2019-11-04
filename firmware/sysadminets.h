/*
Sysadminets
Row1 A3 PF4
Row2 15 PB1
Row3 9 PB5
Col1 A1 PF6
Col2 A0 PF7
Col3 16 PB2
Col4 14 PB3
*/

void Ports_Init() {
	// Init LEDs
	DDRB  |= 1<<0; // B0(Led)
	PORTB |= 1<<0; // LED Off

	DDRD  |= 1<<5; // D5(Led)
	PORTD |= 1<<5; // LED Off

	// Init Rows
	DDRF |= 1<<4;
	PORTF |= 1<<4;

	DDRB |= 1<<1;
	PORTB |= 1<<1;

	DDRB |= 1<<5;
	PORTB |= 1<<5;

	// PullUp cols
	DDRF &= ~(1<<6);
	PORTF |= 1<<6;

	DDRF &= ~(1<<7);
	PORTF |= 1<<7;

	DDRB &= ~(1<<2);
	PORTB |= 1<<2;

	DDRB &= ~(1<<3);
	PORTB |= 1<<3;
}

void LED_On() {
	PORTB &= ~(1<<0);
}
void LED_Off() {
	PORTB |= 1<<0;
}
void LED_Toggle() {
	PORTB ^= 1<<0;
}
void LED_Switch(bool on) {
	if (on) LED_On(); else LED_Off();
}

void LED2_On() {
	PORTD &= ~(1<<5);
}
void LED2_Off() {
	PORTD |= 1<<5;
}
void LED2_Toggle() {
	PORTD ^= 1<<5;
}
void LED2_Switch(bool on) {
	if (on) LED2_On(); else LED2_Off();
}

uint8_t Scan_Tick = 0;
uint16_t Chords_Scan[2];
bool Scan_Side = 0;
void Keyboard_Scan() {
	if (Scan_Tick == 0) {
		Chords_Scan[0] = 0;
		Chords_Scan[1] = 0;
		PORTF &= ~(1<<4);
	} else if (Scan_Tick == 1) {
		if (! (PINF & 1<<6)) Chords_Scan[1] |= 1<<0;
		if (! (PINF & 1<<7)) Chords_Scan[1] |= 1<<2;
		if (! (PINB & 1<<2)) Chords_Scan[1] |= 1<<4;
		if (! (PINB & 1<<3)) Chords_Scan[1] |= 1<<6;

		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<6;
		if (! (PINF & 1<<7)) Chords_Scan[0] |= 1<<4;
		if (! (PINB & 1<<2)) Chords_Scan[0] |= 1<<2;
		if (! (PINB & 1<<3)) Chords_Scan[0] |= 1<<0;
		PORTF |= 1<<4;
		PORTB &= ~(1<<1);
	} else if (Scan_Tick == 2) {
		if (! (PINF & 1<<6)) Chords_Scan[1] |= 1<<1;
		if (! (PINF & 1<<7)) Chords_Scan[1] |= 1<<3;
		if (! (PINB & 1<<2)) Chords_Scan[1] |= 1<<5;
		if (! (PINB & 1<<3)) Chords_Scan[1] |= 1<<7;

		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<7;
		if (! (PINF & 1<<7)) Chords_Scan[0] |= 1<<5;
		if (! (PINB & 1<<2)) Chords_Scan[0] |= 1<<3;
		if (! (PINB & 1<<3)) Chords_Scan[0] |= 1<<1;
		PORTB |= 1<<1;
		PORTB &= ~(1<<5);
	} else if (Scan_Tick == 3) {
		if (! (PINF & 1<<6)) Chords_Scan[1] |= 1<<9;
		if (! (PINF & 1<<7)) Chords_Scan[1] |= 1<<8;
		if (! (PINB & 1<<2)) Chords_Scan[0] |= 1<<8;
		if (! (PINB & 1<<3)) Chords_Scan[0] |= 1<<9;
		if (! (PINF & 1<<7)) {
			Scan_Side = 1;
		} else if (! (PINB & 1<<2)) {
			Scan_Side = 0;
		}
		Chords[Scan_Side] = Chords_Scan[Scan_Side];
		if (Scan_Side) {
			Chords[0] = 0;
		} else {
			Chords[1] = 0;
		}
		PORTB |= 1<<5;
	}
	Scan_Tick ++;
	if (Scan_Tick > 5) { // Scan delay
		Scan_Tick = 0;
	}
}
