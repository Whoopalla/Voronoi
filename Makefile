CC=gcc
CFLAGS= -Wall -ggdb -g 

build: ./src/main.c
	$(CC) $(CFLAGS) -o main ./src/main.c
	./main
	magick output.ppm output.png

