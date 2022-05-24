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
 public:
  static constexpr size_t BLOCK_SIZE = 8ul * sizeof(BlockType);

  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size.
   */
  explicit SimpleBitVector(size_t initial_size)
      : current_size_bits(initial_size),
        blocks(initial_size == 0 ? 0 : (initial_size - 1) / BLOCK_SIZE + 1, 0) {
  }

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
    if (current_size_bits % BLOCK_SIZE == 0) {
      blocks.push_back(0);
    }
    ++current_size_bits;

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
    if (current_size_bits % BLOCK_SIZE == 0) {
      blocks.pop_back();
    }
  }

 private:
  size_t current_size_bits;
  std::vector<BlockType> blocks;
};

}  // namespace bv
}  // namespace ads

#endif
