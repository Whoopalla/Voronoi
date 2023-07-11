CC=gcc
CFLAGS=-Wall -ggdb 


voronoiMake: main.o
	$(CC) $(CFLAGS) -o main main.o 

