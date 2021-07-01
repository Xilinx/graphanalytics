#!/bin/bash

# Copyright 2020 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.


set -e
set -x

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 new_ver pre_ver"
    exit 1
fi 

new_ver=$1
prev_ver=$2

rm -f $PWD/cosinesim/package/*.rpm package/*.deb 

cosinesim_build_cmd=\
"cd $PWD/cosinesim && 
rm -rf pyvenv && 
make clean && 
make && 
make pythonApi && 
make stage && 
make dist "

echo "*****************************************************************************"
echo "INFO: building Cosinesim Ubuntu 18.04 package"
echo "*****************************************************************************"
ssh xsj-dxgradb04 $cosinesim_build_cmd

echo "*****************************************************************************"
echo "INFO: building Cosinesim Ubuntu 16.04 package"
echo "*****************************************************************************"
ssh xsjfislx14 $cosinesim_build_cmd

echo "*****************************************************************************"
echo "INFO: building Cosinesim CentOs 7.8 package"
echo "*****************************************************************************"
ssh xsjkumar50 $cosinesim_build_cmd

cp $PWD/cosinesim/package/*$new_ver*18.04*.deb $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/
cp $PWD/cosinesim/package/*$new_ver*16.04*.deb $PWD//scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/
cp $PWD/cosinesim/package/*$new_ver*7.8*.rpm $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/

if [ "$prev_ver" != "$new_ver" ]; then
    echo "Removing files for the previous version"
    if [ -f "$PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/*$prev_ver*18.04*.deb.vclf" ]; then
        git rm $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/*$prev_ver*18.04*.deb.vclf
    fi

    if [ -f "$PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/*$prev_ver*16.04*.deb.vclf" ]; then
        git rm $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/*$prev_ver*16.04*.deb.vclf
    fi
    if [ -f "$PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/*$prev_ver*7.8*.rpm.vclf" ]; then
        git rm $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/*$prev_ver*7.8*.rpm.vclf
    fi 

    rm -f $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/*$prev_ver*18.04*.deb
    rm -f $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/*$prev_ver*16.04*.deb
    rm -f $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/*$prev_ver*7.8*.rpm

    echo "Adding files for the new version to VCLF repo"
    vclf add --tags ubuntu-18.04 $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/*$new_ver*18.04*.deb
    vclf add --tags ubuntu-18.04 $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/*$new_ver*16.04*.deb
    vclf add --tags ubuntu-18.04 $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/*$new_ver*7.8*.rpm
fi

ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-18.04/cosinesim/
ls -l $PWD/scripts/xilinx-tigergraph-install/ubuntu-16.04/cosinesim/
ls -l $PWD/scripts/xilinx-tigergraph-install/centos-7.8/cosinesim/
