#!/bin/bash
#PBS -N LTS_puzzle
#PBS -l nodes=1:ppn=1,mem=16gb,walltime=0:10:00
#PBS -q qtime

# Set the limit to ~10 GB
#ulimit -v 10000000

#set the time limit to 10 minutes
ulimit -t 600

cd /home/ps11489/dados/psvn/lts/

# Path to which the results will be added
RESULTS=results/puzzle/lts

echo ${PDB}
echo ${PROBLEM}
./sliding_tile4x4_costs_1_200.idlts ${PDB} < ${PROBLEM} >> ${RESULTS}/${INSTNAME}
