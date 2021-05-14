#!/bin/bash

set -e

echo "INFO: Running $0"

. env.sh

if [ "$#" -ne 3 ]; then
    echo $#
    echo "$0 in-mtx-file in-cluster-info-file offset"
    exit 1
fi

in_file=$1
in_cluster_info_file=$2
offset=$3

exe_dir="Release"
if [ "$DEBUG" == "1" ]; then
    exe_dir="Debug"
fi 

cmd="../$exe_dir/louvainmod_test -compute_modularity $in_file $in_cluster_info_file $offset"
echo $cmd
$cmd
