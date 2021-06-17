#!/usr/bin/env bash 

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

cmdName=pardump
if [ -x "$(command -v $script_dir/../Release/$cmdName)" ]; then
    exePath=$script_dir/../Release/$cmdName
    export LD_LIBRARY_PATH=$script_dir/../Release
elif [ -x "$(command -v $script_dir/../Release/$cmdName)" ]; then
    exePath=$script_dir/../Debug/$cmdName
    export LD_LIBRARY_PATH=$script_dir/../Debug
else
    echo "ERROR: $cmdName not found in Release or Debug directories"
    exit 1
fi

$exePath $*
