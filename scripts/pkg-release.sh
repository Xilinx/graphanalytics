#!/bin/bash

# Copyright 2022 Xilinx, Inc.
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

if [ "$#" -lt 1 ]; then
    echo "Usage: $0 version"
    exit 1
fi 

version=$1

tar_file=xilinx-tigergraph-install-$version.tar

# remove the package with the same version
rm -f $tar_file.gz 

#update version number in the pacakge
echo $version > xilinx-tigergraph-install/VERSION

# copy requirements.txt
cp ../requirements.txt xilinx-tigergraph-install/

#generate .tar.gz package
tar cf $tar_file xilinx-tigergraph-install
gzip $tar_file

mv $tar_file.gz /proj/gdba/release
