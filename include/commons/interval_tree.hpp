#pragma once

#include <climits>
#include <set>
#include <vector>

namespace polylin {

struct interval {
  int start;
  int end;
  interval(int s, int e) : start(s), end(e) {}
  interval() = default;
};

// interval tree efficient O(log n) insert/delete of intervals and `O(m log n)`
// point query
struct interval_tree {
  struct Node {
    interval intvl;
    int maxEnd;
    int height;
    Node* left;
    Node* right;

    Node(interval i)
        : intvl(i), maxEnd(i.end), height(1), left(nullptr), right(nullptr) {}
  };

 public:
  interval_tree() : root(nullptr) {}

  // Newly inserted interval must have unique `start` and `end`
  void insert(interval i) { root = insert(root, i); }

  // `i` must exist in the tree for correctness
  void remove(interval i) { root = remove(root, i); }

  // Retrieves all intervals overlapping `point`. `O(m log n)` time complexity,
  // where `m` is the size of output and `n` is the size of tree
  std::vector<interval> query(int point) {
    std::vector<interval> result;
    query(root, point, result);
    return result;
  }

  bool empty() { return root == nullptr; }

 private:
  Node* root;

  int height(Node* n) { return n ? n->height : 0; }

  int maxEnd(Node* n) { return n ? n->maxEnd : -2147483648; }

  int getBalance(Node* n) { return n ? height(n->left) - height(n->right) : 0; }

  Node* rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    y->maxEnd =
        std::max(y->intvl.end, std::max(maxEnd(y->left), maxEnd(y->right)));
    x->maxEnd =
        std::max(x->intvl.end, std::max(maxEnd(x->left), maxEnd(x->right)));

    return x;
  }

  Node* leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    x->maxEnd =
        std::max(x->intvl.end, std::max(maxEnd(x->left), maxEnd(x->right)));
    y->maxEnd =
        std::max(y->intvl.end, std::max(maxEnd(y->left), maxEnd(y->right)));

    return y;
  }

  Node* autoBalance(Node* node) {
    int balance = getBalance(node);
    if (balance >= 2) {                  // left heavy
      if (getBalance(node->left) == -1)  // left child is right heavy
        node->left = leftRotate(node->left);
      return rightRotate(node);
    }
    if (balance <= -2) {                 // right heavy
      if (getBalance(node->right) == 1)  // right child is left heavy
        node->right = rightRotate(node->right);
      return leftRotate(node);
    }
    return node;
  }

  Node* insert(Node* node, interval i) {
    if (!node) return new Node(i);

    if (i.start < node->intvl.start)
      node->left = insert(node->left, i);
    else
      node->right = insert(node->right, i);

    node->height = std::max(height(node->left), height(node->right)) + 1;
    node->maxEnd = std::max(node->intvl.end,
                            std::max(maxEnd(node->left), maxEnd(node->right)));

    return autoBalance(node);
  }

  Node* minValueNode(Node* node) {
    while (node->left != nullptr) node = node->left;
    return node;
  }

  Node* remove(Node* node, interval i) {
    if (!node) return node;

    if (i.start < node->intvl.start) {
      node->left = remove(node->left, i);
    } else if (i.start > node->intvl.start) {
      node->right = remove(node->right, i);
    } else {
      if (!node->left || !node->right) {
        Node* temp = node->left ? node->left : node->right;
        delete node;
        node = temp;
      } else {
        Node* temp = minValueNode(node->right);
        node->intvl = temp->intvl;
        node->right = remove(node->right, temp->intvl);
      }
    }

    if (!node) return node;

    node->height = std::max(height(node->left), height(node->right)) + 1;
    node->maxEnd = std::max(node->intvl.end,
                            std::max(maxEnd(node->left), maxEnd(node->right)));

    return autoBalance(node);
  }

  void query(Node* node, int point, std::vector<interval>& result) {
    if (!node) return;

    if (node->intvl.start <= point && point <= node->intvl.end)
      result.push_back(node->intvl);

    if (node->left && node->left->maxEnd >= point)
      query(node->left, point, result);

    if (node->right && node->intvl.start <= point)
      query(node->right, point, result);
  }
};

}  // namespace polylin