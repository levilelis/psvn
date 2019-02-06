#!/bin/bash

# Set the limit to ~2 GB
ulimit -v 6000000

#set the time limit to one hour
#ulimit -t 600

# Path to which the results will be added
RESULTS=results/hanoi

./hanoi_4p_12d.astar_so hanoi_4p_12d_1.abst < hanoi_4p_12d_1000 >> ${RESULTS}/a_star_hanoi_4p_12d_1000_abst_1
	        
echo

