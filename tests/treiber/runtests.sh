#!/usr/bin/bash

for i in {1..10};
do
	echo -n "Generating testcases...[$i/10]"$'\r'
  numOp=$(( 10000 * $i ))
	for j in {1..3};
	do
		./prodcon-treiber -producers=2 -consumers=2 -operations=$numOp -log_operations=1 -print_summary=0 -shuffle_threads=1 > temp.log
		./format_history.py temp.log treiber.$i$j.log
		rm -f temp.log
	done
done

echo "Starting PolyLin on testcases... find dump in report.txt"
echo filename,number of operations,time taken > report.txt
for i in {1..10};
do
	echo -n "Running testcases...[$i/10]"$'\r'
	for j in {1..3};
	do
	  file=treiber.$i$j.log
		time=$({ /usr/bin/time -v ../../build/engine $file; } 2>&1 | grep "User time" | awk '{print $NF}')
		numOp=$(wc -l $file | awk '{print $1}')
		echo $file,$numOp,$time >> report.txt
	done
done
