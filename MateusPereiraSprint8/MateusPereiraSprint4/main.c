/*
 * MateusPereiraSprint4.c
 *
 * Created: 06/02/2022 14:28:31
 * Author : mmate
 */ 

#define F_CPU 16000000UL
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "SSD1306.h"
#include "Font5x8.h"

#define tamanho_vetor 16
#define tem_delay 3
#define teste_porta(y,porta)(y&(1<<porta))

void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);

uint16_t bateria = 0, temperatura = 0, tamroda = 50, rpm = 0, dist = 0, velocidade = 0, tempo_ms = 0, mudanca = 0, aux_dist = 0, leitura_ADC = 0, leitura_ADC_Bateria = 0, aux_pwm_bateria = 0, leitura_ADC_Temperatura = 0, aux_pwm_temperatura = 0, aux_pwm = 0, modo_p = 0, pqp = 0, aux_dist_eeprom = 0;
uint16_t zerar_eeprom = 0;
uint32_t tempo_borda_subida = 0, distancia_delta_sonar = 0;
unsigned char display_string[tamanho_vetor];

ISR(INT0_vect) //Se Pressionar D2 (+1)
{
	tamroda++;
	eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda na posição 0
}

ISR(INT1_vect) //Se Pressionar D3 (-1)
{
	tamroda--;
	eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda na posição 0
}

ISR(TIMER0_COMPA_vect) // Interrupção do TC0 a  cada 1ms
{
	static float aux;
	tempo_ms++;
	if((tempo_ms % 1000) == 0) // Entra a cada 1 segundo
	{
		aux = mudanca; // Vamos ter rotações por segundo
		rpm = (aux*60);
		mudanca = 0;
		aux_dist++;	// Conta de 1 em 1 Segundo
	}
}

ISR(PCINT2_vect) // Interrupção 2 por mudança de pino
{
	if (!teste_porta (PIND, PD7)) // Pega a variação em PD1
	{
		mudanca++; // Conta descida e subida
	}
	
	if (teste_porta(PIND,PD5)==1) // SE APERTAR O BOTÃO D/R CAI AQUI
	{
		pqp = 13; // Motor sentido anti-horário - R
	}
	else
	{
		pqp = 11; // Motor sentido horário - D
	}
	
	if (teste_porta(PIND,PD4)==0) // SE APERTAR O BOTÃO P CAI AQUI
	{
		pqp = 5; // carro parado - P
	}
}

ISR(ADC_vect) // Faz a Conversão ADC
{
	
	if (!teste_porta (PINC, PC0)) // Pega a variação em PC0
	{
		leitura_ADC = ADC; //cópia do valor ADC para uma variavél
		
		if (ADC==1023) // Resolve o problema do limite superior
		{
			aux_pwm = 255;
		}
		else
		{
			aux_pwm = (ADC/4);
		}	
	}


	if (!teste_porta (PINC, PC1)) // Pega a variação em PC1
	{
		leitura_ADC_Bateria = ADC; //cópia do valor ADC para uma variavél
		
		if (ADC==1023) // Resolve o problema do limite superior
		{
			aux_pwm_bateria = 255;
			bateria = (aux_pwm_bateria * 100) / 255;
		}
		else
		{
			aux_pwm_bateria = (ADC/4);
			bateria = (aux_pwm_bateria * 100) / 255;
		}
	}
	
	
	if (!teste_porta (PINC, PC2)) // Pega a variação em PC2
	{
		leitura_ADC_Temperatura = ADC; //cópia do valor ADC para uma variavél
		
		if (ADC==1023) // Resolve o problema do limite superior
		{
			aux_pwm_temperatura = 255;
			temperatura = (aux_pwm_temperatura - 105)/0.34;
		}
		else
		{
			aux_pwm_temperatura = (ADC/4);
			temperatura = (aux_pwm_temperatura - 105)/0.34;
		}		
	}
	
	
	if (distancia_delta_sonar<300 && velocidade>20) //Teste Para o sonar caso a dist seja menor que 300 cm ou 3 m
	{
		aux_pwm = 102; //Regula a Rotação do motor para 10%, tem 1024 posições
	}
	
	
	if (distancia_delta_sonar < 1)
	{
		aux_pwm = 1; //Carro bateu
	}
	
}

