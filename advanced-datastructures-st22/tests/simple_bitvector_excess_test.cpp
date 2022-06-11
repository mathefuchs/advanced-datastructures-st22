#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <vector>

#include "bp/dynamic_bp_tree.hpp"
#include "bv/simple_bitvector.hpp"

namespace ads_test {

template <class UnsignedT, class SignedT, UnsignedT BlocksPerChunk>
using SimpleExcessBitVector = ads::bv::SimpleBitVector<
    UnsignedT, UnsignedT, SignedT,
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

TEST(ads_test_suite, simple_excess_bitvector_insert_test) {
  srand(0);
  const size_t n = 5000;
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv;
  std::vector<bool> expected;

  // Set bitvector by inserting bits
  for (size_t i = 0; i < n; ++i) {
    size_t pos = expected.empty() ? 0 : rand() % expected.size();
    bool value = rand() % 2 == 0;

    expected.insert(expected.begin() + pos, value);
    bv.insert(pos, value);

    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
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

TEST(ads_test_suite, simple_excess_bitvector_delete_test) {
  srand(0);
  const size_t n = 5000;
  SimpleExcessBitVector<uint16_t, int16_t, 2> bv(n);
  std::vector<bool> expected(n);

  // Set values
  for (size_t i = 0; i < n; ++i) {
    bool value = rand() % 2 == 0;
    expected[i] = value;
    bv.set(i, value);
  }
  int64_t excess = 0;
  int64_t min_excess = 2;
  size_t num_min = 0;
  for (size_t j = 0; j < expected.size(); ++j) {
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

  // Delete until empty
  for (size_t i = 0; i < n; ++i) {
    size_t pos = rand() % expected.size();
    bool value = rand() % 2 == 0;

    expected.erase(expected.begin() + pos);
    bv.delete_element(pos);

    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
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

#ifndef NDEBUG
TEST(ads_test_suite, simple_excess_bitvector_illegal_split_test) {
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv(334);
  ASSERT_THROW(bv.split(), std::invalid_argument);
}
#endif

TEST(ads_test_suite, simple_excess_bitvector_split_test) {
  srand(0);
  const size_t n = 8192;
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv(n);
  std::vector<bool> expected(n);

  // Set values
  for (size_t i = 0; i < n; ++i) {
    bool value = rand() % 2 == 0;
    expected[i] = value;
    bv.set(i, value);
  }
  {
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
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

  // Split
  auto snd_bv = bv.split();
  {  // First half
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < 4096; ++j) {
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
  {  // Second half
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 4096; j < 8192; ++j) {
      if (expected[j] == snd_bv.excess().LEFT) {
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
    const auto bv_excess = snd_bv.excess().compute();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }
}

#ifndef NDEBUG
TEST(ads_test_suite, simple_excess_bitvector_illegal_copy_back_test) {
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv1(334);
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv2(334);
  ASSERT_THROW(bv1.copy_to_back(bv2), std::invalid_argument);
}
#endif

TEST(ads_test_suite, simple_excess_bitvector_copy_back_test) {
  srand(42);
  const size_t n = 4096;
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv1(n);
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv2(n);
  std::vector<bool> expected(2 * n);

  // Set values
  for (size_t i = 0; i < 2 * n; ++i) {
    bool value = rand() % 2 == 0;
    expected[i] = value;
    if (i < n) {
      bv1.set(i, value);
    } else {
      bv2.set(i - n, value);
    }
  }

  // Copy to back
  bv1.copy_to_back(bv2);
  int64_t excess = 0;
  int64_t min_excess = 2;
  size_t num_min = 0;
  for (size_t j = 0; j < 2 * n; ++j) {
    if (expected[j] == bv1.excess().LEFT) {
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
  const auto bv_excess = bv1.excess().compute();
  ASSERT_EQ(bv_excess.block_excess, excess);
  ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
  ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
}

static inline std::pair<bool, size_t> expected_forward_search_result(
    bool left, const std::vector<bool> &bv, size_t start, int64_t d) {
  int64_t excess = 0;
  for (size_t i = start; i < bv.size(); ++i) {
    if (bv[i] == left) {
      ++excess;
    } else {
      --excess;
    }
    if (excess == d) {
      return {true, i};
    }
  }
  return {false, 0};
}

static inline void assert_forward_search(
    const SimpleExcessBitVector<uint32_t, int32_t, 4> &bv,
    const std::vector<bool> &expected, size_t start, int64_t d) {
  auto exp =
      expected_forward_search_result(bv.excess().LEFT, expected, start, d);
  auto act = bv.forward_search(start, d);
  ASSERT_EQ(exp.first, act.found);
  ASSERT_EQ(exp.second, act.position);
}

TEST(ads_test_suite, simple_excess_bitvector_forward_search_test) {
  srand(0);
  const size_t n = 200000;
  SimpleExcessBitVector<uint32_t, int32_t, 4> bv(n);
  std::vector<bool> expected(n);

  // Set values; keep track of excess
  // to have a valid sequence of parentheses
  {
    size_t excess = 0;
    for (size_t i = 0; i < n; ++i) {
      bool value = excess > 0 ? rand() % 2 == 0 : bv.excess().LEFT;
      if (value == bv.excess().LEFT) {
        ++excess;
      } else {
        --excess;
      }
      expected[i] = value;
      bv.set(i, value);
    }
    for (size_t i = 0; i < excess; ++i) {
      expected.push_back(bv.excess().RIGHT);
      bv.push_back(bv.excess().RIGHT);
    }
  }
  {
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
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

  // Forward search
  for (size_t i = 0; i < n; ++i) {
    if (expected[i] == bv.excess().LEFT) {
      assert_forward_search(bv, expected, i, 0);
    }
  }
}

}  // namespace ads_test
