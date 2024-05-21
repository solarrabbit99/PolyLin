#include "stack.hpp"

#include <algorithm>
#include <set>

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

  hist2.emplace(0, Method::PUSH, EMPTY_VALUE, MIN_TIME, MIN_TIME + 1);
  hist2.emplace(0, Method::POP, EMPTY_VALUE, MAX_TIME - 1, MAX_TIME);
  std::swap(hist, hist2);
  return true;
}

bool StackLin::distVal(History& hist) {
  if (!extend(hist) || !tune(hist)) return false;

  std::set<std::tuple<time_type, bool, Operation>> events;
  std::unordered_map<value_type, size_t> opByVal;
  for (const Operation& o : hist) {
    ++opByVal[o.value];
    events.emplace(o.startTime, true, o);
    events.emplace(o.endTime, false, o);
  }

  while (opByVal.size()) {
    std::unordered_map<value_type, size_t> freeOp;
    std::unordered_map<value_type, std::unordered_set<id_type>> runningOp;
    std::unordered_set<value_type> critVal;

    auto iter = events.begin();
    while (iter != events.end()) {
      const auto& [time, isInv, op] = *iter;

      if (!opByVal.count(op.value)) {
        iter = events.erase(iter);
        continue;
      }

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
      ++iter;
    }

    bool hasRem = false;
    for (auto& [value, cnt] : freeOp)
      if (cnt == opByVal[value]) {
        hasRem = true;
        opByVal.erase(value);
      }
    if (!hasRem) return false;
  }

  return true;
}
