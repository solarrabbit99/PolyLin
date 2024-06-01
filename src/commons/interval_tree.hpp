// interval tree efficient insert/delete and point query
// Generated by openai

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
  Node* root;

  interval_tree() : root(nullptr) {}

  void insert(interval i) { root = insert(root, i); }

  void remove(interval i) { root = remove(root, i); }

  std::vector<interval> query(int point) {
    std::vector<interval> result;
    query(root, point, result);
    return result;
  }

  bool empty() { return root == nullptr; }

 private:
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

  Node* insert(Node* node, interval i) {
    if (!node) return new Node(i);

    if (i.start < node->intvl.start) {
      node->left = insert(node->left, i);
    } else {
      node->right = insert(node->right, i);
    }

    node->height = std::max(height(node->left), height(node->right)) + 1;
    node->maxEnd = std::max(node->intvl.end,
                            std::max(maxEnd(node->left), maxEnd(node->right)));

    int balance = getBalance(node);

    if (balance > 1 && i.start < node->left->intvl.start) {
      return rightRotate(node);
    }

    if (balance < -1 && i.start >= node->right->intvl.start) {
      return leftRotate(node);
    }

    if (balance > 1 && i.start >= node->left->intvl.start) {
      node->left = leftRotate(node->left);
      return rightRotate(node);
    }

    if (balance < -1 && i.start < node->right->intvl.start) {
      node->right = rightRotate(node->right);
      return leftRotate(node);
    }

    return node;
  }

  Node* minValueNode(Node* node) {
    Node* current = node;
    while (current->left != nullptr) {
      current = current->left;
    }
    return current;
  }

  Node* remove(Node* root, interval i) {
    if (!root) return root;

    if (i.start < root->intvl.start) {
      root->left = remove(root->left, i);
    } else if (i.start > root->intvl.start) {
      root->right = remove(root->right, i);
    } else {
      if (!root->left || !root->right) {
        Node* temp = root->left ? root->left : root->right;
        if (!temp) {
          temp = root;
          root = nullptr;
        } else {
          *root = *temp;
        }
        delete temp;
      } else {
        Node* temp = minValueNode(root->right);
        root->intvl = temp->intvl;
        root->right = remove(root->right, temp->intvl);
      }
    }

    if (!root) return root;

    root->height = std::max(height(root->left), height(root->right)) + 1;
    root->maxEnd = std::max(root->intvl.end,
                            std::max(maxEnd(root->left), maxEnd(root->right)));

    int balance = getBalance(root);

    if (balance > 1 && getBalance(root->left) >= 0) {
      return rightRotate(root);
    }

    if (balance > 1 && getBalance(root->left) < 0) {
      root->left = leftRotate(root->left);
      return rightRotate(root);
    }

    if (balance < -1 && getBalance(root->right) <= 0) {
      return leftRotate(root);
    }

    if (balance < -1 && getBalance(root->right) > 0) {
      root->right = rightRotate(root->right);
      return leftRotate(root);
    }

    return root;
  }

  void query(Node* node, int point, std::vector<interval>& result) {
    if (!node) return;

    if (node->intvl.start <= point && point <= node->intvl.end) {
      result.push_back(node->intvl);
    }

    if (node->left && node->left->maxEnd >= point) {
      query(node->left, point, result);
    }

    if (node->right && node->intvl.start <= point) {
      query(node->right, point, result);
    }
  }
};

}  // namespace polylin