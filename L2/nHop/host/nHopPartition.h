#ifndef _NHOPPARTITION_H_
#define _NHOPPARTITION_H_

#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <map>

#include "nHopCSR.h"
#include "nHopCSR.hpp"
#include "ctime.hpp"
extern CTimeModule<unsigned long> gtimer;

#define MAX_CORE_COPY (128)
#define MAX_CORE (128)
#define MAX_NUM_CHNL_KNL (16)
// MAX_NUM_CHNL_PAR (MAX_CORE * MAX_CHNL)
#define MAX_NUM_CHNL_PAR (1024)
#define MAX_NUM_KNL_PAR (1024)
#define MAX_NUM_KNL_USED (1024)
#define MAX_NUM_HOP (16)

// return the real number of partition //call FindSegmentPoint()
template <class T>
int CSRPartition_average(int num_chnl_par, CSR<T>& src, T limit_nv, T limit_ne, CSR<T>* des[]);

// call FindPar return ParID
template <class T>
int SelectParID(T v, int num_par, T* tab_startV, int* tab_copy, int* state_copy);

template <class T>
bool CheckRange_v(T v, T V_start, T V_end_1);

template <class T>
struct HopPack {
    T src;
    T des;
    T idx;
    T hop;
    void print() { printf("[s(%ld)d(%ld)i(%ld)h(%ld)]", src, des, idx, hop); }
    void print(long i) { printf(" PackNO(%d):[s(%ld)d(%ld)i(%ld)h(%ld)]\n", i, src, des, idx, hop); }
};
template <class T>
struct AccessPoint {
    T* p_idx;
    int degree;
};

// 1 FPGA compute unit has NUM_CHAL (8 channels) now
// partition data for hop channel
template <class T>
class HopChnl {
   public:
    CSR<T>* pCSR;
    T indexShift;  // initialized by reading offset[0];
    T offsetShift; // initialized by pCSR->V_start;
    HopChnl() { pCSR = NULL; }
    void Init(CSR<T>* pcsr) {
        assert(pcsr);
        pCSR = pcsr;
        indexShift = pCSR->offset[0];
        offsetShift = pCSR->V_start;
    }
    T getOffset(T v) {
        assert(pCSR);
        return pCSR->offset[v - offsetShift];
    }
    bool isVinRange(T v) { return CheckRange_v<T>(v, pCSR->V_start, pCSR->V_end_1); }
    T* LookUp(
        int* degree,
        T v); // For CPU similation, no need to created real data structure like HopPack which will take too more memory
    void LookUp(
        AccessPoint<T>* accp,
        T v); // For CPU similation, no need to created real data structure like HopPack which will take too more memory
    void LookUp(AccessPoint<T>* accp, HopPack<T>* hpk); // For CPU similation, no need to created real data structure
                                                        // like HopPack which will take too more memory
    T LookUp(T* des, int size, T v); // Return the real degree;
};

// for net package
template <class T>
class PackBuff {
    HopPack<T>* pBuff;
    long size;
    long p_write; // alwyas points to a empty place can be wrote
    long p_read;  // points to a place can be read if p_write not points same place
    bool Empty;

   public:
    PackBuff() {
        pBuff = NULL;
        size = 0;
        ResetBuff();
    }
    PackBuff(long size_buff) { InitBuff(size_buff); }
    ~PackBuff() {
        if (pBuff) free(pBuff);
    }
    int InitBuff(long size_buff);
    void ResetBuff();

    int push(AccessPoint<T>* p_accp, HopPack<T>* p_hpk); // return how many package stored
    int push(HopPack<T>* p_hpk);
    int pop(HopPack<T>* p_hpk);

    long GetNum() { return Empty ? 0 : (p_write > p_read) ? (p_write - p_read) : size - (p_read - p_write); }
    bool isFull() { return GetNum() == size ? true : false; }
    bool isFull(int toAdd) {
        assert(toAdd <= size);
        return (GetNum() + toAdd < size) ? true : false;
    }
    bool isEmpty() { return Empty; }

