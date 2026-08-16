cd remote-logger
mkdir build && cd build
cmake .. && make

# Terminal 1
./server

# Terminal 2, 3, 4 (different app types)
./client