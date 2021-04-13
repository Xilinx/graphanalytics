#!/bin/bash

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

nodeID=$1
logFile=$2

nodeID=$(echo $nodeID | grep -oP "m[\d]+")
nodeHostname=$($SCRIPTPATH/get-node-ip.py $nodeID)
logFilename=$(basename $logFile)
tmpLogFile="/tmp/$USER-$nodeHostname-$logFilename"

scp $nodeHostname:$logFile $tmpLogFile
vim $tmpLogFile 
