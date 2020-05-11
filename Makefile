#Makefile for cabinet

CC = gcc
CFLAGS = -c -Wall

OBJ = cabinet

all: $(OBJ)

$(OBJ): fsdriver3.o
	$(CC)  fsdriver3.o -o $(OBJ)

fsdriver3.o: fsdriver3.c
	$(CC) $(CFLAGS) fsdriver3.c


exec:
	./cabinet
clean:
	rm -rf cabinet fsdriver3.o

