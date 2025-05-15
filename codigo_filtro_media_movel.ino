#include <Arduino.h>
#include "signal.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define N 1000  // Número de amostras
#define WINDOW_SIZE_MOBILE 5  // Janela do filtro média móvel
#define WINDOW_SIZE_NULL 3  // Janela do filtro fase nula
#define WINDOW_SIZE_BANDWIDTH 6  // Janela do passa banda

//Configuração Display

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define OLED_RESET -1

Adafruit_SH1106G tela(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float x[N];  // Sinal de entrada (ruído)
//Filtro Média Móvel:
float filteredSignal[N]; // Sinal filtrado (média móvel)
float nullSignal[N]; // Sinal de fase nula
//Filtro Passa-Banda:
float bandwidthFilterSignal[N]; // Sinal passado a banda
//Filtro Notch
float notchFilterSignal[N]; // Sinal notch

void setup() {
    Serial.begin(115200);

    //Display
    Wire.begin(21, 22); // PRECISA especificar os pinos SDA, SCL
    tela.begin(SCREEN_ADDRESS, true);
    tela.clearDisplay();

    tela.setTextSize(2);
    tela.setTextColor(SH110X_WHITE);
    tela.setCursor(0, 0);
    tela.print("Iniciando ...");
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
        //Serial.println(Sinalrecebido[i]);  // Sinal filtrado
        delay(200);  // Pequeno delay para o plotter processar os dados
    }
}

void loop() {
    //código roda apenas uma vez no setup
}
