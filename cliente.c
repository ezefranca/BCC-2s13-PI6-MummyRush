#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <sys/socket.h>

#include <netdb.h>
#include "listas.h"
#include "util.h"
#include "colisao.h"
#include "interface.h"

#define BOUNCER_SIZE 32
#define TAM_TIRO 3

Jogador *raizJogadores;
Tiro *raizTiros;
pthread_mutex_t lock;

typedef struct str_thdata
{
    int sock;
    bool doExit;
} thdata;

int conecta(char* ipServidor)
{
	int sock;

	struct sockaddr_in endereco;
	struct hostent *host;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if(sock == -1)
	erro("Erro ao tentar abrir socket!\n");

	printf("Socket aberto!\n");

	host = gethostbyname(ipServidor);

	if(host == NULL)
	erro("Erro ao tentar obter host!\n");

	printf("Host obtido!\n");

	endereco.sin_family = AF_INET;
	endereco.sin_addr = *((struct in_addr *) host->h_addr_list[0]);
	endereco.sin_port = htons(PORT);

	if(connect(sock, (struct sockaddr *) &endereco, sizeof(struct sockaddr_in)) == -1)
	erro("Erro ao tentar conectar!\n");

	printf("Conectado com o socket %d \n", sock);

	return sock;
}

void desconecta(int sock)
{
	if(close(sock) == -1)
	erro("Erro ao tentar fechar socket!\n");

	printf("Socket fechado!\n");
}

void exibeJogadores()
{
	Jogador *aux = raizJogadores->prox;
	while(aux != NULL){
		printf("Jogador %d : [ %f , %f, %d ] \n", 
		aux->idJogador, 
		aux->pos_x,
		aux->pos_y,
		aux->angulo
		);
		aux = aux->prox;
	}
	printf("Jogo iniciado! \n");
}

void incluiJogador(char *buffer)
{	
	Jogador *aux;
	Jogador *tmpJogador = (Jogador*)malloc(sizeof(Jogador));
        desserializaJogador(buffer, tmpJogador);
	printf("Jogador %d ", tmpJogador->idJogador);
	inclui_jogador(raizJogadores, tmpJogador->idJogador, tmpJogador->idSocket, tmpJogador->angulo, tmpJogador->pos_x, tmpJogador->pos_y);

	for(aux = raizJogadores; aux->idJogador != tmpJogador->idJogador; aux=aux->prox);
	switch(tmpJogador->idJogador)
	{
		case 1:
		aux->bmp = (ALLEGRO_BITMAP*)al_load_bitmap("player1.jpg");
		break;
		case 2:
		aux->bmp = (ALLEGRO_BITMAP*)al_load_bitmap("player2.jpg");
		break;
		case 3:
		aux->bmp = (ALLEGRO_BITMAP*)al_load_bitmap("player3.jpg");
		break;
		case 4:
		aux->bmp = (ALLEGRO_BITMAP*)al_load_bitmap("player4.jpg");
		break;
	}
	printf(" incluido com sucesso!\n");
	free(tmpJogador);
	
}

void setaJogador(char *buffer)
{
	printf("Jogador ");
	jogador = (Jogador*)malloc(sizeof(Jogador));
	novo_tiro = (Tiro*)malloc(sizeof(Tiro));
	desserializaJogador(buffer,jogador);
	novo_tiro->idJogador = jogador->idJogador;
	novo_tiro->idTiro = 0;
	printf(" %d setado com sucesso! \n", jogador->idJogador);
}

void moveJogador(char *buffer)
{
	Jogador *jogadorBuffer = (Jogador*)malloc(sizeof(Jogador));
	desserializaJogador(buffer,jogadorBuffer);

	Jogador *aux = raizJogadores->prox;
	while(aux->idJogador != jogadorBuffer->idJogador)
		aux = aux->prox;
	printf("Atualizando o jogador %d \n", aux->idJogador);
	pthread_mutex_lock(&lock);
	aux->pos_x = jogadorBuffer->pos_x;
	aux->pos_y = jogadorBuffer->pos_y;
	aux->angulo = jogadorBuffer->angulo;
	pthread_mutex_unlock(&lock);

	free(jogadorBuffer);
}

