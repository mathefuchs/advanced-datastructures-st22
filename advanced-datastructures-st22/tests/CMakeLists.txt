project(AdvancedDatastructuresST22Tests VERSION 0.1)

enable_testing()

include_directories(../src)

# Unit tests
add_executable(ads_test
  simple_bitvector_test.cpp
  simple_bitvector_performance_test.cpp
  dynamic_bitvector_test.cpp
  dynamic_bp_tree_test.cpp
  simple_bitvector_excess_test.cpp
  dynamic_bitvector_excess_test.cpp
  simple_tree_test.cpp
  bv_integration_test.cpp
  bp_integration_test.cpp
)
target_link_libraries(ads_test PUBLIC coverage_config gtest gtest_main)
add_test(ads_test ads_test_suite)

# Reliability tests with large inputs 
add_executable(ads_reliability_test
  reliability_test.cpp
)
target_link_libraries(ads_reliability_test PUBLIC coverage_config gtest gtest_main)
add_test(ads_reliability_test ads_reliability_test_suite)
