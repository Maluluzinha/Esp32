
#include <Arduino.h>
#include "signal.h"
#include "statics.h"
#include "filters.h"
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include "arduinoFFT.h"
#include "defines.h"

/*
A FAZER:
TESTAR FILTROS NOVAMENTE NA TFT -> OK
MOVER FILTROS PARA A ABA FILTERS.CPP -> OK 
TESTAR NOVAMENTE NA TELINHA -> OK
COLOCAR A POTÊNCIA NO GRÁFICO DE BARRAS -> OK
MOVER FILTRAGEM PRO VOID LOOP
TESTAR COMUNICAÇÃO SERIAL COM OS FILTROS
TESTAR OUTRAS FUNÇÕES PARA PLOT
IMPLEMENTAR O TOUCH OU BOTÃO
*/

/*--------------------------------------- DEFINES ----------------------------------------------*/

//GLOBAL VAR
float meusDadoss[MAX_DADOS];
int totalDados = 0;
float dados[150];
int contadorDados = 0;

/*--------------------------------------- VARIAVEIS DOS FILTROS----------------------------------------------*/
float x[N];  // Sinal de entrada (ruído)
//Filtro Média Móvel:
float filteredSignal[N]; // Sinal filtrado (média móvel)
float nullSignal[N]; // Sinal de fase nula
//Filtro Passa-Banda:
float bandwidthFilterSignal[N]; // Sinal passado a banda
//Filtro Notch
float notchFilterSignal[N]; // Sinal notch

/*-------------------------------------- CONFIG DISPLAY -----------------------------------------------*/

//COMUNICAÇÃO SERIAL
HardwareSerial SerialPort(2); // UART2
TFT_eSPI tft = TFT_eSPI();

/*-------------------------------------- CONFIG PRA FFT (antigo) -----------------------------------------------*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const float signalFrequency = 1000;
const float samplingFrequency = 5000;
const uint8_t amplitude = 100;

/*-------------------------------------- CONFIG PRA FFT NOVO -----------------------------------------------*/
//FFT
float vReal[SAMPLES];
float vImag[SAMPLES];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLING_FREQ);

/*--------------------------------------- FUNÇÕES PARA FFT --------------------------------------------------*/

void alocarPotenciaEEG(float alocarDados[]) {
  //Copia os dados do array de sinal e zera o imaginário pra FFT
  for (int i = 0; i < SAMPLES; i++) {
    //vReal[i] = sinal[i];
    vReal[i] = alocarDados[i];
    vImag[i] = 0.0;
  }
}

// Função matemática para varrer o espectro e somar as potências no intervalo desejado
float calcularPotenciaIntervalo(float f_min, float f_max) {
  float soma_potencia = 0.0;
  
  // A FFT gera SAMPLES/2 de frequências úteis (Teorema de Nyquist)
  for (int i = 0; i < (SAMPLES / 2); i++) {
    // Calcula qual frequência em Hz aquele ponto do array representa
    float freq_atual = i * ((float)SAMPLING_FREQ / SAMPLES);
    
    if (freq_atual >= f_min && freq_atual <= f_max) {
      // Potência é o quadrado da magnitude (amplitude^2)
      soma_potencia += powf(vReal[i], 2); 
    }
  }
  return soma_potencia;
}

/*--------------------------------------- FUNÇÕES PARA PLOT --------------------------------------------------*/

//Adicionar dado + array destino + valor lido
void adicionarDadoCircular(float meusDados[], float valor) {
 // Adiciona ao array
  if (totalDados < MAX_DADOS) {
    meusDados[totalDados] = valor;
    totalDados++;
  } else {
    // Desliza todos os valores
    for (int i = 0; i < MAX_DADOS - 1; i++) {
      meusDados[i] = meusDados[i + 1];
    }
    meusDados[MAX_DADOS - 1] = valor;
  }
}

//Desenhar gráfico dinâmico
void desenharGraficoCompleto(float meusDados[], float valor) {
  tft.fillRect(11, 31, 298, 198, TFT_BLACK); //Função para redesenhar APENAS a área que o gráfico ta sendo plotado
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Dados: " + String(valor), 150, 10, 2);
 
 for (int i = 0; i < totalDados - 1; i++) {
    int x1 = 11 + i;
    int y1 = 180 - meusDados[i] * 1.5;  // Escala fixa
    int x2 = 11 + i + 1;
    int y2 = 180 - meusDados[i + 1] * 1.5;
   
    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
  }
}

//Desenhar gráfico estático
void desenharGraficoCompletoUnico(float meusDados[], float valor) {

 for (int i = 0; i < totalDados - 1; i++) {
    int x1 = 11 + i;
    int y1 = 180 - meusDados[i] * 1.5;  // Escala fixa
    int x2 = 11 + i + 1;
    int y2 = 180 - meusDados[i + 1] * 1.5;
   
    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
  }

}

