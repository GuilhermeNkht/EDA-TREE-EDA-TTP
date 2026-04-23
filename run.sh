#!/bin/bash
set -e

cd "$(dirname "$0")"

make clean
make all
./test_all.sh