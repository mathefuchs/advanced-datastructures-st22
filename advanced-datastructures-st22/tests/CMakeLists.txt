project(AdvancedDatastructuresST22Tests VERSION 0.1)

enable_testing()

include_directories(../src)
add_executable(ads_test
  simple_bitvector_test.cpp
  simple_bitvector_performance_test.cpp
  dynamic_bitvector_test.cpp
)
target_link_libraries(ads_test gtest gtest_main)

# Code coverage
target_compile_options(ads_test PRIVATE --coverage -fprofile-arcs -ftest-coverage)
target_link_options(ads_test PRIVATE -lgcov)

add_test(ads_test ads_test_suite)
