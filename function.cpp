#include <stdio.h>
#include <string.h>
#include <math.h>
#include "function.h"
#include "signal.h"

float Mediasinal (float recebido[] , int Npontos){
  float soma;

    // Somar todos os elementos do array
  for (int i = 0; i < Npontos; i++) {
    soma += recebido[i];
  }
  // Calcular a média
  float media = soma / Npontos;
  return media;

}

float varianciaSinal (float recebido[] , int nPontos, float media){
  float mediaRecebida = media; //ou só chama a função da média Mediasinal(recebido, nPontos);
  float soma;

    // Calcular a soma dos quadrados das diferenças em relação à média
    for (int i = 0; i < nPontos; i++) {
        soma += (recebido[i] - mediaRecebida) * (recebido[i] - mediaRecebida);
    }
    // Calcular a variância
    float variancia = soma / (nPontos - 1);
    return variancia;

  //Melhor calcular a média dentro da função, ou já receber ela calculada no escopo?
}

float assimetriaSinal (float recebido[] , int nPontos){ //Calcula o coeficiente de momento de assimetria: m3 = Σ(xi − x¯)3 / n --> a3 = m3 / s3
  
  float mediaInicial = Mediasinal(recebido, nPontos);
  float varianciaInicial = varianciaSinal(recebido , nPontos, mediaInicial);
  //Desvio padrão:
  float desvioPadraoInicial = sqrt(varianciaInicial);
  
  //Obtendo m3:
  float soma;
  for (int i = 0; i < nPontos; i++) {
      soma += pow((recebido[i] - mediaInicial), 3);
  }
  float m3 = soma / nPontos;

  //Obtendo a3:
  float a3 = m3 / pow(desvioPadraoInicial, 3);
  return a3;

}

float curtoseSinal (float recebido[] , int nPontos){ //Calcula o coeficiente de momento de assimetria: m4 = Σ(xi − x¯)4 / n --> a4 = m4 / s4
  
  float mediaInicial = Mediasinal(recebido, nPontos);
  float varianciaInicial = varianciaSinal(recebido , nPontos, mediaInicial);
  //Desvio padrão:
  float desvioPadraoInicial = sqrt(varianciaInicial);
  
  //Obtendo m3:
  float soma;
  for (int i = 0; i < nPontos; i++) {
      soma += pow((recebido[i] - mediaInicial), 4);
  }
  float m4 = soma / nPontos;
  
  //Obtendo a3:
  float a4 = m4 / pow(desvioPadraoInicial, 4);
  return a4;

}