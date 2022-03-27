# ATmega328P
 Projeto - Carro Elétrico

Esse projeto foi elaborado na disciplina do Laboratorio de Arquitetura de Sistemas Digitais do Curso de Engenharia Elétrica da UFCG, utilizou-se para sua simulação os seguites softwares:
   - Atmel Studio 7: https://www.microchip.com/mplab/avr-support/atmel-studio-7
   - SIMULIDE: https://www.simulide.com/p/blog-page.html

O projeto final se encontra em "MateusPereiraSprint10" e foram adicionados 2 circuitos na silulação:
   - Sonoro: Adicionou-se uma Buzina para o veículo, que é ativada por meio de um relé, esse faz o controle de acionamento por um botão de retorno por mola ou pelo microcontrolador quando a distância entre o veícilo e algum objeto é menor que 3 metros.
   - Sirene Policial: Para uso final, tem-se o veículo como um carro policial e sua sirene é ligada enviando o comando 'p' no monitor serial e tmabém é desligada enviando o caractere 'p'. A sirene foi contruída com um circuito flip-flop, que tem como base o carregamento de 2 capacitores e a saturação dos seus 2 trasistores, sendo que quando um dos capacitores satura um dos trasistores o outro carrega. Projetado da naneira certa pode-se piscar os leds com apenas uma uníca porta do microcontrolador.

