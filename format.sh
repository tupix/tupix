#!/usr/bin/env sh
find . \( -name "*.c" -o -name "*.h" \) -a -exec clang-format -i '{}' ';'
