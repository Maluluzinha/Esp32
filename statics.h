#ifndef STATICS_H
#define STATICS_H

float Mediasinal (float recebido[], int nPontos);
float varianciaSinal (float recebido[], int nPontos, float media);
float assimetriaSinal (float recebido[], int nPontos);
float curtoseSinal (float recebido[], int nPontos);
float potInstantanea(float recebido[], int nPontos);
float potMedia(float recebido[], int nPontos);

#endif