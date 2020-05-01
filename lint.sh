#!/bin/bash
files=$(cat .ci/sourcelist.txt | tr "\n" " ") # Read files to lint
filters=$(grep -o '^[^#]*' .ci/cpplintfilter.txt | tr "\n" ",") # read filters with periods instead of newlines
filters=${filters// } # remove whitespaces
cpplint --extensions=cpp,cxx,cc,c,c++ --headers=h,hpp,hxx --filter=$filters $files # lint with given filters
