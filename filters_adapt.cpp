#include "filters_adapt.h"
#include "signal.h"
#include <stdio.h>
#include <math.h>
#include "defines.h"

float bqB0, bqB1, bqB2, bqA1, bqA2;
float bufFiltroA[SAMPLES_MAX];
float bufFiltroB[SAMPLES_MAX];

// Filtro de Segunda Ordem - "RLC"
void projetarSecaoPassaBanda(float f0, float Q, float fs) {
  float w0 = 2.0f * PI * f0 / fs;
  float alpha = sinf(w0) / (2.0f * Q);
  float cosw0 = cosf(w0);
  float a0 = 1.0f + alpha;

  bqB0 =  alpha        / a0;
  bqB1 =  0.0f;
  bqB2 = -alpha        / a0;
  bqA1 = -2.0f * cosw0 / a0;
  bqA2 = (1.0f - alpha) / a0;
}

// Aplicação do filtro em um sinal coletado
void aplicarSecao(const float* x, float* y, int n) {
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  for (int i = 0; i < n; i++) {
    float xn = x[i];
    float yn = bqB0 * xn + bqB1 * x1 + bqB2 * x2 - bqA1 * y1 - bqA2 * y2;
    y[i] = yn;
    x2 = x1; x1 = xn;
    y2 = y1; y1 = yn;
  }
}

float mediaQuadratica(const float* v, int n) { //Ponteiro + tamanho do cálculo
  double s = 0; // double evita perda de precisão -> 64 bits -> float só com 32 bits
  for (int i = 0; i < n; i++) {
  
  s += (double)v[i] * v[i];

  }
  return (float)(s / n);
}

// Filtra os segumentos e retorna a potência, tudo junto já
float potenciaDaBanda(const float* segmento, int n, float fs, float low, float high) {
  float f0 = sqrtf(low * high);      // Frequência central
  float Q  = f0 / (high - low);       // Fator de qualidade
  projetarSecaoPassaBanda(f0, Q, fs);

  const float* in = segmento;
  float* out = bufFiltroA;
  for (int s = 0; s < SECOES_POR_BANDA; s++) {
    aplicarSecao(in, out, n);
    in = out;
    out = (out == bufFiltroA) ? bufFiltroB : bufFiltroA;
  }
  return mediaQuadratica(in, n);

}