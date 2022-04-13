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
#include "ap_int.h"
#include "xilinx_runtime_common.hpp"
#include "fuzzymatch.hpp"



namespace xilinx_apps {
namespace fuzzymatch {
    #ifdef DDRBASED
    const int max_num_of_entries_for_big_tbl = 100*1024*1024;
    //For DDR-based AWS F1/U200/U250 cards, the maximum batch_num should be 4096*1024
    const int max_batch_num = 4096*1024;
    #else
    const int max_num_of_entries_for_big_tbl = 25*1024*1024;
    //For U50, each HBM bank is 256MB, the batch_num could be set up to 256*1024.
    const int max_batch_num = 256*1024;
    #endif

    const int max_fuzzy_len=64;
    class FuzzyMatchImpl {
    public:
        Options options_;  // copy of options passed to FuzzyMatch constructor
        
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
        std::vector<std::vector<int> > vec_pattern_id = 
            std::vector<std::vector<int> > (max_len_in_char);
      
        std::vector<int> vec_base ;
        std::vector<int> vec_offset ;
        cl::Buffer buf_csv[2 * PU_NUM];
        cl::Buffer buf_field_i[2];
        cl::Buffer buf_field_o[2];

    
        int getDevice(const std::string& deviceNames);
        int startFuzzyMatch(const std::string& xclbinPath, const std::string& deviceNames);

        int fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern,std::vector<int> vec_id=std::vector<int>());
        
        // batch mode
        // for each string in input_patterns vectors, run fuzzymatch, return maximum top 100 matched results in pair format {id,score} 
        std::vector<std::vector<std::pair<int,int>>> executefuzzyMatch(std::vector<std::string> input_patterns, int similarity_level);

        void preCalculateOffsetPerPU(
                    std::vector<std::vector<std::pair<int, std::string> > >& vec_grp_str,
                    std::vector<int> &vec_base,
                    std::vector<int> &vec_offset);
    
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

