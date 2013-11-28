#ifndef COLISAO_
#define COLISAO_

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>

int mapa[1024][768];

void gera_matriz();
bool verifica_colisao(float x, float y, int size, int dir, int v);

#endif
