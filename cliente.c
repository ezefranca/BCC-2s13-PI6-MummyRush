#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>

#include <netdb.h>
#include "listas.h"
#include "util.h"
#include "colisao.h"
#include "interface.h"

Jogador *raizJogadores;

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
	free(aux);
	printf("Jogo iniciado! \n");
}

void incluiJogador(char *buffer)
{	
	Jogador *tmpJogador = desserializaJogador(buffer);
	printf("Incluindo jogador %d ", tmpJogador->idJogador);
	inclui_jogador(raizJogadores, tmpJogador->idJogador, tmpJogador->idSocket, tmpJogador->angulo, tmpJogador->pos_x, tmpJogador->pos_y);
	free(tmpJogador);
	printf("... incluido com sucesso!\n");
	
}

void setaJogador(char *buffer)
{
	printf("Setando o jogador... \n");
	jogador = desserializaJogador(buffer);
	idJogador = jogador->idJogador;
	angulo = jogador->angulo;
	bouncer_x = jogador->pos_x;
	bouncer_y = jogador->pos_y;
	printf("Id do jogador: %d \n", idJogador);
}

void moveJogador(char *buffer)
{
	printf("Movendo o jogador... \n");
	Jogador *jogadorBuffer = desserializaJogador(buffer);
	Jogador *aux = seleciona_jogador(raizJogadores, jogadorBuffer->idJogador);
	aux->pos_x = jogadorBuffer->pos_x;
	aux->pos_y = jogadorBuffer->pos_y;
	aux->angulo = jogadorBuffer->angulo;
	free(jogadorBuffer);
}

int main() {

	char *buffer = (char*)malloc(sizeof(char)*TAM_MSG);
	int inicioJogo, idTipoAcao, idBouncer = 0;
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

	//load de imagens
	if(prepara_jogo(display, raizJogadores) == -1)
	sair();

	//gera matriz de colisao para mapa
	gera_matriz();

	////////////////////////////////////////////////////
	//inicia o jogo:
	doexit = false;
	refresh = true;
	al_flip_display();

	while(!doexit)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_TIMER) {
		   printf("Evento mover \n");
		   bool moveu = move_personagem(&refresh);
		   if(moveu)
		   {
		   	printf("Moveu \n");
			jogador->pos_x = bouncer_x;
			jogador->pos_y = bouncer_y;
			buffer = serializaJogador(MoveJogador, jogador);
			write(sock, &buffer, TAM_MSG);
		   }
		   move_mira(&refresh);
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
		sleep(1);
		if(read(sock, buffer, TAM_MSG) >= 0)
		{
			idTipoAcao = buffer[IdTipoMensagem] - '0';
			switch (idTipoAcao)
			{
				case MoveJogador:
				printf("Recebeu solicitacao para mover jogador \n");
				moveJogador(buffer);
				refresh = true;
				break;
			}
		}
		if(refresh && al_is_event_queue_empty(event_queue))
		{
			 al_draw_bitmap(bg,0,0,0);
			 Jogador *aux = raizJogadores->prox;

			 while(aux != NULL){
				idBouncer = aux->idJogador-1;
				al_draw_bitmap(bouncer[idBouncer], aux->pos_x, aux->pos_y, 0);
				aux = aux->prox;
			 }
			 free(aux);
			 refresh = false;
			 al_flip_display();
		} 
		sleep(1);
	}
	free(server);
	free(buffer);
	free(jogador);
	free(raizJogadores);
	finaliza_jogo();
	desconecta(sock);
	sair();
	
	return EXIT_SUCCESS;
}
