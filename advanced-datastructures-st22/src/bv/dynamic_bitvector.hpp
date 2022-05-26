#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

#include "bv/simple_bitvector.hpp"

namespace ads {
namespace bv {

/**
 * @brief Dynamic bit vector based on red-black tree.
 *
 * @tparam BlockType The bit vector block type, e.g., uint<w>_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks, e.g., w / 2.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks, e.g., w.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks, e.g., 2 * w.
 */
template <class BlockType, class SizeType, SizeType MinLeafSizeBlocks,
          SizeType InitialLeafSizeBlocks, SizeType MaxLeafSizeBlocks>
class DynamicBitVector {
 private:
  static_assert(MinLeafSizeBlocks < InitialLeafSizeBlocks,
                "Leaf sizes invalid.");
  static_assert(InitialLeafSizeBlocks < MaxLeafSizeBlocks,
                "Leaf sizes invalid.");

  /**
   * @brief Colors of the nodes.
   */
  enum class Color { RED, BLACK, DOUBLE_BLACK };

  /**
   * @brief Node helper class. Represents a leaf if leaf_data not null.
   */
  struct Node {
    // Pointer
    Node *parent = nullptr;
    Node *left = nullptr;
    Node *right = nullptr;

    // Color
    Color color = Color::RED;

    // Data for rank and select queries
    SizeType num_bits_left_tree = 0;
    SizeType ones_in_left_tree = 0;

    // If not null, node represents leaf
    SimpleBitVector<BlockType, SizeType> *leaf_data = nullptr;
  };

  Node *root;
  SizeType current_size;

  /**
   * @brief Get the color of the node.
   *
   * @param node The node.
   * @return The node's color or black if null.
   */
  inline Color get_color(Node *node) const {
    if (node == nullptr) {
      return Color::BLACK;
    } else {
      return node->color;
    }
  }

  /**
   * @brief Set the color of the given node.
   *
   * @param node The node.
   * @param color The color to set.
   */
  inline void set_color(Node *node, Color color) {
    if (node != nullptr) {
      node->color = color;
    }
  }

  /**
   * @brief Left rotation.
   *
   * @param node The node to perform the left rotation on.
   */
  inline void rotate_left(Node *node) {
    // Rotate right node's left child left
    Node *right_child = node->right;
    node->right = right_child->left;
    if (node->right != nullptr) {
      node->right->parent = node;
    }
    right_child->parent = node->parent;

    if (node->parent == nullptr) {
      // Right node is the new root
      root = right_child;
    } else if (node == node->parent->left) {
      // Change parent based on rotation
      node->parent->left = right_child;
    } else {
      // Change parent based on rotation
      node->parent->right = right_child;
    }

    // Complete left rotation
    right_child->left = node;
    node->parent = right_child;

    // Update bit counters
    right_child->num_bits_left_tree += node->num_bits_left_tree;
    right_child->ones_in_left_tree += node->ones_in_left_tree;
  }

  /**
   * @brief Right rotation.
   *
   * @param node The node to perform the right rotation on.
   */
  inline void rotate_right(Node *node) {
    // Rotate left node's right child right
    Node *left_child = node->left;
    node->left = left_child->right;
    if (node->left != nullptr) {
      node->left->parent = node;
    }
    left_child->parent = node->parent;

    if (node->parent == nullptr) {
      // Left node is the new root
      root = left_child;
    } else if (node == node->parent->left) {
      // Change parent based on rotation
      node->parent->left = left_child;
    } else {
      // Change parent based on rotation
      node->parent->right = left_child;
    }

    // Complete right rotation
    left_child->right = node;
    node->parent = left_child;

    // Update bit counters
    node->num_bits_left_tree -= left_child->num_bits_left_tree;
    node->ones_in_left_tree -= left_child->ones_in_left_tree;
  }

  /**
   * @brief Check invariant and rebalance if necessary after insertion.
   *
   * @param node The node
   */
  inline void rebalance_after_insertion(Node *node) {
    // Traverse tree back to root and fix violated invariant
    Node *parent = nullptr;
    Node *grandparent = nullptr;
    while (node != root && get_color(node) == Color::RED &&
           get_color(node->parent) == Color::RED) {
      parent = node->parent;
      grandparent = parent->parent;

      // Whether we are coming from the left or right subtree
      if (parent == grandparent->left) {
        Node *uncle = grandparent->right;

        if (get_color(uncle) == Color::RED) {
          // Recoloring possible
          set_color(uncle, Color::BLACK);
          set_color(parent, Color::BLACK);
          set_color(grandparent, Color::RED);
          node = grandparent;
        } else {
          // Rebalancing necessary
          if (node == parent->right) {
            rotate_left(parent);
            node = parent;
            parent = node->parent;
          }
          rotate_right(grandparent);
          std::swap(parent->color, grandparent->color);
          node = parent;
        }
      } else {
        Node *uncle = grandparent->left;

        if (get_color(uncle) == Color::RED) {
          // Recoloring possible
          set_color(uncle, Color::BLACK);
          set_color(parent, Color::BLACK);
          set_color(grandparent, Color::RED);
          node = grandparent;
        } else {
          // Rebalancing necessary
          if (node == parent->left) {
            rotate_right(parent);
            node = parent;
            parent = node->parent;
          }
          rotate_left(grandparent);
          std::swap(parent->color, grandparent->color);
          node = parent;
        }
      }
    }
    set_color(root, Color::BLACK);
  }

