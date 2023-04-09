# Copyright 2023 Radu Constantinescu <raduic2003@gmail.com>

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS=vma

build: $(TARGETS)

run_vma:
	./vma

vma: main.c vma.c
	$(CC) $(CFLAGS) main.c vma.c -o vma

pack:
	zip -FSr 313CA_ConstantinescuRadu_Ioan_Tema1.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)

.PHONY: pack clean
