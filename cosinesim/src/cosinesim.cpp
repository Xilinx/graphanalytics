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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <assert.h>

#include "cosinesim.hpp"
#include "xf_graph_L3.hpp"
#include "xilinx_apps_common.hpp"

namespace xilinx_apps {
namespace cosinesim {
class PrivateImpl : public ImplBase {

public:
    unsigned indexDeviceCuNm, indexSplitNm, indexNumVertices;
    unsigned numPUs_ = 3;    // number of partitions based on number of PUs
    const unsigned channelsPU = 4; // each PU has 4 HBM channels
    const unsigned cuNm = 2;       // TODO
    const unsigned channelW = 16;

    int valueSize_;
    uint32_t numDevices;
    XString deviceNames;
    int populationVectorRowNm;
    int32_t** numVerticesPU ; // vertex numbers in each PU


    int32_t numEdges;
    int vecLength;
    int64_t numVertices;
    XString xclbinPath;
    std::string kernelName_;
    std::string kernelAlias_;
    int32_t edgeAlign8;
    int subChNm;
    int lastPopulationCnt;
    //std::vector<int> loadOldVectorCnt(channelsPU,0);
    std::vector<int> loadPopulationCnt;

    //xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
    //currently only support int32_t graph
    std::vector<xf::graph::Graph<int32_t, int32_t>*> g;
    //xf::graph::Graph<int32_t, int32_t>** g;

    int load_xgraph_fpga(uint32_t numVertices, uint32_t numEdges, xf::graph::Graph<uint32_t, float> g);
    void load_cu_cosinesim_ss_dense_fpga();
    void load_graph_cosinesim_ss_dense_fpga(uint32_t deviceNeeded, uint32_t cuNm, xf::graph::Graph<int32_t, int32_t>** graph);


    void cosinesim_ss_dense_fpga(uint32_t devicesNeeded,
                                 int32_t sourceLen,
                                 int32_t* sourceWeight,
                                 int32_t* sourceCoeffs,
                                 int32_t topK,
                                 xf::graph::Graph<int32_t, int32_t>** g,
                                 int32_t* resultID,
                                 float* similarity);