void desenharGraficoBarrasEEG (float potAlfa, float potBeta, float potGama) {

  /*OBS: tft.fillRect(x, y, largura, altura, cor);
  x = ponto de inicio, y = ponto final
  largura = o quanto ele se extende no eixo x -> direita
  altura = o quanto ele se extende no eixo y -> baixo
  cor = cor */

  static int largurasAntigas[NUMERO_DE_BARRAS];

  float potencias[NUMERO_DE_BARRAS] = {potAlfa, potBeta, potGama};
  const char* nomes[NUMERO_DE_BARRAS] = {"ALFA", "BETA", "GAMA"};
  uint16_t cores[NUMERO_DE_BARRAS] = {TFT_CYAN, TFT_GREEN, TFT_MAGENTA};

  //Escala de maior potência
  float maxPotencia;
  for (int i = 0; i < NUMERO_DE_BARRAS; i++) {
    if (potencias[i] > maxPotencia)
      maxPotencia = potencias[i];
  }
  
  for (int i = 0; i < NUMERO_DE_BARRAS; i++) {
  //Calculo do y para desenho em relação a posição da última barra
    int y_desenho = 60 + (i * ESPACAMENTO_BARRA);

    // Escreve o nome da banda
    tft.setTextColor(cores[i], TFT_BLACK);
    tft.setTextSize(1.5);
    tft.drawString(nomes[i], 15, y_desenho - 20);

  // Calcula a largura proporcional da barra em pixels
    int nova_largura = (potencias[i] / maxPotencia) * LARGURA_MAX_BARRA;
    if (nova_largura > LARGURA_MAX_BARRA) {
    nova_largura = LARGURA_MAX_BARRA;
    }

    // Se a barra cresceu, desenha a mais - Usa o texto de base, mas pode ser uma margem
    if (nova_largura > largurasAntigas[i]) {
      tft.fillRect(TEXTO_BARRA + largurasAntigas[i], y_desenho, nova_largura - largurasAntigas[i], ALTURA_BARRA, cores[i]);
    
    }
    // Se a barra diminuiu, apaga o excesso com o fundo
    else if (nova_largura < largurasAntigas[i]) {
      tft.fillRect(TEXTO_BARRA + nova_largura, y_desenho, largurasAntigas[i] - nova_largura, ALTURA_BARRA, TFT_BLACK);
    }
    // Largura base anterior
    largurasAntigas[i] = nova_largura;
}

}


/*--------------------------------------- SETUP ----------------------------------------------*/
void setup() {

    Serial.begin(115200);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(10, 30, 300, 200, TFT_WHITE); // drawRect(0, 0, COMPRIMENTO_max, ALTURA_max)
    tft.drawString("Grafico Potencia", 20, 10, 2); // tft.drawString("String", eixo x, eixo y, tamanho da letra);

    alocarPotenciaEEG(Sinalrecebido);

    // Gerar um sinal aleatório (simulando randn do MATLAB)
    randomSeed(analogRead(0));  // Inicializa a semente aleatória
    for (int i = 0; i < N; i++) {
        x[i] = (random(-1000, 1000) / 1000.0); // Normalizado entre -1 e 1
    }

    //Sinal de ruído de 50Hz pra teste
    for (int i = 0; i < 100; i++) {
        //x_noised[i] = (random(-100, 100) / 100.0); 
        //ruído
        float fnoise = 50;
        //float noise = 10 * sin(2*3.14*fnoise*t);
        //x_noised[i] = x_noised[i] + noise;
    }

}

void loop() {

  //Janela de Hamming
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  //Converte os números complexos resultantes em magnitudes (amplitude)
  FFT.complexToMagnitude();
  
  // 5. Isola as bandas de potência combinando os "bins" de frequência
  float pot_alfa = calcularPotenciaIntervalo(alfaBandMin, alfaBandMax);   
  float pot_beta  = calcularPotenciaIntervalo(betaBandMin, betaBandMax); 
  float pot_gama = calcularPotenciaIntervalo(gamaBandMin, gamaBandMax); 

  // Exibe os resultados no Monitor Serial
  Serial.println("RESULTADOS DA ANÁLISE DE POTÊNCIA");
  Serial.printf("Potência Alfa: %.2f\n", pot_alfa);
  Serial.printf("Potência Beta: %.2f\n", pot_beta);
  Serial.printf("Potência Gama: %.2f\n", pot_gama);

  desenharGraficoBarrasEEG(pot_alfa, pot_beta, pot_gama);
  
  delay(1000);
   
}
