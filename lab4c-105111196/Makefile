#NAME: Aayush Patel
#ID: 105111196
#EMAIL: aayushpatel108@ucla.edu

.SILENT:

default:
	gcc -g -Wall -Wextra -lm -lmraa lab4c_tcp.c -o lab4c_tcp 
	gcc -g -Wall -Wextra  -lm -lmraa -lcrypto -lssl lab4c_tls.c -o lab4c_tls 

clean:
	rm -rf *.o *.tar.gz lab4c_tcp lab4c_tls *.txt

dist: default
	tar -cvzf lab4c-105111196.tar.gz README Makefile lab4c_tcp.c lab4c_tls.c  