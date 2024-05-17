#include "queue.hpp"

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

using namespace polylin;

bool QueueLin::extend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> enqDeqDelta;
  for (const Operation& o : hist) {
    if (o.value == EMPTY_VALUE) continue;

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
    if (o.value == EMPTY_VALUE) continue;

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

bool QueueLin::removeEmptyOps(History& hist) {
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
        if (op.method == Method::DEQ) {
          critVal.erase(op.value);
          endedVal.insert(op.value);
        }
      } else {
        if (op.method == Method::ENQ && !endedVal.count(op.value))
          critVal.insert(op.value);
      }
    }

    if (critVal.empty()) runningEmptyOp.clear();

    if (op.value == EMPTY_VALUE) {
      if (isInv)
        runningEmptyOp.insert(op.id);
      else if (runningEmptyOp.count(op.id))
        return false;
      else
        hist.erase(op);
    }
  }

  return true;
}

bool QueueLin::distVal(History& hist) {
  if (!extend(hist) || !tune(hist) || !removeEmptyOps(hist)) return false;

  std::unordered_map<value_type, size_t> opByVal;
  std::vector<std::tuple<time_type, bool, Operation>> enqEvents, otherEvents;
  for (const Operation& o : hist) {
    ++opByVal[o.value];
    if (o.method == Method::ENQ) {
      enqEvents.emplace_back(o.startTime, true, o);
      enqEvents.emplace_back(o.endTime, false, o);
    } else {
      otherEvents.emplace_back(o.startTime, true, o);
      otherEvents.emplace_back(o.endTime, false, o);
    }
  }
  std::sort(enqEvents.begin(), enqEvents.end());
  std::sort(otherEvents.begin(), otherEvents.end());

  std::optional<value_type> lastInvVal;
  std::unordered_set<value_type> pendingVals, confirmedVals;
  std::unordered_map<value_type, size_t> runningOtherOp;

  auto enqIter = enqEvents.rbegin();
  std::function<void()> scanEnqEvents = [&]() {
    while (enqIter != enqEvents.rend()) {
      const auto& [_, isInv, op] = *enqIter;
      if (confirmedVals.count(op.value)) {
        ++enqIter;
        continue;
      }

      if (isInv) break;

      if (pendingVals.count(op.value)) {
        pendingVals.erase(op.value);
        confirmedVals.insert(op.value);
      } else {
        pendingVals.insert(op.value);
      }

      ++enqIter;
    }
  };

  for (auto iter = otherEvents.rbegin(); iter != otherEvents.rend(); ++iter) {
    const auto& [_, isInv, op] = *iter;
    if (confirmedVals.count(op.value)) continue;

    if (isInv) {
      if (lastInvVal && lastInvVal != op.value) {
        scanEnqEvents();
        if (!confirmedVals.count(*lastInvVal) && !confirmedVals.count(op.value))
          return false;

        if (!confirmedVals.count(op.value)) {
          if (runningOtherOp[op.value] + 1 == opByVal[op.value]) return false;

          lastInvVal = op.value;
        } else if (confirmedVals.count(*lastInvVal))
          lastInvVal.reset();
      } else {
        lastInvVal = op.value;

        if (runningOtherOp[op.value] + 1 == opByVal[op.value]) {
          scanEnqEvents();
          if (!confirmedVals.count(op.value)) return false;

          lastInvVal.reset();
        }
      }
    } else {
      ++runningOtherOp[op.value];
      if (runningOtherOp[op.value] + 1 == opByVal[op.value]) {
        if (pendingVals.count(op.value)) {
          pendingVals.erase(op.value);
          confirmedVals.insert(op.value);
        } else {
          pendingVals.insert(op.value);
        }
      }
    }
  }

  return true;
}