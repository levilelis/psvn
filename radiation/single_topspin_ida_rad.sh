#!/bin/bash

# Set the limit to ~1 GB
ulimit -v 1000000

#set the time limit to 10 minutes
ulimit -t 600

cd /home/levi/psvn-work/radiation

# Path to which the results will be added
RESULTS=results/topspin

printf "${I}," >> ${RESULTS}/${C}/${INSTNAME}_${F}

./topspin_17_4.ida ../pdb/topspin/topspin_17_4_10_16 ${F} ${C} < ${PROBLEM} $* | awk '/total cost:/ {printf $3 "," }; /total expanded:/ {printf $3 "," }; /total generated:/ {printf $3 "," }; /total time:/ {printf $3}' >> ${RESULTS}/${C}/${INSTNAME}_${F}
	      
printf "\n" >> ${RESULTS}/${C}/${INSTNAME}_${F}

echo

