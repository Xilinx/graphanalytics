#!/bin/bash

echo Running $0
if [ "$#" -lt 6 ]; then
    echo "$0 <.mtx file> <partition project base name> <number of partitions> <numDevices> <numWorkers> <max_level>"
    echo "Example: $0 /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 3 2"
    exit 1
fi

. env.sh

graph=$1
projdir=$2.par.proj
par=$3
num_dev=$4
num_workers=$5
numlevel=$6
opt_out=
if [ "$#" -eq 6 ]; then
    opt_out="-o $6"
fi

workers="tcp://192.168.1.21:5555 tcp://192.168.1.31:5555"
#workers="tcp://10.18.5.112:5555 tcp://10.18.5.113:5555"
# Set rundir to your dir
xclbinfile=/proj/autoesl/ryanw/kernel_louvain_pruning.xclbin

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 

cmd="../$exe_dir/louvainmod_test -x $xclbinfile $graph -fast -dev $num_dev -num_level $numlevel -par_num $par \
          -load_alveo_partitions $projdir -setwkr $num_workers $workers -driverAlone $opt_out"
echo $cmd
$cmd
