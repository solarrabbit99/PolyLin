#pragma once

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "definitions.hpp"

namespace polylin {

template <typename value_type>
class QueueLin {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  // Assumption: at most one `ENQ`, valid queue operations.
  // Time complexity: O(n log n)
  bool distVal(hist_t& hist) {
    if (!extend(hist) || !tune(hist) || !removeEmptyOps(hist)) return false;

    std::unordered_map<value_type, size_t> opByVal;
    std::vector<std::tuple<time_type, bool, oper_t>> enqEvents, otherEvents;
    for (const oper_t& o : hist) {
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
          if (!confirmedVals.count(*lastInvVal) &&
              !confirmedVals.count(op.value))
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

 private:
  // Returns `false` if there is value where `DEQ` methods > `ENQ` methods
  bool extend(hist_t& hist) {
    time_type maxTime = MIN_TIME;
    std::unordered_map<value_type, int> enqDeqDelta;
    for (const oper_t& o : hist) {
      if (o.value == EMPTY_VALUE) continue;

      if (o.method == Method::ENQ)
        enqDeqDelta[o.value]++;
      else if (o.method == Method::DEQ)
        enqDeqDelta[o.value]--;
      maxTime = std::max(maxTime, o.endTime);
    }
    for (const auto& [value, cnt] : enqDeqDelta) {
      if (cnt < 0) return false;
      for (int i = 0; i < cnt; ++i)
        hist.emplace(Method::DEQ, value, maxTime + 1, maxTime + 2);
    }
    return true;
  }

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `ENQ` operation)
  bool tune(hist_t& hist) {
    std::unordered_map<value_type, time_type> minResTime, maxInvTime;
    std::unordered_map<value_type, oper_t> enqOp, deqOp;
    for (const oper_t& o : hist) {
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
      oper_t& valEnqOp = enqOp.at(value);
      oper_t& valDeqOp = deqOp.at(value);
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

  // Remove empty peek and deq operations
  bool removeEmptyOps(hist_t& hist) {
    std::vector<std::tuple<time_type, bool, oper_t>> events;
    for (const oper_t& o : hist) {
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
};

}  // namespace polylin