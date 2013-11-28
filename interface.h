#ifndef INTERFACE_
#define INTERFACE_

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "listas.h"

#define BOUNCER_SIZE 16

#define SCREEN_W 1024
#define SCREEN_H 768

#define FPS 10.0

enum teclas_movimento {
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

enum teclas_menu {
   UP, DOWN, ENTER
};

bool pos[4];
bool mir[4];
bool doexit;
bool refresh;
char* server;

//allegro stuff
ALLEGRO_DISPLAY *display;
ALLEGRO_EVENT_QUEUE *event_queue;
ALLEGRO_TIMER *timer;
ALLEGRO_FONT *title;
ALLEGRO_FONT *font;

//personagem
ALLEGRO_BITMAP *bouncer[2];
Jogador *jogador;
int velocidade;

//pra onde o personagem esta atirando
ALLEGRO_BITMAP *mira;
float mira_x;
float mira_y;

//background
ALLEGRO_BITMAP *bg;

void tecla_up(ALLEGRO_EVENT ev);
void tecla_down(ALLEGRO_EVENT ev);
bool move_personagem(bool *refresh);
void move_mira(bool *refresh);
void finaliza_jogo();
int inicializa();
void splash();
void exibeTelaEspera();
void loop_menu();
void sair();

#endif
