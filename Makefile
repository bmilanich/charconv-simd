CXX=clang++-19
CXXFLAGS=-g -O3 -std=c++23 -mavx512f -march=native
#CXXFLAGS=-g -O0 -std=c++23 -mavx512f -march=native

all : test

clean :
	rm -rf test *.o

test : test.cc number.h
	$(CXX) $(CXXFLAGS) test.cc -o test
