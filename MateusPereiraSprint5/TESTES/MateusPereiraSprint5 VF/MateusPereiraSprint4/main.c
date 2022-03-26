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
#define teste_porta(y,porta)(y&(1<<porta))


uint16_t tamroda = 50, rpm = 0, dist = 0, velocidade = 0, tempo_ms = 0, mudanca = 0, aux_dist = 0, leitura_ADC = 0, aux_pwm = 0, modo_p = 0, pqp = 0;
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
		aux = mudanca; // Vamos ter rotações por segundo, Divide por 2 pois só queremos a contagem da borda de descida
		rpm = (aux*60);
		mudanca = 0;
		aux_dist++;	// Conta de 1 em 1 Segundo
	}
}

ISR(PCINT2_vect) // Interrupção 2 por mudança de pino
{
	if (!teste_porta (PIND, PD1))
	{
		mudanca++; // Conta descida e subida
	}
	
	if (teste_porta(PIND,PD4)==0) // SE APERTAR O BOTÃO +10 CAI AQUI
	{
		pqp = 5; // carro parado
	}
		else if (teste_porta(PIND,PD5)==0) // SE APERTAR O BOTÃO +10 CAI AQUI
		{
			pqp = 11; // Motor sentido horário - D
		}
		else if (!teste_porta(PIND,PD5))
		{
			pqp = 13; // Motor sentido anti-horário - R
		}
}

ISR(ADC_vect)
{
	leitura_ADC = ADC; //cópia do valor ADC para uma variavél
	
	if (ADC==1023)
	{
		aux_pwm = 255;
	}
	else
	{
		aux_pwm = (ADC/4);
	}
	
}

int main(void)
{
		DDRC  = 0b11111110; //Definindo PC0 como entrada
		DDRB  = 0b11111111;
		DDRD  = 0b01000001; // PD0 e PD6 como saídas
		PORTD = 0b11001101;
		PORTC = 0b11111110; //Desabilita o pullup das entradas
		
		TCCR0A = 0b00000010; // Habilita modo CTC do TC0
		TCCR0B = 0b00000011; //Liga TC0 com prescaler = 64
		OCR0A = 249; // Ajusta o comparador para o TC0 contar ate 249
		TIMSK0 = 0b00000010; // Habilita a interrupção na igualdade de comparação OCR0A. A interrupção ocorre a cada 1ms = (64*(249+1))/16MHz
		
		EICRA = 0b00001010;
		EIMSK = 0b00000011;
		
		PCICR = 0b00000100; //Habilita a interrupção PCINT2
		PCMSK2= 0b00110010; // Habilitação do pino D1, D4 e D5 na interrupção PCINT2
		
		ADMUX = 0b11000000; // Tensão interna de ref (1.1V)
		ADCSRA = 0b11101111; //Habilita o AD, interrupção, conversão continua e coloca o prescaler em 128
		ADCSRB = 0x00; //Modo de conversão contínua
		DIDR0 = 0b00111110; //Habilita o pino PC5 como entrada do  AD0
		
		//Fast PWM, TOP = 0xFF, OC0A e OC0B habilitados
		TCCR0A = 0b10100011; //PWM não invertido nos pinos OC0A e OC0B
		TCCR0B = 0b00000011; //Liga TC0, prescaler = 64, fpwm = f0sc/(256*prescaler) = 16MHz/(256*64) = 976 Hz
		//OCR0A = aux_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256
		
		sei();
				
		nokia_lcd_init();
	
	while (1)
	{
		velocidade = 2*3.14*(tamroda/2)/100000*rpm*60; //Comprimento em Km = 2*3.14*(tamroda/2)/100000, como é 1:1 temos rpm*60 (Rotações por Hora)
		dist = aux_dist*velocidade/3600; // A cada 1s incremeta 1 em aux_dist assim temos X km por segundos
		//aux_pwm = ADC*256/1023; // aux que compara com o estouto de 8 bits
		OCR0A = aux_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256
		
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
		nokia_lcd_set_cursor(55, 33);
		nokia_lcd_write_string(display_string, 2);
		nokia_lcd_set_cursor(73, 35);
		nokia_lcd_write_string("Km", 1);
		
		nokia_lcd_set_cursor(0, 30);
		nokia_lcd_write_string("ADC:", 1);
		itoa(leitura_ADC, &display_string, 10); //chamada da função para converter inteiros em string
		nokia_lcd_set_cursor(25, 30);
		nokia_lcd_write_string(display_string, 1);
				
		/*itoa(pqp, &display_string, 10); //chamada da função para converter inteiros em string
		nokia_lcd_set_cursor(0, 40);
		nokia_lcd_write_string(display_string, 1);*/
		
		if (pqp==5)
		{
			nokia_lcd_set_cursor(0, 40);
			nokia_lcd_write_string("Modo:P", 1);
		}
			else if (pqp==11)
			{
				nokia_lcd_set_cursor(0, 40);
				nokia_lcd_write_string("Modo:D", 1);
			}
			else if (pqp==13)
			{
				nokia_lcd_set_cursor(0, 40);
				nokia_lcd_write_string("Modo:R", 1);
			}
				
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

