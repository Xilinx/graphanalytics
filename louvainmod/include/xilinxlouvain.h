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
#ifndef _XILINXLOUVAIN_H_
#define _XILINXLOUVAIN_H_

#include <exception>
#include <string>

enum {
	ALVEOAPI_NONE=0,
	ALVEOAPI_PARTITION,
	ALVEOAPI_LOAD,
	ALVEOAPI_RUN
};

/**
 * Define this macro to make functions in louvainmod_loader.cpp inline instead of extern.  You would use this macro
 * when including louvainmod_loader.cpp in a header file, as opposed to linking with libXilinxCosineSim_loader.a.
 */
#ifdef XILINX_LOUVAINMOD_INLINE_IMPL
#define XILINX_LOUVAINMOD_IMPL_DECL inline
#else
#define XILINX_LOUVAINMOD_IMPL_DECL extern
#endif

extern "C" {

XILINX_LOUVAINMOD_IMPL_DECL
int create_alveo_partitions(int argc, char *argv[]);

XILINX_LOUVAINMOD_IMPL_DECL
float load_alveo_partitions_wrapper(int argc, char *argv[]);

}

float louvain_modularity_alveo(int argc, char *argv[]);
int compute_modularity(char* inFile, char* clusterInfoFile, int offset);

namespace xilinx_apps {
namespace louvainmod {


/**
 * @brief %Exception class for Louvain modularity run-time errors
 * 
 * This exception class is derived from `std::exception` and provides the standard @ref what() member function.
 * An object of this class is constructed with an error message string, which is stored internally and
 * retrieved with the @ref what() member function.
 */
class Exception : public std::exception {
    std::string message;
public:
    /**
     * Constructs an Exception object.
     * 
     * @param msg an error message string, which is copied and stored internal to the object
     */
    Exception(const std::string &msg) : message(msg) {}
    
    /**
     * Returns the error message string passed to the constructor.
     * 
     * @return the error message string
     */
    virtual const char* what() const noexcept override { return message.c_str(); }
};

}  // namespace louvainmod
}  // namespace xilinx_apps


#endif
