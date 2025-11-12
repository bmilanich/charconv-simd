#include <charconv>
#include <random>
#include <vector>
#include <memory>
#include <chrono>
#include <iostream>
#include <string>
#include "number.h"


int main() {

  unsigned x = 0;
  char str[] = "718115";
  number::from_chars(std::begin(str),std::end(str)-1,x);

  
  constexpr std::size_t N = 100000000; // number of integers

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned> num(0,4294967295);


  std::unique_ptr<char[]> data = std::make_unique_for_overwrite<char[]>(N * 10);
  std::unique_ptr<unsigned[]> values = std::make_unique_for_overwrite<unsigned[]>(N);
  std::unique_ptr<unsigned[]> results = std::make_unique_for_overwrite<unsigned[]>(N);
  std::unique_ptr<const char *[]> offsets = std::make_unique_for_overwrite<const char *[]>(N+1);

  // generate data
  char* ptr = data.get();
  for(std::size_t i=0; i != N; ++i) {
    values[i] = num(gen);
    auto [new_ptr,ec] = std::to_chars(ptr,ptr + 32,values[i]);
    if( ec != std::errc() ) {
      return -1;
    }
    offsets[i] = ptr;
    ptr = new_ptr;
  }
  offsets[N] = ptr;
  // benchmark
  auto start = std::chrono::high_resolution_clock::now();
  for(volatile std::size_t i=0; i!=N; i = i+1) {
    unsigned x;
#ifndef __clang__    
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif    
    number::from_chars(offsets[i],offsets[i+1],x);
    if(x != values[i]){
      std::cout << "Conversion error!" << std::endl;
      return -1;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  std::cout << "number::from_chars time: " << duration.count() << std::endl;


  start = std::chrono::high_resolution_clock::now();
  for(volatile std::size_t i=0; i!=N; i = i+1) {
    unsigned x;
    std::from_chars(offsets[i],offsets[i+1],x);
    if(x != values[i]){
      std::cout << "Conversion error!" << std::endl;
      return -1;
    }
  }
  end = std::chrono::high_resolution_clock::now();
  duration = end - start;
  std::cout << "std::from_chars time: " << duration.count() << std::endl;
  
  return 0;
  
}
