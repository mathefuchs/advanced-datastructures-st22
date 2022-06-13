#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace ads_test {

TEST(ads_test_suite, dynamic_bp_tree_simple_integration_test) {
  // Run bitvector program
  std::system(
      "./build/advanced-datastructures-st22/ads_programm_a bp "
      "examples/simple_bp.txt actual_output.txt");

  // Load expected output file
  std::ifstream expected("examples/simple_bp_output.txt");
  ASSERT_TRUE(expected.is_open());
  std::ostringstream oss_expected;
  oss_expected << expected.rdbuf();
  expected.close();

  // Load actual output file
  std::ifstream actual("actual_output.txt");
  ASSERT_TRUE(actual.is_open());
  std::ostringstream oss_actual;
  oss_actual << actual.rdbuf();
  actual.close();

  // Delete temp file
  std::system("rm actual_output.txt");

  // Check contents
  ASSERT_EQ(oss_expected.str(), oss_actual.str());
}

TEST(ads_test_suite, dynamic_bp_tree_integration_test) {
  // Run bitvector program
  std::system(
      "./build/advanced-datastructures-st22/ads_programm_a bp "
      "examples/bp.txt actual_output.txt");

  // Load expected output file
  std::ifstream expected("examples/bp_output.txt");
  ASSERT_TRUE(expected.is_open());
  std::ostringstream oss_expected;
  oss_expected << expected.rdbuf();
  expected.close();

  // Load actual output file
  std::ifstream actual("actual_output.txt");
  ASSERT_TRUE(actual.is_open());
  std::ostringstream oss_actual;
  oss_actual << actual.rdbuf();
  actual.close();

  // Delete temp file
  std::system("rm actual_output.txt");

  // Check contents
  ASSERT_EQ(oss_expected.str(), oss_actual.str());
}

}  // namespace ads_test
