/*
CatBoard2: https://github.com/ibnteo/catboard
*/

void Ports_Init() {
	// Init cols
	DDRB  |= 0b01001010; // B1,B3,B6
	PORTB |= 0b01001010;
	DDRD  |= 0b01000100; // D2, D6(Led)
	PORTD |= 0b01000100; // + LED Off

	// Init rows (PullUp)
	DDRB &= ~0b00110101; // B0,B2,B4,B5
	PORTB |= 0b00110101;
	DDRC &= ~0b10010100; // C2,C4,C7
	PORTC |= 0b10010100;
	DDRD &= ~0b00110001; // D0,D4,D5
	PORTD |= 0b00110001;
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

uint8_t Scan_Tick = 0;
uint16_t Chords_Scan[2];
void Keyboard_Scan() {
	if (Scan_Tick == 0) {
		Chords_Scan[0] = 0;
		Chords_Scan[1] = 0;
		PORTD &= ~(1<<2); // D2
	} else if (Scan_Tick == 1) {
		if (! (PINC & 1<<2)) Chords_Scan[0] |= 1<<0;
		if (! (PINB & 1<<0)) Chords_Scan[0] |= 1<<2;
		if (! (PIND & 1<<4)) Chords_Scan[0] |= 1<<4;
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<6;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<0;
		if (! (PINB & 1<<2)) Chords_Scan[1] |= 1<<2;
		if (! (PINB & 1<<5)) Chords_Scan[1] |= 1<<4;
		if (! (PINC & 1<<7)) Chords_Scan[1] |= 1<<6;
		PORTD |= 1<<2;
		PORTB &= ~(1<<6); // B6
	} else if (Scan_Tick == 2) {
		if (! (PINC & 1<<2)) Chords_Scan[0] |= 1<<1;
		if (! (PINB & 1<<0)) Chords_Scan[0] |= 1<<3;
		if (! (PIND & 1<<4)) Chords_Scan[0] |= 1<<5;
		if (! (PIND & 1<<0)) Chords_Scan[0] |= 1<<7;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<1;
		if (! (PINB & 1<<2)) Chords_Scan[1] |= 1<<3;
		if (! (PINB & 1<<5)) Chords_Scan[1] |= 1<<5;
		if (! (PINC & 1<<7)) Chords_Scan[1] |= 1<<7;
		PORTB |= 1<<6;
		PORTB &= ~(1<<1); // B1
	} else if (Scan_Tick == 3) {
		if (! (PINC & 1<<2)) Chords_Scan[0] |= 1<<9;
		if (! (PINB & 1<<0)) Chords_Scan[0] |= 1<<8;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<9;
		if (! (PINB & 1<<2)) Chords_Scan[1] |= 1<<8;
		// Double thumb
		if (! (PIND & 1<<5)) {
			Chords_Scan[0] |= 1<<9;
			Chords_Scan[0] |= 1<<8;
		}
		if (! (PINB & 1<<4)) {
			Chords_Scan[1] |= 1<<9;
			Chords_Scan[1] |= 1<<8;
		}
		if (! (PIND & 1<<4)) {
			Chords_Scan[0] |= 1<<6;
			Chords_Scan[0] |= 1<<7;
		}
		if (! (PINB & 1<<5)) {
			Chords_Scan[1] |= 1<<6;
			Chords_Scan[1] |= 1<<7;
		}
		PORTB |= 1<<1;
		// Double fingers
		PORTB &= ~(1<<3); // B3
	} else if (Scan_Tick == 4) {
		if (! (PINC & 1<<2)) {
			Chords_Scan[0] |= 1<<0;
			Chords_Scan[0] |= 1<<1;
		}
		if (! (PINB & 1<<0)) {
			Chords_Scan[0] |= 1<<2;
			Chords_Scan[0] |= 1<<3;
		}
		if (! (PIND & 1<<4)) {
			Chords_Scan[0] |= 1<<4;
			Chords_Scan[0] |= 1<<5;
		}
		if (! (PINC & 1<<4)) {
			Chords_Scan[1] |= 1<<0;
			Chords_Scan[1] |= 1<<1;
		}
		if (! (PINB & 1<<2)) {
			Chords_Scan[1] |= 1<<2;
			Chords_Scan[1] |= 1<<3;
		}
		if (! (PINB & 1<<5)) {
			Chords_Scan[1] |= 1<<4;
			Chords_Scan[1] |= 1<<5;
		}
		PORTB |= 1<<3;
		Chords[0] = Chords_Scan[0];
		Chords[1] = Chords_Scan[1];
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
