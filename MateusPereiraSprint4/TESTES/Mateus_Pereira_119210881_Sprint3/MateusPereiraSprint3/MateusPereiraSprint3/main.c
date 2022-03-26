/*
 * MateusPereiraSprint3.c
 *
 * Created: 13/12/2021 21:27:41
 * Author : mmate
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t dezena=0, centena=0;
//uint8_t unidade=0; caso queira controlar de +1 ou -1

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
	DDRB  = 0b00001111;
	DDRC  = 0b00001111;
	DDRD  = 0b11110011;
	PORTD = 0b00001100;
	
	EICRA = 0b00001010;
	EIMSK = 0b00000011;
	sei();
	
	while (1)
	{
		PORTC = 0b0111;
		PORTB = 0;
		PORTC = 0b0110;
		_delay_ms(1);
		PORTC = 0b0111;
		PORTB = dezena;
		PORTC = 0b0101;
		_delay_ms(1);
		PORTC = 0b0111;
		PORTB = centena;
		PORTC = 0b0011;
		_delay_ms(1);			
	}
}