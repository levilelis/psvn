#!/bin/bash

#maximum number of jobs submitted to cluster before sleeping
MAXSUB=70
#directory in which the problem instances are
dir=$1
#counter of submitted jobs
count=1

for (( i=1; i <= 30; i++ )) ; do
	for F in 1 10 100 1000 10000 100000; do
		for C in 'BoundedVoteAbstraction'; do
			for PROBLEM in `dir -d $dir*.pro` ; do
				INST=$(echo ${PROBLEM} | sed 's/.*\///')
				INSTNAME=${INST%.*}

				qsub -l select=1:ncpus=1:mem=1GB -v I=$i,F=$F,PROBLEM=$PROBLEM,INSTNAME=$INSTNAME,C=$C single_topspin_ida_rad.sh
				
				if [ "$count" == "$MAXSUB" ]; then
					echo "sleeping (11 minutes)... your're running topspin."
					sleep 660
					let "count=1"
			
					q=$(qselect -u levi -s Q | wc -l)
                                        
					while [ "$q" -gt "50" ]
                                        do
                                                echo "queue isn't empty, sleeping for 3 minutes... you are running the topspin puzzle"
                                                echo "$q"
                                                sleep 180
                                                q=$(qselect -u levi -s Q | wc -l)
                                        done
				fi

				let "count+=1"
			done
		done
	done
done
