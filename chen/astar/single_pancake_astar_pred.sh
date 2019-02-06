#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

cd /home/levi/psvn-work/chen/astar

# Path to which the results will be added
RESULTS=resultshashtable/pancake
OUTPUT=a_star_pancake15_pdb_0_1_2_3_4_5_6_7

printf ${PROBES}" "${META_PROBES}"\n" >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
./pancake15.chenastar ${PROBES} ${META_PROBES} 0 0 0 ../../pdb/pancake/pancake15_pdb_0_1_2_3_4_5_6_7 ../../problems/pancake/pancake15_1000 ../solutions/pancake/a_star_pancake15_pdb_0_1_2_3_4_5_6_7 >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
	        
echo

