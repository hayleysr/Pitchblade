#!/bin/bash
rm -rf build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release