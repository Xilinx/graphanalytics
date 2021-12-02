/*
 * Copyright 2019-2021 Xilinx, Inc.
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
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_bo.h>
#include "xilinx_runtime_common.hpp"
#include "xilinxmis.hpp"
#include "utils.hpp"

namespace xilinx_apps {
namespace mis {

    template <typename T>
    class GraphCSR;

    const int MIS_PEs=16;
    
    template <typename T>
    struct aligned_allocator {
        using value_type = T;

        aligned_allocator() {}

        aligned_allocator(const aligned_allocator&) {}

        template <typename U>
        aligned_allocator(const aligned_allocator<U>&) {}

        T* allocate(std::size_t num) {
            void* ptr = nullptr;

    #if defined(_WINDOWS)
            {
                ptr = _aligned_malloc(num * sizeof(T), 4096);
                if (ptr == nullptr) {
                    std::cout << "Failed to allocate memory" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
    #else
            {
                if (posix_memalign(&ptr, 4096, num * sizeof(T))) throw std::bad_alloc();
            }
    #endif
            return reinterpret_cast<T*>(ptr);
        }
        void deallocate(T* p, std::size_t num) {
    #if defined(_WINDOWS)
            _aligned_free(p);
    #else
            free(p);
    #endif
        }
    };

    
    class Graph {
    public:
        //template <typename T>
        //static Graph createFromGraphCSR(GraphCSR<T>* graph);
        //the number of vertex
        uint32_t n;
        //converted adjList Graph
        std::vector<std::vector<uint32_t> > adjList;
        Graph(uint32_t n) {
            this->n = n;
            adjList.resize(n);
        }

        Graph(const std::vector<std::vector<uint32_t> >& adjList) {
            this->n = adjList.size();
            this->adjList = adjList;
        }
        Graph(std::vector<std::vector<uint32_t> >&& adjList) : adjList(adjList) { this->n = this->adjList.size(); }
    };

    class MisImpl {
    public:
        Options options_;
        MisImpl(const Options &options) : options_(options)
        { 

        }
        //std::string xclbinPath;
        int device_id=0;
        
        #ifdef OPENCL
            cl::Context ctx;
            cl::Device device;
            cl::Program prg;
            cl::CommandQueue queue;
            cl::Kernel miskernel;
        #else
            xrt::device m_Device;
            xrt::kernel m_Kernel;
        #endif


        std::vector<uint16_t,aligned_allocator<uint16_t>> mPrior;
        std::unique_ptr<GraphCSR<uint32_t>> mGraph;
        GraphCSR<uint32_t>* mGraphOrig;

        // The intialize process will download FPGA binary to FPGA card
        void startMis(const std::string& xclbinPath,const std::string& deviceNames);
        void setGraph(GraphCSR<uint32_t>* graph);
        std::vector<uint16_t> executeMIS();

        int getDevice(const std::string& deviceNames);
        size_t count() const;
        //internal only
        //static void init(const GraphCSR<host_buffer_t<uint32_t> >*, host_buffer_t<uint16_t>&);
        //static bool verifyMis(GraphCSR<host_buffer_t<uint32_t> >*, const host_buffer_t<uint8_t>&);
        //static size_t count(const int, const host_buffer_t<uint8_t>&);


    };

    int MisImpl::getDevice(const std::string& deviceNames)
    {
        int status = -1;  // initialize to no match device found
        cl_device_id* devices;
        std::vector<cl::Device> devices0 = xcl::get_xil_devices();
        uint32_t totalXilinxDevices = devices0.size();
    
        std::string curDeviceName;        
        for (uint32_t i = 0; i < totalXilinxDevices; ++i) {
            curDeviceName = devices0[i].getInfo<CL_DEVICE_NAME>();
    
            if (deviceNames == curDeviceName) {
                std::cout << "INFO: Found requested device: " << curDeviceName << " ID=" << i << std::endl;
                // save the matching device
                #ifdef OPENCL
                    device = devices0[i];
                #else
                     device_id = i;
                #endif
                status = 0; // found a matching device
                break;
            } else {
                std::cout << "INFO: Skipped non-requested device: " << curDeviceName << " ID=" << i << std::endl;
            }
        }
    
        return status;
    }
    void MIS::startMis() { pImpl_->startMis(pImpl_->options_.xclbinPath, pImpl_->options_.deviceNames); }

    void MisImpl::startMis(const std::string& xclbinPath,const std::string& deviceNames){
        if (getDevice(deviceNames) < 0) {
            //std::cout << "ERROR: Unable to find device " << deviceNames << std::endl;
            //return -2;
            std::ostringstream oss;
            oss << "Uable to find device" << deviceNames << "; Please ensure the machine has proper card" <<std::endl;
            throw xilinx_apps::mis::Exception(oss.str());
            std::cerr << "ERROR: Unable to find device " << deviceNames << std::endl;
            abort();
        } else 
            std::cout << "INFO: Start MIS on " << deviceNames << std::endl;
    
    #ifdef OPENCL
        std::vector<cl::Device> devices;
        // Creating Context and Command Queue for selected device from getDevice
        ctx = cl::Context(device);
        queue = cl::CommandQueue(ctx, device, CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
        std::string devName = device.getInfo<CL_DEVICE_NAME>();
        std::cout << "INFO: found device=" << devName << std::endl; 
    
        // Create program with given xclbin file
        cl::Program::Binaries xclBins = xcl::import_binary_file(xclbinPath);
        devices.resize(1);
        devices[0] = device;
        prg = cl::Program(ctx, devices, xclBins);
    
        std::string kernelName = "findMIS:{findMIS_0}";
        miskernel = cl::Kernel(prg, "findMIS:{findMIS_0}");
    
        std::size_t u50_found = devName.find("u50");

        if (u50_found == std::string::npos) 
        {
            std::cout << "Only U50 is supported so far, please check it "
                            "and re-run"
                        << std::endl;
            exit(1);
        }
    
    #else
        m_Device = xrt::device(device_id);
        auto uuid = m_Device.load_xclbin(xclbinPath);
        std::string kernelName = "findMIS:{findMIS_0}";
        m_Kernel = xrt::kernel(m_Device, uuid, kernelName);
    #endif
        //return 0;
    }

//from xil_mis.hpp 
    template <typename T, typename G>
    void init(const GraphCSR<T>* graph, G& prior) {
        constexpr int LargeNum = 65535;
        int n = graph->n;
        //int aveDegree = graph->colIdx.size() / n;
        int aveDegree = graph->colIdxSize / n;
        for (int i = 0; i < n; i++) {
            int degree = graph->rowPtr[i + 1] - graph->rowPtr[i];
            double r = (rand() % LargeNum) / (double)LargeNum;
            prior[i] = (aveDegree / (aveDegree + degree + r) * 8191);
            prior[i] &= 0x03fff;
        }
        prior[n] = 0x03 << 14;
    }

    template <typename T, typename G>
    bool verifyMis(GraphCSR<T>* graph, G& prior) {
        int n = graph->n;
        for (int i = 0; i < n; i++) {
            int rp = prior[i] >> 14;
            if (rp == 3) {
                bool oppo = false;
                for (int j = graph->rowPtr[i]; j < graph->rowPtr[i + 1]; j++) {
                    int v = graph->colIdx[j];
                    if (v == i || v >= n) continue;
                    int vp = prior[v] >> 14;
                    oppo = oppo || (vp == 1);
                }
                if (!oppo) return false;
            } else if (rp == 1)
                for (int j = graph->rowPtr[i]; j < graph->rowPtr[i + 1]; j++) {
                    int v = graph->colIdx[j];
                    if (v == i || v >= n) continue;
                    int vp = prior[v] >> 14;
                    if (vp == 1) {
                        std::cout << "Conflicted vertices in the set with id " << i << '\t' << v << std::endl;
                        return false;
                    }
                }
            else {
                std::cout << "Undetermined vertex with id= " << i << std::endl;
                return false;
            }
        }
        return true;
    }
    //from liang's local mis_kernel_xrt.cpp
    size_t MIS::count() const { return pImpl_->count(); }
    size_t MisImpl::count() const {
        size_t num = 0;
        for (int i = 0; i < mGraphOrig->n; i++) {
            int rp = mPrior[i] >> 14;
            if (rp == 1) num++;
        }
        return num;
    }


    template <typename T>
    Graph* createFromGraphCSR(GraphCSR<T>* graph) {
        int n = graph->n;
        std::vector<std::vector<uint32_t> > adjList;
        adjList.reserve(n);
        for (int i = 0; i < graph->n; i++) {
            int start = graph->rowPtr[i], stop = graph->rowPtr[i + 1];
            //adjList.emplace_back(std::vector<uint32_t>(graph->colIdx.begin() + start, graph->colIdx.begin() + stop));
            std::vector<uint32_t> tmp;
            for(int i=start;i< stop; i++){
                tmp.push_back(graph->colIdx[i]);
            }
            adjList.emplace_back(tmp);
        }
        return new Graph(adjList);
    }

    template <typename T>
    GraphCSR<T>* createFromGraph(Graph* graph) {
        std::vector<T> rowPtr(graph->n + 1, 0);
        std::vector<T> colIdx;
        for (int i = 0; i < graph->n; i++) {
            rowPtr[i + 1] = rowPtr[i] + graph->adjList[i].size();
            colIdx.insert(colIdx.end(), graph->adjList[i].begin(), graph->adjList[i].end());
        }
        return new GraphCSR<T>(rowPtr, colIdx);
    }


    //void MIS::setGraph(GraphCSR<std::vector<uint32_t> >* graph) { pImpl_->setGraph(graph);}
    void MIS::setGraph(GraphCSR<uint32_t>* graph) { pImpl_->setGraph(graph);}
    //void MisImpl::setGraph(GraphCSR<std::vector<uint32_t> >* graph) {
    void MisImpl::setGraph(GraphCSR<uint32_t>* graph) {
        mGraphOrig = graph;
        int n=mGraphOrig->n;
        int nz=mGraphOrig->colIdxSize;
        //not sure if needed
        /*
        mStatus.resize((mGraphOrig->n + 4) / 4);
        std::fill(mStatus.begin(), mStatus.end(), 0);
        */
        //process the graph
        auto bw = mGraphOrig->bandwidth();
        std::cout << "Processing the graph with " << n << " vertices, " << nz / 2 << " edges and bandwidth " << bw << std::endl;

        //convert it to adjacent list graph
        //Graph graphAdj = createFromGraphCSR(mGraphOrig);
        std::unique_ptr<Graph> graphAdj{createFromGraphCSR(mGraphOrig)};
        //padding 
        for (int r = 0; r < graphAdj->n; r++) {
            std::vector<uint32_t>& row = graphAdj->adjList[r];
            int size = row.size();
            int rem = size % MIS_PEs;
            if (rem == 0) continue;
            graphAdj->adjList[r].reserve(size + MIS_PEs - rem);
            for (int i = 0; i < MIS_PEs - rem; i++) graphAdj->adjList[r].push_back(graphAdj->n);
        }
        //convert back tp graphCSR
        //std::unique_ptr<GraphCSR>  graphAfter{createFromGraph(graphAdj.get())};
        //auto start = chrono::high_resolution_clock::now();
        //generate h_prior
        mPrior.resize(n+1);
        init(mGraphOrig, mPrior);

        //assign back
        //mGraph.reset(createFromGraph(graphAdj.get()));
        //mGraph.reset(graphAfter.get());
        //mGraph.reset(createFromGraph<std::vector<uint32_t>>(graphAdj.get()));
        mGraph.reset(createFromGraph<uint32_t>(graphAdj.get()));
    }

    std::vector<uint16_t> MIS::executeMIS(){ return pImpl_->executeMIS();}
    std::vector<uint16_t> MisImpl::executeMIS() {
        
        timeStamp("Timer initialization");
    #ifdef OPENCL
        std::vector<cl::Event> events_write(2);
        std::vector<cl::Event> events_kernel(1);
        std::vector<cl::Event> events_read(1);
        cl::Buffer d_rowPtr = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_READ_ONLY, mGraph->rowPtr.size() * sizeof(uint32_t), NULL,NULL);
        cl::Buffer d_colIdx = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_READ_ONLY, mGraph->colIdx.size() * sizeof(uint32_t), NULL,NULL);
        cl::Buffer d_prior = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_WRITE_ONLY, mPrior.size() * sizeof(uint16_t), NULL,NULL);
    #else
    /*
        auto d_rowPtr = xrt::bo(m_Device, (void*)mGraph->rowPtr.data(), mGraph->rowPtr.size() * sizeof(uint32_t),
                                m_Kernel.group_id(1));
        auto d_colIdx = xrt::bo(m_Device, (void*)mGraph->colIdx.data(), mGraph->colIdx.size() * sizeof(uint32_t),
                                m_Kernel.group_id(2));
        auto d_prior = xrt::bo(m_Device, (void*)mPrior.data(), mPrior.size() * sizeof(uint16_t), m_Kernel.group_id(3));*/
        auto d_rowPtr = xrt::bo(m_Device, (void*)mGraph->rowPtr, mGraph->rowPtrSize * sizeof(uint32_t),
                                m_Kernel.group_id(1));
        auto d_colIdx = xrt::bo(m_Device, (void*)mGraph->colIdx, mGraph->colIdxSize * sizeof(uint32_t),
                                m_Kernel.group_id(2));
        auto d_prior = xrt::bo(m_Device, (void*)mPrior.data(), mPrior.size() * sizeof(uint16_t), m_Kernel.group_id(3));
    #endif
        timeStamp("Buffer created");
    
    #ifdef OPENCL
        queue.enqueueWriteBuffer(d_rowPtr, CL_FALSE, 0, mGraph->rowPtr.size() * sizeof(uint32_t), (void*)mGraph->rowPtr.data(), nullptr,
                                                &events_write[0]);
        queue.enqueueWriteBuffer(d_colIdx, CL_FALSE, 0, mGraph->colIdx.size() * sizeof(uint32_t), (void*)mGraph->colIdx.data(), nullptr,
                                                &events_write[1]);
    #else
        std::vector<xrt::bo> buffers({d_rowPtr, d_colIdx, d_prior});
        for(auto & bo: buffers)
            bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    #endif
        timeStamp("Sync buffer objects");

    #ifdef OPENCL
        int j = 0;
        miskernel.setArg(j++, mGraph->n);
        miskernel.setArg(j++, d_rowPtr);
        miskernel.setArg(j++, d_colIdx);
        miskernel.setArg(j++, d_prior);
        //std::vector<cl::Event> waitEnqueueEvents0{events_write};
        queue.enqueueTask(miskernel, &events_write, &(events_kernel[0]));
    #else
        auto run = m_Kernel(mGraph->n, d_rowPtr, d_colIdx, d_prior);
        run.wait();
    #endif

        timeStamp("Execution finished");

    #ifdef OPENCL
        std::vector<cl::Event> waitEnqueueReadEvents0{events_kernel};
        queue.enqueueReadBuffer(d_prior, CL_FALSE, 0, mPrior.size() * sizeof(uint16_t), (void*)mPrior.data(), &waitEnqueueReadEvents0, &(events_read[0]));
    #else
        d_prior.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    #endif
        timeStamp("Read results");

        std::vector<uint16_t> result(mPrior.begin(), mPrior.end());
        return result;
    }

//from graph.hpp
    template <typename T>
    uint32_t GraphCSR<T>::bandwidth() {
        int maxB = 0;
        for (int i = 0; i < n; i++) {
            for (int j = rowPtr[i]; j < rowPtr[i + 1]; i++) {
                int b = std::abs((long int)(colIdx[j] - i));
                if (b > maxB) maxB = b;
            }
        }
        return maxB;
    }
 

} // namespace mis
} // namespace xilinx_apps

//#####################################################################################################################

//
// Shared Library Entry Points
//

extern "C" {

xilinx_apps::mis::MisImpl *xilinx_mis_createImpl(const xilinx_apps::mis::Options& options) {
    return new xilinx_apps::mis::MisImpl(options);
}

void xilinx_mis_destroyImpl(xilinx_apps::mis::MisImpl *pImpl) {
    delete pImpl;
}

}  // extern "C"