    float Ratio_used() { return 100.0 * (float)(GetNum()) / (float)size; }
    float Ratio_use() { return 1.0 - Ratio_used(); }
    void ShowInfo_buff() { ShowInfo_buff(true); }
    long getSize() { return size; }
    void ShowInfo_buff(bool isReturn);
    void print();
    void ShowInfo_buff(const char* nm, bool isReturn);
};

struct IndexStatistic { // for index statistic
    long num_v;
    long num_all;   //=0;
    long num_local; //=0;
    long num_send;  //=0;
    long num_agg;   //=0;
    long num_push;
    long num_free;
    IndexStatistic() {
        num_v = 0;
        num_all = 0;
        num_local = 0;
        num_send = 0;
        num_agg = 0;
        num_push = 0;
        num_free = 0;
    }
    float r_local() { return num_all == 0 ? 0 : (100.0 * (float)num_local / (float)num_all); }
    float r_send() { return num_all == 0 ? 0 : (100.0 * (float)num_send / (float)num_all); }
    float r_agg() { return num_all == 0 ? 0 : (100.0 * (float)num_agg / (float)num_all); }
    float r_push() { return num_all == 0 ? 0 : (100.0 * (float)num_push / (float)num_all); }
    float r_free() { return num_all == 0 ? 0 : (100.0 * (float)num_free / (float)num_all); }
    void print() {
        // printf("Stt:V%-4ld)->Idx(%-4ld):Local(%-4ld),Send(%-4ld),Agg(%-4ld); Push(%-4ld),Free(%-4ld)", num_v,
        // num_all, num_local, num_send, num_agg, num_push, num_free);
        // printf("Stt:V%3ld->Idx%3ld::\tLocal:%3ld(%2.1f%%)  +  Send:%3ld(%2.1f%%)  +  Agg:%3ld(%2.1f%%);\t
        // Push:%3ld(%2.1f%%) + Free:%3ld(%2.1f%%)",
        printf(
            "Input package:%-6ld -> Output Idx:%-6ld including:\tLocal:%-6ld(%3d%%)  +  Send:%-6ld(%3d%%)  +  "
            "Agg:%-6ld(%3d%%);\t Push:%-6ld(%3d%%) + Free:%-6ld(%3d%%)",
            num_v, num_all, num_local, (int)r_local(), num_send, (int)r_send(), num_agg, (int)r_agg(), num_push,
            (int)r_push(), num_free, r_free());
    }
};

struct commendInfo{
    int numKernel=1;
    int numPuPerKernel=4;
    std::string xclbin_path;
    int sz_bat=4096;
    int byPass=0; 
    int duplicate=1;
};

template <class T>
class HopKernel { // warpper the hop kernel
   public:
    int start_chnl;
    int num_chnl_knl;
    int num_chnl_par;
    int num_knl_par;
    int id_knl_used;
    HopChnl<T> channels[MAX_NUM_CHNL_KNL];
    T tab_disp_ch[MAX_NUM_CHNL_KNL];
    int tab_copy_ch[MAX_NUM_CHNL_KNL];
    int state_copy_ch[MAX_NUM_CHNL_KNL];
    T tab_disp_knl[MAX_NUM_KNL_PAR];
    int tab_copy_knl[MAX_NUM_KNL_PAR];
    int tab_state_knl[MAX_NUM_KNL_USED];

    PackBuff<T>* p_buff_in;
    PackBuff<T>* p_buff_out;
    PackBuff<T>* p_buff_ping;
    PackBuff<T>* p_buff_pang;
    PackBuff<T>* p_buff_agg;
    PackBuff<T>* p_buff_pp[2];

    HopKernel();
    ~HopKernel();
    // Logic function for judging a package content; There can be a lot of logic funtions for different hop-based
    // application
    bool Judge_equal(HopPack<T>& pkg); // pkg.des==pkg.idx)

// clang-format off
    // hop processing 1 batch of vertex
    int BatchOneHop(PackBuff<T>* p_buff_pop, PackBuff<T>* p_buff_send, PackBuff<T>* p_buff_local,
                    long sz_bat, int num_hop, IndexStatistic* p_stt);

