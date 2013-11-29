#ifndef UTIL_
#define UTIL_
#include <string.h>
#include <stdio.h>
#include "listas.h"
#define HOST "localhost"
#define PORT 10000
#define TAM_MSG 128

typedef enum {
   ANG_0, ANG_45, ANG_90, ANG_135, ANG_180, ANG_225, ANG_270, ANG_315
} ANG;

float angulos[8] = { 0.0, 0.785, 1.570, 2.356, 3.141, 3.927, 4.712, 5.498 };

typedef enum { 
IdTipoMensagem,
IncluiJogador,
SetaJogador,
MoveJogador,
IniciaJogo,
FinalizaJogo
} TiposMensagem;

void erro(char *mensagem);
void serializaJogador(int idAcao, Jogador *jogador, char *buffer);
void desserializaJogador(char *buffer, Jogador *jogador);
#endif
