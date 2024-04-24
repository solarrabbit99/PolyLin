#pragma once
#include <unordered_map>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class StackLin {
 public:
  // Assumption: at most one `PUSH`, valid stack operations.
  // Time complexity: O(n^2)
  bool stackDistValLin(History& hist);

 private:
  // Returns `false` if there is value where `POP` methods > `PUSH` methods
  bool stackExtend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `PUSH` operation)
  bool stackTune(History& hist);

  std::unordered_map<value_type, std::vector<Operation>> opByVal;
};

}  // namespace polylin