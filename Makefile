#Makefile for cabinet

CC = gcc
CFLAGS = -c -Wall

OBJ = cabinet

all: run

$(OBJ): fsdriver3.o
	$(CC)  fsdriver3.o -o $(OBJ) -lm

fsdriver3.o: fsdriver3.c
	$(CC) $(CFLAGS) fsdriver3.c

run: $(OBJ)
	./cabinet newfs
exec:
	./cabinet newfs
clean:
	rm -rf cabinet fsdriver3.o

PHONY: all run
