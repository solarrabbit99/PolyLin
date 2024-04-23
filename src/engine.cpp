#include <iostream>

#include "reader.hpp"
#include "stack.hpp"

int main(int argc, const char* argv[]) {
  if (argc < 2) return -1;
  HistoryReader reader;
  History hist = reader.readFile(argv[1]);
  std::cout << stackDistValLin(hist) << std::endl;
}