#!/bin/bash

set -e

script=$(readlink -f $0)
script_dir=`dirname $script`

echo Running $script
if [ "$#" -lt 3 ]; then
    echo "$script <.mtx file> <subdir name> <number of partitions>."
    echo "Example: $0 /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/as-Skitter-wt.mtx skitter 9 -server_par $4"
    exit 1
fi

graph=$1
subdir=$2
partitions=$3

mkdir -p $subdir

# Set rundir to your dir
rundir=$subdir/louvain_partitions

echo "Removing $rundir"
rm -rf $rundir

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 

export LD_LIBRARY_PATH=$script_dir/../$exe_dir/:$LD_LIBRARY_PATH
cmd="$script_dir/../$exe_dir/louvainmod_test  $1 -fast -par_num $partitions -create_alveo_partitions -name $rundir -server_par $4"
echo $cmd
$cmd

echo "*************************************************************************"
echo "INFO: $partitions partitions saved to $subdir"
echo "*************************************************************************"





