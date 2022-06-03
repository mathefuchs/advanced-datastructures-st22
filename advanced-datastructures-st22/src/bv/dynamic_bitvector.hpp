#ifndef DYNAMIC_BITVECTOR_HPP
#define DYNAMIC_BITVECTOR_HPP

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
  static_assert(2 * MinLeafSizeBlocks <= InitialLeafSizeBlocks,
                "Leaf sizes invalid.");
  static_assert(InitialLeafSizeBlocks <= 2 * MaxLeafSizeBlocks,
                "Leaf sizes invalid.");

  /**
   * @brief Short-hand name for leafs.
   */
  using Leaf = SimpleBitVector<BlockType, SizeType>;

  /**
   * @brief Colors of the nodes.
   */
  enum class Color { RED, BLACK, DOUBLE_BLACK };

  /**
   * @brief Helper enum indicating whether deletion was successful or an
   * underflow has occurred.
   */
  enum class LeafDeletion { DELETED_ZERO, DELETED_ONE, UNDERFLOW };

  /**
   * @brief Helper enum to determine whether a change indicates a change in
   * related counters.
   */
  enum class BitChangeResult { ONE_MORE_ONE, ONE_LESS_ONE, NO_CHANGE };

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
    Leaf *leaf_data = nullptr;
  };

  static constexpr size_t NODE_SIZE = sizeof(Node) * 8ull;

  Node *root;
  SizeType current_size;
  SizeType total_ones;

  /**
   * @brief Get the color of the node.
   *
   * @param node The node.
   * @return The node's color or black if null.
   */
  inline Color get_color(Node *node) const {
    if (!node || node->leaf_data) {
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
    if (node) {
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
    if (node->right) {
      node->right->parent = node;
    }
    right_child->parent = node->parent;

    if (!node->parent) {
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
    if (node->left) {
      node->left->parent = node;
    }
    left_child->parent = node->parent;

    if (!node->parent) {
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
   * @param node The node to start rebalancing at.
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
   * @param node The node to rebalance
   * @param deleted_bit Whether deleted a zero or one.
   */
  inline void rebalance_after_deletion(Node *node, LeafDeletion deleted_bit) {
    if (node == root) {
      // Delete root and move child up
      if (root->left) {
        if (root->left->leaf_data) {
          // Root becomes leaf
          root->leaf_data = root->left->leaf_data;
          delete root->left;
          delete root->right;
          root->left = nullptr;
          root->right = nullptr;
        } else {
          // Left child is new root
          root->left->parent = nullptr;
          Node *new_root = root->left;
          delete root;
          root = new_root;
          set_color(root, Color::BLACK);
        }
      } else {
        if (root->right->leaf_data) {
          // Root becomes leaf
          root->leaf_data = root->right->leaf_data;
          delete root->left;
          delete root->right;
          root->left = nullptr;
          root->right = nullptr;
        } else {
          // Right child is new root
          root->right->parent = nullptr;
          Node *new_root = root->right;
          delete root;
          root = new_root;
          set_color(root, Color::BLACK);
        }
      }
    } else if (get_color(node) == Color::RED ||
               get_color(node->left) == Color::RED ||
               get_color(node->right) == Color::RED) {
      // Recoloring sufficient if any related nodes are red
      Node *child = node->left ? node->left : node->right;

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
            if (deleted_bit == LeafDeletion::DELETED_ONE) {
              --parent->parent->ones_in_left_tree;
            }
            --parent->parent->num_bits_left_tree;
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
              if (deleted_bit == LeafDeletion::DELETED_ONE) {
                --parent->parent->ones_in_left_tree;
              }
              --parent->parent->num_bits_left_tree;
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
      Node *leaf = node->left ? node->left : node->right;
      leaf->parent = node->parent;
      if (node == node->parent->left) {
        node->parent->left = leaf;
      } else {
        node->parent->right = leaf;
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
   * @return Whether the number of ones is changed.
   */
  BitChangeResult set_bit(Node *node, SizeType i, bool value) {
    if (node->leaf_data) {
      // Base case: If in leaf, access bit
      const bool prev_value = (*node->leaf_data)[i];
      node->leaf_data->set(i, value);
      if (prev_value && !value) {
        return BitChangeResult::ONE_LESS_ONE;
      } else if (!prev_value && value) {
        return BitChangeResult::ONE_MORE_ONE;
      } else {
        return BitChangeResult::NO_CHANGE;
      }
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      return set_bit(node->right, i - node->num_bits_left_tree, value);
    } else {
      // Index in left subtree
      const auto change_in_ones = set_bit(node->left, i, value);
      switch (change_in_ones) {
        case BitChangeResult::ONE_LESS_ONE:
          --node->ones_in_left_tree;
          break;
        case BitChangeResult::ONE_MORE_ONE:
          ++node->ones_in_left_tree;
          break;
        case BitChangeResult::NO_CHANGE:
        default:
          break;
      }
      return change_in_ones;
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
        ++node->ones_in_left_tree;
      } else {
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
        node->num_bits_left_tree = left_leaf->size();
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
   * @brief Moves the source leaf's bits to the specified position.
   *
   * @param node The starting node to search for the correct position.
   * @param i The position to insert.
   * @param src The source leaf node.
   * @param num_ones_leaf The number of ones in the bitvector.
   * @param insert_back Whether to insert at the back (true) or front (false).
   */
  void move_to_leaf(Node *node, SizeType i, Node *src, SizeType num_ones_leaf,
                    bool insert_back) {
    if (node->leaf_data) {
      // Base case
      if (insert_back) {
        // Move right leaf into left subtree
        node->leaf_data->copy_to_back(*src->leaf_data);
        delete src->leaf_data;
        src->parent->right = nullptr;
        delete src;
      } else {
        // Move left leaf into right subtree
        src->leaf_data->copy_to_back(*node->leaf_data);
        delete node->leaf_data;
        node->leaf_data = src->leaf_data;
        src->parent->left = nullptr;
        delete src;
      }
    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      move_to_leaf(node->right, i - node->num_bits_left_tree, src,
                   num_ones_leaf, insert_back);
    } else {
      // Index in left subtree
      node->num_bits_left_tree += src->leaf_data->size();
      node->ones_in_left_tree += num_ones_leaf;
      move_to_leaf(node->left, i, src, num_ones_leaf, insert_back);
    }
  }

  /**
   * @brief Delete bit at position under node.
   *
   * @param node The starting node.
   * @param i The position.
   * @param num_bits The number of bits under the node.
   * @param ones The number of ones under the node.
   * @param allow_underflow Whether to allow for underflows in deletion.
   * @return Whether the deletion in the leaf was successful.
   */
  LeafDeletion delete_at_node(Node *node, SizeType i, SizeType num_bits,
                              SizeType ones, bool allow_underflow) {
    if (node->leaf_data) {
      // Base case: If in leaf, delete bit (if not an underflow)
      if (!allow_underflow && node != root &&
          node->leaf_data->size_in_blocks() <= MinLeafSizeBlocks) {
        // Underflow, do not delete and delegate to parent to merge nodes
        return LeafDeletion::UNDERFLOW;
      } else {
        // No underflow
        const bool deleted_one = (*node->leaf_data)[i];
        node->leaf_data->delete_element(i);
        return deleted_one ? LeafDeletion::DELETED_ONE
                           : LeafDeletion::DELETED_ZERO;
      }

    } else if (node->num_bits_left_tree <= i) {
      // Index in right subtree
      const auto deletion_successful =
          delete_at_node(node->right, i - node->num_bits_left_tree,
                         num_bits - node->num_bits_left_tree,
                         ones - node->ones_in_left_tree, allow_underflow);
      if (deletion_successful == LeafDeletion::UNDERFLOW) {
        return LeafDeletion::UNDERFLOW;
      }
      if (deletion_successful == LeafDeletion::DELETED_ONE) {
        --ones;
      }
      if (num_bits - node->num_bits_left_tree ==
          MinLeafSizeBlocks * Leaf::BLOCK_SIZE) {
        const auto stealing_attempt =
            delete_at_node(node->left, node->num_bits_left_tree - 1,
                           node->num_bits_left_tree, 0, false);
        switch (stealing_attempt) {
          case LeafDeletion::UNDERFLOW:
            // Move leaf node->right to right-most position in subtree under
            // node->left
            move_to_leaf(node->left, node->num_bits_left_tree, node->right,
                         ones - node->ones_in_left_tree, true);
            rebalance_after_deletion(node, deletion_successful);
            return deletion_successful;
          case LeafDeletion::DELETED_ZERO:
            insert_at_node(node->right, 0, false);
            --node->num_bits_left_tree;
            break;
          case LeafDeletion::DELETED_ONE:
            insert_at_node(node->right, 0, true);
            --node->num_bits_left_tree;
            --node->ones_in_left_tree;
            break;
        }
      }
      return deletion_successful;

    } else {
      // Index in left subtree
      const auto deletion_successful =
          delete_at_node(node->left, i, node->num_bits_left_tree,
                         node->ones_in_left_tree, allow_underflow);
      if (deletion_successful == LeafDeletion::UNDERFLOW) {
        return LeafDeletion::UNDERFLOW;
      }
      if (deletion_successful == LeafDeletion::DELETED_ONE) {
        --node->ones_in_left_tree;
      }
      if (node->num_bits_left_tree == MinLeafSizeBlocks * Leaf::BLOCK_SIZE) {
        // Correct underflow
        const auto stealing_attempt = delete_at_node(
            node->right, 0, num_bits - node->num_bits_left_tree, 0, false);
        switch (stealing_attempt) {
          case LeafDeletion::UNDERFLOW:
            // Move leaf node->left to left-most position in subtree under
            // node->right
            move_to_leaf(node->right, 0, node->left, node->ones_in_left_tree,
                         false);
            rebalance_after_deletion(node, deletion_successful);
            return deletion_successful;
          case LeafDeletion::DELETED_ZERO:
            insert_at_node(node->left, node->num_bits_left_tree - 1, false);
            break;
          case LeafDeletion::DELETED_ONE:
            insert_at_node(node->left, node->num_bits_left_tree - 1, true);
            ++node->ones_in_left_tree;
            break;
        }
      } else {
        --node->num_bits_left_tree;
      }
      return deletion_successful;
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
        tree_structure << "(" << node->leaf_data->size() << " "
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

  /**
   * @brief Returns the space usage in bits.
   *
   * @return The space usage in bits.
   */
  inline SizeType space_used_at_node(Node *node) const {
    if (node->leaf_data) {
      // Space for leaf block and leaf data
      return NODE_SIZE + node->leaf_data->space_used();
    } else {
      // Space for subtrees and node
      const SizeType left = node->left ? space_used_at_node(node->left) : 0ull;
      const SizeType right =
          node->right ? space_used_at_node(node->right) : 0ull;
      return left + right + NODE_SIZE;
    }
  }

 public:
  /**
   * @brief Construct a new empty dynamic bitvector.
   */
  DynamicBitVector() : root(new Node()), current_size(0), total_ones(0) {
    root->leaf_data = new Leaf(0);
  }

  DynamicBitVector(const DynamicBitVector &) = delete;
  DynamicBitVector &operator=(const DynamicBitVector &) = delete;
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
   * @brief Current number of ones.
   *
   * @return The bitvector's number of ones.
   */
  SizeType num_ones() const { return total_ones; }

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
  void set(SizeType i, bool value) {
    switch (set_bit(root, i, value)) {
      case BitChangeResult::ONE_LESS_ONE:
        --total_ones;
        break;
      case BitChangeResult::ONE_MORE_ONE:
        ++total_ones;
        break;
      case BitChangeResult::NO_CHANGE:
      default:
        break;
    }
  }

  /**
   * @brief Flips the bit at position i.
   *
   * @param i The position to flip.
   */
  void flip(SizeType i) {
    if (flip_bit(root, i)) {
      ++total_ones;
    } else {
      --total_ones;
    }
  }

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
      insert_at_node(root, i, value);
      ++current_size;
      if (value) ++total_ones;
    }
  }

  /**
   * @brief Deletes the bit at position i.
   *
   * @param i The position to delete.
   */
  void delete_element(SizeType i) {
    if (i < current_size) {
      if (delete_at_node(root, i, current_size, total_ones, true) ==
          LeafDeletion::DELETED_ONE) {
        --total_ones;
      }
      --current_size;
    }
  }

  /**
   * @brief Append value at the end.
   *
   * @param value The value to append.
   */
  void push_back(bool value) { insert(current_size, value); }

  /**
   * @brief Pop value from the end.
   */
  void pop_back() { delete_element(current_size - 1); }

  /**
   * @brief Returns the space usage in bits.
   *
   * @return The space usage in bits.
   */
  inline SizeType space_used() const {
    // Size of attributes and the complete tree
    return (2 * sizeof(SizeType) + sizeof(Node *)) * 8ull +
           space_used_at_node(root);
  }

  /**
   * @brief Returns the tree structure for debugging purposes.
   *
   * @return The tree structure.
   */
  std::string get_tree_structure() {
    std::ostringstream tree_structure;
    tree_structure << size() << " " << num_ones() << " ";
    get_tree_structure_at_node(root, tree_structure);
    return tree_structure.str();
  }
};

}  // namespace bv
}  // namespace ads

#endif
