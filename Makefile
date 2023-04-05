CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: main

main: main.o vma.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c vma.h
	$(CC) $(CFLAGS) -c $< -o $@

vma.o: vma.c vma.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main *.o