  /**
   * @brief Check invariant and rebalance if necessary after deletion.
   *
   * @param node The node
   */
  inline void rebalance_after_deletion(Node *node) {
    if (node == nullptr) {
      return;
    }
    if (node == root) {
      root = nullptr;
      return;
    }

    if (get_color(node) == Color::RED || get_color(node->left) == Color::RED ||
        get_color(node->right) == Color::RED) {
      // Recoloring sufficient if any related nodes are red
      Node *child = node->left != nullptr ? node->left : node->right;

      if (node == node->parent->left) {
        node->parent->left = child;
        if (child != nullptr) child->parent = node->parent;
        set_color(child, Color::BLACK);
        delete node;
      } else {
        node->parent->right = child;
        if (child != nullptr) child->parent = node->parent;
        set_color(child, Color::BLACK);
        delete node;
      }
    } else {
      // Else found double black which needs rebalancing
      Node *sibling = nullptr;
      Node *parent = nullptr;
      Node *ptr = node;
      set_color(ptr, Color::DOUBLE_BLACK);

      // Fix all double black occurrences
      while (ptr != root && get_color(ptr) == Color::DOUBLE_BLACK) {
        parent = ptr->parent;
        if (ptr == parent->left) {
          sibling = parent->right;

          if (get_color(sibling) == Color::RED) {
            set_color(sibling, Color::BLACK);
            set_color(parent, Color::RED);
            rotate_left(parent);
          } else {
            if (get_color(sibling->left) == Color::BLACK &&
                get_color(sibling->right) == Color::BLACK) {
              set_color(sibling, Color::RED);
              if (get_color(parent) == Color::RED) {
                set_color(parent, Color::BLACK);
              } else {
                set_color(parent, Color::DOUBLE_BLACK);
              }
              ptr = parent;
            } else {
              if (get_color(sibling->right) == Color::BLACK) {
                set_color(sibling->left, Color::BLACK);
                set_color(sibling, Color::RED);
                rotate_right(sibling);
                sibling = parent->right;
              }
              set_color(sibling, parent->color);
              set_color(parent, Color::BLACK);
              set_color(sibling->right, Color::BLACK);
              rotate_left(parent);
              break;
            }
          }
        } else {
          sibling = parent->left;

          if (get_color(sibling) == Color::RED) {
            set_color(sibling, Color::BLACK);
            set_color(parent, Color::RED);
            rotate_right(parent);
          } else {
            if (get_color(sibling->left) == Color::BLACK &&
                get_color(sibling->right) == Color::BLACK) {
              set_color(sibling, Color::RED);
              if (get_color(parent) == Color::RED) {
                set_color(parent, Color::BLACK);
              } else {
                set_color(parent, Color::DOUBLE_BLACK);
              }
              ptr = parent;
            } else {
              if (get_color(sibling->left) == Color::BLACK) {
                set_color(sibling->right, Color::BLACK);
                set_color(sibling, Color::RED);
                rotate_left(sibling);
                sibling = parent->left;
              }
              set_color(sibling, parent->color);
              set_color(parent, Color::BLACK);
              set_color(sibling->left, Color::BLACK);
              rotate_right(parent);
              break;
            }
          }
        }
      }

      // Completed rebalancing, node can be safely deleted
      if (node == node->parent->left) {
        node->parent->left = nullptr;
      } else {
        node->parent->right = nullptr;
      }
      delete node;
      set_color(root, Color::BLACK);
    }
  }

  /**
   * @brief Recursively deconstruct object.
   *
   * @param node The starting node.
   */
  void delete_recursive(Node *node) {
    if (node) {
      delete_recursive(node->left);
      delete_recursive(node->right);
      if (node->leaf_data) {
        delete node->leaf_data;
      }
      delete node;
    }
  }

  /**
   * @brief Recursively access block with desired bit.
   *
   * @param node The node where to start.
   * @param i The bit position to find.
   * @return true if bit is set.
   */
  bool access_bit(Node *node, SizeType i) const {
    if (node->leaf_data) {
      // Base case: If in leaf, access bit
      return (*node->leaf_data)[i];
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return access_bit(node->right, i - node->num_bits_left_tree);
    } else {
      // Index in left subtree
      return access_bit(node->left, i);
    }
  }

