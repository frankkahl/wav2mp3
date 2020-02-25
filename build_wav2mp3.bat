@echo off
pushd "%~dp0"
echo "--- Cleaning up ---"
del /F /S /Q build
del /F /S /Q  CMakeFiles
del /F CMakeCache.txt

echo "--- Building release version ; using pthreads ---"
cmake  -G "Visual Studio 16 2019" -A x64 CMakeLists.txt
devenv wav2mp3.sln /Rebuild Release
devenv wav2mp3.sln /Rebuild Debug
popd
@echo on
