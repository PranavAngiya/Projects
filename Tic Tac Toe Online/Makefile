objects = ttts 
CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address -pthread

all: $(objects)

ttts: ttts.o
	$(CC) $(CFLAGS) $^ -o $@ 