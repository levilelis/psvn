#!/bin/bash
#SBATCH --cpus-per-task=1   # maximum CPU cores per GPU request: 6 on Cedar, 16 on Graham.
#SBATCH --mem=16000M        # memory per node
#SBATCH --time=0-2:00      # time (DD-HH:MM)

module load gcc/4.8.5

./${ABSTRACTION_NAME}.distSummary ${ABSTRACTION_NAME}.state_map
rm -f ${ABSTRACTION_NAME}.c ${ABSTRACTION_NAME}.distSummary ${ABSTRACTION_NAME}.psvn
#make pdb ss=${PSVN_FILE} absname=${ABSTRACTION_NAME} < ${PDB_DETAILS}
