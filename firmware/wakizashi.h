/*
PF4 (A3) Col4
PF5 (A2) Col3
PF6 (A1) Col2
PF7 (A0) Col1
PB1 (15) Row1
PB3 (14) Row2
PB2 (16) Row3
PB5 (10)
*/

void Ports_Init() {

	// Init rows
	DDRB  |= 0b00001111; // B1,B2,B3, B0 (LED1)
	PORTB |= 0b00001111; // + LED Off
	DDRD  |= 0b00100000; // D5 (LED2)
	PORTD |= 0b00100000; // + LED Off

	// Init cols (PullUp)
	DDRF &= ~0b11110000; // F4,F5,F6,F7
	PORTF |= 0b11110000;
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
		Chords_Scan[1] = 0;
		PORTB &= ~(1<<1); // B1
	} else if (Scan_Tick == 1) {
		if (! (PINF & 1<<7)) Chords_Scan[0] |= 1<<0; // F7
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<2; // F6
		if (! (PINF & 1<<5)) Chords_Scan[0] |= 1<<4; // F5
		if (! (PINF & 1<<4)) Chords_Scan[0] |= 1<<6; // F4
		PORTB |= 1<<1;
		PORTB &= ~(1<<3); // B3
	} else if (Scan_Tick == 2) {
		if (! (PINF & 1<<7)) Chords_Scan[0] |= 1<<1; // F7
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<3; // F6
		if (! (PINF & 1<<5)) Chords_Scan[0] |= 1<<5; // F5
		if (! (PINF & 1<<4)) Chords_Scan[0] |= 1<<7; // F4
		PORTB |= 1<<3;
		PORTB &= ~(1<<2); // B2
	} else if (Scan_Tick == 3) {
		//if (! (PINF & 1<<7)) Chords_Scan[0] |= 1<<10; // F7 ???
		if (! (PINF & 1<<6)) Chords_Scan[0] |= 1<<8; // F6
		if (! (PINF & 1<<5)) Chords_Scan[0] |= 1<<9; // F5
		PORTB |= 1<<2;
		Chords[0] = Chords_Scan[0];
		Chords[1] = Chords_Scan[1];
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
