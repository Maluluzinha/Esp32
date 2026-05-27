
#include <Arduino.h>
#include "signal.h"
#include "statics.h"
#include "filters.h"
//#include <SPI.h>
//#include <Wire.h>
#include <TFT_eSPI.h>
#include <HardwareSerial.h>
#include "arduinoFFT.h"
#include "defines.h"

/*
A FAZER:
TESTAR FILTROS NOVAMENTE NA TFT -> OK
MOVER FILTROS PARA A ABA FILTERS.CPP -> ok eu acho?
TESTAR NOVAMENTE NA TELINHA -> OK
COLOCAR A POTÊNCIA NO GRÁFICO DE BARRAS
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
  //tft.fillRect(11, 31, 298, 198, TFT_BLACK); //Função para redesenhar APENAS a área que o gráfico ta sendo plotado
  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
    //tft.drawString("Dados: " + String(valor), 150, 10, 2);
  //tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.drawString("Dados: " + String(valor), 150, 10, 2);
 
 for (int i = 0; i < totalDados - 1; i++) {
    int x1 = 11 + i;
    int y1 = 180 - meusDados[i] * 1.5;  // Escala fixa
    int x2 = 11 + i + 1;
    int y2 = 180 - meusDados[i + 1] * 1.5;
   
    tft.drawLine(x1, y1, x2, y2, TFT_GREEN);
  }

}
/*--------------------------------------- VARIAVEIS DOS FILTROS----------------------------------------------*/
float x[N];  // Sinal de entrada (ruído)
//Filtro Média Móvel:
float filteredSignal[N]; // Sinal filtrado (média móvel)
float nullSignal[N]; // Sinal de fase nula
//Filtro Passa-Banda:
float bandwidthFilterSignal[N]; // Sinal passado a banda
//Filtro Notch
float notchFilterSignal[N]; // Sinal notch

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

    // Aplicação do filtro de média móvel (fase linear)
    for (int n = WINDOW_SIZE_MOBILE; n < N - WINDOW_SIZE_MOBILE; n++) {
        filteredSignal[n] = filteredSignal[n - 1] + x[n] - x[n - WINDOW_SIZE_MOBILE];
        //desenharGraficoCompleto(filteredSignal, n);
        //filteredTestPlot[n] = filteredSignal[n];
    }
    

    // Aplicação do filtro de fase nula
    for (int n = WINDOW_SIZE_NULL; n < N - WINDOW_SIZE_NULL; n++) {
        nullSignal[n] = nullSignal[n - 1] + x[n + 2] - x[n - WINDOW_SIZE_NULL];
    }

    // Aplicação do filtro passa banda
    for (int n = WINDOW_SIZE_BANDWIDTH; n < N - WINDOW_SIZE_BANDWIDTH; n++) {
        bandwidthFilterSignal[n] = x[n + 6] - x[n - 6] - 2*bandwidthFilterSignal[n - 2] + 2*bandwidthFilterSignal[n - 4] - bandwidthFilterSignal[n - 6];
    }

    //Aplicação Filtro Notch
    float b1 = 0.3;
    for (int n = WINDOW_SIZE_BANDWIDTH; n < N - WINDOW_SIZE_BANDWIDTH; n++) {
        notchFilterSignal[n] = x[n] + 2*x[n - 1] - x[n - 2] - 2*b1*notchFilterSignal[n - 1] - pow(b1,2)*notchFilterSignal[n - 2];
    }

    // Exibir os resultados no Serial Plotter
    for (int i = 0; i < N; i++) {
        Serial.print(x[i]);  // Sinal original
        Serial.print(" ");
        Serial.println(filteredSignal[i]);  // Sinal filtrado
        Serial.print(" ");
        Serial.println(nullSignal[i]);  // Sinal média móvel
        delay(10);  // Pequeno delay para o plotter processar os dados
    }

    desenharGraficoCompleto(filteredSignal, MAX_DADOS);

    /*--------------------------------------- TESTE POT ----------------------------------------------*/

    // float potSinalX = potMedia(x, N);
    // Serial.print("A potencia do sinal NAO filtrado é: ");
    // Serial.println(potSinalX);

    // float potFiltered = potMedia(filteredSignal, N);
    // Serial.print("A potencia do sinal filtrado é: ");
    // Serial.println(potFiltered);

    //adicionarDadoCircular(filteredSignal, novoValor);
    //desenharGraficoCompleto(filteredSignal, novoValor);
    //tft.drawString("Pot filtrado: " + String(potFiltered) , 20, 30, 2); 

}

void loop() {

  // Pega novo dado
  //float novoValor = rand() % 100; //Rand para teste apenas, pode colocar um array no lugar
  //adicionarDadoCircular(meusDadoss, novoValor);
  //desenharGraficoCompleto(filteredTestPlot, MAX_DADOS);
  //float dadoFiltrado[1000];
  //desenharGraficoCompleto(dadoFiltrado, MAX_DADOS);

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
  
  delay(1000);
   
}
