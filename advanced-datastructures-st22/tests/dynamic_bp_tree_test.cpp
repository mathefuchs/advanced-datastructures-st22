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
  using ExcessData =
      ads::bp::MinExcessNodeData<uint64_t, uint64_t, int64_t, 2ull>;
  std::vector<uint64_t> blocks;

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 0).block_excess, 0);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 0).min_excess_in_block,
            2);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 0).num_occ_min_excess,
            0);

  blocks.push_back(0ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 1).block_excess, 1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 1).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 1).num_occ_min_excess,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 43).block_excess, 43);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 43).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 43).num_occ_min_excess,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 63).block_excess, 63);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 63).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 63).num_occ_min_excess,
            1);

  blocks.push_back(7ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 64).block_excess, 64);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 64).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 64).num_occ_min_excess,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 65).block_excess, 63);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 65).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 65).num_occ_min_excess,
            1);

  blocks.push_back(~0ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 128).block_excess, 0);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 128).min_excess_in_block, 2);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 128).num_occ_min_excess,
            0);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 180).block_excess, -52);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 180).min_excess_in_block,
      -52);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 180).num_occ_min_excess,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 192).block_excess, -64);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 192).min_excess_in_block,
      -64);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 192).num_occ_min_excess,
            1);
}

}  // namespace ads_test
