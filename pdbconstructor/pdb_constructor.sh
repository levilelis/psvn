#!/bin/bash

#PBS -N pdb_constructor
#PBS -l nodes=1:ppn=1,mem=23gb,walltime=10:00:00
#PBS -q qtime

# Set the limit to ~1 GB
#ulimit -v 20000000

#set the time limit to 10 minutes
#ulimit -t 7200

cd /home/ps11489/dados/psvn/pdbconstructor

make pdb ss=${PSVN_FILE} absname=${ABSTRACTION_NAME} < ${PDB_DETAILS}
