CC = gcc
CFLAGS = -O3

all:
	$(CC) myshell.c -o myshell $(CFLAGS)

run_interactive:
	./myshell

run_batch:
	./myshell batchfile.txt

purge:
	rm myshell
	rm batchfile.txt
