#ifndef SIMPLE_TREE_HPP
#define SIMPLE_TREE_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

namespace ads {
namespace bp {

class SimpleTree {
 private:
  /**
   * @brief Simple node structure.
   */
  struct Node {
    Node *parent = nullptr;
    std::vector<Node *> children = {};
  };

  /**
   * @brief Recursively deletes the tree.
   *
   * @param node The starting node.
   */
  void delete_recursive(Node *node) {
    for (const auto &child : node->children) {
      delete_recursive(child);
    }
    delete node;
  }

  /**
   * @brief Fills the string buffer with the trees BP representation.
   *
   * @param oss The string stream.
   * @param node The node where to start.
   */
  void get_bp_representation(std::ostringstream &oss, Node *node) const {
    oss << "(";
    for (const auto &child : node->children) {
      get_bp_representation(oss, child);
    }
    oss << ")";
  }

  /**
   * @brief The space used at the given node.
   *
   * @param node The node to start.
   * @return The space used.
   */
  size_t space_used_at_node(Node *node) const {
    size_t s = (node->children.capacity() + 1) * sizeof(Node *);
    for (const auto &child : node->children) {
      s += space_used_at_node(child);
    }
    return s;
  }

 public:
  Node *root;

  /**
   * @brief Constructs a new simple tree object.
   */
  SimpleTree() : root(new Node()) {}

  SimpleTree(SimpleTree &) = delete;
  SimpleTree(const SimpleTree &) = delete;
  SimpleTree &operator=(SimpleTree &) = delete;
  SimpleTree &operator=(const SimpleTree &) = delete;
  SimpleTree(SimpleTree &&) = delete;

  /**
   * @brief Destroys the tree.
   */
  ~SimpleTree() { delete_recursive(root); }

  /**
   * @brief Returns the i-th child of the given node.
   *
   * @param node The node.
   * @param i The i-th child (one-based index).
   * @return The i-th child.
   */
  Node *i_th_child(Node *node, size_t i) const { return node->children[i - 1]; }

  /**
   * @brief Returns the parent node.
   *
   * @param node The node.
   * @return The parent node.
   */
  Node *parent(Node *node) const { return node->parent; }

  /**
   * @brief The size of the subtree under the given node.
   *
   * @param node The node.
   * @return The subtree size.
   */
  size_t subtree_size(Node *node) const {
    size_t size = 1;
    for (const auto &child : node->children) {
      size += subtree_size(child);
    }
    return size;
  }

  /**
   * @brief Deletes the node making all its children children of its parent.
   *
   * @param node The node to delete.
   */
  void delete_node(Node *node) {
    for (const auto &child : node->children) {
      child->parent = node->parent;
    }
    auto it = std::find(node->parent->children.begin(),
                        node->parent->children.end(), node);
    size_t idx = std::distance(node->parent->children.begin(), it);
    node->parent->children.erase(node->parent->children.begin() + idx);
    size_t i = 0;
    for (const auto &child : node->children) {
      node->parent->children.insert(node->parent->children.begin() + idx + i,
                                    child);
      ++i;
    }
    delete node;
  }

  /**
   * @brief Inserts a node under the given parent node as i-th child making the
   * old i-th to (i+k-1)-th children child of the inserted node.
   *
   * @param node The node under which to insert.
   * @param i As which child to insert.
   * @param k How many existing children to move under the inserted node.
   */
  void insert_node(Node *node, size_t i, size_t k) {
    Node *n = new Node();
    n->parent = node;
    for (size_t j = i; j < i + k; ++j) {
      node->children[i - 1]->parent = n;
      n->children.push_back(node->children[i - 1]);
      node->children.erase(node->children.begin() + i - 1);
    }
    node->children.insert(node->children.begin() + i - 1, n);
  }

  /**
   * @brief Returns the BP representation of this tree.
   *
   * @return The BP string.
   */
  std::string get_bp_representation() const {
    std::ostringstream oss;
    get_bp_representation(oss, root);
    return oss.str();
  }

  /**
   * @brief The space used.
   *
   * @return The space used.
   */
  size_t space_used() const { return space_used_at_node(root); }
};

}  // namespace bp
}  // namespace ads

#endif
