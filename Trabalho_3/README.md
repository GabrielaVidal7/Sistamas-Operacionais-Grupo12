# Sistamas-Operacionais-Grupo12
Trabalho 3 da matéria de Sistemas Operacionais I (SSC0640), ministrada pelo professor Julio Cezar Estrella no primeiro semestre de 2020.

## Especificações do trabalho:
O trabalho foi feito em C, por conta da familiaridade que os membros apresentam com essa linguagem de programação. Está comentado em algumas linhas.

Nesse trabalho, considerou-se o tamanho das páginas e dos processos criados como sendo em kBs. Abaixo está um exemplo de entrada de arquivo que deve ser feita para rodar esse programa.

Obs: o formato de arquivo deve ser .txt e o nome do arquivo deve ser necessariamente "entrada"
```
P1 C 5
P1 R 0
P1 R 1024
P1 P 1
P1 R 2
P1 P 2
P1 W 1024
P7 C 10
P7 R 4095
P7 R 800
P7 I 2
P7 R 801
P7 W 4096
P1 R 3
P1 R 4
P1 W 1025
P1 W 1026
```

O processo é inicializado como sendo executado pela CPU. Ele só muda para I/O quando for enviado um comando para o programa pedindo para mudar (número-do-processo I).

Como entrada do programa há 4 dados que são necessários:
```
Tamanho das páginas e quadros de página: 2

Tamanho em bits do endereco logico: 8

Quantas paginas de memoria fisica serao alocadas: 10

Tamanho maximo da memoria secundaria: 10
```
O primeiro é o tamanho em kBs da página, o segundo o tamanho de cada endereço em bits, em seguida o tamanho da memória principal em páginas e por fim o tamanho da memória secundária em páginas.

