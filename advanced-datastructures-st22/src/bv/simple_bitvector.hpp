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
 * @brief Helper struct for forward and backward searches.
 *
 * @tparam SizeType The size type to use.
 * @tparam SignedIntType The signed integer type to use.
 */
template <class SizeType, class SignedIntType>
struct SearchResult {
  SizeType position;
  SignedIntType excess;
  bool found;
};

/**
 * @brief Forward declaration.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks, class AdditionalNodeData,
          class AdditionalLeafData, class AdditionalBlockData,
          bool ExcessQuerySupport>
class DynamicBitVector;

/**
 * @brief Simple Bitvector datastructure.
 *
 * @tparam BlockType The block type, e.g., uint64_t.
 * @tparam SizeType The size type, e.g., size_t.
 * @tparam SignedIntType The signed integer type.
 * @tparam AdditionalBlockData Additional data to
 * store along the raw data blocks.
 * @tparam ExcessQuerySupport Whether to support excess queries.
 */
template <class BlockType, class SizeType = uint64_t,
          class SignedIntType = int64_t,
          class AdditionalBlockData = meta::Empty<SizeType>,
          bool ExcessQuerySupport = false>
class SimpleBitVector {
 private:
  /**
   * @brief Friend class dynamic bitvector for
   * construction out of a simple vector.
   */
  template <class BlockT, class SizeT, class SignedT, SizeT MinLeafSizeBlocks,
            SizeT InitialLeafSizeBlocks, SizeT MaxLeafSizeBlocks,
            class AdditionalNodeData, class AdditionalLeafData,
            class AdditionalBlockDataT, bool ExcessQuerySupportT>
  friend class DynamicBitVector;

  using BitVector = SimpleBitVector<BlockType, SizeType, SignedIntType,
                                    AdditionalBlockData, ExcessQuerySupport>;

