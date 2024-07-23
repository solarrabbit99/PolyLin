#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base.hpp"
#include "stack.hpp"

namespace polylin {

template <typename value_type>
class DequeLin : LinBase<value_type> {
  typedef Operation<value_type> oper_t;
  typedef History<value_type> hist_t;

 public:
  DequeLin()
      : LinBase<value_type>{{PUSH_FRONT, PUSH_BACK}, {POP_FRONT, POP_BACK}} {}

  // Assumption: at most one `PUSH`, valid deque oper_ts.
  // Time complexity: O(n^3)
  bool distVal(hist_t& hist) {
    if (!LinBase<value_type>::preprocess(hist)) return false;

    DistValParams params;
    getOneSidedVals(hist, params.oneSidedVals);
    if (!testOneSidedHists(hist, params.oneSidedVals)) return false;

    removeConcurrentOneSided(hist, params.oneSidedVals);

    for (const oper_t& o : hist) {
      params.events.emplace_back(o.startTime, true, o);
      params.events.emplace_back(o.endTime, false, o);
      if (o.method == Method::PUSH_FRONT) params.pushFrontVals.insert(o.value);
      if (o.method == Method::POP_FRONT) params.popFrontVals.insert(o.value);
      params.frontSizeByVal[o.value] += isFrontMethod(o.method);
      ++params.sizeByVal[o.value];
    }
    std::sort(params.events.begin(), params.events.end());

    size_t n = hist.size();
    std::vector<std::vector<std::optional<bool>>> distValMat = std::vector(
        2 * n, std::vector<std::optional<bool>>(2 * n, std::nullopt));

    return distValHelper(0, 0, distValMat, params);
  }

 private:
  struct DistValParams {
    std::vector<std::tuple<time_type, bool, oper_t>> events;
    std::unordered_set<value_type> oneSidedVals;
    std::unordered_map<value_type, size_t> sizeByVal;
    std::unordered_map<value_type, size_t> frontSizeByVal;
    std::unordered_set<value_type> pushFrontVals;
    std::unordered_set<value_type> popFrontVals;
  };

  // Remove one-sided values where all operations are concurrent
  void removeConcurrentOneSided(
      hist_t& hist, const std::unordered_set<value_type>& oneSidedVals) {
    std::unordered_map<value_type, time_type> minResTime, maxInvTime;
    for (const oper_t& o : hist) {
      if (!minResTime.count(o.value)) {
        minResTime[o.value] = o.endTime;
        maxInvTime[o.value] = o.startTime;
      } else {
        minResTime[o.value] = std::min(minResTime[o.value], o.endTime);
        maxInvTime[o.value] = std::max(maxInvTime[o.value], o.startTime);
      }
    }
    hist_t dup;
    for (const oper_t& o : hist) {
      if (!oneSidedVals.count(o.value) ||
          minResTime[o.value] < maxInvTime[o.value])
        dup.emplace(o);
    }
    std::swap(dup, hist);
  }

  void getOneSidedVals(const hist_t& hist,
                       std::unordered_set<value_type>& vals) {
    std::unordered_set<value_type> backVals;
    for (const oper_t& o : hist) {
      if (isFrontMethod(o.method))
        vals.insert(o.value);
      else
        backVals.insert(o.value);
    }
    for (const value_type& val : backVals) {
      if (vals.count(val))
        vals.erase(val);
      else
        vals.insert(val);
    }
  }

  // Test linearizability of subhist_t of values with only front or only back
  // O(n log n)
  bool testOneSidedHists(const hist_t& hist,
                         std::unordered_set<value_type>& vals) {
    hist_t frontHist, backHist;
    for (const oper_t& oldOp : hist) {
      if (!vals.count(oldOp.value)) continue;

      oper_t newOp{oldOp};
      if (isPush(oldOp.method))
        newOp.method = Method::PUSH;
      else if (isPeek(oldOp.method))
        newOp.method = Method::PEEK;
      else if (isPop(oldOp.method))
        newOp.method = Method::POP;

      if (isFrontMethod(oldOp.method)) {
        frontHist.emplace(std::move(newOp));
      } else
        backHist.emplace(std::move(newOp));
    }

    StackLin<value_type> stackLin;
    return stackLin.distVal(frontHist) && stackLin.distVal(backHist);
  }

  static bool isFrontMethod(Method method) {
    return method == Method::PUSH_FRONT || method == Method::PEEK_FRONT ||
           method == Method::POP_FRONT;
  }

  static bool isPush(Method method) {
    return method == Method::PUSH_FRONT || method == Method::PUSH_BACK;
  }

  static bool isPop(Method method) {
    return method == Method::POP_FRONT || method == Method::POP_BACK;
  }

  static bool isPeek(Method method) {
    return method == Method::PEEK_FRONT || method == Method::PEEK_BACK;
  }

  void findGoodVals(const size_t& i, const size_t& j,
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

  bool distValHelper(const size_t& i, const size_t& j,
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
    DequeLin<value_type>::OngoingSet frontOngoing, backOngoing;
    std::unordered_set<value_type> invalidVals;
    std::unordered_set<value_type> pendingFrontVals(goodVals),
        pendingBackVals(goodVals);

    for (int k = 0; k < n; ++k) {
      const auto& [_, isInv, o] = params.events[k];

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

      // Sanity check must be done per scheduling of oper_ts to keep an O(1)
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

  struct OngoingSet {
    std::unordered_set<value_type> ongoingPush;
    std::unordered_map<value_type, std::unordered_set<id_type>> ongoingPeek;
    std::unordered_set<value_type> ongoingPop;
    std::unordered_set<value_type> peekable, popable;
    std::unordered_set<value_type> peekableNow, popableNow;

    void insert(const oper_t& o) {
      if (DequeLin::isPush(o.method))
        ongoingPush.insert(o.value);
      else if (DequeLin::isPop(o.method)) {
        ongoingPop.insert(o.value);
        if (popable.count(o.value)) popableNow.insert(o.value);
      } else {
        ongoingPeek[o.value].insert(o.id);
        if (peekable.count(o.value)) peekableNow.insert(o.value);
      }
    }

    void erase(const oper_t& o) {
      if (DequeLin::isPush(o.method))
        ongoingPush.erase(o.value);
      else if (DequeLin::isPop(o.method)) {
        ongoingPop.erase(o.value);
        popableNow.erase(o.value);
      } else {
        ongoingPeek[o.value].erase(o.id);
        if (ongoingPeek[o.value].empty()) {
          ongoingPeek.erase(o.value);
          peekableNow.erase(o.value);
        }
      }
    }

    bool contains(const oper_t& o) {
      if (DequeLin::isPush(o.method))
        return ongoingPush.count(o.value);
      else if (DequeLin::isPop(o.method))
        return ongoingPop.count(o.value);
      else
        return ongoingPeek.count(o.value) && ongoingPeek[o.value].count(o.id);
    }

    void markPeekable(const value_type& v) {
      peekable.insert(v);
      if (ongoingPeek.count(v)) peekableNow.insert(v);
    }

    void markPopable(const value_type& v) {
      popable.insert(v);
      if (ongoingPop.count(v)) popableNow.insert(v);
    }
  };
};

}  // namespace polylin