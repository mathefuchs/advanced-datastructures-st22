set -x

lcov --directory build --capture --output-file coverage.info
lcov --list coverage.info

sh <(curl -s https://codecov.io/bash) -f coverage.info || echo "Codecov did not collect coverage reports"
