/*
C2     C4  Rows
D0     C5    Cols
D1     C6    Cols
D4     B4  Rows
D5     B3    Cols
B0     B2    Cols

B1  Rows
*/

void Ports_Init() {

	// Init cols
	DDRB  |= 0b00001101; // B0,B2,B3
	PORTB |= 0b00001101;
	DDRC  |= 0b01100000; // C5,C6
	PORTC |= 0b01100000;
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
	PORTD ^= 1<<6;
}
void LED_Switch(bool on) {
	if (on) LED_On(); else LED_Off();
}

void LED2_On() {
}
void LED2_Off() {
}
void LED2_Toggle() {
}
void LED2_Switch(bool on) {
}

void Chord_Set(uint8_t index, uint8_t bit, bool notset) {
	if (notset) {
		Chords[index] &= ~(1<<bit);
	} else {
		Chords[index] |= 1<<bit;
	}
}

uint8_t Scan_Tick = 0;
void Keyboard_Scan() {
	if (Scan_Tick == 0) {
		PORTB &= ~(1<<0); // B0
	} else if (Scan_Tick == 1) {
		Chord_Set(0, 0, PINC & 1<<2);
		Chord_Set(0, 1, PIND & 1<<4);
		Chord_Set(0, 9, PINB & 1<<1);
		PORTB |= 1<<0;
		PORTD &= ~(1<<1); // D1
	} else if (Scan_Tick == 2) {
		Chord_Set(0, 2, PINC & 1<<2);
		Chord_Set(0, 3, PIND & 1<<4);
		Chord_Set(0, 8, PINB & 1<<1);
		PORTD |= 1<<1;
		PORTD &= ~(1<<5); // D5
	} else if (Scan_Tick == 3) {
		Chord_Set(0, 4, PINC & 1<<2);
		Chord_Set(0, 5, PIND & 1<<4);
		PORTD |= 1<<5;
		PORTD &= ~(1<<0); // D0
	} else if (Scan_Tick == 4) {
		Chord_Set(0, 6, PINC & 1<<2);
		Chord_Set(0, 7, PIND & 1<<4);
		PORTD |= 1<<0;
		PORTB &= ~(1<<2); // B2
	} else if (Scan_Tick == 5) {
		Chord_Set(1, 0, PINC & 1<<4);
		Chord_Set(1, 1, PINB & 1<<4);
		Chord_Set(1, 9, PINB & 1<<1);
		PORTB |= 1<<2;
		PORTC &= ~(1<<6); // C6
	} else if (Scan_Tick == 6) {
		Chord_Set(1, 2, PINC & 1<<4);
		Chord_Set(1, 3, PINB & 1<<4);
		Chord_Set(1, 8, PINB & 1<<1);
		PORTC |= 1<<6;
		PORTC &= ~(1<<5); // C5
	} else if (Scan_Tick == 7) {
		Chord_Set(1, 4, PINC & 1<<4);
		Chord_Set(1, 5, PINB & 1<<4);
		PORTC |= 1<<5;
		PORTB &= ~(1<<3); // B3
	} else if (Scan_Tick == 8) {
		Chord_Set(1, 6, PINC & 1<<4);
		Chord_Set(1, 7, PINB & 1<<4);
		PORTB |= 1<<3;
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
