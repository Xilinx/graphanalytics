#!/bin/bash

set -x

. common.sh

if [ "$load_fpga" -eq 1 ]; then
    echo "Load FPGA with accelerators"
    time gsql -g $xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga($devices_needed)"
fi

for ((a=1; a <= $iterations ; a++))
do    
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/log/fpga.txt\", $devices_needed)"
done 
