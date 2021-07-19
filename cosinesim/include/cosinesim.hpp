/*
 * Copyright 2020-2021 Xilinx, Inc.
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

#ifndef XILINX_APPS_COSINESIM_HPP
#define XILINX_APPS_COSINESIM_HPP

// Open default namespace for Doxygen
namespace xilinx_apps
{
namespace cosinesim
{

/**
 * @mainpage
 * 
 * ## Overview ##
 * 
 * The Cosine Similarity Alveo&tm; Product allows you to use a Xilinx Alveo accelerator card to find the best matches
 * of a given _target vector_ of integers within a set of _population vectors_ of integers.
 * The target vector is paired with each population vector in turn to compute the
 * [cosine similarity](https://en.wikipedia.org/wiki/Cosine_similarity)
 * score of the pair.  The scores are sorted, and the highest scores are returned, along with
 * an identifier (called its _row index_) for the corresponding population vectors.
 * 
 * ## Using the API ##
 * 
 * Follow the steps below to use the API.
 * 
 * 1. Instantiate a CosineSim object
 * 2. Load the population vectors into the Alveo accelerator card
 * 3. Run one or more matches by supplying for each run a target vector and the number of matches to return
 * 
 * ### Instantiate a CosineSim object ###
 * 
 * To instantiate a CosineSim object, first select the options for the object by instantiating an Options object
 * and setting its members, as shown in the example below.  Note that all API identifiers are contained within
 * the namespace xilinx_apps::cosinesim.
 * 
 * ~~~
 * #include "cosinesim.hpp"
 * 
 * xilinx_apps::cosinesim::Options options;
 * options.vecLength = 200;  // every target vector and population vector will have 200 elements
 * options.numDevices = 2;  // Use 2 Alveo accelerator cards
 * ~~~
 * 
 * Options::vecLength determines the _vector length_, or number of elements, of every population and target vector.
 * Options::numDevices determines how many Alveo accelerator cards to use for storing population vectors and executing
 * the cosine similarity match.  Options::numDevices should be at least 1 and no more than the number of installed
 * Alveo acceleration cards.  If you specify fewer than the number of installed cards, the choice of which cards are
 * used is undefined.
 * 
 * NOTE: Setting the `xclbinPath` and `xcbinPathCStr` data members of the Options object currently has no effect,
 * as the XCLBIN (FPGA program) file is always picked up from the default installation location under `/opt/xilinx`.
 * 
 * Next, instantiate the CosineSim object.  The template parameter specifies the integral type of each target and
 * population vector element.  Currently, only 32-bit signed integer types, such as `std::int32_t`, are supported.
 * 
 * ~~~
 * xilinx_apps::cosinesim::CosineSim<std::int32_t> cosineSim(options);
 * ~~~
 * 
 * ### Load the population vectors ###
 * 
 * Loading the population vectors into the Alveo accelerator card is accomplished with the procedure shown in the
 * code example below.
 * 
 * ~~~
 * cosineSim.startLoadPopulation(myPopVectors.size());
 * 
 * for (unsigned myIndex = 0; myIndex < myPopVectors.size(); ++myIndex) {
 *     // Get a population vector buffer and its row index
 *     xilinx_apps::cosinesim::RowIndex rowIndex = 0;
 *     buf = cosineSim.getPopulationVectorBuffer(rowIndex);
 * 
 *     // Fill the buffer and save the row index
 *     memcpy(buf, myPopVectors[myIndex], sizeof(myPopVectors[myIndex]));
 *     popMap[rowIndex] = myIndex;
 * 
 *     // Mark the buffer as finished
 *     cosineSim.finishCurrentPopulationVector(buf);
 * }
 * 
 * cosineSim.finishLoadPopulation();
 * ~~~
 * 
 * The entire loading process is started and ended with calls to CosineSim::startLoadPopulation() and
 * CosineSim::finishLoadPopulation(), respectively.  For CosineSim::startLoadPopulation() you must supply the total
 * number of population vectors to load.  In between the start and finish calls, for every population vector to add,
 * you must fetch a buffer from the CosineSim object and fill it with your population vector values.
 * 
 * The buffer to fill is fetched with CosineSim::getPopulationVectorBuffer().  In addition to returning a pointer to
 * the buffer, the function sets its argument to the _row index_ of the population vector within the Alveo card.
 * Because the row index may be different from your data index, you should save the row index in a map
 * from row index to your population vector (or associated object), so that when you perform the match, you can
 * identify your population vector based on the row index returned in the match results, as demonstrated in the next
 * section.
 * 
 * As the returned buffer is treated as a plain C integer array, you can use any C or C++ technique to copy vector
 * elements into the buffer. In the example above, `myPopVectors[myIndex]` is assumed to be a `Value[]` array, where
 * `Value` is the template parameter type of CosineSim, so `memcpy` is used.
 * 
 * After filling the buffer, you must call CosineSim::finishCurrentPopulationVector() to process the population vector.
 * After CosineSim::finishCurrentPopulationVector() is called, there is no way to modify the vector, and once
 * CosineSim::finishLoadPopulationVectors() has been called, there is no way to add more population vectors.  To change
 * the population vector set, you will need to resubmit all population vectors, starting with another call to
 * CosineSim::startLoadPopulation().
 * 
 * **Multi-threading note:** While the API is designed to accommodate writing to population vectors from multiple
 * threads simultaneously, the API functions themselves are not thread safe.  To use the API in a multi-threaded
 * application,you must wrap each call to CosineSim::getPopulationVectorBuffer() and
 * CosineSim::finishCurrentPopulationVector() in a critical section (with mutex locking).  Once a thread has acquired
 * a buffer, the thread can write to the buffer in parallel with other threads writing to their separate buffers.
 * The get and finish calls do not need to be in the same critical section.  That is, you can unlock the mutex
 * between the two function calls.
 * 
 * ### Run a match ###
 * 
 * After the population vectors have been loaded into the Alveo accelerator card, you can call
 * CosineSim::matchTargetVector() with a target vector to find the population vectors that have the highest cosine
 * similarity with the target vector.  The example below shows how to use the function.
 * 
 * ~~~
 * std::vector<xilinx_apps::cosinesim::Result> results;
 * results = cosineSim.matchTargetVector(10, testVector);
 * for (xilinx_apps::cosinesim::Result &result : results)
 *     std::cout << result.similarity << "       " << popMap[result.index] << std::endl;
 * ~~~
 * 
 * In the example, `testVector`, the target vector, is assumed to be a C array of `Value` integers.
 * Along with that target vector, the function takes the number of results that you would like it to return.
 * In this case, we're requesting the top 10 matches.  The maximum number of results currently supported is 100.
 * The function returns a `std::vector` of Result objects, where each object contains the cosine similarity score
 * (Result::similarity) and row index (Result::index) of a population vector.  Use the map you built during
 * population vector loading to convert the returned row index back to your population vector index or object.
 * 
 * You can call CosineSim::matchTargetVector() repeatedly with different target vectors.  The target vectors do not
 * need to be present in the set of population vectors, as each call to CosineSim::matchTargetVector() transfers the
 * target vector to the Alveo accelerator card before running the cosine similarity search.
 * 
 * ## Alveo accelerator card storage capacity ##
 * 
 * The number of population vectors that an Alveo accelerator card can hold depends on both the vector length of
 * a population vector as well as the memory capacity of the Alveo accelerator card.  The Alveo U50 accelerator card,
 * for example, can hold approximately
 * 
 * > 1.6 billion / `len`
 * 
 * population vectors, where `len` is the vector length rounded up to a multiple of 4.
 * 
 * ## Error handling ##
 * 
 * Every CosineSim member function can potentially throw an exception of type Exception if a run-time error, such as
 * a hardware communication error, is encountered.  You can wrap your load and match operations in a `try`/`catch` block
 * to handle the error.  The exception object provides the Exception::what() member function for retrieving a text
 * message for the error.  As there are currently no programmatically recoverable errors, the Exception object does
 * not supply an error code.  As with any hardware device, recovering from an Alveo accelerator card error may require
 * the intervention of a system operator, so you should consider sending the error message to a destination suitable
 * for the system administrator to access it.
 * 
 * API usage errors are handled within CosineSim simply by emitting an error message to `stdout` or `stderr` and
 * aborting.  API usage errors include passing out-of-range or unsupported values as
 * arguments to CosineSim member functions.
 * 
 * ## Linking your application ##
 * 
 * You have a few choices for how to link the API code into your application:
 * 
 * - Linking directly with the Cosine Similarity shared library (.so)
 * - Linking with the Cosine Similarity dynamic loader archive (.a)
 * - Including the Cosine Similarity dynamic loader source file (.cpp)
 * 
 * ### Linking directly (.so) ###
 * 
 * The simplest method of linking the API into your application is to link directly with the shared library (.so),
 * placing a run-time dependency of your application on the shared library.  Simply add the following arguments to
 * your link line:
 * 
 * ~~~{.mk}
 * -L/opt/xilinx/apps/graphanalytics/cosinesim/@version/lib -lXilinxCosineSim
 * ~~~
 * 
 * ### Linking with the dynamic loader archive (.a) ###
 * 
 * To avoid having a run-time dependency on the shared library, but instead load the shared library on demand
 * (internally using `dlopen()`), you can link with the loader archive by adding the following arguments to
 * your link line:
 * 
 * ~~~{.mk}
 * -L/opt/xilinx/apps/graphanalytics/cosinesim/@version/lib -lXilinxCosineSim_loader -ldl
 * ~~~
 * 
 * ### Including the dynamic loader source file (.cpp) ###
 * 
 * Another way to avoid a run-time dependency on the shared library is by including the loader source file in
 * a header or source file of your program:
 * 
 * ~~~
 * #define XILINX_COSINESIM_INLINE_IMPL
 * #include "cosinesim.hpp"
 * 
 * // Code that uses the API goes here
 * 
 * #include "cosinesim_loader.cpp"
 * ~~~
 * 
 * Note that you will still have to include `-ldl` on your link line to pull in the standard dynamic loading library.
 * 
 * The loader source file is located in `/opt/xilinx/apps/graphanalytics/cosinesim/@version/src`.  Note the macro
 * definition that comes before the inclusion of `%cosinesim.hpp`.
 * 
 * **TIP:** When using either dynamic loading technique, if the order of symbol loading causes unexplained behavior in
 * your application, you can try adding `libXilinxCosineSim.so` to the list of pre-loaded shared libraries,
 * as explained in [this Stack Overflow article](https://stackoverflow.com/questions/426230/what-is-the-ld-preload-trick).
 * 
 * ## Type-erased CosineSim base class ##
 * 
 * The CosineSim class is a template class that ensures type safety for vector elements.  However, if you need
 * a type-free non-template class, you can use CosineSim's base class, CosineSimBase, directly.  Its member functions
 * are the same as for the template class, except that data pointers are of type `void *`, and you must also pass
 * the length in bytes of a vector element.
 * 
 */
}}

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <exception>
#include <cstring>

