rm -fr build

echo "--- Building release version ; using native threading library introduced with C++ 11 ---"
cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE=Release -DUSE_CPP11_THREADS= -S . -B cmake-build-release-cpp-threads
cmake --build cmake-build-release-cpp-threads

echo "--- Building debug version ; using native threading library introduced with C++ 11 ---"
cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE=Debug -DUSE_CPP11_THREADS= -S . -B cmake-build-debug-cpp-threads
cmake --build cmake-build-debug-cpp-threads
