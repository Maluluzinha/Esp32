#include <stdio.h>
#include <string.h>
#include "signal.h"
#include "function.h"
#include <Arduino.h>

//#define TAMANHO_SINAL 1000 //Tamanho aleatório do sinal recebido - até 1000 amostras


void setup()
{

  // Exemplo de uso da função
  float dados[] = {1.2, 2.3, 3.4, 4.5, 5.6};
  int numeroDeDados = sizeof(dados) / sizeof(dados[0]);
  
  // Inicializa a comunicação serial
    Serial.begin(9600);

    // Chama a função para imprimir o array. Funciona!!!
    //printArray();

    //Funções de teste com um sinal pequeno e conhecido
    float mediateste = Mediasinal(dados, 5);
    Serial.print("A média do sinal recebido pelo teste é: ");
    Serial.println(mediateste);

    float varianciateste = varianciaSinal(dados , 5, mediateste);
    Serial.print("A variância do sinal recebido pelo teste é: ");
    Serial.println(varianciateste);


    //Teste com arrays longos
    int numeroPontos = numeroPontosRecebidos(Sinalrecebido, N_MAX_PONTOS);
    Serial.print("Pontos recebidos: ");
    Serial.println(numeroPontos);

    float media = Mediasinal(Sinalrecebido, numeroPontos);
    Serial.print("A média do sinal recebido pelo array é: ");
    Serial.println(media);

    float variancia = varianciaSinal(Sinalrecebido , numeroPontos, media);
    Serial.print("A variância do sinal recebido pelo array é: ");
    Serial.println(variancia);

    float assimetria = assimetriaSinal(Sinalrecebido, numeroPontos);
    Serial.print("A assimetria do sinal recebido pelo array é: ");
    Serial.println(assimetria);

    float curtose = curtoseSinal(Sinalrecebido, numeroPontos);
    Serial.print("A curtose do sinal recebido pelo array é: ");
    Serial.println(curtose);

}
void loop()
{
  //
}

