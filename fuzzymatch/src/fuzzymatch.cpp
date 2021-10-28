/**
* Copyright (C) 2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include <cassert>
#include <iostream>
#include <sstream>
#include <limits>

#include "xilinx_runtime_common.hpp"
#include "fuzzymatch.hpp"

namespace xilinx_apps {
namespace fuzzymatch {
    //extern const int max_validated_pattern;
    //extern const size_t max_len_in_char;
    //extern unsigned int totalThreadNum;
    //extern const size_t max_pattern_len_in_char; // in char

    class FuzzyMatchImpl {
    public:
        Options options_;  // copy of options passed to FuzzyMatch constructor
        int max_fuzzy_len=35;
        FuzzyMatchImpl(const Options &options) : options_(options) 
        {
            
        }
        
        std::string xclbinPath;
        static const int PU_NUM = 8;
        int boost;
        cl::Context ctx;
        cl::Device device;
        cl::Program prg;
        cl::CommandQueue queue;
        cl::Kernel fuzzy[4];
        int buf_f_i_idx=0;
    
        int sum_line;
        std::vector<std::vector<std::string> > vec_pattern_grp =
            std::vector<std::vector<std::string> >(max_len_in_char);
        //std::vector<int> vec_base = std::vector<std::vector<int> >;
        //std::vector<int> vec_offset = std::vector<std::vector<int> >;
        std::vector<int> vec_base ;
        std::vector<int> vec_offset ;
    
        cl::Buffer buf_field_i1;
        cl::Buffer buf_field_i2;
        cl::Buffer buf_csv[2 * PU_NUM];
        cl::Buffer buf_field_o1;
        cl::Buffer buf_field_o2;
    
        std::vector<cl::Event> events_write;
        std::vector<cl::Event> events_kernel;
        std::vector<cl::Event> events_read;
    
        int getDevice(const std::string& deviceNames);
        int startFuzzyMatch(const std::string& xclbinPath, const std::string& deviceNames);
        int fuzzyMatchLoadVec(std::vector<std::string>& patternVec);
    
        // The check method returns whether the transaction is okay, and triggering condition if any.
        bool executefuzzyMatch(const std::string& t); 
        void preCalculateOffsetPerPU(std::vector<std::vector<std::string> >& vec_grp_str,
                        std::vector<int>& vec_base,
                        std::vector<int>& vec_offset);
    
    };
       
    template <typename T>
    T* aligned_alloc(std::size_t num) {
        void* ptr = nullptr;
        #if _WIN32
            ptr = (T*)malloc(num * sizeof(T));
            if (num == 0) {
        #else
            if (posix_memalign(&ptr, 4096, num * sizeof(T))) {
        #endif
            throw std::bad_alloc();
            }
        return reinterpret_cast<T*>(ptr);
    }
    int getRange(const std::string& input, std::vector<int> &vec_base, std::vector<int> &vec_offset, int &base, int &nrow)
    {
        int cnt = 0;
        int len = input.length();
        if (len == 0)
        {
            return -1;
        }
        int med = getMaxDistance(len);
    
        base = vec_base[len - med] * 3; // as 128-bit data width in AXI
        for (int i = len - med; i <= len + med; i++)
        {
            cnt += vec_offset[i];
        }
    
        nrow = cnt * 3;
    
        return 0;
    }
    
    void FuzzyMatchImpl::preCalculateOffsetPerPU(std::vector<std::vector<std::string>> &vec_grp_str,
                            std::vector<int> &vec_base,
                            std::vector<int> &vec_offset)
    {
        // the input table already grouped by length
        // scan the string vec for each length, compute:
        // vec_base : base address for each length in each PU
        // vec_offset : each PU get the vec size
        this->sum_line = 0;
        for (size_t i = 0; i <= this->max_fuzzy_len; i++)
        {
            vec_grp_str[i] = this->vec_pattern_grp[i];
            int size = this->vec_pattern_grp[i].size();
            int delta = (size + this->PU_NUM - 1) / this->PU_NUM;
    
            vec_base[i] = this->sum_line;
            vec_offset[i] = delta;
            this->sum_line += delta;
        }
    
    
    }
    
    int FuzzyMatchImpl::getDevice(const std::string& deviceNames)
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
                device = devices0[i];
                status = 0; // found a matching device
                break;
            } else {
                std::cout << "INFO: Skipped non-requested device: " << curDeviceName << " ID=" << i << std::endl;
            }
        }
    
        return status;
    }
    
    int FuzzyMatch::startFuzzyMatch(){ return pImpl_->startFuzzyMatch(pImpl_->options_.xclbinPath,pImpl_->options_.deviceNames); }
    int FuzzyMatchImpl::startFuzzyMatch(const std::string &xclbinPath, const std::string& deviceNames)
    {
        if (getDevice(deviceNames) < 0) {
            std::cout << "ERROR: Unable to find device " << deviceNames << std::endl;
            return -2;
        } else 
            std::cout << "INFO: Start Fuzzy Match on " << deviceNames << std::endl;
    
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
    
        fuzzy[0] = cl::Kernel(prg, "fuzzy_kernel_1");
        fuzzy[1] = cl::Kernel(prg, "fuzzy_kernel_2");
    
        std::size_t u50_found = devName.find("u50");
        std::size_t u200_found = devName.find("u200");
        std::size_t u250_found = devName.find("u250");
        std::size_t f1_found = devName.find("aws-vu9p-f1");
        if (u50_found != std::string::npos || u200_found != std::string::npos || f1_found != std::string::npos)
        {
            boost = 0;
        }
        else if (u250_found != std::string::npos)
        {
            boost = 1;
        }
        else
        {
            std::cout << "Only U50/U200/U250/AWS-F1 is supported so far, please check it "
                            "and re-run"
                        << std::endl;
            exit(1);
        }
    
        if (boost)
        {
            fuzzy[2] = cl::Kernel(prg, "fuzzy_kernel_3");
            fuzzy[3] = cl::Kernel(prg, "fuzzy_kernel_4");
        }
    
        return 0;
    
    }
    
    int FuzzyMatch::fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern) 
    {
        return pImpl_->fuzzyMatchLoadVec(vec_pattern);
    }
    
    int FuzzyMatchImpl::fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern)
    {
        std::cout << "INFO: FuzzyMatchImpl::fuzzyMatchLoadVec vec_pattern size=" << vec_pattern.size() << std::endl;
            // Create device Buffer
        cl_mem_ext_ptr_t mext_i1, mext_i2, mext_o1, mext_o2;
        //cl_mem_ext_ptr_t mext_i3[2], mext_i4[2], mext_o3[2], mext_o4[2];
        //for (int i = 0; i < 2; i++)
        //{ // call kernel twice
            mext_i1 = {2, nullptr, fuzzy[0]()};
            mext_o1 = {11, nullptr, fuzzy[0]()};
            mext_i2 = {2, nullptr, fuzzy[1]()};
            mext_o2 = {11, nullptr, fuzzy[1]()};
    
            // create device buffer
            buf_field_i1 = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_READ_ONLY, sizeof(uint32_t) * 9, &mext_i1);
            buf_field_o1 = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_WRITE_ONLY, sizeof(uint32_t), &mext_o1);
            buf_field_i2 = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_READ_ONLY, sizeof(uint32_t) * 9, &mext_i2);
            buf_field_o2 = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_WRITE_ONLY, sizeof(uint32_t), &mext_o2);
    
        preSortbyLength(vec_pattern, this->vec_pattern_grp);
        // Do pre-group process for People deny list to boost Fuzzy function
    
        vec_base.resize(40, 0);
        vec_offset.resize(40, 0);
    
        std::cout << "    Pre-sorting..." << std::flush;
        std::vector<std::vector<std::string>> vec_grp_str(40);
        
        preCalculateOffsetPerPU(vec_grp_str, vec_base, vec_offset);
        int sum_line_num = sum_line;
        std::cout << "    Pre-sort completed" << std::endl;
    
        char *csv_part[PU_NUM];
        for (int i = 0; i < PU_NUM; i++)
        {
            //csv_part[i] = (char *)malloc(16 * 3 * sum_line_num);
            csv_part[i] = aligned_alloc<char>(16 * 3 * sum_line_num);
        }
    
        for (uint32_t i = 0; i < vec_grp_str.size(); i++)
        {
            for (int j = 0; j < vec_offset[i]; j++)
            {
                for (int p = 0; p < PU_NUM; p++)
                {
                    std::string str = "";
                    char len = 0;
                    if (unsigned(PU_NUM * j + p) < vec_grp_str[i].size())
                    {
                        str = vec_grp_str[i][PU_NUM * j + p];
                        len = i;
                    }
    
                    char tmp[48] = {len, 0};
                    for (int j = 1; j < 48; j++)
                    {
                        if (j - 1 < len)
                            tmp[j] = str.at(j - 1);
                        else
                            tmp[j] = 0;
                    }
    
                    for (int m = 0; m < 3; m++)
                    {
                        for (int k = 0; k < 16; k++)
                        {
                            csv_part[p][16 * (3 * (vec_base[i] + j) + m) + 15 - k] = tmp[m * 16 + k];
                        }
                    }
                }
            }
        }
    
        //transfer pattern vec
        cl_mem_ext_ptr_t mext_t[2 * PU_NUM];
        for (int i = 0; i < PU_NUM; i++)
        { // kernel-0
            mext_t[i] = {unsigned(i + 3), csv_part[i], fuzzy[0]()};
        }
        for (int i = 0; i < PU_NUM; i++)
        { // kernel-1
            mext_t[PU_NUM + i] = {unsigned(i + 3), csv_part[i], fuzzy[1]()};
        }
    
    
        int dup = (boost == 0) ? 2 : 4;
        for (int i = 0; i < dup * PU_NUM; i++)
        {
            buf_csv[i] = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                    (size_t)16 * 3 * sum_line_num, &mext_t[i]);
        }
    
        // h2d Transfer
        std::vector<cl::Memory> ob_in;
        for (int i = 0; i < dup * PU_NUM; i++)
            ob_in.push_back(buf_csv[i]);
        queue.enqueueMigrateMemObjects(ob_in, 0, nullptr, nullptr);
        queue.finish();
    
        // free memory
        for (int i = 0; i < PU_NUM; i++)
            free(csv_part[i]);
    
        return 0;
    }
    

    bool FuzzyMatch::executefuzzyMatch(std::string t) 
    { 
        return pImpl_->executefuzzyMatch(t);
    }

    bool FuzzyMatchImpl::executefuzzyMatch(const std::string& t)
    {
        uint32_t buf_f_i0[9];
        uint32_t buf_f_o0 = 0;
        uint32_t buf_f_o1 = 1;
    
        std::string name_list[4] = {"nombrePersona1", "nombrePersona2", "bank1", "bank2"};
    
        //for (int i = 0; i < 4; i++)
        //{
            std::string str = t;
            char len = str.length() > 35 ? 35 : str.length(); // truncate
            char tmp[36] = {len, str[0], str[1], str[2], 0};
            for (int j = 4; j < 36; j++)
            {
                if (j - 1 < len)
                    tmp[j] = str.at(j - 1);
                else
                    tmp[j] = 0;
            }
    
            for (int m = 0; m < 9; m++)  //m:PU number
            {
                char str_tmp[4];
                for (int k = 0; k < 4; k++)
                    str_tmp[3 - k] = tmp[m * 4 + k];
                    
                memcpy(&buf_f_i0[m], str_tmp, 4);
                    /*
                if (i % 2 == 0)
                    memcpy(&buf_f_i0[i / 2][m], str_tmp, 4);
                else
                    memcpy(&buf_f_i1[i / 2][m], str_tmp, 4);
                    */
            }
        //}
    
        //bool skip_field[4] = {false, false, false, false};
        bool skip_field = false;
        //bool sw_fuzzy_result[4] = {false, false, false, false};
        bool sw_fuzzy_result=false;
        //int base_trans[2 * 2], nrow_trans[2 * 2];
        int base_trans, nrow_trans;
        //for (int i = 0; i < 2 * 2; i++)
        //{
            if (t.length() > (max_fuzzy_len - 3))
            {
                //if (i < 2)
                sw_fuzzy_result = strFuzzy(max_fuzzy_len, t, vec_pattern_grp);
                // else
                //    sw_fuzzy_result[i] = strFuzzy(max_fuzzy_len, vec_str1[i], vec_pattern_grp);
            }
    
            if (t.length() > max_fuzzy_len || t.length() == 0) // disable this fuzzy on FPGA
                skip_field = true;
            else
                //getRange(t, vec_base[i / 2], vec_offset[i / 2], base_trans[i], nrow_trans[i]);
                getRange(t, vec_base, vec_offset, base_trans, nrow_trans);
        //}
    
        int dup = (boost == 0) ? 2 : 4;
    
            events_write.resize(dup);
            events_kernel.resize(dup);
            events_read.resize(dup);
    
    
        // struct timeval start_time, end_time;
        // std::cout << "INFO: kernel start------" << std::endl;
        // gettimeofday(&start_time, 0);
    
        // launch kernel and calculate kernel execution time
        
            int i=0;
            if (!skip_field) {
                    queue.enqueueWriteBuffer(buf_field_i1, CL_FALSE, 0, sizeof(uint32_t) * 9, buf_f_i0, nullptr,
                                                &events_write[0]);
    
                    queue.enqueueWriteBuffer(buf_field_i2, CL_FALSE, 0, sizeof(uint32_t) * 9, buf_f_i0, nullptr,
                                                &events_write[1]);
            }
        
    //kernel0 will compare input str against first half of pattern tbl
    //kernel1 will compare input str against first half of pattern tbl
            //int nrow00 = (1 + (nrow_trans[i * 2] / 3)) / 2 * 3;
            //int nrow01 = nrow_trans[i * 2] - nrow00;
            int nrow00 = (1 + (nrow_trans / 3)) / 2 * 3;
            int nrow01 = nrow_trans - nrow00;
    
            int j = 0;
            fuzzy[0].setArg(j++, base_trans);
            fuzzy[0].setArg(j++, nrow00);
            fuzzy[0].setArg(j++, buf_field_i1);
            for (int k = 0; k < PU_NUM; k++) fuzzy[0].setArg(j++, buf_csv[k]);
            fuzzy[0].setArg(j++, buf_field_o1);
    
    
            j = 0;
            fuzzy[1].setArg(j++, (base_trans + nrow00));
            fuzzy[1].setArg(j++, nrow01);
            fuzzy[1].setArg(j++, buf_field_i2);
            for (int k =  PU_NUM; k < 2 * PU_NUM; k++) fuzzy[1].setArg(j++, buf_csv[k]);
            fuzzy[1].setArg(j++, buf_field_o2);
    
            std::vector<cl::Event> waitEnqueueEvents0{events_write[0]};
            std::vector<cl::Event> waitEnqueueEvents1{events_write[1]};
    
            if (!skip_field) {
                queue.enqueueTask(fuzzy[0], &waitEnqueueEvents0, &(events_kernel[0]));
    
                queue.enqueueTask(fuzzy[1], &waitEnqueueEvents1, &(events_kernel[1]));
            }
    
            std::vector<cl::Event> waitEnqueueReadEvents0{events_kernel[0]};
            std::vector<cl::Event> waitEnqueueReadEvents1{events_kernel[1]};
    
            if (!skip_field) {
                queue.enqueueReadBuffer(buf_field_o1, CL_FALSE, 0, sizeof(uint32_t), &buf_f_o0, &waitEnqueueReadEvents0, &(events_read[0]));
            
                queue.enqueueReadBuffer(buf_field_o2, CL_FALSE, 0, sizeof(uint32_t), &buf_f_o1, &waitEnqueueReadEvents1, &(events_read[1]));
            }
        
        queue.flush();
        queue.finish();
    
        bool r=false;
        if(sw_fuzzy_result || buf_f_o0 == 1 || buf_f_o1 == 1) r = true; // person1
       
        return r;
    }
       

} // namespace fuzzymatch
} // namespace xilinx_apps

//#####################################################################################################################

//
// Shared Library Entry Points
//

extern "C" {

xilinx_apps::fuzzymatch::FuzzyMatchImpl *xilinx_fuzzymatch_createImpl(const xilinx_apps::fuzzymatch::Options& options) {
    return new xilinx_apps::fuzzymatch::FuzzyMatchImpl(options);
}

void xilinx_fuzzymatch_destroyImpl(xilinx_apps::fuzzymatch::FuzzyMatchImpl *pImpl) {
    delete pImpl;
}

}  // extern "C"
