lcov --directory build --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' '/opt/*' '*/gtest/*' --output-file coverage.info
lcov --list coverage.info
