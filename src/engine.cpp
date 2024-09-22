#include <iostream>

#include "deque.hpp"
#include "priorityqueue.hpp"
#include "queue.hpp"
#include "reader.hpp"
#include "set.hpp"
#include "stack.hpp"

using namespace polylin;

int main(int argc, const char *argv[]) {
  if (argc < 2) return -1;
  HistoryReader<DEFAULT_VALUE_TYPE> reader(argv[1]);
  std::string histType = reader.getHistTypeStr();

  if (histType == "set") {
    History<DEFAULT_VALUE_TYPE> hist = reader.getExtHist();
    std::cout << SetLin<DEFAULT_VALUE_TYPE>().distVal(hist) << std::endl;
    return 0;
  }

  History<DEFAULT_VALUE_TYPE> hist = reader.getHist();

  if (histType == "stack")
    std::cout << StackLin<DEFAULT_VALUE_TYPE>().distVal(hist) << std::endl;
  else if (histType == "queue")
    std::cout << QueueLin<DEFAULT_VALUE_TYPE>().distVal(hist) << std::endl;
  else if (histType == "pqueue")
    std::cout << PriorityQueueLin<DEFAULT_VALUE_TYPE>().distVal(hist)
              << std::endl;
  else if (histType == "deque")
    std::cout << DequeLin<DEFAULT_VALUE_TYPE>().distVal(hist) << std::endl;
}