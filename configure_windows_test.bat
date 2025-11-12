@echo off
REM Builds and runs the unit tests

cmake --build build --target runTests
if %errorlevel% neq 0 goto :eof

cd build
ctest
cd ..