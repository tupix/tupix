#!/usr/bin/env bash
find . \( -name "*.c" -o -name "*.h" \) -a -exec clang-format -i '{}' ';'
