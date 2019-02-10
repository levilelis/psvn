#!/bin/bash

#example of usage: ./sbatch_pdb_constructor.sh sliding_tile4x4_costs_100000_999999 sliding_tile4x4_costs_100000_999999_12345 pdb_4x4_12345
mkdir -p output
sbatch --output=output/${2}-%N-%j.out --export=PSVN_FILE=${1},ABSTRACTION_NAME=${2},PDB_DETAILS=${3} run_pdb_constructor.sh
