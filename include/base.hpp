#pragma once

#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

#include "commons/queue_set.hpp"
#include "definitions.hpp"

namespace polylin {

template <typename value_type>
class LinBase {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  virtual bool distVal(hist_t& hist) = 0;

  hist_t getSubHist(const hist_t& hist, time_type time) {
    hist_t subhist;
    std::unordered_set<value_type> removeOps, allOps;
    for (const auto& o : hist)
      if (o.endTime <= time) {
        subhist.emplace(o);
        if (o.value != EMPTY_VALUE) {
          allOps.insert(o.value);
          if (removes.count(o.method)) removeOps.insert(o.value);
        }
      }

    for (auto o : hist)
      if (o.startTime < time && time < o.endTime && allOps.count(o.value)) {
        o.endTime = time + 3;
        if (removes.count(o.method)) removeOps.insert(o.value);
        subhist.emplace(std::move(o));
      }

    for (const value_type& value : allOps) {
      if (!removeOps.count(value))
        subhist.emplace(*removes.begin(), value, time + 1, time + 2, true);
    }
    return subhist;
  }

 protected:
  LinBase(const std::vector<Method>&& adds, const std::vector<Method>&& removes)
      : adds(adds.begin(), adds.end()),
        removes(removes.begin(), removes.end()) {}

  bool preprocess(hist_t& hist) const {
    return extend(hist) && tune(hist) && removeEmpty(hist);
  }

  // Extend chooses the first remove method as default remove method, O(n)
  bool extend(hist_t& hist) const {
    time_type maxTime = MIN_TIME;
    std::unordered_set<value_type> addOps, removeOps, allOps;
    for (const oper_t& o : hist) {
      if (o.value == EMPTY_VALUE || o.retVal == false) continue;

      allOps.insert(o.value);
      if (adds.count(o.method) && !addOps.insert(o.value).second)
        return false;
      else if (removes.count(o.method) && !removeOps.insert(o.value).second)
        return false;
      maxTime = std::max(maxTime, o.endTime);
    }
    for (const value_type& value : allOps) {
      if (!addOps.count(value)) return false;
      if (!removeOps.count(value))
        hist.emplace(*removes.begin(), value, maxTime + 1, maxTime + 2, true);
    }
    return true;
  };

  // All timings in simplified history are distinct
  bool tune(hist_t& hist) const {
    // Sort events
    std::vector<std::tuple<time_type, bool, oper_t>> events;
    for (const oper_t& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
    }
    hist.clear();
    std::sort(events.begin(), events.end());

    std::unordered_map<value_type, oper_t> addOps, rmOps;
    std::unordered_map<value_type, queue_set<oper_t>> otherOps;

    time_type time = MIN_TIME;
    for (const auto& [_, isInv, o] : events) {
      if (isInv) {
        // Skip checks for empty values
        if (o.value == EMPTY_VALUE) {
          oper_t op{o};
          op.startTime = ++time;
          otherOps[o.value].enqueue(std::move(op));
          continue;
        }

        if (adds.count(o.method)) {
          // Start add operation
          oper_t addOp{o};
          addOp.startTime = ++time;
          addOps.emplace(o.value, std::move(addOp));
          // Increment ongoing other operations start time
          queue_set<oper_t>& oldQueue = otherOps[o.value];
          queue_set<oper_t> newQueue;
          while (!oldQueue.empty()) {
            oper_t op{oldQueue.dequeue()};
            op.startTime = ++time;
            newQueue.enqueue(std::move(op));
          }
          std::swap(oldQueue, newQueue);
          // Increment ongoing remove operation start time
          if (rmOps.count(o.value)) {
            oper_t rmOp{rmOps.at(o.value)};
            rmOp.startTime = ++time;
            rmOps.erase(o.value);
            rmOps.emplace(o.value, std::move(rmOp));
          }
        } else if (removes.count(o.method)) {
          // Start remove operation
          oper_t rmOp{o};
          rmOp.startTime = ++time;
          rmOps.emplace(o.value, std::move(rmOp));
        } else {
          // Enqueue operation
          oper_t op{o};
          op.startTime = ++time;
          otherOps[o.value].enqueue(std::move(op));
          // Increment ongoing remove operation start time
          if (rmOps.count(o.value)) {
            oper_t rmOp{rmOps.at(o.value)};
            // Remove operation responded
            if (hist.count(rmOp)) return false;
            rmOp.startTime = ++time;
            rmOps.erase(o.value);
            rmOps.emplace(o.value, std::move(rmOp));
          }
        }
      } else {
        // Skip checks for empty values
        if (o.value == EMPTY_VALUE) {
          oper_t op{otherOps[o.value].remove(o)};
          op.endTime = ++time;
          hist.emplace(std::move(op));
          continue;
        }

        if (adds.count(o.method)) {
          // End add operation
          oper_t addOp{addOps.at(o.value)};
          addOp.endTime = ++time;
          hist.emplace(std::move(addOp));
        } else if (removes.count(o.method)) {
          // Add operation not yet invoked
          if (!addOps.count(o.value)) return false;
          // End any ongoing add operation
          oper_t addOp{addOps.at(o.value)};
          if (!hist.count(addOp)) {
            addOp.endTime = ++time;
            hist.emplace(std::move(addOp));
          }
          // End any running other operations
          queue_set<oper_t>& opQueue = otherOps[o.value];
          while (!opQueue.empty()) {
            oper_t op{opQueue.dequeue()};
            op.endTime = ++time;
            hist.emplace(std::move(op));
          }
          // End remove operation
          oper_t rmOp{rmOps.at(o.value)};
          rmOp.endTime = ++time;
          hist.emplace(std::move(rmOp));
        } else {
          // Add operation not yet invoked
          if (!addOps.count(o.value)) return false;
          // End any ongoing add operation
          oper_t addOp{addOps.at(o.value)};
          if (!hist.count(addOp)) {
            addOp.endTime = ++time;
            hist.emplace(std::move(addOp));
          }
          // End operation
          if (otherOps[o.value].contains(o)) {
            oper_t op{otherOps[o.value].remove(o)};
            op.endTime = ++time;
            hist.emplace(std::move(op));
          }
        }
      }
    }
    return true;
  }

  // Only works on histories simplifed via `tune(hist)`
  // Note that empty operations can be of any method,
  // the only requirement being `value == EMPTY_VALUE`
  bool removeEmpty(hist_t& hist) const {
    std::vector<std::tuple<time_type, bool, oper_t>> events;
    for (const oper_t& o : hist) {
      events.emplace_back(o.startTime, true, o);
      events.emplace_back(o.endTime, false, o);
    }
    std::sort(events.begin(), events.end());

    std::unordered_set<id_type> runningEmptyOp;
    std::unordered_set<value_type> critVal, endedVal;

    for (const auto& [_, isInv, op] : events) {
      if (op.value != EMPTY_VALUE) {
        if (isInv) {
          if (removes.count(op.method)) {
            critVal.erase(op.value);
            endedVal.insert(op.value);
          }
        } else {
          if (adds.count(op.method) && !endedVal.count(op.value))
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
  };

 private:
  const std::unordered_set<Method> adds;
  const std::unordered_set<Method> removes;
};

}  // namespace polylin