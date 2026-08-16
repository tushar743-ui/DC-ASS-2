#!/bin/bash
cd "$(dirname "$0")/.."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release > /dev/null
make -j4
cd ..
./build/server