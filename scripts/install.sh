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

OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t' | awk -F. '{print $1*100+$2}'`

if [[ $OSDIST == "ubuntu" ]]; then
    if (( $OSREL != 1604 )); then
        echo "ERROR: Ubuntu release version must be 16.04"
        return 1
    fi
else
    echo "ERROR: only Ubuntu 16.04 is supported."
    return 1
fi

if (( $OSREL == 1604 )); then
    pkg_dir="./xilinx-tigergraph-install/ubuntu-16.04"
else
    pkg_dir="./xilinx-tigergraph-install/ubuntu-18.04"
fi

SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

# install XRT/XRM/Deployment shell
echo ""
echo "INFO: installf XRT"
sudo apt install $pkg_dir/xrt/xrt*.deb

echo ""
echo "INFO: install XRM"
sudo apt install $pkg_dir/xrm/xrm*.deb

echo ""
echo "INFO: install deployment shell"
sudo apt install $pkg_dir/deployment-shell/xilinx*.deb

echo ""
echo "INFO: install overlays as user tigergraph. Enter the password for tigergraph below:"
su -c $SCRIPTPATH/install-overlays.sh - tigergraph










