all: build

build:
	gcc -Wall -Werror shell.c read.c write.c transmit.c receive.c list.c -o s-talk -lpthread
	
clean:
	rm -f s-talk