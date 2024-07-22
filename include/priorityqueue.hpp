#pragma once

#include <map>
#include <set>

#include "base.hpp"

namespace polylin {

template <typename value_type>
class PriorityQueueLin : LinBase<value_type> {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  PriorityQueueLin() : LinBase<value_type>{{INSERT}, {POLL}} {}

  // Assumption: at most one `INSERT`, valid priority queue operations.
  // Time complexity: O(n log n)
  bool distVal(hist_t& hist) {
    if (!LinBase<value_type>::preprocess(hist)) return false;

    std::vector<std::tuple<time_type, bool, oper_t>> events;
    for (const oper_t& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
    }
    std::sort(events.begin(), events.end());

    std::map<value_type, std::unordered_set<id_type>> runningOp;
    std::set<value_type> critVal;
    std::unordered_set<value_type> endedVal;

    for (const auto& [_, isInv, op] : events) {
      if (isInv) {
        if (op.method != Method::INSERT) runningOp[op.value].emplace(op.id);

        if (op.method == Method::POLL) {
          critVal.erase(op.value);
          endedVal.insert(op.value);
        }
      } else {
        if (runningOp[op.value].count(op.id)) return false;

        if (op.method == Method::INSERT && !endedVal.count(op.value))
          critVal.insert(op.value);
      }

      if (critVal.empty()) {
        runningOp.clear();
      } else {
        value_type maxPriorityVal = *critVal.rbegin();
        while (runningOp.size()) {
          auto& [val, ops] = *runningOp.rbegin();
          if (val < maxPriorityVal) break;

          runningOp.erase(val);
        }
      }
    }

    return true;
  }
};

}  // namespace polylin