#!/usr/bin/env bash
cmake -Hall -Bbuild
cmake --build build
./build/standalone/glob --help
