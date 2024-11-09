#pragma once

#include <algorithm>
#include <climits>
#include <vector>

namespace polylin {

// `O(log n)` range update
// `O(log n)` point query
// `O(1)` minimum query segment tree
// minimum size 1
struct segment_tree {
 public:
  segment_tree(const size_t& size) : tree(size * 4), size(size) {
    build(1, 0, size - 1);
  }

  void update_range(int l, int r, int addend) {
    update_range(1, 0, size - 1, l, r, addend);
  }

  std::pair<int, int> query_min() {
    return {tree[1].min_value, tree[1].min_pos};
  }

  int query_val(int pos) { return query_val(1, 0, size - 1, pos); }

 private:
  struct segment_tree_node {
    int min_value;
    int min_pos;
    int weight;
  };

  void build(int v, int tl, int tr) {
    tree[v] = {0, tl, 0};
    if (tl != tr) {
      int tm = (tl + tr) / 2;
      build(v * 2, tl, tm);
      build(v * 2 + 1, tm + 1, tr);
    }
  }

  void merge(segment_tree_node& par, const segment_tree_node& a,
             const segment_tree_node& b) {
    if (a.min_value <= b.min_value) {
      par.min_value = a.min_value;
      par.min_pos = a.min_pos;
    } else {
      par.min_value = b.min_value;
      par.min_pos = b.min_pos;
    }
  }

  void propagate(int v) {
    if (tree[v].weight != 0) {
      apply(v * 2, tree[v].weight);
      apply(v * 2 + 1, tree[v].weight);
      tree[v].weight = 0;
    }
  }

  void apply(int v, int addend) {
    tree[v].min_value += addend;
    tree[v].weight += addend;
  }

  void update_range(int v, int tl, int tr, int l, int r, int addend) {
    if (l > r) return;
    if (l == tl && r == tr) {
      apply(v, addend);
    } else {
      propagate(v);
      int tm = (tl + tr) / 2;
      update_range(v * 2, tl, tm, l, std::min(r, tm), addend);
      update_range(v * 2 + 1, tm + 1, tr, std::max(l, tm + 1), r, addend);
      merge(tree[v], tree[v * 2], tree[v * 2 + 1]);
    }
  }

  int query_val(int v, int tl, int tr, int pos) {
    if (tl == tr) return tree[v].weight;
    int tm = (tl + tr) / 2;
    return ((pos <= tm) ? query_val(v * 2, tl, tm, pos)
                        : query_val(v * 2 + 1, tm + 1, tr, pos)) +
           tree[v].weight;
  }

  // root at `1`, left child at `2*par`, right child at `2*par+1`
  // for odd size ranges, mid belongs to right child
  std::vector<segment_tree_node> tree;
  int size;
};

}  // namespace polylin