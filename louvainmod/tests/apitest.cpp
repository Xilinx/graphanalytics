/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xilinxlouvain.h"
#include "ParLV.h"

using namespace xilinx_apps::louvainmod;

int main(int argc, char **argv) {
    ToolOptions toolOptions(argc, argv);
    
    switch (toolOptions.mode_alveo) {
    case ALVEOAPI_PARTITION:
        {
            std::cout << "Partitioning input file " << toolOptions.opts_inFile << std::endl;
            
            Options options;
            LouvainMod louvainMod(options);
            
            LouvainMod::PartitionOptions partOpts;
            partOpts.flow_fast = toolOptions.flow_fast;
            partOpts.nameProj = toolOptions.nameProj;
            partOpts.num_par = toolOptions.num_par;
            partOpts.devNeed_cmd = toolOptions.devNeed;
            partOpts.par_prune = toolOptions.gh_par;
            partOpts.numServers = toolOptions.server_par;
            louvainMod.partitionDataFile(toolOptions.opts_inFile, partOpts);
        }
        break;
    case ALVEOAPI_LOAD:
        std::cout << "ALVEOAPI_LOAD" << std::endl;
        break;
    case ALVEOAPI_RUN:
        std::cout << "ALVEOAPI_RUN" << std::endl;
        break;
    default:
        std::cout << "ERROR: Unknown tool mode " << toolOptions.mode_alveo << std::endl;
        break;
    }
    return 0;
}