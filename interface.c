#include "interface.h"
#include "colisao.h"
#include "listas.h"

ALLEGRO_BITMAP *fundo = NULL;
struct Data data;

struct Data
{
	int menuselection;
	int menuitemcount;
};

typedef void (*funcptr)();

void tecla_down(ALLEGRO_EVENT ev)
{
	switch(ev.keyboard.keycode)
	{
		case ALLEGRO_KEY_W:
		pos[KEY_UP] = true;
		break;
 
		case ALLEGRO_KEY_S:
		pos[KEY_DOWN] = true;
		break;
 
		case ALLEGRO_KEY_A: 
		pos[KEY_LEFT] = true;
		break;
 
		case ALLEGRO_KEY_D:
		pos[KEY_RIGHT] = true;
		break;

		case ALLEGRO_KEY_UP:
		mir[KEY_UP] = true;
		break;
 
		case ALLEGRO_KEY_DOWN:
		mir[KEY_DOWN] = true;
		break;

		case ALLEGRO_KEY_LEFT: 
		mir[KEY_LEFT] = true;
		break;
 
		case ALLEGRO_KEY_RIGHT:
		mir[KEY_RIGHT] = true;
		break;	
	}
}

void tecla_up(ALLEGRO_EVENT ev)
{
	switch(ev.keyboard.keycode)
	{
		case ALLEGRO_KEY_W:
		pos[KEY_UP] = false;
		break;
 
		case ALLEGRO_KEY_S:
		pos[KEY_DOWN] = false;
		break;

		case ALLEGRO_KEY_A: 
		pos[KEY_LEFT] = false;
		break;
 
		case ALLEGRO_KEY_D:
		pos[KEY_RIGHT] = false;
		break;
 
		case ALLEGRO_KEY_UP:
		mir[KEY_UP] = false;
		break;
 
		case ALLEGRO_KEY_DOWN:
		mir[KEY_DOWN] = false;
		break;
 
		case ALLEGRO_KEY_LEFT: 
		mir[KEY_LEFT] = false;
		break;
 
		case ALLEGRO_KEY_RIGHT:
		mir[KEY_RIGHT] = false;
		break;
	}
}

int prepara_jogo(ALLEGRO_DISPLAY *display, Jogador *raizJogadores)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		pos[i] = false;
		mir[i] = false;
	}

	bg = NULL;
	velocidade = 4;	

	//load de mapa
	bg = al_load_bitmap("exemplo.jpg");
	if(!bg)
	{
		al_destroy_bitmap(bg);
		fprintf(stderr, "Failed to load background!\n");
			return -1;
	}
	al_draw_bitmap(bg,0,0,0);

	//load de personagens
	Jogador *aux = raizJogadores->prox;
	int idBouncer;
	while(aux != NULL){
		idBouncer = aux->idJogador-1;
		bouncer[idBouncer] = al_create_bitmap(BOUNCER_SIZE, BOUNCER_SIZE);
		if(!bouncer[idBouncer])
		{
			fprintf(stderr, "failed to create bouncer bitmap!\n");
				return -1;
		}
		al_set_target_bitmap(bouncer[idBouncer]);
		al_clear_to_color(al_map_rgb(0, 150, 255));
		al_set_target_bitmap(al_get_backbuffer(display));
		al_draw_bitmap(bouncer[idBouncer], aux->pos_x, aux->pos_y, 0);
		aux = aux->prox;
	}
	free(aux);

	return 0;
}

void finaliza_jogo()
{
	al_destroy_bitmap(bg);
	al_destroy_bitmap(bouncer[0]);
	al_destroy_bitmap(bouncer[1]);
	al_destroy_bitmap(bouncer[2]);
	al_destroy_bitmap(bouncer[3]);
}

bool move_personagem(bool *refresh)
{
	bool moveu = false;
	if(pos[KEY_UP] && verifica_colisao(bouncer_x, bouncer_y, BOUNCER_SIZE, 0, velocidade))
	{
		if(pos[KEY_LEFT] || pos[KEY_RIGHT])
			bouncer_y -= velocidade*0.7;
		else
			bouncer_y -= velocidade;
		*refresh = true;
		moveu = true;
	}
 
	if(pos[KEY_DOWN] && verifica_colisao(bouncer_x, bouncer_y, BOUNCER_SIZE, 1, velocidade))
	{
		if(pos[KEY_LEFT] || pos[KEY_RIGHT])
			bouncer_y += velocidade*0.7;
		else
			bouncer_y += velocidade;
		*refresh = true;
		moveu = true;
	}
 
       	if(pos[KEY_LEFT] && verifica_colisao(bouncer_x, bouncer_y, BOUNCER_SIZE, 2, velocidade))
	{
		if(pos[KEY_UP] || pos[KEY_DOWN])
			bouncer_x -= velocidade*0.7;
		else
			bouncer_x -= velocidade;
		*refresh = true;
		moveu = true;
	}
 
	if(pos[KEY_RIGHT] && verifica_colisao(bouncer_x, bouncer_y, BOUNCER_SIZE, 3, velocidade))
	{
		if(pos[KEY_UP] || pos[KEY_DOWN])
			bouncer_x += velocidade*0.7;
		else
			bouncer_x += velocidade;
		*refresh = true;
		moveu = true;
	}
	jogador->pos_x = bouncer_x;
	jogador->pos_y = bouncer_y;
	return moveu;
}

