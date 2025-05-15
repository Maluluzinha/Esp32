#include "filters.h"

void filtroMediaMovel (float recebido[], int widowSize, int Npontos){
  // Aplicação do filtro de média móvel (fase linear)
    for (int n = widowSize; n < Npontos - widowSize; n++) {
      float filteredSignal[n];
      filteredSignal[n] = filteredSignal[n - 1] + recebido[n] - recebido[n - widowSize];
    }

}