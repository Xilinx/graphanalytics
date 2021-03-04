#!/bin/bash

set -x

. common.sh

if [ "$3" != "-noload_fpga" ]
then
    echo "Load FPGA with accelerators"
    time gsql -g $xgraph "set query_timeout=240000000 run query loadgraph_cosinesim_ss_fpga()"
fi

LIMIT=1
for ((a=1; a <= LIMIT ; a++))
do    
    time gsql -g $xgraph "set query_timeout=240000000 run query cosinesim_ss_fpga(\"$PWD/log/fpga.txt\")"
done 
