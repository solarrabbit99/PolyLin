#pragma once

#include <optional>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class DequeLin {
 public:
  // Assumption: at most one `PUSH`, valid deque operations.
  // Time complexity: O(n log n)
  bool distVal(History& hist);

 private:
  // Returns `false` if there is value where `POP` methods > `PUSH` methods
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `POP` operation)
  bool tune(History& hist);

  bool isFrontMethod(Method method);

  bool isPush(Method method);

  bool isPop(Method method);

  bool isPeek(Method method);

  bool distValHelper(const size_t& i, const size_t& j);

  std::vector<std::vector<std::optional<bool>>> distValMat;
  std::vector<std::tuple<time_type, bool, Operation>> events;
  std::unordered_set<value_type> oneSidedVals;
};

}  // namespace polylin