void move_mira(bool *refresh)
{
	mira_x = jogador->pos_x;
	mira_y = jogador->pos_y;

	if(mir[KEY_UP])
		mira_y -= 20.0;
	if(mir[KEY_DOWN])
		mira_y += 20.0;
	if(mir[KEY_LEFT])
		mira_x -= 20.0;
	if(mir[KEY_RIGHT])
		mira_x += 20.0;

	*refresh = true;
}

int inicializa()
{
	//inicializa componentes do allegro
	if(!al_init())
	{
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	if(!al_init_image_addon())
		fprintf(stderr,"Failed to initialize image addon!\n");

	if(!al_init_primitives_addon())
		fprintf(stderr,"Failed to initialize primitives addon!\n");

	al_init_font_addon();
	al_init_ttf_addon();

	if(!al_install_keyboard())
	{
		fprintf(stderr, "failed to initialize the keyboard!\n");
	return -1;
	}

	//instancia variaveis do allegro 
	timer = al_create_timer(1.0 / FPS);
	if(!timer)
	{
		fprintf(stderr, "failed to create timer!\n");
	return -1;
	}

	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display)
	{
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	font = al_load_ttf_font("pirulen.ttf",36,0 );
   
	event_queue = al_create_event_queue();
	if(!event_queue)
	{
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}
	if (!font)
	{
		fprintf(stderr, "Could not load 'pirulen.ttf'.\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		al_destroy_event_queue(event_queue);
		return -1;
	}

	//registra fonte de eventos
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_start_timer(timer);

	return 0;
}

void splash()
{
//design - fazer splashscreen

	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(al_map_rgb(0, 0, 0));
	ALLEGRO_BITMAP *fundo = al_create_bitmap(800, 480);
	al_set_target_bitmap(fundo);
	al_clear_to_color(al_map_rgb(255, 255, 0));
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,112,144,0);
	al_destroy_bitmap(fundo);
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),160 ,ALLEGRO_ALIGN_CENTRE, "Mummy Rush!");

	al_flip_display();
 
	al_rest(2.0);
}

void exibeTelaEspera()
{
	//design - fazer splashscreen

	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(al_map_rgb(0, 0, 0));
	ALLEGRO_BITMAP *fundo = al_create_bitmap(800, 480);
	al_set_target_bitmap(fundo);
	al_clear_to_color(al_map_rgb(255, 255, 0));
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,112,144,0);
	al_destroy_bitmap(fundo);
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),160 ,ALLEGRO_ALIGN_CENTRE, "Aguardando  jogadores...");

	al_flip_display();
 
	al_rest(2.0);
}

void jogar()
{
	ALLEGRO_USTR *input = al_ustr_new("");

	//fundo = al_create_bitmap(800, 480);
	//al_set_target_bitmap(fundo);
	//al_clear_to_color(al_map_rgb(10, 120, 190));
	al_set_target_bitmap(al_get_backbuffer(display));

	//posicao na string - necessaria para apagar
        int cur_pos = -1;

	doexit = false;
	ALLEGRO_EVENT ev;

        while (!doexit)
        {
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_TIMER)
			refresh = true;
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			sair();

		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ENTER:
				doexit = true;
				break;	

				case ALLEGRO_KEY_ESCAPE:
				doexit = true;
				break;	

				case ALLEGRO_KEY_BACKSPACE:
				if(cur_pos == -1)
					cur_pos = 0;
				// chop the string
				al_ustr_remove_chr(input, cur_pos); 
				cur_pos --;
				break;
			}
		}

		else if(ev.type == ALLEGRO_EVENT_KEY_CHAR)
		{
			int unichar = ev.keyboard.unichar;
			if (unichar >= 32)
			{
				al_ustr_append_chr(input, unichar);
				cur_pos++;
			}
		}
		if(refresh && al_is_event_queue_empty(event_queue))
		{
			al_draw_bitmap(fundo,112,144,0);
			al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),200 ,ALLEGRO_ALIGN_CENTRE, "Endereço do Servidor");

			al_draw_ustr(font, al_map_rgb(0,0,0), (SCREEN_W/2),400 ,ALLEGRO_ALIGN_CENTRE, input);

			al_flip_display();
		}
	}
	if(ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
	{
		server = malloc(cur_pos+2);
		int i;
		for(i = 0; i < cur_pos+1; i++)
			server[i] = al_ustr_get(input,i);
		server[cur_pos+1] = '\0';
		doexit = true;
		al_ustr_free(input);
	}
	else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
	{
		al_ustr_free(input);
		doexit = false;
	}
	else
		sair();
}

