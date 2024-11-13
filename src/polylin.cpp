#include <unistd.h>

#include <chrono>
#include <iostream>
#include <memory>

#include "deque.hpp"
#include "priorityqueue.hpp"
#include "queue.hpp"
#include "reader.hpp"
#include "set.hpp"
#include "stack.hpp"

using namespace polylin;

template <typename value_type>
std::unique_ptr<LinBase<value_type>> get_monitor(const std::string& type) {
  if (type == "stack") return std::make_unique<StackLin<value_type>>();
  if (type == "queue") return std::make_unique<QueueLin<value_type>>();
  if (type == "pqueue") return std::make_unique<PriorityQueueLin<value_type>>();
  if (type == "set") return std::make_unique<SetLin<value_type>>();
  if (type == "deque") return std::make_unique<DequeLin<value_type>>();
  throw std::invalid_argument("Unknown data type");
}

template <typename value_type>
time_type get_max_time(const History<value_type>& hist) {
  time_type max_time = MIN_TIME;
  for (const auto& o : hist) max_time = std::max(max_time, o.endTime);
  return max_time;
}

typedef std::unique_ptr<LinBase<DEFAULT_VALUE_TYPE>> monitor_ptr_t;
typedef History<DEFAULT_VALUE_TYPE> hist_t;
typedef HistoryReader<DEFAULT_VALUE_TYPE> hist_reader_t;
typedef std::chrono::high_resolution_clock hr_clock;

int main(int argc, char* argv[]) {
  bool incremental = false;
  bool print_time = false;
  std::string input_file;

  int flag;
  while ((flag = getopt(argc, argv, "it")) != -1) switch (flag) {
      case 'i':
        incremental = true;
        break;
      case 't':
        print_time = true;
        break;
      case '?':
        std::cerr << "Unknown option `" << optopt << "'.\n";
        return 1;
      default:
        abort();
    }
  if (optind < argc) input_file = argv[optind];

  hist_reader_t reader(input_file);
  std::string histType = reader.getHistTypeStr();
  monitor_ptr_t monitor = get_monitor<DEFAULT_VALUE_TYPE>(histType);
  auto hist = (histType == "set") ? reader.getExtHist() : reader.getHist();

  hist_t hist_copy;
  if (incremental) hist_copy = hist;  // copy for incremental use

  hr_clock::time_point start = hr_clock::now();
  bool result = monitor->distVal(hist);
  hr_clock::time_point end = hr_clock::now();
  long long time_micros =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  std::cout << result;

  if (incremental) {
    if (result) std::cout << " -1";

    // pseudo polynomial binary search
    // TODO can be optimized to fully polynomial
    time_type low = 0, high = get_max_time(hist_copy);
    while (low < high) {
      time_type mid = low + (high - low) / 2;
      hist_t subhist = monitor->getSubHist(hist_copy, mid);
      result = monitor->distVal(subhist);
      if (result)
        low = mid + 1;
      else
        high = mid;
    }

    std::cout << " " << low;
  }

  if (print_time) std::cout << " " << (time_micros / 1e6);

  std::cout << std::endl;

  return 0;
}