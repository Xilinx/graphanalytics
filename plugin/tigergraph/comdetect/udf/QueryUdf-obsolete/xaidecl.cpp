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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xailoader.hpp"
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace xai {

  // Open Alveo parameters
  const char* host_libname = "libXilinxLouvain.so";
  const char* xclbin_filename = "/opt/xilinx/apps/graphanalytics/louvainmod/1.0/xclbin/louvainmod_pruning_xilinx_u50_gen3x16_xdma_201920_3.xclbin";

  // Load alveo parameters
  bool tg_partition = true;
  bool use_saved_partition = true;
  std::string graph_file = "";
  std::string louvain_project = "";
  std::string num_partitions = "1";
  std::string num_devices = "1";

  // Mutexes
  std::mutex writeMutex;
  std::mutex writeMutexOpenAlveo;

  // States
  bool executedLouvain = false;
  bool loadedAlveo = false;
  bool openedAlveo = false;

  // Construct loader and start timer
  Loader xaiLoader;
  t_time_point timer_start_time = std::chrono::high_resolution_clock::now();

  //Layout of fields in /proc/self/status
  //VmPeak:     8216 kB This is peak Virtual Memory size
  //VmHWM:       752 kB This is peak Resident Set Size
  // return size of field in kB, return value of -1.0 means error
  double extract_size_in_kB(std::string& line) {
    double retValue = -1.0;
    std::istringstream sstream(line);
    std::string field, value, unit;
    sstream >> field >> value >> unit;
    if(unit == "kB") {
      retValue = std::stod(value);
    }
    return retValue;
  }

  bool isHostTheDriver(std::string& workernum)
  {
    bool retVal = false;
    const char* driverHostName = "xsj-dxgradb01";
    char hostname[HOST_NAME_MAX + 1];
    int res_hostname = gethostname(hostname, HOST_NAME_MAX + 1);
    if (res_hostname != 0) {
      std::cout << "XAIDEBUG: gethostname failed\n";
      return retVal;
    }
    std::string hostString(hostname);
    std::cout << "XAIDEBUG: host_name: " << hostString;
    int res_comp = hostString.compare(driverHostName);
    if (res_comp == 0) {
      retVal = true;
      workernum = "0";
      std::cout << " Driver \n" << std::flush;
    } else {
      std::cout << " Worker \n" << std::flush;
      if (hostString.compare("xsj-dxgradb02") == 0) {
        workernum = "1";
      } else if (hostString.compare("xsj-dxgradb03") == 0) {
        workernum = "2";
      } else {
        workernum = "3";
      }
    }
    return retVal;
  }

}