namespace xilinx_apps
{
namespace cosinesim
{
struct Options;
class ImplBase;
}

}

/**
 * Define this macro to make functions in cosinesim_loader.cpp inline instead of extern.  You would use this macro
 * when including cosinesim_loader.cpp in a header file, as opposed to linking with libXilinxCosineSim_loader.a.
 */
#ifdef XILINX_COSINESIM_INLINE_IMPL
#define XILINX_COSINESIM_IMPL_DECL inline
#else
#define XILINX_COSINESIM_IMPL_DECL extern
#endif

/// @cond INTERNAL
extern "C" {
XILINX_COSINESIM_IMPL_DECL
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(const xilinx_apps::cosinesim::Options& options, unsigned valueSize);

XILINX_COSINESIM_IMPL_DECL
void xilinx_cosinesim_destroyImpl(xilinx_apps::cosinesim::ImplBase *pImpl);
}
/// @endcond

namespace xilinx_apps {
namespace cosinesim {

using RowIndex = std::int64_t;  ///< Integral type for population vector row indexes
using ColIndex = std::int32_t;  ///< Integral type for population or target vector element indexes


/**
 * @brief %Exception class for cosine similarity run-time errors
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
 * @brief Tuple (struct) that holds one cosine similarity match result
 */
struct Result {
    RowIndex index = -1L;  ///< the row index of the matched population vector
    double similarity = 0.0;  ///< the cosine similarity score for the match

