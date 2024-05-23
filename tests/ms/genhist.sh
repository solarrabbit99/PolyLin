#!/usr/bin/bash

for i in {1..10};
do
  numOp=$(( 10000 * $i ))
	for j in $(seq 0 4);
	do
		./prodcon-ms -producers=2 -consumers=2 -operations=$numOp -log_operations=1 -print_summary=0 -shuffle_threads=1 > temp.log
		./format_history.py temp.log ms.$i$j.log
		rm -f temp.log
	done
done