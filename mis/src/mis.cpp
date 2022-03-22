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
struct aligned_allocator {
    using value_type = T;

    aligned_allocator() {}

    aligned_allocator(const aligned_allocator&) {}

    template <typename U>
    aligned_allocator(const aligned_allocator<U>&) {}

    T* allocate(std::size_t num) {
        void* ptr = nullptr;
#if defined(_WINDOWS)
        ptr = _aligned_malloc(num * sizeof(T), 4096);
        if (ptr == nullptr) {
            std::cout << "Failed to allocate memory" << std::endl;
            exit(EXIT_FAILURE);
        }
#else
        if (posix_memalign(&ptr, 4096, num * sizeof(T))) {
            std::cout << "Failed to allocate memory." << std::endl;
            throw std::bad_alloc();
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

class MisImpl {
   public:
    Options options_;
    MisImpl(const Options& options) : options_(options) {
        mColIdx.resize(MIS_numChannels);
        d_colIdx.resize(MIS_numChannels);
        d_sync.resize(MIS_numChannels);
    }
    // std::string xclbinPath;
    int device_id = 0;

#ifdef OPENCL
    cl::Context ctx;
    cl::Device device;
    cl::Program prg;
    cl::CommandQueue queue;
    cl::Kernel miskernel;
#else
    xrt::device mDevice;
    xrt::kernel mKernel;
    std::vector<xrt::bo> d_sync, d_colIdx;
#endif

    std::vector<uint16_t, aligned_allocator<uint16_t> > mPrior;
    GraphCSR* mOrigGraph;
    std::vector<int, aligned_allocator<int> > mRowPtr;
    std::vector<int*> mColIdx;

    // The intialize process will download FPGA binary to FPGA card
    void startMis(const std::string& xclbinPath, const std::string& deviceNames);
    void setGraph(GraphCSR* graph);
    std::vector<int> executeMIS();

    int getDevice(const std::string& deviceNames);
    size_t count() const;
    // internal only
    // static void init(const GraphCSR<host_buffer_t<int> >*, host_buffer_t<uint16_t>&);
    // static bool verifyMis(GraphCSR<host_buffer_t<int> >*, const host_buffer_t<uint8_t>&);
    // static size_t count(const int, const host_buffer_t<uint8_t>&);
};

int MisImpl::getDevice(const std::string& deviceNames) {
    int status = -1; // initialize to no match device found
    cl_device_id* devices;
    std::vector<cl::Device> devices0 = xcl::get_xil_devices();
    int totalXilinxDevices = devices0.size();

    std::string curDeviceName;
    for (int i = 0; i < totalXilinxDevices; ++i) {
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
void MIS::startMis() {
    pImpl_->startMis(pImpl_->options_.xclbinPath, pImpl_->options_.deviceNames);
}

void MisImpl::startMis(const std::string& xclbinPath, const std::string& deviceNames) {
    if (getDevice(deviceNames) < 0) {
        // std::cout << "ERROR: Unable to find device " << deviceNames << std::endl;
        // return -2;
        std::ostringstream oss;
        oss << "Uable to find device" << deviceNames << "; Please ensure the machine has proper card" << std::endl;
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

    std::string kernelName = "misKernel:{misKernel_0}";
    miskernel = cl::Kernel(prg, "misKernel:{misKernel_0}");

    std::size_t u50_found = devName.find("u50");

    if (u50_found == std::string::npos) {
        std::cout << "Only U50 is supported so far, please check it "
                     "and re-run"
                  << std::endl;
        exit(1);
    }

#else
    try {
        mDevice = xrt::device(device_id);
        auto uuid = mDevice.load_xclbin(xclbinPath);
        std::string kernelName = "misKernel:{misKernel_0}";
        mKernel = xrt::kernel(mDevice, uuid, kernelName);
        for (int i = 0; i < MIS_numChannels; i++) {
            d_colIdx[i] = xrt::bo(mDevice, MIS_hbmSize, mKernel.group_id(i + 2));
            mColIdx[i] = d_colIdx[i].map<int*>();
        }
    } catch (std::bad_alloc& e) {
        std::cout << "Error: CU on the device is busy." << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error: Device is busy." << std::endl;
        exit(EXIT_FAILURE);
    }
#endif
    // return 0;
}

// from xil_mis.hpp
template <typename G>
void init(const GraphCSR* graph, G& prior) {
    constexpr int LargeNum = 65535;
    int n = graph->n;
    // int aveDegree = graph->colIdx.size() / n;
    int aveDegree = graph->colIdxSize / n;
    for (int i = 0; i < n; i++) {
        int degree = graph->rowPtr[i + 1] - graph->rowPtr[i];
        double r = (rand() % LargeNum) / (double)LargeNum;
        prior[i] = (aveDegree / (aveDegree + degree + r) * 8191);
        prior[i] &= 0x03fff;
    }
}

template <typename G>
bool verifyMis(GraphCSR* graph, G& prior) {
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
// from liang's local mis_kernel_xrt.cpp
size_t MIS::count() const {
    return pImpl_->count();
}
size_t MisImpl::count() const {
    size_t num = 0;
    for (int i = 0; i < mOrigGraph->n; i++) {
        int rp = mPrior[i] >> 14;
        if (rp == 1) num++;
    }
    return num;
}

// void MIS::setGraph(GraphCSR<std::vector<int> >* graph) { pImpl_->setGraph(graph);}
void MIS::setGraph(GraphCSR* graph) {
    pImpl_->setGraph(graph);
}
// void MisImpl::setGraph(GraphCSR<std::vector<int> >* graph) {
void MisImpl::setGraph(GraphCSR* graph) {
    mOrigGraph = graph;
    int n = mOrigGraph->n;
    if (n > MIS_vertexLimits) {
        std::cout << "Graph with more than " << MIS_vertexLimits << " vertices is not supported." << std::endl;
        exit(EXIT_FAILURE);
    }

    int nz = mOrigGraph->colIdxSize;

    // process the graph
    std::cout << "Processing the graph with " << n << " vertices and " << nz / 2 << " edges." << std::endl;

    // convert back tp graphCSR
    // std::unique_ptr<GraphCSR>  graphAfter{createFromGraph(graphAdj.get())};
    // auto start = chrono::high_resolution_clock::now();
    // generate h_prior
    mPrior.resize(n);
    init(mOrigGraph, mPrior);

    mRowPtr.resize(n + 1);
    mRowPtr[0] = 0;
    size_t maxSize = 0;
    std::vector<size_t> index(MIS_numChannels, 0);
    for (int r = 0; r < n; r++) {
        int start = mOrigGraph->rowPtr[r], stop = mOrigGraph->rowPtr[r + 1];
        for (int c = start; c < stop; c++) {
            int colId = mOrigGraph->colIdx[c];
            int ch = colId & (MIS_numChannels - 1);
            if (index[ch] == MIS_hbmSize / sizeof(int)) {
                std::cout << "Graph is not supported due to memory limit." << std::endl;
                exit(EXIT_FAILURE);
            }
            mColIdx[ch][index[ch]++] = colId;
        }

        for (int pe = 0; pe < MIS_numChannels; pe++)
            if (index[pe] > maxSize) maxSize = index[pe];
        for (int pe = 0; pe < MIS_numChannels; pe++) {
            int iter = maxSize - index[pe];
            for (int i = 0; i < iter; i++) mColIdx[pe][index[pe]++] = -1;
        }
        mRowPtr[r + 1] = maxSize * MIS_numChannels;
    }

    for (int i = 0; i < MIS_numChannels; i++) d_sync[i] = xrt::bo(d_colIdx[i], maxSize * sizeof(int), 0);
}

std::vector<int> MIS::executeMIS() {
    return pImpl_->executeMIS();
}
std::vector<int> MisImpl::executeMIS() {
    int nargs = 1;
    auto d_rowPtr = xrt::bo(mDevice, (void*)mRowPtr.data(), mRowPtr.size() * sizeof(int), mKernel.group_id(nargs++));
    nargs += MIS_numChannels;
    auto d_mPrior = xrt::bo(mDevice, (void*)mPrior.data(), mPrior.size() * sizeof(uint16_t), mKernel.group_id(nargs++));

    std::vector<xrt::bo> buffers({d_rowPtr, d_mPrior});
    for (auto& bo : buffers) bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    for (auto& bo : d_sync) bo.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    auto run = xrt::run(mKernel);
    nargs = 0;
    run.set_arg(nargs++, mOrigGraph->n);
    run.set_arg(nargs++, d_rowPtr);
    for (xrt::bo& bo : d_sync) run.set_arg(nargs++, bo);
    run.set_arg(nargs++, d_mPrior);
    run.start();
    run.wait();
    d_mPrior.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    std::vector<int> result;
    result.reserve(mPrior.size());
    for (int idx = 0; idx < mPrior.size(); idx++) {
        int rp = mPrior[idx] >> 14;
        if (rp == 1) result.push_back(idx);
    }
    return result;
}

} // namespace mis
} // namespace xilinx_apps

//#####################################################################################################################

//
// Shared Library Entry Points
//

extern "C" {

xilinx_apps::mis::MisImpl* xilinx_mis_createImpl(const xilinx_apps::mis::Options& options) {
    return new xilinx_apps::mis::MisImpl(options);
}

void xilinx_mis_destroyImpl(xilinx_apps::mis::MisImpl* pImpl) {
    delete pImpl;
}

} // extern "C"
