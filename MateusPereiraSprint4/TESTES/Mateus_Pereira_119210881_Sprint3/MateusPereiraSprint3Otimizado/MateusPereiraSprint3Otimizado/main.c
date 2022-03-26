/*
 * MateusPereiraSprint3.c
 *
 * Created: 13/12/2021 23:34:29
 * Author : mmate
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t dezena=0, centena=0;

ISR(INT0_vect)
{
	if (centena<3)
	{
		dezena++;
	}
	if (dezena==10)
	{
		if(centena<3)
		{
			centena += 1;
			dezena = 0;
		}
		else
		{
			centena = 3;
			dezena = 0;
		}
	}
}

ISR(INT1_vect)
{
	if (dezena>0)
	{
		dezena -= 1;
	}
	else
	{
		if (centena>0)
		{
			centena -= 1;
			dezena=9;
		}
		else
		{
			dezena=0;
			centena=0;
		}
	}
}
	
int main(void)
{
	DDRB  = 0b11111111;
	DDRD  = 0b11110011;
	PORTD = 0b00001100;
	
	EICRA = 0b00001010;
	EIMSK = 0b00000011;
	sei();
	
	while (1)
	{
		PORTB = 0;
		PORTB ^= 0b11100000;
		_delay_ms(1);
		PORTB &= 0b00000000;
		PORTB |= dezena;
		PORTB ^= 0b11010000;
		_delay_ms(1);
		PORTB &= 0b00000000;
		PORTB |= centena;
		PORTB ^= 0b10110000;
		_delay_ms(1);
	}
}