    PrivateImpl(const Options &options, unsigned valueSize){
        //errorCode_ = NoError;
        valueSize_ = valueSize;
        if(valueSize_ != 4) {
            std::cout << "DEBUG: valueType is not supported" << std::endl;
            std::ostringstream oss;
            oss << "the only Value size currently supported is 32 bits.  Please ensure that you have constructed the CosineSim object with a 32-bit template parameter" <<std::endl;
            throw xilinx_apps::cosinesim::Exception(oss.str());
            std::cerr << "ERROR: valueType is not supported" <<std::endl;
            abort();
        }


        //cosinesimPtr = ptr;
        indexDeviceCuNm=0;
        indexSplitNm=0;
        indexNumVertices=0;
        populationVectorRowNm=0;
        subChNm = 1;
        vecLength = 200;
        if(options.vecLength > 0)
            vecLength = options.vecLength;

        numDevices = 1;
        if (options.numDevices > 0)
            numDevices = options.numDevices;

        deviceNames = options.deviceNames;
        // default xclbin to load
        if (deviceNames == XString("xilinx_u50_gen3x16_xdma_201920_3")) {
            xclbinPath = std::string("/opt/xilinx/apps/graphanalytics/cosinesim/") + std::string(VERSION) + 
                         std::string("/xclbin/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin");
            kernelName_ = "denseSimilarityKernel";
            kernelAlias_ = "denseSimilarityKernel_U50";
            numPUs_ = 3;
        } else if (deviceNames == XString("xilinx_u55c_gen3x16_xdma_base_2")) {
            xclbinPath = std::string("/opt/xilinx/apps/graphanalytics/cosinesim/") + std::string(VERSION) + 
                         std::string("/xclbin/cosinesim_32bit_4pu_xilinx_u55c_gen3x16_xdma_base_2.xclbin");
            kernelName_ = "denseSimilarityKernel4PU";
            kernelAlias_ = "denseSimilarityKernel4PU_U55C";
            numPUs_ = 4;
        } else if (deviceNames == XString("xilinx_aws-vu9p-f1_shell-v04261818_201920_2")) {
            xclbinPath = std::string("/opt/xilinx/apps/graphanalytics/cosinesim/") + std::string(VERSION) + 
                         std::string("/xclbin/cosinesim_xilinx_aws-vu9p-f1_shell-v04261818_201920_2.awsxclbin");
            kernelName_ = "denseSimilarityKernel";
            //kernelAlias_ = "denseSimilarityKernel_F1";
            numPUs_ = 1;
        } else {
            std::cout << "DEBUG: deviceNames " << deviceNames << " is not supported" << std::endl;
            std::ostringstream oss;
            oss << "Supported deviceNames: xilinx_u50_gen3x16_xdma_201920_3 and xilinx_u55c_gen3x16_xdma_base_2" <<std::endl;
            throw xilinx_apps::cosinesim::Exception(oss.str());
            std::cerr << "ERROR: deviceNames " << deviceNames << " not supported" <<std::endl;
            abort();
        }

        if (!options.xclbinPath.empty()) {
            std::cout << "INFO: xclbinPath set to options::xcbinPath: " << options.xclbinPath << std::endl;
            xclbinPath = options.xclbinPath;
        } else {
            std::cout << "INFO: xclbinPath set to default:" << xclbinPath<<std::endl;
        }
        std::cout << "INFO: numDevices set to " << numDevices << std::endl;

        numEdges = vecLength;

        //options_ = options;
        edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

        numVerticesPU  = new int32_t*[numDevices * cuNm];
        for (unsigned i = 0; i < numDevices * cuNm; ++i) {
            numVerticesPU[i] = new int32_t[numPUs_];
        }

        edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

        loadPopulationCnt.resize(channelsPU,0);
        g.resize(numDevices*cuNm);
        // g = new xf::graph::Graph<int32_t, int32_t>*[devicesNeeded * cuNm];



        load_cu_cosinesim_ss_dense_fpga();


    }

    ~PrivateImpl(){
      for (unsigned i = 0; i < numDevices * cuNm; ++i)
            delete[] numVerticesPU[i];
      delete[] numVerticesPU;
    }

