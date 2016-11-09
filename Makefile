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

alltest: all gtg test.g exp1 exp2

exp1: 
	# heavy tests
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_0 -D HEAVY_PAR -fopenmp -lm
	sh test_once.sh 10 results1.out 0
	sh test_once.sh 10 results1.out 5
	sh test_once.sh 10 results1.out 10

	# light tests
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_0 -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 10 results1.out 0
	sh test_once.sh 10 results1.out 5
	sh test_once.sh 10 results1.out 10

	# both tests
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_0 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 10 results1.out 0
	sh test_once.sh 10 results1.out 5
	sh test_once.sh 10 results1.out 10

exp2:
	# G0 tests
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_0 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 10 results2.out 5

	# G1 tests
	@g++ -std=c++11 -ggdb -gdwarf-2 parallel-omp.cpp -o parallel-omp.out -D GRANULARITY_0 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 10 results2.out 5
