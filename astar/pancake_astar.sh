#!/bin/bash

# Set the limit to ~8 GB
ulimit -v 8000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=results/pancake

./pancake15.astar_so ../pdb/pancake/pancake15_pdb_0_1_2_3_4_5_6_7.abst < ../problems/pancake/pancake15_1000 >> ${RESULTS}/a_star_pancake15_pdb_0_1_2_3_4_5_6_7
	        
echo

