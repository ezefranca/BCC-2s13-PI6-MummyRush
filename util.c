#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "listas.h"

int tam_total = 128;

void erro(char *mensagem) {
  printf("%s \n", mensagem);

  exit(EXIT_FAILURE);
}

void serializaJogador(int idAcao, Jogador *jogador, char *buffer)
{
	snprintf(buffer, tam_total, "%d%d%d%d|%f|%f|", idAcao,
		jogador->idJogador,
		jogador->idSocket,
		jogador->angulo,
		jogador->pos_x,	
		jogador->pos_y);
	printf("Jogador %d serializado: %s \n", jogador->idJogador, buffer);
}

void desserializaJogador(char *buffer, Jogador *jogador)
{
	char pos_x[15], pos_y[15];
	int indexSeparador;
	int i = 0;
	
	//Encontra a primeira ocorrencia de '|'
	for(indexSeparador = 0; buffer[indexSeparador] != '|'; indexSeparador++);
	//printf("Testando o '|': %c ", buffer[indexSeparador] );
	//Copia o valor de X para o array temporario
	do
	{
		indexSeparador++;
		pos_x[i] = buffer[indexSeparador];
		i++;
	}
	while(buffer[indexSeparador] != '|');
	pos_x[i] = '\0';
	//printf("Testando o 'x': %s ", pos_x );
	//Copia o valor de Y para o array temporario
	i = 0;
	do
	{
		indexSeparador++;
		pos_y[i] = buffer[indexSeparador];
		i++;
	}
	while(buffer[indexSeparador] != '|');
	pos_y[i] = '\0';
	//printf("Testando o 'y': %s ", pos_y );
	//Seta os valores na struct
	jogador->idJogador = buffer[1] - '0';
	jogador->idSocket = buffer[2] - '0';	
	jogador->angulo = buffer[3] - '0';
	jogador->pos_x = atof(pos_x);
	jogador->pos_y = atof(pos_y);


	printf("Jogador desserializado: %d%d%d|%f|%f| \n",
		jogador->idJogador,
		jogador->idSocket,
		jogador->angulo,
		jogador->pos_x,	
		jogador->pos_y);
}
