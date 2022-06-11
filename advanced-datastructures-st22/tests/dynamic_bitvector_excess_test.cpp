#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include "bp/dynamic_bp_tree.hpp"
#include "bv/dynamic_bitvector.hpp"
#include "bv/simple_bitvector.hpp"

namespace ads_test {

template <class UnsignedT, class SignedT, UnsignedT BlocksPerChunk>
using SimpleExcessBitVector = ads::bv::SimpleBitVector<
    UnsignedT, UnsignedT, SignedT,
    ads::bp::MinExcessBlockData<UnsignedT, UnsignedT, SignedT, BlocksPerChunk>,
    true>;

template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks, SizeType BlocksPerChunk>
using DynamicMinExcessBitVector = ads::bv::DynamicBitVector<
    BlockType, SizeType, SignedIntType, MinLeafSizeBlocks,
    InitialLeafSizeBlocks, MaxLeafSizeBlocks,
    ads::bp::MinExcessNodeData<BlockType, SizeType, SignedIntType,
                               BlocksPerChunk>,
    ads::bp::MinExcessLeafData,
    ads::bp::MinExcessBlockData<BlockType, SizeType, SignedIntType,
                                BlocksPerChunk>,
    true>;

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

TEST(ads_test_suite, dynamic_excess_bitvector_set_and_forward_search_test) {
  srand(0);
  const size_t n = 1000000;
  DynamicMinExcessBitVector<uint64_t, uint64_t, int64_t, 8, 16, 32, 8> bv;
  std::vector<bool> expected(n);

  // Set values; keep track of excess
  // to have a valid sequence of parentheses
  {
    size_t excess = 0;
    for (size_t i = 0; i < n; ++i) {
      bool value = excess > 0 ? rand() % 2 == 0 : false;
      if (!value) {
        ++excess;
      } else {
        --excess;
      }
      expected[i] = value;
      bv.push_back(value);
    }
    for (size_t i = 0; i < excess; ++i) {
      expected.push_back(true);
      bv.push_back(true);
    }
  }
  {
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
      if (!expected[j]) {
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
    const auto bv_excess = bv.excess();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }

  // Forward search
  // std::cout << bv.get_tree_structure() << std::endl;
  for (size_t i = 0; i < n; ++i) {
    if (!expected[i]) {
      auto exp = expected_forward_search_result(false, expected, i, 0);
      auto act = bv.forward_search(i, 0);
      ASSERT_EQ(exp.first, act.found);
      ASSERT_EQ(exp.second, act.position);
    }
  }
}

TEST(ads_test_suite, dynamic_excess_bitvector_copy_and_forward_search_test) {
  srand(42);
  const size_t n = 1000000;
  SimpleExcessBitVector<uint64_t, int64_t, 8> simple_bv;
  std::vector<bool> expected(n);

  // Set values; keep track of excess
  // to have a valid sequence of parentheses
  {
    size_t excess = 0;
    for (size_t i = 0; i < n; ++i) {
      bool value = excess > 0 ? rand() % 2 == 0 : false;
      if (!value) {
        ++excess;
      } else {
        --excess;
      }
      expected[i] = value;
      simple_bv.push_back(value);
    }
    for (size_t i = 0; i < excess; ++i) {
      expected.push_back(true);
      simple_bv.push_back(true);
    }
  }

  // Construct dynamic bitvector from existing simple
  DynamicMinExcessBitVector<uint64_t, uint64_t, int64_t, 8, 16, 32, 8> bv(
      simple_bv);
  {
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
      if (!expected[j]) {
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
    const auto bv_excess = bv.excess();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }

  // Forward search
  // std::cout << bv.get_tree_structure() << std::endl;
  for (size_t i = 0; i < n; ++i) {
    if (!expected[i]) {
      auto exp = expected_forward_search_result(false, expected, i, 0);
      auto act = bv.forward_search(i, 0);
      ASSERT_EQ(exp.first, act.found);
      ASSERT_EQ(exp.second, act.position);
    }
  }
}

TEST(ads_test_suite,
     dynamic_excess_bitvector_insert_delete_forward_search_test) {
  srand(23);
  const size_t n = 10000;
  DynamicMinExcessBitVector<uint64_t, uint64_t, int64_t, 8, 16, 32, 8> bv;
  std::vector<bool> expected;

  // Set values
  for (size_t i = 0; i < n; ++i) {
    size_t pos = expected.empty() ? 0 : rand() % expected.size();
    expected.insert(expected.begin() + pos, false);
    expected.insert(expected.begin() + pos + 1, true);
    bv.insert(pos, false);
    bv.insert(pos + 1, true);
  }
  {
    int64_t excess = 0;
    int64_t min_excess = 2;
    size_t num_min = 0;
    for (size_t j = 0; j < expected.size(); ++j) {
      if (!expected[j]) {
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
    const auto bv_excess = bv.excess();
    ASSERT_EQ(bv_excess.block_excess, excess);
    ASSERT_EQ(bv_excess.min_excess_in_block, min_excess);
    ASSERT_EQ(bv_excess.num_occ_min_excess, num_min);
  }

  // Forward search
  // std::cout << bv.get_tree_structure() << std::endl;
  for (size_t i = 0; i < n; ++i) {
    if (!expected[i]) {
      auto exp = expected_forward_search_result(false, expected, i, 0);
      auto act = bv.forward_search(i, 0);
      ASSERT_EQ(exp.first, act.found);
      ASSERT_EQ(exp.second, act.position);
    }
  }

  // Delete all values
  for (size_t i = 0; i < n; ++i) {
    // Pick random left parenthesis
    size_t pos = rand() % expected.size();
    while (expected[pos]) {
      pos = rand() % expected.size();
    }

    // Delete left parentheses with matching right parentheses
    auto exp = expected_forward_search_result(false, expected, pos, 0);
    auto act = bv.forward_search(pos, 0);
    ASSERT_EQ(exp.first, act.found);
    ASSERT_EQ(exp.second, act.position);
    expected.erase(expected.begin() + exp.second);
    expected.erase(expected.begin() + pos);
    bv.delete_element(act.position);
    bv.delete_element(pos);
  }

  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(expected.size(), 0);
}

}  // namespace ads_test