void incluiTiro(char *buffer)
{	
	Tiro *tiroBuffer = (Tiro*)malloc(sizeof(Tiro));
	desserializaTiro(buffer,tiroBuffer);
	Tiro *aux;
	printf("Tiro %d ", tiroBuffer->idTiro);
	pthread_mutex_lock(&lock);
	inclui_tiro(raizTiros, tiroBuffer->idTiro, tiroBuffer->idJogador, tiroBuffer->angulo, tiroBuffer->pos_x, tiroBuffer->pos_y);
	//seleciona o tiro incluido na lista ligada e inicializa o bitmap
	for(aux = raizTiros; aux->idTiro != tiroBuffer->idTiro; aux=aux->prox);
	aux->bmp = (ALLEGRO_BITMAP*)al_create_bitmap(TAM_TIRO, TAM_TIRO);
	al_set_target_bitmap(aux->bmp);
	al_clear_to_color(al_map_rgb(0, 0, 0));

	printf(" incluido com sucesso!\n");
	pthread_mutex_unlock(&lock);

	free(tiroBuffer);
}

void moveTiro(char *buffer)
{
	Tiro *tiroBuffer = (Tiro*)malloc(sizeof(Tiro));
	desserializaTiro(buffer,tiroBuffer);

	Tiro *aux;
	for(aux = raizTiros; aux->idTiro != tiroBuffer->idTiro; aux=aux->prox);

	printf("Atualizando o tiro %d \n", aux->idTiro);
	pthread_mutex_lock(&lock);
	aux->pos_x = tiroBuffer->pos_x;
	aux->pos_y = tiroBuffer->pos_y;
	pthread_mutex_unlock(&lock);

	free(tiroBuffer);
}

void removeTiro(char *buffer)
{
	printf("Tentando remover o tiro... \n");
	int idTiro = buffer[2] - '0';

	pthread_mutex_lock(&lock);
	Tiro *aux = raizTiros->prox;
	while(aux != NULL)
	{
		if(aux->idTiro == idTiro)
		{
			aux->ativo = 0;
			break;
		}
		aux = aux->prox;
	}
	
	pthread_mutex_unlock(&lock);
	printf("Tiro %d removido com sucesso!\n", idTiro);
}

void* le_servidor(void *arg)
{
    thdata *data = (thdata*)arg;
    char buffer[TAM_MSG];
    int idTipoMensagem, idTipoAcao;

    while(!data->doExit)
    {
	if(read(data->sock, buffer, TAM_MSG) >= 0)
	{
		printf("Buffer recebido: %s \n", buffer);
	        idTipoMensagem = buffer[IdTipoMensagem] - '0';
	        idTipoAcao = buffer[IdTipoAcao] - '0';
		switch(idTipoMensagem)
		{
			case AcaoJogador:
			    switch(idTipoAcao)
			    {
				case MoveJogador:
				moveJogador(buffer);
				break;
			    }
			break;
			case AcaoTiro:
			    switch(idTipoAcao)
			    {
				case IncluiTiro:		                
				incluiTiro(buffer);
				break;
				case MoveTiro:
				moveTiro(buffer);
				break;
				case RemoveTiro:
				removeTiro(buffer);
				break;
			    }
			break;
		}
	}
    }
    pthread_exit(0);
}

