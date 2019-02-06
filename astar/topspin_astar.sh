#!/bin/bash

# Set the limit to ~8 GB
ulimit -v 8000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=results/topspin

./topspin_17_4.astar_so ../pdb/topspin/topspen_17_4_pdb.abst < ../problems/topspin/topspin_17_4_1000 >> ${RESULTS}/topspen_17_4_pdb
	        
echo

