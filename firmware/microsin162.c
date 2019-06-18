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