    virtual void startLoadPopulation(std::int64_t numVertices){
        //--------------- Free and delete -----------------------------------

        cleanGraph();

        indexDeviceCuNm=0;
        indexSplitNm=0;
        indexNumVertices=0;
        populationVectorRowNm=0;
        this->numVertices = numVertices;
        //the following calculation and assignment is based on numVertices
        int general = ((numVertices - (numDevices * cuNm * numPUs_ * channelsPU + 1)) /
                (numDevices * cuNm * numPUs_ * channelsPU)) * channelsPU;
        // handle the case where numVertices is too small
        if (general == 0)
            general = numVertices/(numDevices * cuNm * numPUs_);

        int rest = numVertices - general * (numDevices * cuNm * numPUs_ - 1);

#ifndef NDEBUG
        std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
                << "\n    numVertices=" << numVertices 
                << "\n    cuNm=" << cuNm 
                << "\n    numVertices=" << numVertices 
                << "\n    numPUs_=" << numPUs_
                << "\n    channelsPU=" << channelsPU
                << "\n    general=" << general
                << "\n    rest=" << rest
                << "\n    split=" << numDevices * cuNm * numPUs_ 
                << "\n    numDevices=" << numDevices << std::endl;
#endif

        //assert((rest >= 0));


        if (rest < 0) {
            std::cerr << "ERROR: Graph Partition is Failed." <<std::endl;
            abort();

        }

        for (unsigned i = 0; i < numDevices * cuNm; ++i) {
            for (unsigned j = 0; j < numPUs_; ++j) {
                numVerticesPU[i][j] = general;
            }
        }
        numVerticesPU[numDevices * cuNm - 1][numPUs_ - 1] = rest;
        //initialize graph properties
        int fpgaNodeNm = 0;
        for(unsigned i=0;i<numDevices*cuNm; i++){
            //TODO change to Value
            //g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);

            if(valueSize_ == 4) {
                g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * numPUs_, numEdges,  numVerticesPU[i]);
            } else {
                // must exit at this point, or else crash on next line.  Throw exception here.
                std::cout << "DEBUG: valueType is not supported" << std::endl;
                std::ostringstream oss;
                oss << "the only Value size currently supported is 32 bits.  Please ensure that you have constructed the CosineSim object with a 32-bit template parameter" <<std::endl;
                throw xilinx_apps::cosinesim::Exception(oss.str());
                std::cerr << "ERROR: valueType is not supported." <<std::endl;
                abort();

            }
            g[i]->numEdgesPU = new int32_t[numPUs_];
            g[i]->numVerticesPU = new int32_t[numPUs_];
            g[i]->edgeNum = numEdges;
            g[i]->nodeNum = numVertices;
            g[i]->splitNum = numPUs_;
            g[i]->refID = fpgaNodeNm;
            for (unsigned j = 0; j < numPUs_; ++j) {
                fpgaNodeNm += numVerticesPU[i][j];
                int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
                g[i]->numVerticesPU[j] = numVerticesPU[i][j];
                g[i]->numEdgesPU[j] = depth;

            }
        }



    }

    virtual void *getPopulationVectorBuffer(RowIndex &rowIndex){

        void * pbuf;
        if(indexDeviceCuNm == numDevices * cuNm)
            return nullptr;
        subChNm = (numVerticesPU[indexDeviceCuNm][indexSplitNm] + channelsPU - 1) / channelsPU;
#ifndef NDEBUG
        std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
                << "\n    numVerticesPU[indexDeviceCuNm][indexSplitNm]=" << numVerticesPU[indexDeviceCuNm][indexSplitNm] 
                << "\n    indexDeviceCuNm=" << indexDeviceCuNm
                << "\n    indexSplitNm=" << indexSplitNm
                << "\n    subChNm=" << subChNm << std::endl;
#endif

        pbuf = &(g[indexDeviceCuNm]->weightsDense[indexSplitNm * channelsPU + indexNumVertices / subChNm][loadPopulationCnt[indexNumVertices / subChNm] * edgeAlign8]);

        loadPopulationCnt[indexNumVertices / subChNm] += 1;
        lastPopulationCnt = loadPopulationCnt[indexNumVertices / subChNm];
        indexNumVertices++;
        if(indexNumVertices == unsigned(numVerticesPU[indexDeviceCuNm][indexSplitNm])) {
            indexSplitNm++;
            indexNumVertices=0;
            std::fill(loadPopulationCnt.begin(),loadPopulationCnt.end(),0);
            if(indexSplitNm == numPUs_) {
                indexDeviceCuNm++;
                indexSplitNm = 0;
            }
        }

        rowIndex = populationVectorRowNm++;
        return pbuf;

    }

    virtual void finishCurrentPopulationVector(void * pbuf){
        memset((unsigned char *)pbuf+vecLength*valueSize_, 0, ( edgeAlign8 - vecLength)*valueSize_ );


    }

