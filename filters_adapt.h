#ifndef FILTERS_ADAPT_H
#define FILTERS_ADAPT_H

/*
Potencia de banda agora e calculada no dominio do tempo, filtrando o sinal com filtros IIR (biquadrada) derivados da equacao
diferencial de um filtro passa-banda de 2a ordem: y''(t) + (w0/Q) y'(t) + w0^2 y(t) = (w0/Q) x'(t) discretizada via transformada bilinear. 
Cada banda usa 2 secoes em cascata filtro de 4a ordem) para uma separacao mais nitida entre bandas vizinhas.
- Sinal usado vem de Sinalrecebido, com janela deslizante de SAMPLES pontos para simular aquisicao continua.
*/

void projetarSecaoPassaBanda(float f0, float Q, float fs);
void aplicarSecao(const float* x, float* y, int n) ;
float mediaQuadratica(const float* v, int n);
float potenciaDaBanda(const float* segmento, int n, float fs, float low, float high);

#endif
