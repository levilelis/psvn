#!/bin/bash

#directory in which the problem instances are
dir=../problem-singles/puzzle_4x4/
pdb=../pdb/sliding_tile4x4_costs_1_200_123456

for PROBLEM in `dir -d $dir*.pro` ; do
	INST=$(echo ${PROBLEM} | sed 's/.*\///')
	INSTNAME=${INST%.*}
	(qsub -v PDB=$pdb,PROBLEM=$PROBLEM,INSTNAME=$INSTNAME single_puzzle_lts.sh) &
done