  /**
   * @brief The raw block data.
   */
  class BlockData : public AdditionalBlockData {
   private:
    /**
     * @brief Friend class dynamic bitvector for
     * construction out of a simple vector.
     */
    template <class BlockT, class SizeT, class SignedT, SizeT MinLeafSizeBlocks,
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
    const SizeType chunk_idx =
        i / (BLOCK_SIZE * AdditionalBlockData::BLOCKS_PER_CHUNK);
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
        data.chunk_array[i].num_occ_min_excess = 1;
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
  const BlockData& excess() const { return data; }

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
    if (current_size_bits == BLOCK_SIZE * size_in_blocks()) {
      // Add new excess chunk if necessary
      if constexpr (ExcessQuerySupport) {
        if (data.chunk_array.size() * data.BLOCKS_PER_CHUNK ==
            size_in_blocks()) {
          data.chunk_array.emplace_back();
        }
      }

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
        set(block * BLOCK_SIZE, last_block_value);  // TODO
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
        set(last_block_pos, (*this)[block * BLOCK_SIZE]);  // TODO
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

      // Remove last excess chunk if necessary
      if constexpr (ExcessQuerySupport) {
        if ((data.chunk_array.size() - 1) *
                AdditionalBlockData::BLOCKS_PER_CHUNK ==
            size_in_blocks()) {
          data.chunk_array.pop_back();
        }
      }
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
   * Assumes that the number of blocks moved is a multiple of the excess chunk
   * size.
   *
   * @return The second half.
   */
  BitVector split() {
    // Init new bitvector with #blocks / 2 new blocks
    // (plus one to avoid immediate allocation on insert)
    const SizeType moved_blocks = size_in_blocks() / 2;
    BitVector split_out_bv(current_size_bits - moved_blocks * BLOCK_SIZE);

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

    // Copy excess chunks
    if constexpr (ExcessQuerySupport) {
#ifndef NDEBUG
      if (moved_blocks % AdditionalBlockData::BLOCKS_PER_CHUNK != 0) {
        throw std::invalid_argument("Invalid block size to split.");
      }
#endif

      const SizeType moved_chunks =
          moved_blocks / AdditionalBlockData::BLOCKS_PER_CHUNK;
      const SizeType previous_num_chunks = data.chunk_array.size();
      for (SizeType i = moved_chunks; i < previous_num_chunks; ++i) {
        split_out_bv.data.chunk_array[i - moved_chunks] = data.chunk_array[i];
      }
      for (SizeType i = moved_chunks; i < previous_num_chunks; ++i) {
        data.chunk_array.pop_back();
      }
    }

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
  void copy_to_back(const BitVector& other) {
    // Check that other vector is not empty
    if (other.size_in_blocks() == 0) {
      return;
    }

    // Reserve enough blocks
    const SizeType required_blocks =
        get_required_blocks(current_size_bits + other.size());
    const SizeType old_num_blocks = size_in_blocks();

    // Copy all blocks
    const SizeType insert_pos = current_size_bits % BLOCK_SIZE;
    if (insert_pos != 0) {
#ifndef NDEBUG
      if constexpr (ExcessQuerySupport) {
        throw std::invalid_argument("Non-aligned copy-to-back not supported.");
      }
#endif

      // Allocate enough space
      data.blocks.reserve(required_blocks);
      for (SizeType i = old_num_blocks; i < required_blocks; ++i) {
        data.blocks.push_back(0);
      }

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
      // Insert position aligned with block makes things easier
      data.blocks.reserve(required_blocks);
      for (SizeType i = old_num_blocks; i < required_blocks; ++i) {
        data.blocks.push_back(other.data.blocks[i - old_num_blocks]);
      }

      if constexpr (ExcessQuerySupport) {
        const SizeType required_chunks =
            (required_blocks - 1) / AdditionalBlockData::BLOCKS_PER_CHUNK + 1;
        const SizeType old_num_chunks = data.chunk_array.size();
        data.chunk_array.reserve(required_chunks);
        for (SizeType i = old_num_chunks; i < required_chunks; ++i) {
          data.chunk_array.push_back(
              other.data.chunk_array[i - old_num_chunks]);
        }
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

  /**
   * @brief Forward search for excess
   *
   * @param pos The inclusive starting position.
   * @param d The excess to search for.
   * @return The search result.
   */
  SearchResult<SizeType, SignedIntType> forward_search(SizeType pos,
                                                       SignedIntType d) const {
    static constexpr SizeType BITS_PER_CHUNK =
        AdditionalBlockData::BLOCKS_PER_CHUNK * BLOCK_SIZE;
    const SizeType chunk_idx = pos / BITS_PER_CHUNK;
    const SizeType chunk_pos = pos % BITS_PER_CHUNK;

    // Scan chunk until end
    SignedIntType current_excess = 0;
    if (chunk_pos != 0) {
      for (SizeType i = pos; i < (chunk_idx + 1) * BITS_PER_CHUNK && i < size();
           ++i) {
        if ((*this)[i] == AdditionalBlockData::LEFT) {
          ++current_excess;
        } else {
          --current_excess;
        }

        if (current_excess == d) {
          return {i, d, true};
        }
      }
    }

    // If not found yet, use whole chunk information
    SizeType c = chunk_pos != 0 ? chunk_idx + 1 : chunk_idx;
    for (; c < data.chunk_array.size(); ++c) {
      if (current_excess + data.chunk_array[c].min_excess_in_block <= d) {
        break;
      }
      current_excess += data.chunk_array[c].block_excess;
    }

    // If found chunk with desired excess, scan it
    for (SizeType i = c * BITS_PER_CHUNK;
         i < (c + 1) * BITS_PER_CHUNK && i < size(); ++i) {
      if ((*this)[i] == AdditionalBlockData::LEFT) {
        ++current_excess;
      } else {
        --current_excess;
      }

      if (current_excess == d) {
        return {i, d, true};
      }
    }

    // Did not find d; returning current excess and next position
    return {0, current_excess, false};
  }

  /**
   * @brief Backward search for excess
   *
   * @param pos The exclusive starting position.
   * @param d The excess to search for.
   * @return The search result.
   */
  SearchResult<SizeType, SignedIntType> backward_search(SizeType pos,
                                                        SignedIntType d) const {
    static constexpr SizeType BITS_PER_CHUNK =
        AdditionalBlockData::BLOCKS_PER_CHUNK * BLOCK_SIZE;
    const SizeType chunk_idx = pos / BITS_PER_CHUNK;
    const SizeType chunk_pos = pos % BITS_PER_CHUNK;

    // Scan chunk until start
    SignedIntType current_excess = 0;
    for (SignedIntType i = static_cast<SignedIntType>(pos - 1);
         i >= chunk_idx * BITS_PER_CHUNK && i >= 0; --i) {
      if ((*this)[i] == AdditionalBlockData::LEFT) {
        --current_excess;
      } else {
        ++current_excess;
      }

      if (current_excess == d) {
        return {static_cast<SizeType>(i), d, true};
      }
    }

    // If not found yet, use whole chunk information
    SignedIntType c = chunk_idx - 1;
    for (; c >= 0; --c) {
      const SignedIntType possible_excess =
          current_excess - data.chunk_array[c].block_excess +
          data.chunk_array[c].min_excess_in_block;
      if (possible_excess <= d) {
        break;
      }
      current_excess -= data.chunk_array[c].block_excess;
      if (current_excess == d) {
        return {static_cast<SizeType>(c * BITS_PER_CHUNK), d, true};
      }
    }

    // If found chunk with desired excess, scan it
    for (SignedIntType i = (c + 1) * BITS_PER_CHUNK - 1;
         i >= c * BITS_PER_CHUNK && i >= 0; --i) {
      if ((*this)[i] == AdditionalBlockData::LEFT) {
        --current_excess;
      } else {
        ++current_excess;
      }

      if (current_excess == d) {
        return {static_cast<SizeType>(i), d, true};
      }
    }

    // Did not find d; returning current excess and next position
    return {0, current_excess, false};
  }
};

/**
 * @brief Define block size to be used in other compilation units.
 *
 * @tparam BlockType The block type.
 */
template <class BlockType, class SizeType, class SignedIntType,
          class AdditionalBlockData, bool ExcessQuerySupport>
constexpr SizeType
    SimpleBitVector<BlockType, SizeType, SignedIntType, AdditionalBlockData,
                    ExcessQuerySupport>::BLOCK_SIZE;

}  // namespace bv
}  // namespace ads

#endif