    //padding the row and loadgraph
    virtual void finishLoadPopulation() {
        //for the last PU, the last channels may need to padded as the row will be the multiple of channelsPU
        const unsigned numCus = numDevices * cuNm;
        int paddingDepth = ((numVerticesPU[numCus - 1][numPUs_ - 1] + channelsPU - 1) / channelsPU) * channelsPU
                - numVerticesPU[numCus - 1][numPUs_ - 1];
        int startPaddingDepth = numVerticesPU[numCus - 1][numPUs_ - 1] * edgeAlign8;
        for (int k = startPaddingDepth; k < paddingDepth*edgeAlign8; ++k) {
            g[indexDeviceCuNm]->weightsDense[ (indexSplitNm-1) * channelsPU + 3][k] = 0;
        }

        load_graph_cosinesim_ss_dense_fpga(numDevices, cuNm, g.data());

    }

    virtual void cleanGraph() {

        for (unsigned i = 0; i < numDevices * cuNm; ++i) {
            if (!g[i])
                continue;

            g[i]->freeBuffers();
            delete[] g[i]->numEdgesPU;
            g[i]->numEdgesPU = nullptr;
            delete[] g[i]->numVerticesPU;
            g[i]->numVerticesPU = nullptr;
            delete g[i];
            g[i] = nullptr;

        }

    }


    std::vector<xf::graph::L3::event<int> > cosineSimilaritySSDenseMultiCard(
        std::shared_ptr<xf::graph::L3::Handle>& handle,
        int32_t numDevices,
        int32_t sourceNUM,
        int32_t* sourceWeights,
        int32_t* sourceCoeffs,
        int32_t topK,
        xf::graph::Graph<int32_t, int32_t>** g,
        int32_t** resultID,
        float** similarity) 
    {
        std::vector<xf::graph::L3::event<int> > eventQueue;
        std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time;

        for (int i = 0; i < numDevices; ++i) {
#ifdef __PROFILING__            
            l_start_time = std::chrono::high_resolution_clock::now();           
            std::cout << "LOG2TIMELINE: " << __FUNCTION__
                      << "::addworkInt" << i << " start=" << l_start_time.time_since_epoch().count()
                      << std::endl;
#endif
            eventQueue.push_back(
                (handle->opsimdense)->addworkInt(
                    1, // similarityType
                    0, // dataType
                    sourceNUM, 
                    sourceWeights, 
                    sourceCoeffs,
                    topK, 
                    g[i][0], 
                    resultID[i], 
                    similarity[i]));
        }
        return eventQueue;
    };

    virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements){
        // Don't allow more results to be returned than the number of population vectors.  The kernel would return
        // blank results (index and similarity 0) in that case, so we need to prevent it here.
        if (numResults > this->numVertices)
            numResults = this->numVertices;
        
        std::vector<Result> result;
        //---------------- Generate Source Indice and Weight Array -------
        int sourceLen = edgeAlign8; // sourceIndice array length
        int32_t* sourceFeatures = xf::graph::internal::aligned_alloc<int32_t>(sourceLen); // values of features in source
        int32_t* sourceCoeffs = xf::graph::internal::aligned_alloc<int32_t>(sourceLen);   // weights of features in source 
#ifndef NDEBUG
        std::cout << "DEBUG: " << __FUNCTION__ << " sourceLen=" << sourceLen << std::endl;
#endif
        for (int i = 0; i < sourceLen; i++) {
            if (i < vecLength) {
                if(valueSize_ == 4)
                    sourceFeatures[i] = (reinterpret_cast<int32_t*>(elements))[i];
                else {
                    // must exit at this point, or else crash on next line.  Throw exception here.
                    std::cout << "DEBUG: valueType is not supported" << std::endl;
                    std::ostringstream oss;
                    oss << "the only Value size currently supported is 32 bits.  Please ensure that you have constructed the CosineSim object with a 32-bit template parameter" <<std::endl;
                    throw xilinx_apps::cosinesim::Exception(oss.str());
                    std::cerr << "ERROR: valueType is not supported." <<std::endl;
                    abort();

                }
            } else
                sourceFeatures[i] = 0;

            sourceCoeffs[i] = 1; // set weights to 1 for now. TODO: need to pass ths from graph
        }
            
        float* similarity = xf::graph::internal::aligned_alloc<float>(numResults);
        int32_t* resultID = xf::graph::internal::aligned_alloc<int32_t>(numResults);
        std::memset(resultID, 0, numResults * sizeof(int32_t));
        std::memset(similarity, 0, numResults * sizeof(float));

        //---------------- Run L3 API -----------------------------------
        cosinesim_ss_dense_fpga(numDevices * cuNm, sourceLen, sourceFeatures, sourceCoeffs,
                                numResults, g.data(),resultID, similarity);

        for (unsigned int k = 0; k < numResults; k++) 
            result.push_back(Result(resultID[k], similarity[k]));

        free(sourceFeatures);
        free(sourceCoeffs);
        free(similarity);
        free(resultID);

        return result;
    }

}; // class PrivateImpl


