#include "listas.h"

Jogador* inicializa_Jogadores()
{
	Jogador *novo, *aux;
	novo = (Jogador*)malloc(sizeof(Jogador));
	if(novo == NULL) exit(0);
	novo->idJogador = 0;
	novo->prox = NULL;
	aux = novo;
	return (aux);
}

int inclui_jogador(Jogador *raizJogadores, int idJogador, int idSocket, int angulo, float x, float y)
{
	Jogador *jogador = (Jogador*)malloc(sizeof(Jogador));
	jogador->idSocket = idSocket;
	jogador->idJogador = idJogador;
	jogador->pos_x = x;
	jogador->pos_y = y;
	jogador->angulo = angulo;	
	jogador->prox = NULL;
	Jogador *aux;
	for(aux = raizJogadores; aux->prox != NULL; aux=aux->prox);
	aux->prox = jogador;
	return jogador->idJogador;
}

Jogador* seleciona_jogador(Jogador *raizJogadores, int idJogador)
{
	Jogador *aux;
	for(aux = raizJogadores; aux->prox != NULL && aux->idJogador != idJogador; aux=aux->prox);
	return (aux);
}
