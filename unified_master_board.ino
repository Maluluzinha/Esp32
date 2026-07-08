#include <Arduino.h>
#include "signal.h"
#include "statics.h"
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

NOVO NESTA VERSAO:
- REMOVIDA a FFT (arduinoFFT). Potencia de banda agora e calculada no dominio
  do tempo, filtrando o sinal com filtros IIR (biquad) derivados da equacao
  diferencial de um filtro passa-banda de 2a ordem:
      y''(t) + (w0/Q) y'(t) + w0^2 y(t) = (w0/Q) x'(t)
  discretizada via transformada bilinear. Cada banda usa 2 secoes em cascata
  (filtro de 4a ordem) para uma separacao mais nitida entre bandas vizinhas.
- Potencia de cada banda = media do quadrado (mean square) do sinal filtrado.
- Sinal usado vem de Sinalrecebido (signal.cpp / signal.h), com janela
  deslizante de SAMPLES pontos para simular aquisicao continua.
- filters.h removido (nao esta mais em uso).
*/

/*--------------------------------------- COMUNICACAO / DISPLAY ----------------------------------------------*/
HardwareSerial SerialPort(2); // UART2
TFT_eSPI tft = TFT_eSPI();

/*-------------------------------------- JANELA DESLIZANTE SOBRE O SINAL --------------------------*/
// Varre o buffer Sinalrecebido (signal.cpp, N_MAX_PONTOS pontos) em janelas de
// SAMPLES pontos, avancando STEP pontos por iteracao, simulando aquisicao continua.
// Quando houver ADC ao vivo, troque o preenchimento do segmento por leituras novas.
constexpr int STEP = 16;
int startIndex = 0;

float segmentoAtual[SAMPLES]; // janela atual do sinal bruto (dominio do tempo)

/*--------------------------------------- FILTRO IIR (EQUACAO DIFERENCIAL) --------------------------------------------------*/

struct Biquad {
  float b0, b1, b2, a1, a2; // a0 ja normalizado para 1
};

constexpr int SECOES_POR_BANDA = 2; // cascata de 2 secoes -> filtro de 4a ordem por banda

// Discretiza a EDO do passa-banda de 2a ordem via transformada bilinear
// (ganho de pico 0 dB na frequencia central)
Biquad projetarSecaoPassaBanda(float f0, float Q, float fs) {
  float w0 = 2.0f * PI * f0 / fs;
  float alpha = sinf(w0) / (2.0f * Q);
  float cosw0 = cosf(w0);
  float a0 = 1.0f + alpha;

  Biquad c;
  c.b0 =  alpha        / a0;
  c.b1 =  0.0f;
  c.b2 = -alpha        / a0;
  c.a1 = -2.0f * cosw0 / a0;
  c.a2 = (1.0f - alpha) / a0;
  return c;
}

// Equacao de diferencas: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
void aplicarSecao(const Biquad& c, const float* x, float* y, int n) {
  float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
  for (int i = 0; i < n; i++) {
    float xn = x[i];
    float yn = c.b0 * xn + c.b1 * x1 + c.b2 * x2 - c.a1 * y1 - c.a2 * y2;
    y[i] = yn;
    x2 = x1; x1 = xn;
    y2 = y1; y1 = yn;
  }
}

float mediaQuadratica(const float* v, int n) {
  double s = 0.0; // acumula em double para nao perder precisao em muitas amostras
  for (int i = 0; i < n; i++) s += (double)v[i] * v[i];
  return (float)(s / n);
}

// buffers de trabalho reutilizados para cada banda (ping-pong entre eles na cascata)
float bufFiltroA[SAMPLES];
float bufFiltroB[SAMPLES];

// Filtra o segmento inteiro para a banda [low, high] e retorna a potencia (media quadratica)
float potenciaDaBanda(const float* segmento, int n, float fs, float low, float high) {
  float f0 = sqrtf(low * high);      // frequencia central geometrica
  float Q  = f0 / (high - low);       // fator de qualidade a partir da largura de banda
  Biquad c = projetarSecaoPassaBanda(f0, Q, fs);

  const float* in = segmento;
  float* out = bufFiltroA;
  for (int s = 0; s < SECOES_POR_BANDA; s++) {
    aplicarSecao(c, in, out, n);
    in = out;
    out = (out == bufFiltroA) ? bufFiltroB : bufFiltroA;
  }
  return mediaQuadratica(in, n);
}

/*--------------------------------------- FUNCOES PARA PLOT --------------------------------------------------*/

// Grafico de barras - mostra POTENCIA RELATIVA (% do total das 5 bandas)
void desenharGraficoBarrasEEG(float potDelta, float potTetha, float potAlfa, float potBeta, float potGama) {
  static int largurasAntigas[NUMERO_DE_BARRAS];

  float potencias[NUMERO_DE_BARRAS] = {potDelta, potTetha, potAlfa, potBeta, potGama};
  const char* nomes[NUMERO_DE_BARRAS] = {"DELTA", "THETA", "ALFA", "BETA", "GAMA"};
  uint16_t cores[NUMERO_DE_BARRAS] = {TFT_CYAN, TFT_GREEN, TFT_MAGENTA, TFT_BLUE, TFT_YELLOW};

  float total = 0.0f;
  for (int i = 0; i < NUMERO_DE_BARRAS; i++) total += potencias[i];
  if (total <= 0.0f) total = 1.0f; // evita divisao por zero

  for (int i = 0; i < NUMERO_DE_BARRAS; i++) {
    int y_desenho = 45 + (i * ESPACAMENTO_BARRA);
    float percentual = (potencias[i] / total) * 100.0f;

    tft.setTextColor(cores[i], TFT_BLACK);
    tft.setTextSize(1);
    tft.drawString(nomes[i], 15, y_desenho - 20);

    tft.fillRect(TEXTO_BARRA, y_desenho + ALTURA_BARRA + 2, 60, 10, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(percentual, 1) + "%", TEXTO_BARRA, y_desenho + ALTURA_BARRA + 2);

    int nova_largura = (int)((percentual / 100.0f) * LARGURA_MAX_BARRA);
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

// Grafico de linha do sinal bruto (janela atual), na coluna direita.
// Decima as SAMPLES amostras para caber na largura disponivel da regiao.
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
    tft.drawLine(xAnterior, yAnterior, xAtual, yAtual, TFT_GREEN);
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

  Serial.println("Setup concluido");
}

void loop() {
  // copia a janela atual do buffer de sinal (troque por leitura ADC real quando disponivel)
  for (int i = 0; i < SAMPLES; i++) {
    segmentoAtual[i] = Sinalrecebido[startIndex + i];
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
  if (startIndex + SAMPLES >= N_MAX_PONTOS) startIndex = 0;

  delay(200);
}
