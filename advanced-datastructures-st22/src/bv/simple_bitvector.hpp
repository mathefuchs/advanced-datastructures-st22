#ifndef SIMPLE_BITVECTOR_HPP
#define SIMPLE_BITVECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

#include "meta/empty.hpp"

namespace ads {
namespace bv {

/**
 * @brief Forward declaration.
 */
template <class BlockType, class SizeType, SizeType MinLeafSizeBlocks,
          SizeType InitialLeafSizeBlocks, SizeType MaxLeafSizeBlocks,
          class AdditionalNodeData, class AdditionalLeafData,
          class AdditionalBlockData, bool ExcessQuerySupport>
class DynamicBitVector;

/**
 * @brief Simple Bitvector datastructure.
 *
 * @tparam BlockType The block type, e.g., uint64_t.
 * @tparam SizeType The size type, e.g., size_t.
 */
template <class BlockType, class SizeType = uint64_t,
          class AdditionalBlockData = meta::Empty<SizeType>,
          bool ExcessQuerySupport = false>
class SimpleBitVector {
 private:
  /**
   * @brief Friend class dynamic bitvector for
   * construction out of a simple vector.
   */
  template <class BlockT, class SizeT, SizeT MinLeafSizeBlocks,
            SizeT InitialLeafSizeBlocks, SizeT MaxLeafSizeBlocks,
            class AdditionalNodeData, class AdditionalLeafData,
            class AdditionalBlockDataT, bool ExcessQuerySupportT>
  friend class DynamicBitVector;

  /**
   * @brief The raw block data.
   */
  class BlockData : public AdditionalBlockData {
   private:
    /**
     * @brief Friend class dynamic bitvector for
     * construction out of a simple vector.
     */
    template <class BlockT, class SizeT, SizeT MinLeafSizeBlocks,
              SizeT InitialLeafSizeBlocks, SizeT MaxLeafSizeBlocks,
              class AdditionalNodeData, class AdditionalLeafData,
              class AdditionalBlockDataT, bool ExcessQuerySupportT>
    friend class DynamicBitVector;
    friend class SimpleBitVector;

    std::vector<BlockType> blocks;

   public:
    explicit BlockData(SizeType initial_block_size)
        : AdditionalBlockData(initial_block_size),
          blocks(initial_block_size, 0) {}
  };

  SizeType current_size_bits;
  BlockData data;

  /**
   * @brief Use built-in popcount function with correct width.
   *
   * @param block The block.
   * @return The popcount.
   */
  static constexpr SizeType popcount(BlockType block) {
    if constexpr (sizeof(BlockType) == sizeof(long)) {
      return static_cast<SizeType>(__builtin_popcountl(block));
    } else if constexpr (sizeof(BlockType) == sizeof(long long)) {
      return static_cast<SizeType>(__builtin_popcountll(block));
    } else {
      return static_cast<SizeType>(__builtin_popcount(block));
    }
  }

  /**
   * @brief Gets the number of required blocks.
   *
   * @param num_bits The number of bits.
   * @return The number of blocks.
   */
  static constexpr SizeType get_required_blocks(SizeType num_bits) {
    return (num_bits - 1) / BLOCK_SIZE + 1;
  }

  /**
   * @brief Updates the excess chunk that contains the given bit position.
   *
   * @param i The bit position.
   */
  void update_excess_chunk(SizeType i) {
    const SizeType chunk_idx = i / (BLOCK_SIZE * data.BLOCKS_PER_CHUNK);
    data.chunk_array[chunk_idx] =
        AdditionalBlockData::MinExcessData::compute_block_excess(
            data.blocks, chunk_idx, size());
  }

 public:
  static constexpr SizeType BLOCK_SIZE =
      static_cast<SizeType>(8ull * sizeof(BlockType));

  /**
   * @brief Accesses a bit at the given position.
   *
   * @param block The block.
   * @param i The position.
   * @return true if bit is set.
   */
  static constexpr bool access_bit(BlockType block, SizeType i) {
    return (block >> i) & 1ull;
  }

  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size in bits.
   */
  SimpleBitVector(SizeType initial_size)
      : current_size_bits(initial_size),
        data(initial_size == 0 ? 0 : get_required_blocks(initial_size)) {
    if constexpr (ExcessQuerySupport) {
      // Fill chunk array
      static constexpr SizeType bits_per_chunk =
          AdditionalBlockData::BLOCKS_PER_CHUNK * BLOCK_SIZE;
      SizeType remaining_size = initial_size;
      for (SizeType i = 0; i < data.chunk_array.size(); ++i) {
        data.chunk_array[i].block_excess =
            remaining_size < bits_per_chunk ? remaining_size : bits_per_chunk;
        data.chunk_array[i].min_excess_in_block = 1;
        remaining_size -= bits_per_chunk;
      }
    }
  }

