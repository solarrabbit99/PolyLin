#include "deque.hpp"

#include <algorithm>

#include "stack.hpp"

using namespace polylin;

bool DequeLin::extend(History& hist) {
  time_type maxTime = MIN_TIME;
  proc_type maxProc = 0;
  std::unordered_map<value_type, int> pushPopDelta;
  for (const Operation& o : hist) {
    if (isPush(o.method))
      pushPopDelta[o.value]++;
    else if (isPop(o.method))
      pushPopDelta[o.value]--;
    maxTime = std::max(maxTime, o.endTime);
    maxProc = std::max(maxProc, o.proc);
  }
  for (const auto& [value, cnt] : pushPopDelta) {
    if (cnt < 0) return false;
    for (int i = 0; i < cnt; ++i)
      hist.emplace(++maxProc, Method::POP_FRONT, value, maxTime + 1,
                   maxTime + 2);
  }
  return true;
}

bool DequeLin::tune(History& hist,
                    const std::unordered_set<value_type>& oneSidedVals) {
  std::unordered_map<value_type, time_type> minResTime, maxInvTime;
  std::unordered_map<value_type, Operation> pushOp, popOp;
  std::unordered_map<value_type, std::unordered_set<Operation>> peekOps;
  for (const Operation& o : hist) {
    if (!minResTime.count(o.value)) {
      minResTime[o.value] = o.endTime;
      maxInvTime[o.value] = o.startTime;
    } else {
      minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
      maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
    }

    if (isPush(o.method)) pushOp.emplace(o.value, o);
    if (isPop(o.method)) popOp.emplace(o.value, o);
    if (isPeek(o.method)) peekOps[o.value].emplace(o);
  }
  for (const auto& [value, resTime] : minResTime) {
    if (!pushOp.count(value)) return false;
    Operation& valPushOp = pushOp.at(value);
    Operation& valPopOp = popOp.at(value);
    valPushOp.endTime = resTime;
    valPopOp.startTime = maxInvTime[value];

    if (valPushOp.startTime >= valPushOp.endTime ||
        valPopOp.startTime >= valPopOp.endTime)
      return false;

    hist.erase(valPushOp);
    hist.erase(valPopOp);

    if (valPushOp.endTime > valPopOp.startTime && oneSidedVals.count(value)) {
      hist.erase(peekOps[value].begin(), peekOps[value].end());
    } else {
      hist.emplace(valPushOp);
      hist.emplace(valPopOp);
    }
  }
  return true;
}

bool DequeLin::getOneSidedVals(const History& hist,
                               std::unordered_set<value_type>& vals) {
  std::unordered_set<value_type> frontSidedVals, backSidedVals;
  for (const Operation& o : hist) {
    if (isFrontMethod(o.method))
      frontSidedVals.insert(o.value);
    else
      backSidedVals.insert(o.value);
  }
  vals = std::move(frontSidedVals);
  for (const value_type& val : backSidedVals) {
    if (vals.count(val))
      vals.erase(val);
    else
      vals.insert(val);
  }
  // Test linearizability of subhistory of values with only front or only back
  History frontHist, backHist;
  for (const Operation& o : hist) {
    if (!vals.count(o.value)) continue;

    Operation o2{o};
    if (isPush(o.method))
      o2.method = Method::PUSH;
    else if (isPeek(o.method))
      o2.method = Method::PEEK;
    else if (isPop(o.method))
      o2.method = Method::POP;

    if (isFrontMethod(o.method)) {
      frontHist.emplace(std::move(o2));
    } else
      backHist.emplace(std::move(o2));
  }

  StackLin stackLin;
  return stackLin.distVal(frontHist) && stackLin.distVal(backHist);
}

bool DequeLin::isFrontMethod(Method method) {
  return method == Method::PUSH_FRONT || method == Method::PEEK_FRONT ||
         method == Method::POP_FRONT;
}

bool DequeLin::isPush(Method method) {
  return method == Method::PUSH_FRONT || method == Method::PUSH_BACK;
}