    /**
     * Constructs a Result tuple given its fields.
     * 
     * @param index the row index of a matched population vector
     * @param similarity the cosine similarity score of the match
     */
    Result(RowIndex index, double similarity) {
        this->index = index;
        this->similarity = similarity;
    }
};

/**
 * @brief Struct containing CosineSim configuration options
 */
struct Options {
    /// vector length of all population and target vectors.  Default is 200.
    ColIndex vecLength;
    /// number of Alveo accelerator cards to use.  Default is 1.
    std::int32_t numDevices;
    
    /**
     * FPGA binary file (XCLBIN) path.  Default is the package installation path.
     * 
     * When setting this field, the field takes ownership of the buffer pointed to.
     * The buffer must have been allocated with new[].
     * To avoid dealing with allocation, use the @ref setXclbinPath() function instead.
     */
    char *xclbinPath = nullptr;

    /**
     * Destroys this Options object.
     */
    ~Options() {
        delete[] xclbinPath;
    }
    
    /**
     * Constructs an Options object with default values.
     */
    Options() = default;
    
    /**
     * Copy constructor, which deep-copies the reference Options object.
     * @param opt the Options object to copy from
     */
    Options(const Options &opt) { copyIn(opt); }
    
    /**
     * Deep-copies the reference Options object into this Options object.
     * 
     * @param opt the Options object to copy from
     * @return this Options object
     */
    Options &operator=(const Options &opt) { copyIn(opt); return *this; }
    
