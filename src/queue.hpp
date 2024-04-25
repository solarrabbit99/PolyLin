#pragma once

#include "definitions.hpp"

namespace polylin {

class QueueLin {
 public:
  // Assumption: at most one `ENQ`, valid queue operations.
  // Time complexity: O(n log n)
  bool distVal(History& hist);

 private:
  // Returns `false` if there is value where `DEQ` methods > `ENQ` methods
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `ENQ` operation)
  bool tune(History& hist);
};

}  // namespace polylin