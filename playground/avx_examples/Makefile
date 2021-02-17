# CC=g++ -std=c++14 -mavx -O3
CC=g++ -std=c++14 -mavx -O2 -mfma

all: main_mul main_add main_fma

main_mul: main_mul.cpp
	$(CC) main_mul.cpp -o bench_mul

main_add: main_add.cpp
	$(CC) main_add.cpp -o bench_add

main_fma: main_fma.cpp
	$(CC) main_fma.cpp -o bench_fma
