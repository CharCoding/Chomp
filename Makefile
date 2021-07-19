CXX = g++ -std=c++17

# There are a bunch of warnings with -Wsign-conversion, but who cares...
chomp: chomp.cpp chomp.h
	$(CXX) -O2 chomp.cpp -o chomp

# need .contains() from c++20
# this is also 3 times slower than phmap_solver
solver: solver.cpp
	g++ -std=c++20 -O3 solver.cpp -o solver

# should take less than 30 seconds
phmap_solver: solver.cpp
	$(CXX) -O3 -DPHMAP solver.cpp -o solver || echo "Get phmap: https://github.com/greg7mdp/parallel-hashmap"

debug: chomp.cpp chomp.h
	$(CXX) -g3 -DDEBUG chomp.cpp -o chomp_debug

all: chomp debug phmap_solver

clean:
	rm chomp chomp_debug solver

# expect < 30 seconds of computing time
# output is stored in binary file "16x", 6.33 MiB
# can be compressed using unranking and differential encoding (see commented out code in compress.cpp)
# compression and decompression program left as exercise to reader
solve: phmap_solver
	time ./solver

.PHONY: all chomp phmap_solver solver debug clean solve
# static:
#		cppcheck --enable=all --suppress=missingIncludeSystem *.h *.cpp