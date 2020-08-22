# NAME: Aayush Patel
# EMAIL: aayushpatel108@ucla.edu
# UID: 105111196

CC = gcc
CFLAGS = -g -o

.SILENT: 

default:
	gcc -g -lz lab1b-server.c -Wall -Wextra -o lab1b-server
	gcc -g -lz lab1b-client.c -Wall -Wextra -o lab1b-client

clean:
	rm -rf  lab1b-server lab1b-client *.txt lab1b-105111196.tar.gz

dist: default
	  tar -czvf lab1b-105111196.tar.gz lab1b-server.c lab1b-client.c Makefile README
