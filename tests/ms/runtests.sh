#!/usr/bin/bash

for file in $(find . -name "ms.*.log");
do
	time=$({ /usr/bin/time -v ../../build/engine $file; } 2>&1 | grep "User time" | awk '{print $NF}')
	echo $file $time
done
