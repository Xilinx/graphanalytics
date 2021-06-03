#ifndef _PARLV_H_
#define _PARLV_H_
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "xilinxlouvain.h"
#include "xilinxlouvainInternal.h"
#include "partitionLouvain.hpp"
#include "xf_graph_L3.hpp"
#include "zmq.h"
#define MAX_PARTITION (512)
#define MAX_DEVICE (64)

const long headGLVBin = 0xffff5555ffff5555;

struct TimePartition {
	double time_star;
	double time_done_par;
	double time_done_lv;
	double time_done_pre;
	double time_done_mg;
	double time_done_fnl;
	//
	double timePar   [MAX_PARTITION];
	double timePar_all;
	double timePar_save;
	double timeLv    [MAX_PARTITION];
	double timeLv_dev[MAX_DEVICE];
	double timeLv_all;
	double timeWrkSend[MAX_DEVICE];
	double timeWrkLoad[MAX_DEVICE];
	double timeWrkCompute[MAX_DEVICE];//the compute time on each nodes(May have multi cards)
	double timeDriverSend;
	double timeDriverLoad;
	double timeDriverExecute;
	double timeDriverCollect;
	double timeDriverRecv;
	double timePre;
	double timeMerge;
	double timeFinal;
	double timeAll;

};

struct ParLVVar{
	bool st_Partitioned;
	bool st_ParLved;
	bool st_PreMerged;
	bool st_Merged;
	bool st_FinalLved;
	bool st_Merged_ll;
	bool st_Merged_gh;
	bool isMergeGhost;
	bool isOnlyGL;
	int   flowMode;
    int   num_par;
    int   num_dev;
    long   NV;
    long   NVl;
    long   NV_gh;
    long   NE;
    long   NEll;
    long   NElg;
    long   NEgl;
    long   NEgg;
    long   NEself;
	long   off_src[MAX_PARTITION];
	long   off_lved[MAX_PARTITION];
    long   max_NV;
    long   max_NE;
    long   max_NVl;
    long   max_NElg;
    int    scl_NV;
    int    scl_NE;
    int    scl_NVl;
    int    scl_NElg;
    long  NE_list_all;
    long  NE_list_ll;
    long  NE_list_gl;
    long  NE_list_gg;
    long  NV_list_all;
    long  NV_list_l;
    long  NV_list_g;
    bool  isPrun;
	int   th_prun;
};

class ParLV{
public:
	bool st_Partitioned;
	bool st_ParLved;
	bool st_PreMerged;
	bool st_Merged;
	bool st_FinalLved;
	bool st_Merged_ll;
	bool st_Merged_gh;
	bool isMergeGhost;
	bool isOnlyGL;
	int   flowMode;
    int   num_par;
    int   num_dev;
    long   NV;
    long   NVl;
    long   NV_gh;
    long   NE;
    long   NEll;
    long   NElg;
    long   NEgl;
    long   NEgg;
    long   NEself;
	long   off_src[MAX_PARTITION];
	long   off_lved[MAX_PARTITION];
    long   max_NV;
    long   max_NE;
    long   max_NVl;
    long   max_NElg;
    int    scl_NV;
    int    scl_NE;
    int    scl_NVl;
    int    scl_NElg;
    long  NE_list_all;
    long  NE_list_ll;
    long  NE_list_gl;
    long  NE_list_gg;
    long  NV_list_all;
    long  NV_list_l;
    long  NV_list_g;
    bool  isPrun;
	int   th_prun;
	///////////////////////////////////
	int num_server;// default '1' means using concentration partition
	int parInServer[MAX_PARTITION];
	long parOffsets[MAX_PARTITION];//start vertex for each partition. Currently no use for it. Just recored in .par.proj file
	///////////////////////////////////
	GLV* plv_src;
	GLV* par_src[MAX_PARTITION];
	GLV* par_lved[MAX_PARTITION];
	GLV* plv_merged;
	GLV* plv_final;
	long* p_v_new[MAX_PARTITION];
	SttGPar stt[MAX_PARTITION];
	TimePartition timesPar;
	map<long, long> m_v_gh;
	//list<GLV*> lv_list;

    edge* elist;
    long* M_v;

    void Init(int mode);
    void Init(int mode, GLV* src, int numpar, int numdev);
    void Init(int mode, GLV* src, int num_p, int num_d, bool isPrun, int th_prun);
    ParLV();
    ~ParLV();
    void PrintSelf();
    void UpdateTimeAll();
    void CleanList(GLV* glv_curr, GLV* glv_temp);
    GLV* MergingPar2(int& );
    GLV* FinalLouvain(char*, int , int& , long minGraphSize, double threshold, double C_threshold, bool isParallel, int numPhase);

