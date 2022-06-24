#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "bp/dynamic_bp_tree.hpp"
#include "bv/dynamic_bitvector.hpp"

namespace ads_reliability_test {

TEST(ads_reliability_test_suite, dynamic_bitvector_tree_test) {
  srand(42);
  std::ofstream query_file("examples/bv_benchmark.txt", std::ios::trunc);
  std::ofstream output_file("examples/bv_benchmark_output.txt",
                            std::ios::trunc);
  ASSERT_TRUE(query_file.is_open());
  ASSERT_TRUE(output_file.is_open());

  // Initial values
  size_t initial_n = 10000000;
  query_file << initial_n << "\n";
  ads::bv::DynamicBitVector<uint64_t, uint64_t, int64_t, 8, 16, 32> bv;
  // std::vector<bool> exp;
  for (size_t i = 0; i < initial_n; ++i) {
    bool value = rand() % 2;
    bv.push_back(value);
    // exp.push_back(value);
    query_file << (value ? "1\n" : "0\n");
  }

  // Check equality
  // ASSERT_EQ(exp.size(), bv.size());
  // for (size_t j = 0; j < exp.size(); ++j) {
  //   ASSERT_EQ(exp[j], bv[j]);
  // }

  // Queries
  size_t query_reps = 10000000;
  for (size_t i = 0; i < query_reps; ++i) {
    // Print progress
    if (i % (query_reps / 100) == 0) {
      std::cout << (0.01f * (i / (query_reps / 100))) << std::endl;
    }

    // Inserts
    for (size_t j = 0; j < 3; ++j) {
      size_t pos = rand() % (bv.size() + 1);
      bool value = rand() % 2;

      bv.insert(pos, value);
      // exp.insert(exp.begin() + pos, value);
      query_file << "insert " << pos << " " << (value ? "1\n" : "0\n");
    }

    // Flip random element
    for (size_t j = 0; j < 3; ++j) {
      size_t pos = rand() % bv.size();
      bv.flip(pos);
      // exp[pos] = !exp[pos];
      query_file << "flip " << pos << "\n";
    }

    // Check equality
    // ASSERT_EQ(exp.size(), bv.size());
    // for (size_t j = 0; j < exp.size(); ++j) {
    //   ASSERT_EQ(exp[j], bv[j]);
    // }

    // Ranks
    for (size_t j = 0; j < 3; ++j) {
      bool value = rand() % 2;
      size_t pos = (rand() % bv.size()) + 1;

      // size_t exp_rank = 0;
      // for (size_t k = 0; k < pos; ++k) {
      //   if (exp[k] == value) ++exp_rank;
      // }

      size_t r = bv.rank(value, pos);
      // ASSERT_EQ(r, exp_rank);

      query_file << "rank " << (value ? "1 " : "0 ") << pos << "\n";
      output_file << r << "\n";
    }

    // Deletes
    for (size_t j = 0; j < 3; ++j) {
      size_t pos = rand() % bv.size();

      bv.delete_element(pos);
      // exp.erase(exp.begin() + pos);
      query_file << "delete " << pos << "\n";
    }

    // Selects
    for (size_t j = 0; j < 3; ++j) {
      bool value = rand() % 2;
      size_t num = value ? bv.num_ones() : bv.size() - bv.num_ones();
      size_t pos = (rand() % num) + 1;

      // size_t exp_select = 0;
      // size_t k = 0;
      // for (; k < exp.size(); ++k) {
      //   if (exp[k] == value) {
      //     ++exp_select;
      //     if (exp_select == pos) break;
      //   }
      // }

      size_t s = bv.select(value, pos);
      // ASSERT_EQ(s, k);

      query_file << "select " << (value ? "1 " : "0 ") << pos << "\n";
      output_file << s << "\n";
    }
  }
}

TEST(ads_reliability_test_suite, dynamic_bp_tree_test) {
  srand(42);
  std::ofstream query_file("examples/bp_benchmark.txt", std::ios::trunc);
  std::ofstream output_file("examples/bp_benchmark_output.txt",
                            std::ios::trunc);
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

      if (num_children != 0) {
        const size_t child_v = actual.i_th_child(0, child);
        const size_t num_child_child = actual.num_children(child_v);
        if (num_child_child > 0) {
          query_file << "parent " << (child_v + 1) << "\n";
          output_file << child_v << "\n";

          query_file << "child " << child_v << " " << num_child_child << "\n";
          output_file << actual.i_th_child(child_v, num_child_child) << "\n";
        } else {
          query_file << "parent " << actual.i_th_child(0, child) << "\n";
          output_file << "0\n";
        }
      }

      query_file << "insertchild 0 " << child << " " << take_children << "\n";
      actual.insert_node(0, child, take_children);
    }
    {
      const size_t num_children = actual.num_children(0);
      const size_t child = (rand() % num_children) + 1;
      const size_t delete_node = actual.i_th_child(0, child);
      query_file << "deletenode " << delete_node << "\n";
      actual.delete_node(delete_node);
    }

    query_file << "subtree_size 0\n";
    output_file << (((i + 1) * 2) + 1) << "\n";

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
