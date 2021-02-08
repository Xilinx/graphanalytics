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

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t' | awk -F. '{print $1*100+$2}'`
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

if [[ $OSDIST == "ubuntu" ]]; then
    if (( $OSREL == 1604 )); then
        pkg_dir="./ubuntu-16.04"
    elif (( $OSREL == 1804 )); then
        pkg_dir="./ubuntu-18.04"
    else
        echo "ERROR: Ubuntu release version must be 16.04 or 18.04."
        return 1
    fi
elif [[ $OSDIST == "centos" ]]; then
    pkg_dir="./centos-7.8"

else 
    echo "ERROR: only Ubuntu and Centos are supported."
    return 1
fi

if [[ $OSDIST == "ubuntu" ]]; then
    read -p "XRT will be removed if present. Continue? (Y/N): " confirm && \
           [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

    printf "\nRemove XRT if present\n"
    sudo apt remove xrt -y

    # install XRT/XRM/Deployment shell
    printf "\nINFO: install XRT\n"
    sudo apt install $pkg_dir/xrt/xrt*.deb

    printf "\nINFO: install XRM\n"
    sudo apt install $pkg_dir/xrm/xrm*.deb

    printf "\nINFO: install deployment shell\n"
    sudo apt install $pkg_dir/deployment-shell/xilinx*.deb

    # install required package
    sudo apt install jq -y

    printf "\nINFO: install Xilinx overlaysas on TigerGraph installation\n"
    read -p "Enter username used for TigerGraph installation [default: tigergraph]:" tg_username
    tg_username=${tg_username:-tigergraph}
    su -c $SCRIPTPATH/install-overlays.sh - $tg_username
fi

if [[ $OSDIST == "centos" ]]; then
    read -p "XRT will be removed if present. Continue? (Y/N): " confirm && \
           [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

    printf "\nRemove XRT if present\n"
    sudo yum remove xrt -y

    # install XRT/XRM/Deployment shell
    printf "\nINFO: install XRT\n"
    sudo yum install $pkg_dir/xrt/xrt*.rpm

    printf "\nINFO: install XRM\n"
    sudo yum install $pkg_dir/xrm/xrm*.rpm

    printf "\nINFO: install deployment shell\n"
    sudo yum install $pkg_dir/deployment-shell/xilinx*.rpm

    # install required package
    sudo yum install jq -y

    printf "\nINFO: install Xilinx overlaysas on TigerGraph installation\n"
    read -p "Enter username used for TigerGraph installation [default: tigergraph]:" tg_username
    tg_username=${tg_username:-tigergraph}
    su -c $SCRIPTPATH/install-overlays.sh - $tg_username
fi


printf "\nINFO: please run the command below to flash the card if needed\n" 
printf "${YELLOW}sudo /opt/xilinx/xrt/bin/xbmgmt flash --update --shell xilinx_u50_gen3x16_xdma_201920_3${NC}\n"
