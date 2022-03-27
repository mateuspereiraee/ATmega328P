# ATmega328P
 Projeto - Carro Elétrico

Esse projeto foi elaborado na disciplina do Laboratório de Arquitetura de Sistemas Digitais do Curso de Engenharia Elétrica da UFCG, utilizou-se para sua simulação os seguintes softwares:
   - Atmel Studio 7: https://www.microchip.com/mplab/avr-support/atmel-studio-7
   - SIMULIDE: https://www.simulide.com/p/blog-page.html

O projeto final se encontra em "MateusPereiraSprint10" e foram adicionados 2 circuitos na simulação:
   - Sonoro: Adicionou-se uma Buzina para o veículo, que é ativada por meio de um relé, esse faz o controle de acionamento por um botão de retorno por mola ou pelo microcontrolador quando a distância entre o veículo e algum objeto é menor que 3 metros.
   - Sirene Policial: Para uso final, tem-se o veículo como um carro policial e sua sirene é ligada enviando o comando 'p' no monitor serial e também é desligada enviando o caractere 'p'. A sirene foi construída com um circuito flip-flop, que tem como base o carregamento de 2 capacitores e a saturação dos seus 2 transistores, sendo que quando um dos capacitores satura um dos transistores o outro carrega. Projetado da maneira certa pode-se piscar os leds com apenas uma única porta do microcontrolador.