// persistent storge for L3::Handle that is shared by L3 functions
class sharedHandlesCosSimDense {
   public:
    std::unordered_map<unsigned int, std::shared_ptr<xf::graph::L3::Handle> > handlesMap;
    static sharedHandlesCosSimDense& instance() {
        static sharedHandlesCosSimDense theInstance;
        return theInstance;
    }
};

void freeSharedHandle()
{
    if (!sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        // free the object stored in handlsMap[0] and erase handlsMap[0]
        std::cout << "INFO: " << __FUNCTION__ << std::endl;
        std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];
        handle0->free();
        sharedHandlesCosSimDense::instance().handlesMap.erase(0);
    }
}

// create a new shared handle if
// 1. The shared handle does not exist or
// 2. The numDevices option changes
// 
// Return values:
// 0: Using exsiting handle
// 1: A new handle is created
int createSharedHandle(uint32_t numDevices)
{
    // return right away if the handle has already been created
    if (!sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

#ifndef NDEBUG
        std::cout << "DEBUG: " << __FUNCTION__ << " numDevices=" << handle0->getNumDevices() << std::endl;
#endif        
        handle0->showHandleInfo();
        if (numDevices != handle0->getNumDevices()) {
            std::cout << "INFO: " << __FUNCTION__ << "numDevices changed. Creating a new handle." 
                      << " numDevices loaded=" << handle0->getNumDevices()
                      << " numDevices requested=" << numDevices << std::endl;
            freeSharedHandle();
        } else {
            std::cout << "INFO: " << __FUNCTION__ << " Using exsiting handle with "
                     << handle0->getNumDevices() << " devices loaded." << std::endl;
            return 0;
        }
    }
    std::cout << "INFO: " << __FUNCTION__ << " Create a new sharedHandle" << std::endl;
    std::shared_ptr<xf::graph::L3::Handle> handleInstance(new xf::graph::L3::Handle);
    sharedHandlesCosSimDense::instance().handlesMap[0] = handleInstance;
    
    return 1;
}


