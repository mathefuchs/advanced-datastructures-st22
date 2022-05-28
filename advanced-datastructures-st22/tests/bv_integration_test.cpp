#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace ads_test {

TEST(ads_test_suite, bitvector_integration_test) {
  // Run bitvector program
  std::system(
      "./build/advanced-datastructures-st22/ads_programm_a bv "
      "examples/long.txt actual_output.txt");

  // Load expected output file
  std::ifstream expected("examples/long_output.txt");
  ASSERT_TRUE(expected.is_open());
  std::string expected_str;
  expected >> expected_str;
  expected.close();

  // Load actual output file
  std::ifstream actual("actual_output.txt");
  ASSERT_TRUE(actual.is_open());
  std::string actual_str;
  actual >> actual_str;
  actual.close();

  // Delete temp file
  std::system("rm actual_output.txt");

  // Check contents
  ASSERT_EQ(expected_str, actual_str);
}

}  // namespace ads_test