  /**
   * @brief Constructs a new bit vector.
   */
  SimpleBitVector() : SimpleBitVector(0) {}

  /**
   * @brief Copy from existing object.
   *
   * @param other The other bitvector.
   */
  SimpleBitVector(const SimpleBitVector& other)
      : current_size_bits(other.current_size_bits), data(other.data) {}

  /**
   * @brief Returns an accessor to the excess functionality.
   *
   * @return The excess functions.
   */
  const BlockData& excess() { return data; }

  /**
   * @brief Accesses the bit vector at index i.
   *
   * @param i The index to access.
   * @return true if set.
   * @return false otherwise.
   */
  bool operator[](SizeType i) const {
    return (data.blocks[i / BLOCK_SIZE] >> (i % BLOCK_SIZE)) & 1ull;
  }

  /**
   * @brief Sets the i-th element to the given value.
   *
   * @param i The index to set.
   * @param value The value to set it to.
   */
  void set(SizeType i, bool value) {
    if (value) {
      set(i);
    } else {
      reset(i);
    }
  }

  /**
   * @brief Sets the i-th element.
   *
   * @param i The index to set.
   */
  void set(SizeType i) {
    // Update block data
    data.blocks[i / BLOCK_SIZE] |= (1ull << (i % BLOCK_SIZE));

    // Update excess chunk
    if constexpr (ExcessQuerySupport) {
      update_excess_chunk(i);
    }
  }

  /**
   * @brief Resets the i-th element.
   *
   * @param i The index to reset.
   */
  void reset(SizeType i) {
    // Update block data
    data.blocks[i / BLOCK_SIZE] &= ~(1ull << (i % BLOCK_SIZE));

    // Update excess chunk
    if constexpr (ExcessQuerySupport) {
      update_excess_chunk(i);
    }
  }

  /**
   * @brief Returns the size in blocks.
   *
   * @return The size in blocks.
   */
  SizeType size_in_blocks() const {
    return static_cast<SizeType>(data.blocks.size());
  }

  /**
   * @brief Returns the size in bits.
   *
   * @return The size in bits.
   */
  SizeType size() const { return current_size_bits; }

  /**
   * @brief Flips the i-th bit.
   *
   * @param i The bit to flip.
   */
  void flip(SizeType i) {
    // Update block data
    data.blocks[i / BLOCK_SIZE] ^= (1ull << (i % BLOCK_SIZE));

    // Update excess chunk
    if constexpr (ExcessQuerySupport) {
      update_excess_chunk(i);
    }
  }

  /**
   * @brief Inserts an element at position i.
   *
   * @param i The position to insert.
   * @param value The value of the inserted element.
   */
  void insert(SizeType i, bool value) {
    // Update counters
    if (current_size_bits == BLOCK_SIZE * data.blocks.size()) {
      data.blocks.push_back(0);
    }

    if (current_size_bits++ == i) {
      // Appending at end does not need copying of bits
      set(i, value);
    } else {
      // Shift everything right of inserted position in block of i
      const SizeType block_num = i / BLOCK_SIZE;
      const SizeType block_pos = i % BLOCK_SIZE;
      bool last_block_value = (*this)[block_num * BLOCK_SIZE + BLOCK_SIZE - 1];
      if ((block_pos + 1) % BLOCK_SIZE != 0) {
        // Shift remaining values in block (only necessary when not inserting at
        // end of block)
        const BlockType values = ((~0ull << block_pos) & data.blocks[block_num])
                                 << 1;
        const BlockType mask = (~0ull << (block_pos + 1));
        data.blocks[block_num] =
            (data.blocks[block_num] & ~mask) | (values & mask);
      }

      // Set inserted element
      set(i, value);

      // Shift all other blocks after it
      for (SizeType block = block_num + 1; block < size_in_blocks(); ++block) {
        const bool new_last_block_value =
            (*this)[block * BLOCK_SIZE + BLOCK_SIZE - 1];
        data.blocks[block] = (data.blocks[block] << 1) & ~1ull;
        set(block * BLOCK_SIZE, last_block_value);
        last_block_value = new_last_block_value;
      }
    }
  }

