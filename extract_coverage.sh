lcov --directory build/advanced-datastructures-st22 --capture --output-file coverage.info --no-external
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info
