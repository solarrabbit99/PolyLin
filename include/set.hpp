#pragma once

#include "base.hpp"

namespace polylin {

template <typename value_type>
class SetLin : LinBase<value_type> {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  SetLin() : LinBase<value_type>{{INSERT}, {REMOVE}} {}

  // Assumption: at most one `INSERT`, valid priority queue operations.
  // Time complexity: O(n)
  bool distVal(hist_t& hist) {
    hist_t hist2;
    for (oper_t o : hist) {
      if (o.method == INSERT && o.retVal == false) {
        o.method = CONTAINS;
        o.retVal = true;
      } else if (o.method == REMOVE && o.retVal == false) {
        o.method = CONTAINS;
      }
      hist2.emplace(std::move(o));
    }

    if (!LinBase<value_type>::extend(hist2)) return false;

    std::unordered_map<value_type, time_type> minRes, maxInv;
    for (const oper_t& o : hist2) {
      if (o.retVal) {
        minRes[o.value] = minRes.count(o.value)
                              ? std::min(minRes[o.value], o.endTime)
                              : o.endTime;
        maxInv[o.value] = maxInv.count(o.value)
                              ? std::max(maxInv[o.value], o.startTime)
                              : o.startTime;
      }
    }

    for (const oper_t& o : hist2) {
      if (o.retVal) {
        if (o.method == INSERT && o.startTime > minRes[o.value]) return false;
        if (o.method == REMOVE && o.endTime < maxInv[o.value]) return false;
      } else {
        if (minRes[o.value] < o.startTime && o.endTime < maxInv[o.value])
          return false;
      }
    }

    return true;
  }
};

}  // namespace polylin