  /**
   * @brief Deletes an element at the given index.
   *
   * @param i The index to delete.
   */
  void delete_element(SizeType i) {
    // Update counters
    --current_size_bits;

    if (i == current_size_bits) {
      // If deleting last element, no copying needed.
      reset(i);
    } else {
      // Shift everything left of inserted position in block of i
      const SizeType block_num = i / BLOCK_SIZE;
      const SizeType block_pos = i % BLOCK_SIZE;
      SizeType last_block_pos = block_num * BLOCK_SIZE + BLOCK_SIZE - 1;
      const BlockType values =
          ((~0ull << (block_pos + 1)) & data.blocks[block_num]) >> 1;
      const BlockType mask = ~0ull << block_pos;
      data.blocks[block_num] =
          (data.blocks[block_num] & ~mask) | (values & mask);

      // Shift all other blocks after it
      for (SizeType block = block_num + 1; block < size_in_blocks(); ++block) {
        // Move first bit of block to block before
        set(last_block_pos, (*this)[block * BLOCK_SIZE]);
        last_block_pos += BLOCK_SIZE;

        // Shift complete block
        data.blocks[block] >>= 1;
      }

      // Reset last block's last bit
      reset(last_block_pos);
    }

    // Delete empty blocks
    if (!data.blocks.empty() &&
        current_size_bits == BLOCK_SIZE * (data.blocks.size() - 1)) {
      data.blocks.pop_back();
    }
  }

  /**
   * @brief Number of ones until position i (exclusive).
   *
   * @param i The position.
   * @return Number of ones.
   */
  SizeType rank_one(SizeType i) const {
    const SizeType block_num = i / BLOCK_SIZE;
    SizeType rank =
        popcount(data.blocks[block_num] & ~(~0ull << (i % BLOCK_SIZE)));
    for (SizeType block = 0; block < block_num; ++block) {
      rank += popcount(data.blocks[block]);
    }
    return rank;
  }

  /**
   * @brief Number of zeros until position i (exclusive).
   *
   * @param i The position.
   * @return Number of zeros.
   */
  SizeType rank_zero(SizeType i) const { return i - rank_one(i); }

  /**
   * @brief Get the position of the i-th one.
   *
   * @param i The i-th one (one-based index).
   * @return The position.
   */
  SizeType select_one(SizeType i) const {
    // Determine which block to scan
    SizeType select = i;
    SizeType block_idx = 0;
    for (; block_idx < size_in_blocks(); ++block_idx) {
      const auto num_ones = popcount(data.blocks[block_idx]);
      if (select > num_ones) {
        select -= num_ones;
      } else {
        break;
      }
    }

    // Scan identified block
    BlockType block = data.blocks[block_idx];
    SizeType block_pos = BLOCK_SIZE * block_idx;
    for (;; ++block_pos, block >>= 1) {
      if (block & 1ull) {
        --select;

        if (select == 0) {
          break;
        }
      }
    }

    return block_pos;
  }

  /**
   * @brief Get the position of the i-th zero.
   *
   * @param i The i-th zero (one-based index).
   * @return The position.
   */
  SizeType select_zero(SizeType i) const {
    // Determine which block to scan
    SizeType select = i;
    SizeType block_idx = 0;
    for (; block_idx < size_in_blocks(); ++block_idx) {
      const auto num_zeros = BLOCK_SIZE - popcount(data.blocks[block_idx]);
      if (select > num_zeros) {
        select -= num_zeros;
      } else {
        break;
      }
    }

    // Scan identified block
    BlockType block = data.blocks[block_idx];
    SizeType block_pos = BLOCK_SIZE * block_idx;
    for (;; ++block_pos, block >>= 1) {
      if ((block & 1ull) == 0) {
        --select;

        if (select == 0) {
          break;
        }
      }
    }

    return block_pos;
  }

  /**
   * @brief Number of ones/zeros until position i (exclusive).
   *
   * @param rank_one Whether to count ones.
   * @param i The position to count up to.
   * @return The rank.
   */
  SizeType rank(bool rank_one, SizeType i) const {
    if (rank_one) {
      return this->rank_one(i);
    } else {
      return this->rank_zero(i);
    }
  }

  /**
   * @brief Select the i-th one/zero.
   *
   * @param select_one Whether to select ones.
   * @param i The i-th relevant bit.
   * @return The select result.
   */
  SizeType select(bool select_one, SizeType i) const {
    if (select_one) {
      return this->select_one(i);
    } else {
      return this->select_zero(i);
    }
  }

