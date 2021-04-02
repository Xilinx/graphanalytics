#include <vector>
#include <cstdint>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include "cosinesim.hpp"
#include "xf_graph_L3.hpp"



namespace xilinx_apps {
namespace cosinesim {
class PrivateImpl : public ImplBase {

public:
    unsigned indexDeviceCuNm, indexSplitNm, indexNumVertices;
    const unsigned splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
    const unsigned channelsPU = 4; // each PU has 4 HBM channels
    const unsigned cuNm = 2;
    const unsigned channelW = 16;

    int valueSize_;
    uint32_t numDevices;
    int populationVectorRowNm;
    int32_t** numVerticesPU ; // vertex numbers in each PU


    int32_t numEdges;
    int vecLength;
    int64_t numVertices;
    std::string xclbinPath;
    int32_t edgeAlign8;
    int subChNm;
    int lastPopulationCnt;
    //std::vector<int> loadOldVectorCnt(channelsPU,0);
    std::vector<int> loadPopulationCnt;

    //xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
    //currently only support int32_t graph
    std::vector<xf::graph::Graph<int32_t, int32_t>*> g;
    //xf::graph::Graph<int32_t, int32_t>** g;

    // initial value is 0; means no errors;
    //ErrorCode errorCode_;

    //options
    //Options options_;

    int load_xgraph_fpga(uint32_t numVertices, uint32_t numEdges, xf::graph::Graph<uint32_t, float> g);
    void load_cu_cosinesim_ss_dense_fpga();
    void load_graph_cosinesim_ss_dense_fpga(uint32_t deviceNeeded, uint32_t cuNm, xf::graph::Graph<int32_t, int32_t>** graph);


    void cosinesim_ss_dense_fpga(uint32_t devicesNeeded,
                                 int32_t sourceLen,
                                 int32_t* sourceWeight,
                                 int32_t topK,
                                 xf::graph::Graph<int32_t, int32_t>** g,
                                 int32_t* resultID,
                                 float* similarity);

