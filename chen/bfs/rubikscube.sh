#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 2000000

#set the time limit to one hour
ulimit -t 600

# Maximum depth to be searched.
MAX_DEPTH="200"

# Path to which the results will be added
RESULTS=results_socs/rubiks


for RUN in {1..100}; do
	for PROBES in 50 60 70 80 90 100 150 200; do
		for META_PROBES in 100 500 1000 5000 10000 20000 30000 40000 50000 60000; do
			for SEARCH_NODES_PER_TYPE in 0 ; do
				for META_NODES_PER_TYPE in 0 ; do
		            OUTPUT="p"${PROBES}"_metap"${META_PROBES}"_nodes"${SEARCH_NODES_PER_TYPE}"_metanodes"${META_NODES_PER_TYPE}
		            printf "${RUN}","${PROBES}","${META_PROBES}","${SEARCH_NODES_PER_TYPE}","${META_NODES_PER_TYPE}","0","${MAX_DEPTH}\n" >> ${RESULTS}/${OUTPUT}
		            ./rubik3Sticker.cg ${PROBES} ${META_PROBES} ${SEARCH_NODES_PER_TYPE} ${META_NODES_PER_TYPE} 0 ${MAX_DEPTH} >> ${RESULTS}/${OUTPUT}	        
			    done
			done
		done
	done
done

echo

