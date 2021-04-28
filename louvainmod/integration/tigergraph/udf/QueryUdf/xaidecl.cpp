#include "xailoader.hpp"
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string>
#include <iostream>

namespace xai {

 const char* host_libname = "libgraphL3.so";
 const char* xclbin_filename = "/proj/autoesl/ryanw/kernel_louvain_pruning.xclbin";
 const unsigned int elementSize = sizeof(int); // Number of bytes for each property
 const unsigned int vectorLength = 200;        // Number of properties in each vector
 const unsigned int startPropertyIndex = 3;    // Start index of property in the vector, 0, 1 and 2 are ressrved for norm, id */
 const unsigned int numVectorPerChannel=54050;  //1.6M: Number of vectors per PE or Channel
 const unsigned int numChannels = 14;          // Number of PE  or channels per CU
 const unsigned int maxTopK = 100;             // Maximum top 100 results HW can handle
 const unsigned int numDevices = 1;            // Number of FPGA devices
 int* subjectVector = nullptr;
 int* populationVectorArray_cu0 = nullptr;
 int* populationVectorArray_cu1 = nullptr;
 int* curPopulationVectorArray = nullptr;
 unsigned int curElementIndex = 0;  // number of elements written out so far
 unsigned int maxElementPerCU = 0; //  = vectorLength * numVectorPerChannel * numChannels
 std::ofstream pv_file;
 Loader xaiLoader;
 int defaultNorm = 1264618991;
 int defaultId0 = 1111111;
 int defaultId1 = 0;
 int defaultProperty = -10480;
 t_time_point timer_start_time = std::chrono::high_resolution_clock::now();
 p_xai_context xaiCP = nullptr;
 xaiHandle appContext = nullptr;
 p_xai_id_value_pair resultVec = nullptr;
 std::mutex writeMutex;
 bool calledExecuteLouvain = false;
 std::mutex writeMutexOpenAlveo;
 bool openedAlveo = false;


 std::int64_t abs64(std::int64_t x)
 {
   return (x >= 0) ? x : -x;
 }

 p_xai_context createXSIContext() {
    p_xai_context xaiCP = new s_xai_context;
    xaiCP->xclbin_filename = xai::xclbin_filename;
    xaiCP->num_devices  = xai::numDevices;
    xaiCP->vector_length = xai::vectorLength;
    xaiCP->num_result = xai::maxTopK;
    xaiCP->num_CUs = 2;
    xaiCP->num_Channels = xai::numChannels;
    xaiCP->start_index = xai::startPropertyIndex;
    xaiCP->element_size = sizeof(SPATIAL_dataType);

    return xaiCP;
 }

 bool isHostTheDriver(std::string& workernum)
 {
   bool retVal = false;
   const char* driverHostName = "xsj-dxgradb01";
   char hostname[HOST_NAME_MAX + 1];
   int res_hostname = gethostname(hostname, HOST_NAME_MAX + 1);
   if(res_hostname != 0) {
     std::cout << "XAIDEBUG: gethostname failed\n";
     return retVal;
   }
   std::string hostString(hostname);
   std::cout << "XAIDEBUG: host_name: " << hostString;
   int res_comp = hostString.compare(driverHostName);
   if(res_comp == 0) {
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
