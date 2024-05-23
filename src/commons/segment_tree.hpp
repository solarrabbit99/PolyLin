// Range update range minimum query segment tree
// Generated by openai

#pragma once

#include <algorithm>
#include <climits>
#include <vector>

namespace polylin {

class SegmentTree {
 private:
  struct Node {
    int min_value;
    int min_pos;
    Node(int v = INT_MAX, int p = -1) : min_value(v), min_pos(p) {}
  };

  std::vector<Node> tree;
  std::vector<int> lazy;
  int size;

  void build(int v, int tl, int tr) {
    if (tl == tr) {
      tree[v] = Node(0, tl);
    } else {
      int tm = (tl + tr) / 2;
      build(v * 2, tl, tm);
      build(v * 2 + 1, tm + 1, tr);
      tree[v] = merge(tree[v * 2], tree[v * 2 + 1]);
    }
  }

  Node merge(const Node& a, const Node& b) {
    if (a.min_value < b.min_value) return a;
    if (b.min_value < a.min_value) return b;
    return a.min_pos < b.min_pos ? a : b;
  }

  void push(int v) {
    if (lazy[v] != 0) {
      apply(v * 2, lazy[v]);
      apply(v * 2 + 1, lazy[v]);
      lazy[v] = 0;
    }
  }

  void apply(int v, int addend) {
    tree[v].min_value += addend;
    lazy[v] += addend;
  }

  void update_range(int v, int tl, int tr, int l, int r, int addend) {
    if (l > r) return;
    if (l == tl && r == tr) {
      apply(v, addend);
    } else {
      push(v);
      int tm = (tl + tr) / 2;
      update_range(v * 2, tl, tm, l, std::min(r, tm), addend);
      update_range(v * 2 + 1, tm + 1, tr, std::max(l, tm + 1), r, addend);
      tree[v] = merge(tree[v * 2], tree[v * 2 + 1]);
    }
  }

  Node query_min(int v, int tl, int tr, int l, int r) {
    if (l > r) return Node();
    if (l <= tl && tr <= r) {
      return tree[v];
    }
    push(v);
    int tm = (tl + tr) / 2;
    return merge(query_min(v * 2, tl, tm, l, std::min(r, tm)),
                 query_min(v * 2 + 1, tm + 1, tr, std::max(l, tm + 1), r));
  }

 public:
  SegmentTree(const size_t& size) {
    tree.resize(size * 4);
    lazy.resize(size * 4);
    build(1, 0, size - 1);
  }

  void update_range(int l, int r, int addend) {
    update_range(1, 0, size - 1, l, r, addend);
  }

  std::pair<int, int> query_min(int l, int r) {
    Node res = query_min(1, 0, size - 1, l, r);
    return {res.min_value, res.min_pos};
  }
};

}  // namespace polylin