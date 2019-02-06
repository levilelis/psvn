#!/bin/bash

# Set the limit to ~8 GB
ulimit -v 12000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=results/gripper

./gripper20.astar_so ../pdb/gripper/gripper20_pdb.abst < ../problems/gripper/gripper20_1000 >> ${RESULTS}/a_star_gripper20_pdb
	        
echo