    //PrivateImpl(CosineSimBase* ptr, unsigned valueSize){
    PrivateImpl(const Options &options, unsigned valueSize){
        //errorCode_ = NoError;
        valueSize_ = valueSize;
       if(valueSize_ != 4) {
            std::cout << "DEBUG: valueType is not supported" << std::endl;
            std::ostringstream oss;
            oss << "the only Value size currently supported is 32 bits.  Please ensure that you have constructed the CosineSim object with a 32-bit template parameter" <<std::endl;
            throw xilinx_apps::cosinesim::Exception(oss.str());
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
        if(options.numDevices > 0)
            numDevices = options.numDevices;

        xclbinPath = "/opt/xilinx/apps/graphanalytics/cosinesim/xclbin/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin";
//        if (options.xclbinPathCStr != nullptr)
//            xclbinPath = options.xclbinPathCStr;
//        else if (!options.xclbinPath.empty()){
//           xclbinPath = options.xclbinPath;
//        }
        std::cout << "INFO: xclbinPath set to " <<xclbinPath<<std::endl;

        numEdges = vecLength;

        //options_ = options;
        edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

        numVerticesPU  = new int32_t*[numDevices * cuNm];
        for (unsigned i = 0; i < numDevices * cuNm; ++i) {
            numVerticesPU[i] = new int32_t[splitNm];
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


        //vecLength = options_.vecLength;
        //numEdges = options_.vecLength;
        //numVertices = cosinesimPtr->getOptions().numVertices;
        this->numVertices = numVertices;
        //devicesNeeded = options_.numDevices;

        //the following calculation and assignment is based on numVertices
        int general = ((numVertices - (numDevices * cuNm * splitNm * channelsPU + 1)) /
                (numDevices * cuNm * splitNm * channelsPU)) * channelsPU;
        int rest = numVertices - general * (numDevices * cuNm * splitNm - 1);

#ifdef __DEBUG__
        std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__
                << " numVertices=" << numVertices << ", general=" << general
                << ", rest=" << rest
                << ", split=" << numDevices * cuNm * splitNm 
                << ", numDevices=" << numDevices << std::endl;
#endif

        if (rest < 0) {
//            errorCode_= ErrorGraphPartition;
//TODO assert
            return;
        }

        for (unsigned i = 0; i < numDevices * cuNm; ++i) {
            for (unsigned j = 0; j < splitNm; ++j) {
                numVerticesPU[i][j] = general;
            }
        }
        numVerticesPU[numDevices * cuNm - 1][splitNm - 1] = rest;
        //initialize graph properties
        int fpgaNodeNm = 0;
        for(unsigned i=0;i<numDevices*cuNm; i++){
            //TODO change to Value
            //g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);

            if(valueSize_ == 4) {
                g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges,  numVerticesPU[i]);
                //} else if(valueSize_ == 8) {
                //	g[i] = new xf::graph::Graph<int32_t, int64_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);
            } else {
                //errorCode_ =  ErrorUnsupportedValueType;
                // TODO: must exit at this point, or else crash on next line.  Throw exception here?
            }
            g[i]->numEdgesPU = new int32_t[splitNm];
            g[i]->numVerticesPU = new int32_t[splitNm];
            g[i]->edgeNum = numEdges;
            g[i]->nodeNum = numVertices;
            g[i]->splitNum = splitNm;
            g[i]->refID = fpgaNodeNm;
            for (unsigned j = 0; j < splitNm; ++j) {
                fpgaNodeNm += numVerticesPU[i][j];
                int depth = ((numVerticesPU[i][j] + channelsPU - 1) / channelsPU) * edgeAlign8;
                g[i]->numVerticesPU[j] = numVerticesPU[i][j];
                g[i]->numEdgesPU[j] = depth;

            }
        }



    }
    //void loadgraph_cosinesim_ss_dense_fpga(unsigned deviceNeeded,unsigned cuNm, xf::graph::Graph<int32_t, int32_t>** g)

    virtual void *getPopulationVectorBuffer(RowIndex &rowIndex){

        void * pbuf;
        if(indexDeviceCuNm == numDevices * cuNm)
            return nullptr;
        subChNm = (numVerticesPU[indexDeviceCuNm][indexSplitNm] + channelsPU - 1) / channelsPU;

        pbuf = &(g[indexDeviceCuNm]->weightsDense[indexSplitNm * channelsPU + indexNumVertices / subChNm][loadPopulationCnt[indexNumVertices / subChNm] * edgeAlign8]);

        loadPopulationCnt[indexNumVertices / subChNm] += 1;
        lastPopulationCnt = loadPopulationCnt[indexNumVertices / subChNm];
        indexNumVertices++;
        if(indexNumVertices == unsigned(numVerticesPU[indexDeviceCuNm][indexSplitNm])) {
            indexSplitNm++;
            indexNumVertices=0;
            std::fill(loadPopulationCnt.begin(),loadPopulationCnt.end(),0);
            if(indexSplitNm == splitNm) {
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
    virtual void finishLoadPopulationVectors() {

        //for the last PU, the last channels may need to padded as the row will be the multiple of channelsPU
        const unsigned numCus = numDevices * cuNm;
        int paddingDepth = ((numVerticesPU[numCus - 1][splitNm - 1] + channelsPU - 1) / channelsPU) * channelsPU
                - numVerticesPU[numCus - 1][splitNm - 1];
        int startPaddingDepth = numVerticesPU[numCus - 1][splitNm - 1] * edgeAlign8;
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


    int cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
                                         int32_t deviceNm,
                                         int32_t sourceNUM,
                                         int32_t* sourceWeights,
                                         int32_t topK,
                                         xf::graph::Graph<int32_t, int32_t>** g,
                                         int32_t* resultID,
                                         float* similarity) {
        std::vector<xf::graph::L3::event<int> > eventQueue;
        float** similarity0 = new float*[deviceNm];
        int32_t** resultID0 = new int32_t*[deviceNm];
        int counter[deviceNm];
        for (int i = 0; i < deviceNm; ++i) {
            counter[i] = 0;
            similarity0[i] = xf::graph::internal::aligned_alloc<float>(topK);
            resultID0[i] = xf::graph::internal::aligned_alloc<int32_t>(topK);
            memset(resultID0[i], 0, topK * sizeof(int32_t));
            memset(similarity0[i], 0, topK * sizeof(float));
        }

        for (int i = 0; i < deviceNm; ++i) {
            eventQueue.push_back((handle->opsimdense)->addworkInt(
                1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID0[i], similarity0[i]));
        }
        int ret = 0;
        for (unsigned int i = 0; i < eventQueue.size(); ++i) {
            ret += eventQueue[i].wait();
        }
        for (int i = 0; i < topK; ++i) {
            similarity[i] = similarity0[0][counter[0]];
            int32_t prev = 0;
            resultID[i] = resultID0[0][counter[0]];
            counter[0]++;
            for (int j = 1; j < deviceNm; ++j) {
                if (similarity[i] < similarity0[j][counter[j]]) {
                    similarity[i] = similarity0[j][counter[j]];
                    resultID[i] = resultID0[j][counter[j]];
                    counter[prev]--;
                    prev = j;
                    counter[j]++;
                }
            }
        }
        for (int i = 0; i < deviceNm; ++i) {
            free(similarity0[i]);
            free(resultID0[i]);
        }
        delete[] similarity0;
        delete[] resultID0;
        return ret;
    };

    std::vector<xf::graph::L3::event<int> > cosineSimilaritySSDenseMultiCard(std::shared_ptr<xf::graph::L3::Handle>& handle,
                                                              int32_t deviceNm,
                                                              int32_t sourceNUM,
                                                              int32_t* sourceWeights,
                                                              int32_t topK,
                                                              xf::graph::Graph<int32_t, int32_t>** g,
                                                              int32_t** resultID,
                                                              float** similarity) {
        std::vector<xf::graph::L3::event<int> > eventQueue;
        std::chrono::time_point<std::chrono::high_resolution_clock> l_start_time;

        for (int i = 0; i < deviceNm; ++i) {
            l_start_time = std::chrono::high_resolution_clock::now();
            std::cout << "LOG2TIMELINE: " << __FUNCTION__
                      << "::addworkInt" << i << " start=" << l_start_time.time_since_epoch().count()
                      << std::endl;

            eventQueue.push_back(
                (handle->opsimdense)
                    ->addworkInt(1, 0, sourceNUM, sourceWeights, topK, g[i][0], resultID[i], similarity[i]));
        }
        return eventQueue;
    };

    virtual std::vector<Result> matchTargetVector(unsigned numResults, void *elements){

        std::vector<Result> result;
        //---------------- Generate Source Indice and Weight Array -------
        int sourceLen = edgeAlign8; // sourceIndice array length
        int32_t* sourceWeight =
                xf::graph::internal::aligned_alloc<int32_t>(sourceLen); // weights of source vertex's out members
                //int32_t newVecLen = newVector.size() - 3;

                for (int i = 0; i < sourceLen; i++) {
                    if (i < vecLength) {
                        //sourceWeight[i] = elements[i + 3];

                        try {
                            if(valueSize_ == 4)
                                sourceWeight[i] = (reinterpret_cast<int32_t*>(elements))[i];
                            //else if (valueSize_== 8)
                            //	sourceWeight[i] = (reinterpret_cast<int64_t*>(elements))[i];
                            else
                              throw 1;

                    }
                    catch (...) {

                        //std::cout << CosineSim::ValueType << " is not supported for now, Currently supports Value Type int32_t" <<std:endl;
                        // errorCode_ =  ErrorUnsupportedValueType;
                    }

                    } else {
                        sourceWeight[i] = 0;
                    }
                }
                float* similarity = xf::graph::internal::aligned_alloc<float>(numResults);
                int32_t* resultID = xf::graph::internal::aligned_alloc<int32_t>(numResults);
                std::memset(resultID, 0, numResults * sizeof(int32_t));
                std::memset(similarity, 0, numResults * sizeof(float));

                //---------------- Run L3 API -----------------------------------

                cosinesim_ss_dense_fpga(numDevices * cuNm, sourceLen, sourceWeight, numResults, g.data(),resultID, similarity);

                for (unsigned int k = 0; k < numResults; k++) {
                    //result += testResults(VERTEX(xai::IDMap[resultID[k]]), similarity[k]);
                    result.push_back(Result(resultID[k], similarity[k]));
                }

                delete[] sourceWeight;
                delete[] similarity;
                delete[] resultID;
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

void free_shared_handle()
{
    if (!sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        // free the object stored in handlsMap[0] and erase handlsMap[0]
        std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];
        handle0->free();
        sharedHandlesCosSimDense::instance().handlesMap.erase(0);
    }
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
    std::string kernelName = "denseSimilarityKernel";
    //TODO check if requestLoad need to bring up to options for user to set
    int requestLoad = 100;

  //----------------- Setup denseSimilarityKernel thread ---------
  xf::graph::L3::Handle::singleOP op0;
  op0.operationName = (char*)opName.c_str();
  op0.setKernelName((char*)kernelName.c_str());
  op0.requestLoad = requestLoad;
  //op0.xclbinFile = (char*)xclbinPath.c_str();
  op0.xclbinFile = (char*)xclbinPath.c_str();
  op0.deviceNeeded = numDevices;
  op0.cuPerBoard = cuNm;

  std::fstream xclbinFS(xclbinPath, std::ios::in);
  try {
      if (!xclbinFS) {
          //std::cout << "Error : xclbinFile doesn't exist: " << options_.xclbinPath << std::endl;
          //errorCode_ = ErrorXclbinNotExist;
          //return;
          throw 2;
      }
  }
  catch(...) {
      std::cout << "Error : xclbinFile doesn't exist: " << xclbinPath << std::endl;
      return;
  }

    // return right away if the handle has already been created
    if (!sharedHandlesCosSimDense::instance().handlesMap.empty()) {
        std::cout << "INFO: " << __FUNCTION__ << " skipped:"
              << " sharedHandlesCosSimDense.handlesMap.empty="
              << sharedHandlesCosSimDense::instance().handlesMap.empty() << std::endl;

        return;
        //return XF_GRAPH_L3_SUCCESS;
    }
    std::shared_ptr<xf::graph::L3::Handle> handleInstance(new xf::graph::L3::Handle);
    sharedHandlesCosSimDense::instance().handlesMap[0] = handleInstance;
    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

    handle0->addOp(op0);
    int status = handle0->setUp();
    if (status != 0) {
        std::cout<< "ERROR: FPGA is not setup properly. free the handle! status:"<<status<<std::endl;
        free_shared_handle();

    }
    return;
    //TODO throw exceptions
    //return status;
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

        //return XF_GRAPH_L3_ERROR_CU_NOT_SETUP;
        //TODO throw exceptions
        return;
    }

    std::shared_ptr<xf::graph::L3::Handle> handle0 = sharedHandlesCosSimDense::instance().handlesMap[0];

    //---------------- Run Load Graph -----------------------------------
    for (uint32_t i = 0; i < deviceNeeded * cuNm; ++i) {
        (handle0->opsimdense)->loadGraphMultiCardNonBlocking(i / cuNm, i % cuNm, graph[i][0]);
    }

    //return XF_GRAPH_L3_SUCCESS;
    return;
}

//-----------------------------------------------------------------------------
// Execute kernel to compute cosine similarity and return topK values
//-----------------------------------------------------------------------------
void PrivateImpl::cosinesim_ss_dense_fpga(uint32_t devicesNeeded,
                                           int32_t sourceLen,
                                           int32_t* sourceWeight,
                                           int32_t topK,
                                           xf::graph::Graph<int32_t, int32_t>** g,
                                           int32_t* resultID,
                                           float* similarity) {
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

        //return XF_GRAPH_L3_ERROR_CU_NOT_SETUP;
        //TODO throw exceptions
        return;
    }

    std::shared_ptr<xf::graph::L3::Handle> handle0 =
                        sharedHandlesCosSimDense::instance().handlesMap[0];
    handle0->debug();
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
            handle0, hwNm, sourceLen, sourceWeight, topK, g,
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
void close_fpga() {
    free_shared_handle();
    std::cout << __FUNCTION__ << " completed. sharedHandlesCosSimDense.handlesMap.empty=" <<
              sharedHandlesCosSimDense::instance().handlesMap.empty() << std::endl;

}


} //cosinesim
} //xilinx_app


extern "C" {
xilinx_apps::cosinesim::ImplBase *xilinx_cosinesim_createImpl(const xilinx_apps::cosinesim::Options& options, unsigned valueSize){
    std::cout << "DEBUG: inside .so xilinx_cosinesim_createImpl" << std::endl;
    return new xilinx_apps::cosinesim::PrivateImpl(options,valueSize);
}

void xilinx_cosinesim_destroyImpl( xilinx_apps::cosinesim::ImplBase *pImpl){
    delete pImpl;
}
}
