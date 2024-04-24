#include <iostream>

#include "queue.hpp"
#include "reader.hpp"
#include "stack.hpp"

using namespace polylin;

int main(int argc, const char* argv[]) {
  if (argc < 2) return -1;
  HistoryReader reader;
  History hist = reader.getHist(argv[1]);
  std::string histType = reader.getHistTypeStr(argv[1]);

  if (histType == "stack")
    std::cout << StackLin().distVal(hist) << std::endl;
  else if (histType == "queue")
    std::cout << QueueLin().distVal(hist) << std::endl;
}