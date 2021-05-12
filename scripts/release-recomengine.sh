#!/bin/bash

set -e

ver=1.0
rm -f $PWD/plugin/tigergraph/package/*.rpm package/*.deb 

build_cmd=\
"cd $PWD/plugin/tigergraph && 
make clean && 
make stage && 
make dist "

echo "INFO: building RecommEngine Ubuntu 18.04 package"
ssh xsj-dxgradb04 $build_cmd

echo "INFO: building RecommEngine Ubuntu 16.04 package"
ssh xsjfislx14 $build_cmd

echo "INFO: building RecommEngine CentOs 7.8 package"
ssh xsjkumar50 $build_cmd

cp $PWD/plugin/tigergraph/package/*$ver*18.04*.deb $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/recomengine/
cp $PWD/plugin/tigergraph/package/*$ver*16.04*.deb $PWD//scripts/xilinx-tigergraph-install/ubuntu-16.04/recomengine/
cp $PWD/plugin/tigergraph/package/*$ver*7.8*.rpm $PWD/scripts/xilinx-tigergraph-install/centos-7.8/recomengine/

ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/recomengine/
ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/recomengine/
ls -l $PWD/scripts/xilinx-tigergraph-install/centos-7.8/recomengine/
