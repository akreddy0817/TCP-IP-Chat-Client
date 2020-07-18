#Avaneesh Kolluri worked with Akhilesh Reddy
#I pledge my honor that I have abided by the Stevens Honor System.
CC     = gcc
C_FILE = $(wildcard *.c)
CFLAGS = -O3 -Wall -Werror -pedantic-errors

all:
	$(CC) $(CFLAGS) -o chatclient chatclient.c
clean:
	rm -f chatclient chatclient.exe
