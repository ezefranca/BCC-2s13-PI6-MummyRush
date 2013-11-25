#include "colisao.h"

void calcula_matriz()
{
	ALLEGRO_BITMAP *bgpb = al_load_bitmap("exemplopb.png");
	al_lock_bitmap(bgpb, ALLEGRO_LOCK_READONLY, al_get_bitmap_format(bgpb));


	ALLEGRO_COLOR preto = al_get_pixel(bgpb, 12, 12);//ponto preto
	ALLEGRO_COLOR branco = al_get_pixel(bgpb, 750, 400);
	ALLEGRO_COLOR aux;

	for(int i = 0; i < 1024; i++)
		for(int j = 0; j < 768; j++)
		{
			aux = al_get_pixel(bgpb,i,j);
			if(!memcmp(&aux,&branco,sizeof(ALLEGRO_COLOR)))
				mapa[i][j] = 0;
			else if(!memcmp(&aux,&preto,sizeof(ALLEGRO_COLOR)))
				mapa[i][j] = 1;
		}
	al_unlock_bitmap(bgpb);
	al_destroy_bitmap(bgpb);
}

void escreve_matriz()
{
	FILE *p = fopen("map.txt","w");

	for(int i = 0; i < 1024; i++)
	{
		for(int j = 0; j < 768; j++)
		{
			fprintf(p,"%d",mapa[i][j]);
			fprintf(p, ";");
		}
	fprintf(p,"\n");
	}

	free(p);
}

void gera_matriz()
{
	//inicializa a matriz
	for(int i = 0; i < 1024; i++)
		for(int j = 0; j < 768; j++)
			mapa[i][j] = 0;

	//preenche a matriz
	calcula_matriz();
}

//verifica se ocorre colisao com o mapa
bool verifica_colisao(float px, float py, int size, int dir, int v)
{
	int x = (int)px;
	int y = (int)py;
	switch(dir)
	{
		//para cima
		case 0:
		if(mapa[x][y-v] == 0 && mapa[x+size][y-v] == 0)
			return true;
		break;

		case 1://para baixo
		if(mapa[x][y+size+v] == 0 && mapa[x+size][y+size+v] == 0)
			return true;
		break;

		case 2://para esquerda
		if(mapa[x-v][y] == 0 && mapa[x-v][y+size] == 0)
			return true;
		break;

		case 3:// para direita
		if(mapa[x+size+v][y] == 0 && mapa[x+size+v][y+size] == 0)
			return true;
		break;
	}
	return false;
}