    /**
     * Sets the @ref xclbinPath field to the given string, which is deep-copied.
     * 
     * @param newXclbinPath the XCLBIN path string to set @ref xclbinPath to
     */
    void setXclbinPath(const char *newXclbinPath) {
        delete[] xclbinPath;
        xclbinPath = nullptr;
        if (newXclbinPath != nullptr) {
            xclbinPath = new char[std::strlen(newXclbinPath) + 1];
            std::strcpy(xclbinPath, newXclbinPath);
        }
    }
    
    /**
     * Sets the @ref xclbinPath field to the given string, which is deep-copied.
     * 
     * @param newXclbinPath the XCLBIN path string to set @ref xclbinPath to
     */
    void setXclbinPath(const std::string &xclbinPath) { setXclbinPath(xclbinPath.c_str()); }

private:    
    void copyIn(const Options &opt) {
        vecLength = opt.vecLength;
        numDevices = opt.numDevices;
        setXclbinPath(opt.xclbinPath);
    }
};


template <typename Value>
class CosineSim;

/// @cond INTERNAL
class ImplBase {
public:
    virtual ~ImplBase(){};
    virtual void startLoadPopulation(std::int64_t numVertices) = 0;
    virtual void *getPopulationVectorBuffer(RowIndex &rowIndex) = 0;
    virtual void finishCurrentPopulationVector(void * pbuf) = 0;
    virtual void finishLoadPopulation() =0;
    virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements) = 0;
    virtual void cleanGraph() =0;
};
/// @endcond

/**
 * @brief Non-template version of CosineSim class.
 * 
 * This class provides the same functionality as the template-based CosineSim class, except without the type
 * parameter for vector elements.  Use this class for situations where a template-based object would be inconvenient,
 * such as when building a wrapper for this API for a language which does not support C++ template implementations.
 */
class CosineSimBase {
public:
    /**
     * Constructs a CosineSimBase object.
     * 
     * @param options the configuration options for consine similarity operations.  See the Options struct for details.
     * @param valueSize the size in bytes of each vector element.  4 is the only supported value for this release.
     */
    CosineSimBase(const Options &options, unsigned valueSize)
    : options_(options), pImpl_(::xilinx_cosinesim_createImpl(options, valueSize))
    {}

    /**
     * Destroys this CosineSimBase object.
     */
    ~CosineSimBase() {
        pImpl_->cleanGraph();
        ::xilinx_cosinesim_destroyImpl(pImpl_);
    }

    /**
     * Starts the procedure for loading population vectors to the Alveo accelerator card.
     * 
     * @param numVectors the total number of population vectors to load
     * 
     * Call this function at the start of loading or reloading all population vectors to the Alveo accelerator card.
     */
    void startLoadPopulation(std::int64_t numVectors){pImpl_->startLoadPopulation(numVectors);}  //

    /**
     * Allocates a buffer for the caller to fill with population vector element values.
     * 
     * @param rowIndex an integer variable to receive the row index for the buffer
     * @return a memory buffer large enough to hold enough integers for one population vector
     * 
     * This function is the same as CosineSim::getPopulationVectorBuffer(), except that the returned buffer
     * is a type-free `void *` instead of an array of a typed element value.
     */
    void *getPopulationVectorBuffer(RowIndex &rowIndex) {
        return pImpl_->getPopulationVectorBuffer(rowIndex);
    }

