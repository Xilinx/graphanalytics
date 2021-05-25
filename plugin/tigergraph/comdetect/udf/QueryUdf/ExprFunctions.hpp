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
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

#include <gle/engine/cpplib/headers.hpp>

/**     XXX Warning!! Put self-defined struct in ExprUtil.hpp **
*  No user defined struct, helper functions (that will not be directly called
*  in the GQuery scripts) etc. are allowed in this file. This file only
*  contains user-defined expression function's signature and body.
*  Please put user defined structs, helper functions etc. in ExprUtil.hpp
*/
#include "ExprUtil.hpp"
#include <thread>
#include <mutex>
//#define XAIDEBUG true

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
    inline float getWeight(tup t) {
        return t.weight;
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

    inline bool udf_reset_states(int mode)
    {
        xai::executedLouvain = false;
        if(mode>=1) {
            xai::loadedAlveo = false;
        }
        if(mode>=2) {
            xai::openedAlveo = false;
        }
        return true;
    }

    inline int64_t udf_peak_memory_usage(double& VmPeak, double& VmHWM)
    {
        // Open the /proc/self/status and grep for relevant fields
        //Layout of fields in /proc/self/status
        //VmPeak:     8216 kB This is peak Virtual Memory size
        //VmHWM:       752 kB This is peak Resident Set Size
        uint64_t vm_peak, vm_hwm;
        string line;
        std::ifstream proc_status("/proc/self/status", std::ios_base::in);
        while (std::getline(proc_status, line)) {
            std::size_t pos = line.find("VmPeak");
            if( pos != string::npos) {
                VmPeak = xai::extract_size_in_kB(line);
            }
            pos = line.find("VmHWM");
            if( pos != string::npos) {
                VmHWM = xai::extract_size_in_kB(line);
            }
        }
        return 0L;
    }


    inline string udf_open_alveo(int mode)
    {
        std::lock_guard<std::mutex> lockGuard(xai::writeMutexOpenAlveo);

        if (!xai::openedAlveo) {
            std::cout << "DEBUG: " << __FUNCTION__ << " xai::openedAlveo=" << xai::openedAlveo << std::endl;
            if(xai::openedAlveo) return "";
            xai::openedAlveo = true;
            string result("Initialized Alveo");
            try
            {
                std::cout << "DEBUG: Opening XAI library " << std::endl;
                xai::xaiLoader.load_library(xai::host_libname);
                std::cout << "DEBUG: Opened XAI library " << std::endl;
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

    // TODO: Change signature as needed
    // This function combined with GSQL code should traverse memory of TigerGraph on Each
    // server and build the partitions for each server in the form Louvain host code can consume it
    inline int udf_load_partitions(string num_partitions, string num_devices)
    {

        std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
        int ret=0;
        if (!xai::loadedAlveo) {
            if(xai::loadedAlveo) return 0;
            xai::loadedAlveo = true;
            xai::num_partitions = num_partitions;
            xai::num_devices = num_devices;
        }
        // TODO: make call into host code and add needed functions
    }

    inline int udf_create_load_alveo_partitions(bool use_saved_partition, string graph_file, string louvain_project, string num_partitions, string num_devices)
    {
        std::lock_guard<std::mutex> lockGuard(xai::writeMutex);
        int ret=0;
        if (!xai::loadedAlveo) {
            if(xai::loadedAlveo) return 0;
            xai::loadedAlveo = true;
            xai::graph_file = graph_file;
            xai::louvain_project = louvain_project;
            xai::num_partitions = num_partitions;
            xai::num_devices = num_devices;
            int argc = 9;
            char* argv[] = {"host.exe", xai::graph_file.c_str(), "-fast", "-par_num", xai::num_partitions.c_str(),
            "-create_alveo_partitions", "-name", xai::louvain_project.c_str(), "-server_par", NULL};
            std::cout << "DEBUG: Calling create_partitions. graph_file: " <<  xai::graph_file.c_str()
            << " num_partitions: " << xai::num_partitions.c_str() << " louvain project: " << xai::louvain_project.c_str()
            << std::flush;
            if(!use_saved_partition) {
                string workernum("1");
                bool isDriver = xai::isHostTheDriver(workernum);
                if(isDriver) {
                    std::cout
                    << "DEBUG: "
                    << "Calling xaiLoader.create_partitions" <<  "\n";
                    ret = xai::xaiLoader.create_partitions(argc, (char**)(argv));
                }
            }
            ret = 0;
        }
        return ret;
    }

    inline int udf_louvain_alveo(int64_t max_iter, int64_t max_level, float tolerence, bool intermediateResult, bool verbose, string result_file, bool print_final_Q, bool print_all_Q)
    {

        std::lock_guard<std::mutex> lockGuard(xai::writeMutex);

        if (!xai::openedAlveo) {
            std::cout << "ERROR: Please run udf_open_alveo first" << std::endl;
            return -1;
        }
        if (!xai::executedLouvain) {
            std::cout << "DEBUG: " << __FUNCTION__
            << " xai::executedLouvain=" << xai::executedLouvain << std::endl;

            if(xai::executedLouvain) return 0;
            xai::executedLouvain = true;
            string workernum("1");
            string num_workers("2");
            bool isDriver = xai::isHostTheDriver(workernum);
            int my_argc = 18;
            string worker_or_driver("-workerAlone");
            char* optional_arg = NULL;
            if(isDriver) {
                worker_or_driver = "-driverAlone";
            } else {
                optional_arg = (char*) workernum.c_str();
                my_argc++;
            }
            string partitions_project = xai::louvain_project + ".par.proj";

            char* my_argv[] = {"host.exe", "-x", xai::xclbin_filename, xai::graph_file.c_str(),
            "-o", result_file.c_str(), "-fast", "-dev",
            xai::num_devices.c_str(), "-par_num", xai::num_partitions.c_str(),
            "-load_alveo_partitions", partitions_project.c_str(),
            "-setwkr", num_workers.c_str(), "tcp://192.168.1.21:5555", "tcp://192.168.1.31:5555",
            worker_or_driver.c_str(), optional_arg, NULL};

            std::cout
            << "DEBUG: "
            << "Calling execute_louvain. input_graph: " <<  xai::graph_file.c_str()
            << " num_partitions: " << xai::num_partitions.c_str()
            << " project: " << partitions_project.c_str()
            << " num workers: " << num_workers.c_str()
            << " worker_or_driver: " << worker_or_driver.c_str()
            << " worker num: " << workernum.c_str() << "\n"
            << std::flush;

            int retVal = xai::xaiLoader.execute_louvain(my_argc, (char**)(my_argv));
            std::cout << "DEBUG: Returned from execute_louvain, isDriver = " << isDriver << "\n" << std::flush;
            return retVal;
        }
        return 0;
    }

    /* End Xilinx Louvain Additions */

}
/****************************************/

#endif /* EXPRFUNCTIONS_HPP_ */
