#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

cd /home/levi/psvn-work/chen/astar

# Path to which the results will be added
RESULTS=resultshashtable/hanoi
OUTPUT=a_star_hanoi_4p_12d_1000_abst_1_2

printf ${PROBES}" "${META_PROBES}"\n" >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
./hanoi_4p_12d.chenastar ${PROBES} ${META_PROBES} 0 0 0 ../../pdb/hanoi/hanoi_4p_12d_1 ../../problems/hanoi/hanoi_4p_12d_1000 ../solutions/hanoi/a_star_hanoi_4p_12d_1000_abst_1 >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
	        
echo

