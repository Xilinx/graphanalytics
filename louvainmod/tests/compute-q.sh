#!/bin/bash

set -e

echo "INFO: Running $0"

. env.sh

if [ "$#" -ne 5 ]; then
    echo $#
    echo "$0 <.mtx file> <partition-dir> <number of partitions> numDevices numWorkers"
fi

in_file=$1

exe_dir="Release"
if [ "$DEBUG" -eq 1 ]; then
    exe_dir="Debug"
fi 


cmd="../$exe_dir/louvainmod_test -compute_modularity $1"
echo $cmd
$cmd
