#include <iostream>
#include <memory>

#include "deque.hpp"
#include "priorityqueue.hpp"
#include "queue.hpp"
#include "reader.hpp"
#include "set.hpp"
#include "stack.hpp"

using namespace polylin;

typedef std::unique_ptr<LinBase<DEFAULT_VALUE_TYPE>> monitor_ptr_t;

struct polylin_params {
  bool incremental;
  bool print_time;
  std::string input_file;
};

monitor_ptr_t getMonitor(const std::string& type) {
  if (type == "stack") return std::make_unique<StackLin<DEFAULT_VALUE_TYPE>>();
  if (type == "queue") return std::make_unique<QueueLin<DEFAULT_VALUE_TYPE>>();
  if (type == "pqueue")
    return std::make_unique<PriorityQueueLin<DEFAULT_VALUE_TYPE>>();
  if (type == "set") return std::make_unique<SetLin<DEFAULT_VALUE_TYPE>>();
  if (type == "deque") return std::make_unique<DequeLin<DEFAULT_VALUE_TYPE>>();
  throw std::invalid_argument("Unknown data type");
}

int main(int argc, const char* argv[]) {
  if (argc < 2) return -1;

  polylin_params params{false, false, {}};

  HistoryReader<DEFAULT_VALUE_TYPE> reader(argv[1]);
  std::string histType = reader.getHistTypeStr();
  monitor_ptr_t monitor = getMonitor(histType);

  auto hist = (histType == "set") ? reader.getExtHist() : reader.getHist();
  std::cout << monitor->distVal(hist) << std::endl;

  return 0;
}