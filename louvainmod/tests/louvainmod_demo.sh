#!/usr/bin/env bash 

set -e

SCRIPT=$(readlink -f $0)
script_dir=`dirname $SCRIPT`

echo "Command line: $0 $* "

. ${XILINX_XRT:=/opt/xilinx/xrt}/setup.sh
. ${XILINX_XRM:=/opt/xilinx/xrm}/setup.sh

pwd=$(pwd)
outputDir=$pwd/louvain-out
projectDir=$outputDir/project
projectName=$projectDir/louvain
logDir=$outputDir/log

#######################################################################################################################

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -a alveoCard         : Alveo card type: u50 or u55c (default=u50)"
    echo "  -b                   : Run in debug mode (executables and libraries from Debug instead of Release)"
    echo "  -c \"ip1 ip2...\"      : A list of IP addresses of servers to run on (default=192.168.1.11 .21 .31)"
    echo "  -d numDevices        : number of Alveo cards per server (default=1)"
    echo "  -l                   : Run on local host, same as -c 127.0.0.1"
    echo "  -n numPartitions     : Number of Alveo partitions (default=auto calculate)"    
    echo "  -s dataSource        : A .mtx file containing input graph. Required unless using -x"
    echo "  -o communityFileName : Generate output file containing communities"
    echo "  -x                   : Use existing partitions from disks"
    echo "  -h                   : Print this help message"
}

clusterIps="192.168.1.11 192.168.1.21 192.168.1.31"
isDebug=0
debugStr=
alveoProject="/proj/gdba/xgstore/dliddell_islands9"
workerNum=0
doPartitioning=1
communityFileName=
numDevices=
numDevicesOpt=
dOpt=
alveoCard=u50
kernelMode=0
xclbinFile=
dataSource=
numPartitions=9
numPartitionsOpt="-num_pars ${numPartitions}"
nOpt="-n ${numPartitions}"

while getopts ":a:bc:d:hln:o:s:w:x" opt
do
case $opt in
    a) alveoCard=$OPTARG;;
    b) isDebug=1; debugStr="-b";;
    c) clusterIps=$OPTARG;;
    d) numDevices=$OPTARG; numDevicesOpt="-numDevices ${numDevices}"; dOpt="-d ${numDevices}";;
    h) usage; exit 0;;
    l) clusterIps="127.0.0.1";;
    n) numPartitions=$OPTARG; numPartitionsOpt="-num_pars ${numPartitions}"; nOpt="-n ${numPartitions}";;
    o) communityFileName=$OPTARG;;
    s) dataSource=$OPTARG;;
    w) workerNum=$OPTARG;;
    x) doPartitioning=0;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done

# Convert IP separators from , to space if any (as when passed from driver to worker)
clusterIps=${clusterIps//,/ }

if [ $alveoCard == "u50" ]; then
    kernelMode=2
    xclbinFile=louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin
    deviceNames=xilinx_u50_gen3x16_xdma_201920_3
elif [ $alveoCard == "u55c" ]; then
    kernelMode=4
    xclbinFile=louvainmod_2cu_xilinx_u55c_gen3x16_xdma_2_202110_1.xclbin
    deviceNames=xilinx_u55c_gen3x16_xdma_base_2
else
    echo "ERROR: Unrecognized Alveo card ${alveoCard}.  Must be u50 or u55c."
    exit 2
fi

buildType=Release
if [ $isDebug -eq 1 ]; then
    buildType=Debug
fi

#######################################################################################################################

buildDir=$script_dir/../$buildType
stagingDir=$script_dir/../staging
xclbinPath=$stagingDir/xclbin/$xclbinFile

LD_LIBRARY_PATH=$buildDir:$LD_LIBRARY_PATH

# Extract the first IP address, which should be the driver, and the remaining IP addresses for the workers
driverIp=$(echo $clusterIps | cut -d ' ' -f1)
workerIps=$(echo $clusterIps | cut -d ' ' -f2- -s)
numWorkers=$(echo $workerIps | wc -w)
numNodes=$(echo $clusterIps | wc -w)

commandLineCommon="$buildDir/cppdemo -x $xclbinPath -kernel_mode $kernelMode -num_nodes $numNodes $numDevicesOpt \
    -devices $deviceNames -num_level 100 -num_iter 100 -load_alveo_partitions $projectName.par.proj"

# If running on the driver server, launch worker processes, then run the driver process

if [ $workerNum -eq 0 ]; then
    # Error out if not running this script from the driver machine
    if [ $(/sbin/ifconfig | grep -c "inet $driverIp") -eq 0 ]; then
        echo "Please run this script from ${driverIp}."
        exit 1
    fi

    # Partition the design
    if [ $doPartitioning -eq 1 ]; then
        # Make sure we have a source .mtx file unless running in -x mode
        if [ -z $dataSource ]; then
            echo "ERROR: -s option required unless reusing previously created partitions (-x)."
            exit 3
        fi
        rm -rf $projectDir
        mkdir -p $projectDir
	$buildDir/cppdemo $dataSource -kernel_mode $kernelMode $numPartitionsOpt \
            -create_alveo_partitions -name $projectName
    fi

    # Loop through the number of workers (IP list minus the first), launching each worker in the background via ssh
    mkdir -p $logDir
    workerNum=1
    for ip in $workerIps; do
        echo "Launching worker ${workerNum} on node ${ip}"
        ssh -f $ip "cd $pwd; $SCRIPT -a $alveoCard -c ${clusterIps// /,} $debugStr $dOpt $nOpt -w $workerNum" \
            > $logDir/worker${workerNum}.log 2>&1
        workerNum=$(($workerNum + 1))
    done

    # Run the driver process
    echo "Running driver locally on node ${driverIp}"
    $commandLineCommon -setwkr $numWorkers $workerIps -driverAlone # $(communityOutputOpt)

# Running on a worker server: kill off any old worker processes, then run the worker process

else
    # Kill any existing process first
    oldProcesses=$(ps x | grep cppdemo | grep -v grep | awk '{print $1;}')
    for procId in $oldProcesses; do
        echo "Killing old process ${procId}"
        kill -9 $procId
    done
    $commandLineCommon -workerAlone $workerNum
fi
