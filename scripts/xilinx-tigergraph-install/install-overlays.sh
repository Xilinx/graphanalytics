#!/bin/bash
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
#
SCRIPT=$(readlink -f $0)
SCRIPTPATH=`dirname $SCRIPT`

echo "Checking TigerGraph installation directory"
tg_root_dir=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
echo "Found TigerGraph installation in $tg_root_dir"

# Copy xclbins to TG root directory
mkdir -p $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin
cp $SCRIPTPATH/overlays/cosineSimilaritySSDenseIntBench/xilinx_u50_gen3x16_xdma_201920_3/denseSimilarityKernel.xclbin $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin

echo "XCLBIN files are copied to $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin"

