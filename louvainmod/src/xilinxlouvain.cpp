
#include "xilinxlouvain.h"
#include "ParLV.h"
#include "ctrlLV.h"
#include <vector>
#include <memory>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>


namespace {

using namespace xilinx_apps::louvainmod;

// Max number of vertices that fit on one Alveo card
long NV_par_max = 64*1000*1000;


// Values determined from the global options
//
struct ComputedSettings {
    std::vector<std::string> hostIps;
    int numServers = 1;
    int modeZmq = ZMQ_NONE;
    int numPureWorker = 0;
    std::vector<std::string> nameWorkers;
    unsigned int nodeId = 0;
    
    ComputedSettings(const Options &options) {
        const std::string delimiters(" ");
        const std::string hostIpStr = options.clusterIpAddresses.c_str();
        const std::string hostIpAddress = options.hostIpAddress.c_str();
        for (int i = hostIpStr.find_first_not_of(delimiters, 0); i != std::string::npos;
            i = hostIpStr.find_first_not_of(delimiters, i))
        {
            auto tokenEnd = hostIpStr.find_first_of(delimiters, i);
            if (tokenEnd == std::string::npos)
                tokenEnd = hostIpStr.size();
            const std::string token = hostIpStr.substr(i, tokenEnd - i);
            hostIps.push_back(token);
            if (token == hostIpAddress)
                //TODO: nodeId = hostIps.size();
                nodeId = 0;
            else
                nameWorkers.push_back(std::string("tcp://" + token + ":5555"));
            i = tokenEnd;
        }
        
        numServers = hostIps.size();
        modeZmq = (nodeId == 0) ? ZMQ_DRIVER : ZMQ_WORKER;
        numPureWorker = nameWorkers.size();
    }
};

// Keeps all state for a partition run
//
class PartitionRun {
public:
    const Options &globalOpts_;
    const ComputedSettings &settings_;
    LouvainMod::PartitionOptions partOpts_;
    bool isVerbose_ = false;
    ParLV parlv_;
    std::string projName_;
    std::string projPath_;
    int i_svr_ = 0;  // current server number
    std::vector<int> parInServer_;  // number of partitions for each server
    std::string inputFileName_;  // file name for the source file for the graph, or empty if no file

    PartitionRun(const Options &globalOpts, const ComputedSettings &settings,
        const LouvainMod::PartitionOptions &partOpts)
    : globalOpts_(globalOpts), settings_(settings), partOpts_(partOpts), isVerbose_(globalOpts.verbose)
    {
        int flowMode = 2;
        switch (globalOpts.flow_fast) {
        case 1:
            flowMode = 1;
            break;
        case 2:
            flowMode = 2;
            break;
        case 3:
            flowMode = 3;
            break;
        default:
            {
                std::ostringstream oss;
                oss << "Invalid flow_fast value " << globalOpts.flow_fast << ".  The supported values are 1, 2, and 3.";
                throw Exception(oss.str());
            }
            break;
        }
        
        parlv_.Init(flowMode, nullptr, partOpts.num_par, globalOpts.numDevices, true, partOpts.par_prune);
        parlv_.num_par = partOpts.num_par;
        parlv_.th_prun = partOpts.par_prune;
        parlv_.num_server = settings_.numServers;

        assert(!globalOpts.nameProj.empty());
        //////////////////////////// Set the name for partition project////////////////////////////
        char path_proj[1024];
        char name_proj[256];
        std::strcpy(name_proj, NameNoPath(globalOpts.nameProj));
        PathNoName(path_proj, globalOpts.nameProj);
        projName_ = name_proj;
        projPath_ = path_proj;

#ifdef PRINTINFO
        printf("\033[1;37;40mINFO\033[0m:Partition Project: path = %s name = %s\n", path_proj, name_proj);
#endif

        parlv_.timesPar.timePar_all = getTime();
    }

    ~PartitionRun() {}

    void setFileName(const char *inFileName) { inputFileName_ = inFileName; }

    int getCurServerNum() const { return int(parInServer_.size()); }

