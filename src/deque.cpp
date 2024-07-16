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
  std::unordered_set<value_type> critValsFront, critValsBack;
  std::unordered_map<value_type, size_t> endedFront, endedBack;
  std::unordered_map<value_type, size_t> nexti, nextj;
  OngoingSet frontOngoing, backOngoing;
  std::unordered_set<value_type> invalidVals;
  std::unordered_set<value_type> pendingFrontVals(goodVals),
      pendingBackVals(goodVals);

  for (int k = 0; k < n; ++k) {
    const auto& [_, isInv, o] = params.events[k];
    if (!goodVals.count(o.value) && !params.oneSidedVals.count(o.value))
      continue;

    if (isInv) {
      if (params.oneSidedVals.count(o.value) && isPop(o.method)) {
        if (isFrontMethod(o.method))
          critValsFront.erase(o.value);
        else
          critValsBack.erase(o.value);
      } else if (goodVals.count(o.value)) {
        if (isFrontMethod(o.method))
          frontOngoing.insert(o);
        else
          backOngoing.insert(o);
      }
    } else {
      if (params.oneSidedVals.count(o.value) && isPush(o.method)) {
        if (isFrontMethod(o.method))
          critValsFront.insert(o.value);
        else
          critValsBack.insert(o.value);
      } else if (goodVals.count(o.value)) {
        if (frontOngoing.contains(o) || backOngoing.contains(o)) {
          invalidVals.insert(o.value);
          pendingFrontVals.erase(o.value);
          pendingBackVals.erase(o.value);
        }
        if (isFrontMethod(o.method)) {
          std::unordered_set<value_type> tmp;
          for (const value_type& v : pendingFrontVals)
            if (v != o.value)
              invalidVals.insert(v);
            else
              tmp.insert(v);
          std::swap(pendingFrontVals, tmp);
        } else {
          std::unordered_set<value_type> tmp;
          for (const value_type& v : pendingBackVals)
            if (v != o.value)
              invalidVals.insert(v);
            else
              tmp.insert(v);
          std::swap(pendingBackVals, tmp);
        }
      }
    }

    // Sanity check must be done per scheduling of operations to keep an O(1)
    // time complexity
    auto sanityCheck = [&](const value_type& v) {
      if (endedFront[v] == params.frontSizeByVal.at(v) && !nexti.count(v)) {
        pendingFrontVals.erase(v);
        nexti[v] = k + 1;
      }
      if (endedBack[v] ==
              params.sizeByVal.at(v) - params.frontSizeByVal.at(v) &&
          !nextj.count(v)) {
        pendingBackVals.erase(v);
        nextj[v] = k + 1;
      }
      if (endedFront[v] + endedBack[v] + 1 == params.sizeByVal.at(v)) {
        frontOngoing.markPopable(v);
        backOngoing.markPopable(v);
      }
    };
    // Try scheduling any pending push
    if (critValsFront.empty() && k + 1 >= i) {
      for (const value_type& v : frontOngoing.ongoingPush) {
        ++endedFront[v];
        frontOngoing.markPeekable(v);
        backOngoing.markPeekable(v);
        sanityCheck(v);
      }
      frontOngoing.ongoingPush.clear();
    }
    if (critValsBack.empty() && k + 1 >= j) {
      for (const value_type& v : backOngoing.ongoingPush) {
        ++endedBack[v];
        frontOngoing.markPeekable(v);
        backOngoing.markPeekable(v);
        sanityCheck(v);
      }
      backOngoing.ongoingPush.clear();
    }
    // Try scheduling any pending peeks
    if (critValsFront.empty() && k + 1 >= i) {
      for (const value_type& v : frontOngoing.peekableNow) {
        endedFront[v] += frontOngoing.ongoingPeek[v].size();
        frontOngoing.ongoingPeek.erase(v);
        sanityCheck(v);
      }
      frontOngoing.peekableNow.clear();
    }
    if (critValsBack.empty() && k + 1 >= j) {
      for (const value_type& v : backOngoing.peekableNow) {
        endedBack[v] += backOngoing.ongoingPeek[v].size();
        backOngoing.ongoingPeek.erase(v);
        sanityCheck(v);
      }
      backOngoing.peekableNow.clear();
    }
    // Try scheduling any pending pop
    if (critValsFront.empty() && k + 1 >= i) {
      for (const value_type& v : frontOngoing.popableNow) {
        ++endedFront[v];
        frontOngoing.ongoingPop.erase(v);
        sanityCheck(v);
        if (!invalidVals.count(v) &&
            distValHelper(nexti.at(v), nextj.at(v), distValMat, params)) {
          distValMat[i][j] = true;
          return true;
        }
      }
      frontOngoing.popableNow.clear();
    }
    if (critValsBack.empty() && k + 1 >= j) {
      for (const value_type& v : backOngoing.popableNow) {
        ++endedBack[v];
        backOngoing.ongoingPop.erase(v);
        sanityCheck(v);
        if (!invalidVals.count(v) &&
            distValHelper(nexti.at(v), nextj.at(v), distValMat, params)) {
          distValMat[i][j] = true;
          return true;
        }
      }
      backOngoing.popableNow.clear();
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
    params.frontSizeByVal[o.value] += isFrontMethod(o.method);
    ++params.sizeByVal[o.value];
  }
  std::sort(params.events.begin(), params.events.end());

  size_t n = hist.size();
  std::vector<std::vector<std::optional<bool>>> distValMat =
      std::vector(2 * n, std::vector<std::optional<bool>>(2 * n, std::nullopt));

  return distValHelper(0, 0, distValMat, params);
}