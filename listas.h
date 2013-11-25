#ifndef LISTAS_
#define LISTAS_

#include <stdlib.h>
#include <stdio.h>

typedef struct jogador{
	int idJogador;
	int idSocket;
	float pos_x;
	float pos_y;
	int angulo;
	struct jogador *prox;
}Jogador;

Jogador* inicializa_Jogadores();

int inclui_jogador(Jogador *raizJogadores, int idJogador, int idSocket, int angulo, float x, float y);
Jogador* seleciona_jogador(Jogador *raizJogadores, int idJogador);
#endif
