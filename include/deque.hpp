#pragma once

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class DequeLin {
 public:
  // Assumption: at most one `PUSH`, valid deque operations.
  // Time complexity: O(n^4)
  bool distVal(History& hist);

 private:
  struct DistValParams {
    std::vector<std::tuple<time_type, bool, Operation>> events;
    std::unordered_set<value_type> oneSidedVals;
    std::unordered_map<value_type, size_t> sizeByVal;
    std::unordered_map<value_type, size_t> frontSizeByVal;
    std::unordered_set<value_type> pushFrontVals;
    std::unordered_set<value_type> popFrontVals;
  };

  // Returns `false` if there is value where `POP` methods > `PUSH` methods.
  // O(n)
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `POP` operation) O(n)
  bool tune(History& hist, const std::unordered_set<value_type>& oneSidedVals);

  // O(n log n) for testing stack subinstance
  bool getOneSidedVals(const History& hist,
                       std::unordered_set<value_type>& vals);

  static bool isFrontMethod(Method method);

  static bool isPush(Method method);

  static bool isPop(Method method);

  static bool isPeek(Method method);

  void findGoodVals(const size_t& i, const size_t& j,
                    const DistValParams& params,
                    std::unordered_set<value_type>& goodVals);

  bool distValHelper(const size_t& i, const size_t& j,
                     std::vector<std::vector<std::optional<bool>>>& distValMat,
                     const DistValParams& params);

  struct OngoingSet {
    std::unordered_set<value_type> ongoingPush;
    std::unordered_map<value_type, std::unordered_set<id_type>> ongoingPeek;
    std::unordered_set<value_type> ongoingPop;
    std::unordered_set<value_type> peekable, popable;
    std::unordered_set<value_type> peekableNow, popableNow;

    void insert(const Operation& o) {
      if (isPush(o.method))
        ongoingPush.insert(o.value);
      else if (isPop(o.method)) {
        ongoingPop.insert(o.value);
        if (popable.count(o.value)) popableNow.insert(o.value);
      } else {
        ongoingPeek[o.value].insert(o.id);
        if (peekable.count(o.value)) peekableNow.insert(o.value);
      }
    }

    void erase(const Operation& o) {
      if (isPush(o.method))
        ongoingPush.erase(o.value);
      else if (isPop(o.method)) {
        ongoingPop.erase(o.value);
        popableNow.erase(o.value);
      } else {
        ongoingPeek[o.value].erase(o.id);
        if (ongoingPeek[o.value].empty()) {
          ongoingPeek.erase(o.value);
          peekableNow.erase(o.value);
        }
      }
    }

    bool contains(const Operation& o) {
      if (isPush(o.method))
        return ongoingPush.count(o.value);
      else if (isPop(o.method))
        return ongoingPop.count(o.value);
      else
        return ongoingPeek.count(o.value) && ongoingPeek[o.value].count(o.id);
    }

    void markPeekable(const value_type& v) {
      peekable.insert(v);
      if (ongoingPeek.count(v)) peekableNow.insert(v);
    }

    void markPopable(const value_type& v) {
      popable.insert(v);
      if (ongoingPop.count(v)) popableNow.insert(v);
    }
  };
};

}  // namespace polylin