    // hop processing 1 batch of vertex on FPGA
    int BatchOneHopOnFPGA(PackBuff<T>* p_buff_pop, PackBuff<T>* p_buff_send, PackBuff<T>* p_buff_local, PackBuff<T>* p_buff_agg,
                          T NV, T NE, T numSubPairs, int num_hop, T estimateBatchSize, commendInfo commendInfo, IndexStatistic* p_stt);
// clang-format on
    long estimateBatchSize(int cnt_hop, long sz_suggest, PackBuff<T>* p_buff_pop);

    int ConsumeBatch(T NV, T NE, int rnd, T numSubpair, long sz_bat, int num_hop, commendInfo commendInfo); // call BatchOneHopOnFPGA()

    void InitBuffs(long sz_in, long sz_out, long sz_pp, long sz_agg);
    void InitCore(CSR<T>* par_chnl_csr[], int start, int num_ch_knl, int num_ch_par);
    void InitCore(CSR<T>* par_chnl_csr[],
                  int start,
                  int num_ch_knl,
                  int num_ch_par,
                  int num_knl_p,
                  int id_knl_u,
                  T* tab_disp_k, //[MAX_NUM_KNL_PAR];
                  int* tab_copy_k /*[MAX_NUM_KNL_PAR];*/);
    void InitCore(CSR<T>* par_chnl_csr[], int start, int num) { InitCore(par_chnl_csr, start, num, num); }
    void LookUp(AccessPoint<T>* p_accp, HopPack<T>* p_hpk); // For CPU similation, no need to created real data
                                                            // structure like HopPack which will take too more memory
    int SelectChnlParID(T v);
    int GetChnlParID(int i_ch) { return start_chnl + i_ch % this->num_chnl_par; }
    int GetNumChnlCopy() { return (num_chnl_knl + num_chnl_par - 1) / num_chnl_par; }
    int GetKnlParID() { return this->id_knl_used % this->num_knl_par; }
    void ShowHopKernelInfo(int id_core);
    void ShowDispTab_intra(int id_core);
};

template <class T>
HopKernel<T>::HopKernel() {
    start_chnl = 0;
    num_chnl_knl = 0;
    num_chnl_par = 0;
    num_knl_par = 0;
    id_knl_used = 0;
    p_buff_in = NULL;
    p_buff_out = NULL;
    p_buff_ping = NULL;
    p_buff_pang = NULL;
    p_buff_agg = NULL;
    p_buff_pp[0] = NULL;
    p_buff_pp[1] = NULL;
}
template <class T>
HopKernel<T>::~HopKernel() {
    if (p_buff_in) free(p_buff_in);
    if (p_buff_out) free(p_buff_out);
    if (p_buff_ping) free(p_buff_ping);
    if (p_buff_pang) free(p_buff_pang);
    if (p_buff_agg) free(p_buff_agg);
}

template <class T>
class PartitionHop { // the class for partition graph, generate all pointer of channel data
   public:
    CSR<T>* hop_src;

    // Number of hop channel in kernel. Now it is suppoosed that all kernels have same number of hop channel
    // Different kernel implementation may have different num_chnl_knl, from 2 ~ 16 maybe.
    // Usually, for graphs with large degree less channel may already match the bandwidth of network and more HBM space
    // can be saved for buffering
    int num_chnl_knl;
    T limit_v_b;
    T limit_e_b;

    // Number of kernel(card) used for cover entire graph
    // The ID of partition kernel begins with 0, as 0, 1, 2, 3, .....num_knl_par - 1
    int num_knl_par;

    // Number of hop channel used for cover entire graph, = num_knl_par * num_chnl_inDev
    // The ID of partition channel begins with 0, as 0, 1, 2, 3, .....num_chnl_par - 1
    int num_chnl_par;

