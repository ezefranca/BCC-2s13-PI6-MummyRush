#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/select.h>
#include <netdb.h>
#include <pthread.h>
#include <math.h>

#include "util.h"
#include "listas.h"
#include "interface.h"
#include "colisao.h"

#define TRUE   1
#define FALSE  0
#define MAX_CLIENTS  4
#define BOUNCER_SIZE 32
#define TAM_TIRO 3

Jogador* raizJogadores;
Tiro *raizTiros;

float angulos[8] = { 0.0, 0.785, 1.570, 2.356, 3.141, 3.927, 4.712, 5.498 };
float vTiro = 10.0;
pthread_mutex_t lock;
struct sockaddr_in address;
int addrlen;
//set of socket descriptors
fd_set readfds;

typedef struct str_thdata
{
    int sock;
    int doExit;
} thdata;

int incluiJogador(int idSocket)
{
    float X = 800.0 + (idSocket*10.0);
    float Y = 600.0 + (idSocket*10.0);

    int totalJogadores = inclui_jogador(raizJogadores, 0, idSocket, ANG_180, X, Y); 
    printf("Jogador %d com o socket %d na [ %f, %f ] \n", totalJogadores, idSocket, X, Y);
    if(totalJogadores < MAX_CLIENTS) 
	return 1;
    else
	return 0;
}

void iniciaJogo()
{
	char bf_inclui_jogador[TAM_MSG];
        char bf_seta_jogador[TAM_MSG];
        char bf_inicia_jogo[TAM_MSG];
	Jogador *aux = raizJogadores->prox;
	Jogador *aux2 = (Jogador*)malloc(sizeof(Jogador));
	while(aux != NULL)
	{
		aux2 = raizJogadores->prox;
		while(aux2 != NULL)
		{
			printf("Serializando o jogador %d \n",  aux2->idJogador);
			serializaJogador(IncluiJogador, aux2, bf_inclui_jogador);
			send(aux->idSocket, bf_inclui_jogador, TAM_MSG, 0 );
			aux2 = aux2->prox;
		}
		printf("Setando o jogador %d \n",  aux->idJogador);
		serializaJogador(SetaJogador, aux, bf_seta_jogador);
		send(aux->idSocket, bf_seta_jogador, TAM_MSG, 0 );
			
		sleep(1);
		aux = aux->prox;
	}
	free(aux2);
	printf("Pronto para iniciar! \n");
	snprintf(bf_inicia_jogo, TAM_MSG, "%d%d", AcaoJogo, IniciaJogo);
	aux = raizJogadores->prox;
	while(aux != NULL)
	{
		send(aux->idSocket, bf_inicia_jogo , TAM_MSG, 0 );
		aux = aux->prox;
	}
}

void moveJogador(char *buffer)
{
    Jogador *tmpJogador = (Jogador*)malloc(sizeof(Jogador));
    desserializaJogador(buffer,tmpJogador);
    Jogador *aux = raizJogadores->prox;
    while(aux != NULL)
    {
	if(aux->idJogador == tmpJogador->idJogador)
	{
		aux->pos_x = tmpJogador->pos_x;
		aux->pos_y = tmpJogador->pos_y;
		aux->angulo = tmpJogador->angulo;
	}
	send(aux->idSocket, buffer, TAM_MSG, 0 );
	aux = aux->prox;
    }
    free(tmpJogador);
}

void incluiTiro(char *buffer)
{	
	Tiro *tiroBuffer = (Tiro*)malloc(sizeof(Tiro));
	desserializaTiro(buffer, tiroBuffer);
	char bufferEnvio[TAM_MSG];

	pthread_mutex_lock(&lock);
	printf("Incluindo tiro...\n");
	tiroBuffer->idTiro = inclui_tiro(raizTiros, 0, tiroBuffer->idJogador, tiroBuffer->angulo, tiroBuffer->pos_x, tiroBuffer->pos_y);
	serializaTiro(IncluiTiro, tiroBuffer, &bufferEnvio);
	Jogador *aux = raizJogadores->prox;
	while(aux != NULL)
	{
		send(aux->idSocket, bufferEnvio, TAM_MSG, 0 );
		aux = aux->prox;
	}
	printf("Tiro %d incluido com sucesso!\n", tiroBuffer->idTiro);
	pthread_mutex_unlock(&lock);

	free(tiroBuffer);
}

void* move_tiros(void *arg)
{
	thdata *data = (thdata*)arg;
	char buffer[TAM_MSG];	
	float dX, dY;
	Tiro *tiroAnterior;
	Tiro *tiroAtual;

	//gera matriz de colisao para mapa
	gera_matriz();

	while(!data->doExit)
	{
		tiroAnterior = raizTiros;
		do
		{
		    tiroAtual = tiroAnterior->prox;
		    if(tiroAtual == NULL)
		    {
			sleep(1);
			break;
		    }
		    if(tiroAtual->ativo == 1)
		    {
			dX = vTiro * cos(angulos[tiroAtual->angulo]);
			dY = vTiro * sin(angulos[tiroAtual->angulo]);
			printf("Tiro %d: dX = %f, dY = %f, angulo = %d \n", tiroAtual->idTiro, dX, dY, tiroAtual->angulo);
			pthread_mutex_lock(&lock);
			tiroAtual->pos_x += dX;
			tiroAtual->pos_y += dY;
			pthread_mutex_unlock(&lock);
			//Verifica colisao com paredes
			if(verifica_colisao(tiroAtual->pos_x, tiroAtual->pos_y, TAM_TIRO, tiroAtual->angulo))
			{
				//Movimenta o tiro
				serializaTiro(MoveTiro, tiroAtual, &buffer);
				printf("movimentado com sucesso!\n");
			}//Verifica colisao com monstros... se sim, verifica jogo ganho
			else
			{
				printf(" testando a remocao... !\n");
				//Remove o tiro	
				//serializaTiro(RemoveTiro, tiroAtual, &buffer);
				pthread_mutex_lock(&lock);

				tiroAtual->ativo = 0;
				snprintf(buffer, TAM_MSG, "%d%d%d", AcaoTiro, RemoveTiro, tiroAtual->idTiro);

				//tiroAnterior->prox = tiroAtual->prox;
				//free(tiroAtual);
				//tiroAtual = tiroAnterior->prox;
				pthread_mutex_unlock(&lock);
				printf("removido com sucesso!\n");
			}
			Jogador *aux = raizJogadores->prox;
			while(aux != NULL)
			{
				send(aux->idSocket, buffer, TAM_MSG, 0 );
				aux = aux->prox;
			}
		    }
		    tiroAnterior = tiroAtual;
		}while(tiroAtual != NULL);
	}
	
	pthread_exit(0);
}

