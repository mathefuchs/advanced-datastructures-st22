#ifndef BITVECTOR_HPP
#define BITVECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace ads {
namespace ds {

/**
 * @brief Bit-vector datastructure.
 */
class BitVector {
 public:
  /**
   * @brief Constructs a new bit vector.
   *
   * @param initial_size The initial size.
   */
  explicit BitVector(size_t initial_size)
      : current_size_blocks(initial_size),
        blocks(initial_size == 0 ? 0 : (initial_size - 1) / 64 + 1, 0) {}

  /**
   * @brief Accesses the bit vector at index i.
   *
   * @param i The index to access.
   * @return true if set.
   * @return false otherwise.
   */
  inline bool operator[](size_t i) const {
    return (blocks[i / 64] >> (i % 64)) & 1ULL;
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
  inline void set(size_t i) { blocks[i / 64] |= (1ULL << (i % 64)); }

  /**
   * @brief Resets the i-th element.
   *
   * @param i The index to reset.
   */
  inline void reset(size_t i) { blocks[i / 64] &= ~(1ULL << (i % 64)); }

  /**
   * @brief Resets the complete bit vector.
   */
  inline void reset() {
    for (size_t i = 0; i < blocks.size(); ++i) {
      blocks[i] = 0;
    }
  }

  /**
   * @brief Returns the size in blocks.
   *
   * @return The size in blocks.
   */
  inline size_t size_in_blocks() const { return current_size_blocks; }

 private:
  size_t current_size_blocks;
  std::vector<uint64_t> blocks;
};

}  // namespace ds
}  // namespace ads

#endif
