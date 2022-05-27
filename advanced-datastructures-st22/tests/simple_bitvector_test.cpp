#include "bv/simple_bitvector.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, simple_bitvector_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
  ASSERT_EQ(bv.size(), 0);
}

TEST(ads_test_suite, simple_bitvector_one_element_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1);
  ASSERT_EQ(bv.size_in_blocks(), 1);
  ASSERT_EQ(bv.size(), 1);
}

TEST(ads_test_suite, simple_bitvector_one_block_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(
      ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE);
  ASSERT_EQ(bv.size_in_blocks(), 1);
  ASSERT_EQ(bv.size(), ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE);
}

TEST(ads_test_suite, simple_bitvector_one_block_plus_one_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(
      ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE + 1);
  ASSERT_EQ(bv.size_in_blocks(), 2);
  ASSERT_EQ(bv.size(), ads::bv::SimpleBitVector<uint64_t>::BLOCK_SIZE + 1);
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

TEST(ads_test_suite, simple_bitvector_set_value_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(512);
  std::vector<bool> expected(512, false);
  for (size_t i = 0; i < 512; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
  }
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_set_reset_value_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(512);
  for (size_t i = 0; i < 512; ++i) {
    bv.set(i, i % 2);
  }

  std::vector<bool> expected(512, false);
  for (size_t i = 0; i < 512; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    if (value) {
      bv.set(i);
    } else {
      bv.reset(i);
    }
  }
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
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

TEST(ads_test_suite, simple_bitvector_flip_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(512);
  std::vector<bool> expected(512, false);
  for (size_t i = 0; i < 512; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    if (value) {
      bv.flip(i);
    }
  }
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_insert_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv;
  bv.insert(0, 1);
  bv.insert(0, 0);
  bv.insert(0, 1);
  bv.insert(0, 1);
  bv.insert(0, 0);
  bv.insert(0, 0);
  ASSERT_EQ(bv.size(), 6);
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
  ASSERT_EQ(bv.size(), 1006);
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
  ASSERT_EQ(bv.size(), 1006);
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
  ASSERT_EQ(set_elements.size(), bv.size());
  for (size_t i = 0; i < bv.size(); ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_insert_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(512);
  std::vector<bool> expected(1024, false);
  for (size_t i = 0; i < 512; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.insert(i, value);
    for (size_t j = 0; j <= i; ++j) {
      ASSERT_EQ(bv[j], expected[j]);
    }
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_until_empty_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(5);
  bv.delete_element(0);
  bv.delete_element(0);
  bv.delete_element(0);
  bv.delete_element(0);
  bv.delete_element(0);
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.size_in_blocks(), 0);
}

TEST(ads_test_suite, simple_bitvector_delete_middle_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  bv.set(100);
  bv.set(200);
  bv.set(300);
  bv.set(400);
  bv.set(500);
  bv.delete_element(100);
  bv.delete_element(199);
  bv.delete_element(298);
  bv.delete_element(397);
  bv.delete_element(496);
  ASSERT_EQ(bv.size(), 995);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < bv.size(); ++i) {
    ASSERT_FALSE(bv[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_full_test) {
  // Expected values
  srand(0);
  std::vector<bool> set_elements(10000, false);
  for (size_t i = 0; i < 10000; ++i) {
    set_elements[i] = rand() % 2;
  }

  // Set values
  ads::bv::SimpleBitVector<uint32_t> bv(10000);
  for (size_t i = 0; i < 10000; ++i) {
    bv.set(i, set_elements[i]);
  }

  // Insert values
  size_t i = 0;
  for (auto it = std::begin(set_elements); it != std::end(set_elements); ++it) {
    if (i % 3 == 0) {
      it = set_elements.erase(it);
      bv.delete_element(i);
    }
    ++i;
  }

  // Check values
  ASSERT_EQ(set_elements.size(), bv.size());
  for (size_t i = 0; i < bv.size(); ++i) {
    ASSERT_EQ(bv[i], set_elements[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(1024);
  std::vector<bool> expected(1024, false);
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);

  for (size_t i = 0; i < 512; ++i) {
    // Delete every second element
    bv.delete_element(i);
  }
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);

  ASSERT_EQ(bv.size(), 512);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(bv[i], expected[2 * i + 1]);
  }
}

TEST(ads_test_suite, simple_bitvector_rank_test) {
  ads::bv::SimpleBitVector<uint64_t> bv(1000);
  bv.set(0);
  bv.set(1);
  bv.set(50);
  bv.set(63);
  bv.set(64);
  bv.set(65);
  bv.set(100);
  bv.set(200);
  bv.set(300);
  bv.set(800);

  ASSERT_EQ(bv.size(), 1000);
  ASSERT_EQ(bv.size_in_blocks(), 16);

  ASSERT_EQ(bv.rank_one(1), 1);
  ASSERT_EQ(bv.rank_zero(1), 0);
  ASSERT_EQ(bv.rank_one(2), 2);
  ASSERT_EQ(bv.rank_zero(2), 0);
  ASSERT_EQ(bv.rank_one(500), 9);
  ASSERT_EQ(bv.rank_zero(500), 491);
}

TEST(ads_test_suite, simple_bitvector_rank_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(1024);
  std::vector<bool> expected(1024, false);
  std::vector<size_t> ones_so_far(1025, 0);
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
    if (value) {
      ones_so_far[i + 1] = ones_so_far[i] + 1;
    } else {
      ones_so_far[i + 1] = ones_so_far[i];
    }
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
    ASSERT_EQ(bv.rank_one(i), ones_so_far[i]);
    ASSERT_EQ(bv.rank_zero(i), i - ones_so_far[i]);
    ASSERT_EQ(bv.rank(true, i), ones_so_far[i]);
    ASSERT_EQ(bv.rank(false, i), i - ones_so_far[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_select_test) {
  ads::bv::SimpleBitVector<uint8_t> bv(1000);
  bv.set(0);
  bv.set(1);
  bv.set(50);
  bv.set(63);
  bv.set(64);
  bv.set(65);
  bv.set(100);
  bv.set(200);
  bv.set(300);
  bv.set(800);

  ASSERT_EQ(bv.size(), 1000);
  ASSERT_EQ(bv.size_in_blocks(), 125);

  ASSERT_EQ(bv.select_one(1), 0);
  ASSERT_EQ(bv.select_zero(1), 2);
  ASSERT_EQ(bv.select_one(2), 1);
  ASSERT_EQ(bv.select_zero(2), 3);
  ASSERT_EQ(bv.select_one(9), 300);
  ASSERT_EQ(bv.select_zero(50), 52);
}

TEST(ads_test_suite, simple_bitvector_select_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(1024);
  std::vector<bool> expected(1024, false);
  std::vector<size_t> one_positions;
  std::vector<size_t> zero_positions;
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
    if (value) {
      one_positions.push_back(i);
    } else {
      zero_positions.push_back(i);
    }
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
  for (size_t i = 0; i < one_positions.size(); ++i) {
    ASSERT_EQ(bv.select_one(i + 1), one_positions[i]);
  }
  for (size_t i = 0; i < zero_positions.size(); ++i) {
    ASSERT_EQ(bv.select_zero(i + 1), zero_positions[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_split_test) {
  ads::bv::SimpleBitVector<uint16_t> bv(1000);
  bv.set(0);
  bv.set(1);
  bv.set(50);
  bv.set(63);
  bv.set(64);
  bv.set(65);
  bv.set(100);
  bv.set(600);
  bv.set(700);
  bv.set(800);

  ASSERT_EQ(bv.size(), 1000);
  ASSERT_EQ(bv.size_in_blocks(), 63);

  auto *right_half_bv = bv.split();
  EXPECT_EQ(bv.size_in_blocks(), 31);
  EXPECT_EQ(bv.size(), 31 * 16);
  EXPECT_EQ(right_half_bv->size_in_blocks(), 32);
  EXPECT_EQ(right_half_bv->size(), 1000 - 31 * 16);
  EXPECT_TRUE(bv[0]);
  EXPECT_TRUE(bv[1]);
  EXPECT_TRUE(bv[50]);
  EXPECT_TRUE(bv[63]);
  EXPECT_TRUE(bv[64]);
  EXPECT_TRUE(bv[65]);
  EXPECT_TRUE(bv[100]);
  EXPECT_TRUE((*right_half_bv)[104]);
  EXPECT_TRUE((*right_half_bv)[204]);
  EXPECT_TRUE((*right_half_bv)[304]);
  delete right_half_bv;
}

TEST(ads_test_suite, simple_bitvector_split_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(1024);
  std::vector<bool> expected(1024, false);
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);

  auto *second_half = bv.split();
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);
  ASSERT_EQ(second_half->size(), 512);
  ASSERT_EQ(second_half->size_in_blocks(), 8);
  for (size_t i = 0; i < 512; ++i) {
    EXPECT_EQ(bv[i], expected[i]);
  }
  for (size_t i = 0; i < 512; ++i) {
    EXPECT_EQ((*second_half)[i], expected[i + 512]);
  }
  delete second_half;
}

TEST(ads_test_suite, simple_bitvector_num_ones_test) {
  ads::bv::SimpleBitVector<uint8_t> bv1(1000);
  ASSERT_EQ(bv1.num_ones(), 0);
  for (size_t i = 56; i < 346; ++i) {
    bv1.set(i, i % 2 == 1);
  }
  ASSERT_EQ(bv1.num_ones(), (346 - 56) / 2);

  ads::bv::SimpleBitVector<uint16_t> bv2(1000);
  ASSERT_EQ(bv2.num_ones(), 0);
  for (size_t i = 56; i < 346; ++i) {
    bv2.set(i, i % 2 == 1);
  }
  ASSERT_EQ(bv2.num_ones(), (346 - 56) / 2);

  ads::bv::SimpleBitVector<uint32_t> bv3(1000);
  ASSERT_EQ(bv3.num_ones(), 0);
  for (size_t i = 56; i < 346; ++i) {
    bv3.set(i, i % 2 == 1);
  }
  ASSERT_EQ(bv3.num_ones(), (346 - 56) / 2);

  ads::bv::SimpleBitVector<uint64_t> bv4(1000);
  ASSERT_EQ(bv4.num_ones(), 0);
  for (size_t i = 56; i < 346; ++i) {
    bv4.set(i, i % 2 == 1);
  }
  ASSERT_EQ(bv4.num_ones(), (346 - 56) / 2);
}

TEST(ads_test_suite, simple_bitvector_num_ones_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(1024);
  std::vector<bool> expected(1024, false);
  size_t ones = 0;
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    bv.set(i, value);
    if (value) {
      ++ones;
    }
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
  ASSERT_EQ(bv.num_ones(), ones);
}

TEST(ads_test_suite, simple_bitvector_push_pop_test) {
  ads::bv::SimpleBitVector<uint8_t> bv;
  for (size_t i = 0; i < 100; ++i) {
    bv.push_back(i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 100);
  ASSERT_EQ(bv.size_in_blocks(), 13);
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }

  for (size_t i = 0; i < 50; ++i) {
    bv.pop_back();
  }
  ASSERT_EQ(bv.size(), 50);
  ASSERT_EQ(bv.size_in_blocks(), 7);
  for (size_t i = 0; i < 50; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, simple_bitvector_push_pop_block_size_test) {
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(512);
  std::vector<bool> expected(1024, false);
  for (size_t i = 0; i < 1024; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    if (i < 512) {
      bv.set(i, value);
    } else {
      bv.push_back(value);
    }
  }
  ASSERT_EQ(bv.size(), 1024);
  ASSERT_EQ(bv.size_in_blocks(), 16);
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }

  for (size_t i = 0; i < 512; ++i) {
    bv.pop_back();
  }
  ASSERT_EQ(bv.size(), 512);
  ASSERT_EQ(bv.size_in_blocks(), 8);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_copy_to_back_test) {
  // Prepare
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> dst(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> src(0);
  std::vector<bool> expected(3100, false);
  for (size_t i = 0; i < 2100; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    dst.push_back(value);
  }
  for (size_t i = 2100; i < 3100; ++i) {
    bool value = rand() % 2 == 1;
    expected[i] = value;
    src.push_back(value);
  }
  ASSERT_EQ(src.size(), 1000);
  ASSERT_EQ(dst.size(), 2100);

  // Act
  dst.copy_to_back(src);

  // Assert
  ASSERT_EQ(src.size(), 1000);
  ASSERT_EQ(dst.size(), 3100);
  for (size_t i = 0; i < 1000; ++i) {
    ASSERT_EQ(src[i], expected[i + 2100]);
  }
  for (size_t i = 0; i < 3100; ++i) {
    ASSERT_EQ(dst[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_copy_to_back_bug_test) {
  // Bug when bit-vectors have sizes that are multiples of the block size.
  // Prepare
  srand(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> dst(0);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> src(0);
  std::vector<bool> expected(1024, false);
  for (size_t i = 0; i < 512; ++i) {
    bool value = rand() % 3 == 0;
    expected[i] = value;
    dst.push_back(value);
  }
  for (size_t i = 512; i < 1024; ++i) {
    bool value = rand() % 2 == 1;
    expected[i] = value;
    src.push_back(value);
  }
  ASSERT_EQ(src.size(), 512);
  ASSERT_EQ(dst.size(), 512);

  // Act
  dst.copy_to_back(src);

  // Assert
  ASSERT_EQ(src.size(), 512);
  ASSERT_EQ(dst.size(), 1024);
  for (size_t i = 0; i < 512; ++i) {
    ASSERT_EQ(src[i], expected[i + 512]);
  }
  for (size_t i = 0; i < 1024; ++i) {
    ASSERT_EQ(dst[i], expected[i]);
  }
}

TEST(ads_test_suite, simple_bitvector_delete_big_example_test) {
  // Build large simple bitvector
  srand(0);
  std::vector<bool> expected(10000, false);
  ads::bv::SimpleBitVector<uint64_t, uint64_t> bv(expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    bool value = rand() % 2 == 0;
    bv.set(i, value);
    expected[i] = value;
  }
  ASSERT_EQ(bv.size(), expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }

  // Fully delete it again
  std::vector<int> expected_pos;
  std::vector<int> actual_pos;
  for (int i = 0; i < expected.size(); ++i) {
    expected_pos.push_back(i);
    actual_pos.push_back(i);
  }
  for (int i = 0; i < expected.size(); ++i) {
    int deleted_idx = rand() % (expected.size() - i);
    bv.delete_element(deleted_idx);

    for (int p = 0; p < expected_pos.size(); ++p) {
      if (actual_pos[p] > deleted_idx) {
        --actual_pos[p];
      } else if (actual_pos[p] == deleted_idx) {
        actual_pos[p] = -1;
        expected_pos[p] = -1;
      }
    }

    for (int p = 0; p < expected_pos.size(); ++p) {
      if (expected_pos[p] != -1) {
        ASSERT_EQ(bv[actual_pos[p]], expected[expected_pos[p]]);
      }
    }
  }
  ASSERT_EQ(bv.size(), 0);
}

}  // namespace ads_test