    /**
     * Processes a population vector buffer after the caller has filled it with population vector values.
     * 
     * @param pbuf the population vector buffer
     * 
     * This function is the same as CosineSim::finishCurrentPopulationVector(), except that the buffer for this
     * function has no element type (that is, the buffer is a `void *`).
     */
    void finishCurrentPopulationVector(void *pbuf){pImpl_->finishCurrentPopulationVector(pbuf);}

    /**
     * Ends the procedure for loading population vectors to the Alveo accelerator card.
     * 
     * Call this function after loading all population vectors to transfer the population vectors to
     * the Alveo acclerator cards.
     */
    void finishLoadPopulation(){pImpl_->finishLoadPopulation();}

    /**
     * Runs a match of a given target vector against all population vectors.
     * 
     * @param numResults the number of match results to return
     * @param elements a C array of target vector elements
     * @return a `std::vector` of Result objects, one per match result
     * 
     * This function is the same as CosineSim::matchTargetVector(), except without the type safety of the array type.
     */
    std::vector<Result> matchTargetVector(unsigned numResults, void *elements) {
        return pImpl_->matchTargetVector(numResults, elements);
    }

private:
    Options options_;
    ImplBase *pImpl_ = nullptr;
};


/**
 * @brief The main API class for the Cosine Similarity Alveo Product
 * 
 * Instantiate an object of this class to store population vectors in an Alveo accelerator card and run
 * cosine similarity matches with target vectors.
 */
template <typename Value>
class CosineSim : public CosineSimBase {
public:
    /**
     * the integral type of population and target vector elements (same as the `Value` template parameter)
     */
    using ValueType = Value;
    
    /**
     * Returns the configuration settings with which this CosineSim object was constructed.  See Options for details.
     * 
     * @return an Options struct containing the configuration settings
     */
    const Options& getOptions() {return options_;};

   // CosineSim( const Options &options) :options_(options), pImpl_(::xilinx_cosinesim_createImpl(options, sizeof(Value)))
    
    /**
     * Constructs a CosineSim object.
     * 
     * @param options the configuration options for consine similarity operations.  See the Options struct for details.
     */
    CosineSim(const Options &options) : CosineSimBase(options, sizeof(Value)) {};

    /**
     * Allocates a buffer for the caller to fill with population vector element values.
     * 
     * @param rowIndex an integer variable to receive the row index for the buffer
     * @return a memory buffer large enough to hold enough integers for one population vector
     * 
     * Call this function to fetch a buffer to fill with integers for one population vector.  Also supply
     * an integer variable, which is passed by reference, to receive the assigned row index for the population
     * vector.  Upon running matchTargetVector(), match results will contain this same row index to refer to the
     * matching population vector.
     * 
     * After fetching the memory buffer, write your population vector values to the buffer.  The buffer will be
     * interpreted as a C array whose element type is an integer of the size specified in the `valueSize`
     * argument to the CosineSimBase() constructor.
     * 
     * You may write to separate buffers from multiple threads simultaneously, but this function itself is
     * not thread safe, so be sure to call it in a critical section.
     */
    Value *getPopulationVectorBuffer(RowIndex &rowIndex) {
        return reinterpret_cast<Value *>(CosineSimBase::getPopulationVectorBuffer(rowIndex));
    }


    /**
     * Processes a population vector buffer after the caller has filled it with population vector values.
     * 
     * @param pbuf the population vector buffer
     * 
     * After fetching a buffer with CosineSim::getPopulationVectorBuffer() and filling it with population vector
     * values, call this function, passing the same buffer, to process the buffer for sending to the Alveo
     * accelerator card.
     */
    void finishCurrentPopulationVector(Value *pbuf){CosineSimBase::finishCurrentPopulationVector(pbuf);}

    /**
     * Runs a match of a given target vector against all population vectors.
     * 
     * @param numResults the number of match results to return
     * @param elements a C array of target vector elements
     * @return a `std::vector` of Result objects, one per match result
     * 
     * The `elements` array argument must contain as many elements as specified in Options::vecLength.
     */
    std::vector<Result> matchTargetVector(unsigned numResults, Value *elements) {
        return CosineSimBase::matchTargetVector(numResults, elements);
    }
private:
    Options options_;
    ImplBase *pImpl_ = nullptr;

};

} // namespace cosinesim
} // namespace xilinx_apps


#endif /* XILINX_APPS_COSINESIM_HPP */
