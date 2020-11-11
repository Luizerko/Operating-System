LFLAGS = -lreadline

all: ep3

ep3: ep3.o
	gcc ep3.o -o ep3 ${LFLAGS}

ep3.o: ep3.c
	gcc -g -Wall ep3.c -c

clean:
	rm -rf *.o ep3
