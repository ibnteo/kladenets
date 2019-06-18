/*
CatBoard2: https://github.com/ibnteo/catboard
*/

void Ports_Init() {
	// Init cols
	DDRB  |= 0b01000010; // B1,B6
	PORTB |= 0b01000010;
	//DDRC  |= 0b00000000;
	//PORTC |= 0b00000000;
	DDRD  |= 0b01000100; // D2, D6(Led)
	PORTD |= 0b01000100; // + LED Off

	// Init rows (PullUp)
	DDRB &= ~0b00100101; // B0,B2,B5
	PORTB |= 0b00100101;
	DDRC &= ~0b10010100; // C2,C4,C7
	PORTC |= 0b10010100;
	DDRD &= ~0b00010001; // D0,D4
	PORTD |= 0b00010001;
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

void Keyboard_Scan() {
	Chords[0] = 0;
	Chords[1] = 0;

	PORTD &= ~(1<<2); // D2
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<0; // C2
	if (! (PINB & 1<<0)) Chords[0] |= 1<<2; // B0
	if (! (PIND & 1<<4)) Chords[0] |= 1<<4; // D4
	if (! (PIND & 1<<0)) Chords[0] |= 1<<6; // D0
	if (! (PINC & 1<<4)) Chords[1] |= 1<<0; // C4
	if (! (PINB & 1<<2)) Chords[1] |= 1<<2; // B2
	if (! (PINB & 1<<5)) Chords[1] |= 1<<4; // B5
	if (! (PINC & 1<<7)) Chords[1] |= 1<<6; // C7
	PORTD |= 1<<2;

	PORTB &= ~(1<<6); // B6
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<1; // C2
	if (! (PINB & 1<<0)) Chords[0] |= 1<<3; // B0
	if (! (PIND & 1<<4)) Chords[0] |= 1<<5; // D4
	if (! (PIND & 1<<0)) Chords[0] |= 1<<7; // D0
	if (! (PINC & 1<<4)) Chords[1] |= 1<<1; // C4
	if (! (PINB & 1<<2)) Chords[1] |= 1<<3; // B2
	if (! (PINB & 1<<5)) Chords[1] |= 1<<5; // B5
	if (! (PINC & 1<<7)) Chords[1] |= 1<<7; // C7
	PORTB |= 1<<6;

	PORTB &= ~(1<<1); // B1
	_delay_us(1);
	if (! (PINC & 1<<2)) Chords[0] |= 1<<9; // C2
	if (! (PINB & 1<<0)) Chords[0] |= 1<<8; // B0
	if (! (PINC & 1<<4)) Chords[1] |= 1<<9; // C4
	if (! (PINB & 1<<2)) Chords[1] |= 1<<8; // B2
	PORTB |= 1<<1;

	_delay_ms(5);
}