    int getRange(int similarity_level,
             std::string& input,
             std::vector<int>& vec_base,
             std::vector<int>& vec_offset,
             int& base,
             int& nrow) {
        const int line_per_elem = (max_fuzzy_len + 1 + 4 + 15) / 16;
        int len = input.length();
        int med = len * (100 - similarity_level) / 100;
        int cnt = 0;
        int max_len = (len + med) > 64 ? 64 : (len + med);
        if (len == 0) {
            std::cout << "Warning: lenght of input pattern string is too short, please check again." << std::endl;
            return -1;
        }


        base = vec_base[len - med] * line_per_elem; // as 128-bit data width in AXI
        for (int i = len - med; i <= max_len; i++) {
            cnt += vec_offset[i];
        }

        nrow = cnt * line_per_elem;

        return 0;
    }
    void FuzzyMatchImpl::preCalculateOffsetPerPU(
                            std::vector<std::vector<std::pair<int, std::string> > >& vec_grp_str,
                            std::vector<int> &vec_base,
                            std::vector<int> &vec_offset)
    {
        // the input table already grouped by length
        // scan the string vec for each length, compute:
        // vec_base : base address for each length in each PU
        // vec_offset : each PU get the vec size
        this->sum_line = 0;
        for (size_t i = 0; i <= max_fuzzy_len; i++)
        {
            //vec_grp_str[i] = this->vec_pattern_grp[i];
            int size = this->vec_pattern_grp[i].size();
            for (int j = 0; j < size; j++) {
                vec_grp_str[i].push_back(std::make_pair(this->vec_pattern_id[i][j], this->vec_pattern_grp[i][j]));
            }
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
    
            if (deviceNames == curDeviceName || (deviceNames == "azure_u250" && curDeviceName == "xilinx_u250_gen3x16_xdma_shell_2_1")) {
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

        std::string krnl_name = "fuzzy_kernel";
        cl_int err = 0;
        cl_uint cu_num;
        {
            cl_int err_code;
            cl::Kernel k = cl::Kernel(prg, krnl_name.c_str(), &err_code);
            if (err_code != CL_SUCCESS) {
                std::cout << "ERROR: failed to create kernel." << std::endl;
                exit(1);
            }
            k.getInfo<cl_uint>(CL_KERNEL_COMPUTE_UNIT_COUNT, &cu_num);
            std::cout << "INFO: " << krnl_name << " has " << cu_num << " CU(s)" << std::endl;
        }
        //cl::Kernel fuzzy[4];
        for (cl_uint i = 0; i < cu_num; i++) {
            std::string krnl_full_name = krnl_name + ":{" + krnl_name + "_" + std::to_string(i + 1) + "}";
            fuzzy[i] = cl::Kernel(prg, krnl_full_name.c_str(), &err);
        }

    
        return 0;
    
    }
    
    int FuzzyMatch::fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern,std::vector<int> vec_id) 
    {
        return pImpl_->fuzzyMatchLoadVec(vec_pattern, vec_id);
    }
    
    int FuzzyMatchImpl::fuzzyMatchLoadVec(std::vector<std::string>& vec_pattern,std::vector<int> vec_id)
    {
        std::cout << "INFO: FuzzyMatchImpl::fuzzyMatchLoadVec vec_pattern size=" << vec_pattern.size() << std::endl;
        // check big table size should ot larger than max_num_of_entries_for_big_tbl
        if (vec_pattern.size() > max_num_of_entries_for_big_tbl) {
            std::ostringstream oss;
            oss << "Input Pattern vector size should not larger than" << max_num_of_entries_for_big_tbl << "; Please split pattern vector properly and run again" <<std::endl;
            throw xilinx_apps::fuzzymatch::Exception(oss.str());
            std::cerr << "Input Pattern vector size should not larger than" << max_num_of_entries_for_big_tbl << "; Please split pattern vector properly and run again" <<std::endl;
            abort();
        }
        //group the pattern id
        if(!vec_id.empty()){    
            for (int idx=0; idx < vec_pattern.size(); idx++) {
                size_t len = vec_pattern[idx].length();
                //assert(len < max_pattern_len_in_char && "Defined <max_people_len_in_char> is not enough!");
                 if (len > max_fuzzy_len) {
                    std::ostringstream oss;
                    oss << "the string " << vec_pattern[idx] << " length should not larger than" << max_fuzzy_len <<std::endl;
                    throw xilinx_apps::fuzzymatch::Exception(oss.str());
                    std::cerr << "ERROR: the string " << vec_pattern[idx] << " length should not larger than" << max_fuzzy_len <<std::endl;
                    abort();
                }
                vec_pattern_id[len].push_back(vec_id[idx]);
            }
        } else {
            //default assignment
            int id_cnt = 0;
            for (std::vector<std::string>::iterator it = vec_pattern.begin(); it != vec_pattern.end(); ++it) {
                size_t len = it->length();
                //assert(len < max_pattern_len_in_char && "Defined <max_people_len_in_char> is not enough!");
                if (len > max_fuzzy_len) {
                    std::ostringstream oss;
                    oss << "the string " << *it << " length should not larger than" << max_fuzzy_len <<std::endl;
                    throw xilinx_apps::fuzzymatch::Exception(oss.str());
                    std::cerr << "ERROR: the string " << *it << " length should not larger than" << max_fuzzy_len <<std::endl;
                    abort();
                }
                vec_pattern_id[len].push_back(id_cnt++);
            }
        }
        preSortbyLength(vec_pattern, this->vec_pattern_grp);
        // Do pre-group process for People deny list to boost Fuzzy function
    
        vec_base.resize(100, 0);
        vec_offset.resize(100, 0);
    
        //std::cout << "    Pre-sorting..." << std::flush;
        std::vector<std::vector<std::pair<int, std::string> > > vec_grp_str(100);
        preCalculateOffsetPerPU(vec_grp_str, vec_base, vec_offset);
        int sum_line_num = sum_line;
        //std::cout << "    Pre-sort completed" << std::endl;
    
        const int line_per_elem = (max_fuzzy_len + 1 + 4 + 15) / 16;
        char *csv_part[PU_NUM];
        for (int i = 0; i < PU_NUM; i++)
        {
            //csv_part[i] = (char *)malloc(16 * 3 * sum_line_num);
            csv_part[i] = aligned_alloc<char>(16 * line_per_elem  * sum_line_num);
        }
    
        for (uint32_t i = 0; i < vec_grp_str.size(); i++)
        {
            for (int j = 0; j < vec_offset[i]; j++)
            {
                for (int p = 0; p < PU_NUM; p++)
                {
                    std::string str = "";
                    char len = 0;
                    int id = 0;
                    if (unsigned(PU_NUM * j + p) < vec_grp_str[i].size())
                    {
                        str = vec_grp_str[i][PU_NUM * j + p].second;
                        id = vec_grp_str[i][PU_NUM * j + p].first;
                        len = i;
                    }
    
                    char tmp[(line_per_elem - 1) * 16] = {0};

                    for (int j = 0; j < len; j++) tmp[j] = str.at(j);

                    for (int m = 0; m < line_per_elem - 1; m++) {
                        for (int k = 0; k < 16; k++) {
                            csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + m) + 15 - k] = tmp[m * 16 + k];
                        }
                    }
                    csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + line_per_elem - 1) + 0] = len;
                    csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + line_per_elem - 1) + 1] = id & 0xFF;
                    csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + line_per_elem - 1) + 2] = (id >> 8) & 0xFF;
                    csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + line_per_elem - 1) + 3] = (id >> 16) & 0xFF;
                    csv_part[p][16 * (line_per_elem * (vec_base[i] + j) + line_per_elem - 1) + 4] = (id >> 24) & 0xFF;
                }
            }
        }
    
        //transfer pattern vec
        cl_mem_ext_ptr_t mext_t[2 * PU_NUM];

        for (int i = 0; i < PU_NUM; i++) { // kernel-0
            mext_t[i] = {unsigned(i + 2), csv_part[i], fuzzy[0]()};
        }
        for (int i = 0; i < PU_NUM; i++) { // kernel-1
            mext_t[PU_NUM + i] = {unsigned(i + 2), csv_part[i], fuzzy[1]()};
        }
    
    
       
        for (int i = 0; i < 2 * PU_NUM; i++)
        {
            buf_csv[i] = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                                    (size_t)16 * line_per_elem * sum_line_num, &mext_t[i]);
        }
    
        // h2d Transfer
        std::vector<cl::Memory> ob_in;
        for (int i = 0; i < 2 * PU_NUM; i++)
            ob_in.push_back(buf_csv[i]);
        queue.enqueueMigrateMemObjects(ob_in, 0, nullptr, nullptr);
        queue.finish();
    
        // free memory
        for (int i = 0; i < PU_NUM; i++)
            free(csv_part[i]);
    
        return 0;
    }
    

    //bool FuzzyMatch::executefuzzyMatch(std::string t,int similarity_level)
    std::vector<std::vector<std::pair<int,int>>> FuzzyMatch::executefuzzyMatch(std::vector<std::string> input_patterns,int similarity_level)  
    { 
        return pImpl_->executefuzzyMatch(input_patterns, similarity_level);
    }

    //bool FuzzyMatchImpl::executefuzzyMatch(const std::string& t, int similarity_level)
    std::vector<std::vector<std::pair<int,int>>> FuzzyMatchImpl::executefuzzyMatch(std::vector<std::string> input_patterns, int similarity_level)
    {
        int batch_num = input_patterns.size();
        //check batch_num should not larger than max_batch_num due to memroy size
        if (batch_num > max_batch_num) {
            std::ostringstream oss;
            oss << "Input patterns size should not larger than " << max_batch_num << "; Please split the input_patterns vector into smaller vectors and run again" <<std::endl;
            throw xilinx_apps::fuzzymatch::Exception(oss.str());
            std::cerr << "ERROR: Input patterns size should not larger than " << max_batch_num << "; Please split the input_patterns vector into smaller vectors and run again" <<std::endl;
            abort();
        } 
        cl_mem_ext_ptr_t mext_i[2], mext_o[2];
       // cl::Buffer buf_field_i[2], buf_csv[2 * PU_NUM], buf_field_o[2];

        for (int i = 0; i < 2; i++) { // for each kernel
            mext_i[i] = {1, nullptr, fuzzy[i]()};
            mext_o[i] = {10, nullptr, fuzzy[i]()};
            buf_field_i[i] = cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_READ_ONLY,
                                        sizeof(uint32_t) * (batch_num * (4 + (max_fuzzy_len + 3) / 4)), &mext_i[i]);
            buf_field_o[i] =
                cl::Buffer(ctx, CL_MEM_EXT_PTR_XILINX | CL_MEM_WRITE_ONLY, sizeof(uint32_t) * batch_num * 201, &mext_o[i]);
        }

        const int threshold = similarity_level;
        const int fold_len_per_str = 1 + (max_fuzzy_len + 3) / 4;
        int base_trans = 0;
        int nrow_trans = 0;
        ap_uint<32>* buf_f_i[2];
        ap_uint<32>* buf_result[2];

        for (int i = 0; i < 2; i++) {
            buf_f_i[i] = (ap_uint<32>*)malloc(sizeof(ap_uint<32>) * batch_num * (3 + fold_len_per_str));
            buf_result[i] = (ap_uint<32>*)malloc(sizeof(ap_uint<32>) * batch_num * 201);  
        }
        double avg_time = 0.0;

        int batchNumForkernel[2];
        if(batch_num%2==0) {
            batchNumForkernel[0]=batch_num/2;
            batchNumForkernel[1]= batch_num/2;
        } else {
            batchNumForkernel[0] = (batch_num+1)/2;
            batchNumForkernel[1] = batch_num - (batch_num+1)/2;
        }
        
        // batch processing
        for (int i = 0; i < batch_num; i++) { // each batch
            // first half of strings run in kernel0; the second half of input strings run in kernel 1
            int k = (i < (batch_num+1)/2) ? 0:1;
            std::string ptn_string = input_patterns[i];
            //std::cout << "Trans-" << i << ", Pattern String: <" << ptn_string
            //            << "> being allocated to kernel-" << k << std::endl;
            getRange(threshold, ptn_string, vec_base, vec_offset, base_trans, nrow_trans);
            
            int localIdx = i % ((batch_num+1)/2);
            buf_f_i[k][localIdx * (3 + fold_len_per_str)] = base_trans;
            buf_f_i[k][localIdx * (3 + fold_len_per_str) + 1] = nrow_trans;
            buf_f_i[k][localIdx * (3 + fold_len_per_str) + 2] = threshold;
            for (unsigned int j = 0; j < max_fuzzy_len; j++) {
                if (j < ptn_string.size())
                    buf_f_i[k][localIdx * (3 + fold_len_per_str) + 3 + (j / 4)]((3 - j % 4) * 8 + 7, (3 - j % 4) * 8) =
                        ptn_string.at(j);
                else
                    buf_f_i[k][localIdx * (3 + fold_len_per_str) + 3 + (j / 4)]((3 - j % 4) * 8 + 7, (3 - j % 4) * 8) = 0;
            }
            buf_f_i[k][(localIdx + 1) * (3 + fold_len_per_str) - 1](7, 0) = ptn_string.size();
        }

        std::vector<std::vector<cl::Event> > events_write =
            std::vector<std::vector<cl::Event> >(2, std::vector<cl::Event>(1));
        std::vector<std::vector<cl::Event> > events_kernel =
            std::vector<std::vector<cl::Event> >(2, std::vector<cl::Event>(1));
        std::vector<std::vector<cl::Event> > events_read =
            std::vector<std::vector<cl::Event> >(2, std::vector<cl::Event>(1));
            // h2d + kernel run + d2h
 
        //corner case batch num =1 
        int idx=0;
        while(idx<batch_num && idx<2) {
            int k = idx%2;
            queue.enqueueWriteBuffer(buf_field_i[k], CL_FALSE, 0, sizeof(uint32_t) * batch_num * (3 + fold_len_per_str),
                                        buf_f_i[k], nullptr, &events_write[k][0]);
            int j = 0;
            
            fuzzy[k].setArg(j++, batchNumForkernel[k]);
            fuzzy[k].setArg(j++, buf_field_i[k]);
            for (int m = k * PU_NUM; m < (k + 1) * PU_NUM; m++) fuzzy[k].setArg(j++, buf_csv[m]);
            fuzzy[k].setArg(j++, buf_field_o[k]);
            queue.enqueueTask(fuzzy[k], &events_write[k], &events_kernel[k][0]);
            queue.enqueueReadBuffer(buf_field_o[k], CL_FALSE, 0, sizeof(uint32_t) * batch_num * 201, buf_result[k],
                                    &events_kernel[k], &events_read[k][0]);
            idx++;
        }
        queue.flush();
        queue.finish();
        //std::vector<std::vector<int>> results(batch_num); 
        // for each pattern string store top 100 matched patterns. fixed size 201 ints. 
        // 0 th is hit number ; 1..100 is match id; 101..200 is score
        std::vector<std::vector<std::pair<int,int>>> results(batch_num);
        for(int bi = 0;bi < batch_num; bi++){
            int i = bi % ((batch_num+1)/2);
            int k = (bi<(batch_num+1)/2) ? 0 :1;
            int cnt = buf_result[k][i*201]; 
            for(int j=0; j<cnt; j++){
                results[bi].push_back(std::make_pair(buf_result[k][i*201 + 1+j], buf_result[k][i*201+101+j]));
            }               
        }
        
        return results;
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
