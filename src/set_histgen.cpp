#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#define value_type uint32_t
#define NUM_OPER_INDEX 1
#define NUM_PROD_INDEX 2
#define NUM_CONS_INDEX 3

std::mutex cond_mutex;
std::condition_variable* cond_vars;
std::mutex print_mutex;
std::unordered_set<value_type> shm;

uint32_t time() {
  static std::atomic_uint32_t time{0};
  return time.fetch_add(1, std::memory_order_acq_rel);
}

void producer(size_t tid, size_t num_oper) {
  std::vector<uint32_t> times;

  for (size_t i; i < num_oper; ++i) {
    times.emplace_back(time());
    std::scoped_lock lock{cond_mutex};
    shm.insert((tid << 17) + i);  // 17 bits used for item
    times.emplace_back(time());
    cond_vars[tid].notify_one();
  }

  // print summary
  for (size_t i; i < num_oper; ++i) {
    std::scoped_lock lock{print_mutex};
    std::cout << "insert " << ((tid << 17) + i) << " 1 " << times[i * 2] << " "
              << times[i * 2 + 1] << std::endl;
  }
}

void consumer(size_t tid, size_t num_oper) {
  std::vector<uint32_t> times;
  std::vector<value_type> values;
  std::vector<bool> retVal;
  std::vector<int> isRem;

  size_t i = 0;
  while (i < num_oper) {
    value_type value = (tid << 17) + i;
    // phase 1: log first
    {
      times.emplace_back(time());
      std::scoped_lock lock{cond_mutex};
      if (shm.count(value)) {
        values.push_back(value);
        retVal.push_back(true);
        isRem.push_back(false);
      } else {
        values.push_back(value);
        retVal.push_back(false);
        isRem.push_back(false);
      }
      times.emplace_back(time());
    }
    // phase 2: wait for value to be in set, roll dice to remove
    {
      times.emplace_back(time());
      std::unique_lock lock{cond_mutex};
      cond_vars[tid].wait(lock, [&]() { return shm.count(value); });
      if (std::rand() & 1) {  // remove
        values.push_back(value);
        retVal.push_back(true);
        isRem.push_back(true);
        shm.erase(value);
        ++i;
      } else {
        values.push_back(value);
        retVal.push_back(true);
        isRem.push_back(false);
      }
      times.emplace_back(time());
    }
    // phase 3: log last
    {
      times.emplace_back(time());
      std::scoped_lock lock{cond_mutex};
      if (shm.count(value)) {
        values.push_back(value);
        retVal.push_back(true);
        isRem.push_back(false);
      } else {
        values.push_back(value);
        retVal.push_back(false);
        isRem.push_back(false);
      }
      times.emplace_back(time());
    }
    // repeat
  }

  // print summary
  for (size_t i; i < values.size(); ++i) {
    if (isRem[i]) {
      std::scoped_lock lock{print_mutex};
      std::cout << "remove " << values[i] << " 1 " << times[i * 2] << " "
                << times[i * 2 + 1] << std::endl;
      continue;
    }
    if (std::rand() & 1) {  // contains
      std::scoped_lock lock{print_mutex};
      std::cout << "contains " << values[i] << " " << retVal[i] << " "
                << times[i * 2] << " " << times[i * 2 + 1] << std::endl;
      continue;
    }
    // replace
    std::scoped_lock lock{print_mutex};
    std::cout << (retVal[i] ? "insert " : "remove ") << values[i] << " 0 "
              << times[i * 2] << " " << times[i * 2 + 1] << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cout << "usage: ./set_histgen <num_oper> <num_prod>" << std::endl;
    return -1;
  }

  std::cout << "# set" << std::endl;

  int num_oper = std::stoi(argv[NUM_OPER_INDEX]);
  int num_prod = std::stoi(argv[NUM_PROD_INDEX]);

  std::vector<std::thread> threads;
  cond_vars = new std::condition_variable[num_prod];

  for (size_t i = 0; i < num_prod; ++i) {
    threads.emplace_back(consumer, i, num_oper);
    threads.emplace_back(producer, i, num_oper);
  }

  for (std::thread& thread : threads) thread.join();
}