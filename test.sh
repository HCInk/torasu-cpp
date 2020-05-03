#!/bin/sh

if cmake --build build --config Debug --target torasu-cpp-test; then
	./build/torasu-cpp-test
else 
	echo "Build of tests failed!"
fi