    // Number of kernel used for deploying graph. num_knl_used >= num_knl_par
    // The ID of kernel used begins with 0, as 0, 1, 2, 3, .....num_knl_used - 1
    // The mapping for ID_knl_used to ID_knl_par =  ID_knl_used % num_knl_par, this will be used by host for deploying
    // data on kernel
    // The number of cpopy for eatch ID_knl_par =  ID_knl_used / num_knl_par, this will be deploied on kernel
    // The mapping for ID_knl_par to ID_knl_used =  ID_knl_par + Status_RoundRobin * num_knl_par, this will be used for
    // dispatching package on dev
    int num_knl_used;

    CSR<T>* par_chnl_csr[MAX_NUM_CHNL_PAR]; //
    HopKernel<T>* hopKnl[MAX_NUM_KNL_USED];
    // T StartVTab_chnl[MAX_NUM_CHNL_PAR];
    // T StartVTab_knl[MAX_NUM_KNL_PAR];
    int tab_copy_knl[MAX_NUM_KNL_PAR];
    int tab_state_knl[MAX_NUM_KNL_USED];
    T tab_disp_knl[MAX_NUM_KNL_PAR];

    PartitionHop();
    PartitionHop(CSR<T>* src);
    ~PartitionHop();

    int CreatePartitionForKernel(int num_knl_in, int num_chnl_knl_in, T limit_nv_byte, T limit_ne_byte);
    int LoadPair2Buffs(ap_uint<64>* pairs, int num_pair, T NV, T NE, int num_hop, commendInfo commendInfo);

    // ParID_ch to ParID_knl
    int ParID_knl(int ParID_ch) { return ParID_ch / num_chnl_knl; }
    // ParID_knl to NumCpy
    int NumCpy_knl() { return (num_knl_used + num_knl_par - 1) / num_knl_par; }
    int NumCpy_knl(int ParID_knl);
    // ParID_ch to NumCpy
    int NumCpy_ch(int ParID_ch) { return NumCpy_knl(ParID_knl(ParID_ch)); }
    // ParID_knl to NumCpy //TODO

    int GetNumKnlCopy(int id_kp); // Return how many copies for this kernel partition
    void syncDispTab_inter();
    void syncDispTab_intra(HopKernel<T>* pKnl, int id_knl_used);
    void ShowDevPar();
    // int Deploykernel(int num_knl_used_in, HopKernel<T>* p_knl){}
    // give a channel parition ID, the function return how many and which kernel and which hop-channel in the kernel
    // deployed the channel paritition with the ID
    //   int* p_id_knl, //output: kernels' id in a stream
    //   int* p_no_ch,  //output: which channel in the kernel used for store the 'id_ch_par'
    //   int id_ch_par  //intput: Channel-paritition ID
    int FindChnlInKnl(int* p_id_knl, int* p_no_ch, int id_ch_par);
    void ShowInfo();
    void ShowChnlPar();
    void ShowInfo_buff_in();
    void ShowInfo_buff_out();
    void ShowInfo_buff_agg();
    void ShowInfo_buff_pp(int pp);
    void ShowInfo_buffs(int rnd);
    int DoSwitching(int id_src);
    int DoSwitching();
    int DoAggregation(int id_src);
    int DoAggregation();
    void FreePar();
    void FreeKnl();
};

template <class T>
PartitionHop<T>::PartitionHop() {
    num_knl_used = 0;
    num_chnl_par = 0;
    hop_src = NULL;
    for (int i = 0; i < MAX_NUM_CHNL_PAR; i++) par_chnl_csr[i] = NULL;
    for (int i = 0; i < MAX_NUM_KNL_USED; i++) {
        tab_state_knl[i] = 0;
        hopKnl[i] = NULL;
    }
}
template <class T>
PartitionHop<T>::PartitionHop(CSR<T>* src) {
    PartitionHop();
    hop_src = src;
}
template <class T>
PartitionHop<T>::~PartitionHop() {
    FreePar();
    FreeKnl();
}
#endif
