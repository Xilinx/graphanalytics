#!/bin/bash

echo Running $0
if [ "$#" -lt 6 ]; then
    echo "$0 <.mtx file> <partition-dir> <number of partitions> numDevices numWorkers"
    echo "Example: $0 /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9 3 2"
    exit 1
fi

. env.sh

graph=$1
subdir=$2
par=$3
num_dev=$4
num_workers=$5
opt_out=
if [ "$#" -eq 6 ]; then
    opt_out="-o $6"
fi

workers="tcp://192.168.1.21:5555 tcp://192.168.1.31:5555"
# Set rundir to your dir
rundir=$subdir/louvain_partitions
projdir=$rundir.par.proj
xclbinfile=/proj/autoesl/ryanw/kernel_louvain_pruning.xclbin

exe_dir="Release"
if [ "$DEBUG" -eq 1 ]; then
    exe_dir="Debug"
fi 

#make
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/coPapersDBLP-wt.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/europe_osm-wt100M.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt600M.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt900M.mtx -fast -dev 3 -par_num 18 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt400M_2.mtx -fast -dev 3 -par_num 9 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt1350M.mtx -fast -dev 3 -par_num 27 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_gh.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt600M.mtx  -dev 3 -par_num 12 -driver

# e.g. ./run_driver.sh /proj/gdba/datasets/louvain-graphs/as-Skitter-wt.mtx as-skitter-par9 9
cmd="../$exe_dir/louvainmod_test -x $xclbinfile $graph -fast -dev $num_dev -par_num $par \
          -load_alveo_partitions $projdir -setwkr $num_workers $workers -driverAlone $opt_out"
echo $cmd
$cmd
