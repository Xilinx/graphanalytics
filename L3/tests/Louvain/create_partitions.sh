#!/bin/bash
echo Running $0
if [ "$#" -ne 3 ]; then
    echo "$0 <.mtx file> <subdir name> <number of partitions>. Example: $0 /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/as-Skitter-wt.mtx skitter 9"
    exit 1
fi

# Change rootdir appropriately
rootdir=/proj/gdba/ywu/ghe/poc_louvain
graph=$1
subdir=$2
partitions=$3

mkdir -p $rootdir/$subdir
# Set rundir to your dir
rundir=$rootdir/$subdir/louvain_partitions
rm -rf $rundir*
echo removing $rundir*
echo ./host.exe  $1 -fast -par_num $partitions -create_alveo_partitions -name $rundir

export LD_LIBRARY_PATH=$PWD/../../../louvainmod/L3/lib:$LD_LIBRARY_PATH
./host.exe  $1 -fast -par_num $partitions -create_alveo_partitions -name $rundir

