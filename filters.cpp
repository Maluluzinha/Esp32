#include "filters.h"
#include "signal.h"
#include <stdio.h>
#include <math.h>

void filtroMediaMovel (float recebido[], int widowSize, int Npontos){
  // Aplicação do filtro de média móvel (fase linear)
    for (int n = widowSize; n < Npontos - widowSize; n++) {
      float filteredSignal[n];
      filteredSignal[n] = filteredSignal[n - 1] + recebido[n] - recebido[n - widowSize];
    }

}

void filtroMediaMovelFaseNula (float recebido[], int widowSize, int Npontos){
  // Aplicação do filtro de média móvel (fase nula)
    for (int n = widowSize; n < Npontos - widowSize; n++){
    //for (int n = WINDOW_SIZE_NULL; n < N - WINDOW_SIZE_NULL; n++) {
      float nullSignal[n];
        nullSignal[n] = nullSignal[n - 1] + recebido[n + 2] - recebido[n - widowSize];
    //}
    }
}

void filtroPassaBanda (float recebido[], int widowSize, int Npontos){
    for (int n = widowSize; n < Npontos - widowSize; n++){
    float bandwidthFilterSignal[n];
        bandwidthFilterSignal[n] = recebido[n + 6] - recebido[n - 6] - 2*bandwidthFilterSignal[n - 2] + 2*bandwidthFilterSignal[n - 4] - bandwidthFilterSignal[n - 6];
    }

}

//void filtroNotch (float recebido[], int widowSize, int Npontos){
  //float b1 = 0.3;
   // for (int n = widowSize; n < Npontos - widowSize; n++){
    //  float notchFilterSignal[n];
    //    notchFilterSignal[n] = recebido[n] + 2*recebido[n - 1] - recebido[n - 2] - 2*b1*notchFilterSignal[n - 1] - pow(b1,2)*notchFilterSignal[n - 2];
   // }

//}

void filtroMediaMovelNovo(const float* recebido, float* filtrado, int windowSize, int nPontos) {
  // Primeira janela ainda não tem os valores futuros para ser -1
  float soma = 0.0f;
  for (int i = 0; i < windowSize; i++) soma += recebido[i];
  filtrado[windowSize - 1] = soma / windowSize;
 
  //atualizacao recursiva
  for (int n = windowSize; n < nPontos; n++) {
    soma += recebido[n] - recebido[n - windowSize];
    filtrado[n] = soma / windowSize;
  }
 
  //Enquanto não há sinal o suficiente
  for (int i = 0; i < windowSize - 1 && i < nPontos; i++) {
  filtrado[i] = recebido[i];
  }

}
 



