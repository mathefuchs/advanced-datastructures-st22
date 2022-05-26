#include "bv/dynamic_bitvector.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, dynamic_bitvector_insert_test) {
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 32, 64, 128> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 100; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 100);
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, dynamic_bitvector_insert_split_test) {
  ads::bv::DynamicBitVector<uint16_t, uint32_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 1200);
  ASSERT_EQ(bv.get_tree_structure(),
            "(512 171 (256 85 (256 85)(256 86))(256 85 (256 85)(432 144)))");
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, dynamic_bitvector_set_test) {
  ads::bv::DynamicBitVector<uint16_t, uint64_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 1200);
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }

  for (size_t i = 0; i < 1200; i += 5) {
    bv.set(i, true);
  }
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], (i % 3 == 1) || (i % 5 == 0));
  }
}

TEST(ads_test_suite, dynamic_bitvector_flip_test) {
  ads::bv::DynamicBitVector<uint16_t, uint16_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 0);
  }
  ASSERT_EQ(bv.size(), 1200);
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 0);
  }

  for (size_t i = 0; i < 1200; i += 5) {
    bv.flip(i);
  }
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], (i % 3 == 0) ^ (i % 5 == 0));
  }
}

TEST(ads_test_suite, dynamic_bitvector_rank_test) {
  ads::bv::DynamicBitVector<uint16_t, uint32_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 2);
  }
  ASSERT_EQ(bv.size(), 1200);
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 2);
    ASSERT_EQ(bv.rank(true, i), i / 3);
    ASSERT_EQ(bv.rank(false, i), i - i / 3);
  }

  ASSERT_EQ(bv.rank(true, 1), 0);
  ASSERT_EQ(bv.rank(true, 60), 20);
  ASSERT_EQ(bv.rank(true, 600), 200);
  ASSERT_EQ(bv.rank(false, 1), 1);
  ASSERT_EQ(bv.rank(false, 60), 40);
  ASSERT_EQ(bv.rank(false, 600), 400);
}

TEST(ads_test_suite, dynamic_bitvector_select_test) {
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);

  for (size_t i = 0; i < 30000; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 30000);
  for (size_t i = 0; i < 30000; i += 3) {
    ASSERT_EQ(bv[i], false);
    ASSERT_EQ(bv[i + 1], true);
    ASSERT_EQ(bv[i + 2], false);
    ASSERT_EQ(bv.select(false, i - i / 3 + 1), i);
    ASSERT_EQ(bv.select(true, i / 3 + 1), i + 1);
    ASSERT_EQ(bv.select(false, i - i / 3 + 2), i + 2);
  }
}

TEST(ads_test_suite, dynamic_bitvector_delete_test) {}

}  // namespace ads_test
