#include "utils2.hpp"

ap_uint<64>* GenPair(long NV, long num){
    ap_uint<64>* pair = aligned_alloc<ap_uint<64> >(num);
    for(int i=0; i<num; i++){
        ap_uint<32> head= random()%NV+1;
        ap_uint<32> tail= random()%NV+1;
        pair[i](31,0) = head;
        pair[i](63,32) = tail; 
    }
    return pair;
}
void SavePair(ap_uint<64>* pair, long num, const char* name){
    FILE* fp=fopen(name, "w");
    fprintf(fp,"%ld\n", num);
    for(int i=0; i<num; i++){
        long head= pair[i](31,0);
        long tail= pair[i](63,32);
        fprintf(fp, "%ld %ld \n", head, tail);
    }
    fclose(fp);
}