    long MergingPar2_ll();
    long MergingPar2_gh();
    long CheckGhost();
    int  partition(GLV* glv_src, int& id_glv, int num, long th_size, int th_maxGhost);
    void PreMerge();
    void Addedge(edge* edges, long head, long tail, double weight, long* M_g);
    long FindGhostInLocalC(long m);
    int  FindParIdx(long e_org);
    int  FindParIdxByID(int id);
    pair<long, long> FindCM_1hop(int idx, long e_org);
    pair<long, long>FindCM_1hop(long e_org);
    long FindC_nhop(long m_gh);
    int  AddGLV(GLV* plv);
    void PrintTime();
    void PrintTime2();
    void CleanTmpGlv();
    double TimeStar();
    double TimeDonePar();
    double TimeDoneLv();
    double TimeDonePre();
    double TimeDoneMerge();
    double TimeDoneFinal();
    double TimeAll_Done();
};

struct ToolOptions {
    int argc;
    char** argv;  // strings not owned!
    
    double opts_C_thresh;   //; //Threshold with coloring on
    long opts_minGraphSize; //; //Min |V| to enable coloring
    double opts_threshold;  //; //Value of threshold
    int opts_ftype;         //; //File type
    char opts_inFile[4096];  //;
    bool opts_coloring;     //
    bool opts_output;       //;
    char opts_outputFile[4096];
    bool opts_VF; //;
    char opts_xclbinPath[4096];
    int numThreads;
    int num_par;
    int gh_par;  // same as par_prune
    bool flow_fast;
    int devNeed;
    int mode_zmq;
    char path_zmq[4096];
    bool useCmd;
    int mode_alveo;
    char nameProj[4096];
    char nameMetaFile[4096];
    int numPureWorker;
    char *nameWorkers[128];
    int nodeID;
    int server_par;
    int max_num_level;
    int max_num_iter;
    
    ToolOptions(int argc, char **argv);
};

GLV* par_general(GLV* src, SttGPar* pstt, int&id_glv, long start, long end, bool isPrun, int th_prun);
GLV* par_general(GLV* src, int&id_glv, long start, long end, bool isPrun,  int th_prun);

GLV* LouvainGLV_general_par(
		int flowMode,
		ParLV &parlv,
		//
		char* xclbinPath, int numThreads, int& id_glv,
		long minGraphSize, double threshold, double C_threshold, bool isParallel, int numPhase);
GLV* LouvainGLV_general_par_OneDev(
		int flowMode,
		ParLV &parlv,
		//
		char* xclbinPath, int numThreads, int& id_glv,
		long minGraphSize, double threshold, double C_threshold, bool isParallel, int numPhase);

GLV* LouvainGLV_general_par(
		int flowMode,
		GLV* glv_orig, int num_par, int num_dev, int isPrun, int th_prun,
		//
		char* xclbinPath, int numThreads, int& id_glv,
		long minGraphSize, double threshold, double C_threshold, bool isParallel, int numPhase);

void ParLV_general_batch_thread(
		int flowMode, GLV* plv_orig,
		int id_dev, int num_dev, int num_par,
		double* timeLv, GLV* par_src[], GLV* par_lved[],
		char* xclbinPath, int numThreads,
		long minGraphSize, double threshold, double C_threshold, bool isParallel, int numPhase);

GLV* LouvainGLV_general_top(xf::graph::L3::Handle* handle0,
                            ParLV& parlv,
                            int& id_glv,
                            bool opts_coloring,
                            long opts_minGraphSize,
                            double opts_threshold,
                            double opts_C_thresh,
                            int numThreads);

GLV* CreateByFile_general(char* inFile, int& id_glv);

int SaveGLVBin(char* name, GLV* glv);

double getTime();

int xai_save_partition(long* offsets_tg, edge* edgelist_tg, long* drglist_tg,
		long  start_vertex,     // If a vertex is smaller than star_vertex, it is a ghost
		long  end_vertex,	    // If a vertex is larger than star_vertex-1, it is a ghost
		char* path_prefix,      // For saving the partition files like <path_prefix>_xxx.par
							    // Different server can have different path_prefix
		int par_prune,          // Can always be set with value '1'
		long NV_par_recommand,  // Allow to partition small graphs not bigger than FPGA limitation
		long NV_par_max		    //  64*1000*1000;
		);

void SaveParLV(char* name, ParLV* p_parlv);

void sim_getServerPar(
  //input
  graphNew* G,   //Looks like a Global Graph but here only access dataset within
  long start_vertex, // a range from start_vertex to end_vertex, which is stored locally
  long end_vertex,   // Here we assume that the vertices of a TigerGraph partition
                 // stored on a node are continuous
  //Output
  long* offsets_tg, // we can also use �degree� instead of �offsets�
  edge* edges_tg,   //
  long* dgr_tail_tg // degrees for the tail of each edge;
);

#endif