  /**
   * @brief Splits the bitvector in half. This instance is the first half.
   *
   * @return The second half.
   */
  SimpleBitVector<BlockType, SizeType> split() {
    // Init new bitvector with #blocks / 2 new blocks
    // (plus one to avoid immediate allocation on insert)
    const SizeType moved_blocks = size_in_blocks() / 2;
    SimpleBitVector<BlockType, SizeType> split_out_bv(
        current_size_bits - moved_blocks * BLOCK_SIZE);

    // Copy second half to new destination
    const SizeType previous_num_blocks = size_in_blocks();
    for (SizeType i = moved_blocks; i < previous_num_blocks; ++i) {
      split_out_bv.data.blocks[i - moved_blocks] = data.blocks[i];
    }
    for (SizeType i = moved_blocks; i < previous_num_blocks; ++i) {
      data.blocks.pop_back();
    }

    // Update size
    current_size_bits = moved_blocks * BLOCK_SIZE;

    return split_out_bv;
  }

  /**
   * @brief Returns the number of ones in the complete bitvector.
   *
   * @return The number of ones.
   */
  SizeType num_ones() const {
    SizeType rank = 0;
    for (SizeType block = 0; block < size_in_blocks(); ++block) {
      rank += popcount(data.blocks[block]);
    }
    return rank;
  }

  /**
   * @brief Append value at the end.
   *
   * @param value The value to append.
   */
  void push_back(bool value) { insert(current_size_bits, value); }

  /**
   * @brief Pop value from the end.
   */
  void pop_back() { delete_element(current_size_bits - 1); }

  /**
   * @brief Copies bits from the given bitvector into this bitvector's back.
   *
   * @param other The other bitvector.
   */
  void copy_to_back(const SimpleBitVector<BlockType, SizeType>& other) {
    // Check that other vector is not empty
    if (other.size_in_blocks() == 0) {
      return;
    }

    // Reserve enough blocks
    const SizeType required_blocks =
        get_required_blocks(current_size_bits + other.size());
    const SizeType old_num_blocks = size_in_blocks();
    for (SizeType i = old_num_blocks; i < required_blocks; ++i) {
      data.blocks.push_back(0);
    }

    // Copy all blocks
    const SizeType insert_pos = current_size_bits % BLOCK_SIZE;
    if (insert_pos != 0) {
      // Insert position not aligned with block size
      BlockType last_block = data.blocks[old_num_blocks - 1];
      BlockType next_block = other.data.blocks[0];
      SizeType next_block_to_copy = 1;
      for (SizeType i = old_num_blocks - 1; i < size_in_blocks(); ++i) {
        // Copy block with offset
        const BlockType mask = ~0ull << insert_pos;
        const SizeType last_block_shift =
            i == old_num_blocks - 1 ? 0 : BLOCK_SIZE - insert_pos;
        data.blocks[i] = ((last_block >> last_block_shift) & ~mask) |
                         ((next_block << insert_pos) & mask);
        last_block = next_block;
        next_block = next_block_to_copy < other.size_in_blocks()
                         ? other.data.blocks[next_block_to_copy]
                         : 0;
        ++next_block_to_copy;
      }
    } else {
      // Insert position aligned with block, makes things easier
      for (SizeType i = old_num_blocks; i < size_in_blocks(); ++i) {
        data.blocks[i] = other.data.blocks[i - old_num_blocks];
      }
    }

    // Update counter
    current_size_bits += other.size();
  }

  /**
   * @brief Returns the space usage in bits.
   *
   * @return The space usage in bits.
   */
  SizeType space_used() const {
    // Space for all blocks plus the current size variable
    auto s = (size_in_blocks() * sizeof(BlockType) + sizeof(SizeType)) * 8ull;

    // Additional excess chunk data
    if constexpr (ExcessQuerySupport) {
      s += data.space_used();
    }

    return s;
  }
};

/**
 * @brief Define block size to be used in other compilation units.
 *
 * @tparam BlockType The block type.
 */
template <class BlockType, class SizeType, class AdditionalBlockData,
          bool ExcessQuerySupport>
constexpr SizeType SimpleBitVector<BlockType, SizeType, AdditionalBlockData,
                                   ExcessQuerySupport>::BLOCK_SIZE;

}  // namespace bv
}  // namespace ads

#endif