  /**
   * @brief Recursively access block with desired bit.
   *
   * @param node The node where to start.
   * @param i The bit position to find.
   * @return true if there is one more one.
   */
  bool set_bit(Node *node, SizeType i, bool value) {
    if (node->leaf_data) {
      // Base case: If in leaf, access bit
      const bool one_more_one = value ? !((*node->leaf_data)[i]) : false;
      node->leaf_data->set(i, value);
      return one_more_one;
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return set_bit(node->right, i - node->num_bits_left_tree, value);
    } else {
      // Index in left subtree
      const bool one_more_one = set_bit(node->left, i, value);
      if (one_more_one) ++node->ones_in_left_tree;
      return one_more_one;
    }
  }

  /**
   * @brief Recursively access block with desired bit and flip it.
   *
   * @param node The node where to start.
   * @param i The bit position to flip.
   * @return true if bit is set afterwards.
   */
  bool flip_bit(Node *node, SizeType i) {
    if (node->leaf_data) {
      // Base case: If in leaf, flip bit
      node->leaf_data->flip(i);
      return (*node->leaf_data)[i];
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return flip_bit(node->right, i - node->num_bits_left_tree);
    } else {
      // Index in left subtree
      const bool flipped_to_one = flip_bit(node->left, i);
      if (flipped_to_one) {
        ++node->num_bits_left_tree;
        ++node->ones_in_left_tree;
      } else {
        --node->num_bits_left_tree;
        --node->ones_in_left_tree;
      }
      return flipped_to_one;
    }
  }

  /**
   * @brief Recursively computes the number of ones/zeros until position i.
   *
   * @param node The node where to start.
   * @param rank_one Whether to count ones.
   * @param i The position until to count (exclusive).
   * @param acc Accumulator of intermediate rank results in recursion.
   * @return The rank.
   */
  SizeType rank_at_node(Node *node, bool rank_one, SizeType i,
                        SizeType acc) const {
    if (node->leaf_data) {
      // Base case: If in leaf, return rank in leaf
      return acc + (rank_one ? node->leaf_data->rank_one(i)
                             : node->leaf_data->rank_zero(i));
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return rank_at_node(node->right, rank_one, i - node->num_bits_left_tree,
                          acc + (rank_one ? node->ones_in_left_tree
                                          : node->num_bits_left_tree -
                                                node->ones_in_left_tree));
    } else {
      // Index in left subtree
      return rank_at_node(node->left, rank_one, i, acc);
    }
  }

  /**
   * @brief Select position of i-th one/zero under starting node.
   *
   * @param node The starting node.
   * @param select_one Whether to select ones.
   * @param i The i-th one/zero to select.
   * @param acc Accumulator of intermediate select results in recursion.
   * @return The select result.
   */
  SizeType select_at_node(Node *node, bool select_one, SizeType i,
                          SizeType acc) const {
    if (node->leaf_data) {
      // Base case: If in leaf, return rank in leaf
      return acc + (select_one ? node->leaf_data->select_one(i)
                               : node->leaf_data->select_zero(i));
    } else {
      const SizeType relevant_bits_in_left_tree =
          (select_one ? node->ones_in_left_tree
                      : node->num_bits_left_tree - node->ones_in_left_tree);
      if (relevant_bits_in_left_tree < i) {
        // Index in right subtree
        return select_at_node(node->right, select_one,
                              i - relevant_bits_in_left_tree,
                              acc + node->num_bits_left_tree);
      } else {
        // Index in left subtree
        return select_at_node(node->left, select_one, i, acc);
      }
    }
  }

  /**
   * @brief Inserts a value at the given position under a given node.
   *
   * @param root The starting node.
   * @param i The position to insert.
   * @param value The value to insert.
   */
  void insert_at_node(Node *node, SizeType i, bool value) {
    if (node->leaf_data) {
      // Base case: If in leaf, insert bit
      auto *left_leaf = node->leaf_data;
      left_leaf->insert(i, value);

      if (left_leaf->size_in_blocks() >= MaxLeafSizeBlocks) {
        // Split if necessary
        auto *right_leaf = left_leaf->split();

        // Leaf node becomes inner node with two children
        node->leaf_data = nullptr;
        node->color = Color::RED;
        node->num_bits_left_tree = left_leaf->size_in_bits();
        node->ones_in_left_tree = left_leaf->num_ones();

        // Left child
        node->left = new Node();
        node->left->color = Color::BLACK;
        node->left->parent = node;
        node->left->leaf_data = left_leaf;

        // Right child
        node->right = new Node();
        node->right->color = Color::BLACK;
        node->right->parent = node;
        node->right->leaf_data = right_leaf;

        // Rebalance if necessary
        rebalance_after_insertion(node);
      }
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      insert_at_node(node->right, i - node->num_bits_left_tree, value);
    } else {
      // Index in left subtree
      ++node->num_bits_left_tree;
      if (value) ++node->ones_in_left_tree;
      insert_at_node(node->left, i, value);
    }
  }

