#include "ds/bitset.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, bitset_empty_test) {
  ads::ds::Bitset bitset(0);
  ASSERT_EQ(bitset.size(), 0);
}

TEST(ads_test_suite, bitset_one_element_test) {
  ads::ds::Bitset bitset(1);
  ASSERT_EQ(bitset.size(), 1);
}

TEST(ads_test_suite, bitset_set_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::ds::Bitset bitset(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bitset.set(7 + 13 * i);
  }

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bitset[i], set_elements[i]);
  }
}

TEST(ads_test_suite, bitset_reset_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set and reset values
  ads::ds::Bitset bitset(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bitset.set(7 + 13 * i);
  }
  bitset.set(0);
  for (size_t i = 0; i < 750; ++i) {
    set_elements[7 + 26 * i] = false;
    bitset.reset(7 + 26 * i);
  }
  bitset.reset(0);
  bitset.reset(2);
  bitset.reset(1);

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bitset[i], set_elements[i]);
  }
}

}  // namespace ads_test