    int addPartitionData(const LouvainMod::PartitionData &partitionData) {
        // Determine the prefix string for each partition (.par) file
        //For compatibility, when num_server is 1, no 'srv<n>' surfix used
        char pathName_proj_svr[1024];
        if (settings_.numServers > 1)
            std::sprintf(pathName_proj_svr, "%s_svr%d", globalOpts_.nameProj.c_str(), i_svr_);//louvain_partitions_svr0_000.par
        else
            std::strcpy(pathName_proj_svr, globalOpts_.nameProj);                    //louvain_partitions_000.par

        // If the client has supplied a desired partition size, use that

        long NV_par_recommand = partitionData.NV_par_recommand;

        // No supplied desired partition size: calculate it

        if (NV_par_recommand == 0) {
            // Assume that we want one partition per Alveo card unless overridden by num_par option
            int numPartitionsThisServer = globalOpts_.numDevices;

            // If num_par specifies more partitions than we have total number of devices in the cluster,
            // create num_par devices instead.  This feature is for testing partitioning of smaller graphs,
            // but can also be used to pre-calculate the partitions for graphs that are so large that each
            // Alveo card needs to process more than its maximum number of vertices.
            int totalNumDevices = settings_.numServers * globalOpts_.numDevices;
            if (partOpts_.num_par > totalNumDevices) {
                // Distribute partitions evenly among servers
                numPartitionsThisServer = partOpts_.num_par / settings_.numServers;
                // Distribute the L leftover partitions (where L = servers % partitions) among the first
                // L servers
                int extraPartitions = partOpts_.num_par % settings_.numServers;
                if (extraPartitions > getCurServerNum())
                    ++numPartitionsThisServer;
            }

            // Determine the number of vertices for each partition on this server, which is the lesser of
            // (a) 80% Alveo card capacity and (b) the number of vertices for this server divided by the number of
            // partitions on this server.
            NV_par_recommand = (long)((float)NV_par_max * 0.80);//20% space for ghosts.
            const long numVerticesThisServer = partitionData.end_vertex - partitionData.start_vertex;
            const long numPartitionVertices = (numVerticesThisServer + numPartitionsThisServer - 1) / numPartitionsThisServer;
            if (numPartitionVertices < NV_par_recommand)
                NV_par_recommand = numPartitionVertices;
        }

        int numPartitionsCreated = xai_save_partition(
            const_cast<long *>(partitionData.offsets_tg),
            const_cast<Edge *>(partitionData.edgelist_tg),
            const_cast<long *>(partitionData.drglist_tg),
            partitionData.start_vertex,
            partitionData.end_vertex,
            pathName_proj_svr, // num_server==1? <dir>/louvain_partitions_ : louvain_partitions_svr<num_server>
            partOpts_.par_prune,         // always be '1'
            NV_par_recommand,  // Allow to partition small graphs not bigger than FPGA limitation
            NV_par_max
        );
        if (numPartitionsCreated < 0) {
            std::ostringstream oss;
            oss << "ERROR: Failed to create Alveo partition #" << parInServer_.size() << " for server partition "
                << "start_vertex=" << partitionData.start_vertex << ", end_vertex=" << partitionData.end_vertex << ".";
            throw Exception(oss.str());
        }
        parInServer_.push_back(numPartitionsCreated);
        ++i_svr_;
        return numPartitionsCreated;
    }

    void finishPartitioning() {
        parlv_.st_Partitioned = true;
        parlv_.timesPar.timePar_all = getTime() - parlv_.timesPar.timePar_all;

        //////////////////////////// save <metadata>.par file for loading //////////////////
        // Format: -create_alveo_partitions <inFile> -par_num <par_num> -par_prune <par_prun> -name <ProjectFile>
        char* meta = (char*)malloc(4096);
        char pathName_tmp[1024];
        std::sprintf(pathName_tmp, "%s%s.par.proj", projPath_.c_str(), projName_.c_str());
        std::sprintf(meta, "-create_alveo_partitions %s -par_num %d -par_prune %d -name %s -time_par %f -time_save %f ",
                inputFileName_.c_str(), partOpts_.num_par, partOpts_.par_prune,
                globalOpts_.nameProj.c_str(), parlv_.timesPar.timePar_all, parlv_.timesPar.timePar_save);
        ///////////////////////////////////////////////////////////////////////
        //adding : -server_par <num_server> <num_par on server0> �..<num_par on server?>
        char tmp_str[128];
        std::sprintf(tmp_str, "-server_par %d ", settings_.numServers);
        std::strcat(meta, tmp_str);
        for(int i_svr = 0, end = parInServer_.size(); i_svr < end; i_svr++){
             std::sprintf(tmp_str, "%d ",  parInServer_[i_svr]);
             std::strcat(meta, tmp_str);
        }///////////////////////////////////////////////////////////////////////
        std::strcat(meta, "\n");

        FILE* fp = fopen(pathName_tmp, "w");
        fwrite(meta, sizeof(char), strlen(meta), fp);
        fclose(fp);
    #ifdef PRINTINFO
        printf("\033[1;37;40mINFO\033[0m:Partition Project Meta Data saved in file \033[1;37;40m%s\033[0m\n", pathName_tmp);
        printf(" \t\t Meta Data in file is: %s\n", meta);
    #endif
        std::sprintf(pathName_tmp, "%s%s.par.parlv", projPath_.c_str(), projName_.c_str());
        SaveParLV(pathName_tmp, &parlv_);
        std::sprintf(pathName_tmp, "%s%s.par.src", projPath_.c_str(), projName_.c_str());
        SaveHead<GLVHead>(pathName_tmp, (GLVHead*)parlv_.plv_src);

        if (isVerbose_) {
            printf("************************************************************************************************\n");
            printf("*****************************  Louvain Partition Summary   *************************************\n");
            printf("************************************************************************************************\n");

            std::cout << "Number of servers                  : " << settings_.numServers << std::endl;
            std::cout << "Output Alveo partition project     : " << globalOpts_.nameProj << std::endl;
            std::cout << "Number of partitions               : " << partOpts_.num_par << std::endl;
            printf("Time for partitioning the graph    : %lf = ",
                   (parlv_.timesPar.timePar_all + parlv_.timesPar.timePar_save));
            printf(" partitioning +  saving \n");
            printf("    Time for partition             : %lf (s)\n",
                   parlv_.timesPar.timePar_all);
            printf("    Time for saving                : %lf (s)\n",
                   parlv_.timesPar.timePar_save);
            printf("************************************************************************************************\n");
        }
    #ifdef PRINTINFO
        printf("Deleting... \n");
    #endif
        // CleanTmpGlv appears not to be necessary, as partitioning doesn't set any of the fields cleaned by this function,
        // and those fields are never uninitialized, leaving them filled with garbage.
    //    parlv_.CleanTmpGlv();

        if (isVerbose_)
            parlv_.plv_src->printSimple();
    }
};


} // anonymous namespace

