#pragma once

#include <algorithm>
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

  class DequeLinImpl {
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

   public:
    DequeLinImpl(const std::unordered_set<value_type>& goodVals,
                 const DistValParams& params)
        : pendingFrontVals(goodVals),
          pendingBackVals(goodVals),
          freeFront{false},
          freeBack{false},
          params{params} {};
    void setFreeFront(bool b) { freeFront = b; }
    void setFreeBack(bool b) { freeBack = b; }
    void schedulePushes(time_type k) {
      if (freeFront) {
        for (const value_type& v : ongoingFront.ongoingPush) {
          ++scheduledFront[v];
          ongoingFront.markPeekable(v);
          ongoingBack.markPeekable(v);
          latestFront[v] = k;
          check(v);
        }
        ongoingFront.ongoingPush.clear();
      }
      if (freeBack) {
        for (const value_type& v : ongoingBack.ongoingPush) {
          ++scheduledBack[v];
          ongoingFront.markPeekable(v);
          ongoingBack.markPeekable(v);
          latestBack[v] = k;
          check(v);
        }
        ongoingBack.ongoingPush.clear();
      }
    }
    void schedulePeeks(time_type k) {
      if (freeFront) {
        for (const value_type& v : ongoingFront.peekableNow) {
          scheduledFront[v] += ongoingFront.ongoingPeek[v].size();
          ongoingFront.ongoingPeek.erase(v);
          latestFront[v] = k;
          check(v);
        }
        ongoingFront.peekableNow.clear();
      }
      if (freeBack) {
        for (const value_type& v : ongoingBack.peekableNow) {
          scheduledBack[v] += ongoingBack.ongoingPeek[v].size();
          ongoingBack.ongoingPeek.erase(v);
          latestBack[v] = k;
          check(v);
        }
        ongoingBack.peekableNow.clear();
      }
    }
    std::vector<value_type> schedulePops(time_type k) {
      std::vector<value_type> poppedValues;
      if (freeFront) {
        for (const value_type& v : ongoingFront.popableNow) {
          ++scheduledFront[v];
          ongoingFront.ongoingPop.erase(v);
          if (!invalidVals.count(v)) poppedValues.push_back(v);
          check(v);
        }
        ongoingFront.popableNow.clear();
      }
      if (freeBack) {
        for (const value_type& v : ongoingBack.popableNow) {
          ++scheduledBack[v];
          ongoingBack.ongoingPop.erase(v);
          if (!invalidVals.count(v)) poppedValues.push_back(v);
          check(v);
        }
        ongoingBack.popableNow.clear();
      }
      return poppedValues;
    }
    void addOngoing(const oper_t& o) {
      if (isFrontMethod(o.method))
        ongoingFront.insert(o);
      else
        ongoingBack.insert(o);
    }
    bool isScheduled(const oper_t& o) {
      if (isFrontMethod(o.method)) return !ongoingFront.contains(o);
      return !ongoingBack.contains(o);
    }
    // Sanity check must be done per scheduling of oper_ts to keep an O(1)
    // time complexity
    void check(const value_type& v) {
      if (scheduledFront[v] == params.frontSizeByVal.at(v))
        pendingFrontVals.erase(v);
      if (scheduledBack[v] ==
          params.sizeByVal.at(v) - params.frontSizeByVal.at(v))
        pendingBackVals.erase(v);
      if (scheduledFront[v] + scheduledBack[v] + 1 == params.sizeByVal.at(v)) {
        ongoingFront.markPopable(v);
        ongoingBack.markPopable(v);
      }
    }
    void invalidateOthersFront(const value_type& v) {
      std::unordered_set<value_type> tmp;
      for (const value_type& v2 : pendingFrontVals)
        if (v2 != v)
          invalidVals.insert(v2);
        else
          tmp.insert(v);
      std::swap(pendingFrontVals, tmp);
    }
    void invalidateOthersBack(const value_type& v) {
      std::unordered_set<value_type> tmp;
      for (const value_type& v2 : pendingBackVals)
        if (v2 != v)
          invalidVals.insert(v2);
        else
          tmp.insert(v);
      std::swap(pendingBackVals, tmp);
    }
    void invalidate(const value_type& v) {
      invalidVals.insert(v);
      pendingFrontVals.erase(v);
      pendingBackVals.erase(v);
    }

    std::unordered_map<value_type, time_type> latestFront;
    std::unordered_map<value_type, time_type> latestBack;

   private:
    bool freeFront;
    bool freeBack;
    OngoingSet ongoingFront;
    OngoingSet ongoingBack;
    std::unordered_set<value_type> pendingFrontVals;
    std::unordered_set<value_type> pendingBackVals;
    std::unordered_set<value_type> invalidVals;
    std::unordered_map<value_type, size_t> scheduledFront;
    std::unordered_map<value_type, size_t> scheduledBack;
    const DistValParams& params;
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
    DequeLinImpl impl{goodVals, params};

    for (int k = 0; k < n; ++k) {
      const auto& [_, isInv, o] = params.events[k];
      auto updateFree = [&]() {
        if (isInv && isPop(o.method)) {
          if (isFrontMethod(o.method))
            critValsFront.erase(o.value);
          else
            critValsBack.erase(o.value);
        } else if (!isInv && isPush(o.method)) {
          if (isFrontMethod(o.method))
            critValsFront.insert(o.value);
          else
            critValsBack.insert(o.value);
        }
        impl.setFreeFront(critValsFront.empty() && k + 1 >= i);
        impl.setFreeBack(critValsBack.empty() && k + 1 >= j);
      };

      if (params.oneSidedVals.count(o.value))
        updateFree();
      else if (goodVals.count(o.value)) {
        if (isInv) {
          impl.addOngoing(o);
        } else {
          if (!impl.isScheduled(o)) impl.invalidate(o.value);
          if (isFrontMethod(o.method))
            impl.invalidateOthersFront(o.value);
          else
            impl.invalidateOthersBack(o.value);
        }
      }
      // Try scheduling any pending push
      impl.schedulePushes(k + 1);
      // Try scheduling any pending peeks
      impl.schedulePeeks(k + 1);
      // Try scheduling any pending pop
      for (const value_type& v : impl.schedulePops(k + 1))
        if (distValHelper(impl.latestFront.at(v), impl.latestBack.at(v),
                          distValMat, params)) {
          distValMat[i][j] = true;
          return true;
        }
    }
    distValMat[i][j] = false;
    return false;
  }
};

}  // namespace polylin