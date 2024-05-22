#pragma once
#include <set>
#include <unordered_map>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class StackLin {
 public:
  // Assumption: at most one `PUSH`, valid stack operations.
  // Time complexity: O(n^2)
  bool distVal(History& hist);

 private:
  // Add `POP` methods. Returns `false` if there is value where `POP` methods >
  // `PUSH` methods
  bool extend(History& hist);

  typedef std::pair<time_type, time_type> interval;
  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `PUSH` operation)
  bool tune(History& hist,
            std::unordered_map<value_type, interval>& critIntervalByVal);
};

}  // namespace polylin