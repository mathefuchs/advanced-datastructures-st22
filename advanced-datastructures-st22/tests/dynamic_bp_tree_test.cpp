#include "bp/dynamic_bp_tree.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include "bp/simple_tree.hpp"

namespace ads_test {

TEST(ads_test_suite, dynamic_bp_tree_block_excess_test) {
  using ExcessData =
      ads::bp::MinExcessNodeData<uint64_t, uint64_t, int64_t, 2ull>;
  std::vector<uint64_t> blocks;

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 0).block_excess, 0);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 0).min_excess_in_block,
            2);

  blocks.push_back(0ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 1).block_excess, 1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 1).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 43).block_excess, 43);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 43).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 63).block_excess, 63);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 63).min_excess_in_block,
            1);

  blocks.push_back(7ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 64).block_excess, 64);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 64).min_excess_in_block,
            1);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 65).block_excess, 63);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 0, 65).min_excess_in_block,
            1);

  blocks.push_back(~0ull);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 128).block_excess, 0);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 128).min_excess_in_block, 2);

  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 180).block_excess, -52);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 180).min_excess_in_block,
      -52);
  ASSERT_EQ(ExcessData::compute_block_excess(blocks, 1, 192).block_excess, -64);
  ASSERT_EQ(
      ExcessData::compute_block_excess(blocks, 1, 192).min_excess_in_block,
      -64);
}

TEST(ads_test_suite, dynamic_bp_tree_insert_delete_test) {
  srand(42);
  ads::bp::SimpleTree expected;
  ads::bp::DynamicBPTree<uint64_t, uint32_t, int32_t, 16, 32, 64, 8> actual;

  for (size_t i = 0; i < 20000; ++i) {
    const size_t num_children = expected.root->children.size();
    const size_t child = num_children == 0 ? 1 : (rand() % num_children) + 1;
    ASSERT_EQ(actual.parent(actual.i_th_child(0, child)), 0);

    const size_t take_children =
        child >= num_children ? 0 : rand() % (num_children - child);
    expected.insert_node(expected.root, child, take_children);
    actual.insert_node(0, child, take_children);
    ASSERT_EQ(expected.get_bp_representation(), actual.get_bp_representation());
    ASSERT_EQ(expected.subtree_size(expected.root), actual.subtree_size(0));
    ASSERT_EQ(expected.subtree_size(expected.i_th_child(expected.root, 1)),
              actual.subtree_size(actual.i_th_child(0, 1)));

    if (i % 100 == 0) {
      // Compare DFS traversal strings
      std::ostringstream oss1;
      expected.pre_order_children_sizes(oss1);
      std::ostringstream oss2;
      actual.pre_order_children_sizes(oss2, 0);
      ASSERT_EQ(oss1.str(), oss2.str());
    }
  }

  // Concrete sizes implementation-specific
  // ASSERT_EQ(expected.space_used(), 320008);
  // ASSERT_EQ(actual.space_used(), 61696);
  ASSERT_GT(expected.space_used(), 4 * actual.space_used());

  const size_t n = expected.subtree_size(expected.root) - 1;
  for (size_t i = 0; i < n; ++i) {
    const size_t num_children = expected.root->children.size();
    const size_t child = (rand() % num_children) + 1;
    expected.delete_node(expected.i_th_child(expected.root, child));
    actual.delete_node(actual.i_th_child(0, child));
    expected.i_th_child(expected.root, child);
    ASSERT_EQ(expected.get_bp_representation(), actual.get_bp_representation());
    ASSERT_EQ(expected.subtree_size(expected.root), actual.subtree_size(0));

    if (i % 100 == 0) {
      // Compare DFS traversal strings
      std::ostringstream oss1;
      expected.pre_order_children_sizes(oss1);
      std::ostringstream oss2;
      actual.pre_order_children_sizes(oss2, 0);
      ASSERT_EQ(oss1.str(), oss2.str());
    }
  }
  ASSERT_EQ(expected.get_bp_representation(), "()");
  ASSERT_EQ(actual.get_bp_representation(), "()");
}

}  // namespace ads_test
