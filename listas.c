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

Tiro* inicializa_Tiros()
{
	Tiro *novo, *aux;
	novo = (Tiro*)malloc(sizeof(Tiro));
	if(novo == NULL) exit(0);
	novo->idTiro = 0;
	novo->prox = NULL;
	aux = novo;
	return (aux);
}

int inclui_jogador(Jogador *raizJogadores, int idJogador, int idSocket, int angulo, float x, float y)
{
	Jogador *jogador = (Jogador*)malloc(sizeof(Jogador));
	jogador->idSocket = idSocket;
	jogador->pos_x = x;
	jogador->pos_y = y;
	jogador->angulo = angulo;	
	jogador->prox = NULL;
	Jogador *aux;
	for(aux = raizJogadores; aux->prox != NULL; aux=aux->prox);
	if(idJogador == 0)
		jogador->idJogador = aux->idJogador+1;
	else
		jogador->idJogador = idJogador;
	aux->prox = jogador;
	return jogador->idJogador;
}

int inclui_tiro(Tiro *raizTiros, int idTiro, int idJogador, int angulo, float x, float y)
{
	Tiro *tiro = (Tiro*)malloc(sizeof(Tiro));
	tiro->pos_x = x;
	tiro->pos_y = y;
	tiro->angulo = angulo;
	tiro->idJogador = idJogador;
	tiro->ativo = 1;	
	tiro->prox = NULL;
	Tiro *aux;
	for(aux = raizTiros; aux->prox != NULL; aux=aux->prox);
	if(idTiro == 0)
		tiro->idTiro = aux->idTiro+1;//usado no servidor
	else
		tiro->idTiro = idTiro;//usado no cliente
	aux->prox = tiro;
	return tiro->idTiro;
}
