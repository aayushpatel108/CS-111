#NAME: Aayush Patel
#EMAIL: aayushpatel108@ucla.edu
#ID: 105111196

default:
	gcc -Wall -Wextra -g -o lab2_list -lrt -lprofiler -lpthread lab2_list.c SortedList.c -o lab2_list

tests: default
	./lab2_list --threads=1 --iterations=1000 --sync=s > lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=10 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=20 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=40 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=80 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --sync=s --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --sync=s --yield=id --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=10 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=20 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=40 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=80 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=10 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=20 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=40 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=80 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=10 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=20 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=40 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=80 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=10 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=20 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=40 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=80 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=10 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=20 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=40 --sync=m --yield=id --lists=4 >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=80 --sync=m --yield=id --lists=4 >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

	./lab2_list --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

	
profile: default
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so
	CPUPROFILE=./raw.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_list_operations ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf profile.gperf lab2_list

graphs: tests
	./lab2b_list.gp

dist: graphs profile
	tar -czvf lab2b-105111196.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png README lab2b_list.gp

clean: 
	rm -f lab2b-105111196.tar.gz lab2_list 