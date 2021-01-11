#!/bin/bash
files=$(cat .ci/sourcelist.txt | tr "\n" " ") # Read files to lint
filters=$(grep -o '^[^#]*' .ci/cpplintfilter.txt | tr "\n" ",") # read filters with periods instead of newlines
filters=${filters// } # remove whitespaces

echo "// Ignored errors when linting " > .ci/cpplint-ignored-errors.txt
# lint with given filters
cpplint --extensions=cpp,cxx,cc,c,c++ --headers=h,hpp,hxx --filter=$filters $files \
2> >(while read -r line ; do
	if [[ $line == *"[runtime/explicit]"* ]]; then
		srcLine=`sed -n "$(cut -f 2 -d : <<< $(echo $line))p" $(cut -f 1 -d ":" <<< $(echo $line))`

		if [[ $srcLine == *"/* implicit */"* ]]; then
			echo "$srcLine     // $line" >> .ci/cpplint-ignored-errors.txt
		else
    		>&2 echo "$line"
		fi
	else
    	>&2 echo "$line"
	fi

done)

ignored=`wc -l .ci/cpplint-ignored-errors.txt | cut -f 1 -d " "`
((ignored--))
echo "(Ignored $ignored)"
