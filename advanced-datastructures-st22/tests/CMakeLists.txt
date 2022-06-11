project(AdvancedDatastructuresST22Tests VERSION 0.1)

enable_testing()

include_directories(../src)
add_executable(ads_test
  simple_bitvector_test.cpp
  simple_bitvector_performance_test.cpp
  dynamic_bitvector_test.cpp
  bv_integration_test.cpp
  dynamic_bp_tree_test.cpp
  simple_bitvector_excess_test.cpp
  dynamic_bitvector_excess_test.cpp
)
target_link_libraries(ads_test PUBLIC coverage_config gtest gtest_main)

# Add test suite
add_test(ads_test ads_test_suite)
