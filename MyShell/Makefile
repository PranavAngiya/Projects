objects = mysh
CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address

all: $(objects)

mysh: mysh.o
	$(CC) $(CFLAGS) $^ -o $@
