#pragma once

#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "base.hpp"

namespace polylin {

template <typename value_type>
class QueueLin : public LinBase<value_type> {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  QueueLin() : LinBase<value_type>{{ENQ}, {DEQ}} {}

  // Assumption: at most one `ENQ`, valid queue operations.
  // Time complexity: O(n log n)
  bool distVal(hist_t& hist) {
    if (!LinBase<value_type>::preprocess(hist)) return false;

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
};

}  // namespace polylin