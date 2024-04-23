#include <iostream>

#include "reader.hpp"
#include "stack.hpp"

int main(int argc, const char* argv[]) {
  if (argc < 2) return -1;
  History hist = readFile(argv[1]);
  std::cout << stackDistValLin(hist) << std::endl;
}