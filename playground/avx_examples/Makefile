# CC=g++ -std=c++14 -mavx -O3
# CC=g++ -std=c++14 -mavx -mavx2 -O3 -mfma
CC=g++ -std=c++14 -O3 -mavx -mfma -mavx2 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store -Wno-deprecated -fvisibility-inlines-hidden -fopenmp -DNDEBUG -O3 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow

all: main_mul main_add main_fma main_add_uint8

main_mul: main_mul.cpp
	$(CC) main_mul.cpp -o bench_mul

main_add: main_add.cpp
	$(CC) main_add.cpp -o bench_add

main_fma: main_fma.cpp
	$(CC) main_fma.cpp -o bench_fma

main_add_uint8: main_add_uint8.cpp
	$(CC) main_add_uint8.cpp -o bench_add_uint8

check_add_uint8: check_add_uint8.cpp
	$(CC) check_add_uint8.cpp -o check_add_uint8