void* trata_eventos(void *arg)
{
	thdata *data = (thdata*)arg;
	Jogador *auxJogador;
	Tiro *auxTiro;
	char buffer[TAM_MSG];    
	bool moveu, disparou;
	ALLEGRO_EVENT ev;
	
	////////////////////////////////////////////////
	//Configuracoes iniciais
	int i = 0;
	for(i = 0; i < 4; i++)
	{
		pos[i] = false;
		mir[i] = false;
	}

	bg = NULL;
	velocidade = 4;	

	//gera matriz de colisao para mapa
	gera_matriz();
	//load de mapa
	bg = al_load_bitmap("exemplo.jpg");
	if(!bg)
	{
		al_destroy_bitmap(bg);
		fprintf(stderr, "Failed to load background!\n");
			return -1;
	}
	//load de personagens
	auxJogador = raizJogadores->prox;
	while(auxJogador != NULL)
	{
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_bitmap(auxJogador->bmp, auxJogador->pos_x, auxJogador->pos_y, 0);
		auxJogador = auxJogador->prox;
	}

	doexit = false;
	refresh = true;
	al_flip_display();
	
	////////////////////////////////////////////////////
	//inicia o jogo:

	while(!data->doExit)
	{
		moveu = false;
		disparou = false;
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_TIMER) 
		{
			moveu = move_personagem(&refresh);
			if(moveu)
			{
				printf("Moveu para a coordenada [ %f, %f ] \n", jogador->pos_x, jogador->pos_y);
				pthread_mutex_lock(&lock);
				serializaJogador(MoveJogador, jogador, &buffer);
				printf("Enviando movimento para o servidor: %s \n", buffer);
				write(data->sock, &buffer, TAM_MSG);
				pthread_mutex_unlock(&lock);
			}
			disparou = move_mira(&refresh);
			if(disparou)
			{
				printf("Disparou no angulo %d \n", novo_tiro->angulo);
				pthread_mutex_lock(&lock);
				serializaTiro(IncluiTiro, novo_tiro, &buffer);
				printf("Enviando tiro para o servidor: %s \n", buffer);
				write(data->sock, &buffer, TAM_MSG);
				pthread_mutex_unlock(&lock);
			}
		}

		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			doexit = true;
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			tecla_down(ev);
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
			if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				doexit = true;
			tecla_up(ev);
		}

		if(refresh && al_is_event_queue_empty(event_queue))
		{	
			pthread_mutex_lock(&lock);
			
			al_draw_bitmap(bg,0,0,0);

			auxJogador = raizJogadores->prox;
			while(auxJogador != NULL)
			{
				al_set_target_bitmap(al_get_backbuffer(display));
				al_draw_bitmap(auxJogador->bmp, auxJogador->pos_x, auxJogador->pos_y, 0);
				auxJogador = auxJogador->prox;
			}

			auxTiro = raizTiros->prox;
			while(auxTiro != NULL)
			{
				if(auxTiro->ativo == 1)
				{
					al_set_target_bitmap(al_get_backbuffer(display));
					al_draw_bitmap(auxTiro->bmp, auxTiro->pos_x, auxTiro->pos_y, 0);
				}
				auxTiro = auxTiro->prox;
			}
			al_flip_display();
			refresh = false;
			printf("Atualizou a tela \n");
			pthread_mutex_unlock(&lock);
		} 
	}
	pthread_exit(0);
}

int main() {

	pthread_t thread1, thread2;  /* thread variables */
	thdata data;         /* struct to be passed to threads */

	char buffer[TAM_MSG];
	int inicioJogo, idTipoMensagem, idTipoAcao;
	raizJogadores = inicializa_Jogadores();
	raizTiros = inicializa_Tiros();	

	//inicializa componentes do allegro
	if(inicializa() == -1)
		return -1;

	//splashscreen com nome do jogo
	splash();

	//executa menu inicial
	loop_menu();
        
	//conecta ao servidor
	int sock = conecta(server);
	
	//exibe a tela de espera
	exibeTelaEspera();
	inicioJogo = 0;
	//aguarda todos os jogadores se conectarem
	while(inicioJogo == 0)
	{
		if(read(sock, buffer, TAM_MSG) > 0)
		{
			printf("Mensagem recebida do servidor: %s \n", buffer);
			idTipoMensagem = buffer[IdTipoMensagem] - '0';
			idTipoAcao = buffer[IdTipoAcao] - '0';
			switch(idTipoMensagem)
			{
				case AcaoJogador:
				    switch(idTipoAcao)
				    {
					case IncluiJogador:
				        incluiJogador(buffer);
					break;
					case SetaJogador:
					setaJogador(buffer);
					break;
				    }
				break;
				case AcaoJogo:
				    switch(idTipoAcao)
				    {
					case IniciaJogo:
					inicioJogo = 1;
					break;
				    }
				break;
			}
		}
	}
	exibeJogadores();

	//Inicia o mutex
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

	// initialize data to pass to threads
	data.sock = sock;
	data.doExit = false;

	// create threads 1,2   
	pthread_create (&thread1, NULL, (void *) &le_servidor, (void *) &data);
	pthread_create (&thread2, NULL, (void *) &trata_eventos, (void *) &data);

	// Main block now waits for both threads to terminate, before it exits
	//If main block exits, both threads exit, even if the threads have not
	//finished their work 
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
        
	pthread_mutex_destroy(&lock);
	finaliza_jogo();
	free(server);
	free(jogador);
	free(raizJogadores);
	free(raizTiros);
	free(novo_tiro);
	desconecta(sock);
	sair();
	
	return EXIT_SUCCESS;
}
