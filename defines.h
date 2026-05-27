#ifndef DEFINES_H
#define DEFINES_H

//DEFINES DOS FILTROS
#define N 1000  // Número de amostras
#define WINDOW_SIZE_MOBILE 5  // Janela do filtro média móvel
#define WINDOW_SIZE_NULL 3  // Janela do filtro fase nula
#define WINDOW_SIZE_BANDWIDTH 6  // Janela do passa banda

//DEFINES DO DISPLAY
#define MAX_DADOS 280 //Máximo de pontos pra plotar na tela
#define LARGURA_TELA 240
#define ALTURA_GRAFICO 150

//DEFINES DA FFT, obrigatoriamente potência de 2 nas amostras
#define SAMPLES 512          
#define SAMPLING_FREQ 1000

//DEFINES da potência das bandas
#define alfaBandMin 8
#define alfaBandMax 12
#define betaBandMin 13
#define betaBandMax 30
#define gamaBandMin 31
#define gamaBandMax 45

#endif