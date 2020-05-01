#!/bin/bash
files=$(cat .ci/sourcelist.txt | tr "\n" " ") # Read files to format

flags=$(grep -o '^[^#]*' .ci/formatflags.txt | tr "\n" " ") # read flags with spaces instead of newlines

astyle $flags $files