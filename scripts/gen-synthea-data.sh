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

if [ "$#" -lt 1 ]; then 
    echo "Usage: $0 num_of_patients"
    exit 1
fi
RED='\033[0;31m'
NC='\033[0m' # No Color
num_of_patients=$1
disk_estimate=$[$num_of_patients * 300 / 1000]
time_estimate=$[$num_of_patients * 80 / 1000]
echo "INFO: Generating synthea data for $num_of_patients patients..."
printf "${RED}INFO: Estimated disk size=$disk_estimate MB. Estimated run time=$time_estimate seconds${NC}\n"
output_dir=$num_of_patients\_patients
synthea_jar="synthea-with-dependencies.jar"
if [ ! -f $HOME/synthea/$synthea_jar ]; then
    mkdir -p $HOME/synthea
    wget -O $HOME/synthea/$synthea_jar https://github.com/synthetichealth/synthea/releases/download/master-branch-latest/synthea-with-dependencies.jar
    echo "INFO: $synthea_jar downloaded to $HOME/synthea."
fi

if [ -d $output_dir ]; then
    read -p "$output_dir already exists. Overwrite? (y/n): " confirm && \
           [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1
fi

/usr/bin/time -v java -jar $HOME/synthea/synthea-with-dependencies.jar \
        --exporter.csv.export true \
        --exporter.fhir.export false \
        --exporter.hospital.fhir.export false \
        --exporter.practitioner.fhir.export false \
        --generate.log_patients.detail none \
        --exporter.baseDirectory $output_dir \
        -a 0-100 -p $num_of_patients

echo "INFO: Generated data for $num_of_patients patients"

