#!/bin/bash

set -x

. common.sh

devices_needed=2

if [ "$3" != "-noload_fpga" ]
then
    echo "Load FPGA with accelerators"
    time gsql -g $xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga($devices_needed)"
fi

LIMIT=1000
for ((a=1; a <= LIMIT ; a++))
do    
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/log/fpga.txt\", $devices_needed)"
done 
