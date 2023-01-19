#!/bin/bash

# Test performance of mdu program
##### Constants
export TIMEFORMAT="%3R"

# create file to write to
echo -n "" > output.txt
printf "Threads\tTime (ms)\n" >> output.txt # Add column headers

# perform test with 1 - 100 threads
echo "Test initialized, please wait."
for n in {1..100};
do
	{ time ./mdu -j $n /pkg/ > /dev/null ; } 2>&1 | awk -v n="$n" '!/mdu/ {printf("%d\t%d\n",n,$1*1000)}' >> output.txt
done

echo "Test completed, see output.txt for test results"
