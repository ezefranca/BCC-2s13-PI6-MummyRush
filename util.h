#ifndef UTIL_
#define UTIL_
#include <string.h>
#include <stdio.h>
#include "listas.h"
#define HOST "localhost"
#define PORT 10000
#define TAM_MSG 128
#define BOUNCER_SIZE 32
#define TAM_TIRO 3

typedef enum {
   ANG_0, ANG_45, ANG_90, ANG_135, ANG_180, ANG_225, ANG_270, ANG_315
} ANG;

typedef enum { 
IdTipoMensagem,
IdTipoAcao,
} TiposMensagem;

typedef enum { 
AcaoJogo,
AcaoJogador,
AcaoMonstro,
AcaoTiro
} Acoes;

typedef enum { 
IncluiJogador,
MoveJogador,
RemoveJogador,
SetaJogador
} AcoesJogador;

typedef enum { 
IncluiTiro,
MoveTiro,
RemoveTiro
} AcoesTiro;

typedef enum { 
IniciaJogo,
JogoGanho,
JogoPerdido
} AcoesJogo;

void erro(char *mensagem);

void serializaJogador(int idAcao, Jogador *jogador, char *buffer);
void desserializaJogador(char *buffer, Jogador *jogador);

void serializaTiro(int idAcao, Tiro *tiro, char *buffer);
void desserializaTiro(char *buffer, Tiro *tiro);

#endif
