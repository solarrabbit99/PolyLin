#pragma once

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>

#include "base.hpp"
#include "commons/interval_tree.hpp"
#include "commons/segment_tree.hpp"

namespace polylin {

template <typename value_type>
class StackLin : public LinBase<value_type> {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  StackLin() : LinBase<value_type>{{PUSH}, {POP}} {}

  // Assumption: at most one `PUSH`, valid stack operations.
  // Time complexity: O(n log n)
  bool distVal(hist_t& hist) {
    std::unordered_map<value_type, interval> critIntervalByVal;
    if (!LinBase<value_type>::preprocess(hist)) return false;

    flush(hist, critIntervalByVal);
    if (hist.empty()) return true;
    size_t n = hist.size();

    std::unordered_map<time_type, value_type> startTimeToVal;
    interval_tree ops;
    std::unordered_map<value_type, interval_tree> opByVal;
    segment_tree critIntervals{2 * n - 1};   // +1 per value
    segment_tree critIntervals2{2 * n - 1};  // +i per value i

    for (const auto& [value, intvl] : critIntervalByVal) {
      critIntervals.update_range(intvl.start, intvl.end - 1, 1);
      critIntervals2.update_range(intvl.start, intvl.end - 1, value);
    }
    for (const oper_t& o : hist) {
      startTimeToVal[o.startTime] = o.value;
      ops.insert({o.startTime, o.endTime});
      opByVal[o.value].insert({o.startTime, o.endTime});
    }

    std::unordered_map<value_type, std::vector<time_type>> pointsByLastVal;
    std::unordered_set<value_type> clearedVals;

    // remove intervals overlapping [point, point + 1]
    auto removeOverlap = [&](interval_tree& intervals, const int& point) {
      for (const interval& intvl : intervals.query(point)) {
        if (intvl.end == point) continue;
        ops.remove(intvl);
        value_type val = startTimeToVal[intvl.start];
        opByVal[val].remove(intvl);
        if (opByVal[val].empty()) clearedVals.insert(val);
      }
    };

    while (!ops.empty()) {
      // find empty points, clear overlapping oper_ts
      std::pair<int, int> pr = critIntervals.query_min();
      while (pr.first == 0) {
        time_type pos = pr.second;
        removeOverlap(ops, pos);
        critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
        pr = critIntervals.query_min();
      }

      // find single layer interval points and value, clear overlapping oper_ts
      while (pr.first == 1) {
        time_type pos = pr.second;
        value_type val = critIntervals2.query_val(pos);
        removeOverlap(opByVal[val], pos);
        critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
        pointsByLastVal[val].push_back(pos);
        pr = critIntervals.query_min();
      }

      if (clearedVals.empty()) return false;

      // remove criticals intervals of cleared values
      // remove overlapping oper_ts
      std::unordered_set<value_type> clearedVals2{std::move(clearedVals)};
      for (const value_type& val : clearedVals2) {
        auto& [b, e] = critIntervalByVal[val];
        critIntervals.update_range(b, e - 1, -1);
        critIntervals2.update_range(b, e - 1, -val);
        for (const time_type& t : pointsByLastVal[val]) removeOverlap(ops, t);
      }
    }

    return true;
  }

 private:
  // flush timings one more time and remove concurrent push and pop
  void flush(hist_t& hist,
             std::unordered_map<value_type, interval>& critIntervalByVal) {
    std::vector<std::tuple<time_type, bool, oper_t>> events;
    for (const oper_t& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
    }
    std::sort(events.begin(), events.end());
    hist.clear();

    std::unordered_set<value_type> removableVals;
    std::queue<oper_t> opQueue;
    for (const auto& [_, isInv, oldOp] : events) {
      oper_t op{oldOp};
      // Add padding to values so that min value is 1
      ++op.value;
      if (isInv) {
        if (op.method == POP) removableVals.erase(oldOp.value);
        opQueue.emplace(op);
      } else {
        if (op.method == PUSH) removableVals.insert(oldOp.value);
        opQueue.emplace(op);
      }
    }

    time_type time = MIN_TIME;
    std::unordered_set<oper_t> ongoingOps;
    while (!opQueue.empty()) {
      if (removableVals.count(opQueue.front().value)) {
        opQueue.pop();
        continue;
      }

      if (!ongoingOps.count(opQueue.front())) {
        oper_t op{opQueue.front()};
        op.startTime = time;
        if (op.method == POP) critIntervalByVal[op.value].end = time;
        ongoingOps.emplace(op);
      } else {
        oper_t op{*ongoingOps.find(opQueue.front())};
        op.endTime = time;
        if (op.method == PUSH) critIntervalByVal[op.value].start = time;
        hist.emplace(op);
      }

      opQueue.pop();
      ++time;
    }
  }
};

}  // namespace polylin