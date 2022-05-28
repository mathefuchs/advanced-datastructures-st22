project(AdvancedDatastructuresST22Tests VERSION 0.1)

enable_testing()

include_directories(../src)
add_executable(ads_test
  simple_bitvector_test.cpp
  simple_bitvector_performance_test.cpp
  dynamic_bitvector_test.cpp
  bv_integration_test.cpp
)
target_link_libraries(ads_test gtest gtest_main)

# Add test suite
add_test(ads_test ads_test_suite)

# Code coverage
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(ads_test INTERFACE -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(ads_test INTERFACE -lgcov)
endif()
