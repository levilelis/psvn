#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=resultshash_multipleprobes/gripper
OUTPUT=a_star_gripper20_pdb

for PROBES in 100; do
	for META_PROBES in 2000 3000 5000 10000 100000; do
		printf ${PROBES}" "${META_PROBES}"\n" >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
		(./gripper20.chenastar ${PROBES} ${META_PROBES} 0 0 0 ../../pdb/gripper/gripper20_pdb ../../problems/gripper/gripper20_1000 ../solutions/gripper/a_star_gripper20_pdb >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}) &
	done
done
	        
echo

