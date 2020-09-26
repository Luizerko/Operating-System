CFLAGS = -g -Wall
LFLAGS = -lreadline
LFLAGS2 = -pthread

all: bccsh gera_arquivo_simulador ep1

bccsh: bccsh.o
	gcc bccsh.o -o bccsh ${LFLAGS}

bccsh.o: bccsh.c
	gcc $(CFLAGS) bccsh.c -c 

gera_arquivo_simulador: gera_arquivo_simulador.o
	gcc gera_arquivo_simulador.o -o gera

gera_arquivo_simulador.o: gera_arquivo_simulador.c
	gcc $(CFLAGS) gera_arquivo_simulador.c -c

ep1: ep1.o queue.o
	gcc -o ep1 ep1.o queue.o ${LFLAGS2}

ep1.o: ep1.c
	gcc $(CFLAGS) ep1.c -c

queue.o: queue.c
	gcc $(CFLAGS) queue.c -c

clean:
	rm -rf *.o bccsh gera ep1
