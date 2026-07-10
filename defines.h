#ifndef DEFINES_H
#define DEFINES_H

/* 
  320x240 pixels, rotação com o set rotation (1) -> HORIZONTAL

  ────────────────────────────────────────────────────  y=0
│                                                    |
│  BARRAS (x=0..159)    │  LINHAS (x=160..319)       |
│                       │                            │
│  DELTA ███            │  ___                       │
│  THETA ███            │ /   \    /\                │
│  ALFA  ███            │/     \  /  \___            │
│  BETA  ███            │       \/                   │
│  GAMA  ███            │                            │
│                       │                            │
  ───────────────────────────────────────────────────  y=240
x=0                   x=160                        x=320      

*/

//DEFINES DOS FILTROS
#define N 1000  // Número de amostras
#define WINDOW_SIZE_MOBILE 5  // Janela do filtro média móvel
#define WINDOW_SIZE_NULL 3  // Janela do filtro fase nula
#define WINDOW_SIZE_BANDWIDTH 6  // Janela do passa banda
#define SAMPLES_MAX 512      // Freq de coleta do sinal de EEG
#define SECOES_POR_BANDA 2   
#define PI 3.14 

//DEFINES DO DISPLAY - Gráfico de linhas
#define MAX_DADOS 280 //Máximo de pontos pra plotar na tela
#define LARGURA_TELA 240
#define ALTURA_GRAFICO 150

//DEFINES DO DISPLAY - Gráfico de barras
#define TEXTO_BARRA 15  // Recuo horizontal da barra
//#define LARGURA_MAX_BARRA 250  // Tamanho máximo da barra dentro da tela
#define LARGURA_MAX_BARRA 120
#define ALTURA_BARRA 10   // Espessura da barra
#define ESPACAMENTO_BARRA 40   // Distância vertical entre barras
#define NUMERO_DE_BARRAS 5

//DEFINES DA FFT, obrigatoriamente potência de 2 nas amostras
#define SAMPLES 512          
#define SAMPLING_FREQ 1000

//DEFINES da potência das bandas
#define deltaBandMin 0.5
#define deltaBandMax 4
#define tethaBandMin 4
#define tethaBandMax 8
#define alfaBandMin 8
#define alfaBandMax 13
#define betaBandMin 13
#define betaBandMax 30
#define gamaBandMin 30
#define gamaBandMax 80

// DEFINES de divisão da tela em 2 - coordenadas
#define DIVISOR_X 160            // Linha vertical que separa barras (esquerda) de sinal (direita)
#define BARRAS_X0 11             // Inicio da area de barras
#define BARRAS_X1 155            // Fim da area de barras
#define BARRAS_Y0 31
#define BARRAS_Y1 229
#define SINAL_X0 165              // Inicio da area do grafico de sinal
#define SINAL_X1 309
#define SINAL_Y0 31
#define SINAL_Y1 229



#endif