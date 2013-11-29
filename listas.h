#ifndef LISTAS_
#define LISTAS_

#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

#define BOUNCER_SIZE 32

typedef struct jogador{
	int idJogador;
	int idSocket;
	float pos_x;
	float pos_y;
	int angulo;
	ALLEGRO_BITMAP *bmp;
	struct jogador *prox;
}Jogador;

typedef struct tiro{
	int idTiro;
	int ativo;
	float pos_x;
	float pos_y;
	int angulo;
	int idJogador;
	ALLEGRO_BITMAP *bmp;
	struct tiro *prox;
}Tiro;

Jogador* inicializa_Jogadores();
Tiro* inicializa_Tiros();

int inclui_jogador(Jogador *raizJogadores, int idJogador, int idSocket, int angulo, float x, float y);
int inclui_tiro(Tiro *raizTiros, int idTiro, int idJogador, int angulo, float x, float y);

#endif
