#!/bin/sh

if cmake --build build -j --config Debug --target torasu-cpp-test; then
	echo "Running tests..."
	./build/torasu-cpp-test
else 
	echo "Build of tests failed!"
fi