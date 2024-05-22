#!/usr/bin/bash

for file in $(find . -name "treiber.*.log");
do
	time=$({ /usr/bin/time -v ../../build/engine $file; } 2>&1 | grep "User time" | awk '{print $NF}')
	numOp=$(wc -l $file | awk '{print $1}')
	echo $file $numOp $time
done
