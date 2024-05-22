#pragma once
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

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `PUSH` operation)
  bool tune(History& hist);
};

}  // namespace polylin