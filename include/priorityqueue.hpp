#pragma once

#include <map>
#include <set>

#include "definitions.hpp"

namespace polylin {

template <typename value_type>
class PriorityQueueLin {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  // Assumption: at most one `INSERT`, valid priority queue operations.
  // Time complexity: O(n log n)
  bool distVal(hist_t& hist) {
    if (!extend(hist) || !tune(hist)) return false;

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

 private:
  // Returns `false` if there is value where `POLL` methods > `INSERT` methods
  bool extend(hist_t& hist) {
    time_type maxTime = MIN_TIME;
    std::unordered_map<value_type, int> insertPollDelta;
    for (const oper_t& o : hist) {
      if (o.method == Method::INSERT)
        insertPollDelta[o.value]++;
      else if (o.method == Method::POLL)
        insertPollDelta[o.value]--;
      maxTime = std::max(maxTime, o.endTime);
    }
    for (const auto& [value, cnt] : insertPollDelta) {
      if (cnt < 0) return false;
      for (int i = 0; i < cnt; ++i)
        hist.emplace(Method::POLL, value, maxTime + 1, maxTime + 2);
    }
    return true;
  }

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `POLL` operation)
  bool tune(hist_t& hist) {
    std::unordered_map<value_type, time_type> minResTime, maxInvTime;
    std::unordered_map<value_type, oper_t> insertOp, pollOp;
    for (const oper_t& o : hist) {
      if (!minResTime.count(o.value)) {
        minResTime[o.value] = o.endTime;
        maxInvTime[o.value] = o.startTime;
      } else {
        minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
        maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
      }

      if (o.method == Method::INSERT) insertOp.emplace(o.value, o);
      if (o.method == Method::POLL) pollOp.emplace(o.value, o);
    }
    hist_t updated;
    for (const oper_t& o : hist) {
      oper_t dup{o};
      switch (o.method) {
        case INSERT:
          dup.endTime = minResTime[o.value];
          if (dup.startTime >= dup.endTime) return false;
          break;
        case PEEK:
          if (!insertOp.count(o.value)) return false;
          dup.startTime =
              std::max(dup.startTime, insertOp.at(o.value).startTime);
          break;
        case POLL:
          dup.startTime = maxInvTime[o.value];
          if (dup.startTime >= dup.endTime) return false;
          break;
      }
      updated.insert(std::move(dup));
    }

    std::swap(hist, updated);
    return true;
  }
};

}  // namespace polylin