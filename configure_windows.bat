@echo off
REM USAGE: sh .\ configure_windows.sh

DEL /S /F /Q "build"
DEL  "CMakeCache.txt"

cmake "-S" "." "-B" "build" "-G" "Visual Studio 17 2022" "-A" "x64" "-T" "host=x64"

REM Run only this to rebuild the plugin and not all of the configuration.
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake "--build" "build" --target Pitchblade

