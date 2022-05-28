lcov --directory build --capture --output-file coverage.info
lcov --list coverage.info

curl -Os https://uploader.codecov.io/latest/linux/codecov
chmod +x codecov
./codecov