//-----------------------------------------------------------------------------
// Perform the tasks below
// * Load xclbin
// * Allocate CUs
// * Set up kernels
//-----------------------------------------------------------------------------
void PrivateImpl::load_cu_cosinesim_ss_dense_fpga()
{

    std::string opName = "similarityDense";
    //TODO check if requestLoad need to bring up to options for user to set
    int requestLoad = 100;

    //----------------- Setup denseSimilarityKernel thread ---------
    xf::graph::L3::Handle::singleOP op0;
    op0.operationName = (char*)opName.c_str();
    op0.setKernelName(kernelName_);
    op0.setKernelAlias(kernelAlias_);
    op0.requestLoad = requestLoad;
    //op0.xclbinFile = (char*)xclbinPath.c_str();
    op0.xclbinPath = (char *)xclbinPath.c_str();
    op0.numDevices = numDevices;
    op0.cuPerBoard = cuNm;
  
    std::fstream xclbinFS((char*)xclbinPath.c_str(), std::ios::in);

    if (!xclbinFS) {
        std::ostringstream oss;
        oss << "xclbin file doesn't exist. Please ensure that you have set valid xclbinPath" <<std::endl;
        throw xilinx_apps::cosinesim::Exception(oss.str());
        std::cerr << "ERROR: xclbin file doesn't exist." <<std::endl;
        abort();
    }

#ifndef NDEBUG
    std::cout << "DEBUG:" << __FUNCTION__ 
              << "\n    deviceNames=" << deviceNames
              << std::endl;
#endif
    int statusSetUp = 0;
    int statusCreateHandle = createSharedHandle(numDevices);
    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];
    if (statusCreateHandle == 1) {
        // new handle is created. Set it up
        handle0->addOp(op0);
        statusSetUp = handle0->setUp(deviceNames);
    }
    if (statusSetUp != 0) {
        // std::cout<< "ERROR: FPGA is not setup properly. free the handle! status:"<<status<<std::endl;
        freeSharedHandle();
        std::ostringstream oss;
        oss << "FPGA is not setup properly. Try the instructions in the error messages above. " <<std::endl;
        oss << "    If not working, seek help for Xilinx technical support " << std::endl;
        throw xilinx_apps::cosinesim::Exception(oss.str());
        std::cerr << "ERROR: FPGA is not setup properly." <<std::endl;
        abort();

    }
    return;
}

//-----------------------------------------------------------------------------
// Load the graph data onto FPGA
//-----------------------------------------------------------------------------
void PrivateImpl::load_graph_cosinesim_ss_dense_fpga(
    uint32_t deviceNeeded, uint32_t cuNm, xf::graph::Graph<int32_t, int32_t>** graph)
{
    // return right away if the handle has already been created
    if (sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        std::cout << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:"
                << " sharedHandlesCosSimDense.handlesMap.empty="
                << sharedHandlesCosSimDense::instance().handlesMap.empty() << std::endl;

        std::ostringstream oss;
        oss << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:" <<std::endl;
        throw xilinx_apps::cosinesim::Exception(oss.str());
       //std::cerr << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:" <<std::endl;
        abort();
    }

    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

    //---------------- Run Load Graph -----------------------------------
    uint32_t deviceId, cuId;
    for (uint32_t i = 0; i < deviceNeeded * cuNm; ++i) {
        deviceId = handle0->supportedDeviceIds_[i / cuNm];
        cuId = i % cuNm;
        (handle0->opsimdense)->loadGraphMultiCardNonBlocking(deviceId, cuId, graph[i][0]);
    }

    return;
}

