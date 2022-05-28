#include "bv/dynamic_bitvector.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

namespace ads_test {

TEST(ads_test_suite, dynamic_bitvector_insert_test) {
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 32, 64, 128> bv;
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.num_ones(), 0);

  for (size_t i = 0; i < 100; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 100);
  ASSERT_EQ(bv.num_ones(), 33);
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, dynamic_bitvector_insert_split_test) {
  ads::bv::DynamicBitVector<uint16_t, uint32_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.num_ones(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 1200);
  ASSERT_EQ(bv.num_ones(), 400);
  ASSERT_EQ(
      bv.get_tree_structure(),
      "1200 400 (512 171 (256 85 (256 85)(256 86))(256 85 (256 85)(432 144)))");
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, dynamic_bitvector_set_test) {
  ads::bv::DynamicBitVector<uint16_t, uint64_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.num_ones(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 1200);
  ASSERT_EQ(bv.num_ones(), 400);
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }

  for (size_t i = 0; i < 1200; i += 5) {
    bv.set(i, true);
  }
  size_t actual_ones = 0;
  for (size_t i = 0; i < 1200; ++i) {
    bool value = (i % 3 == 1) || (i % 5 == 0);
    if (value) ++actual_ones;
    ASSERT_EQ(bv[i], value);
  }
  ASSERT_EQ(bv.num_ones(), actual_ones);
}

TEST(ads_test_suite, dynamic_bitvector_set_bug_test) {
  ads::bv::DynamicBitVector<uint32_t, uint64_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.num_ones(), 0);

  for (size_t i = 0; i < 10000; ++i) {
    bv.insert(i, true);
  }
  ASSERT_EQ(bv.size(), 10000);
  ASSERT_EQ(bv.num_ones(), 10000);
  for (size_t i = 0; i < 10000; ++i) {
    ASSERT_EQ(bv[i], true);
    ASSERT_EQ(bv.rank(true, i), i);
    ASSERT_EQ(bv.rank(false, i), 0);
  }

  // Bug: Setting 1 to 0 should also change internal counters correctly
  for (size_t i = 0; i < 10000; ++i) {
    bv.set(i, i % 3 == 2);
  }
  ASSERT_EQ(bv.num_ones(), 3333);
  for (size_t i = 0; i < 10000; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 2);
    ASSERT_EQ(bv.rank(true, i), i / 3);
    ASSERT_EQ(bv.rank(false, i), i - i / 3);
  }
}

TEST(ads_test_suite, dynamic_bitvector_flip_test) {
  ads::bv::DynamicBitVector<uint16_t, uint16_t, 8, 16, 32> bv;
  ASSERT_EQ(bv.size(), 0);
  ASSERT_EQ(bv.num_ones(), 0);

  for (size_t i = 0; i < 1200; ++i) {
    bv.insert(i, i % 3 == 0);
  }
  ASSERT_EQ(bv.size(), 1200);
  ASSERT_EQ(bv.num_ones(), 400);
  for (size_t i = 0; i < 1200; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 0);
  }

  for (size_t i = 0; i < 1200; i += 5) {
    bv.flip(i);
  }
  size_t actual_ones = 0;
  for (size_t i = 0; i < 1200; ++i) {
    bool value = (i % 3 == 0) ^ (i % 5 == 0);
    if (value) ++actual_ones;
    ASSERT_EQ(bv[i], value);
  }
  ASSERT_EQ(bv.num_ones(), actual_ones);
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

