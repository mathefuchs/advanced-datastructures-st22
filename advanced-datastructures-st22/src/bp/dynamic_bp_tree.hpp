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
 * @brief Forward declaration.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType BlocksPerChunk>
struct MinExcessBlockData;

/**
 * @brief Additional data to store in the leafs of the dynamic bitvector.
 */
struct MinExcessLeafData {};

/**
 * @brief Additional data that needs to be maintained per node for (minimum)
 * excess queries.
 *
 * @tparam BlockType The bit vector block type, e.g., uint64_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 * @tparam BlocksPerChunk The number of blocks per min-excess-chunk, e.g., w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType BlocksPerChunk>
struct MinExcessNodeData {
  using ExcessData =
      MinExcessNodeData<BlockType, SizeType, SignedIntType, BlocksPerChunk>;
  using SimpleBitVector = bv::SimpleBitVector<
      BlockType, SizeType, SignedIntType,
      MinExcessBlockData<BlockType, SizeType, SignedIntType, BlocksPerChunk>,
      true>;

  static constexpr SizeType BLOCK_SIZE = SimpleBitVector::BLOCK_SIZE;
  static constexpr SizeType BLOCKS_PER_CHUNK = BlocksPerChunk;

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
   * @param blocks The blocks to compute the excess over.
   * @param chunk_idx The index of the chunk to update.
   * @param current_size The current size in bits.
   * @return The segment's excess.
   */
  static inline ExcessData compute_block_excess(
      const std::vector<BlockType> &blocks, SizeType chunk_idx,
      SizeType current_size) {
    ExcessData excess;

    const SizeType start_block = chunk_idx * BLOCKS_PER_CHUNK;
    const SizeType end_block = (chunk_idx + 1) * BLOCKS_PER_CHUNK;
    SizeType remaining_bits = current_size - start_block * BLOCK_SIZE;

    for (SizeType b = start_block; b < end_block && b < blocks.size(); ++b) {
      for (SizeType i = 0; i < BLOCK_SIZE && i < remaining_bits; ++i) {
        if (SimpleBitVector::access_bit(blocks[b], i) == LEFT) {
          ++excess.block_excess;
        } else {
          --excess.block_excess;
        }

        // Update min excess
        if (excess.block_excess < excess.min_excess_in_block) {
          excess.min_excess_in_block = excess.block_excess;
        }
      }
      remaining_bits -= BLOCK_SIZE;
    }

    return excess;
  }
};

