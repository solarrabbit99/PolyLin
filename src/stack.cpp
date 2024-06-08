#include "stack.hpp"

#include <algorithm>
#include <queue>

#include "commons/interval_tree.hpp"
#include "commons/segment_tree.hpp"

using namespace polylin;

bool StackLin::extend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> pushPopDelta;
  for (const Operation& o : hist) {
    if (o.value == EMPTY_VALUE) continue;

    if (o.method == Method::PUSH)
      pushPopDelta[o.value]++;
    else if (o.method == Method::POP)
      pushPopDelta[o.value]--;
    maxTime = std::max(maxTime, o.endTime);
    maxProc = std::max(maxProc, o.proc);
  }
  for (const auto& [value, cnt] : pushPopDelta) {
    if (cnt < 0) return false;
    for (int i = 0; i < cnt; ++i)
      hist.emplace(++maxProc, Method::POP, value, maxTime + 1, maxTime + 2);
  }
  return true;
}

bool StackLin::tune(
    History& hist,
    std::unordered_map<value_type, interval>& critIntervalByVal) {
  std::vector<std::tuple<time_type, bool, Operation>> events;
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  hist.clear();
  std::sort(events.begin(), events.end());

  std::unordered_map<value_type, Operation> pushOp, popOp;
  std::unordered_map<value_type, std::queue<Operation>> peekOps;
  std::unordered_set<id_type> inPeekQueue;
  Operation emptyPush{0, Method::PUSH, EMPTY_VALUE, 0, 1};
  pushOp.emplace(EMPTY_VALUE, emptyPush);
  hist.emplace(emptyPush);
  time_type time = 1;
  for (const auto& [oldTime, isInv, o] : events) {
    if (isInv) {
      if (o.method == Method::PUSH) {
        // Check for ongoing PEEK operations
        std::queue<Operation> peekQueue2;
        std::queue<Operation>& peekQueue = peekOps[o.value];
        while (peekQueue.size()) {
          Operation peekOp2{peekQueue.front()};
          peekQueue.pop();
          peekOp2.startTime = ++time;
          peekQueue2.emplace(std::move(peekOp2));
        }
        peekOps[o.value] = std::move(peekQueue2);
        // Check for ongoing POP operations
        if (popOp.count(o.value)) {
          Operation popOp2{popOp.at(o.value)};
          popOp.erase(o.value);
          popOp2.startTime = ++time;
          popOp.emplace(o.value, std::move(popOp2));
        }
        // Start PUSH operation
        Operation pushOp2{o};
        pushOp2.startTime = ++time;
        pushOp.emplace(o.value, std::move(pushOp2));
      } else if (o.method == Method::PEEK) {
        // Check for ongoing POP
        if (popOp.count(o.value)) {
          Operation popOp2{popOp.at(o.value)};
          if (hist.count(popOp2)) return false;
          popOp.erase(o.value);
          popOp2.startTime = ++time;
          popOp.emplace(o.value, std::move(popOp2));
        }
        // Enqueue PEEK
        Operation peekOp2{o};
        peekOp2.startTime = ++time;
        inPeekQueue.emplace(peekOp2.id);
        peekOps[o.value].emplace(std::move(peekOp2));
      } else {
        // Start POP
        Operation popOp2{o};
        popOp2.startTime = ++time;
        popOp.emplace(o.value, std::move(popOp2));
      }
    } else {
      if (o.method == Method::PUSH) {
        // End PUSH operation
        Operation pushOp2{pushOp.at(o.value)};
        pushOp2.endTime = ++time;
        hist.emplace(std::move(pushOp2));
      } else if (o.method == Method::PEEK) {
        if (!inPeekQueue.count(o.id)) continue;
        // End running PUSH operation, if any
        if (!pushOp.count(o.value)) return false;
        Operation pushOp2{pushOp.at(o.value)};
        if (!hist.count(pushOp2)) {
          pushOp2.endTime = ++time;
          hist.emplace(std::move(pushOp2));
        }
        std::queue<Operation>& peekQueue = peekOps.at(o.value);
        // End PEEK operation
        while (peekQueue.front() != o) {
          inPeekQueue.erase(peekQueue.front().id);
          peekQueue.pop();
        }
        Operation peekOp2{peekQueue.front()};
        inPeekQueue.erase(peekQueue.front().id);
        peekQueue.pop();
        peekOp2.endTime = ++time;
        hist.emplace(std::move(peekOp2));
      } else {  // POP
        // End running PUSH operation, if any
        if (!pushOp.count(o.value)) return false;
        Operation pushOp2{pushOp.at(o.value)};
        if (!hist.count(pushOp2)) {
          pushOp2.endTime = ++time;
          hist.emplace(std::move(pushOp2));
        }
        // End running PEEK operations, if any
        std::queue<Operation>& peekQueue = peekOps[o.value];
        while (peekQueue.size()) {
          Operation peekOp2{peekQueue.front()};
          inPeekQueue.erase(peekQueue.front().id);
          peekQueue.pop();
          peekOp2.endTime = ++time;
          hist.emplace(std::move(peekOp2));
        }
        // End POP operation
        Operation popOp2{popOp.at(o.value)};
        popOp2.endTime = ++time;
        hist.emplace(std::move(popOp2));
      }
    }
  }
  hist.emplace(0, Method::POP, EMPTY_VALUE, time + 1, time + 2);

  // flush timings one more time
  events.clear();
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  std::sort(events.begin(), events.end());
  hist.clear();

  time = 0;
  std::unordered_map<value_type, std::queue<Operation>> opQueue;
  for (const auto& [oldTime, isInv, o] : events) {
    std::queue<Operation>& dq = opQueue[o.value];
    if (isInv) {
      Operation o2{o};
      // Add padding to values so that min value is 1
      o2.value += 2;
      o2.startTime = time;
      dq.emplace(o2);
      if (critIntervalByVal.count(o2.value))
        critIntervalByVal[o2.value].end = time;
    } else {
      Operation o2{dq.front()};
      dq.pop();
      o2.endTime = time;
      hist.emplace(o2);
      if (!critIntervalByVal.count(o2.value))
        critIntervalByVal[o2.value].start = time;
    }
    ++time;
  }

  return true;
}