bool DequeLin::isPeek(Method method) {
  return method == Method::PEEK_FRONT || method == Method::PEEK_BACK;
}

bool DequeLin::isPop(Method method) {
  return method == Method::POP_FRONT || method == Method::POP_BACK;
}

void DequeLin::findGoodVals(const size_t& i, const size_t& j,
                            const DistValParams& params,
                            std::unordered_set<value_type>& goodVals) {
  std::unordered_map<value_type, size_t> ongoings;
  std::unordered_set<value_type> badVals;
  for (int k = 0; k < std::max(i, j); ++k) {
    const auto& [time, isInv, o] = params.events[k];
    if (isInv) {
      if (k < i && isFrontMethod(o.method)) {
        if (o.method == Method::PEEK_FRONT ||
            (i <= j && o.method == Method::PUSH_FRONT) ||
            (i >= j && o.method == Method::POP_FRONT))
          ++ongoings[o.value];
      }
      if (k < j && !isFrontMethod(o.method)) {
        if (o.method == Method::PEEK_BACK ||
            (i >= j && o.method == Method::PUSH_BACK) ||
            (i <= j && o.method == Method::POP_BACK))
          ++ongoings[o.value];
      }
    } else {
      if (k < i && isFrontMethod(o.method)) {
        if (o.method == Method::PEEK_FRONT ||
            (i <= j && o.method == Method::PUSH_FRONT) ||
            (i >= j && o.method == Method::POP_FRONT))
          --ongoings[o.value];
        badVals.insert(o.value);
      }
      if (k < j && !isFrontMethod(o.method)) {
        if (o.method == Method::PEEK_BACK ||
            (i >= j && o.method == Method::PUSH_BACK) ||
            (i <= j && o.method == Method::POP_BACK))
          --ongoings[o.value];
        badVals.insert(o.value);
      }
    }
  }
  for (const auto& [value, size] : params.sizeByVal) {
    if (badVals.count(value) || params.oneSidedVals.count(value)) continue;
    if (size == ongoings[value]) {
      if (i == j) continue;
      if (i < j && params.pushFrontVals.count(value) &&
          !params.popFrontVals.count(value))
        continue;
      if (i > j && !params.pushFrontVals.count(value) &&
          params.popFrontVals.count(value))
        continue;
    }
    goodVals.insert(value);
  }
}

