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

echo "Installing louvain_alveo queries"
time gsql -g social query_louvain_alveo.gsql

mkdir -p log
chmod a+w log

echo "Running louvain_distributed_cpu"
START=$(date +%s%3N)
time gsql -g social "run query louvain_distributed_cpu(10, [\"Person\"], [\"Coworker\"], \"$PWD/log/cpu_out.txt\")"
TOTAL_TIME=$(($(date +%s%3N) - START))

echo "louvain_cpu runtime: " $TOTAL_TIME

run_alveo=1
if [ "$run_alveo" -eq 1 ]; then
    START=$(date +%s%3N)
    time gsql -g social 'run query load_alveo()'
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "load_alveo: " $TOTAL_TIME
    
    START=$(date +%s%3N)
    #command example
    #time gsql -g social "run query louvain_alveo(10, [\"Person\"], [\"Coworker\"], \"$PWD/log/alveo_out.txt\", \"$PWD/as-skitter/as-skitter-wt-e110k.mtx\", \"$PWD/as-skitter/as-skitter-partitions/louvain_partitions\")"

    time gsql -g social "run query louvain_alveo(10, [\"Person\"], [\"Coworker\"], \
                                                 \"$PWD/log/alveo_out.txt\", \
                                                 \"$data_source\", \
                                                 \"$partition_prj\")"
    TOTAL_TIME=$(($(date +%s%3N) - START))
    echo "louvain_alveo: " $TOTAL_TIME
fi

