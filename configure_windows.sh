# USAGE: sh .\ configure_windows.sh

rm -rf build
rm -f CMakeCache.txt

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -T host=x64

# Run only this to rebuild the plugin and not all of the configuration.
cmake --build build