bool DequeLin::distValHelper(
    const size_t& i, const size_t& j,
    std::vector<std::vector<std::optional<bool>>>& distValMat,
    const DistValParams& params) {
  size_t n = params.events.size();
  if (i == n || j == n) return true;
  if (distValMat[i][j].has_value()) return distValMat[i][j].value();
  // Get good values (not in one-sided) O(n)
  std::unordered_set<value_type> goodVals;
  findGoodVals(i, j, params, goodVals);
  if (goodVals.empty()) {
    distValMat[i][j] = true;
    return true;
  }

  // Interate through good values and check if schedulable
  for (const value_type& val : goodVals) {
    std::unordered_set<value_type> critValsFront, critValsBack;
    std::unordered_set<id_type> ongoingsFront, ongoingsBack;
    size_t endedFront = 0, endedBack = 0;
    size_t nexti = i, nextj = j;
    bool pendingPush = false, pendingPop = false, hasPushed = false;
    id_type popId, pushId;
    for (int k = 0; k < n; ++k) {
      const auto& [_, isInv, o] = params.events[k];
      if (isInv) {
        if (params.oneSidedVals.count(o.value) && isPop(o.method)) {
          if (isFrontMethod(o.method))
            critValsFront.erase(o.value);
          else
            critValsBack.erase(o.value);
        } else if (o.value == val) {
          if (isFrontMethod(o.method)) ongoingsFront.insert(o.id);
          if (!isFrontMethod(o.method)) ongoingsBack.insert(o.id);
          if (isPush(o.method)) {
            pendingPush = true;
            pushId = o.id;
          }
          if (isPop(o.method)) {
            pendingPop = true;
            popId = o.id;
          }
        }
      } else {
        if (params.oneSidedVals.count(o.value) && isPush(o.method)) {
          if (isFrontMethod(o.method))
            critValsFront.insert(o.value);
          else
            critValsBack.insert(o.value);
        } else if (o.value == val) {
          if (isFrontMethod(o.method) && ongoingsFront.count(o.id)) break;
          if (!isFrontMethod(o.method) && ongoingsBack.count(o.id)) break;
        } else if (goodVals.count(o.value)) {
          if (isFrontMethod(o.method) && !endedFront) break;
          if (!isFrontMethod(o.method) && !endedBack) break;
        }
      }
      // Scheduling logic
      if (!pendingPush) continue;
      auto clearFrontExceptPop = [&]() {
        bool hasPop = pendingPop && ongoingsFront.count(popId);
        endedFront += ongoingsFront.size();
        if (endedFront == params.frontSizeByVal.at(val)) nexti = k + 1;
        ongoingsFront.clear();
        if (hasPop) {
          --endedFront;
          ongoingsFront.insert(popId);
        }
      };
      auto clearBackExceptPop = [&]() {
        bool hasPop = pendingPop && ongoingsBack.count(popId);
        endedBack += ongoingsBack.size();
        if (endedBack ==
            params.sizeByVal.at(val) - params.frontSizeByVal.at(val))
          nextj = k + 1;
        ongoingsBack.clear();
        if (hasPop) {
          --endedBack;
          ongoingsBack.insert(popId);
        }
      };
      // Try scheduling any pending push
      if (ongoingsFront.count(pushId) && critValsFront.empty() && k + 1 >= i) {
        hasPushed = true;
        clearFrontExceptPop();
      } else if (ongoingsBack.count(pushId) && critValsBack.empty() &&
                 k + 1 >= j) {
        hasPushed = true;
        clearBackExceptPop();
      }
      if (!hasPushed) continue;
      // Try scheduling any pending peeks
      if (critValsFront.empty() && k + 1 >= i &&
          (!pendingPop || !ongoingsFront.count(popId)))
        clearFrontExceptPop();
      if (critValsBack.empty() && k + 1 >= j &&
          (!pendingPop || !ongoingsBack.count(popId)))
        clearBackExceptPop();
      // Try scheduling any pending pop
      if (critValsFront.empty() && k + 1 >= i && pendingPop &&
          ongoingsFront.count(popId) &&
          endedFront + endedBack + 1 == params.sizeByVal.at(val)) {
        nexti = k + 1;
        if (distValHelper(nexti, nextj, distValMat, params)) {
          distValMat[i][j] = true;
          return true;
        }
        break;
      }
      if (critValsBack.empty() && k + 1 >= j && pendingPop &&
          ongoingsBack.count(popId) &&
          endedFront + endedBack + 1 == params.sizeByVal.at(val)) {
        nextj = k + 1;
        if (distValHelper(nexti, nextj, distValMat, params)) {
          distValMat[i][j] = true;
          return true;
        }
        break;
      }
    }
  }
  distValMat[i][j] = false;
  return false;
}

bool DequeLin::distVal(History& hist) {
  DistValParams params;
  if (!extend(hist) || !getOneSidedVals(hist, params.oneSidedVals) ||
      !tune(hist, params.oneSidedVals))
    return false;

  for (const Operation& o : hist) {
    params.events.emplace_back(o.startTime, true, o);
    params.events.emplace_back(o.endTime, false, o);
    if (o.method == Method::PUSH_FRONT) params.pushFrontVals.insert(o.value);
    if (o.method == Method::POP_FRONT) params.popFrontVals.insert(o.value);
    if (isFrontMethod(o.method)) ++params.frontSizeByVal[o.value];
    ++params.sizeByVal[o.value];
  }
  std::sort(params.events.begin(), params.events.end());

  size_t n = hist.size();
  std::vector<std::vector<std::optional<bool>>> distValMat =
      std::vector(2 * n, std::vector<std::optional<bool>>(2 * n, std::nullopt));

  return distValHelper(0, 0, distValMat, params);
}