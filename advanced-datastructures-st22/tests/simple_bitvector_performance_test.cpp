#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "bv/simple_bitvector.hpp"
#include "util.hpp"

namespace ads_test {

template <class BlockType>
static inline int64_t time_simple_bitvector(size_t n) {
  // BitVector performance
  auto bitvector_start = std::chrono::high_resolution_clock::now();
  {
    // Creation and set
    ads::bv::SimpleBitVector<BlockType> bv(n);
    for (size_t i = 0; i < n; ++i) {
      bv.set(i, (i + 7) % 5 == 0);
    }

    // Flip every second value
    for (size_t i = 0; i < n; ++i) {
      if (i % 4 == 0) {
        bv.flip(i);
      }
    }

    // Randomly set all
    for (size_t i = 0; i < n / 1000; ++i) {
      bv.set(i + 340, i % 3 == 0);
      bv.set(i + 15, i % 5 == 0);
      bv.set(i + 41, i % 7 == 0);
      bv.set(i + 102, i % 2 == 0);
      bv.set(i + 740, i % 3 == 0);
      bv.set(i + 224, i % 8 == 0);
      bv.set(i + 833, i % 8 == 0);
      bv.set(i + 373, i % 2 == 0);
      bv.set(i + 9, i % 1 == 0);
      bv.set(i + 451, i % 5 == 0);
    }
  }
  auto bitvector_end = std::chrono::high_resolution_clock::now();
  return ads::util::time_diff(bitvector_start, bitvector_end);
}

static inline int64_t time_vector_bool(size_t n) {
  // Vector bool performance
  auto vector_bool_start = std::chrono::high_resolution_clock::now();
  {
    // Creation and set
    std::vector<bool> set_elements(n, false);
    for (size_t i = 0; i < n; ++i) {
      set_elements[i] = (i + 7) % 5 == 0;
    }

    // Flip every second value
    for (size_t i = 0; i < n; ++i) {
      if (i % 4 == 0) {
        set_elements[i].flip();
      }
    }

    // Randomly set all
    for (size_t i = 0; i < n / 1000; ++i) {
      set_elements[i + 340] = i % 3 == 0;
      set_elements[i + 15] = i % 5 == 0;
      set_elements[i + 41] = i % 7 == 0;
      set_elements[i + 102] = i % 2 == 0;
      set_elements[i + 740] = i % 3 == 0;
      set_elements[i + 224] = i % 8 == 0;
      set_elements[i + 833] = i % 8 == 0;
      set_elements[i + 373] = i % 2 == 0;
      set_elements[i + 9] = i % 1 == 0;
      set_elements[i + 451] = i % 5 == 0;
    }
  }
  auto vector_bool_end = std::chrono::high_resolution_clock::now();
  return ads::util::time_diff(vector_bool_start, vector_bool_end);
}

TEST(ads_test_suite, simple_bitvector_performance) {
#ifdef NDEBUG
  size_t n = 100000000;
  size_t reps = 20;
#else
  size_t n = 1000000;
  size_t reps = 5;
#endif

  // Check performance
  int64_t vector_bool_time = 0;
  for (size_t i = 0; i < reps; ++i) vector_bool_time += time_vector_bool(n);
  int64_t bitvector_8_time = 0;
  for (size_t i = 0; i < reps; ++i)
    bitvector_8_time += time_simple_bitvector<uint8_t>(n);
  int64_t bitvector_16_time = 0;
  for (size_t i = 0; i < reps; ++i)
    bitvector_16_time += time_simple_bitvector<uint16_t>(n);
  int64_t bitvector_32_time = 0;
  for (size_t i = 0; i < reps; ++i)
    bitvector_32_time += time_simple_bitvector<uint32_t>(n);
  int64_t bitvector_64_time = 0;
  for (size_t i = 0; i < reps; ++i)
    bitvector_64_time += time_simple_bitvector<uint64_t>(n);
  std::cout << "--------------------------------------------\n"
            << "Vector<bool>: " << vector_bool_time << "ms\n"
            << "SimpleBitVector<uint8_t>: " << bitvector_8_time << "ms\n"
            << "SimpleBitVector<uint16_t>: " << bitvector_16_time << "ms\n"
            << "SimpleBitVector<uint32_t>: " << bitvector_32_time << "ms\n"
            << "SimpleBitVector<uint64_t>: " << bitvector_64_time << "ms\n"
            << "--------------------------------------------" << std::endl;
}

}  // namespace ads_test
