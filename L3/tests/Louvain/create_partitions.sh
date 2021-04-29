#!/bin/bash

script=$(readlink -f $0)
script_dir=`dirname $script`

echo Running $script
if [ "$#" -ne 3 ]; then
    echo "$script <.mtx file> <subdir name> <number of partitions>."
    echo "Example: $0 /proj/autoesl/ryanw/graph/Demo_For_webinary_WT/as-Skitter-wt.mtx skitter 9"
    exit 1
fi

graph=$1
subdir=$2
partitions=$3

graphabs=$(readlink -f $graph)
graphdir=`dirname $graphabs`

mkdir -p $graphdir/$subdir
# Set rundir to your dir
rundir=$graphdir/$subdir/louvain_partitions

echo "Removing $rundir"
rm -rf $rundir

cmd="$script_dir/host.exe  $1 -fast -par_num $partitions -create_alveo_partitions -name $rundir"
echo $cmd
export LD_LIBRARY_PATH=$script_dir/../../lib:$LD_LIBRARY_PATH
$cmd

echo "*************************************************************************"
echo "INFO: $partitions partitions saved to $graphdir/$subdir"
echo "*************************************************************************"





