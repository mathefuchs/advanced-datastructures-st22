#include "bp/dynamic_bp_tree.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, dynamic_bp_tree_block_excess_test) {
  using ExcessData = ads::bp::MinExcessNodeData<uint64_t, uint64_t, int64_t>;

  ASSERT_EQ(ExcessData::compute_block_excess(0ull).block_excess,
            ExcessData::BitVector::BLOCK_SIZE);
  ASSERT_EQ(ExcessData::compute_block_excess(0ull).min_excess_in_block, 1);
  ASSERT_EQ(ExcessData::compute_block_excess(~0ull).block_excess,
            -ExcessData::BitVector::BLOCK_SIZE);
  ASSERT_EQ(ExcessData::compute_block_excess(~0ull).min_excess_in_block,
            -ExcessData::BitVector::BLOCK_SIZE);
  ASSERT_EQ(ExcessData::compute_block_excess(7ull).block_excess,
            ExcessData::BitVector::BLOCK_SIZE - 2 * 3);
  ASSERT_EQ(ExcessData::compute_block_excess(7ull).min_excess_in_block, -3);
  ASSERT_EQ(ExcessData::compute_block_excess(~7ull).block_excess,
            -ExcessData::BitVector::BLOCK_SIZE + 2 * 3);
  ASSERT_EQ(ExcessData::compute_block_excess(~7ull).min_excess_in_block,
            -ExcessData::BitVector::BLOCK_SIZE + 2 * 3);
}

}  // namespace ads_test
