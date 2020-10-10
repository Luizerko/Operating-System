CFLAGS = -g -Wall
LFLAGS = -pthread

all: ep2

ep2: ep2.o
	gcc -o ep2 ep2.o ${LFLAGS}

ep2.o: ep2.c
	gcc $(CFLAGS) ep2.c -c

clean:
	rm -rf *.o ep2
