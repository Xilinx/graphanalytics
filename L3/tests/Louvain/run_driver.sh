#!/bin/bash
echo Running $0
if [ "$#" -ne 3 ]; then
    echo "$0 <.mtx file> <subdir for saved partitions> <number of partitions>. Example: $0 /proj/isimsj/graphdb/louvain/data/europe_osm-wt900M.mtx 900 18"
    echo "create_partitions.sh saves partitions at rootdir/<subdir>. rootdir is set in each of create_partitions.sh, run_driver.sh and run_worker.sh and must be same in all three scripts"
    exit 1
fi
. env.sh
rootdir=/proj/gdba/ywu/ghe/poc_louvain
graph=$1
subdir=$2
par=$3
num_dev=3
num_workers=2
workers="tcp://192.168.1.21:5555 tcp://192.168.1.31:5555"
# Set rundir to your dir
rundir=$rootdir/$subdir/louvain_partitions
projdir=$rundir.par.proj
xclbinfile=/proj/autoesl/ryanw/kernel_louvain_pruning.xclbin

#make
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/coPapersDBLP-wt.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/europe_osm-wt100M.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt600M.mtx  -fast  -dev 3 -par_num 12 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt900M.mtx -fast -dev 3 -par_num 18 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt400M_2.mtx -fast -dev 3 -par_num 9 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_pruning.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt1350M.mtx -fast -dev 3 -par_num 27 -driver
#./host.exe -x /proj/autoesl/ryanw/kernel_louvain_gh.xclbin /wrk/xsjhdnobkup1/ryanw/poc_louvain/HugeGraphData/europe_osm-wt600M.mtx  -dev 3 -par_num 12 -driver
echo "./host.exe -x $xclbinfile $graph -fast -dev $num_dev -par_num $par -load_alveo_partitions $projdir -setwkr $num_workers $workers -driverAlone"
export LD_LIBRARY_PATH=$PWD/../../../louvainmod/L3/lib:$LD_LIBRARY_PATH
./host.exe -x $xclbinfile $graph -fast -dev $num_dev -par_num $par -load_alveo_partitions $projdir -setwkr $num_workers $workers -driverAlone