void sobre()
{
	//desenha nomes
	al_draw_bitmap(fundo,112,144,0);
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),200 ,ALLEGRO_ALIGN_CENTRE, "Programaçao");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),260 ,ALLEGRO_ALIGN_CENTRE, "Graziella Bonizi");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),300 ,ALLEGRO_ALIGN_CENTRE, "Lucas Chao");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),360 ,ALLEGRO_ALIGN_CENTRE, "Design");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),400 ,ALLEGRO_ALIGN_CENTRE, "Renan Valeiro");

	al_draw_text(font, al_map_rgb(255,255,0), (SCREEN_W/2),500 ,ALLEGRO_ALIGN_CENTRE, "Voltar");
	al_flip_display();

	//aguarda tecla enter
	while(!doexit)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			doexit = true;
		
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_ENTER:
				doexit = true;
				break;

				case ALLEGRO_KEY_BACKSPACE:
				doexit = true;
				break;

				case ALLEGRO_KEY_ESCAPE:
				doexit = true;
				break;
			}
		}
	}
	//volta para menu
	doexit = false;
}

void sair()
{
	//encerra jogo
	al_destroy_font(font);
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	al_unregister_event_source(event_queue, al_get_keyboard_event_source());
	al_unregister_event_source(event_queue, al_get_display_event_source(display));
	al_unregister_event_source(event_queue, al_get_timer_event_source(timer));
//   al_register_event_source(event_queue, al_get_mouse_event_source());
	al_shutdown_font_addon();
	al_shutdown_primitives_addon();
	al_shutdown_image_addon();
	al_uninstall_keyboard();

	exit(0);
}


static const char* menuitems[] =
{
	"Jogar",
	"Sobre",
	"Sair"
};

void menu_item_selected()
{
	if(data.menuselection == 2)
		al_destroy_bitmap(fundo);

	static funcptr actions[] =
	{
		&jogar,
		&sobre,
		&sair
	};

	actions[data.menuselection]();
}

void display_main_menu()
{
	const int menux = 512;
	const int menuy = 196;
	const int menugap = 32;

	ALLEGRO_COLOR selectedmenuitemcolor = al_map_rgb(255, 255, 0);
	ALLEGRO_COLOR unselectedmenuitemcolor = al_map_rgb(128, 128, 127);
	int index;
	for (index = 0; index < data.menuitemcount; index++)
	{
		if (index == data.menuselection)
		{
			// this item is selected, we draw its text with a highlight
			al_draw_text(font, selectedmenuitemcolor, menux, menuy + index * menugap ,ALLEGRO_ALIGN_CENTRE, menuitems[index]);
		}
		else
		{
			// the item is not selected, we just draw its text
			al_draw_text(font, unselectedmenuitemcolor, menux, menuy + index * menugap ,ALLEGRO_ALIGN_CENTRE, menuitems[index]);
		}
	}
	
}

void update_main_menu(bool key[])
{
	static bool upkey = false;
	static bool downkey = false;
	static bool enterkey = false;

	if (key[UP])
	{
		if (!upkey)
		{
			upkey = true;
		}
	}
	else
	{
		if (upkey)
		{
			if (--data.menuselection < 0)
			{
				data.menuselection = data.menuitemcount - 1;
			}
			upkey = false;
		}
	}

	if (key[DOWN])
	{
		if (!downkey)
		{
			downkey = true;
		}
	}
	else
	{
		if (downkey)
		{
			if (++data.menuselection == data.menuitemcount)
			{
				data.menuselection = 0;
			}
			downkey = false;
		}
	}

	if (key[ENTER])
	{
		if (!enterkey)
		{
			enterkey = true;
		}
	}
	else
	{
		if (enterkey)
		{
			menu_item_selected();
			enterkey = false;
		}
	}
}

void loop_menu()
{
	bool key[3] = { true, false, false };
	data.menuselection = 0;
	data.menuitemcount = sizeof(menuitems) / sizeof(const char*);
	doexit = false;
	refresh = false;
 	fundo = al_create_bitmap(800, 480);
	al_set_target_bitmap(fundo);
	al_clear_to_color(al_map_rgb(10, 120, 190));
	al_set_target_bitmap(al_get_backbuffer(display));

	while(!doexit)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_TIMER)
		{
			update_main_menu(key);
			refresh = true;
		}
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			doexit = true;
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_UP:
				key[UP] = true;
				break;

				case ALLEGRO_KEY_DOWN:
				key[DOWN] = true;
				break;

				case ALLEGRO_KEY_ENTER:
				key[ENTER] = true;
				break;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch(ev.keyboard.keycode)
			{
				case ALLEGRO_KEY_UP:
				key[UP] = false;
				break;

				case ALLEGRO_KEY_DOWN:
				key[DOWN] = false;
				break;

				case ALLEGRO_KEY_ENTER:
				key[ENTER] = false;
				break;
			}
		}
		if(refresh && al_is_event_queue_empty(event_queue))
		{
			al_draw_bitmap(fundo,112,144,0);
			display_main_menu();
			al_flip_display();
		}
	}
	al_destroy_bitmap(fundo);
}

