#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

for P in 100; do
	for MP in 2000 3000 4000 5000 10000 100000 110000 120000 130000 140000 150000; do
		(qsub -l select=1:ncpus=1:mem=3GB -v PROBES=$P,META_PROBES=$MP single_gripper_astar_pred.sh) &
	done
done
	        
echo