//SONAR
ISR(TIMER1_CAPT_vect) //Interrupção por captura do valor do IC1
{
	if(TCCR1B & (1<<ICES1)) //Lê o valor de contagem do TC1 na borda de subida do sinal
	{
		tempo_borda_subida = ICR1;
	}
	else
	{
		distancia_delta_sonar = (ICR1 - tempo_borda_subida)*16/58; //Passa para us e depois converte para a formula do sonar
		if (distancia_delta_sonar==1724)
		{
			distancia_delta_sonar = 0;
		}
	}
	TCCR1B ^= (1<<ICES1); //Inverte a borda de captura
}

int main(void)
{
		DDRC  = 0b11111000; //Definindo PC0, PC1 e PC2 como entradas
		DDRB  = 0b11111110;
		DDRD  = 0b01000001; // PD0 e PD6 como saídas
		PORTD = 0b01001101;
		PORTC = 0b11111000; //Desabilita o pullup das entradas
		PORTB = 0b00000001; //Pullups Ativos
		
		TCCR0A = 0b00000010; // Habilita modo CTC do TC0
		TCCR0B = 0b00000011; //Liga TC0 com prescaler = 64
		OCR0A = 249; // Ajusta o comparador para o TC0 contar ate 249
		TIMSK0 = 0b00000010; // Habilita a interrupção na igualdade de comparação OCR0A. A interrupção ocorre a cada 1ms = (64*(249+1))/16MHz
		
		EICRA = 0b00001010;
		EIMSK = 0b00000011;
		
		//INTERRUPÇÕES PARA AS PORTAS D
		PCICR = 0b00000100; //Habilita a interrupção PCINT2
		PCMSK2= 0b10110000; // Habilitação do pino D7, D4 e D5 na interrupção PCINT2
		
		
		//ADC - Configurações
		//ADMUX = 0b11000000; // Tensão interna de ref (1.1V), Canal 0
		//ADMUX = 0b11000001; // Tensão interna de ref (1.1V), Canal 1
		//ADMUX = 0b11000010; // Tensão interna de ref (1.1V), Canal 2
		ADCSRA = 0b11101111; //Habilita o AD, interrupção, conversão continua e coloca o prescaler em 128
		ADCSRB = 0x00; //Modo de conversão contínua
		DIDR0 = 0b00111000; //Habilita o pino PC0, PC1 e PC2 como entrada do  ADC
		
		//Fast PWM, TOP = 0xFF, OC0A e OC0B habilitados
		TCCR0A = 0b10100011; //PWM não invertido nos pinos OC0A e OC0B
		TCCR0B = 0b00000011; //Liga TC0, prescaler = 64, fpwm = f0sc/(256*prescaler) = 16MHz/(256*64) = 976 Hz
		//OCR0A = aux_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256
		
		//SONAR
		TCCR1B = (1<<ICES1)|(1<<CS12); //Captura na borda de subida, TC1 com prescaler = 356.
		//Estouro a cada 256*(2^16)/16MHz = (2^16)*16us = 1,04s
		TIMSK1 = 1<<ICIE1; //Habilita interrupção por captura
		
		sei();
		
		if (zerar_eeprom==1) //Zerar a memória da EEPROM (mudar o valor da variavél na declaração)
		{
			eeprom_write_byte(0,tamroda);
			eeprom_write_byte(1,0);
		}
		
		if(eeprom_read_byte(1)==255)
		{
			eeprom_write_byte(1,aux_dist_eeprom);   //Salvo a distância inicialmente
												   //Pois logo quando liga o micro a memoria
												  //não terá seu valor real já que ainda não
			                                     //foi feita nenhuma atribuição
		}
		
		if(eeprom_read_byte(0)==255)
		{
			eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda inicialmente,
										 //Pois logo quando liga o micro a memória
										//não terá seu valor real já que ainda não
									   //foi pressionado nenhum botão
		}
		
		aux_dist_eeprom = eeprom_read_byte(1); //Carrego a distância pecorrida salvo na EEPROM
											  //em uma variavél aux para não afetar a lógica
		
		
		GLCD_Setup(); // Inicia o novo LCD
		GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite); // Seta a fonte
		GLCD_InvertScreen(); // Deixa Branco com Letra Preta

			
	while (1)
	{
		velocidade = 2*3.14*(tamroda/2)/100000*rpm*60; //Comprimento em Km = 2*3.14*(tamroda/2)/100000, como é 1:1 temos rpm*60 (Rotações por Hora)
		dist = aux_dist_eeprom + aux_dist*velocidade/3600; // A cada 1s incremeta 1 em aux_dist assim temos X km por segundos
		
		tamroda = eeprom_read_byte(0); //Carrego o tamnho da roda salvo na EEPROM
		eeprom_write_byte(1,dist); //Salvo a distância pecorrida na posição 1
		
		//aux_pwm = ADC*256/1023; // aux que compara com o estouto de 8 bits
		OCR0A = aux_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256
		

		GLCD_Clear();
		
		GLCD_GotoXY(22, 1);
		GLCD_PrintString("COMP. DE BORDO");
		
		GLCD_GotoXY(0, 15);
		GLCD_PrintString("Diam(cm):");
		GLCD_GotoXY(55, 15);
		GLCD_PrintInteger(tamroda);

		GLCD_GotoXY(0, 28);
		GLCD_PrintString("RPM:");
		GLCD_GotoXY(25, 28);
		GLCD_PrintInteger(rpm);
		
		//SONAR
		GLCD_GotoXY(0, 54);
		GLCD_PrintString("Sonar:");
		GLCD_GotoXY(37, 54);
		GLCD_PrintInteger(distancia_delta_sonar);
		
		GLCD_GotoXY(90, 67);
		GLCD_PrintInteger(dist);
		GLCD_GotoXY(110, 67);
		GLCD_PrintString("Km");

		// Preciso mudar o canal a ser lido para fazer as conversões ADC
		
		ADMUX = 0b11000000; // Tensão interna de ref (1.1V), Canal 0
		_delay_ms(100);
		GLCD_GotoXY(40, 41);
		GLCD_PrintInteger(aux_pwm);
		GLCD_GotoXY(0, 41);
		GLCD_PrintString("Acele:");
		
		ADMUX = 0b11000001; // Tensão interna de ref (1.1V), Canal 1
		_delay_ms(100);
		GLCD_GotoXY(90, 40);
		GLCD_PrintInteger(bateria);
		GLCD_GotoXY(110, 40);
		GLCD_PrintString("%");
		
		ADMUX = 0b11000010; // Tensão interna de ref (1.1V), Canal 2
		_delay_ms(100);
		GLCD_GotoXY(90, 28);
		GLCD_PrintInteger(temperatura);
		GLCD_GotoXY(110, 28);
		GLCD_PrintString("'C");

		
		if (pqp==11)
		{
			GLCD_GotoXY(90, 15);
			GLCD_PrintString("Modo:D");
		}
			else if (pqp==13)
			{
				GLCD_GotoXY(90, 15);
				GLCD_PrintString("Modo:R");
			}
			
		if (pqp==5)
		{
			GLCD_GotoXY(90, 15);
			GLCD_PrintString("Modo:P");
		}
				
		GLCD_Render(); // Vai Atualizar o Display
	
		PORTB &= 0b00000001;		
		PORTB |= 0b11000000; // Zero é aceso e liga só o das unidades
		PORTB |= ((((velocidade/1)%10)*2) & 0b00011110);
		_delay_ms(tem_delay);
		
		PORTB &= 0b00000001;
		PORTB |= 0b10100000;
		PORTB |= ((((velocidade/10)%10)*2) & 0b00011110);
		_delay_ms(tem_delay);
		
		PORTB &= 0b00000001;
		PORTB |= 0b01100000;
		PORTB |= ((((velocidade/100)%10)*2) & 0b00011110);
		_delay_ms(tem_delay);
	}
}

