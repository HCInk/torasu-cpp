#!/bin/bash

function build_runMake() {
	if [ -f  "$(command -v nproc)" ]; then
		threads=$(nproc --all)
		echo "Building multithreaded ($threads)..."
		make --jobs=$threads
	else
		echo "Building singlethreaded..."
		make
	fi
}

if [ -n "$1" ]; then

	if [ "$1" == "delbuild" ]; then
	
		echo "Deleting build-folder..."
		rm -r build

	elif [ "$1" == "wincross" ]; then

		echo "Building cross via MinGW-CMake for windows"
		mkdir -p build/cross/win/
		cd build/cross/win/
		x86_64-w64-mingw32-cmake -Wno-dev ../../../
		build_runMake

	elif [ "$1" == "delcross" ]; then
	
		echo "Wiping cross build-folder..."
		rm -r build/cross


	else

		echo "Unknown argument \"$1\"!"
		echo "Available arguments: "
		echo "	delbuild - Deletes all buld files (build/)"
		echo "	wincross - Builds windows binary into build/cross/win/"
		echo "	delcross - Removes cross build-folder (build/cross/)"
		echo "No arguments will just run a normal build."

	fi

else

	mkdir -p build
	cd build
	cmake -Wno-dev ../
	build_runMake

fi