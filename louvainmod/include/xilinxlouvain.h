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
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace xilinx_apps {
namespace louvainmod {

class LouvainModImpl;
class Options;

}
}

// TODO: Move this to internal header after create/load_alveo_partitions are removed
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
int create_and_load_alveo_partitions(int argc, char *argv[]);

XILINX_LOUVAINMOD_IMPL_DECL
float load_alveo_partitions_wrapper(int argc, char *argv[]);

XILINX_LOUVAINMOD_IMPL_DECL
xilinx_apps::louvainmod::LouvainModImpl *xilinx_louvainmod_createImpl(const xilinx_apps::louvainmod::Options& options);

XILINX_LOUVAINMOD_IMPL_DECL
void xilinx_louvainmod_destroyImpl(xilinx_apps::louvainmod::LouvainModImpl *pImpl);

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


/**
 * @brief Simple string class for avoiding ABI problems
 * 
 * This class provides string memory management like `std::string` but without ABI compatibility issues.
 * ABI (Application Binary Interface) problems can arise when using standard C++ classes in a coding environment
 * that uses multiple compiler versions.  For example, if you compile your application code using a different
 * version of the g++ compiler from the one used to compile this library, then when using dynamic loading,
 * standard C++ types, such as `std::string`, may not pass correctly from your code into the library.
 * This string class avoids these compatibility issues by using only plain data types internally.
 */
class XString {
public:
    XString() = default;
    ~XString() { clear(); }
    XString(const XString &other) { copyIn(other.data); }
    XString(XString &&other) { steal(std::forward<XString>(other)); }
    XString(const char *cstr) { copyIn(cstr); }
    XString &operator=(const XString &other) { copyIn(other.data); return *this; }
    XString &operator=(XString &&other) { steal(std::forward<XString>(other)); return *this; }
    XString &operator=(const char *cstr) { copyIn(cstr); return *this; }
    operator std::string() const { return (data == nullptr) ? std::string() : std::string(data); }
    operator const char *() const { return data; }
    const char *c_str() const { return data; }
    bool empty() const { return data == nullptr || std::strlen(data) == 0; }

    bool operator==(const XString &other) const {
        if (data == nullptr && other.data == nullptr)
            return true;
        if (data == nullptr || other.data == nullptr)
            return false;
        return std::strcmp(data, other.data) == 0;
    }

    void clear() {
        if (data != nullptr)
            std::free(data);
        data = nullptr;
    }

private:
    char *data = nullptr;
    
    void copyIn(const char *other) {
        clear();
        if (other != nullptr) {
            data = static_cast<char *>(std::malloc(std::strlen(other) + 1));
            std::strcpy(data, other);
        }
    }
    
    void steal(XString &&other) {
        clear();
        data = other.data;
        other.data = nullptr;
    }
};


struct Edge {
    long head;
    long tail;
    double weight;
};


struct Options {
    bool verbose = true;
};


class LouvainMod {
public:
    /// Index for a vertex, which should be between 0 and the total number of vertices in the whole graph.
    using VertexIndex = std::uint64_t;
    
    struct PartitionOptions {
        int flow_fast = 2;  // C
        XString nameProj;  // C
        int num_par = 1;  // total number of partitions (across all servers)?  // CF
        int devNeed_cmd = 1;  // C
        int par_prune = 1;  // C
        int numServers = 1;  // number of servers // CF
        
        PartitionOptions() = default;
        PartitionOptions(const PartitionOptions &opt) = default;
        PartitionOptions &operator=(const PartitionOptions &opt) = default;
    };
    
    struct PartitionData {
        const long* offsets_tg = nullptr;  // values are indexes into edgelist_tg (not into some global edge list)
        const Edge* edgelist_tg = nullptr; // head and tail are global vertex ids
        const long* drglist_tg = nullptr;
        long start_vertex = 0;
        long end_vertex = 0;
        long NV_par_recommand = 0;  // Recommended NV per partition.  Leave as 0 to calculate from num Alveo cards on each server
    };

    struct RunOptions {
        
    };
    
    LouvainMod(const Options &options) : pImpl_(xilinx_louvainmod_createImpl(options)) {}
    ~LouvainMod() { xilinx_louvainmod_destroyImpl(pImpl_); }
    
    /**
     * Reads an input data file and partitions it in preparation for computing Louvain modularity.
     * 
     * @param fileName the name of the file containing the graph to partition
     */
    void partitionDataFile(const char *fileName, const PartitionOptions &options);
    
    void startPartitioning(const PartitionOptions &options);
    int addPartitionData(const PartitionData &);  // Returns actual number of partitions created
    void finishPartitioning();
    
    void loadAlveo();  // Loads .par files into CPU memory.  Can we load first .par per card into HBM here?
    void runLouvain(const RunOptions &);
    
private:
    LouvainModImpl *pImpl_ = nullptr;
};

}  // namespace louvainmod
}  // namespace xilinx_apps


#endif
