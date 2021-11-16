#!/usr/bin/env bash 

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

#if [ -x "$(command -v grun)" ]; then
#    grun all ${script_dir}/resetc-node.sh
#    exit 0
#fi

echo "Note: Resets execute in the background.  The process is finished"
echo "when you see \"<machine> done\" messages for all machines."
machines="192.168.1.11 192.168.1.21 192.168.1.31"
pids=
for machine in $machines; do
#    ssh -f $machine "${script_dir}/resetc-node.sh; echo $machine done."
    ssh -f $machine "${script_dir}/resetc-node.sh >/dev/null; echo $machine done."
done
