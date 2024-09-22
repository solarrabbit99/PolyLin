#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#define value_type uint32_t
#define NUM_OPER_INDEX 1
#define NUM_PROD_INDEX 2
#define NUM_CONS_INDEX 3

std::mutex cond_mutex;
std::condition_variable cond_var;
std::mutex print_mutex;
std::priority_queue<value_type> shm;

uint32_t time() {
  static std::atomic_uint32_t time{0};
  return time.fetch_add(1, std::memory_order_acq_rel);
}

void producer(size_t tid, size_t num_oper) {
  std::vector<uint32_t> times;

  for (size_t i; i < num_oper; ++i) {
    times.emplace_back(time());
    std::scoped_lock lock{cond_mutex};
    shm.push((tid << 17) + i);  // 17 bits used for item
    times.emplace_back(time());
    cond_var.notify_one();
  }

  // print summary
  for (size_t i; i < num_oper; ++i) {
    std::scoped_lock lock{print_mutex};
    std::cout << "insert " << ((tid << 17) + i) << " " << times[i * 2] << " "
              << times[i * 2 + 1] << std::endl;
  }
}

void consumer(size_t num_oper) {
  std::vector<uint32_t> times;
  std::vector<value_type> values;
  std::vector<bool> isPoll;

  while (num_oper) {
    times.emplace_back(time());
    std::unique_lock lock{cond_mutex};
    cond_var.wait(lock, []() { return shm.size(); });
    auto value = shm.top();
    values.push_back(value);
    if (std::rand() % 2) {
      isPoll.push_back(false);
    } else {
      shm.pop();
      isPoll.push_back(true);
      --num_oper;
    }
    times.emplace_back(time());
  }

  // print summary
  for (size_t i; i < values.size(); ++i) {
    std::scoped_lock lock{print_mutex};
    std::cout << (isPoll[i] ? "poll " : "peek ") << values[i] << " "
              << times[i * 2] << " " << times[i * 2 + 1] << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << "usage: ./pqueue_histgen <num_oper> <num_prod> <num_cons>"
              << std::endl;
    return -1;
  }

  std::cout << "# pqueue" << std::endl;

  int num_oper = std::stoi(argv[NUM_OPER_INDEX]);
  int num_prod = std::stoi(argv[NUM_PROD_INDEX]);
  int num_cons = std::stoi(argv[NUM_CONS_INDEX]);

  std::vector<std::thread> threads;

  for (size_t i = 0; i < num_cons; ++i)
    threads.emplace_back(consumer, num_oper);

  for (size_t i = 0; i < num_prod; ++i)
    threads.emplace_back(producer, i, num_oper);

  for (std::thread& thread : threads) thread.join();
}