bool StackLin::distVal(History& hist) {
  std::unordered_map<value_type, interval> critIntervalByVal;
  if (!extend(hist) || !tune(hist, critIntervalByVal)) return false;

  size_t n = hist.size();

  std::unordered_map<time_type, value_type> startTimeToVal;
  interval_tree operations;
  std::unordered_map<value_type, interval_tree> operationsByVal;
  segment_tree critIntervals{2 * n - 1};   // +1 per value
  segment_tree critIntervals2{2 * n - 1};  // +i per value i

  for (const auto& [value, intvl] : critIntervalByVal) {
    critIntervals.update_range(intvl.start, intvl.end - 1, 1);
    critIntervals2.update_range(intvl.start, intvl.end - 1, value);
  }
  for (const Operation& o : hist) {
    startTimeToVal[o.startTime] = o.value;
    operations.insert({o.startTime, o.endTime});
    operationsByVal[o.value].insert({o.startTime, o.endTime});
  }

  std::unordered_map<value_type, std::vector<time_type>> pointsByLastVal;
  std::unordered_set<value_type> clearedVals;

  // remove intervals overlapping [point, point + 1]
  auto removeOverlap = [&](interval_tree& intervals, const int& point) {
    for (const interval& intvl : intervals.query(point)) {
      if (intvl.end == point) continue;
      operations.remove(intvl);
      value_type val = startTimeToVal[intvl.start];
      operationsByVal[val].remove(intvl);
      if (operationsByVal[val].empty()) clearedVals.insert(val);
    }
  };

  while (!operations.empty()) {
    // find empty points, clear overlapping operations
    std::pair<int, int> pr = critIntervals.query_min(0, 2 * n - 2);
    while (pr.first == 0) {
      time_type pos = pr.second;
      removeOverlap(operations, pos);
      critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
      pr = critIntervals.query_min(0, 2 * n - 2);
    }

    // find single layer interval points and value, clear overlapping operations
    while (pr.first == 1) {
      time_type pos = pr.second;
      value_type val = critIntervals2.query_min(pos, pos).first;
      removeOverlap(operationsByVal[val], pos);
      critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
      pointsByLastVal[val].push_back(pos);
      pr = critIntervals.query_min(0, 2 * n - 2);
    }

    if (clearedVals.empty()) return false;

    // remove criticals intervals of cleared values
    // remove overlapping operations
    std::unordered_set<value_type> clearedVals2{std::move(clearedVals)};
    for (const value_type& val : clearedVals2) {
      auto& [b, e] = critIntervalByVal[val];
      critIntervals.update_range(b, e - 1, -1);
      critIntervals2.update_range(b, e - 1, -val);
      for (const time_type& t : pointsByLastVal[val])
        removeOverlap(operations, t);
    }
  }

  return true;
}