//-----------------------------------------------------------------------------
// Execute kernel to compute cosine similarity and return topK values
//-----------------------------------------------------------------------------
void PrivateImpl::cosinesim_ss_dense_fpga(uint32_t devicesNeeded,
                                           int32_t sourceLen,
                                           int32_t* sourceWeight,
                                           int32_t* sourceCoeffs,
                                           int32_t topK,
                                           xf::graph::Graph<int32_t, int32_t>** g,
                                           int32_t* resultID,
                                           float* similarity) 
{
    //---------------- Run Load Graph -----------------------------------
#ifdef __PROFILING__
    std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "LOG2TIMELINE: " << __FUNCTION__ << " time0=" << l_start_time.time_since_epoch().count()
              << std::endl;
#endif

    if (sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        std::cout << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:"
              << " sharedHandlesCosSimDense.handlesMap.empty="
              << sharedHandlesCosSimDense::instance().handlesMap.empty() << std::endl;


        std::ostringstream oss;
        oss << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:" <<std::endl;
        throw xilinx_apps::cosinesim::Exception(oss.str());
        //std::cerr << "ERROR: " << __FUNCTION__ << " CUs need to be set up first:" <<std::endl;
        abort();

    }

    std::shared_ptr<xf::graph::L3::Handle> handle0 =
                        sharedHandlesCosSimDense::instance().handlesMap[0];
    handle0->showHandleInfo();  // no-op in Release
    int32_t requestNm = 1;
    int32_t hwNm = devicesNeeded;
    std::vector<xf::graph::L3::event<int> > eventQueue[requestNm];
    float** similarity0[requestNm];
    int32_t** resultID0[requestNm];
    int counter[requestNm][hwNm];
    for (int m = 0; m < requestNm; ++m) {
        similarity0[m] = new float*[hwNm];
        resultID0[m] = new int32_t*[hwNm];
        for (int i = 0; i < hwNm; ++i) {
            counter[m][i] = 0;
            similarity0[m][i] = xf::graph::internal::aligned_alloc<float>(topK);
            resultID0[m][i] = xf::graph::internal::aligned_alloc<int32_t>(topK);
            memset(resultID0[m][i], 0, topK * sizeof(int32_t));
            memset(similarity0[m][i], 0, topK * sizeof(float));
        }
    }
    //---------------- Run L3 API -----------------------------------
    for (int m = 0; m < requestNm; ++m) {
        eventQueue[m] = cosineSimilaritySSDenseMultiCard(
            handle0, hwNm, sourceLen, sourceWeight, sourceCoeffs, topK, g,
            resultID0[m], similarity0[m]);
    }

    int ret = 0;
    for (int m = 0; m < requestNm; ++m) {
        for (unsigned int i = 0; i < eventQueue[m].size(); ++i) {
            ret += eventQueue[m][i].wait();
        }
    }
    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < topK; ++i) {
            similarity[i] = similarity0[m][0][counter[m][0]];
            int32_t prev = 0;
            resultID[i] = resultID0[m][0][counter[m][0]];
            counter[m][0]++;
            for (int j = 1; j < hwNm; ++j) {
                if (similarity[i] < similarity0[m][j][counter[m][j]]) {
                    similarity[i] = similarity0[m][j][counter[m][j]];
                    resultID[i] = resultID0[m][j][counter[m][j]];
                    counter[m][prev]--;
                    prev = j;
                    counter[m][j]++;
                }
            }
        }
    }

    for (int m = 0; m < requestNm; ++m) {
        for (int i = 0; i < hwNm; ++i) {
            free(similarity0[m][i]);
            free(resultID0[m][i]);
        }
        delete[] similarity0[m];
        delete[] resultID0[m];
    }
#ifdef __PROFILING__
    std::chrono::time_point<std::chrono::high_resolution_clock> l_end_time =
            std::chrono::high_resolution_clock::now();
    std::cout << "LOG2TIMELINE: " << __FUNCTION__
              << " end=" << l_end_time.time_since_epoch().count() << std::endl;
    std::chrono::duration<double> l_durationSec = l_end_time - l_start_time;
    double l_timeMs = l_durationSec.count() * 1e3;
    std::cout << "LOG2TIMELINE: " << __FUNCTION__
              << " runtime= " << std::fixed << std::setprecision(6) << l_timeMs << std::endl;
#endif
}

//-----------------------------------------------------------------------------
// close_fpga
//-----------------------------------------------------------------------------
void close_fpga() 
{
    freeSharedHandle();
    std::cout << __FUNCTION__ << " completed. sharedHandlesCosSimDense.handlesMap.empty=" <<
              sharedHandlesCosSimDense::instance().handlesMap.empty() << std::endl;

}


} //cosinesim
} //xilinx_app


extern "C" {
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(
    const xilinx_apps::cosinesim::Options& options, 
    unsigned valueSize) 
{
#ifndef NDEBUG    
    std::cout << "DEBUG: inside .so xilinx_cosinesim_createImpl" << std::endl;
#endif    
    return new xilinx_apps::cosinesim::PrivateImpl(options,valueSize);
}

void xilinx_cosinesim_destroyImpl( xilinx_apps::cosinesim::ImplBase *pImpl){
    delete pImpl;
}
}
