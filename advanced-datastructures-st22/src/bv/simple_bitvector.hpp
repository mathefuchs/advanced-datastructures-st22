#ifndef SIMPLE_BITVECTOR_HPP
#define SIMPLE_BITVECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <sstream>
#include <vector>

namespace ads {
namespace bv {

/**
 * @brief Simple Bitvector datastructure.
 *
 * @tparam BlockType The block type, e.g., uint64_t.
 * @tparam SizeType The size type, e.g., size_t.
 */
template <class BlockType, class SizeType = size_t>
class SimpleBitVector {
 private:
  SizeType current_size_bits;
  std::vector<BlockType> blocks;

  /**
   * @brief Use built-in popcount function with correct width.
   *
   * @param block The block.
   * @return The popcount.
   */
  inline SizeType popcount(BlockType block) const {
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

 public:
  static constexpr SizeType BLOCK_SIZE =
      static_cast<SizeType>(8ul * sizeof(BlockType));

  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size in bits.
   */
  SimpleBitVector(SizeType initial_size)
      : current_size_bits(initial_size),
        blocks(initial_size == 0 ? 0 : get_required_blocks(initial_size), 0) {}

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
      : current_size_bits(other.current_size_bits), blocks(other.blocks) {}

  /**
   * @brief Accesses the bit vector at index i.
   *
   * @param i The index to access.
   * @return true if set.
   * @return false otherwise.
   */
  inline bool operator[](SizeType i) const {
    return (blocks[i / BLOCK_SIZE] >> (i % BLOCK_SIZE)) & 1ULL;
  }

  /**
   * @brief Sets the i-th element to the given value.
   *
   * @param i The index to set.
   * @param value The value to set it to.
   */
  inline void set(SizeType i, bool value) {
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
  inline void set(SizeType i) {
    blocks[i / BLOCK_SIZE] |= (1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Resets the i-th element.
   *
   * @param i The index to reset.
   */
  inline void reset(SizeType i) {
    blocks[i / BLOCK_SIZE] &= ~(1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Returns the size in blocks.
   *
   * @return The size in blocks.
   */
  inline SizeType size_in_blocks() const {
    return static_cast<SizeType>(blocks.size());
  }

  /**
   * @brief Returns the size in bits.
   *
   * @return The size in bits.
   */
  inline SizeType size() const { return current_size_bits; }

  /**
   * @brief Flips the i-th bit.
   *
   * @param i The bit to flip.
   */
  inline void flip(SizeType i) {
    blocks[i / BLOCK_SIZE] ^= (1ULL << (i % BLOCK_SIZE));
  }

  /**
   * @brief Inserts an element at position i.
   *
   * @param i The position to insert.
   * @param value The value of the inserted element.
   */
  inline void insert(SizeType i, bool value) {
    // Update counters
    if (current_size_bits == BLOCK_SIZE * blocks.size()) {
      blocks.push_back(0);
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
        const BlockType values = ((~0ull << block_pos) & blocks[block_num])
                                 << 1;
        const BlockType mask = (~0ull << (block_pos + 1));
        blocks[block_num] = (blocks[block_num] & ~mask) | (values & mask);
      }

      // Set inserted element
      set(i, value);

      // Shift all other blocks after it
      for (SizeType block = block_num + 1; block < size_in_blocks(); ++block) {
        const bool new_last_block_value =
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
  inline void delete_element(SizeType i) {
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
          ((~0ull << (block_pos + 1)) & blocks[block_num]) >> 1;
      const BlockType mask = ~0ull << block_pos;
      blocks[block_num] = (blocks[block_num] & ~mask) | (values & mask);

      // Shift all other blocks after it
      for (SizeType block = block_num + 1; block < size_in_blocks(); ++block) {
        // Move first bit of block to block before
        set(last_block_pos, (*this)[block * BLOCK_SIZE]);
        last_block_pos += BLOCK_SIZE;

        // Shift complete block
        blocks[block] >>= 1;
      }

      // Reset last block's last bit
      reset(last_block_pos);
    }

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
  inline SizeType rank_one(SizeType i) const {
    const SizeType block_num = i / BLOCK_SIZE;
    SizeType rank = popcount(blocks[block_num] & ~(~0ull << (i % BLOCK_SIZE)));
    for (SizeType block = 0; block < block_num; ++block) {
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
  inline SizeType rank_zero(SizeType i) const { return i - rank_one(i); }

  /**
   * @brief Get the position of the i-th one.
   *
   * @param i The i-th one (one-based index).
   * @return The position.
   */
  inline SizeType select_one(SizeType i) const {
    // Determine which block to scan
    SizeType select = i;
    SizeType block_idx = 0;
    for (; block_idx < size_in_blocks(); ++block_idx) {
      const auto num_ones = popcount(blocks[block_idx]);
      if (select > num_ones) {
        select -= num_ones;
      } else {
        break;
      }
    }

    // Scan identified block
    BlockType block = blocks[block_idx];
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
  inline SizeType select_zero(SizeType i) const {
    // Determine which block to scan
    SizeType select = i;
    SizeType block_idx = 0;
    for (; block_idx < size_in_blocks(); ++block_idx) {
      const auto num_zeros = BLOCK_SIZE - popcount(blocks[block_idx]);
      if (select > num_zeros) {
        select -= num_zeros;
      } else {
        break;
      }
    }

    // Scan identified block
    BlockType block = blocks[block_idx];
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
  SimpleBitVector<BlockType, SizeType>* split() {
    // Init new bitvector with #blocks / 2 new blocks
    // (plus one to avoid immediate allocation on insert)
    const SizeType moved_blocks = size_in_blocks() / 2;
    auto split_out_bv = new SimpleBitVector<BlockType, SizeType>(
        current_size_bits - moved_blocks * BLOCK_SIZE);

    // Copy second half to new destination
    const SizeType previous_num_blocks = size_in_blocks();
    for (SizeType i = moved_blocks; i < previous_num_blocks; ++i) {
      split_out_bv->blocks[i - moved_blocks] = blocks[i];
    }
    for (SizeType i = moved_blocks; i < previous_num_blocks; ++i) {
      blocks.pop_back();
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
      rank += popcount(blocks[block]);
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
      blocks.push_back(0);
    }

    // Copy all blocks
    const SizeType insert_pos = current_size_bits % BLOCK_SIZE;
    if (insert_pos != 0) {
      // Insert position not aligned with block size
      BlockType last_block = blocks[old_num_blocks - 1];
      BlockType next_block = other.blocks[0];
      SizeType next_block_to_copy = 1;
      for (SizeType i = old_num_blocks - 1; i < size_in_blocks(); ++i) {
        // Copy block with offset
        const BlockType mask = ~0ull << insert_pos;
        const SizeType last_block_shift =
            i == old_num_blocks - 1 ? 0 : BLOCK_SIZE - insert_pos;
        blocks[i] = ((last_block >> last_block_shift) & ~mask) |
                    ((next_block << insert_pos) & mask);
        last_block = next_block;
        next_block = next_block_to_copy < other.size_in_blocks()
                         ? other.blocks[next_block_to_copy]
                         : 0;
        ++next_block_to_copy;
      }
    } else {
      // Insert position aligned with block, makes things easier
      for (SizeType i = old_num_blocks; i < size_in_blocks(); ++i) {
        blocks[i] = other.blocks[i - old_num_blocks];
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
  inline SizeType space_used() const {
    // Space for all blocks plus the current size variable
    return (size_in_blocks() * sizeof(BlockType) + sizeof(SizeType)) * 8ull;
  }
};

/**
 * @brief Define block size to be used in other compilation units.
 *
 * @tparam BlockType The block type.
 */
template <class BlockType, class SizeType>
constexpr SizeType SimpleBitVector<BlockType, SizeType>::BLOCK_SIZE;

/**
 * @brief Custom operator to print bit-vectors.
 *
 * @tparam BlockType The block type.
 * @tparam SizeType The size type.
 * @param os The output stream.
 * @param bv The bit vector.
 * @return The output stream.
 */
template <class BlockType, class SizeType>
static std::ostream& operator<<(
    std::ostream& os, const SimpleBitVector<BlockType, SizeType>& bv) {
  for (SizeType i = 0; i < bv.size(); ++i) {
    os << (bv[i] ? "1" : "0");
  }
  os << "\n";
  return os;
}

}  // namespace bv
}  // namespace ads

#endif