void* trata_eventos(void *arg)
{
	thdata *data = (thdata*)arg;
	char buffer[TAM_MSG];
	int activity, valread, idTipoMensagem, idTipoAcao = 0;

	while(!data->doExit)
	{
		//Check if it was for closing , and also read the incoming message
		if ((valread = read( data->sock , buffer, TAM_MSG)) == 0)
		{
		    //Somebody disconnected , get his details and print
		    getpeername(data->sock , (struct sockaddr*)&address , (socklen_t*)&addrlen);
		    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		      
		    //Close the socket and mark as 0 in list for reuse
		    close( data->sock );
		    //client_socket[i] = 0;
		}
		  
		//Echo back the message that came in
		else if (valread > 0)
		{
		    //set the string terminating NULL byte on the end of the data read
		    //buffer[valread] = '\0';
		    printf("Mensagem recebida do cliente: %s \n", buffer);
		    //send(data->sock , buffer , TAM_MSG , 0 );
		    idTipoMensagem = buffer[IdTipoMensagem] - '0';
		    idTipoAcao = buffer[IdTipoAcao] - '0';

		    switch(idTipoMensagem)
		    {
			case AcaoJogador:
			    switch(idTipoAcao)
			    {
				case MoveJogador:
				printf("Movendo o jogador \n");
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
			    }
			break;
		    }
		}
	}

	pthread_exit(0);
}

int main()
{
    raizJogadores = inicializa_Jogadores();
    raizTiros = inicializa_Tiros();
    int opt = TRUE;
    int master_socket , new_socket , max_clients = MAX_CLIENTS , activity, i , valread , sd;
    int max_sd;
    int idTipoMensagem, idTipoAcao;
    pthread_t thread1, thread2, thread3, thread4, thread5;  /* thread variables */
    thdata data1, data2, data3, data4, data5; /* thread variables */
    char buffer[TAM_MSG];
    int sock_novo;
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
  
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    //bind the socket to localhost port
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);
     
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
      
    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    int acceptNewConnections = 1;
    while(acceptNewConnections == 1)
    {

        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
	Jogador *aux = raizJogadores->prox;
	while(aux != NULL)
	{
            //if valid socket descriptor then add to read list
            if(aux->idSocket > 0)
                FD_SET( aux->idSocket , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(aux->idSocket > max_sd)
                max_sd = aux->idSocket;	
	
	    aux = aux->prox;
	}

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

	    //inform user of socket number - used in send and receive commands
	    printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
	    sock_novo = 1;
	    Jogador *aux = raizJogadores->prox;
	    while(aux != NULL)
	    {
		    //if valid socket descriptor then add to read list
		    if(aux->idSocket == new_socket)
		    {
			sock_novo = 0;
			break;	
                    }
		    aux = aux->prox;
	    }
	    if(sock_novo == 1)
            	acceptNewConnections = incluiJogador(new_socket);
		
	}
     }
     //inicializa componentes do allegro
     if(inicializa() == -1)
		return -1;

     printf("Iniciando o jogo... %d \n" , acceptNewConnections);
     iniciaJogo();
	
	//Inicia o mutex
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\n mutex init failed\n");
		return 1;
	}

	Jogador *aux = raizJogadores->prox;

	// initialize data to pass to threads
	data1.sock = aux->idSocket;
	data1.doExit = FALSE;

	aux = aux->prox;

	// initialize data to pass to threads
	data2.sock = aux->idSocket;
	data2.doExit = FALSE;

	aux = aux->prox;

	// initialize data to pass to threads
	data3.sock = aux->idSocket;
	data3.doExit = FALSE;

	aux = aux->prox;

	// initialize data to pass to threads
	data4.sock = aux->idSocket;
	data4.doExit = FALSE;

	aux = aux->prox;

	// initialize data to pass to threads
	data5.sock = 0;
	data5.doExit = FALSE;

	// create threads 1,2   
	pthread_create (&thread1, NULL, (void *) &trata_eventos, (void *) &data1);
	pthread_create (&thread2, NULL, (void *) &trata_eventos, (void *) &data2);
	pthread_create (&thread3, NULL, (void *) &trata_eventos, (void *) &data3);
	pthread_create (&thread4, NULL, (void *) &trata_eventos, (void *) &data4);
	pthread_create (&thread5, NULL, (void *) &move_tiros, (void *) &data5);

	// Main block now waits for both threads to terminate, before it exits
	//If main block exits, both threads exit, even if the threads have not
	//finished their work 
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	pthread_join(thread5, NULL);
        
	pthread_mutex_destroy(&lock);
	
      
    return 0;
}
