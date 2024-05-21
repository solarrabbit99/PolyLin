#include "stack.hpp"

#include <algorithm>

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

bool StackLin::tune(History& hist) {
  std::unordered_map<value_type, std::vector<Operation>> opByVal;
  for (const Operation& o : hist) opByVal[o.value].emplace_back(o);

  std::unordered_map<value_type, time_type> minResTime, maxInvTime;
  std::unordered_map<value_type, Operation> pushOp, popOp;
  for (const Operation& o : hist) {
    if (o.value == EMPTY_VALUE) continue;

    if (!minResTime.count(o.value)) {
      minResTime[o.value] = o.endTime;
      maxInvTime[o.value] = o.startTime;
    } else {
      minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
      maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
    }

    if (o.method == Method::PUSH) pushOp.emplace(o.value, o);
    if (o.method == Method::POP) popOp.emplace(o.value, o);
  }
  for (const auto& [value, resTime] : minResTime) {
    if (!pushOp.count(value)) return false;
    Operation& valPushOp = pushOp.at(value);
    Operation& valPopOp = popOp.at(value);
    valPushOp.endTime = resTime;
    valPopOp.startTime = maxInvTime[value];

    if (valPopOp.startTime <= valPushOp.endTime) {
      for (const Operation& o : opByVal[value]) hist.erase(o);
      continue;
    }

    if (valPushOp.startTime >= valPushOp.endTime ||
        valPopOp.startTime >= valPopOp.endTime)
      return false;

    hist.erase(valPushOp);
    hist.erase(valPopOp);
    hist.emplace(valPushOp);
    hist.emplace(valPopOp);
  }
  return true;
}

bool StackLin::removeEmptyOps(History& hist) {
  std::vector<std::tuple<time_type, bool, Operation>> events;
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  std::sort(events.begin(), events.end());

  std::unordered_set<id_type> runningEmptyOp;
  std::unordered_set<value_type> critVal, endedVal;

  for (const auto& [time, isInv, op] : events) {
    if (op.value != EMPTY_VALUE) {
      if (isInv) {
        if (op.method == Method::POP) {
          critVal.erase(op.value);
          endedVal.insert(op.value);
        }
      } else {
        if (op.method == Method::PUSH && !endedVal.count(op.value))
          critVal.insert(op.value);
      }
    } else {
      if (isInv)
        runningEmptyOp.insert(op.id);
      else if (runningEmptyOp.count(op.id))
        return false;
      else
        hist.erase(op);
    }

    if (critVal.empty()) runningEmptyOp.clear();
  }

  return true;
}

bool StackLin::distVal(History& hist) {
  if (!extend(hist) || !tune(hist) || !removeEmptyOps(hist)) return false;

  std::unordered_set<value_type> removedVals;
  std::vector<std::tuple<time_type, bool, Operation>> events;
  for (const Operation& o : hist) {
    events.emplace_back(o.startTime, true, o);
    events.emplace_back(o.endTime, false, o);
  }
  std::sort(events.begin(), events.end());

  std::unordered_map<value_type, size_t> opByVal;
  for (const Operation& o : hist) ++opByVal[o.value];

  while (removedVals.size() < opByVal.size()) {
    std::unordered_map<value_type, size_t> freeOp;
    std::unordered_map<value_type, std::unordered_set<id_type>> runningOp;
    std::unordered_set<value_type> critVal;

    for (const auto& [time, isInv, op] : events) {
      if (removedVals.count(op.value)) continue;

      if (isInv) {
        runningOp[op.value].emplace(op.id);
        if (op.method == Method::POP) critVal.erase(op.value);
      } else {
        runningOp[op.value].erase(op.id);
        if (op.method == Method::PUSH) critVal.insert(op.value);
      }

      if (critVal.empty()) {
        for (auto& [value, set] : runningOp) freeOp[value] += set.size();
        runningOp.clear();
      }
      if (critVal.size() == 1) {
        value_type value = *critVal.begin();
        freeOp[value] += runningOp[value].size();
        runningOp.erase(value);
      }
    }

    bool hasRem = false;
    for (auto& [value, cnt] : freeOp)
      if (cnt == opByVal[value]) {
        hasRem = true;
        removedVals.insert(value);
      }
    if (!hasRem) return false;
  }

  return true;
}
