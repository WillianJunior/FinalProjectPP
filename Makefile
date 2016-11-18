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
	@./GTgraph/random/GTgraph-random -t 1 -n 1000000 -m 30000000 -o test.g
	@rm log

gtg: ./GTgraph/random/GTgraph-random
	@make -s -C ./GTgraph
	@make -s -C ./GTgraph rand

alltest: gtg test.g exp0 exp1 exp2

exp0:
	# serial
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -lm
	sh test_once.sh 6 results0.out 1 holy_mary.out

exp1:
	# relax1
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D RELAX_PAR1 -fopenmp -lm
	sh test_once.sh 6 results1.out 1 holy_mary.out

	# relax2
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D RELAX_PAR2 -fopenmp -lm
	sh test_once.sh 6 results1.out 1 holy_mary.out

	# both
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D RELAX_PAR1 -D RELAX_PAR2 -fopenmp -lm
	sh test_once.sh 6 results1.out 1 holy_mary.out

exp2: 
	# heavy tests
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D GRANULARITY_0 -D HEAVY_PAR -fopenmp -lm
	sh test_once.sh 6 results2.out 1 holy_mary.out

	# light tests
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D GRANULARITY_0 -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 6 results2.out 1 holy_mary.out

	# both tests
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D GRANULARITY_0 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 6 results2.out 1 holy_mary.out

exp3:
	# G0 tests
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D GRANULARITY_0 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 6 results3.out 1 holy_mary.out

	# G1 tests
	@g++ -std=c++11 -ggdb -gdwarf-2 holy_mary.cpp -o holy_mary.out -D GRANULARITY_1 -D HEAVY_PAR -D LIGHT_PAR -fopenmp -lm
	sh test_once.sh 6 results3.out 1 holy_mary.out
