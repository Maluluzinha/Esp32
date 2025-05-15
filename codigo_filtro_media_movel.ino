#include <arduinoFFT.h>
#include <Arduino.h>
#include "signal.h"
#include "statics.h"
#include "filters.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>

/*--------------------------------------- DEFINES ----------------------------------------------*/
#define N 1000  // Número de amostras
#define WINDOW_SIZE_MOBILE 5  // Janela do filtro média móvel
#define WINDOW_SIZE_NULL 3  // Janela do filtro fase nula
#define WINDOW_SIZE_BANDWIDTH 6  // Janela do passa banda

/*-------------------------------------- CONFIG DISPLAY -----------------------------------------------*/

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C  // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SDA_PIN 21
#define SCL_PIN 22
#define OLED_RESET -1

//Definição do objeto display, saber a versão dele!
Adafruit_SSD1306 tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*-------------------------------------- CONFIG PRA FFT -----------------------------------------------*/
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const float signalFrequency = 1000;
const float samplingFrequency = 5000;
const uint8_t amplitude = 100;

float vReal[samples];
float vImag[samples];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency); /* Create FFT object */

/*--------------------------------------- VAR ----------------------------------------------*/
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

    //Iniciar Tela:
    Wire.begin(SDA_PIN, SCL_PIN); // Pinos SDA, SCL
    if (!tela.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Endereço I2C padrão
    Serial.println(F("OLED não encontrado!"));
    while (true);
    }

    tela.clearDisplay();
    tela.setTextSize(1);
    tela.setTextColor(SSD1306_WHITE);
    tela.setCursor(0, 0);
    tela.print("Display iniciado!");
    tela.display();

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

    //Teste com arrays longos
    // int numeroPontos = numeroPontosRecebidos(Sinalrecebido, N_MAX_PONTOS);
    // Serial.print("Pontos recebidos: ");
    // Serial.println(numeroPontos);

    /*--------------------------------------- TESTE POT ----------------------------------------------*/

    float potSinalX = potMedia(x, N);
    Serial.print("A potencia do sinal NAO filtrado é: ");
    Serial.println(potSinalX);

    float potFiltered = potMedia(filteredSignal, N);
    Serial.print("A potencia do sinal filtrado é: ");
    Serial.println(potFiltered);

    tela.setTextSize(1);
    tela.setTextColor(SSD1306_WHITE);
    tela.setCursor(0, 10);
    tela.print("Potencia Media: ");
    tela.display();

    tela.setCursor(90, 10);
    tela.print(potSinalX);
    tela.display();

    tela.setCursor(0, 20);
    tela.print("Potencia Media: ");
    tela.display();

    tela.setCursor(90, 20);
    tela.print(potFiltered);
    tela.display();
}

void loop() {
    //código roda apenas uma vez no setup
    // Get samples
    FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward);	/* Weigh data */
    FFT.compute(FFTDirection::Forward); /* Compute FFT */
    FFT.complexToMagnitude(); /* Compute magnitudes */
    float x = FFT.majorPeak();
    // Rest of the code
}
