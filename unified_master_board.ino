#include <Arduino.h>
#include "signal.h"
#include "statics.h"
#include "filters.h"
#include "filters_adapt.h"
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include "defines.h"

/*
A FAZER:
TESTAR FILTROS NOVAMENTE NA TFT -> OK
MOVER FILTROS PARA A ABA FILTERS.CPP -> OK
TESTAR NOVAMENTE NA TELINHA -> OK
COLOCAR A POTENCIA NO GRAFICO DE BARRAS -> OK
TESTAR OUTRAS FUNCOES PARA PLOT -> OK (grafico de sinal adicionado)
IMPLEMENTAR O TOUCH OU BOTAO
*/

/*--------------------------------------- COMUNICACAO / DISPLAY ----------------------------------------------*/
HardwareSerial SerialPort(2); // UART2
TFT_eSPI tft = TFT_eSPI();

/*-------------------------------------- JANELA DESLIZANTE SOBRE O SINAL --------------------------*/
// Varre o buffer Sinalrecebido em janelas de SAMPLES pontos, avancando STEP pontos por iteracao, simulando aquisicao continua.
constexpr int STEP = 16;
int startIndex = 0;

float segmentoAtual[SAMPLES]; // janela atual do sinal
float segmentoAtualFiltro[SAMPLES]; // janela atual do sinal bruto (dominio do tempo)

/*--------------------------------------- VARIAVEIS PARA TESTE DE FILTROS --------------------------------------------------*/
float SinalrecebidoMediaMovel[N_MAX_PONTOS];

/*--------------------------------------- FUNCOES PARA PLOT --------------------------------------------------*/

// Grafico de barras - mostra POTENCIA RELATIVA (% do total das 5 bandas)
void desenharGraficoBarrasEEG(float potDelta, float potTetha, float potAlfa, float potBeta, float potGama) {
  static int largurasAntigas[NUMERO_DE_BARRAS];

  float potencias[NUMERO_DE_BARRAS] = {potDelta, potTetha, potAlfa, potBeta, potGama};
  const char* nomes[NUMERO_DE_BARRAS] = {"DELTA", "THETA", "ALFA", "BETA", "GAMA"};
  uint16_t cores[NUMERO_DE_BARRAS] = {TFT_CYAN, TFT_GREEN, TFT_MAGENTA, TFT_BLUE, TFT_YELLOW};

  float total = 0;
  for (int i = 0; i < NUMERO_DE_BARRAS; i++) total += potencias[i];
  if (total <= 0) total = 1; // evita divisao por zero

  for (int i = 0; i < NUMERO_DE_BARRAS; i++) {
    int y_desenho = 45 + (i * ESPACAMENTO_BARRA);
    float percentual = (potencias[i] / total) * 100.0f;

    tft.setTextColor(cores[i], TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString(nomes[i], 15, y_desenho - 15);

    tft.fillRect(TEXTO_BARRA, y_desenho + ALTURA_BARRA + 2, 60, 10, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(percentual, 1) + "%", TEXTO_BARRA, y_desenho + ALTURA_BARRA + 2);

    int nova_largura = (int)((percentual / 100) * LARGURA_MAX_BARRA);
    if (nova_largura > LARGURA_MAX_BARRA) nova_largura = LARGURA_MAX_BARRA;
    if (nova_largura < 0) nova_largura = 0;

    if (nova_largura > largurasAntigas[i]) {
      tft.fillRect(TEXTO_BARRA + largurasAntigas[i], y_desenho, nova_largura - largurasAntigas[i], ALTURA_BARRA, cores[i]);
    } else if (nova_largura < largurasAntigas[i]) {
      tft.fillRect(TEXTO_BARRA + nova_largura, y_desenho, largurasAntigas[i] - nova_largura, ALTURA_BARRA, TFT_BLACK);
    }
    largurasAntigas[i] = nova_largura;
  }
}

// Grafico de linha do sinal
void desenharGraficoSinal(const float* segmento, int nAmostras) {
  tft.fillRect(SINAL_X0, SINAL_Y0, SINAL_X1 - SINAL_X0, SINAL_Y1 - SINAL_Y0, TFT_BLACK);

  int largura = SINAL_X1 - SINAL_X0;
  int altura  = SINAL_Y1 - SINAL_Y0;
  int meio_y  = SINAL_Y0 + altura / 2;

  float maxAbs = 0.0001f;
  for (int i = 0; i < nAmostras; i++) {
    float a = fabsf(segmento[i]);
    if (a > maxAbs) maxAbs = a;
  }
  float escala = (altura / 2.0f - 5) / maxAbs;

  int passo = nAmostras / largura;
  if (passo < 1) passo = 1;

  int xAnterior = SINAL_X0;
  int yAnterior = meio_y - (int)(segmento[0] * escala);

  for (int px = 1; px < largura; px++) {
    int idx = px * passo;
    if (idx >= nAmostras) break;
    int xAtual = SINAL_X0 + px;
    int yAtual = meio_y - (int)(segmento[idx] * escala);
    tft.drawLine(xAnterior, yAnterior, xAtual, yAtual, TFT_CYAN);
    xAnterior = xAtual;
    yAnterior = yAtual;
  }
}

/*--------------------------------------- SETUP ----------------------------------------------*/
void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.drawRect(10, 10, 300, 220, TFT_WHITE);
  tft.drawLine(DIVISOR_X, 10, DIVISOR_X, 230, TFT_WHITE);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Potencia relativa", 15, 15);
  tft.drawString("Sinal EEG", SINAL_X0, 15);

  filtroMediaMovelNovo(Sinalrecebido, SinalrecebidoMediaMovel, WINDOW_SIZE_MOBILE, 1000); 
  //Serial.println("Setup concluido");
}

/*--------------------------------------- LOOP ----------------------------------------------*/

void loop() {
  
  for (int i = 0; i < SAMPLES; i++) {
    segmentoAtual[i] = Sinalrecebido[startIndex + i];
    //segmentoAtual[i] = SinalrecebidoMediaMovel[startIndex + i];
  }

  float pot_delta = potenciaDaBanda(segmentoAtual, SAMPLES, SAMPLING_FREQ, deltaBandMin, deltaBandMax);
  float pot_tetha = potenciaDaBanda(segmentoAtual, SAMPLES, SAMPLING_FREQ, tethaBandMin, tethaBandMax);
  float pot_alfa  = potenciaDaBanda(segmentoAtual, SAMPLES, SAMPLING_FREQ, alfaBandMin, alfaBandMax);
  float pot_beta  = potenciaDaBanda(segmentoAtual, SAMPLES, SAMPLING_FREQ, betaBandMin, betaBandMax);
  float pot_gama  = potenciaDaBanda(segmentoAtual, SAMPLES, SAMPLING_FREQ, gamaBandMin, gamaBandMax);

  Serial.printf("Delta: %.4f | Theta: %.4f | Alpha: %.4f | Beta: %.4f | Gamma: %.4f\n",
                pot_delta, pot_tetha, pot_alfa, pot_beta, pot_gama);

  desenharGraficoBarrasEEG(pot_delta, pot_tetha, pot_alfa, pot_beta, pot_gama);
  desenharGraficoSinal(segmentoAtual, SAMPLES);

  startIndex += STEP;
  
  //if (startIndex + SAMPLES >= N_MAX_PONTOS) startIndex = 0; //Reinicia
  if (startIndex + SAMPLES >= N_MAX_PONTOS) while(1); //Trava as barras na última medida

  delay(200);
}
