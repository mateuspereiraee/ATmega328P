#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "nokia5110.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define tamanho_vetor 16
#define tem_delay 3


uint16_t tamroda = 50, rpm = 0, dist = 0, velocidade = 0, tempo_ms = 0, mudanca = 0, aux_dist =0;
unsigned char display_string[tamanho_vetor];

ISR(INT0_vect)
{
	tamroda++;
}

ISR(INT1_vect)
{
	tamroda--;
}

ISR(TIMER0_COMPA_vect)
{
	static float aux;
	tempo_ms++;
	if((tempo_ms % 1000) == 0)
	{
		aux = (mudanca/2);
		rpm = (aux*60);
		mudanca = 0;
		aux_dist++;	
	}
}

ISR(PCINT2_vect)
{
	mudanca++;
}

int main(void)
{
		DDRC  = 0b11111111;
		DDRB  = 0b11111111;
		DDRD  = 0b00000001;
		PORTD = 0b11111101;
		
		TCCR0A = 0b00000010;
		TCCR0B = 0b00000011;
		OCR0A = 249;
		TIMSK0 = 0b00000010;
		
		EICRA = 0b00001010;
		EIMSK = 0b00000011;
		sei();
		
		PCICR = 0b00000100;
		PCMSK2 = 0b00000010;
				
		nokia_lcd_init();
	
	while (1)
	{
		velocidade = 2*3.14*(tamroda/2)/100000*rpm*60;
		dist = aux_dist*velocidade/3600;
		
		if((tempo_ms % 9) == 0)
		{
		nokia_lcd_clear();
		nokia_lcd_write_string("COMP. DE BORDO",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("Diam(cm):", 1);
		itoa(tamroda, &display_string, 10);
		nokia_lcd_set_cursor(60, 10);
		nokia_lcd_write_string(display_string, 1);
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("RPM:", 1);
		itoa(rpm, &display_string, 10);
		nokia_lcd_set_cursor(30, 20);
		nokia_lcd_write_string(display_string, 1);
		itoa(dist, &display_string, 10);
		nokia_lcd_set_cursor(30, 30);
		nokia_lcd_write_string(display_string, 2);
		nokia_lcd_render();
		}
	
		PORTB &= 0b10000000;		
		PORTB |= 0b01100000;
		PORTB |= (((velocidade/1)%10) & 0b00001111);
		_delay_ms(tem_delay);
		
		PORTB &= 0b10000000;
		PORTB |= 0b01010000;
		PORTB |= (((velocidade/10)%10) & 0b00001111);
		_delay_ms(tem_delay);
		
		PORTB &= 0b10000000;
		PORTB |= 0b00110000;
		PORTB |= (((velocidade/100)%10) & 0b00001111);
		_delay_ms(tem_delay);
	}
}
