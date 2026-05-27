#ifndef FILTERS_H
#define FILTERS_H

void filtroMediaMovel (float recebido[], int widowSize, int Npontos);
void filtroMediaMovelFaseNula (float recebido[], int widowSize, int Npontos);
void filtroPassaBanda (float recebido[], int widowSize, int Npontos);
void filtroNotch (float recebido[], int widowSize, int Npontos);

#endif