/**
 * @brief Additional data to store in the leafs of the dynamic bitvector.
 *
 * @tparam BlockType The bit vector block type, e.g., uint64_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 * @tparam BlocksPerChunk The number of blocks per min-excess-chunk, e.g., w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType BlocksPerChunk>
class MinExcessBlockData {
 public:
  using MinExcessData =
      MinExcessNodeData<BlockType, SizeType, SignedIntType, BlocksPerChunk>;

  /**
   * @brief The number of blocks per min-excess-chunk.
   */
  static constexpr SizeType BLOCKS_PER_CHUNK = BlocksPerChunk;

  static constexpr bool LEFT = MinExcessData::LEFT;
  static constexpr bool RIGHT = MinExcessData::RIGHT;

  /**
   * @brief Excess information per block to have Theta(w) operations.
   */
  std::vector<MinExcessData> chunk_array;

  explicit MinExcessBlockData(SizeType initial_block_size)
      : chunk_array(initial_block_size == 0
                        ? 0
                        : (initial_block_size - 1) / BLOCKS_PER_CHUNK + 1) {}

  /**
   * @brief Returns the space used by the additional excess information.
   *
   * @return The space used.
   */
  SizeType space_used() const {
    return chunk_array.size() * sizeof(MinExcessData) * 8ull;
  }

  /**
   * @brief Compute the complete excess over all chunks.
   *
   * @return The excess information.
   */
  MinExcessData compute() const {
    MinExcessData excess_data;
    for (SizeType c = 0; c < chunk_array.size(); ++c) {
      if (excess_data.block_excess + chunk_array[c].min_excess_in_block <
          excess_data.min_excess_in_block) {
        excess_data.min_excess_in_block =
            excess_data.block_excess + chunk_array[c].min_excess_in_block;
      }

      excess_data.block_excess += chunk_array[c].block_excess;
    }
    return excess_data;
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
 * @tparam BlocksPerChunk The number of blocks per min-excess-chunk, e.g., w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks, SizeType BlocksPerChunk>
using DynamicMinExcessBitVector = bv::DynamicBitVector<
    BlockType, SizeType, SignedIntType, MinLeafSizeBlocks,
    InitialLeafSizeBlocks, MaxLeafSizeBlocks,
    MinExcessNodeData<BlockType, SizeType, SignedIntType, BlocksPerChunk>,
    MinExcessLeafData,
    MinExcessBlockData<BlockType, SizeType, SignedIntType, BlocksPerChunk>,
    true>;

/**
 * @brief Dynamic succinct tree datastructure.
 *
 * @tparam BlockType The bit vector block type, e.g., uint<w>_t.
 * @tparam SizeType The bit vector size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type, e.g., int64_t.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks, e.g., w / 2.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks, e.g., w.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks, e.g., 2 * w.
 * @tparam BlocksPerChunk The number of blocks per min-excess-chunk, e.g., w.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks, SizeType BlocksPerChunk>
class DynamicBPTree {
 private:
  static_assert(2 * MinLeafSizeBlocks <= InitialLeafSizeBlocks,
                "Leaf sizes invalid.");
  static_assert(InitialLeafSizeBlocks <= 2 * MaxLeafSizeBlocks,
                "Leaf sizes invalid.");
  static_assert(MinLeafSizeBlocks % BlocksPerChunk == 0,
                "Blocks have to align with chunks.");
  static_assert(InitialLeafSizeBlocks % BlocksPerChunk == 0,
                "Blocks have to align with chunks.");
  static_assert(MaxLeafSizeBlocks % BlocksPerChunk == 0,
                "Blocks have to align with chunks.");

  static constexpr bool LEFT =
      MinExcessNodeData<BlockType, SizeType, SignedIntType,
                        BlocksPerChunk>::LEFT;
  static constexpr bool RIGHT =
      MinExcessNodeData<BlockType, SizeType, SignedIntType,
                        BlocksPerChunk>::RIGHT;
  using SimpleBitVector = bv::SimpleBitVector<
      BlockType, SizeType, SignedIntType,
      MinExcessBlockData<BlockType, SizeType, SignedIntType, BlocksPerChunk>,
      true>;
  using BitVector =
      DynamicMinExcessBitVector<BlockType, SizeType, SignedIntType,
                                MinLeafSizeBlocks, InitialLeafSizeBlocks,
                                MaxLeafSizeBlocks, BlocksPerChunk>;

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
    SimpleBitVector bv(2);
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
   * @param i The i-th child (one-based index).
   * @return The i-th child.
   */
  SizeType i_th_child(SizeType node, SizeType i) const {
    SizeType child_idx = 1;
    SizeType current_child = node + 1;
    while (child_idx != i) {
      current_child = bitvector->forward_search(current_child, 0).position + 1;
      ++child_idx;
    }
    return current_child;
  }

  /**
   * @brief Returns the parent node.
   *
   * @param node The node.
   * @return The parent node.
   */
  SizeType parent(SizeType node) const {
    return bitvector->backward_search(node, -1).position;
  }

  /**
   * @brief The size of the subtree under the given node.
   *
   * @param node The node.
   * @return The subtree size.
   */
  SizeType subtree_size(SizeType node) const {
    return (bitvector->forward_search(node, 0).position - node + 1) / 2;
  }

  /**
   * @brief Deletes the node making all its children children of its parent.
   *
   * @param node The node to delete.
   */
  void delete_node(SizeType node) {
    const auto right_parenthesis = bitvector->forward_search(node, 0).position;
    bitvector->delete_element(right_parenthesis);
    bitvector->delete_element(node);
  }

  /**
   * @brief Inserts a node under the given parent node as i-th child making the
   * old i-th to (i+k-1)-th children child of the inserted node.
   *
   * @param node The node under which to insert.
   * @param i As which child to insert.
   * @param k How many existing children to move under the inserted node.
   */
  void insert_node(SizeType node, SizeType i, SizeType k) {
    // Find the i-th child
    SizeType child_idx = 1;
    SizeType current_child = node + 1;
    while (child_idx != i) {
      current_child = bitvector->forward_search(current_child, 0).position + 1;
      ++child_idx;
    }
    const SizeType insert_pos = current_child;

    // Find the ending of the child i+k
    while (child_idx != i + k) {
      current_child = bitvector->forward_search(current_child, 0).position + 1;
      ++child_idx;
    }

    // Insert pair of parentheses
    bitvector->insert(insert_pos, LEFT);
    bitvector->insert(current_child + 1, RIGHT);
  }

  /**
   * @brief The string BP representation.
   *
   * @return The string BP representation.
   */
  std::string get_bp_representation() const {
    std::ostringstream oss;
    for (SizeType i = 0; i < bitvector->size(); ++i) {
      if ((*bitvector)[i] == LEFT) {
        oss << "(";
      } else {
        oss << ")";
      }
    }
    return oss.str();
  }

  /**
   * @brief Returns the space used.
   *
   * @return The space used.
   */
  SizeType space_used() const { return bitvector->space_used(); }
};

}  // namespace bp
}  // namespace ads

#endif
