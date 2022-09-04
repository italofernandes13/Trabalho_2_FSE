# FSE - Trabalho 2 - 2022-1

Repositório focado no trabalho 2 da matéria de Fundamentos de sistemas embarcados feito no Rasp43 e Rasp44

## Aluno
|Matrícula | Aluno |
| -- | -- |
| 18/0102613  |  Ítalo Fernandes Sales de Serra |

## Objetivo

Este trabalho tem por objetivo a implementação de um sistema (que simula) o controle de uma Air Fryer. Abaixo vemos alguns exemplos de fornos comerciais para este propósito.

**Linguagem**: 
- C<br>

**Bibliotecas**: 

Por meio das bibliotecas em C listadas abaixo, consegui manipular os dispositivos disponíveis para que assim manipulassem em dois tipos de operação o forno de soldagem.

 - stdlib.h
 - string.h
 - stdio.h
 - signal.h
 - wiringPi.h
 - wiringPiI2C.h
 - unistd.h
 - softPwm.h
 - time.h
 - linux/types.h
 - linux/kernel.h
 - linux/i2c-dev.h
 - stdint.h
 - stddef.h
 - sys/ioctl.h
 - sys/types.h
 - string.h
 - fcntl.h
 - termios.h

O funcionamento se dá por meio da dashboard disponibilizada pelo professor e assim a cada manipulação o código trata estes eventos.

Neles é possível escolher um modo de funcionamento, sendo ele, um setup pre-definido ou no modo manual;
Pre-definido possui 4 alimentos:
- Batata Frita simbolizada por 'B'
- Peixe simbolizada por 'P'
- Frango simbolizada por 'F'
- Carne simbolizada por 'C'

O sistema inicia quando pressionar o botão de "Ligar", logo em seguida é escolhido entre o modo manual ou pre-definido pressionando no botão "Menu", e depois pressionando o botão "iniciar", o sistema irá pre-aquecer até chegar na temperatura desejada. Com  a chegada na temperatura o timer inicializará, e acabando esse processo ou pressionando o botão "Parar" ele irá resfriar o sistema. Para encerrar o programa deve pressionar o botão "Desligar".

## Experimento

As informações necessárias dos experimentos foram salvas em "experimento.csv" que é populado com a data, hora, temperatura interna, temperatura ambiente, temperatura referencia.


## Requisitos

Para executar o código necessita-se de um ambiente raspbian, por isso é necessário conectar á uma Raspberry Pi, cuja qual foi utilizada para desenvolver este projeto.

## Uso

Para executar o código apresentado , precisa-se clonar este repositório. acessar a pasta clonada e executar o comando abaixo:

Siga as instruções a seguir :

1) Clonar o repositório:
```sh 
git clone https://github.com/italofernandes13/Trabalho_2_FSE.git
```

2) Acessar a pasta da aplicação:
```sh
cd Trabalho_2_FSE
```

3) Copie a pasta para a placa rasp inserindo seu user no lugar de <user_>, seu caminho ao inves de <insira/seu/caminho/ate/Trabalho_1_FSE> e o IP ao inves de <000.00.00.00>:
```
scp -P 13508 -r "<insira/seu/caminho/ate/Trabalho_2_FSE>" <user_>@<000.00.00.00>:/home/<user_>
```
exemplo:
```
scp -P 13508 -r ~/Downloads/Trabalho_2_FSE italoserra@164.41.98.15:/home/italoserra
```

4) Acessar via ssh a placa rasp inserindo seu user no lugar de <user_> e o IP ao inves de <000.00.00.00>:
```
ssh <user_>@<000.00.00.00> -p 13508
```
exemplo:
```
ssh italoserra@164.41.98.15 -p 13508
```

6) Acessar a pasta da aplicação no ssh:
```sh
cd Trabalho_2_FSE
```

7) Compile e Execute:
```sh
make
```
