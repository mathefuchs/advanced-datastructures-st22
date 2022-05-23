set -x

git pull
git submodule update --init --recursive
mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make -j
