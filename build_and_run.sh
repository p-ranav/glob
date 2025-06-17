#!/usr/bin/env bash
cmake -Hall -Bbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --parallel
./build/standalone/glob --help
