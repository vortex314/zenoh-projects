set -v
mkdir build
cd build
rm -rf ../build/*
cmake .. 
make VERBOSE=1 -j1
