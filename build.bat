@echo off
setlocal

echo -------------------------------------
echo Building with CMake

cd build
cmake ..
cmake --build .

echo -------------------------------------