CC=$(CROSS_COMPILE)gcc

all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) -o aesdsocket aesdsocket.o

aesdsocket.o: aesdsocket.c
	$(CC) -c aesdsocket.c

clean:
	rm -f aesdsocket *.o