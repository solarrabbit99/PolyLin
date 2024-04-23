#pragma once
#include <unordered_map>
#include <vector>

#include "definitions.hpp"

// Returns `false` if there is value where `POP` methods > `PUSH` methods
bool stackExtend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> pushPopDelta;
  for (const Operation& o : hist) {
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

// For distinct value restriction, return `false` if impossible to tune (e.g.
// value has no `PUSH` operation)
bool stackTune(History& hist) {
  std::unordered_map<value_type, time_type> minResTime, maxInvTime;
  std::unordered_map<value_type, Operation> pushOp, popOp;
  for (const Operation& o : hist) {
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

// Assumption: at most one `PUSH`, valid stack operations.
// Time complexity: O(n^2)
bool stackDistValLin(History& hist) {
  if (!stackExtend(hist) || !stackTune(hist)) return false;
  while (hist.size()) {
    std::vector<std::tuple<time_type, bool, Operation>> events;
    std::unordered_map<value_type, std::vector<Operation>> opByVal;
    for (const Operation& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
      opByVal[o.value].emplace_back(o);
    }
    std::sort(events.begin(), events.end());
    std::unordered_map<value_type, size_t> freeOp;
    std::unordered_map<value_type, std::unordered_set<id_type>> runningOp;
    std::unordered_set<value_type> critVal;
    for (const auto& [time, isInv, op] : events) {
      if (isInv) {
        runningOp[op.value].emplace(op.id);
        if (op.method == Method::POP) critVal.erase(op.value);
      } else {
        runningOp[op.value].erase(op.id);
        if (op.method == Method::PUSH) critVal.insert(op.value);
      }
      if (critVal.empty()) {
        for (auto& [value, set] : runningOp) {
          freeOp[value] += set.size();
          set.clear();
        }
      }
      if (critVal.size() == 1) {
        value_type value = *critVal.begin();
        freeOp[value] += runningOp[value].size();
        runningOp[value].clear();
      }
    }
    bool hasRem = false;
    for (auto& [value, cnt] : freeOp)
      if (cnt == opByVal[value].size()) {
        hasRem = true;
        for (auto& op : opByVal[value]) hist.erase(op);
      }
    if (!hasRem) return false;
  }
  return true;
}