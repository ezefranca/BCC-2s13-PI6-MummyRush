#include "interface.h"
#include "colisao.h"
#include "listas.h"
#include "util.h"

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

bool move_personagem(bool *refresh)
{
	bool moveu = false;
	float tmp_x = jogador->pos_x;
	float tmp_y = jogador->pos_y;
	int tmp_angulo = jogador->angulo;
	if(pos[KEY_UP])
	{
		if(pos[KEY_LEFT] || pos[KEY_RIGHT])
			tmp_y -= velocidade*0.7;
		else
			tmp_y -= velocidade;
		tmp_angulo = ANG_90;
	}
 
	if(pos[KEY_DOWN])
	{
		if(pos[KEY_LEFT] || pos[KEY_RIGHT])
			tmp_y += velocidade*0.7;
		else
			tmp_y += velocidade;
		tmp_angulo = ANG_270;
	}
 
       	if(pos[KEY_LEFT])
	{
		if(pos[KEY_UP] || pos[KEY_DOWN])
			tmp_x -= velocidade*0.7;
		else
			tmp_x -= velocidade;
		tmp_angulo = ANG_180;
	}
 
	if(pos[KEY_RIGHT])
	{
		if(pos[KEY_UP] || pos[KEY_DOWN])
			tmp_x += velocidade*0.7;
		else
			tmp_x += velocidade;
		tmp_angulo = ANG_0;
	}
	if((jogador->pos_x != tmp_x || jogador->pos_y != tmp_y)
		&& verifica_colisao(tmp_x, tmp_y, BOUNCER_SIZE, tmp_angulo))
	{
		*refresh = true;
		moveu = true;
		jogador->pos_x = tmp_x;
		jogador->pos_y = tmp_y;
		jogador->angulo = tmp_angulo;
	}
	return moveu;
}

bool move_mira(bool *refresh)
{
	bool moveu = false;
	/*
	if(mir[KEY_UP] && mir[KEY_RIGHT])
	{
		novo_tiro->angulo = ANG_45;
		moveu = true;
	}
	else if(mir[KEY_UP] && mir[KEY_LEFT])
	{
		novo_tiro->angulo = ANG_135;
		moveu = true;
	}
	else if(mir[KEY_DOWN] && mir[KEY_RIGHT])
	{
		novo_tiro->angulo = ANG_315;
		moveu = true;
	}
	else if(mir[KEY_DOWN] && mir[KEY_LEFT])
	{
		novo_tiro->angulo = ANG_225;
		moveu = true;
	}*/
	if(mir[KEY_RIGHT])
	{
		novo_tiro->angulo = ANG_0;
		moveu = true;
	}
	else if(mir[KEY_UP])
	{
		novo_tiro->angulo = ANG_270;
		moveu = true;
	}
	else if(mir[KEY_LEFT])
	{
		novo_tiro->angulo = ANG_180;
		moveu = true;
	}
	else if(mir[KEY_DOWN])
	{
		novo_tiro->angulo = ANG_90;
		moveu = true;
	}

	if(moveu)
	{	
		switch(novo_tiro->angulo)
		{
			case ANG_0:
			novo_tiro->pos_x = jogador->pos_x+BOUNCER_SIZE;
			novo_tiro->pos_y = jogador->pos_y+BOUNCER_SIZE/2;			
			break;
			case ANG_45:
			novo_tiro->pos_x = jogador->pos_x+BOUNCER_SIZE;
			novo_tiro->pos_y = jogador->pos_y;			
			break;
			case ANG_270:
			novo_tiro->pos_x = jogador->pos_x+BOUNCER_SIZE/2;
			novo_tiro->pos_y = jogador->pos_y;			
			break;
			case ANG_135:
			novo_tiro->pos_x = jogador->pos_x;
			novo_tiro->pos_y = jogador->pos_y;			
			break;
			case ANG_180:
			novo_tiro->pos_x = jogador->pos_x;
			novo_tiro->pos_y = jogador->pos_y+BOUNCER_SIZE/2;			
			break;
			case ANG_225:
			novo_tiro->pos_x = jogador->pos_x;
			novo_tiro->pos_y = jogador->pos_y+BOUNCER_SIZE;			
			break;
			case ANG_90:
			novo_tiro->pos_x = jogador->pos_x+BOUNCER_SIZE/2;
			novo_tiro->pos_y = jogador->pos_y+BOUNCER_SIZE;			
			break;
			case ANG_315:
			novo_tiro->pos_x = jogador->pos_x+BOUNCER_SIZE;
			novo_tiro->pos_y = jogador->pos_y+BOUNCER_SIZE;			
			break;
		}
		jogador->angulo = novo_tiro->angulo;
	}
	*refresh = true;
	return moveu;
}

void finaliza_jogo()
{
	al_destroy_bitmap(bg);
	al_destroy_bitmap(fundo);
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

	font = al_load_ttf_font("Egyptian Nights.ttf",48,0 );
   
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
		fprintf(stderr, "Could not load 'Egyptian Nights.ttf'.\n");
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
	fundo = al_load_bitmap("MUMMY_RUSH.jpg");
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,100,100,0);
	al_flip_display();
 
	al_rest(2.0);
}

void exibeTelaEspera()
{
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,100,100,0);
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),(SCREEN_H/2),ALLEGRO_ALIGN_CENTRE, "Aguardando  jogadores");

	al_flip_display();
 
	al_rest(2.0);
}

void jogar()
{
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,100,100,0);
	al_flip_display();

	ALLEGRO_USTR *input = al_ustr_new("");

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
			al_draw_bitmap(fundo,100,100,0);
			al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),200 ,ALLEGRO_ALIGN_CENTRE, "Endereco do Servidor");

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
	al_set_target_bitmap(al_get_backbuffer(display));
	al_draw_bitmap(fundo,100,100,0);

	//desenha nomes
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),200 ,ALLEGRO_ALIGN_CENTRE, "Programacao");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),260 ,ALLEGRO_ALIGN_CENTRE, "Graziella Bonizi");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),300 ,ALLEGRO_ALIGN_CENTRE, "Lucas Chao");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),360 ,ALLEGRO_ALIGN_CENTRE, "Design");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),420 ,ALLEGRO_ALIGN_CENTRE, "Renan Valeiro");
	al_draw_text(font, al_map_rgb(0,0,0), (SCREEN_W/2),460 ,ALLEGRO_ALIGN_CENTRE, "Guilherme Esteves");

	al_draw_text(font, al_map_rgb(255,255,0), (SCREEN_W/2),520 ,ALLEGRO_ALIGN_CENTRE, "Voltar");
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
	const int menuy = 296;
	const int menugap = 40;

	ALLEGRO_COLOR selectedmenuitemcolor = al_map_rgb(255, 255, 0);
	ALLEGRO_COLOR unselectedmenuitemcolor = al_map_rgb(0, 0, 0);
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
			al_draw_bitmap(fundo,100,100,0);
			display_main_menu();
			al_flip_display();
		}
	}
}

