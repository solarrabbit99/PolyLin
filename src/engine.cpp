#include <iostream>

#include "reader.hpp"
#include "stack.hpp"

using namespace polylin;

int main(int argc, const char* argv[]) {
  if (argc < 2) return -1;
  HistoryReader reader;
  History hist = reader.readFile(argv[1]);
  std::cout << StackLin().stackDistValLin(hist) << std::endl;
}