ifdef CROSS_COMPILE
	cc = $(CROSS_COMPILE)gcc
else
	cc = gcc
endif

all: 
	$(cc) ./writer.c -o writer

clean:
	rm -rf writer *.o
