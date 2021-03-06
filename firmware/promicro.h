/*
PD0 (3)     (A2) PF5  Rows
PD4 (4)     (A1) PF6    Cols
PC6 (5)     (A0) PF7    Cols
PD7 (6)     (15) PB1  Rows
PE6 (7)     (14) PB3    Cols
PB4 (8)     (16) PB2    Cols

PB5 (9)  Rows
*/

void Ports_Init() {

	// Init cols
	DDRB  |= 0b00011101; // B2,B3,B4, B0 (LED1)
	PORTB |= 0b00011101; // + LED Off
	DDRC  |= 0b01000000; // C6
	PORTC |= 0b01000000; // 
	DDRD  |= 0b00110000; // D4, D5 (LED2)
	PORTD |= 0b00110000; // + LED Off
	DDRE  |= 0b01000000; // E6
	PORTE |= 0b01000000; // 
	DDRF  |= 0b11000000; // F6,F7
	PORTF |= 0b11000000; // 

	// Init rows (PullUp)
	DDRB &= ~0b00100010; // B1,B5
	PORTB |= 0b00100010;
	DDRD &= ~0b10000001; // D0,D7
	PORTD |= 0b10000001;
	DDRF &= ~0b00100000; // F5
	PORTF |= 0b00100000;
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
		PORTB &= ~(1<<4); // B4
	} else if (Scan_Tick == 1) {
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<0;
		if (! (PIND & 1<<7)) Chords_Scan[0] |= 1<<1;
		if (! (PINB & 1<<5)) Chords_Scan[0] |= 1<<9;
		PORTB |= 1<<4;
		PORTC &= ~(1<<6); // C6
	} else if (Scan_Tick == 2) {
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<2;
		if (! (PIND & 1<<7)) Chords_Scan[0] |= 1<<3;
		if (! (PINB & 1<<5)) Chords_Scan[0] |= 1<<8;
		PORTC |= 1<<6;
		PORTE &= ~(1<<6); // E6
	} else if (Scan_Tick == 3) {
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<4;
		if (! (PIND & 1<<7)) Chords_Scan[0] |= 1<<5;
		PORTE |= 1<<6;
		PORTD &= ~(1<<4); // D4
	} else if (Scan_Tick == 4) {
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<6;
		if (! (PIND & 1<<7)) Chords_Scan[0] |= 1<<7;
		PORTD |= 1<<4;
		PORTB &= ~(1<<2); // B2
	} else if (Scan_Tick == 5) {
		if (! (PINF & 1<<5)) Chords_Scan[1] |= 1<<0;
		if (! (PINB & 1<<1)) Chords_Scan[1] |= 1<<1;
		if (! (PINB & 1<<5)) Chords_Scan[1] |= 1<<9;
		PORTB |= 1<<2;
		PORTF &= ~(1<<7); // F7
	} else if (Scan_Tick == 6) {
		if (! (PINF & 1<<5)) Chords_Scan[1] |= 1<<2;
		if (! (PINB & 1<<1)) Chords_Scan[1] |= 1<<3;
		if (! (PINB & 1<<5)) Chords_Scan[1] |= 1<<8;
		PORTF |= 1<<7;
		PORTF &= ~(1<<6); // F6
	} else if (Scan_Tick == 7) {
		if (! (PINF & 1<<5)) Chords_Scan[1] |= 1<<4;
		if (! (PINB & 1<<1)) Chords_Scan[1] |= 1<<5;
		PORTF |= 1<<6;
		PORTB &= ~(1<<3); // B3
	} else if (Scan_Tick == 8) {
		if (! (PINF & 1<<5)) Chords_Scan[1] |= 1<<6;
		if (! (PINB & 1<<1)) Chords_Scan[1] |= 1<<7;
		PORTB |= 1<<3;
		Chords[0] = Chords_Scan[0];
		Chords[1] = Chords_Scan[1];
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
