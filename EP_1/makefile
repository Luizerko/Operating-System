CFLAGS = -g -Wall
LFLAGS = -lreadline

all: bccsh

bccsh: bccsh.o
	gcc bccsh.o -o bccsh ${LFLAGS}

bccsh.o: bccsh.c
	gcc $(CFLAGS) bccsh.c -c 

gera_arquivo_simulador: gera_arquivo_simulador.o
	gcc gera_arquivo_simulador.o -o gera

gera_arquivo_simulador.o: gera_arquivo_simulador.c
	gcc $(CFLAGS) gera_arquivo_simulador.c -c

ep1: ep1.o
	gcc ep1.o -o ep1

ep1.o: ep1.c
	gcc $(CFLAGS) ep1.c -c

clean:
	rm -rf *.o bccsh gera ep1
