#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include <sys/socket.h>

#include <netdb.h>
#include "listas.h"
#include "util.h"
#include "colisao.h"
#include "interface.h"

#define BOUNCER_SIZE 16

Jogador *raizJogadores;
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
	Jogador *tmpJogador = (Jogador*)malloc(sizeof(Jogador));
        desserializaJogador(buffer, tmpJogador);
	printf("Incluindo jogador %d ", tmpJogador->idJogador);
	inclui_jogador(raizJogadores, tmpJogador->idJogador, tmpJogador->idSocket, tmpJogador->angulo, tmpJogador->pos_x, tmpJogador->pos_y);
	free(tmpJogador);
	printf("... incluido com sucesso!\n");
	
}

void setaJogador(char *buffer)
{
	printf("Setando o jogador... \n");
	jogador = (Jogador*)malloc(sizeof(Jogador));
	desserializaJogador(buffer,jogador);
	printf("Id do jogador: %d \n", jogador->idJogador);
}

void moveJogador(Jogador *jogadorBuffer)
{
	Jogador *aux = raizJogadores->prox;
	while(aux->idJogador != jogadorBuffer->idJogador)
		aux = aux->prox;
	printf("Atualizando o jogador %d \n", aux->idJogador);
	pthread_mutex_lock(&lock);
	aux->pos_x = jogadorBuffer->pos_x;
	aux->pos_y = jogadorBuffer->pos_y;
	aux->angulo = jogadorBuffer->angulo;
	pthread_mutex_unlock(&lock);
}

void* le_servidor(void *arg)
{
    thdata *data = (thdata*)arg;
    char buffer[TAM_MSG];
    int idTipoAcao = 0;
    Jogador *jogadorBuffer = (Jogador*)malloc(sizeof(Jogador));
    while(!data->doExit)
    {
	if(read(data->sock, buffer, TAM_MSG) >= 0)
	{
		printf("Buffer recebido: %s \n", buffer);

		idTipoAcao = buffer[IdTipoMensagem] - '0';
		switch (idTipoAcao)
		{
			case MoveJogador:
			
                        desserializaJogador(&buffer, jogadorBuffer);
			moveJogador(jogadorBuffer);
			break;
		}
	}
	
    }
    free(jogadorBuffer);
    pthread_exit(0);
}

void* trata_eventos(void *arg)
{
	thdata *data = (thdata*)arg;
	Jogador *aux;
	char buffer[TAM_MSG];    
	bool moveu;
	int i, idBouncer;
	ALLEGRO_EVENT ev;

	////////////////////////////////////////////////
	//Configuracoes iniciais
	for(i = 0; i < 4; i++)
	{
		pos[i] = false;
		mir[i] = false;
	}

	bg = NULL;
	bouncer[0] = NULL;
	bouncer[1] = NULL;
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
	aux = raizJogadores->prox;
	//carrega o player 1
	bouncer[0] = al_load_bitmap("player1.jpg");
	if(!bouncer[0])
	{
		al_destroy_bitmap(bouncer[0]);
		fprintf(stderr, "Falha ao carregar o player 1!\n");
			return -1;
	}
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(bouncer[0], aux->pos_x, aux->pos_y, 0);
	aux = aux->prox;
	//carrega o player 2
	bouncer[1] = al_load_bitmap("player2.jpg");
	if(!bouncer[1])
	{
		al_destroy_bitmap(bouncer[1]);
		fprintf(stderr, "Falha ao carregar o player 2!\n");
			return -1;
	}
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(bouncer[1], aux->pos_x, aux->pos_y, 0);
	aux = aux->prox;

	doexit = false;
	refresh = true;
	al_flip_display();

	////////////////////////////////////////////////////
	//inicia o jogo:

	while(!data->doExit)
	{
		moveu = false;
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_TIMER) 
		{
			moveu = move_personagem(&refresh);
			if(moveu)
			{
				printf("Moveu o socket %d \n", data->sock);
				pthread_mutex_lock(&lock);
				serializaJogador(MoveJogador, jogador, &buffer);
				printf("Enviando para o servidor: %s \n", buffer);
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
			aux = raizJogadores->prox;	
			while(aux != NULL)
			{
				idBouncer = aux->idJogador-1;
				al_set_target_bitmap(al_get_backbuffer(display));
				al_draw_bitmap(bouncer[idBouncer], aux->pos_x, aux->pos_y, 0);
				aux = aux->prox;
			}
			al_flip_display();
			refresh = false;

			pthread_mutex_unlock(&lock);
		} 
	}

	pthread_exit(0);
}

int main() {

	pthread_t thread1, thread2;  /* thread variables */
	thdata data;         /* struct to be passed to threads */

	char buffer[TAM_MSG];
	int inicioJogo, idTipoAcao;
	raizJogadores = inicializa_Jogadores();
	
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
			idTipoAcao = buffer[IdTipoMensagem] - '0';
			switch (idTipoAcao)
			{
				case IncluiJogador:
				incluiJogador(buffer);
				break;
				case SetaJogador:
				setaJogador(buffer);
				break;
				case IniciaJogo:
				inicioJogo = 1;
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
	desconecta(sock);
	sair();
	
	return EXIT_SUCCESS;
}