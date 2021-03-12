CXX = g++ -std=c++20

all: chomp solver

chomp: chomp.cpp chomp.h
	$(CXX) -O3 chomp.cpp -o chomp

solver: solver.cpp
	$(CXX) -O3 solver.cpp -o solver

solver_compressed: solver.cpp
	$(CXX) -O3 -D COMPRESS solver.cpp -o solver_compressed

debug: chomp.cpp chomp.h
	$(CXX) -g3 -DDEBUG chomp.cpp -o chomp_debug

clean:
	rm chomp chomp_debug solver

# expect 4.5 minutes of computing time
# output is stored in binary file "16x", 12.7 MiB
# can be compressed to 4.0 MiB using code commented out in solver.cpp
# decompress program left as exercise to reader
solve: solver
	./solver

.PHONY: all chomp solver solver_compressed debug clean solve
# static:
#		cppcheck --enable=all --suppress=missingIncludeSystem *.h *.cpp