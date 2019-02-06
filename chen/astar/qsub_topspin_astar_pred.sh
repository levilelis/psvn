#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

for P in 100; do
	for MP in 1000 2000 3000 5000 7000 10000 11000 12000 13000 14000 15000; do
		(qsub -l select=1:ncpus=1:mem=3GB -v PROBES=$P,META_PROBES=$MP single_topspin_astar_pred.sh) &
	done
done
	        
echo

