#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=resultshash_multipleprobes/topspin
OUTPUT=topspen_17_4_pdb

for PROBES in 100; do
	for META_PROBES in 1000 2000 3000 4000 5000; do
		printf ${PROBES}" "${META_PROBES}"\n" >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
		./topspin_17_4.chenastar ${PROBES} ${META_PROBES} 0 0 0 ../../pdb/topspin/topspen_17_4_pdb ../../problems/topspin/topspin_17_4_1000 ../solutions/topspin/topspen_17_4_pdb >> ${RESULTS}/${OUTPUT}_${PROBES}_${META_PROBES}
	done
done
	        
echo

