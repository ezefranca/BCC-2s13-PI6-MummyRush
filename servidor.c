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
  
#include "util.h"
#include "listas.h"

#define TRUE   1
#define FALSE  0
#define MAX_CLIENTS  2

Jogador* raizJogadores;

int incluiJogador(int idJogador, int idSocket)
{
    float X = 800.0 + (idJogador*20.0);
    float Y = 600.0 + (idJogador*20.0);
    printf("Incluindo o jogador %d com o socket %d na [ %f, %f ] \n",  idJogador, idSocket, X, Y);
    int totalJogadores = inclui_jogador(raizJogadores, idJogador, idSocket, ANG_135, X, Y); 
    if(totalJogadores < MAX_CLIENTS) 
	return 1;
    else
	return 0;
}

void iniciaJogo()
{
	char *bufferServidor;
	Jogador *aux = raizJogadores->prox;
	Jogador *aux2;
	while(aux != NULL)
	{
		aux2 = raizJogadores->prox;
		while(aux2 != NULL)
		{
			printf("Serializando o jogador %d \n",  aux->idJogador);
			bufferServidor = serializaJogador(IncluiJogador, aux2);
			send(aux->idSocket, bufferServidor , TAM_MSG, 0 );
			free(bufferServidor);
			aux2 = aux2->prox;
		}
		printf("Setando o jogador %d \n",  aux->idJogador);
		bufferServidor = serializaJogador(SetaJogador, aux);
		send(aux->idSocket, bufferServidor , TAM_MSG, 0 );
		free(bufferServidor);	
		sleep(1);
		aux = aux->prox;
	}
	printf("Pronto para iniciar! \n");
	
	bufferServidor = (char*)malloc(sizeof(char)*TAM_MSG);
	snprintf(bufferServidor, TAM_MSG, "%d", IniciaJogo);
	aux = raizJogadores->prox;
	while(aux != NULL)
	{
		send(aux->idSocket, bufferServidor , TAM_MSG, 0 );
		aux = aux->prox;
	}
	free(aux);
	free(aux2);
	free(bufferServidor);
}

void moveJogador(char *buffer)
{
    Jogador *tmpJogador = desserializaJogador(buffer);
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

int main()
{
    raizJogadores = inicializa_Jogadores();
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS] , max_clients = MAX_CLIENTS , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;
    int idTipoAcao;
      
    char *buffer = (char*)malloc(sizeof(char)*TAM_MSG);
      
    //set of socket descriptors
    fd_set readfds;
        
    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }
      
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
    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
         
        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];
             
            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);
             
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
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
            if(acceptNewConnections){
		    //inform user of socket number - used in send and receive commands
		    printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		
	    
		    //add new socket to array of sockets
		    for (i = 0; i < max_clients; i++)
		    {
		        //if position is empty
		        if( client_socket[i] == 0 )
		        {
		            client_socket[i] = new_socket;
		            printf("Adding to list of sockets as %d\n" , i);
		            acceptNewConnections = incluiJogador(i+1, new_socket);
		            break;
		        }
		    }
		    if(acceptNewConnections == 0)
			iniciaJogo();
	    }
	    else {
		close(new_socket);
		 printf("Socket %d fechado \n" , new_socket);
	    } 
	}


	for (i = 0; i < max_clients; i++)
	{
	    sd = client_socket[i];
	      
	    if (FD_ISSET( sd , &readfds))
	    {
	        //Check if it was for closing , and also read the incoming message
	        if ((valread = read( sd , buffer, TAM_MSG)) == 0)
	        {
	            //Somebody disconnected , get his details and print
	            getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
	            printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
	              
	            //Close the socket and mark as 0 in list for reuse
	            close( sd );
	            client_socket[i] = 0;
	        }
	          
	        //Echo back the message that came in
	        else
	        {
	            //set the string terminating NULL byte on the end of the data read
	            buffer[valread] = '\0';
	            send(sd , buffer , strlen(buffer) , 0 );
		    idTipoAcao = buffer[IdTipoMensagem] - '0';
		    printf("Recebeu msg do cliente do tipo %d \n", idTipoAcao);
		    switch(idTipoAcao)
		    {
			case MoveJogador:
			printf("Movendo o jogador \n");
			moveJogador(buffer);
			break;
		    }
	        }
	    }
	}

    }
      
    return 0;
} 
