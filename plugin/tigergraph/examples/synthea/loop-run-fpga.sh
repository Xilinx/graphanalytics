#!/bin/bash

script_dir=$(dirname "$0")

. $script_dir/bin/common.sh

if [ "$load_fpga" -eq 1 ]; then
    echo "Load FPGA with accelerators"
    time gsql -g $xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga($devices_needed)"
fi

# do a basic multi-user test

for ((i=1; i <= $iterations ; i++)); do
    echo "------------------ iteration $i -------------------------------"
    for ((u=1; u <= 8; u++)); do
        echo "################ user $u ################"
        gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/log/fpgai$u.txt\", $devices_needed)" &
    done
    wait
done 
