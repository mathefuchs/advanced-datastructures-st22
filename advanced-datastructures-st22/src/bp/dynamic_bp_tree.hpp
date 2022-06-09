#ifndef DYNAMIC_BP_TREE_HPP
#define DYNAMIC_BP_TREE_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

#include "bv/dynamic_bitvector.hpp"
#include "bv/simple_bitvector.hpp"

namespace ads {
namespace bp {

/**
 * @brief Additional data to store in the leafs of the dynamic bitvector.
 *
 * @tparam BlockType The bit vector block type, e.g., uint64_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 */
template <class BlockType, class SizeType>
struct MinExcessLeafData {};

/**
 * @brief Additional data that needs to be maintained per node for (minimum)
 * excess queries.
 *
 * @tparam BlockType The bit vector block type, e.g., uint64_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 */
template <class BlockType, class SizeType, class SignedIntType>
struct MinExcessNodeData {
  using ExcessData = MinExcessNodeData<BlockType, SizeType, SignedIntType>;
  using BitVector = bv::SimpleBitVector<BlockType, SizeType>;

  /**
   * @brief Constant for left parenthesis.
   */
  static constexpr bool LEFT = false;

  /**
   * @brief Constant for right parenthesis.
   */
  static constexpr bool RIGHT = true;

  SignedIntType block_excess = 0;
  SignedIntType min_excess_in_block = 2;

  /**
   * @brief Computes the block's excess.
   *
   * @param block The block.
   * @return The excess.
   */
  static inline ExcessData compute_block_excess(BlockType block) {
    ExcessData excess;
    for (SizeType i = 0; i < BitVector::BLOCK_SIZE; ++i) {
      if (BitVector::access_bit(block, i) == LEFT) {
        ++excess.block_excess;
      } else {
        --excess.block_excess;
      }

      // Update min excess
      if (excess.block_excess < excess.min_excess_in_block) {
        excess.min_excess_in_block = excess.block_excess;
      }
    }
    return excess;
  }
};

/**
 * @brief A dynamic bitvector supporting (minimum excess queries).
 *
 * @tparam BlockType The bit vector block type, e.g., uint<w>_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks, e.g., w / 2.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks, e.g., w.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks, e.g., 2 * w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks>
using DynamicMinExcessBitVector =
    bv::DynamicBitVector<BlockType, SizeType, MinLeafSizeBlocks,
                         InitialLeafSizeBlocks, MaxLeafSizeBlocks,
                         MinExcessNodeData<BlockType, SizeType, SignedIntType>,
                         MinExcessLeafData<BlockType, SizeType>, true>;

/**
 * @brief Dynamic succinct tree datastructure.
 *
 * @tparam BlockType The bit vector block type, e.g., uint<w>_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks, e.g., w / 2.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks, e.g., w.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks, e.g., 2 * w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks>
class DynamicBPTree {
 private:
  static constexpr bool LEFT =
      MinExcessNodeData<BlockType, SizeType, SignedIntType>::LEFT;
  static constexpr bool RIGHT =
      MinExcessNodeData<BlockType, SizeType, SignedIntType>::RIGHT;
  using BitVector =
      DynamicMinExcessBitVector<BlockType, SizeType, SignedIntType,
                                MinLeafSizeBlocks, InitialLeafSizeBlocks,
                                MaxLeafSizeBlocks>;

  /**
   * @brief Underlying bitvector datastructure.
   */
  BitVector *bitvector;

 public:
  /**
   * @brief Constructs a new dynamic BP tree object.
   */
  DynamicBPTree() {
    // BP representation of tree with only the root node
    bv::SimpleBitVector<BlockType, SizeType> bv(2);
    bv.set(0, LEFT);
    bv.set(1, RIGHT);
    bitvector = new BitVector(bv);
  }

  DynamicBPTree(const DynamicBPTree &) = delete;
  DynamicBPTree &operator=(const DynamicBPTree &) = delete;
  DynamicBPTree(DynamicBPTree &&) = delete;

  /**
   * @brief Destroys the dynamic BP tree.
   */
  ~DynamicBPTree() { delete bitvector; }

  /**
   * @brief Returns the i-th child of the given node.
   *
   * @param node The node.
   * @return The i-th child.
   */
  SizeType i_th_child(SizeType node) const;

  /**
   * @brief Returns the parent node.
   *
   * @param node The node.
   * @return The parent node.
   */
  SizeType parent(SizeType node) const;

  /**
   * @brief The size of the subtree under the given node.
   *
   * @param node The node.
   * @return The subtree size.
   */
  SizeType subtree_size(SizeType node) const;

  /**
   * @brief Deletes the node making all its children children of its parent.
   *
   * @param node The node to delete.
   */
  void delete_node(SizeType node);

  /**
   * @brief Inserts a node under the given parent node as i-th child making the
   * old i-th to (i+k-1)-th child child of the inserted node.
   *
   * @param node The node under which to insert.
   * @param i As which child to insert.
   * @param k How many existing children to move under the inserted node.
   */
  void insert_node(SizeType node, SizeType i, SizeType k);
};

}  // namespace bp
}  // namespace ads

#endif
