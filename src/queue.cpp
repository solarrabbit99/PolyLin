#include "queue.hpp"

#include <iostream>

using namespace polylin;

bool QueueLin::extend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> enqDeqDelta;
  for (const Operation& o : hist) {
    if (o.method == Method::ENQ)
      enqDeqDelta[o.value]++;
    else if (o.method == Method::DEQ)
      enqDeqDelta[o.value]--;
    maxTime = std::max(maxTime, o.endTime);
    maxProc = std::max(maxProc, o.proc);
  }
  for (const auto& [value, cnt] : enqDeqDelta) {
    if (cnt < 0) return false;
    for (int i = 0; i < cnt; ++i)
      hist.emplace(++maxProc, Method::DEQ, value, maxTime + 1, maxTime + 2);
  }
  return true;
}

bool QueueLin::tune(History& hist) {
  std::unordered_map<value_type, time_type> minResTime, maxInvTime;
  std::unordered_map<value_type, Operation> enqOp, deqOp;
  for (const Operation& o : hist) {
    if (!minResTime.count(o.value)) {
      minResTime[o.value] = o.endTime;
      maxInvTime[o.value] = o.startTime;
    } else {
      minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
      maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
    }

    if (o.method == Method::ENQ) enqOp.emplace(o.value, o);
    if (o.method == Method::DEQ) deqOp.emplace(o.value, o);
  }
  for (const auto& [value, resTime] : minResTime) {
    if (!enqOp.count(value)) return false;
    Operation& valEnqOp = enqOp.at(value);
    Operation& valDeqOp = deqOp.at(value);
    valEnqOp.endTime = resTime;
    valDeqOp.startTime = maxInvTime[value];

    if (valEnqOp.startTime >= valEnqOp.endTime ||
        valDeqOp.startTime >= valDeqOp.endTime)
      return false;

    hist.erase(valEnqOp);
    hist.erase(valDeqOp);
    hist.emplace(valEnqOp);
    hist.emplace(valDeqOp);
  }
  return true;
}

bool QueueLin::distVal(History& hist) {
  if (!extend(hist)) return false;

  for (const Operation& o : hist) opByVal[o.value].emplace_back(o);

  if (!tune(hist)) return false;

  while (hist.size()) {
    std::vector<std::tuple<time_type, bool, Operation>> events;
    for (const Operation& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
    }
    std::sort(events.begin(), events.end());
    std::unordered_set<value_type> runningEnqOp, endedEnqOp;
    std::unordered_map<value_type, size_t> runningOtherOp, endedOtherOp;
    for (const auto& [time, isInv, op] : events) {
      if (isInv) {
        if (op.method == Method::ENQ) {
          runningEnqOp.insert(op.value);
          endedEnqOp.clear();
        } else {
          runningOtherOp[op.value]++;
          auto iter = endedOtherOp.begin();
          auto endIter = endedOtherOp.end();
          while (iter != endIter) {
            if (iter->first != op.value)
              iter = endedOtherOp.erase(iter);
            else
              ++iter;
          }
        }
      } else {
        if (op.method == Method::ENQ) {
          runningEnqOp.erase(op.value);
          endedEnqOp.insert(op.value);
        } else {
          runningOtherOp[op.value]--;
          endedOtherOp[op.value]++;
        }
      }
    }
    bool hasRem = false;
    for (auto& [value, cnt] : endedOtherOp)
      if (cnt + endedEnqOp.count(value) == opByVal[value].size()) {
        hasRem = true;
        for (const Operation& op : opByVal[value]) hist.erase(op);
      }
    if (!hasRem) return false;
  }
  return true;
}