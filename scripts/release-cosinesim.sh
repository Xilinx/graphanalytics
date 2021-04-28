#!/bin/bash

set -e

ver=1.0
rm -f $PWD/cosinesim/package/*.rpm package/*.deb 

cosinesim_build_cmd=\
"cd $PWD/cosinesim && 
rm -rf pyvenv && 
make clean && 
make && 
make pythonApi && 
make stage && 
make dist "

echo "INFO: building Cosinesim Ubuntu 18.04 package"
ssh xsj-dxgradb04 $cosinesim_build_cmd

echo "INFO: building Cosinesim Ubuntu 16.04 package"
ssh xsjfislx14 $cosinesim_build_cmd

echo "INFO: building Cosinesim CentOs 7.8 package"
ssh xsjkumar50 $cosinesim_build_cmd

cp $PWD/cosinesim/package/*$ver*18.04*.deb $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/
cp $PWD/cosinesim/package/*$ver*16.04*.deb $PWD//scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/
cp $PWD/cosinesim/package/*$ver*7.8*.rpm $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/

ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/
ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/
ls -l $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/