//#####################################################################################################################

namespace xilinx_apps {
namespace louvainmod {

class LouvainModImpl {
public:
    Options options_;  // copy of options passed to LouvainMod constructor
    ComputedSettings settings_;
    std::unique_ptr<PartitionRun> partitionRun_;  // the active or most recent partition run
    
    LouvainModImpl(const Options &options) : options_(options), settings_(options) {}

};

//#####################################################################################################################

//
// LouvainMod Members
//

void LouvainMod::partitionDataFile(const char *fileName, const PartitionOptions &partOptions) {
//        create_alveo_partitions(options.opts_inFile, options.num_par, options.par_prune, options.nameProj, parlv);

    assert(fileName);
    int id_glv = 0;
    GLV* glv_src = CreateByFile_general(const_cast<char *>(fileName), id_glv);  // Louvain code not const correct
    if (glv_src == NULL)
        throw Exception("Unable to read data file");  // TODO: better error message
    const long NV = glv_src->NV;

    startPartitioning(partOptions);
    pImpl_->partitionRun_->setFileName(fileName);
    ParLV &parlv = pImpl_->partitionRun_->parlv_;
    parlv.plv_src = glv_src;

    //////////////////////////// partition ////////////////////////////

    const int num_server = pImpl_->settings_.numServers;

    //For simulation: create server-level partition
    std::vector<long> start_vertex(num_server);
    std::vector<long> end_vertex(num_server);
    std::vector<long> vInServer(num_server);
    for(int i_svr=0; i_svr<num_server; i_svr++) {
        start_vertex[i_svr] =  i_svr* (NV / num_server);
        if(i_svr!=num_server-1)
            end_vertex[i_svr] = start_vertex[i_svr] + NV / num_server;
        else
            end_vertex[i_svr] = NV;
        vInServer[i_svr] = end_vertex[i_svr] - start_vertex[i_svr];
    }

    //For simulation: create server-level partition
    int num_partition = partOptions.num_par;
    int start_par[MAX_PARTITION];// eg. {0, 3, 6} when par_num == 9
    int end_par[MAX_PARTITION];  // eg. {3, 6, 9}  when par_num == 9
    int parInServer[MAX_PARTITION];//eg. {3, 3, 3} when par_num == 9 and num_server==3
    for(int i_svr=0; i_svr<num_server; i_svr++){
        start_par[i_svr] = i_svr * (num_partition / num_server);
        if(i_svr!=num_server-1)
            end_par[i_svr] = start_par[i_svr] + (num_partition / num_server);
        else
            end_par[i_svr] = num_partition;
        parInServer[i_svr] = end_par[i_svr] - start_par[i_svr];
    }

    long* offsets_glb  = glv_src->G->edgeListPtrs;
    num_partition = 0;

    for (int i_svr = 0; i_svr < num_server; i_svr++) {
        //For simulation: To get partition on server(i_svr)
        long* offsets_tg   = (long*) malloc( sizeof(long) * (vInServer[i_svr] + 1) );
        edge* edgelist_tg  = (edge*) malloc( sizeof(edge) * (offsets_glb[end_vertex[i_svr]] - offsets_glb[start_vertex[i_svr]]) );
        long* drglist_tg   = (long*) malloc( sizeof(long) * (offsets_glb[end_vertex[i_svr]] - offsets_glb[start_vertex[i_svr]]) );

        sim_getServerPar(// This function should be repleased by GSQL
                glv_src->G, start_vertex[i_svr], end_vertex[i_svr],
                offsets_tg, edgelist_tg, drglist_tg);
#ifdef PRINTINFO
                printf("DBG:: SERVER(%d): start_vertex=%d, end_vertex=%d, NV_tg=%d, start_par=%d, parInServer=%d, pathName:%s\n",
        i_svr, start_vertex[i_svr], end_vertex[i_svr], vInServer[i_svr], start_par[i_svr], parInServer[i_svr], pathName_proj);
#endif

        long NV_par_recommand = 0;
        if (partOptions.num_par>1)
            NV_par_recommand = (NV + partOptions.num_par-1) / partOptions.num_par;//allow to partition small graph with -par_num

        LouvainMod::PartitionData partitionData;
        partitionData.offsets_tg = offsets_tg;
        partitionData.edgelist_tg = edgelist_tg;
        partitionData.drglist_tg = drglist_tg;
        partitionData.start_vertex = start_vertex[i_svr];
        partitionData.end_vertex = end_vertex[i_svr];
        partitionData.NV_par_recommand = NV_par_recommand;
        parInServer[i_svr] = addPartitionData(partitionData);

        num_partition +=parInServer[i_svr] ;
        free(offsets_tg);
        free(edgelist_tg);
        free(drglist_tg);
    }

    finishPartitioning();
}


void LouvainMod::startPartitioning(const PartitionOptions &partOpts) {
    pImpl_->partitionRun_.reset(new PartitionRun(pImpl_->options_, pImpl_->settings_, partOpts));
}


int LouvainMod::addPartitionData(const PartitionData &partitionData) {
    return pImpl_->partitionRun_->addPartitionData(partitionData);
}


void LouvainMod::finishPartitioning() {
    pImpl_->partitionRun_->finishPartitioning();
}

void LouvainMod::setAlveoProject(const char* alveoProject) { pImpl_->options_.alveoProject = alveoProject; }

void LouvainMod::loadAlveo() {}
void LouvainMod::computeLouvain(const ComputeOptions &computeOpts) {}

float LouvainMod::loadAlveoAndComputeLouvain(const ComputeOptions &computeOpts)
{
    float finalQ;
    char* nameWorkers[128];

    std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
    
    int i = 0;
    //for (auto it = pImpl_->settings_.nameWorkers.begin(); it != pImpl_->settings_.nameWorkers.end(); ++it){
    //    nameWorkers[i++] = (char *)it->c_str();
    //}
  
#ifndef NDEBUG  
    std::cout << "DEBUG: " << __FUNCTION__ 
              << "\n    xclbinPath=" << pImpl_->options_.xclbinPath
              << "\n    alveoProject=" << pImpl_->options_.alveoProject 
              << "\n    nodeId=" << pImpl_->settings_.nodeId
              << "\n    modeZmq=" << pImpl_->settings_.modeZmq
              << "\n    numPureWorker=" << pImpl_->settings_.numPureWorker
              << std::endl;
#endif              
    finalQ = ::loadAlveoAndComputeLouvain(
                (char *)(pImpl_->options_.xclbinPath.c_str()), 
                pImpl_->options_.flow_fast, 
                pImpl_->options_.numDevices, 
                (char*)(pImpl_->options_.alveoProject.c_str()),
                pImpl_->settings_.modeZmq, 
                pImpl_->settings_.numPureWorker, 
                nameWorkers, 
                pImpl_->settings_.nodeId, 
                (char *)(computeOpts.outputFile.c_str()), 
                computeOpts.max_iter, computeOpts.max_level, 
                computeOpts.tolerance, computeOpts.intermediateResult, 
                pImpl_->options_.verbose, computeOpts.final_Q, computeOpts.all_Q); 
    
    std::cout << "DEBUG: " << __FUNCTION__ << " finalQ=" << finalQ << std::endl;
    return finalQ;
}

}  // namespace louvainmod
}  // namespace xilinx_apps


//#####################################################################################################################

//
// Shared Library Entry Points
//

extern "C" {

xilinx_apps::louvainmod::LouvainModImpl *xilinx_louvainmod_createImpl(const xilinx_apps::louvainmod::Options& options) {
    return new xilinx_apps::louvainmod::LouvainModImpl(options);
}

void xilinx_louvainmod_destroyImpl(xilinx_apps::louvainmod::LouvainModImpl *pImpl) {
    delete pImpl;
}

}  // extern "C"
