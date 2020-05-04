#!/bin/bash
mkdir -p build
cd build
cmake -Wno-dev ../
make
