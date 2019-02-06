#!/bin/bash

#directory in which the problem instances are
dir=$1

for PROBLEM in `dir -d $dir*.pro` ; do
	INST=$(echo ${PROBLEM} | sed 's/.*\///')
	INSTNAME=${INST%.*}
	(qsub -l select=1:ncpus=1:mem=1GB -v PROBLEM=$PROBLEM,INSTNAME=$INSTNAME single_topspin_dfid.sh) &
done


