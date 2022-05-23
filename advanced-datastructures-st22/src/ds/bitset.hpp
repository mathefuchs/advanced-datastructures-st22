#ifndef BITSET_HPP
#define BITSET_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <vector>

namespace ads {
namespace ds {

/**
 * Bitset datastructure.
 */
class Bitset {
 public:
  explicit Bitset(size_t n) : n(n), blocks(n / 64 + 1, 0) {}

  inline bool operator[](size_t i) {
    return (blocks[i / 64] >> (i % 64)) & 1ULL;
  }

  inline void set(size_t i) {
    blocks[i / 64] |= static_cast<uint64_t>(1ULL << (i % 64));
  }

  inline void reset(size_t i) {
    blocks[i / 64] &= ~static_cast<uint64_t>(1ULL << (i % 64));
  }

  inline void reset() {
    for (size_t i = 0; i < blocks.size(); ++i) {
      blocks[i] = 0;
    }
  }

  inline size_t size() { return n; }

 private:
  size_t n;
  std::vector<uint64_t> blocks;
};

}  // namespace ds
}  // namespace ads

#endif  // BITSET_HPP
