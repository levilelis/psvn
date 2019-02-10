#!/bin/bash

#example of usage: ./qsub_pdb_constructor.sh sliding_tile4x4_costs sliding_tile4x4_cost_1234 pdb_4x4_1234
qsub -v PSVN_FILE=$1,ABSTRACTION_NAME=$2,PDB_DETAILS=$3 pdb_constructor.sh
