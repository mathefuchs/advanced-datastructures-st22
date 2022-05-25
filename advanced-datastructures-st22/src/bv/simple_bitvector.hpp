#ifndef BITVECTOR_HPP
#define BITVECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace ads {
namespace bv {

/**
 * @brief Bit-vector datastructure.
 */
template <class BlockType>
class SimpleBitVector {
 private:
  size_t current_size_bits;
  std::vector<BlockType> blocks;

  /**
   * @brief Use built-in popcount function with correct width.
   *
   * @param block The block.
   * @return The popcount.
   */
  inline size_t popcount(BlockType block) {
    if constexpr (sizeof(BlockType) == sizeof(long)) {
      return __builtin_popcountl(block);
    } else if constexpr (sizeof(BlockType) == sizeof(long long)) {
      return __builtin_popcountll(block);
    } else {
      return __builtin_popcount(block);
    }
  }

 public:
  static constexpr size_t BLOCK_SIZE = 8ul * sizeof(BlockType);

  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size in bits.
   * @param capacity The capacity in bits.
   */
  SimpleBitVector(size_t initial_size, size_t capacity)
      : current_size_bits(initial_size),
        blocks(
            capacity >= initial_size
                ? (capacity == 0 ? 0 : (capacity - 1) / BLOCK_SIZE + 1)
                : (initial_size == 0 ? 0 : (initial_size - 1) / BLOCK_SIZE + 1),
            0) {}

  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size in bits.
   */
  explicit SimpleBitVector(size_t initial_size)
      : SimpleBitVector(initial_size, initial_size) {}

  /**
   * @brief Accesses the bit vector at index i.
   *
   * @param i The index to access.
   * @return true if set.
   * @return false otherwise.
   */
  inline bool operator[](size_t i) const {
    return (blocks[i / BLOCK_SIZE] >> (i % BLOCK_SIZE)) & 1ULL;
  }

  /**
   * @brief Sets the i-th element to the given value.
   *
   * @param i The index to set.
   * @param value The value to set it to.
   */
  inline void set(size_t i, bool value) {
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
  inline void set(size_t i) {
    blocks[i / BLOCK_SIZE] |= (1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Resets the i-th element.
   *
   * @param i The index to reset.
   */
  inline void reset(size_t i) {
    blocks[i / BLOCK_SIZE] &= ~(1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Returns the size in blocks.
   *
   * @return The size in blocks.
   */
  inline size_t size_in_blocks() const { return blocks.size(); }

  /**
   * @brief Returns the size in bits.
   *
   * @return The size in bits.
   */
  inline size_t size_in_bits() const { return current_size_bits; }

  /**
   * @brief Flips the i-th bit.
   *
   * @param i The bit to flip.
   */
  inline void flip(size_t i) {
    blocks[i / BLOCK_SIZE] ^= (1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Inserts an element at position i.
   *
   * @param i The position to insert.
   * @param value The value of the inserted element.
   */
  inline void insert(size_t i, bool value) {
    // Update counters
    if (current_size_bits == BLOCK_SIZE * blocks.size()) {
      blocks.push_back(0);
    }

    if (current_size_bits++ == i) {
      // Appending at end does not need copying of bits
      set(i, value);
    } else {
      // Shift everything right of inserted position in block of i
      const size_t block_num = i / BLOCK_SIZE;
      const size_t block_pos = i % BLOCK_SIZE;
      BlockType last_block_value =
          (*this)[block_num * BLOCK_SIZE + BLOCK_SIZE - 1];
      const BlockType values = ((~0ull << block_pos) & blocks[block_num]) << 1;
      const BlockType mask = (~0ull << (block_pos + 1));
      blocks[block_num] = (blocks[block_num] & ~mask) | (values & mask);

      // Set inserted element
      set(i, value);

      // Shift all other blocks after it
      for (size_t block = block_num + 1; block < blocks.size(); ++block) {
        const BlockType new_last_block_value =
            (*this)[block * BLOCK_SIZE + BLOCK_SIZE - 1];
        blocks[block] = (blocks[block] << 1) & ~1ull;
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
  inline void delete_elem(size_t i) {
    // Update counters
    --current_size_bits;

    // Shift everything left of inserted position in block of i
    const size_t block_num = i / BLOCK_SIZE;
    const size_t block_pos = i % BLOCK_SIZE;
    size_t last_block_pos = block_num * BLOCK_SIZE + BLOCK_SIZE - 1;
    const BlockType values =
        ((~0ull << (block_pos + 1)) & blocks[block_num]) >> 1;
    const BlockType mask = ~0ull << block_pos;
    blocks[block_num] = (blocks[block_num] & ~mask) | (values & mask);

    // Shift all other blocks after it
    for (size_t block = block_num + 1; block < blocks.size(); ++block) {
      // Move first bit of block to block before
      set(last_block_pos, (*this)[block * BLOCK_SIZE]);
      last_block_pos += BLOCK_SIZE;

      // Shift complete block
      blocks[block] >>= 1;
    }

    // Reset last block's last bit
    reset(last_block_pos);

    // Delete empty blocks
    if (!blocks.empty() &&
        current_size_bits == BLOCK_SIZE * (blocks.size() - 1)) {
      blocks.pop_back();
    }
  }

  /**
   * @brief Number of ones until position i (exclusive).
   *
   * @param i The position.
   * @return Number of ones.
   */
  inline size_t rank_one(size_t i) {
    const size_t block_num = i / BLOCK_SIZE;
    size_t rank = popcount(blocks[block_num] & ~(~0ul << (i % BLOCK_SIZE)));
    for (size_t block = 0; block < block_num; ++block) {
      rank += popcount(blocks[block]);
    }
    return rank;
  }

  /**
   * @brief Number of zeros until position i (exclusive).
   *
   * @param i The position.
   * @return Number of zeros.
   */
  inline size_t rank_zero(size_t i) { return i - rank_one(i); }

  /**
   * @brief Get the position of the i-th one.
   *
   * @param i The i-th one (one-based index).
   * @return The position.
   */
  inline size_t select_one(size_t i) {
    // Determine which block to scan
    size_t select = i;
    size_t block_idx = 0;
    for (; block_idx < blocks.size(); ++block_idx) {
      const auto num_ones = popcount(blocks[block_idx]);
      if (select > num_ones) {
        select -= num_ones;
      } else {
        break;
      }
    }

    // Scan identified block
    size_t block = blocks[block_idx];
    size_t block_pos = BLOCK_SIZE * block_idx;
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
  inline size_t select_zero(size_t i) {
    // Determine which block to scan
    size_t select = i;
    size_t block_idx = 0;
    for (; block_idx < blocks.size(); ++block_idx) {
      const auto num_zeros = BLOCK_SIZE - popcount(blocks[block_idx]);
      if (select > num_zeros) {
        select -= num_zeros;
      } else {
        break;
      }
    }

    // Scan identified block
    size_t block = blocks[block_idx];
    size_t block_pos = BLOCK_SIZE * block_idx;
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
};

/**
 * @brief Define block size to be used in other compilation units.
 *
 * @tparam BlockType The block type.
 */
template <class BlockType>
constexpr size_t SimpleBitVector<BlockType>::BLOCK_SIZE;

}  // namespace bv
}  // namespace ads

#endif
