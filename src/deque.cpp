#include "deque.hpp"

#include <unordered_map>

using namespace polylin;

bool DequeLin::extend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> pushPopDelta;
  for (const Operation& o : hist) {
    if (isPush(o.method))
      pushPopDelta[o.value]++;
    else if (isPop(o.method))
      pushPopDelta[o.value]--;
    maxTime = std::max(maxTime, o.endTime);
    maxProc = std::max(maxProc, o.proc);
  }
  for (const auto& [value, cnt] : pushPopDelta) {
    if (cnt < 0) return false;
    for (int i = 0; i < cnt; ++i)
      hist.emplace(++maxProc, Method::POP_FRONT, value, maxTime + 1,
                   maxTime + 2);
  }
  return true;
}

bool DequeLin::tune(History& hist) {
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

    if (isPush(o.method)) pushOp.emplace(o.value, o);
    if (isPop(o.method)) popOp.emplace(o.value, o);
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

bool DequeLin::isFrontMethod(Method method) {
  return method == Method::PUSH_FRONT || method == Method::PEEK_FRONT ||
         method == Method::POP_FRONT;
}

bool DequeLin::isPush(Method method) {
  return method == Method::PUSH_FRONT || method == Method::PUSH_BACK;
}

bool DequeLin::isPeek(Method method) {
  return method == Method::PEEK_FRONT || method == Method::PEEK_BACK;
}

bool DequeLin::isPop(Method method) {
  return method == Method::POP_FRONT || method == Method::POP_BACK;
}