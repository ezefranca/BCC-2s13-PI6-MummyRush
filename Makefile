FLAGS = -Wall -std=c99

all: cliente servidor

util.o: util.c
	gcc $(FLAGS) -c util.c -o util.o

listas.o: listas.c
	gcc $(FLAGS) -c listas.c -o listas.o

colisao.o: colisao.c
	gcc $(FLAGS) -c colisao.c -o colisao.o

interface.o: interface.c
	gcc $(FLAGS) -c interface.c -o interface.o

cliente: util.o listas.o colisao.o interface.o cliente.c
	gcc $(FLAGS) util.o listas.o colisao.o interface.o cliente.c -o cliente -lallegro -lallegro_image -lallegro_primitives -lallegro_font -lallegro_ttf -lm -lpthread

servidor: util.o listas.o servidor.c
	gcc $(FLAGS) util.o listas.o servidor.c -o servidor -lm

clean:
	rm -f servidor cliente util.o listas.o colisao.o interface.o

install:
	

uninstall:

