#!/usr/bin/env bash
set -e
set -x

cd $(realpath $(dirname $0))

clang++ \
    @../compiler_flags.txt \
    -I. \
    -o test \
    test.cc ../*.cc

./test ./test
