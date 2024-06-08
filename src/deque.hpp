#pragma once

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "definitions.hpp"

namespace polylin {

class DequeLin {
 public:
  // Assumption: at most one `PUSH`, valid deque operations.
  // Time complexity: O(n^4)
  bool distVal(History& hist);

 private:
  struct DistValParams {
    std::vector<std::tuple<time_type, bool, Operation>> events;
    std::unordered_set<value_type> oneSidedVals;
    std::unordered_map<value_type, size_t> sizeByVal;
    std::unordered_map<value_type, size_t> frontSizeByVal;
    std::unordered_set<value_type> pushFrontVals;
    std::unordered_set<value_type> popFrontVals;
  };

  // Returns `false` if there is value where `POP` methods > `PUSH` methods.
  // O(n)
  bool extend(History& hist);

  // For distinct value restriction, return `false` if impossible to tune (e.g.
  // value has no `POP` operation) O(n)
  bool tune(History& hist, const std::unordered_set<value_type>& oneSidedVals);

  // O(n log n) for testing stack subinstance
  bool getOneSidedVals(const History& hist,
                       std::unordered_set<value_type>& vals);

  bool isFrontMethod(Method method);

  bool isPush(Method method);

  bool isPop(Method method);

  bool isPeek(Method method);

  void findGoodVals(const size_t& i, const size_t& j,
                    const DistValParams& params,
                    std::unordered_set<value_type>& goodVals);

  bool distValHelper(const size_t& i, const size_t& j,
                     std::vector<std::vector<std::optional<bool>>>& distValMat,
                     const DistValParams& params);
};

}  // namespace polylin