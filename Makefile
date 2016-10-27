all:
	@g++ -std=c++11 -ggdb -gdwarf-2 serial.cpp -o serial.out -lm

test: all
	@./serial.out in.txt 0 5 < in.txt > t
	@diff t out.txt
	@-rm t