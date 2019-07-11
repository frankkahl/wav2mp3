rm -fr build

echo "--- Building release version ; using pthreads ---"
rm -fr CMakeFiles
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release CMakeLists.txt
make

echo "--- Building debug version ; using pthreads ---"
rm -fr CMakeFiles
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Debug CMakeLists.txt
make
