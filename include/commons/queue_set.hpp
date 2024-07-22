#include <queue>
#include <unordered_set>

namespace polylin {

// Template queue set class supporting amortized O(1) insertion to queue and
// removal from anywhere from the queue.
template <typename T>
class queue_set {
 public:
  void enqueue(T&& item) {
    q.emplace(item);
    s.emplace(item);
  }

  T dequeue() {
    while (!s.count(q.front())) q.pop();
    T item{q.front()};
    s.erase(item);
    q.pop();
    return item;
  }

  T& front() {
    while (!s.count(q.front())) q.pop();
    return q.front();
  }

  T remove(const T& item) {
    T clone{*s.find(item)};
    s.erase(item);
    return clone;
  }

  bool contains(const T& item) const { return s.count(item); }

  bool empty() const { return s.empty(); }

 private:
  std::queue<T> q;
  std::unordered_set<T> s;

  friend void swap(queue_set<T>& lhs, queue_set<T>& rhs) {
    std::swap(lhs.q, rhs.q);
    std::swap(lhs.s, rhs.s);
  }
};

}  // namespace polylin