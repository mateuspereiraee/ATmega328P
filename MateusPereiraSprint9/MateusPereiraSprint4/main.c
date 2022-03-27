/*
 * MateusPereiraSprint4.c
 *
 * Created: 06/02/2022 14:28:31
 * Author : mmate
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "SSD1306.h"
#include "Font5x8.h"

#define tamanho_vetor 3
#define tem_delay 3
#define teste_porta(y,porta)(y&(1<<porta))

void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);

uint16_t bateria = 0, temperatura = 0, rotacao_motor_pwm = 0, aux_temp_eprom = 0, tamroda = 50, rpm = 0, dist = 0, velocidade = 0, tempo_ms = 0, mudanca = 0, aux_dist = 0, leitura_ADC = 0, leitura_ADC_Bateria = 0, aux_pwm_bateria = 0, leitura_ADC_Temperatura = 0, aux_pwm_temperatura = 0, aux_pwm = 0, modo_p = 0, marcha = 0, aux_dist_eeprom = 0;
uint16_t zerar_eeprom = 0;
uint32_t tempo_borda_subida = 0, distancia_delta_sonar = 0;
unsigned char display_string[tamanho_vetor];

ISR(INT0_vect) //Se Pressionar D2 (+1)
{
	tamroda++;
	eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda na posi��o 0
}

ISR(INT1_vect) //Se Pressionar D3 (-1)
{
	tamroda--;
	eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda na posi��o 0
}

ISR(TIMER0_COMPA_vect) // Interrup��o do TC0 a  cada 1ms
{
	static float aux;
	tempo_ms++;
	if((tempo_ms % 1000) == 0) // Entra a cada 1 segundo
	{
		aux = mudanca; // Vamos ter rota��es por segundo
		rpm = (aux*60);
		mudanca = 0;
		aux_dist++;	// Conta de 1 em 1 Segundo
	}
}

ISR(PCINT2_vect) // Interrup��o 2 por mudan�a de pino
{
	if (!teste_porta (PIND, PD7)) // Pega a varia��o em PD1
	{
		mudanca++; // Conta descida e subida
	}
	
	if (teste_porta(PIND,PD5)==1) // SE APERTAR O BOT�O D/R CAI AQUI
	{
		marcha = 13; // Motor sentido anti-hor�rio - R
	}
	else
	{
		marcha = 11; // Motor sentido hor�rio - D
	}
	
	if (teste_porta(PIND,PD4)==0) // SE APERTAR O BOT�O P CAI AQUI
	{
		marcha = 5; // carro parado - P
	}
}

ISR(ADC_vect) // Faz a Convers�o ADC
{
	
	if (!teste_porta (PINC, PC0)) // Pega a varia��o em PC0
	{
		leitura_ADC = ADC; //c�pia do valor ADC para uma variav�l
		
		if (ADC==1023) // Resolve o problema do limite superior
		{
			aux_pwm = 255;
			rotacao_motor_pwm = 255;
		}
		else
		{
			aux_pwm = (ADC/4);
			rotacao_motor_pwm = (ADC/4);
			
			if (distancia_delta_sonar<300 && velocidade>20) //Teste Para o sonar caso a dist seja menor que 300 cm ou 3 m
			{
				rotacao_motor_pwm = 102; //Regula a Rota��o do motor para 10%, tem 1024 posi��es
			}
				
			if (distancia_delta_sonar < 1)
			{
				rotacao_motor_pwm = 1; //Carro bateu
			}
		}	
	}


	if (!teste_porta (PINC, PC1)) // Pega a varia��o em PC1
	{
		leitura_ADC_Bateria = ADC; //c�pia do valor ADC para uma variav�l
		
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
	
	
	if (!teste_porta (PINC, PC2)) // Pega a varia��o em PC2
	{
		leitura_ADC_Temperatura = ADC; //c�pia do valor ADC para uma variav�l
		
		if (ADC==1023) // Resolve o problema do limite superior
		{
			aux_pwm_temperatura = 255;
			temperatura = (aux_pwm_temperatura - 105)/0.34;
		}
		else
		{
			aux_pwm_temperatura = (ADC/4);
			temperatura = (aux_pwm_temperatura - 105)/0.34;
			//temperatura = ADC*2597/(1023-ADC) - 259;
		}		
	}	
}

//SONAR
ISR(TIMER1_CAPT_vect) //Interrup��o por captura do valor do IC1
{
	if(TCCR1B & (1<<ICES1)) //L� o valor de contagem do TC1 na borda de subida do sinal
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

//Interrup��o para a USART
ISR(USART_RX_vect)
{
	char recebido;
	
	uint8_t contador_string = 0, contador_imprime_serial = 0;
	
	itoa(aux_temp_eprom, &display_string, 10); //chamada da fun��o para converter inteiros em string

	recebido = UDR0;
	
	if(recebido=='l') //Limpa o valor da temperatura m�xima e vai para o atual
	{
		//USART_Transmit(recebido);
		eeprom_write_byte(12,0);
	}
	if (recebido=='d') //Envia a m�xima temperatura
	{
		while(display_string[contador_string] != '\0') //busco o fim da string para saber o tamanho
		{
		   contador_string++;
		}

		for (contador_imprime_serial=0; contador_imprime_serial < contador_string; contador_imprime_serial++)
		{
			//Precisa enviar uma posi��o por vez
			USART_Transmit(display_string[contador_imprime_serial]);
			//_delay_ms(1000);
		}
		
		USART_Transmit(' '); //Espa�o para separar um valor e outro
	}
}

// Inicia a USART
void USART_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr>>8); //Ajusta a taxa de transmiss�o
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); // Habilita a interrup��o, receptor e transmissor
	UCSR0C = (1<<USBS0)|(3<<UCSZ00); // Ajusta o formato: 8 bits de dados e 2 de parada
}

void USART_Transmit(unsigned char data)
{
	while(!( UCSR0A & (1<<UDRE0))); // Espera limpar o registrador de transmiss�o
	UDR0 = data; // Coloca o dado no registrador e envia
}

unsigned char USART_Receive(void)
{
	while(!( UCSR0A & (1<<RXC0))); // Espera o dado ser recebido
	return UDR0; // L� e retorna o dado recebido
}



int main(void)
{
		DDRC  = 0b11111000; //Definindo PC0, PC1 e PC2 como entradas
		DDRB  = 0b11111110;
		DDRD  = 0b01000001; // PD0 e PD6 como sa�das
		PORTD = 0b01001101;
		PORTC = 0b11111000; //Desabilita o pullup das entradas
		PORTB = 0b00000001; //Pullups Ativos
		
		TCCR0A = 0b00000010; // Habilita modo CTC do TC0
		TCCR0B = 0b00000011; //Liga TC0 com prescaler = 64
		OCR0A = 249; // Ajusta o comparador para o TC0 contar ate 249
		TIMSK0 = 0b00000010; // Habilita a interrup��o na igualdade de compara��o OCR0A. A interrup��o ocorre a cada 1ms = (64*(249+1))/16MHz
		
		EICRA = 0b00001010;
		EIMSK = 0b00000011;
		
		//INTERRUP��ES PARA AS PORTAS D
		PCICR = 0b00000100; //Habilita a interrup��o PCINT2
		PCMSK2= 0b10110000; // Habilita��o do pino D7, D4 e D5 na interrup��o PCINT2
		
		
		//ADC - Configura��es
		//ADMUX = 0b11000000; // Tens�o interna de ref (1.1V), Canal 0
		//ADMUX = 0b11000001; // Tens�o interna de ref (1.1V), Canal 1
		//ADMUX = 0b11000010; // Tens�o interna de ref (1.1V), Canal 2
		ADCSRA = 0b11101111; //Habilita o AD, interrup��o, convers�o continua e coloca o prescaler em 128
		ADCSRB = 0x00; //Modo de convers�o cont�nua
		DIDR0 = 0b00111000; //Habilita o pino PC0, PC1 e PC2 como entrada do  ADC
		
		//Fast PWM, TOP = 0xFF, OC0A e OC0B habilitados
		TCCR0A = 0b10100011; //PWM n�o invertido nos pinos OC0A e OC0B
		TCCR0B = 0b00000011; //Liga TC0, prescaler = 64, fpwm = f0sc/(256*prescaler) = 16MHz/(256*64) = 976 Hz
		//OCR0A = aux_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256
		
		//SONAR
		TCCR1B = (1<<ICES1)|(1<<CS12); //Captura na borda de subida, TC1 com prescaler = 356.
		//Estouro a cada 256*(2^16)/16MHz = (2^16)*16us = 1,04s
		TIMSK1 = 1<<ICIE1; //Habilita interrup��o por captura
		
		//USART
		USART_Init(MYUBRR); // Inicia		
		
		sei();
		
		if (zerar_eeprom==1) //Zerar a mem�ria da EEPROM (mudar o valor da variav�l na declara��o)
		{
			eeprom_write_byte(0,tamroda);
			eeprom_write_byte(1,0);
		}
		
		if(eeprom_read_byte(1)==255)
		{
			eeprom_write_byte(1,aux_dist_eeprom);   //Salvo a dist�ncia inicialmente
												   //Pois logo quando liga o micro a memoria
												  //n�o ter� seu valor real j� que ainda n�o
			                                     //foi feita nenhuma atribui��o
		}
		
		if(eeprom_read_byte(0)==255)
		{
			eeprom_write_byte(0,tamroda); //Salvo o tamanho da Roda inicialmente,
										 //Pois logo quando liga o micro a mem�ria
										//n�o ter� seu valor real j� que ainda n�o
									   //foi pressionado nenhum bot�o
		}
		
		if(eeprom_read_byte(12)==255)
		{
			eeprom_write_byte(12,0); //Por padr�o a eeprom coloca 255 como valor
									//inicial, assim para limpar as primeiras compara��es
								   //com a temperatura m�xima inicia-se com zero
		}
		
		aux_dist_eeprom = eeprom_read_byte(1); //Carrego a dist�ncia pecorrida salvo na EEPROM
											  //em uma variav�l aux para n�o afetar a l�gica
		
		
		GLCD_Setup(); // Inicia o novo LCD
		GLCD_SetFont(Font5x8, 5, 8, GLCD_Overwrite); // Seta a fonte
		GLCD_InvertScreen(); // Deixa Branco com Letra Preta

			
	while (1)
	{
		velocidade = 2*3.14*(tamroda/2)/100000*rpm*60; //Comprimento em Km = 2*3.14*(tamroda/2)/100000, como � 1:1 temos rpm*60 (Rota��es por Hora)
		dist = aux_dist_eeprom + aux_dist*velocidade/3600; // A cada 1s incremeta 1 em aux_dist assim temos X km por segundos
		
		tamroda = eeprom_read_byte(0); //Carrego o tamnho da roda salvo na EEPROM
		eeprom_write_byte(1,dist); //Salvo a dist�ncia pecorrida na posi��o 1
		
		//aux_pwm = ADC*256/1023; // aux que compara com o estouto de 8 bits
		//OCR0A = rotacao_motor_pwm; //Controle do ciclo ativo do PWM OC0A (PD6), dutty = aux_pwm/256

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

		// Preciso mudar o canal a ser lido para fazer as convers�es ADC
		
		ADMUX = 0b11000000; // Tens�o interna de ref (1.1V), Canal 0
		_delay_ms(100);
		OCR0A = rotacao_motor_pwm;   //Controle do ciclo ativo do PWM do motor OC0A (PD6)
									//dutty = aux_motor_pwm/256
								   //por conta da vara��o da leitura ADC 
								  //tem que fazer esse controle aqui
		GLCD_GotoXY(40, 41);
		GLCD_PrintInteger(aux_pwm);
		GLCD_GotoXY(0, 41);
		GLCD_PrintString("Acele:");
		
		ADMUX = 0b11000001; // Tens�o interna de ref (1.1V), Canal 1
		_delay_ms(100);
		GLCD_GotoXY(90, 40);
		GLCD_PrintInteger(bateria);
		GLCD_GotoXY(110, 40);
		GLCD_PrintString("%");
		
		ADMUX = 0b11000010; // Tens�o interna de ref (1.1V), Canal 2
		_delay_ms(100);
		GLCD_GotoXY(90, 28);
		GLCD_PrintInteger(temperatura);
		GLCD_GotoXY(110, 28);
		GLCD_PrintString("'C");
		
		//EEPROM SERIAL
		aux_temp_eprom = eeprom_read_byte(12);
		
		if (temperatura > aux_temp_eprom)
		{
			eeprom_write_byte(12,temperatura);
		}

		
		if (marcha==11)
		{
			GLCD_GotoXY(90, 15);
			GLCD_PrintString("Modo:D");
		}
			else if (marcha==13)
			{
				GLCD_GotoXY(90, 15);
				GLCD_PrintString("Modo:R");
			}
			
		if (marcha==5)
		{
			GLCD_GotoXY(90, 15);
			GLCD_PrintString("Modo:P");
		}
				
		GLCD_Render(); // Vai Atualizar o Display
	
		PORTB &= 0b00000001;		
		PORTB |= 0b11000000; // Zero � aceso e liga s� o das unidades
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

