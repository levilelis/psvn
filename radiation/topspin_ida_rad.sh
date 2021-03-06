#!/bin/bash

# Mapping of names in the code to names in the paper
# Bounded -> Pessimistic
# BoundedVoteAbstraction -> CMCD
# BoundedHamming -> PMCD
# None -> IDA*

# For F=0 you will run the code without corruptions, thus it could be run only once as there is no variance

#directory in which the problem instances are
dir=$1
#counter of submitted jobs
count=1

for (( i=1; i <= 30; i++ )) ; do
	for F in 1 10 100 1000 10000 100000 1000000 10000000; do
		for C in 'Bounded' 'BoundedVoteAbstraction' 'BoundedHamming' 'None'; do
			for PROBLEM in `dir -d $dir*.pro` ; do
				./topspin_17_4.ida ../pdb/topspin/topspin_17_4_10_16 ${F} ${C} < ${PROBLEM}			
			done
		done
	done
done
