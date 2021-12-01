#!/usr/bin/env bash 

set -e

pids=
source /opt/xilinx/xrt/setup.sh
#cards=$(xbutil scan | grep '\[.*\]' | sed -e 's/^ \[\(.*\)\].*/\1/')
cards=$(xbutil examine | grep '\[.*\]' | sed -e 's/^ *\[\(.*\)\].*/\1/')
for card in $cards; do
#    yes | xbutil reset -d $card
    xbutil --force reset --device $card &
    pids="${pids} $!"
done

for pid in $pids; do
    wait $pid
done

hostname=$(hostname)
echo  "xbutil reset on host $hostname is done."
