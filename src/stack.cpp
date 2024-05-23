#include "stack.hpp"

#include <algorithm>
#include <queue>
#include <set>

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
  std::unordered_map<value_type, time_type> minResTime, maxInvTime;
  std::unordered_map<value_type, Operation> pushOp, popOp;
  History hist2;
  for (const Operation& o : hist) {
    if (o.value == EMPTY_VALUE) {
      Operation o2{o};
      o2.method = Method::PEEK;
      hist2.emplace(std::move(o2));
      continue;
    }

    if (!minResTime.count(o.value)) {
      minResTime[o.value] = o.endTime;
      maxInvTime[o.value] = o.startTime;
    } else {
      minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
      maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
    }

    if (o.method == Method::PUSH)
      pushOp.emplace(o.value, o);
    else if (o.method == Method::POP)
      popOp.emplace(o.value, o);
    else
      hist2.emplace(o);
  }

  for (const auto& [value, resTime] : minResTime) {
    if (!pushOp.count(value)) return false;
    Operation& valPushOp = pushOp.at(value);
    Operation& valPopOp = popOp.at(value);
    valPushOp.endTime = resTime;
    valPopOp.startTime = maxInvTime[value];

    if (valPopOp.startTime <= valPushOp.endTime) continue;

    if (valPushOp.startTime >= valPushOp.endTime ||
        valPopOp.startTime >= valPopOp.endTime)
      return false;

    hist2.emplace(valPushOp);
    hist2.emplace(valPopOp);
  }
  hist.clear();

  for (const Operation& o : hist2) {
    Operation o2{o};
    o2.startTime = ((o2.startTime + 2) << 2);
    o2.endTime = ((o2.endTime + 2) << 2);
    if (o.method == PEEK) {
      o2.startTime += 1;
      o2.endTime += 1;
    } else if (o.method == POP) {
      o2.startTime += 2;
      o2.endTime += 2;
    }
    hist.emplace(std::move(o2));
  }
  hist.emplace(0, Method::PUSH, EMPTY_VALUE, MIN_TIME, MIN_TIME + 1);
  hist.emplace(0, Method::POP, EMPTY_VALUE, MAX_TIME - 1, MAX_TIME);
  hist2.clear();

  std::vector<std::tuple<time_type, bool, Operation>> events;
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  std::sort(events.begin(), events.end());
  hist.clear();

  std::unordered_map<value_type, std::queue<Operation>> opQueue;
  time_type time = MIN_TIME;
  for (const auto& [oldTime, isInv, o] : events) {
    std::queue<Operation>& dq = opQueue[o.value];
    if (isInv) {
      Operation o2{o};
      o2.startTime = ++time;
      dq.emplace(o2);
    } else {
      while (dq.front() != o) dq.pop();
      Operation o2{dq.front()};
      dq.pop();
      o2.endTime = ++time;
      hist.emplace(o2);
    }
  }
  opQueue.clear();
  events.clear();

  // flush timings one more time
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  std::sort(events.begin(), events.end());
  hist.clear();

  time = 0;

  for (const auto& [oldTime, isInv, o] : events) {
    std::queue<Operation>& dq = opQueue[o.value];
    if (isInv) {
      Operation o2{o};
      // Add padding to values so that min value is 1
      o2.value += 2;
      o2.startTime = time;
      dq.emplace(o2);
      if (critIntervalByVal.count(o2.value))
        critIntervalByVal[o2.value].second = time;
    } else {
      Operation o2{dq.front()};
      dq.pop();
      o2.endTime = time;
      hist.emplace(o2);
      if (!critIntervalByVal.count(o2.value))
        critIntervalByVal[o2.value].first = time;
    }
    ++time;
  }

  return true;
}

bool StackLin::distVal(History& hist) {
  std::unordered_map<value_type, interval> critIntervalByVal;
  if (!extend(hist) || !tune(hist, critIntervalByVal)) return false;

  size_t n = hist.size();

  using namespace lib_interval_tree;
  std::unordered_map<time_type, value_type> timeToVal;
  interval_tree_t<time_type> operations;
  std::unordered_map<value_type, interval_tree_t<time_type>> operationsByVal;
  SegmentTree critIntervals{2 * n - 1};   // +1 per value
  SegmentTree critIntervals2{2 * n - 1};  // +i per value i

  for (const auto& [value, intvl] : critIntervalByVal) {
    critIntervals.update_range(intvl.first, intvl.second - 1, 1);
    critIntervals2.update_range(intvl.first, intvl.second - 1, value);
  }
  for (const Operation& o : hist) {
    timeToVal[o.startTime] = o.value;
    operations.insert({o.startTime, o.endTime - 1});
    operationsByVal[o.value].insert({o.startTime, o.endTime - 1});
  }

  std::unordered_map<value_type, std::vector<time_type>> lastPoints;
  std::unordered_set<value_type> clearedVals;

  auto removeInterval = [&](const interval& intvl) {
    operations.erase(operations.find({intvl.first, intvl.second}));
    value_type val = timeToVal[intvl.first];
    operationsByVal[val].erase(
        operationsByVal[val].find({intvl.first, intvl.second}));
    if (operationsByVal[val].empty()) clearedVals.insert(val);
  };

  while (!operations.empty()) {
    // find empty interval point clear operation on point handle satisfied
    // critical intervals
    std::pair<int, int> pr = critIntervals.query_min(0, 2 * n - 1);
    while (pr.first == 0) {
      time_type pos = pr.second;
      std::vector<interval> toRemove;
      operations.overlap_find_all({pos, pos + 1}, [&](auto iter) {
        toRemove.emplace_back(iter->interval().low(), iter->interval().high());
        return true;
      });
      for (const interval& intvl : toRemove) removeInterval(intvl);
      critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
      pr = critIntervals.query_min(0, 2 * n - 1);
    }

    // find single layer interval point and value clear operations on point
    // handle satisfied critical intervals
    while (pr.first == 1) {
      time_type pos = pr.second;
      value_type val = critIntervals2.query_min(pos, pos).first;
      std::vector<interval> toRemove;
      operationsByVal[val].overlap_find_all({pos, pos + 1}, [&](auto iter) {
        toRemove.emplace_back(iter->interval().low(), iter->interval().high());
        return true;
      });
      for (const interval& intvl : toRemove) removeInterval(intvl);
      critIntervals.update_range(pos, pos, 2 * n);  // 2*n is a sentinel value
      lastPoints[val].push_back(pos);

      pr = critIntervals.query_min(0, 2 * n - 1);
    }

    if (clearedVals.empty()) return false;

    // remove criticals intervals of cleared values
    std::unordered_set<value_type> clearedVals2;
    for (const value_type& val : clearedVals) {
      auto& [b, e] = critIntervalByVal[val];
      critIntervals.update_range(b, e - 1, -1);
      critIntervals2.update_range(b, e - 1, -val);
      std::vector<interval> toRemove;
      for (const time_type& t : lastPoints[val]) {
        operations.overlap_find_all({t, t + 1}, [&](auto iter) {
          toRemove.emplace_back(iter->interval().low(),
                                iter->interval().high());
          return true;
        });
        for (const interval& intvl : toRemove) removeInterval(intvl);
      }
    }
    clearedVals = std::move(clearedVals2);
  }

  return true;
}
