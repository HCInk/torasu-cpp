#!/bin/bash
if [ -n "$1" ]; then

	if [ "$1" == "wincross" ]; then

		echo "Building cross via MinGW-CMake for windows"
		mkdir -p build/cross/win/
		cd build/cross/win/
		x86_64-w64-mingw32-cmake -Wno-dev ../../../
		make

	elif [ "$1" == "delcross" ]; then
	
		echo "wiping cross build--folder..."
		rm -r build/cross

	else

		echo "Unknown argument \"$1\"!"
		echo "Available arguments: "
		echo "	wincross - Builds windows binary into build/cross/win/"
		echo "	delcross - Removes cross build-folder (build/cross/)"
		echo "No arguments will just run a normal build."

	fi

else

	mkdir -p build
	cd build
	cmake -Wno-dev ../
	make

fi