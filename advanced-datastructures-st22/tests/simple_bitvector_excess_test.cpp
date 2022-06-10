#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <vector>

#include "bp/dynamic_bp_tree.hpp"
#include "bv/simple_bitvector.hpp"

namespace ads_test {

template <class UnsignedT, class SignedT, UnsignedT BlocksPerChunk>
using SimpleExcessBitVector = ads::bv::SimpleBitVector<
    UnsignedT, UnsignedT,
    ads::bp::MinExcessBlockData<UnsignedT, UnsignedT, SignedT, BlocksPerChunk>,
    true>;

TEST(ads_test_suite, simple_excess_bitvector_empty_test) {
  SimpleExcessBitVector<uint64_t, int64_t, 3> bv(0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
  ASSERT_EQ(bv.size(), 0);

  const auto excess = bv.excess().compute();
  ASSERT_EQ(excess.block_excess, 0);
  ASSERT_EQ(excess.min_excess_in_block, 2);
  ASSERT_EQ(excess.num_occ_min_excess, 0);
}

TEST(ads_test_suite, simple_excess_bitvector_set_value_test) {
  srand(0);
  const size_t n = 5000;
  SimpleExcessBitVector<uint64_t, int64_t, 3> bv(n);
  std::vector<bool> expected(n, false);

  for (size_t i = 0; i < n; ++i) {
    bool value = rand() % 2 == 0;
    expected[i] = value;
    bv.set(i, value);

    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < n; ++j) {
      if (expected[j] == bv.excess().LEFT) {
        ++excess;
      } else {
        --excess;
      }

      if (excess < min_excess) {
        min_excess = excess;
        num_min = 1;
      } else if (excess == min_excess) {
        min_excess = excess;
        ++num_min;
      }
    }

    const auto bv_excess = bv.excess().compute();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }
}

TEST(ads_test_suite, simple_excess_bitvector_flip_test) {
  srand(0);
  const size_t n = 5000;
  SimpleExcessBitVector<uint64_t, int64_t, 3> bv(n);
  std::vector<bool> expected(n, false);

  // Set bitvector by flipping
  for (size_t i = 0; i < n; ++i) {
    bool value = rand() % 2 == 0;
    expected[i] = value;
    if (value) {
      bv.flip(i);
    }

    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < n; ++j) {
      if (expected[j] == bv.excess().LEFT) {
        ++excess;
      } else {
        --excess;
      }

      if (excess < min_excess) {
        min_excess = excess;
        num_min = 1;
      } else if (excess == min_excess) {
        min_excess = excess;
        ++num_min;
      }
    }

    const auto bv_excess = bv.excess().compute();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }

  // Flip all and check
  for (size_t i = 0; i < n; ++i) {
    bv.flip(i);

    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < n; ++j) {
      if ((j <= i && expected[j] == bv.excess().RIGHT) ||
          (j > i && expected[j] == bv.excess().LEFT)) {
        ++excess;
      } else {
        --excess;
      }

      if (excess < min_excess) {
        min_excess = excess;
        num_min = 1;
      } else if (excess == min_excess) {
        min_excess = excess;
        ++num_min;
      }
    }

    const auto bv_excess = bv.excess().compute();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }
}

TEST(ads_test_suite, simple_excess_bitvector_space_used_test) {
  SimpleExcessBitVector<uint64_t, int32_t, 3> bv(10000);
  size_t block_space =
      ((10000 / 64 + 1) * sizeof(uint64_t) + sizeof(uint64_t)) * 8ull;
  size_t excess_space = ((10000 / 64 + 1) / 3 + 1) *
                        (2ull * sizeof(int32_t) + sizeof(uint64_t)) * 8ull;
  ASSERT_EQ(bv.space_used(), block_space + excess_space);
}

}  // namespace ads_test
