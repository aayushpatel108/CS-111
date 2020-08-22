# NAME: Aayush Patel
# EMAIL: aayushpatel108@ucla.edu
# UID: 105111196

CC = gcc
CFLAGS = -g -o

.SILENT: 

default:
	gcc -g -lz server.c -o server
	gcc -g -lz client.c -o client

clean:
	rm -rf  lab1a *.o *.txt *.tar.gz

dist: default
	  tar -czvf lab1b-105111196.tar.gz server.c client.c Makefile README
