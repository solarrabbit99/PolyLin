#pragma once

#include "definitions.hpp"

namespace polylin {

class PriorityQueueLin {
 public:
  // Assumption: at most one `INSERT`, valid priority queue operations.
  // Time complexity: O(n log n)
  bool distVal(History& hist);

 private:
  // Returns `false` if there is value where `POLL` methods > `INSERT` methods
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `POLL` operation)
  bool tune(History& hist);
};

}  // namespace polylin