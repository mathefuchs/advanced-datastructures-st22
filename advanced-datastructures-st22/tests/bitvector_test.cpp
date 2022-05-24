#include "ds/bitvector.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, bitset_empty_test) {
  ads::ds::BitVector bv(0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
}

TEST(ads_test_suite, bitset_one_element_test) {
  ads::ds::BitVector bv(1);
  ASSERT_EQ(bv.size_in_blocks(), 1);
}

TEST(ads_test_suite, bitset_set_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::ds::BitVector bv(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bv.set(7 + 13 * i);
  }

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, bitset_reset_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set and reset values
  ads::ds::BitVector bv(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bv.set(7 + 13 * i);
  }
  bv.set(0);
  for (size_t i = 0; i < 750; ++i) {
    set_elements[7 + 26 * i] = false;
    bv.reset(7 + 26 * i);
  }
  bv.reset(0);
  bv.reset(2);
  bv.reset(1);

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

}  // namespace ads_test
