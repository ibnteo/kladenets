/*
PriKlad
*/

void Ports_Init() {
	// Init LEDs
	DDRD  |= 1<<6; // D6(Led)
	PORTD |= 1<<6; // LED Off

	// PullUp
	DDRC &= ~(1<<4);
	PORTC |= 1<<4;

	DDRC &= ~(1<<2);
	PORTC |= 1<<2;

	DDRC &= ~(1<<6);
	PORTC |= 1<<6;

	DDRC &= ~(1<<7);
	PORTC |= 1<<7;

	DDRD &= ~(1<<1);
	PORTD |= 1<<1;

	DDRD &= ~(1<<2);
	PORTD |= 1<<2;
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
uint8_t Chords_Press[10];
void Keyboard_Scan() {

	if (Scan_Tick == 0) {
		// Set 0
		DDRC  |= 1<<5;
		PORTC &= ~(1<<5);
	} else if (Scan_Tick == 1) {
		if (! (PINC & 1<<4) && Chords_Press[9] == 1) {
			Chords[1] &= ~(1<<9);
			Chords_Press[9] = 0;
		}
		if (! (PINC & 1<<7) && Chords_Press[0] == 1) {
			Chords[1] &= ~(1<<0);
			Chords_Press[0] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[0] == 2) {
			Chords[1] &= ~(1<<0);
			Chords_Press[0] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<0;
			Chords_Press[0] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<0;
			Chords_Press[0] = 2;
		}

		// Hi-Z
		DDRC  &= ~(1<<5);
		PORTC &= ~(1<<5);

		// Set 0
		DDRB  |= 1<<5;
		PORTB &= ~(1<<5);
	} else if (Scan_Tick == 2) {
		if (! (PINC & 1<<4)) {
			Chords[1] |= 1<<9;
			Chords_Press[9] = 1;
		}
		if (! (PINC & 1<<7) && Chords_Press[2] == 1) {
			Chords[1] &= ~(1<<2);
			Chords_Press[2] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[2] == 2) {
			Chords[1] &= ~(1<<2);
			Chords_Press[2] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<2;
			Chords_Press[2] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<2;
			Chords_Press[2] = 2;
		}

		// Hi-Z
		DDRB  &= ~(1<<5);
		PORTB &= ~(1<<5);

		// Set 0
		DDRB  |= 1<<6;
		PORTB &= ~(1<<6);
	} else if (Scan_Tick == 3) {
		if (! (PINC & 1<<4)) {
			Chords[1] |= 1<<9;
			Chords_Press[9] = 2;
		}
		if (! (PINC & 1<<7) && Chords_Press[4] == 1) {
			Chords[1] &= ~(1<<4);
			Chords_Press[4] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[4] == 2) {
			Chords[1] &= ~(1<<4);
			Chords_Press[4] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<4;
			Chords_Press[4] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<4;
			Chords_Press[4] = 2;
		}

		// Hi-Z
		DDRB  &= ~(1<<6);
		PORTB &= ~(1<<6);

		// Set 0
		DDRB  |= 1<<7;
		PORTB &= ~(1<<7);
	} else if (Scan_Tick == 4) {
		if (! (PINC & 1<<4) && Chords_Press[9] == 2) {
			Chords[1] &= ~(1<<9);
			Chords_Press[9] = 0;
		}
		if (! (PINC & 1<<7) && Chords_Press[6] == 1) {
			Chords[1] &= ~(1<<6);
			Chords_Press[6] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[6] == 2) {
			Chords[1] &= ~(1<<6);
			Chords_Press[6] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<6;
			Chords_Press[6] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<6;
			Chords_Press[6] = 2;
		}

		// Hi-Z
		DDRB  &= ~(1<<7);
		PORTB &= ~(1<<7);

		// Set 0
		DDRD  |= 1<<0;
		PORTD &= ~(1<<0);
	} else if (Scan_Tick == 5) {
		if (! (PINC & 1<<2) && Chords_Press[8] == 1) {
			Chords[1] &= ~(1<<8);
			Chords_Press[8] = 0;
		}
		if (! (PINC & 1<<7) && Chords_Press[1] == 1) {
			Chords[1] &= ~(1<<1);
			Chords_Press[1] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[1] == 2) {
			Chords[1] &= ~(1<<1);
			Chords_Press[1] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<1;
			Chords_Press[1] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<1;
			Chords_Press[1] = 2;
		}

		// Hi-Z
		DDRD  &= ~(1<<0);
		PORTD &= ~(1<<0);

		// Set 0
		DDRD  |= 1<<5;
		PORTD &= ~(1<<5);
	} else if (Scan_Tick == 6) {
		if (! (PINC & 1<<2)) {
			Chords[1] |= 1<<8;
			Chords_Press[8] = 1;
		}
		if (! (PINC & 1<<7) && Chords_Press[3] == 1) {
			Chords[1] &= ~(1<<3);
			Chords_Press[3] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[3] == 2) {
			Chords[1] &= ~(1<<3);
			Chords_Press[3] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<3;
			Chords_Press[3] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<3;
			Chords_Press[3] = 2;
		}

		// Hi-Z
		DDRD  &= ~(1<<5);
		PORTD &= ~(1<<5);

		// Set 0
		DDRD  |= 1<<4;
		PORTD &= ~(1<<4);
	} else if (Scan_Tick == 7) {
		if (! (PINC & 1<<2)) {
			Chords[1] |= 1<<8;
			Chords_Press[8] = 2;
		}
		if (! (PINC & 1<<7) && Chords_Press[5] == 1) {
			Chords[1] &= ~(1<<5);
			Chords_Press[5] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[5] == 2) {
			Chords[1] &= ~(1<<5);
			Chords_Press[5] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<5;
			Chords_Press[5] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<5;
			Chords_Press[5] = 2;
		}

		// Hi-Z
		DDRD  &= ~(1<<4);
		PORTD &= ~(1<<4);

		// Set 0
		DDRD  |= 1<<3;
		PORTD &= ~(1<<3);
	} else if (Scan_Tick == 8) {
		if (! (PINC & 1<<2) && Chords_Press[8] == 2) {
			Chords[1] &= ~(1<<8);
			Chords_Press[8] = 0;
		}
		if (! (PINC & 1<<7) && Chords_Press[7] == 1) {
			Chords[1] &= ~(1<<7);
			Chords_Press[7] = 0;
		}
		if (! (PINC & 1<<6) && Chords_Press[7] == 2) {
			Chords[1] &= ~(1<<7);
			Chords_Press[7] = 0;
		}
		if (! (PIND & 1<<2)) {
			Chords[1] |= 1<<7;
			Chords_Press[7] = 1;
		}
		if (! (PIND & 1<<1)) {
			Chords[1] |= 1<<7;
			Chords_Press[7] = 2;
		}

		// Hi-Z
		DDRD  &= ~(1<<3);
		PORTD &= ~(1<<3);
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
