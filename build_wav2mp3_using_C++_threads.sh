rm -fr build

echo "--- Building release version ; using native threading library introduced with C++ 11 ---"
rm -fr CMakeFiles
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_CPP11_THREADS= CMakeLists.txt
make

echo "--- Building debug version ; using native threading library introduced with C++ 11 ---"
rm -fr CMakeFiles
rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_CPP11_THREADS= CMakeLists.txt
make
