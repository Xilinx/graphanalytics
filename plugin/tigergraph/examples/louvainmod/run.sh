#!/usr/bin/env bash

set -e

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

. $script_dir/bin/common.sh

#files=/proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx 

echo "Running schema.gsql"
time gsql schema.gsql

echo "Installing load.gsql"
time gsql -g social load.gsql

echo "Loading $files"
time gsql -g social "run loading job load_job USING file_name = \"$data_source\""

echo "Installing louvain_distributed_cpu query"
time gsql -g social louvain_distributed_cpu.gsql

echo "Installing louvain_alveo query"
time gsql -g social louvain_alveo.gsql

mkdir -p log
chmod a+w log

echo "Running louvain_distributed_cpu"
START=$(date +%s%3N)
time gsql -g social "run query louvain_distributed_cpu(10, [\"Person\"], [\"Coworker\"], \"$PWD/log/cpu_out.txt\")"
TOTAL_TIME=$(($(date +%s%3N) - START))

echo "louvain_cpu runtime: " $TOTAL_TIME
START=$(date +%s%3N)

#time gsql -g social 'run query load_alveo()'
#TOTAL_TIME=$(($(date +%s%3N) - START))
#echo "load_alveo: " $TOTAL_TIME
#START=$(date +%s%3N)
#time gsql -g social 'run query louvain_alveo(10, ["Person"], ["Coworker"], "alveo_out.txt")'
#TOTAL_TIME=$(($(date +%s%3N) - START))
#echo "louvain_alveo: " $TOTAL_TIME