TEST(ads_test_suite, dynamic_bitvector_delete_single_leaf_test) {
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 8, 16, 32> bv;
  for (size_t i = 0; i < 1000; ++i) {
    bv.insert(i, i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 1000);
  ASSERT_EQ(bv.num_ones(), 333);
  ASSERT_EQ(bv.get_tree_structure(), "1000 333 (1000 333)");

  for (int i = 999; i >= 0; i -= 4) {
    bv.delete_element(i);
  }
  ASSERT_EQ(bv.size(), 750);
  ASSERT_EQ(bv.num_ones(), 250);
  ASSERT_EQ(bv.get_tree_structure(), "750 250 (750 250)");
}

TEST(ads_test_suite, dynamic_bitvector_delete_merge_test) {
  srand(0);
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 8, 16, 32> bv;
  std::vector<bool> expected(5000, false);
  std::vector<bool> expected_after;
  for (size_t i = 0; i < 5000; ++i) {
    bool value = rand() % 3 == 0;
    bv.insert(i, value);
    expected[i] = value;
    if (i < 1000 || i >= 4000) {
      expected_after.push_back(value);
    }
  }
  ASSERT_EQ(bv.size(), 5000);
  for (size_t i = 0; i < 5000; ++i) {
    ASSERT_EQ(bv[i], expected[i]);
  }
  // Github pipeline uses other random number generator, sequences not identical
  // ASSERT_EQ(
  // bv.get_tree_structure(),
  // "5000 1642 (2048 674 (1024 340 (1024 340)(1024 334))(1024 346 (1024 "
  // "346)(1928 622)))");

  ads::bv::SimpleBitVector<uint64_t, uint64_t> a(0), b(0), c(0);
  for (size_t i = 0; i < 3000; ++i) {
    bv.delete_element(1000);

    for (size_t j = 0; j < 5000 - i - 1; ++j) {
      if (j < 1000) {
        ASSERT_EQ(bv[j], expected[j]);
      } else {
        ASSERT_EQ(bv[j], expected[j + i + 1]);
      }
    }

    // if (i == 100) {
    //   ASSERT_EQ(
    //       bv.get_tree_structure(),
    //       "4899 1606 (1947 638 (1000 333 (1000 333)(947 305))(1024 346 (1024
    //       " "346)(1928 622)))");
    // } else if (i == 536) {
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "4463 1453 (1511 485 (999 333 (999 333)(512 152))(1024 346 "
    //             "(1024 346)(1928 622)))");
    // } else if (i == 541) {
    //   // Before min leaf size is reached
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "4458 1450 (1506 482 (994 333 (994 333)(512 149))(1024 346 "
    //             "(1024 346)(1928 622)))");
    // } else if (i == 542) {
    //   // Stealing, (994 333)(512 149) -> (993 332)(512 150)
    //   // (delete zero, move one)
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "4457 1450 (1505 482 (993 332 (993 332)(512 150))(1024 346 "
    //             "(1024 346)(1928 622)))");
    // } else if (i == 1023) {
    //   // Reached min size of 512 for neighboring leafs (512 163)(512 178)
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "3976 1309 (1024 341 (512 163 (512 163)(512 178))(1024 346 "
    //             "(1024 346)(1928 622)))");
    // } else if (i == 1024) {
    //   ASSERT_EQ(
    //       bv.get_tree_structure(),
    //       "3975 1309 (1023 341 (1023 341)(1024 346 (1024 346)(1928 622)))");
    // } else if (i == 1564) {
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "3435 1123 (1000 333 (1000 333)(512 168 (512 168)(1923
    //             622)))");
    // } else if (i == 1565) {
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "3434 1123 (1000 333 (1000 333)(512 169 (512 169)(1922
    //             621)))");
    // } else if (i == 2000) {
    //   ASSERT_EQ(bv.get_tree_structure(),
    //             "2999 981 (1000 333 (1000 333)(512 164 (512 164)(1487
    //             484)))");
    // }
  }

  ASSERT_EQ(bv.size(), 2000);
  for (size_t i = 0; i < 2000; ++i) {
    ASSERT_EQ(bv[i], expected_after[i]);
  }

  std::vector<int> expected_pos;
  std::vector<int> actual_pos;
  for (int i = 0; i < 1000; ++i) {
    expected_pos.push_back(i);
    actual_pos.push_back(i);
  }
  for (int i = 4000; i < 5000; ++i) {
    expected_pos.push_back(i);
    actual_pos.push_back(i - 3000);
  }

  for (int i = 0; i < 2000; ++i) {
    int deleted_idx = rand() % (2000 - i);
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
}

TEST(ads_test_suite, dynamic_bitvector_delete_big_example_test) {
  // Build large dynamic bitvector
  srand(0);
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 32, 64, 128> bv;
  std::vector<bool> expected(10000, false);
  for (size_t i = 0; i < expected.size(); ++i) {
    bool value = rand() % 2 == 0;
    bv.insert(i, value);
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

TEST(ads_test_suite, dynamic_bitvector_push_pop_test) {
  ads::bv::DynamicBitVector<uint8_t, uint8_t, 1, 2, 4> bv;
  for (size_t i = 0; i < 100; ++i) {
    bv.push_back(i % 3 == 1);
  }
  ASSERT_EQ(bv.size(), 100);
  for (size_t i = 0; i < 100; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }

  for (size_t i = 0; i < 50; ++i) {
    bv.pop_back();
  }
  ASSERT_EQ(bv.size(), 50);
  for (size_t i = 0; i < 50; ++i) {
    ASSERT_EQ(bv[i], i % 3 == 1);
  }
}

TEST(ads_test_suite, dynamic_bitvector_space_used_test) {
  ads::bv::DynamicBitVector<uint64_t, uint64_t, 32, 64, 128> bv;
  for (size_t i = 0; i < 10000; ++i) {
    bv.push_back(i % 3 == 1);
  }
  ASSERT_EQ(bv.space_used(), 11712);
}

}  // namespace ads_test
