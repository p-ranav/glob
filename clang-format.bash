#!/usr/bin/env bash
find ./include ./samples ./src -type f \( -iname \*.cpp -o -iname \*.hpp \) | xargs clang-format -style="{ColumnLimit : 90}" -i
