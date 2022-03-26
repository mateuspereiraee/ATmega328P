/*
 * MateusPereiraSprint2Certo.c
 *
 * Created: 10/12/2021 16:12:17
 * Author : mmate
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRB  = 0b11111111;
	DDRC  = 0b00001111;
	DDRD  = 0b11111100;
	PORTD = 0b00000011;
	
	/*
	N°  PC6 PC5 PC4 PC3  
	0    0   0   0   0    
	1    0   0   0   1     
	2    0   0   1   0   
	3    0   0   1   1  
	4    0   1   0   0  
	5    0   1   0   1  
	6    0   1   1   0   
	7    0   1   1   1  
	8    1   0   0   0  
	9    1   0   0   1  
	*/
	
	uint8_t dezena=0, centena=0, out=0;
	//uint8_t unidade=0; caso queira controlar de +1 ou -1
	
    while (1) 
    {
//////////////////////////////////// SE APERTAR O BOTÃO +10 CAI AQUI
		if (!(PIND & (1<<0)))
		{
			if (centena<3)
			{
				dezena++;
			}
			if (dezena==10)
			{
				if(centena<3)
				{
					centena++;
					dezena=0;
				}
				else
					{
						centena=3;
						dezena=0;
					}
			}
		}
		
//////////////////////////////////// SE APERTAR O BOTÃO -10 CAI AQUI
		if (!(PIND & (1<<1)))
		{
			if (dezena>0)
			{
				dezena--;
			}
			else
			{
				if (centena>0)
				{
					centena--;
					dezena=9;
				}
				else
				{
					dezena=0;
					centena=0;
				}
			}
		}
		
		_delay_ms(100);
		
		switch (dezena)
		{
		case 0:
			out = 0b00000000;
			break;
		case 1:
			out = 0b00010000;
			break;
		case 2:
			out = 0b00100000;
			break;
		case 3:
			out = 0b00110000;
			break;
		case 4:
			out = 0b01000000;
			break;
		case 5:
			out = 0b01010000;
			break;
		case 6:
			out = 0b01100000;
			break;
		case 7:
			out = 0b01110000;
			break;
		case 8:
			out = 0b10000000;
			break;
		case 9:
			out = 0b10010000;
			break;
		}
	PORTB = out;
	
	switch (centena)
	{
		case 0:
		out = 0b0000;
		break;
		case 1:
		out = 0b0001;
		break;
		case 2:
		out = 0b0010;
		break;
		case 3:
		out = 0b0011;
		break;
	}
	PORTC = out;
	
	}
}