  /**
   * @brief Delete bit at position under node.
   *
   * @param node The starting node.
   * @param i The position.
   * @return true if deleted a one.
   */
  bool delete_at_node(Node *node, SizeType i) {
    if (node->leaf_data) {
      // Base case: If in leaf, delete bit
      auto *left_leaf = node->leaf_data;
      const bool deleted_one = (*left_leaf)[i];
      left_leaf->delete_elem(i);

      if (node != root && left_leaf->size_in_blocks() <= MinLeafSizeBlocks) {
        // Delete leaf and merge with other
        // Node *parent = node->parent;
        // SizeType num_bits = left_leaf->size_in_bits();
        // SizeType num_ones = left_leaf->size_in_bits();
        // if (parent->left == node) {
        //   parent->left = nullptr;
        // } else {
        //   parent->right = nullptr;
        // }
        // delete node;
        // rebalance_after_deletion(parent);

        // Rebalance if necessary
        // TODO
      }

      return deleted_one;
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return delete_at_node(node->right, i - node->num_bits_left_tree);
    } else {
      // Index in left subtree
      const bool deleted_one = delete_at_node(node->left, i);
      --node->num_bits_left_tree;
      if (deleted_one) --node->ones_in_left_tree;
      return deleted_one;
    }
  }

  /**
   * @brief Retrieves the tree structure under a node.
   *
   * @param node The node.
   * @param tree_structure The tree structure string.
   */
  void get_tree_structure_at_node(Node *node,
                                  std::ostringstream &tree_structure) {
    if (node) {
      if (node->leaf_data) {
        tree_structure << "(" << node->leaf_data->size_in_bits() << " "
                       << node->leaf_data->num_ones() << ")";
      } else {
        tree_structure << "(" << node->num_bits_left_tree << " "
                       << node->ones_in_left_tree << " ";
        get_tree_structure_at_node(node->left, tree_structure);
        get_tree_structure_at_node(node->right, tree_structure);
        tree_structure << ")";
      }
    }
  }

 public:
  /**
   * @brief Construct a new empty dynamic bitvector.
   */
  DynamicBitVector() : root(new Node()), current_size(0) {
    root->leaf_data = new SimpleBitVector<BlockType, SizeType>(0);
  }

  DynamicBitVector(DynamicBitVector &) = delete;
  DynamicBitVector(DynamicBitVector &&) = delete;

  /**
   * @brief Destroy the dynamic bitvector.
   */
  ~DynamicBitVector() { delete_recursive(root); }

  /**
   * @brief Current number of bits.
   *
   * @return The bitvector's size.
   */
  SizeType size() const { return current_size; }

  /**
   * @brief Access the bit at position i.
   *
   * @param i The position to access.
   * @return true if bit is set.
   * @return false otherwise.
   */
  bool operator[](SizeType i) const { return access_bit(root, i); }

  /**
   * @brief Sets the bit at position i.
   *
   * @param i The position.
   * @param value The value.
   */
  void set(SizeType i, bool value) { set_bit(root, i, value); }

  /**
   * @brief Flips the bit at position i.
   *
   * @param i The position to flip.
   */
  void flip(SizeType i) { flip_bit(root, i); }

  /**
   * @brief Number of ones/zeros until position i (exclusive).
   *
   * @param rank_one Whether to count ones.
   * @param i The position to count up to.
   * @return The rank.
   */
  SizeType rank(bool rank_one, SizeType i) const {
    return rank_at_node(root, rank_one, i, 0);
  }

  /**
   * @brief Select the i-th one/zero.
   *
   * @param select_one Whether to select ones.
   * @param i The i-th relevant bit.
   * @return The select result.
   */
  SizeType select(bool select_one, SizeType i) const {
    return select_at_node(root, select_one, i, 0);
  }

  /**
   * @brief Inserts a given value at position i.
   *
   * @param i The position to insert.
   * @param value The value to insert.
   */
  void insert(SizeType i, bool value) {
    if (i <= current_size) {
      ++current_size;
      insert_at_node(root, i, value);
    }
  }

  /**
   * @brief Deletes the bit at position i.
   *
   * @param i The position to delete.
   */
  void delete_element(SizeType i) {
    if (i < current_size) {
      --current_size;
      delete_at_node(root, i);
    }
  }

  /**
   * @brief Returns the tree structure for debugging purposes.
   *
   * @return The tree structure.
   */
  std::string get_tree_structure() {
    std::ostringstream tree_structure;
    get_tree_structure_at_node(root, tree_structure);
    return tree_structure.str();
  }
};

}  // namespace bv
}  // namespace ads

#endif
