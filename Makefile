CFLAGS=-gcc -g -Wall
CC = gcc
ush: ush.o expand.o builtin.o
	$((CC) $(CFLAGS) -o ush ush.o builtin.o expand.o

ush.o: ush.c
	$((CC) $(CFLAGS) -c -o ush.o ush.c

expand.o: expand.c
		$((CC) $(CFLAGS) -c -o expand.o expand.c

builtin.o: builtin.c
	$((CC) $(CFLAGS) -c -o builtin.o builtin.c
