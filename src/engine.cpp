#include <iostream>

#include "priorityqueue.hpp"
#include "queue.hpp"
#include "reader.hpp"
#include "stack.hpp"

using namespace polylin;

int main(int argc, const char *argv[]) {
  if (argc < 2) return -1;
  // OperationHistoryReader reader(argv[1]);
  EventHistoryReader reader(argv[1]);
  History hist = reader.getHist();
  std::string histType = reader.getHistTypeStr();

  if (histType == "stack")
    std::cout << StackLin().distVal(hist) << std::endl;
  else if (histType == "queue")
    std::cout << QueueLin().distVal(hist) << std::endl;
  else if (histType == "pqueue")
    std::cout << PriorityQueueLin().distVal(hist) << std::endl;
}