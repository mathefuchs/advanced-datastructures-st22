#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "bp/dynamic_bp_tree.hpp"

namespace ads_reliability_test {

TEST(ads_reliability_test_suite, dynamic_bp_tree_test) {
  srand(42);
  std::ofstream query_file("examples/bp_benchmark.txt", std::ios::trunc);
  std::ofstream output_file("examples/bp_benchmark_output.txt", std::ios::trunc);
  ASSERT_TRUE(query_file.is_open());
  ASSERT_TRUE(output_file.is_open());

  ads::bp::DynamicBPTree<uint64_t, uint32_t, int32_t, 16, 32, 64, 8> actual;
  const size_t n = 1000000;
  for (size_t i = 0; i < n; ++i) {
    if (i % (n / 100) == 0) {
      std::cout << (0.01f * (i / (n / 100))) << std::endl;
    }
    for (size_t j = 0; j < 3; ++j) {
      const size_t num_children = actual.num_children(0);
      const size_t child = num_children == 0 ? 1 : (rand() % num_children) + 1;
      const size_t take_children =
          child >= num_children ? 0 : rand() % (num_children - child);
      query_file << "insertchild 0 " << child << " " << take_children
                 << "\n";
      actual.insert_node(0, child, take_children);
    }
    {
      const size_t num_children = actual.num_children(0);
      const size_t child = (rand() % num_children) + 1;
      const size_t delete_node = actual.i_th_child(0, child);
      query_file << "deletenode " << delete_node << "\n";
      actual.delete_node(delete_node);
    }
    // Check DFS traversal string creation
    if (i % (n / 100) == 0) {
      std::ostringstream oss;
      actual.pre_order_children_sizes(oss);
    }
  }

  actual.pre_order_children_sizes(output_file);

  // Close files
  query_file.close();
  output_file.close();
}

}  // namespace ads_reliability_test
