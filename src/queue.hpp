#pragma once
#include <unordered_map>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class QueueLin {
 public:
  // Assumption: at most one `ENQ`, valid queue operations.
  // Time complexity: O(n^2)
  bool distVal(History& hist);

 private:
  // Returns `false` if there is value where `DEQ` methods > `ENQ` methods
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `ENQ` operation)
  bool tune(History& hist);

  std::unordered_map<value_type, std::vector<Operation>> opByVal;
};

}  // namespace polylin