rm -fr build

echo "--- Building release version ; using pthreads ---"
cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE=Release -S . -B cmake-build-release
cmake --build cmake-build-release

echo "--- Building debug version ; using pthreads ---"
cmake --fresh -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B cmake-build-debug
cmake --build cmake-build-debug
