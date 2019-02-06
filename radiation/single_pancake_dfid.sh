#!/bin/bash

# Set the limit to ~1 GB
ulimit -v 1000000

#set the time limit to 10 minutes
ulimit -t 600

cd /home/levi/psvn-work/radiation

# Path to which the results will be added
RESULTS=results/pancake/dfid

./pancake15.dfid < ${PROBLEM}  $* | awk '/total cost:/ {printf $3 "," }; /total expanded:/ {printf $3 "," }; /total generated:/ {printf $3 "," }; /total time:/ {printf $3}' >> ${RESULTS}/${INSTNAME}
