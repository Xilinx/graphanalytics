/******************************************************************************
* Copyright (c) 2015-2016, TigerGraph Inc.
* All rights reserved.
* Project: TigerGraph Query Language
* udf.hpp: a library of user defined functions used in queries.
*
* - This library should only define functions that will be used in
*   TigerGraph Query scripts. Other logics, such as structs and helper
*   functions that will not be directly called in the GQuery scripts,
*   must be put into "ExprUtil.hpp" under the same directory where
*   this file is located.
*
* - Supported type of return value and parameters
*     - int
*     - float
*     - double
*     - bool
*     - string (don't use std::string)
*     - accumulators
*
* - Function names are case sensitive, unique, and can't be conflict with
*   built-in math functions and reserve keywords.
*
* - Please don't remove necessary codes in this file
*
* - A backup of this file can be retrieved at
*     <tigergraph_root_path>/dev_<backup_time>/gdk/gsql/src/QueryUdf/ExprFunctions.hpp
*   after upgrading the system.
*
******************************************************************************/

#ifndef EXPRFUNCTIONS_HPP_
#define EXPRFUNCTIONS_HPP_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string>
#include <chrono>
#include <gle/engine/cpplib/headers.hpp>

/**     XXX Warning!! Put self-defined struct in ExprUtil.hpp **
*  No user defined struct, helper functions (that will not be directly called
*  in the GQuery scripts) etc. are allowed in this file. This file only
*  contains user-defined expression function's signature and body.
*  Please put user defined structs, helper functions etc. in ExprUtil.hpp
*/
#include "ExprUtil.hpp"
#include <string>
#include <thread>
#include <mutex>
//#define XAIDEBUG true
#define XAIWORKAROUND true

namespace UDIMPL {

  typedef std::string string; //XXX DON'T REMOVE

  /****** BIULT-IN FUNCTIONS **************/
  /****** XXX DON'T REMOVE ****************/
  inline int64_t str_to_int (string str) {
    return atoll(str.c_str());
  }

  inline int64_t float_to_int (float val) {
    return (int64_t) val;
  }

  inline string to_string (double val) {
    char result[200];
    sprintf(result, "%g", val);
    return string(result);
  }

  inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ")+=std::to_string(val);
    return true;
  }

  inline bool greater_than_three (double x) {
    return x > 3;
  }

  inline string reverse(string str) {
    std::reverse(str.begin(), str.end());
    return str;
  }

  template <typename tuple>
  inline uint64_t getDeltaQ (tuple tup) {
    return tup.deltaQ;
  }

  template<typename tup>
  inline int64_t getOutDegree(tup t) {
    return t.OutDgr;
  }

  template<typename tup>
  inline VERTEX getCc(tup t) {
    return t.cc;
  }

  /* Start Xilinx UDF additions */
  inline int64_t udf_reinterpret_double_as_int64(double val) {
    int64_t double_to_int64 = *(reinterpret_cast<int64_t*>(&val));
    return double_to_int64;
  }

  inline double udf_reinterpret_int64_as_double(int64_t val) {
    double int64_to_double = *(reinterpret_cast<double*>(&val));
    return int64_to_double;
  }

  inline int64_t udf_lsb32bits(uint64_t val) {
    return val & 0x00000000FFFFFFFF;
  }

  inline int64_t udf_msb32bits(uint64_t val) {
    return ( val >> 32 ) & 0x00000000FFFFFFFF;
  }

  inline VERTEX udf_getvertex(uint64_t vid) {
    return VERTEX(vid);
  }

  inline bool udf_reset_timer(bool dummy) {
    xai::timer_start_time = std::chrono::high_resolution_clock::now();
    return true;
  }

  inline double udf_elapsed_time(bool dummy) {
    xai::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - xai::timer_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
  }


    inline string udf_open_alveo(int mode)
    {
        std::lock_guard<std::mutex> lockGuard(xai::writeMutexOpenAlveo);
    
        if(!xai::openedAlveo) {
            std::cout << "DEBUG: " << __FUNCTION__ << " xai::openedAlveo=" << xai::openedAlveo << std::endl;
            if(xai::openedAlveo) return "";
            xai::openedAlveo = true;
            string result("Initialized Alveo");
            try
            {
                std::cout << "Opening XAI library " << std::endl;
                xai::xaiLoader.load_library(xai::host_libname);
                std::cout << "Opened XAI library " << std::endl;
            }
            catch (std::exception& e) {
                std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
                (result += ": STD Exception:")+=e.what();
            }
            catch (...) {
                std::cerr << "ERROR: An unknown exception occurred." << std::endl;
                (result += ": Unknown Exception:")+=" Reason unknown, check LD_LIBRARY_PATH";
            }
            return result;
        }
        return "";
    }

  inline bool udf_close_alveo(int mode)
  {
    std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
    return true;
  }

  inline int udf_create_alveo_partitions(std::string input_graph, std::string partitions_project, std::string num_patitions)
  {
    std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
    xai::calledExecuteLouvain = false;
    int argc = 8;
    char* argv[] = {"host.exe", input_graph.c_str(), "-fast", "-par_num", num_patitions.c_str(),
          "-create_alveo_partitions", "-name", partitions_project.c_str(), NULL};
    std::cout << "Calling create_partitions. input_graph: " <<  input_graph.c_str()
              << " num_partitions: " << num_patitions.c_str() << " project: " << partitions_project.c_str()
              << std::flush;
    return xai::xaiLoader.create_partitions(argc, (char**)(argv));
  }

  inline int udf_execute_reset(int mode) {
    xai::calledExecuteLouvain = false;
  }

    inline int udf_execute_alveo_louvain(
        std::string input_graph, std::string partitions_project, 
        std::string num_devices, std::string num_patitions, 
        std::string num_workers, std::string community_file)
    {
        if (!xai::calledExecuteLouvain) {
            std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
            if(xai::calledExecuteLouvain) return 0;
            xai::calledExecuteLouvain = true;
            std::string workernum("1");
            bool isDriver = xai::isHostTheDriver(workernum);
            int my_argc = 18;
            std::string worker_or_driver("-workerAlone");
            char* optional_arg = NULL;
            if(isDriver) {
                worker_or_driver = "-driverAlone";
            } else {
                optional_arg = (char*) workernum.c_str();
                my_argc++;
            }
            partitions_project += ".par.proj";

            char* my_argv[] = {"host.exe", "-x", xai::xclbin_filename, input_graph.c_str(), 
                               "-o", community_file.c_str(), "-fast", "-dev", 
                               num_devices.c_str(), "-par_num", num_patitions.c_str(),
                               "-load_alveo_partitions", partitions_project.c_str(), 
                               "-setwkr", num_workers.c_str(), "tcp://192.168.1.21:5555", "tcp://192.168.1.31:5555", 
	                           worker_or_driver.c_str(), optional_arg, NULL};

            std::cout
                << "Calling execute_louvain. input_graph: " <<  input_graph.c_str()
                << " num_partitions: " << num_patitions.c_str()
                << " project: " << partitions_project.c_str()
                << " num workers: " << num_workers.c_str()
                << " worker_or_driver: " << worker_or_driver.c_str()
                << " worker num: " << workernum.c_str() << "\n"
                << std::flush;
        
            //if(isDriver) {
            //unsigned int sleep_time = 240;
            //std::cout << "KD: Going to slpep for seconds: " << sleep_time << "\n";
            //std::this_thread::sleep_for (std::chrono::seconds(sleep_time));
            //std::cout << "KD: Woke up from sleep after seconds: " << sleep_time << "\n";
            //}
            int retVal = xai::xaiLoader.execute_louvain(my_argc, (char**)(my_argv));
            std::cout << "KD: Returned from execute_louvain, isDriver = " << isDriver << "\n" << std::flush;
            return retVal;
        }
        return 0;
    }

  /* End Xilinx Louvain Additions */

}
/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
