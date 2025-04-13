CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

all: main

main: main.c
	$(CC) $(CFLAGS) -o ASTRAemu.o main.c -l raylib 

clean:
	rm -f ASTRAemu.o

