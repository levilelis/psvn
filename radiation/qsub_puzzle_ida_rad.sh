#!/bin/bash

#maximum number of jobs submitted to cluster before sleeping
MAXSUB=70
#directory in which the problem instances are
dir=$1
#counter of submitted jobs
count=1

for (( i=1; i <= 30; i++ )) ; do
	echo "$i"
	for F in 1 10 100 1000 10000 100000; do
		for C in 'BoundedVoteAbstraction'; do
			for PROBLEM in `dir -d $dir*.pro` ; do
				INST=$(echo ${PROBLEM} | sed 's/.*\///')
				INSTNAME=${INST%.*}

				qsub -l select=1:ncpus=1:mem=1GB -v I=$i,F=$F,PROBLEM=$PROBLEM,INSTNAME=$INSTNAME,C=$C single_puzzle_ida_rad.sh
				
				if [ "$count" == "$MAXSUB" ]; then
					let "count=1"
					
					echo "sleeping (11 minutes)... you are executing the sliding-tile puzzle"
                                        sleep 660
                                        let "count=1"

					q=$(qselect -u levi -s Q | wc -l)

					while [ "$q" -gt "50" ]
					do
						echo "queue isn't empty, sleeping for 11 minutes... you are running the sliding-tile puzzle"
						echo "$q"
						sleep 660
						q=$(qselect -u levi -s Q | wc -l)
					done
				fi

				let "count+=1"
			done
		done
	done
done
