#!/usr/bin/env bash

pushd $(dirname ${BASH_SOURCE[0]}) >/dev/null 2>&1

if [[ ! -f bin/main ]]
then
    echo -e "binary file \033[0;31mnot found\033[0m"
    exit 1
fi
./bin/main

popd >/dev/null # 2>&1

