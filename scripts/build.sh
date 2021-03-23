#!/usr/bin/env bash

pushd $(dirname ${BASH_SOURCE[0]}) >/dev/null 2>&1

cd ..
[[ -d bin/ ]] || mkdir bin
g++ --std=c++17 -pthread -o bin/main main.cpp

popd >/dev/null 2>&1

