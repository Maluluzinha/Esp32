#ifndef SIGNAL_H
#define SIGNAL_H

#include <stdio.h>
#include <string.h>

#define N_MAX_PONTOS 1000 //Tamanho aleatório do sinal recebido - até 1000 amostras

extern float Sinalrecebido[]; //Declara Array

//Funções:
void printArray();
int numeroPontosRecebidos(float recebido[], int tamanho_max);

#endif // SIGNAL_H