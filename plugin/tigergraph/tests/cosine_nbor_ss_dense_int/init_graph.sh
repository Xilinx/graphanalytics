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
# common.sh sets up things like gsql client, username and passowrd, graph name, etc
. common.sh

echo "data_root=$data_root"
patients_infile="$data_root/patients.csv"
immunizations_infile="$data_root/immunizations.csv"
allergies_infile="$data_root/allergies.csv"
conditions_infile="$data_root/conditions.csv"
imaging_studies_infile="$data_root/imaging_studies.csv"
procedures_infile="$data_root/procedures.csv"
careplans_infile="$data_root/careplans.csv"

time gsql "$(cat schema_xgraph.gsql | sed "s/@graph/$xgraph/")"
time gsql "$(cat load_xgraph.gsql | sed "s/@graph/$xgraph/")"
# set timeout of loading job to 1 hour
time gsql -g $xgraph "SET QUERY_TIMEOUT=3600000 RUN LOADING JOB load_xgraph USING \
                        patients_infile=\"$patients_infile\", \
                        immunizations_infile=\"$immunizations_infile\", \
                        allergies_infile=\"$allergies_infile\", \
                        conditions_infile=\"$conditions_infile\", \
                        imaging_studies_infile=\"$imaging_studies_infile\", \
                        procedures_infile=\"$procedures_infile\", \
                        careplans_infile=\"$careplans_infile\" "
time gsql -g $xgraph "DROP JOB load_xgraph"
echo "INFO: -------- $(date) load_xgraph completed. --------"

./install_query.sh $@

