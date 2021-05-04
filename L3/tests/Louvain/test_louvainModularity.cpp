/*
 * Copyright 2020 Xilinx, Inc.
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

#include "utils2.hpp"
#include "xf_graph_L3.hpp"
#include "stdio.h"

#include "common.hpp"
#include <cstdlib>
#include <time.h>
#include "zmq.h"

#include "defs.h"
#include "xilinxlouvain.h"

int main(int argc, char *argv[]) {

    int mode_alveo=ALVEOAPI_NONE;
    for(int i=1; i<argc; i++){
    	//printf("%s  %d\n", argv[i], i);
    	if(0==strcmp(argv[i], "-create_alveo_partitions"))
    		return create_alveo_partitions(argc, argv);
    	else if(0==strcmp(argv[i], "-load_alveo_partitions"))
    		return load_alveo_partitions(argc, argv);
    }
    printf("\n\n");
    printf("\033[1;37;40mTo CREATE partition for Louvain:\033[0m\n\t");
    printf("\033[1;31;40m.\/host.exe\033[0m <graph file> \033[1;31;40m-par_num\033[0m <number of partitions> \033[1;31;40m-create_alveo_partitions\033[0m [\033[1;31;40m-name\033[0m <project name>]\n");
    printf("\n");
    printf("\033[1;37;40mTo LOAD partition and EXECUTE Louvain for a WORKER:\033[0m\n\t");
    printf("\033[1;31;40m.\/host.exe\033[0m <graph file> \033[1;31;40m-x\033[0m <kernel> [-fast] [-dev <num>] \033[1;31;40m-load_alveo_partitions\033[0m <project name>.par.proj ");
	printf("\033[1;31;40m-setwkr\033[0m <number of worker> <worker list> \033[1;31;40m-workerAlone\033[0m <ID of worker> \n");
	printf("\n");
	printf("\033[1;37;40mTo LOAD partition and EXECUTE Louvain for a DRIVER:\033[0m\n\t");
    printf("\033[1;31;40m.\/host.exe\033[0m <graph file> \033[1;31;40m-x\033[0m <kernel> [-fast] [-dev <num>] \033[1;31;40m-load_alveo_partitions\033[0m <project name>.par.proj ");
    printf("\033[1;31;40m-setwkr\033[0m <number of worker> <worker list> \033[1;31;40m-driverAlone\033[0m\n");
	printf("\n\n");
    return 0;
}

