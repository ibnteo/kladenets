/*
Accordionum
*/

/*
1, 2, 3, 4, 5, 6
B3,B4,B5,B6,B7,C7

C6,C5,C4,D1,D0,C2
7, 8, 9, 10,11,12
*/

void Ports_Init() {
	// Init cols
	//DDRB  |= 0b11111000; // B3,B4,B5,B6,B7
	//PORTB |= 0b11111000;
	//DDRC  |= 0b10000000; // C7
	//PORTC |= 0b10000000;
	DDRD  |= 0b01000000; // D6(Led)
	PORTD |= 0b01000000; // + LED Off

	// Init rows (PullUp)
	DDRC &= ~0b01110100; // C2,C4,C5,C6
	PORTC |= 0b01110100;
	DDRD &= ~0b00000011; // D0,D1
	PORTD |= 0b00000011;
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
	/*PORTB &= ~(1<<4); // B4
	LED_Switch(PIND & 1<<1);
	return;*/
	if (Scan_Tick == 0) {
		Chords_Scan[0] = 0;
		Chords_Scan[1] = 0;
		DDRC  |= 1<<7;
		PORTC &= ~(1<<7); // C7
	} else if (Scan_Tick == 1) {
		if (! (PINC & 1<<5)) Chords_Scan[1] |= 1<<8;
		DDRC  &= ~(1<<7);
		PORTC &= ~(1<<7);
		DDRB  |= 1<<7;
		PORTB &= ~(1<<7); // B7
	} else if (Scan_Tick == 2) {
		if (! (PINC & 1<<5)) Chords_Scan[1] |= 1<<7;
		DDRB  &= ~(1<<7);
		PORTB &= ~(1<<7);
		DDRB  |= 1<<6;
		PORTB &= ~(1<<6); // B6
	} else if (Scan_Tick == 3) {
		if (! (PINC & 1<<6)) Chords_Scan[1] |= 1<<0;
		if (! (PINC & 1<<5)) Chords_Scan[1] |= 1<<1;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<0;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<1;
		DDRB  &= ~(1<<6);
		PORTB &= ~(1<<6);
		DDRB  |= 1<<5;
		PORTB &= ~(1<<5); // B5
	} else if (Scan_Tick == 4) {
		if (! (PINC & 1<<6)) Chords_Scan[1] |= 1<<2;
		if (! (PINC & 1<<5)) Chords_Scan[1] |= 1<<3;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<2;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<3;
		if (! (PIND & 1<<1)) Chords_Scan[1] |= 1<<9;
		DDRB  &= ~(1<<5);
		PORTB &= ~(1<<5);
		DDRB  |= 1<<4;
		PORTB &= ~(1<<4); // B4
	} else if (Scan_Tick == 5) {
		if (! (PINC & 1<<6)) Chords_Scan[1] |= 1<<4;
		if (! (PINC & 1<<5)) Chords_Scan[1] |= 1<<5;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<4;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<5;
		if (! Chords_Scan[1] && 0x30) {
			if (! (PIND & 1<<1)) Chords_Scan[1] |= 1<<8;
			if (! (PIND & 1<<1)) Chords_Scan[1] |= 1<<9;
		}
		DDRB  &= ~(1<<4);
		PORTB &= ~(1<<4);
		DDRB  |= 1<<3;
		PORTB &= ~(1<<3); // B3
	} else if (Scan_Tick == 6) {
		if (! (PINC & 1<<6)) Chords_Scan[1] |= 1<<6;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<6;
		if (! (PINC & 1<<4)) Chords_Scan[1] |= 1<<7;
		DDRB  &= ~(1<<3);
		PORTB &= ~(1<<3);
		Chords[0] = Chords_Scan[0];
		Chords[1] = Chords_Scan[1];
	}
	Scan_Tick ++;
	if (Scan_Tick > 20) { // Scan delay
		Scan_Tick = 0;
	}
}
