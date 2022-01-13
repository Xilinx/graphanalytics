#include "ap_int.h"
#include "hls_memfifo.hpp"

void hls_watchInStrm(ap_uint<16> id_broadcast,
                     ap_uint<16> id_self,
                     ap_uint<8> blen_write, // how many data can send a token for next bust pushing
                     long cycle2sleep,
                     long times_sleep2over,
                     hls::stream<ap_uint<512> >& strm_in,
                     hls::stream<ap_uint<512> >& strm_out_wdg, // contains ending code
                     hls::stream<ap_uint<8> >& strm_out_token,
                     hls::stream<bool>& strm_out_status_e,
                     hls::stream<ap_uint<512> >& strm_out_status) {
    watchInStrm<512, 8>(id_broadcast, id_self, blen_write, cycle2sleep, times_sleep2over, strm_in, strm_out_wdg,
                        strm_out_token, strm_out_status_e, strm_out_status);
}

void hls_pushToMem(hls::stream<ap_uint<8> >& strm_token,
                   hls::stream<ap_uint<512> >& strm_in,
                   ap_uint<512>* p_buff_full,
                   long userSize) {
#pragma HLS INTERFACE m_axi depth = 65536 latency = 32 port = p_buff_full bundle = gmem1 num_read_outstanding = \
    32 num_write_outstanding = 32
    ap_uint<8> t;
    long p_write;
    long num_space;
    long p_read;
    ap_uint<512>* p_buff = p_buff_full + 2;
    pushToMem(strm_token, strm_in, p_buff, userSize);
}

void hls_popFromMem(long pop_blen_read, // suggested burst reading lenght, it not met, then waiting pop_ii_check cycles
                                        // and check again, it check times met pop_max_check then read anyway
                    long pop_ii_check,  // watint cycles for next checking
                    long pop_max_check, // bigest checking times when pop_blen_read not met
                    hls::stream<bool>& strm_out_e,
                    hls::stream<ap_uint<512> >& strm_out,
                    ap_uint<512>* p_buff_full,
                    long userSize) {
#pragma HLS INTERFACE m_axi depth = 65536 latency = 32 port = p_buff_full bundle = gmem1 num_read_outstanding = \
    32 num_write_outstanding = 32
    ap_uint<512>* p_buff = p_buff_full + 2;
    popFromMem(pop_blen_read, pop_ii_check, pop_max_check, strm_out_e, strm_out, p_buff, userSize);
}

void hls_memfifo(ap_uint<512>* buff1,
                 ap_uint<512>* buff2,

                 // for both push and popwhich
                 long userSize, // (HBM-PC)/ sizeof(ap_uint<512>) - 2

                 // only for watch dog
                 long cycle2sleep,      // try large number firstly eg. 1,000,000(4ms) -> 1ms
                 long times_sleep2over, // try large number firstly eg. 32 (4ms *32 = 128ms )

                 // only for pop
                 long pop_blen_read, // suggested burst reading lenght, it not met, then waiting pop_ii_check cycles and
                                     // check again, it check times met pop_max_check then read anyway
                 // compariable number of batch size, eg. size_batch/4  but should be small than batch size
                 long pop_ii_check, // watint cycles for next checking . eg. size_batch * 4

                 long pop_max_check, // biggest checking times when pop_blen_read not met. It should not be too big to
                                     // avoid long stay-time. eg.  1~2

                 ap_uint<16> id_broadcast, // ID for reports need to be broadcasted. There might be a post-processing
                                           // module to translate the ID to exact IDs according to ID table
                 ap_uint<16> id_self,      // ID of current kernel
                 ap_uint<8> blen_write,    // burst length for pushing data into memory-fifo

                 hls::stream<ap_uint<512> >& strm_in, // input stream

                 hls::stream<ap_uint<512> >& strm_out,       // output data
                 hls::stream<bool>& strm_out_e,
                 hls::stream<ap_uint<512> >& strm_out_status, // output of status report in which can be heart-beating,
                 hls::stream<bool>& strm_out_status_e
                 ) {
#pragma HLS DATAFLOW
#pragma HLS INTERFACE m_axi depth = 65536 latency = 32 port = buff1 bundle = gmem1 num_read_outstanding = \
    32 num_write_outstanding = 32
#pragma HLS INTERFACE m_axi depth = 65536 latency = 32 port = buff2 bundle = gmem2 num_read_outstanding = \
    32 num_write_outstanding = 32

    hls::stream<ap_uint<512>, 2048> strm_out_wdg("strm_out_wdg");
    hls::stream<ap_uint<8>, 2048> strm_out_token("strm_out_token");

    hls_watchInStrm(id_broadcast, // id for reporting;
                    id_self,      // self id
                    blen_write,   // burst length for writting
                    cycle2sleep,  //
                    times_sleep2over,

                    // input
                    strm_in,

                    // output to pushToMem
                    strm_out_wdg, strm_out_token,

                    // output for reporting, can be connected with net
                    strm_out_status_e, strm_out_status);

    hls_pushToMem(strm_out_token, strm_out_wdg, buff1, userSize);

    hls_popFromMem(pop_blen_read, pop_ii_check, pop_max_check, strm_out_e, strm_out, buff2, userSize);
}
