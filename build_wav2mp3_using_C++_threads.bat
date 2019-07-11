@echo off
pushd "%~dp0"
echo "--- Cleaning up ---"
del /F /S /Q build
del /F /S /Q  CMakeFiles
del /F CMakeCache.txt

echo "--- Building release version ; using native threading library introduced with C++ 11 ---"
cmake  -G "Visual Studio 15 2017 Win64" -DUSE_CPP11_THREADS= CMakeLists.txt
devenv wav2mp3.sln /Rebuild Release
devenv wav2mp3.sln /Rebuild Debug
popd
@echo on
