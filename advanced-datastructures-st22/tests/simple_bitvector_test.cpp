#include "bv/simple_bitvector.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, simple_bitvector_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
  ASSERT_EQ(bv.size_in_bits(), 0);
}

TEST(ads_test_suite, simple_bitvector_one_element_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1);
  ASSERT_EQ(bv.size_in_blocks(), 1);
  ASSERT_EQ(bv.size_in_bits(), 1);
}

TEST(ads_test_suite, simple_bitvector_one_block_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(
      ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE);
  ASSERT_EQ(bv.size_in_blocks(), 1);
  ASSERT_EQ(bv.size_in_bits(), ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE);
}

TEST(ads_test_suite, simple_bitvector_one_block_plus_one_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(
      ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE + 1);
  ASSERT_EQ(bv.size_in_blocks(), 2);
  ASSERT_EQ(bv.size_in_bits(),
            ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE + 1);
}

TEST(ads_test_suite, simple_bitvector_set_value_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = static_cast<bool>(i % 2);
  }

  // Set values
  ads::bv::SimpleBitVector<uint64_t> bv(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bv.set(7 + 13 * i, static_cast<bool>(i % 2));
  }

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_set_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::bv::SimpleBitVector<uint64_t> bv(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bv.set(7 + 13 * i);
  }

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_reset_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set and reset values
  ads::bv::SimpleBitVector<uint64_t> bv(20000);
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

TEST(ads_test_suite, simple_bitvector_flip_test) {
  // Expected values
  std::vector<bool> set_elements(20000, false);
  for (size_t i = 0; i < 1500; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::bv::SimpleBitVector<uint64_t> bv(20000);
  for (size_t i = 0; i < 1500; ++i) {
    bv.set(7 + 13 * i);
  }

  // Flip every second value
  for (size_t i = 0; i < 20000; ++i) {
    if (i % 2 == 0) {
      set_elements[i].flip();
      bv.flip(i);
    }
  }

  // Check values
  for (size_t i = 0; i < 20000; ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_insert_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(0);
  bv.insert(0, 1);
  bv.insert(0, 0);
  bv.insert(0, 1);
  bv.insert(0, 1);
  bv.insert(0, 0);
  bv.insert(0, 0);
  ASSERT_EQ(bv.size_in_bits(), 6);
  ASSERT_EQ(bv.size_in_blocks(), 1);
  ASSERT_FALSE(bv[0]);
  ASSERT_FALSE(bv[1]);
  ASSERT_TRUE(bv[2]);
  ASSERT_TRUE(bv[3]);
  ASSERT_FALSE(bv[4]);
  ASSERT_TRUE(bv[5]);
}

TEST(ads_test_suite, simple_bitvector_insert_middle_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  bv.insert(100, 1);
  bv.insert(200, 0);
  bv.insert(300, 1);
  bv.insert(400, 1);
  bv.insert(500, 0);
  bv.insert(600, 0);
  ASSERT_EQ(bv.size_in_bits(), 1006);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  ASSERT_TRUE(bv[100]);
  ASSERT_FALSE(bv[200]);
  ASSERT_TRUE(bv[300]);
  ASSERT_TRUE(bv[400]);
  ASSERT_FALSE(bv[500]);
  ASSERT_FALSE(bv[600]);
}

TEST(ads_test_suite, simple_bitvector_insert_reverse_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  bv.insert(600, 1);
  bv.insert(500, 0);
  bv.insert(400, 1);
  bv.insert(300, 1);
  bv.insert(200, 0);
  bv.insert(100, 0);
  ASSERT_EQ(bv.size_in_bits(), 1006);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  ASSERT_TRUE(bv[605]);
  ASSERT_FALSE(bv[504]);
  ASSERT_TRUE(bv[403]);
  ASSERT_TRUE(bv[302]);
  ASSERT_FALSE(bv[201]);
  ASSERT_FALSE(bv[100]);
}

TEST(ads_test_suite, simple_bitvector_insert_full_test) {
  // Expected values
  std::vector<bool> set_elements(1000, false);
  for (size_t i = 0; i < 75; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  for (size_t i = 0; i < 75; ++i) {
    bv.set(7 + 13 * i);
  }

  // Insert values
  size_t i = 0;
  for (auto it = std::begin(set_elements); it != std::end(set_elements); ++it) {
    if (i % 20 == 0) {
      it = set_elements.insert(it, i % 3);
      bv.insert(i, i % 3);
    }
    ++i;
  }

  // Check values
  ASSERT_EQ(set_elements.size(), bv.size_in_bits());
  for (size_t i = 0; i < bv.size_in_bits(); ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_until_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(5);
  bv.delete_elem(0);
  bv.delete_elem(0);
  bv.delete_elem(0);
  bv.delete_elem(0);
  bv.delete_elem(0);
  ASSERT_EQ(bv.size_in_bits(), 0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
}

TEST(ads_test_suite, simple_bitvector_delete_middle_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  bv.set(100);
  bv.set(200);
  bv.set(300);
  bv.set(400);
  bv.set(500);
  bv.delete_elem(100);
  bv.delete_elem(199);
  bv.delete_elem(298);
  bv.delete_elem(397);
  bv.delete_elem(496);
  ASSERT_EQ(bv.size_in_bits(), 995);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < bv.size_in_bits(); ++i) {
    ASSERT_FALSE(bv[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_full_test) {
  // Expected values
  std::vector<bool> set_elements(1000, false);
  for (size_t i = 0; i < 75; ++i) {
    set_elements[7 + 13 * i] = true;
  }

  // Set values
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  for (size_t i = 0; i < 75; ++i) {
    bv.set(7 + 13 * i);
  }

  // Insert values
  size_t i = 0;
  for (auto it = std::begin(set_elements); it != std::end(set_elements); ++it) {
    if (i % 20 == 0) {
      it = set_elements.erase(it);
      bv.delete_elem(i);
    }
    ++i;
  }

  // Check values
  ASSERT_EQ(set_elements.size(), bv.size_in_bits());
  for (size_t i = 0; i < bv.size_in_bits(); ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

}  // namespace ads_test
