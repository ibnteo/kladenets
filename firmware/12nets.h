/*
12nets
Row1 16 PB2
Row2 10 PB6
Row3 A0 PF7
Row4 A3 PF4
Col1 A1 PF6
Col2 A2 PF5
Col3 14 PB3
Col4 15 PB1
*/

void Ports_Init() {
	// Init LEDs
	DDRB  |= 1<<0; // B0(Led)
	PORTB |= 1<<0; // LED Off

	DDRD  |= 1<<5; // D5(Led)
	PORTD |= 1<<5; // LED Off

	// Init Rows
	DDRB |= 1<<2;
	PORTB |= 1<<2;

	DDRB |= 1<<6;
	PORTB |= 1<<6;

	DDRF |= 1<<7;
	PORTF |= 1<<7;

	DDRF |= 1<<4;
	PORTF |= 1<<4;

	// PullUp cols
	DDRF &= ~(1<<6);
	PORTF |= 1<<6;

	DDRF &= ~(1<<5);
	PORTF |= 1<<5;

	DDRB &= ~(1<<3);
	PORTB |= 1<<3;

	DDRB &= ~(1<<1);
	PORTB |= 1<<1;
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
void Keyboard_Scan() {
	if (Scan_Tick == 0) {
		Chords_Scan[0] = 0;
		PORTB &= ~(1<<2);
	} else if (Scan_Tick == 1) {
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<1;
		if (! (PINB & 1<<1)) Chords_Scan[0] |= 1<<0;
		PORTB |= 1<<2;
		PORTB &= ~(1<<6);
	} else if (Scan_Tick == 2) {
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<3;
		if (! (PINB & 1<<1)) Chords_Scan[0] |= 1<<2;
		if (! (PINF & 1<<5)) Chords_Scan[0] |= 1<<8;
		if (! (PINB & 1<<3)) Chords_Scan[0] |= 1<<9;
		PORTB |= 1<<6;
		PORTF &= ~(1<<7);
	} else if (Scan_Tick == 3) {
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<5;
		if (! (PINB & 1<<1)) Chords_Scan[0] |= 1<<4;
		//if (! (PINF & 1<<5)) Chords_Scan[0] |= 1<<8;
		//if (! (PINB & 1<<3)) Chords_Scan[0] |= 1<<9;
		PORTF |= 1<<7;
		PORTF &= ~(1<<4);
	} else if (Scan_Tick == 4) {
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<7;
		if (! (PINB & 1<<1)) Chords_Scan[0] |= 1<<6;
		PORTF |= 1<<4;
		//LED_Switch(Chords_Scan[0]);
		Chords[0] = Chords_Scan[0];
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
