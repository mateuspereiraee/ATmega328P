/*
 * MateusPereiraSprint4.c
 *
 * Created: 06/02/2022 14:28:31
 * Author : mmate
 */ 

/* Nokia 5110 LCD AVR Library example
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by SkewPL, http://skew.tk
 */

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


uint16_t tamroda = 50, rpm = 0, dist = 0, velocidade = 0, tempo_ms = 0, mudanca = 0, aux_dist = 0;
unsigned char display_string[tamanho_vetor];

ISR(INT0_vect)
{
	tamroda++;
}

ISR(INT1_vect)
{
	tamroda--;
}

ISR(TIMER0_COMPA_vect) // Interrupção do TC0 a  cada 1ms
{
	static float aux;
	tempo_ms++;
	if((tempo_ms % 1000) == 0) // Entra a cada 1 segundo
	{
		aux = (mudanca/2); // Vamos ter rotações por segundo, Divide por 2 pois só queremos a contagem da borda de descida
		rpm = (aux*60);
		mudanca = 0;
		aux_dist++;	// Conta de 1 em 1 Segundo
	}
}

ISR(PCINT2_vect) // Interrupção 2 por mudança de pino
{
	mudanca++; // Conta descida e subida
}

int main(void)
{
		DDRC  = 0b11111111;
		DDRB  = 0b11111111;
		DDRD  = 0b00000001;
		PORTD = 0b11111101;
		
		TCCR0A = 0b00000010; // Habilita modo CTC do TC0
		TCCR0B = 0b00000011; //Liga TC0 com prescaler = 64
		OCR0A = 249; // Ajusta o comparador para o TC0 contar ate 249
		TIMSK0 = 0b00000010; // Habilita a interrupção na igualdade de comparação OCR0A. A interrupção ocorre a cada 1ms = (64*(249+1))/16MHz
		
		EICRA = 0b00001010;
		EIMSK = 0b00000011;
		sei();
		
		PCICR = 0b00000100; //Habilita a interrupção PCINT2
		PCMSK2 = 0b00000010; // Habilitação do pino D1 na interrupção PCINT2
				
		nokia_lcd_init();
	
	while (1)
	{
		velocidade = 2*3.14*(tamroda/2)/100000*rpm*60; //Comprimento em Km = 2*3.14*(tamroda/2)/100000, como é 1:1 temos rpm*60 (Rotações por Hora)
		dist = aux_dist*velocidade/3600; // A cada 1s incremeta 1 em aux_dist assim temos X km por segundos
		
		//nokia_lcd_init();
		nokia_lcd_clear();
		nokia_lcd_write_string("COMP. DE BORDO",1);
		nokia_lcd_set_cursor(0, 10);
		nokia_lcd_write_string("Diam(cm):", 1);
		itoa(tamroda, &display_string, 10); //chamada da função para converter inteiros em string
		nokia_lcd_set_cursor(55, 10);
		nokia_lcd_write_string(display_string, 1);
		nokia_lcd_set_cursor(0, 20);
		nokia_lcd_write_string("RPM:", 1);
		itoa(rpm, &display_string, 10); //chamada da função para converter inteiros em string
		nokia_lcd_set_cursor(30, 20);
		nokia_lcd_write_string(display_string, 1);
		itoa(dist, &display_string, 10); //chamada da função para converter inteiros em string
		nokia_lcd_set_cursor(35, 33);
		nokia_lcd_write_string(display_string, 2);
		nokia_lcd_set_cursor(60, 35);
		nokia_lcd_write_string("Km", 1);
		nokia_lcd_render();
	
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

