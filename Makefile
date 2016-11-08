all: serial.cpp parallel-omp.cpp
	@g++ -std=c++11 -ggdb -gdwarf-2 serial.cpp -o serial.out -lm
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_1 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm

test: all
	@./serial.out in.txt 0 5 < in.txt > t
	@diff t out.txt
	@-rm t

ttest: all gtg test.g
	./serial.out test.g 0.5
	# ./parallel-omp.out test.g 0.5

test.g: gtg
	@./GTgraph/random/GTgraph-random -t 1 -n 100000 -m 5000000 -o test.g
	@rm log

gtg: ./GTgraph/random/GTgraph-random
	@make -s -C ./GTgraph
	@make -s -C ./GTgraph rand