#!/bin/bash
#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
# Copyright 2020-2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

##########################################################################################
# Update package version for each release
##########################################################################################
louvain_pkg=xilinx-louvainmod-0.1

function usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -p product-name  : Supported products: cosinesim, louvainmod. Only base libraries are installed if product-name is not set."
    echo "  -h               : Print this help message"
}

product="none"
while getopts ":p:h" opt
do
case $opt in
    p) product=$OPTARG;;
    h) usage; exit 1;;
    ?) echo "ERROR: Unknown option: -$OPTARG"; usage; exit 1;;
esac
done


OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t' | awk -F. '{print $1*100+$2}'`
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [[ $OSDIST == "ubuntu" ]]; then
    if (( $OSREL == 1804 )); then
        pkg_dir="./ubuntu-18.04"
    elif (( $OSREL == 2004 )); then
        pkg_dir="./ubuntu-20.04"        
    else
        echo "ERROR: Ubuntu release version must be 18.04 or 20.04."
        return 1
    fi
elif [[ $OSDIST == "centos" ]]; then
    pkg_dir="./centos-7.8"
else 
    echo "ERROR: only Ubuntu and Centos are supported."
    return 1
fi

echo "INFO: Installing packages on $OSDIST $OSREL"

read -p "It is recommended to remove current XRT/XRM if present. Remove? (y/n): " confirm 

if [[ $OSDIST == "ubuntu" ]]; then
    if [[ $confirm == [yY] ]]; then
        printf "\nEnter sudo password below to remove XRT/XRM:\n"
        sudo apt remove xrm xrt -y
    fi

    # install XRT/XRM/Deployment shell
    printf "\n-------------------------------------------------------------\n"
    printf "INFO: Install XRT. Enter sudo password if asked."
    printf "\n-------------------------------------------------------------\n"
    sudo apt install $pkg_dir/xrt/xrt*.deb

    printf "\n-------------------------------------------------------------\n"
    printf "INFO: Install XRM. Enter sudo password if asked."
    printf "\n-------------------------------------------------------------\n"
    sudo apt install $pkg_dir/xrm/xrm*.deb

    printf "\n-------------------------------------------------------------\n"
    printf "INFO: Install deployment shell. Enter sudo password if asked."
    printf "\n-------------------------------------------------------------\n"
    sudo apt install $pkg_dir/deployment-shell/xilinx*.deb

    # install required package
    sudo apt install jq opencl-headers -y

    if [[ $product == "cosinesim" ]]; then
        # install required package
        sudo apt install jq opencl-headers -y
        printf "\n-------------------------------------------------------------\n"
        printf "INFO: Install Xilinx CosineSim. Enter sudo password if asked."
        printf "\n-------------------------------------------------------------\n"
        sudo apt install $pkg_dir/cosinesim/xilinx-cosinesim*.deb

        printf "\n-------------------------------------------------------------\n"
        printf "INFO: Install Xilinx Recommend Engine. Enter sudo password if asked."
        printf "\n-------------------------------------------------------------\n"
        sudo apt install $pkg_dir/recomengine/xilinx-recomengine*.deb
    fi

    if [[ $product == "louvainmod" ]]; then
        printf "\n-------------------------------------------------------------\n"
        printf "INFO: Install Xilinx LouvainMod. Enter sudo password if asked."
        printf "\n-------------------------------------------------------------\n"
        sudo apt remove -y $louvain_pkg
        sudo apt install $pkg_dir/louvainmod/$louvain_pkg*.deb
    fi
fi

if [[ $OSDIST == "centos" ]]; then
    if [[ $confirm == [yY] ]]; then
        printf "\nEnter sudo password below to remove XRT/XRM:\n"
        sudo yum remove xrt -y
    fi

    # install XRT/XRM/Deployment shell
    printf "\nINFO: Install XRT. \n"
    sudo yum install $pkg_dir/xrt/xrt*.rpm

    printf "\nINFO: Install XRM. Enter sudo password if asked.\n"
    sudo yum install $pkg_dir/xrm/xrm*.rpm

    printf "\nINFO: Install deployment shell\n"
    sudo yum install $pkg_dir/deployment-shell/xilinx*.rpm

    if [[ $product == "cosinesim" ]]; then
        # install required package
        sudo yum install jq opencl-headers -y

        printf "\nINFO: Install Xilinx CosineSim package\n"
        sudo yum install $pkg_dir/cosinesim/xilinx-cosinesim*.rpm

        printf "\nINFO: Install Xilinx Recommend Engine package\n"
        sudo yum install $pkg_dir/recomengine/xilinx-recomengine*.rpm
    fi

    # only need to run this on CentOS
    #copy the standard libstdc++ to $HOME/libstd
    mkdir -p $HOME/libstd
    cp /usr/lib64/libstdc++.so.6* $HOME/libstd
fi


printf "\nINFO: All packages have been installed. Please run the command below to flash the card if needed. \n" 
printf "${YELLOW}sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3${NC}\n"
