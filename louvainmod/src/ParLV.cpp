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
#include "defs.h"
#include "ParLV.h"
#include "partitionLouvain.hpp"
#include "louvainPhase.h"
#include "ctrlLV.h"
#include <thread>
#include <chrono>
#include "zmq/driver-worker/node.hpp"
#include "zmq/driver-worker/worker.hpp"
#include "zmq/driver-worker/driver.hpp"

using namespace std;

//extern int glb_max_num_level;
//extern int glb_max_num_iter;
int glb_max_num_level = MAX_NUM_PHASE;
int glb_max_num_iter = MAX_NUM_TOTITR;
// time functions

typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePointType;

double getTime() {
    TimePointType t1 = chrono::high_resolution_clock::now();
    chrono::duration<double> l_durationSec = chrono::duration<double>(t1.time_since_epoch());
    return l_durationSec.count();
}

void getDiffTime(TimePointType& t1, TimePointType& t2, double& time) {
    t2 = chrono::high_resolution_clock::now();
    chrono::duration<double> l_durationSec = t2 - t1;
    time = l_durationSec.count();
}

// time functions end

// functions for tranmitting data via zmq
int sendGLV(const long headGLVBin, Node* worker_node, GLV* glv) {
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long ne = g->numEdges;
    long ne_undir = g->edgeListPtrs[nv];
    long nc = glv->NC;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    double Q = glv->Q;
    worker_node->send(headGLVBin);
    worker_node->send(nv);
    worker_node->send(ne);
    worker_node->send(ne_undir);
    worker_node->send(nc);
    worker_node->send(Q);
    worker_node->send(nvl);
    worker_node->send(nelg);
    worker_node->send(g->edgeListPtrs, sizeof(long) * (nv + 1));
    worker_node->send(g->edgeList, sizeof(edge) * ne_undir);
    worker_node->send(glv->M, sizeof(long) * nv);
    worker_node->send(glv->C, sizeof(long) * nv);
#ifdef PRINTINFO
    printf("INFO: sendGLV Successfully nv=%ld ne=%ld undir ne=%ld nc=%ld Q=%lf \n", nv, ne, ne_undir, nc, Q);
#endif
    return 0;
}

int sendGLV_OnlyC(const long headGLVBin, Node* worker_node, GLV* glv) {
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    worker_node->send(headGLVBin);
    worker_node->send(nv);
    worker_node->send(nc);
    worker_node->send(Q);
    worker_node->send(nvl);
    worker_node->send(nelg);
    worker_node->send(glv->C, sizeof(long) * nv);
    worker_node->send(glv->times, sizeof(TimeLv));
#ifdef PRINTINFO
    printf("INFO: sendGLV_OnlyC  nv=%ld nc=%ld Q=%lf Successfully \n", nv, nc, Q);
#endif
    return 0;
}

int receiveGLV_OnlyC(Node* driver_node, GLV* glv) {
    long nv = 0, nc = 0, head = 0;
    double Q = -1;
    long nvl = 0, nelg = 0;
    driver_node->receive(head);
    driver_node->receive(nv);
    driver_node->receive(nc);
    driver_node->receive(Q);
    driver_node->receive(nvl);
    driver_node->receive(nelg);
    glv->C = (long*)malloc(sizeof(long) * nv);
    driver_node->receive(glv->C, sizeof(long) * nv);
    driver_node->receive(glv->times, sizeof(TimeLv));
    glv->NV = nv;
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;
    glv->NElg = nelg;
#ifdef PRINTINFO
    printf("INFO: receiveGLV_OnlyC nv=%ld nc=%ld Q=%lf Successfully \n", nv, nc, Q);
#endif
    return 0;
}

GLV* receiveGLV(Node* driver_node, int& id_glv) {
    graphNew* g = (graphNew*)malloc(sizeof(graphNew));
    long nv = 0, ne = 0, ne_undir = 0, nc = 0, head = 0;
    double Q = -1;
    long nvl = 0, nelg = 0;

    driver_node->receive(head);
    driver_node->receive(nv);
    driver_node->receive(ne);
    driver_node->receive(ne_undir);
    driver_node->receive(nc);
    driver_node->receive(Q);
    driver_node->receive(nvl);
    driver_node->receive(nelg);
    g->numEdges = ne;
    g->numVertices = nv;
#ifdef PRINTINFO
    printf("INFO: receiveGLV nv=%ld ne=%ld undir ne=%ld nc=%ld Q=%lf \n", nv, ne, ne_undir, nc, Q);
#endif
    g->edgeListPtrs = (long*)malloc(sizeof(long) * (nv + 1));
    g->edgeList = (edge*)malloc(sizeof(edge) * ne_undir);
    long* M = (long*)malloc(sizeof(long) * nv);
    assert(g->edgeListPtrs);
    assert(g->edgeList);
    assert(M);
    driver_node->receive(g->edgeListPtrs, sizeof(long) * (nv + 1));
    driver_node->receive(g->edgeList, sizeof(edge) * ne_undir);
    driver_node->receive(M, sizeof(long) * nv);
    GLV* glv = new GLV(id_glv);
    glv->SetByOhterG(g, M);
    driver_node->receive(glv->C, sizeof(long) * nv);
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;
    glv->NElg = nelg;
    return glv;
}
// functions for tranmitting data via zmq end

// generator/parser for messages sent/received through zmq

#define MAX_LEN_MESSAGE (4096)

int MessageGen_W2D(char* msg, int nodeID) { // Format:  parlv_req -num <> -path <> [name]*
    assert(msg);
    sprintf(msg, "Ready -node %d \n", nodeID);
    int len = 20;
#ifdef PRINTINFO
    printf("MESSAGE W2D parlv_done: %s", msg);
#endif
    return 0;
}

int MessageGen_W2D(char* msg, ParLV& parlv, int nodeID) { // Format:  parlv_req -num <> -path <> [name]*
    assert(msg);
    sprintf(msg, "parlv_done -num %d -node %d -comp %lf -io %lf %lf", parlv.num_par, nodeID,
            parlv.timesPar.timeWrkCompute[0], parlv.timesPar.timeWrkLoad[0], parlv.timesPar.timeWrkSend[0]);
    strcat(msg, "\n");
#ifdef PRINTINFO
    printf("MESSAGE W2D parlv_done: %s", msg);
#endif
    return 0;
}

int MessageParser_W2D(char* msg) {
    assert(msg);
    myCmd ps;
    ps.cmd_Getline(msg);
    if (ps.argc < 1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: argc(%d) <1\n", ps.argc);
        return -1;
    }
    if (strcmp("Ready", ps.argv[0]) != 0) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Unknow command%s\n", ps.argv[0]);
        return -1;
    }

    int id_node = ps.cmd_findPara("-node");
    if (id_node == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Can't find parameter -node\n");
        return -1;
    }
    int nodeID = atoi(ps.argv[id_node + 1]);
    return nodeID;
}

int MessageParser_W2D(int id_wkr, char* msg, ParLV& parlv) {
    assert(msg);
    myCmd ps;
    ps.cmd_Getline(msg);
    if (ps.argc < 6) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: argc(%d) <6\n", ps.argc);
        return -1;
    }

    if (strcmp("parlv_done", ps.argv[0]) != 0) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: Unknow command%s\n", ps.argv[0]);
        return -1;
    }

    int id_num_par = ps.cmd_findPara("-num");
    if (id_num_par == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: Can't find parameter -num\n");
        return -1;
    }
    int num_par_sub = atoi(ps.argv[id_num_par + 1]);

    int id_node = ps.cmd_findPara("-node");
    if (id_node == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: Can't find parameter -node\n");
        return -1;
    }
    int nodeID = id_wkr; // atoi(ps.argv[id_node+1]);

    int id_timeCompute = ps.cmd_findPara("-comp");
    if (id_timeCompute == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: Can't find parameter -comp\n");
        return -1;
    }
    double tmp = atof(ps.argv[id_timeCompute + 1]);
#ifdef PRINTINFO
    printf("INFO : parlv.timesPar.timeWrkCompute : %lf\n", tmp);
#endif
    parlv.timesPar.timeWrkCompute[nodeID] = tmp;

    int id_timeIO = ps.cmd_findPara("-io");
    if (id_timeIO == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_W2D: Can't find parameter -io\n");
        return -1;
    }
    tmp = atof(ps.argv[id_timeIO + 1]);
#ifdef PRINTINFO
    printf("INFO : parlv.timesPar.timeWrkLoad : %lf\n", tmp);
#endif
    parlv.timesPar.timeWrkLoad[nodeID] = tmp;
    tmp = atof(ps.argv[id_timeIO + 2]);
    parlv.timesPar.timeWrkSend[nodeID] = tmp;
    return num_par_sub;
}

int MessageGen_D2W(char* msg, int nodeID) { // Format:  parlv_req -num <> -path <> [name]*
    assert(msg);
    sprintf(msg, "Start -node %d \n", nodeID);
    int len = 30;
#ifdef PRINTINFO
    printf("MESSAGE D2W parlv_req: %s", msg);
#endif
    return 0;
}
int MessageParser_D2W(char* msg) {
    assert(msg);
    myCmd ps;
    ps.cmd_Getline(msg);
    if (ps.argc < 1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: argc(%d) <1\n", ps.argc);
        return -1;
    }
    if (strcmp("Start", ps.argv[0]) != 0) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Unknow command%s\n", ps.argv[0]);
        return -1;
    }

    int id_node = ps.cmd_findPara("-node");
    if (id_node == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Can't find parameter -node\n");
        return -1;
    }
    int nodeID = atoi(ps.argv[id_node + 1]);
    return 0;
}

int MessageGen_D2W(char* msg, ParLV& parlv, char* path, int start, int end, int nodeID) 
{ 
    // Format:  parlv_req -num <> -path <> [name]*
    assert(msg);
    assert(path);
    assert(start >= 0 && start < parlv.num_par);
    assert(end > start && end <= parlv.num_par);
    sprintf(msg, "parlv_req -num %d -path %s ", (end - start), path);
    int len = 30;
    len += strlen(path);
    for (int p = start; p < end; p++) {
        len += (strlen(parlv.par_src[p]->name) + 1);
        if (len > MAX_LEN_MESSAGE) {
            printf("\033[1;31;40mERROR\033[0m length (%d) >MAX_LEN_MESSAGE (%d) ", len, MAX_LEN_MESSAGE);
            return -1;
        }
        strcat(msg, " ");
        strcat(msg, parlv.par_src[p]->name);
    }
    char tmp[10];
    sprintf(tmp, " -node %d ", nodeID);
    strcat(msg, tmp);
    strcat(msg, "\n");
#ifdef PRINTINFO
    printf("MESSAGE D2W parlv_req: %s", msg);
#endif
    return 0;
}

/*
  * Set number of partions (parlv.num_par) as specified in -num option in msg
*/
int MessageParser_D2W(char* msg, ParLV& parlv, char* path_driver, char names[][256], int& nodeID) 
{
    assert(msg);
    myCmd ps;
    ps.cmd_Getline(msg);
    if (ps.argc < 6) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: argc(%d) <6\n", ps.argc);
        return -1;
    }
    if (strcmp("parlv_req", ps.argv[0]) != 0) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Unknow command%s\n", ps.argv[0]);
        return -1;
    }

    int id_num_par = ps.cmd_findPara("-num");
    if (id_num_par == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Can't find parameter -num\n");
        return -1;
    }
    parlv.num_par = atoi(ps.argv[id_num_par + 1]);

    int id_path = ps.cmd_findPara("-path");
    if (id_path == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Can't find parameter -path\n");
        return -1;
    }
    strcpy(path_driver, ps.argv[id_path + 1]);
    for (int p = 0; p < parlv.num_par; p++) {
        int id_name = id_path + 2 + p;
        if (id_name >= ps.argc) {
            printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: number of files in message is not match\n");
            return -1;
        }
        strcpy(names[p], ps.argv[id_name]);
    }

    int id_node = ps.cmd_findPara("-node");
    if (id_node == -1) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Can't find parameter -node\n");
        return -1;
    }
    nodeID = atoi(ps.argv[id_node + 1]);
    return 0;
}

// generator/parser for messages sent/received through zmq end
// load/send GLV through data files
int SaveGLVBin(char* name, GLV* glv) {
    assert(name);
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long ne = g->numEdges;
    long ne_undir = g->edgeListPtrs[nv];
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    FILE* fp = fopen(name, "wb");
    if (fp == NULL) {
        printf("ERROR: SaveGLVBin failed to open %s \n", name);
        return -1;
    }
    fwrite(&headGLVBin, sizeof(long), 1, fp);
    fwrite(&nv, sizeof(long), 1, fp);
    fwrite(&ne, sizeof(long), 1, fp);
    fwrite(&ne_undir, sizeof(long), 1, fp);
    fwrite(&nc, sizeof(long), 1, fp);
    fwrite(&Q, sizeof(double), 1, fp);
    fwrite(&nvl, sizeof(long), 1, fp);
    fwrite(&nelg, sizeof(long), 1, fp);
    fwrite(g->edgeListPtrs, sizeof(long), nv + 1, fp);
    fwrite(g->edgeList, sizeof(edge), ne_undir, fp);
    fwrite(glv->M, sizeof(long), nv, fp);
    fwrite(glv->C, sizeof(long), nv, fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: SaveGLVBin %s Successfully nv=%ld ne=%ld undir ne=%ld nc=%ld Q=%lf \n", name, nv, ne, ne_undir, nc,
           Q);
#endif

    return 0;
}

long UseInt(long nv, long* src, FILE* fp) {
    int* tmp = (int*)malloc(sizeof(int) * nv);
    for (int i = 0; i < nv; i++) tmp[i] = src[i];
    long ret = fwrite(tmp, sizeof(int), nv, fp);
    free(tmp);

    return ret;
}

int SaveGLVBin(char* name, GLV* glv, bool useInt) {
    assert(name);
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long ne = g->numEdges;
    long ne_undir = g->edgeListPtrs[nv];
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    FILE* fp = fopen(name, "wb");
    if (fp == NULL) {
        printf("ERROR: SaveGLVBin failed to open %s \n", name);
        return -1;
    }
    fwrite(&headGLVBin, sizeof(long), 1, fp);
    fwrite(&nv, sizeof(long), 1, fp);
    fwrite(&ne, sizeof(long), 1, fp);
    fwrite(&ne_undir, sizeof(long), 1, fp);
    fwrite(&nc, sizeof(long), 1, fp);
    fwrite(&Q, sizeof(double), 1, fp);
    fwrite(&nvl, sizeof(long), 1, fp);
    fwrite(&nelg, sizeof(long), 1, fp);
    fwrite(g->edgeListPtrs, sizeof(long), nv + 1, fp);
    fwrite(g->edgeList, sizeof(edge), ne_undir, fp);
    fwrite(glv->M, sizeof(long), nv, fp);
    fwrite(glv->C, sizeof(long), nv, fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: SaveGLVBin %s Successfully nv=%ld ne=%ld undir ne=%ld nc=%ld Q=%lf \n", name, nv, ne, ne_undir, nc,
           Q);
#endif
    return 0;
}
int SaveGLVBin_OnlyC(char* name, GLV* glv, bool useInt) {
    assert(name);
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    FILE* fp = fopen(name, "wb");
    if (fp == NULL) {
        printf("ERROR: SaveGLVBin_OnlyC failed for open %s \n", name);
        return -1;
    }
    fwrite(&headGLVBin, sizeof(long), 1, fp);
    fwrite(&nv, sizeof(long), 1, fp);
    fwrite(&nc, sizeof(long), 1, fp);
    fwrite(&Q, sizeof(double), 1, fp);
    fwrite(&nvl, sizeof(long), 1, fp);
    fwrite(&nelg, sizeof(long), 1, fp);
    if (useInt) {
        int* tmp = (int*)malloc(sizeof(int) * nv);
        for (int i = 0; i < nv; i++) tmp[i] = glv->C[i];
        fwrite(tmp, sizeof(int), nv, fp);
        free(tmp);
    } else
        fwrite(glv->C, sizeof(long), nv, fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: SaveGLVBin_OnlyC %s nv=%ld nc=%ld Q=%lf Successfully \n", name, nv, nc, Q);
#endif
    return 0;
}
int SaveGLVBin_OnlyC(char* name, GLV* glv) {
    assert(name);
    assert(glv);
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    FILE* fp = fopen(name, "wb");
    if (fp == NULL) {
        printf("ERROR: SaveGLVBin_OnlyC failed for open %s \n", name);
        return -1;
    }
    fwrite(&headGLVBin, sizeof(long), 1, fp);
    fwrite(&nv, sizeof(long), 1, fp);
    fwrite(&nc, sizeof(long), 1, fp);
    fwrite(&Q, sizeof(double), 1, fp);
    fwrite(&nvl, sizeof(long), 1, fp);
    fwrite(&nelg, sizeof(long), 1, fp);
    fwrite(glv->C, sizeof(long), nv, fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: SaveGLVBin_OnlyC %s nv=%ld nc=%ld Q=%lf Successfully \n", name, nv, nc, Q);
#endif
    return 0;
}

int SaveGLVBinBatch(GLV* glv[], int num_par, const char* path, bool useInt) {
    assert(glv);
    assert(num_par < MAX_PARTITION);
    int ret = 0;
    for (int i = 0; i < num_par; i++) {
        char pathName[1024];
        if (strlen(path)) {
            strcpy(pathName, path);
            strcat(pathName, "/");
        } else
            strcpy(pathName, "./");
        strcat(pathName, glv[i]->name);
        ret += SaveGLVBin(pathName, glv[i], useInt);
    }
    return ret;
}

int SaveGLVBinBatch_OnlyC(GLV* glv[], int num_par, const char* path) {
    assert(glv);
    assert(num_par < MAX_PARTITION);
    int ret = 0;
    for (int i = 0; i < num_par; i++) {
        char pathName[1024];
        if (strlen(path)) {
            strcpy(pathName, path);
            strcat(pathName, "/");
        } else
            strcpy(pathName, "./");
        strcat(pathName, glv[i]->name);
        ret += SaveGLVBin_OnlyC(pathName, glv[i]);
    }
    return ret;
}
int LoadGLVBin_OnlyC(char* name, GLV* glv, bool useInt) {
    assert(name);

    long nv = 0;
    long nc = 0;
    long head = 0;
    double Q = -1;
    long nvl;  //= glv->NVl;
    long nelg; //= glv->NElg;

    FILE* fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("ERROR: LoadGLVBin failed for open %s \n", name);
        return -1;
    }
    fread(&head, sizeof(long), 1, fp);
    if (head != headGLVBin) {
        printf("ERROR: head(%ld)!=headGLVBin(%ld) \n", head, headGLVBin);
        fclose(fp);
        return -1;
    }
    fread(&nv, sizeof(long), 1, fp);

    fread(&nc, sizeof(long), 1, fp);
    if (nc < 0) {
        printf("ERROR: value(%ld) of NC is not right!!!  \n", nc);
        fclose(fp);
        return -1;
    }
    fread(&Q, sizeof(double), 1, fp);
    if (Q > 1) {
        printf("ERROR: value(%lf) of Q is not right!!!  \n", Q);
        fclose(fp);
        return -1;
    }
    fread(&nvl, sizeof(long), 1, fp);
    fread(&nelg, sizeof(long), 1, fp);
    if (useInt) {
        int* tmp = (int*)malloc(sizeof(int) * nv);
        fread(tmp, sizeof(int), nv, fp);
        for (int i = 0; i < nv; i++) glv->C[i] = tmp[i];
        free(tmp);
    } else
        fread(glv->C, sizeof(long), nv, fp);
    glv->NV = nv;
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;   //= glv->NVl;
    glv->NElg = nelg; //= glv->NElg;
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: LoadGLVBin_OnlyC %s nv=%ld nc=%ld Q=%lf Successfully \n", name, nv, nc, Q);
#endif
    return 0;
}

int LoadGLVBin_OnlyC(char* name, GLV* glv) {
    assert(name);

    long nv = 0;
    long nc = 0;
    long head = 0;
    double Q = -1;
    long nvl;  //= glv->NVl;
    long nelg; //= glv->NElg;

    FILE* fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("ERROR: LoadGLVBin failed for open %s \n", name);
        return -1;
    }
    fread(&head, sizeof(long), 1, fp);
    if (head != headGLVBin) {
        printf("ERROR: head(%ld)!=headGLVBin(%ld) \n", head, headGLVBin);
        fclose(fp);
        return -1;
    }
    fread(&nv, sizeof(long), 1, fp);
    /*if(nv!=glv->NV){
            printf("ERROR: NV is not same %ld != %ld !!!  \n", nv, glv->NV);
            fclose(fp);
            return -1;
    }*/
    fread(&nc, sizeof(long), 1, fp);
    if (nc < 0) {
        printf("ERROR: value(%ld) of NC is not right!!!  \n", nc);
        fclose(fp);
        return -1;
    }
    fread(&Q, sizeof(double), 1, fp);
    if (Q > 1) {
        printf("ERROR: value(%lf) of Q is not right!!!  \n", Q);
        fclose(fp);
        return -1;
    }
    fread(&nvl, sizeof(long), 1, fp);
    fread(&nelg, sizeof(long), 1, fp);
    fread(glv->C, sizeof(long), nv, fp);
    glv->NV = nv;
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;   //= glv->NVl;
    glv->NElg = nelg; //= glv->NElg;
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: LoadGLVBin_OnlyC %s nv=%ld nc=%ld Q=%lf Successfully \n", name, nv, nc, Q);
#endif
    return 0;
}

int LoadGLVBin_OnlyC_malloc(char* name, GLV* glv) {
    assert(name);

    long nv = 0;
    long nc = 0;
    long head = 0;
    double Q = -1;
    long nvl;  //= glv->NVl;
    long nelg; //= glv->NElg;

    FILE* fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("ERROR: LoadGLVBin failed for open %s \n", name);
        return -1;
    }
    fread(&head, sizeof(long), 1, fp);
    if (head != headGLVBin) {
        printf("ERROR: head(%ld)!=headGLVBin(%ld) \n", head, headGLVBin);
        fclose(fp);
        return -1;
    }
    fread(&nv, sizeof(long), 1, fp);
    fread(&nc, sizeof(long), 1, fp);
    if (nc < 0) {
        printf("ERROR: value(%ld) of NC is not right!!!  \n", nc);
        fclose(fp);
        return -1;
    }
    fread(&Q, sizeof(double), 1, fp);
    if (Q > 1) {
        printf("ERROR: value(%lf) of Q is not right!!!  \n", Q);
        fclose(fp);
        return -1;
    }
    fread(&nvl, sizeof(long), 1, fp);
    fread(&nelg, sizeof(long), 1, fp);
    if (glv->C != NULL) free(glv->C);
    glv->C = (long*)malloc(nv * sizeof(long));
    fread(glv->C, sizeof(long), nv, fp);
    glv->NV = nv;
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;   //= glv->NVl;
    glv->NElg = nelg; //= glv->NElg;
    fclose(fp);
#ifdef PRINTINFO
    printf("INFO: LoadGLVBin_OnlyC %s nv=%ld nc=%ld Q=%lf Successfully \n", name, nv, nc, Q);
#endif
    return 0;
}
int LoadGLVBin_OnlyC(char* path, char* file, GLV* glv) {
    assert(path);
    assert(file);
    char pathName[2048];
    strcpy(pathName, path);
    strcat(pathName, file);
    return LoadGLVBin_OnlyC(pathName, glv);
}
int LoadGLVBin_OnlyC_malloc(char* path, char* file, GLV* glv) {
    assert(path);
    assert(file);
    char pathName[2048];
    strcpy(pathName, path);
    strcat(pathName, file);
    return LoadGLVBin_OnlyC_malloc(pathName, glv);
}
GLV* LoadGLVBin(char* name, int& id_glv) {
    assert(name);

    graphNew* g = (graphNew*)malloc(sizeof(graphNew));
    long nv = 0;
    long ne = 0;
    long ne_undir = 0;
    long nc = 0;
    double Q = -1;
    long head = 0;
    long nvl;  //= glv->NVl;
    long nelg; //= glv->NElg;

    FILE* fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("ERROR: LoadGLVBin failed for open %s \n", name);
        fclose(fp);
        return NULL;
    }
    fread(&head, sizeof(long), 1, fp);
    if (head != headGLVBin) {
        printf("ERROR: head(%ld)!=headGLVBin(%ld) \n", head, headGLVBin);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&nv, sizeof(long), 1, fp);
    if (nv <= 0) {
        printf("ERROR: value(%ld) of NV is not right!!!  \n", nv);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&ne, sizeof(long), 1, fp);
    if (ne <= 0) {
        printf("ERROR: value(%ld) of NE is not right!!!  \n", ne);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&ne_undir, sizeof(long), 1, fp);
    if (ne_undir <= 0) {
        printf("ERROR: value(%ld) of ne_undir is not right!!!  \n", ne_undir);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&nc, sizeof(long), 1, fp);
    if (nc < 0) {
        printf("ERROR: value(%ld) of NC is not right!!!  \n", nc);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&Q, sizeof(double), 1, fp);
    if (Q > 1) {
        printf("ERROR: value(%lf) of Q is not right!!!  \n", Q);
        fclose(fp);
        free(g);
        return NULL;
    }
    fread(&nvl, sizeof(long), 1, fp);
    fread(&nelg, sizeof(long), 1, fp);

    g->numEdges = ne;
    g->numVertices = nv;
#ifdef PRINTINFO
    printf("INFO: LoadGLVBin %s Successfully: nv=%ld ne=%ld undir ne=%ld nc=%ld Q=%lf \n", name, nv, ne, ne_undir, nc,
           Q);
#endif
    g->edgeListPtrs = (long*)malloc(sizeof(long) * nv + 1);
    g->edgeList = (edge*)malloc(sizeof(edge) * ne_undir);
    long* M = (long*)malloc(sizeof(long) * nv);
    assert(g->edgeListPtrs);
    assert(g->edgeList);
    assert(M);
    fread(g->edgeListPtrs, sizeof(long), nv + 1, fp);
    fread(g->edgeList, sizeof(edge), ne_undir, fp);
    fread(M, sizeof(long), nv, fp);
    GLV* glv = new GLV(id_glv);
    glv->SetByOhterG(g, M);
    fread(glv->C, sizeof(long), nv, fp);
    glv->NC = nc;
    glv->Q = Q;
    glv->NVl = nvl;   //= glv->NVl;
    glv->NElg = nelg; //= glv->NElg;
    fclose(fp);
    return glv;
}

GLV* LoadGLVBin(char* path, char* file, int& id_glv) {
    assert(path);
    assert(file);
    char pathName[2048];
    strcpy(pathName, path);
    strcat(pathName, file);
    return LoadGLVBin(pathName, id_glv);
}

void LoadGLVBinBatch(GLV* glv[], int num_par, char* path, char* names[], int& id_glv) {
    assert(glv);
    assert(num_par < MAX_PARTITION);
    int ret = 0;
    for (int i = 0; i < num_par; i++) {
        char pathName[1024];
        if (strlen(path)) {
            strcpy(pathName, path);
            strcat(pathName, "/");
        } else
            strcpy(pathName, "./");
        strcat(pathName, names[i]);
        glv[i] = LoadGLVBin(pathName, id_glv);
    }
}
// load/send GLV through data files end


int DEMOzmq_get_num_par(int src) {
    return src;
}

char* DEMOzmq_get_path(char* des, const char* src) {
    return strcpy(des, src);
}

char* DEMOzmq_get_name(char* des, char* src) {
    return strcpy(des, src);
}


void ConnectWorkers(Driver* drivers, int numPureWorker, char* nameWorkers[]) 
{
    if (numPureWorker == 0)
        return;

    std::cout << "INFO: Connecting to " << numPureWorker << " workers..." << std::endl;
    for (int i = 0; i < numPureWorker; i++) {
        drivers[i].connect(nameWorkers[i]);
        std::cout << "INFO: Connected to worker " << nameWorkers[i] << std::endl;
    }
}

void isAllWorkerLoadingDone(Node* nodes, int numPureWorker) {
    char tmp_msg_1w2d[MAX_LEN_MESSAGE]; // 4096 usually
    for (int i = 0; i < numPureWorker; i++) {
        printf("INFO: Listen to workers from requester %d\n", i);
        nodes[i].receive(tmp_msg_1w2d, MAX_LEN_MESSAGE);
        printf("INFO: Received from requester[%d] %s\n", i, tmp_msg_1w2d);
    }
}

void enalbeAllWorkerLouvain(Node* nodes, int numPureWorker) {
    char msg_d2w[MAX_LEN_MESSAGE]; // numWorker*4096 usually
    for (int i = 0; i < numPureWorker; i++) {
        printf("Sending work plan to worker %d\n", (i + 1));
        MessageGen_D2W(msg_d2w, (i + 1));
        nodes[i].send(msg_d2w, MAX_LEN_MESSAGE, 0);
    }
}

// functions for printing info or read command lines
int general_findPara(int argc, char** argv, const char* para) {
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp(argv[i], para)) return i;
    }
    return -1;
}
void ParameterError(const char* msg) {
    printf("\033[1;31;40mPARAMETER ERROR\033[0m: %s \n", msg);
    exit(1);
}

int host_ParserParameters(int argc,
                          char** argv,
                          double& opts_C_thresh,   //; //Threshold with coloring on
                          long& opts_minGraphSize, //; //Min |V| to enable coloring
                          double& opts_threshold,  //; //Value of threshold
                          int& opts_ftype,         //; //File type
                          char opts_inFile[4096],  //;
                          bool& opts_coloring,     //
                          bool& opts_output,       //;
                          std::string& opts_outputFile,
                          bool& opts_VF, //;
                          char opts_xclbinPath[4096],
                          int& numThread,
                          int& num_par,
                          int& gh_par,
                          int& flow_prune,
                          int& devNeed,
                          int& mode_zmq,
                          char* path_zmq,
                          bool& useCmd,
                          int& mode_alveo,
                          char* nameProj,
                          std::string& nameMetaFile,
                          int& numPureWorker,
                          char* nameWorkers[128],
                          int& nodeID,
						  int& server_par,
						  int& max_num_level,
						  int& max_num_iter) {
    const int max_parameter = 100;
    bool rec[max_parameter];
    for (int i = 1; i < argc; i++) rec[i] = false;
    int has_opts_C_thresh = general_findPara(argc, argv, "-d");
    int has_opts_minGraphSize = general_findPara(argc, argv, "-m");
    int has_opts_threshold = general_findPara(argc, argv, "-t");
    int has_opts_ftype = general_findPara(argc, argv, "-f");
    int has_opts_inFile; //= general_findPara(argc, argv, "-thread");
    int has_opts_coloring = general_findPara(argc, argv, "-c");
    int has_opts_output = general_findPara(argc, argv, "-o");
    int has_opts_VF = general_findPara(argc, argv, "-v");
    int has_opts_xclbinPath = general_findPara(argc, argv, "-x");
    int has_numThread = general_findPara(argc, argv, "-thread");
    int has_num_par = general_findPara(argc, argv, "-par_num");
    int has_gh_par = general_findPara(argc, argv, "-par_prune");
    int has_flow_prune = general_findPara(argc, argv, "-prun");
    int has_flow_fast = general_findPara(argc, argv, "-fast");
    int has_flow_fast2 = general_findPara(argc, argv, "-fast2");
    int has_devNeed = general_findPara(argc, argv, "-dev");
    int has_driver = general_findPara(argc, argv, "-driver");
    int has_worker = general_findPara(argc, argv, "-worker");
    int has_driverAlone = general_findPara(argc, argv, "-driverAlone");
    int has_workerAlone = general_findPara(argc, argv, "-workerAlone");
    int has_cmd = general_findPara(argc, argv, "-cmd");
    int has_server_par = general_findPara(argc, argv, "-server_par");
    int has_max_num_level = general_findPara(argc, argv, "-num_level");
    int has_max_num_iter = general_findPara(argc, argv, "-num_iter");

    if (general_findPara(argc, argv, "-create_alveo_partitions") != -1) {
        mode_alveo = ALVEOAPI_PARTITION;
        int indx = general_findPara(argc, argv, "-name");
        if (argc > indx && indx != -1) strcpy(nameProj, argv[indx + 1]);
    } else if (general_findPara(argc, argv, "-load_alveo_partitions") != -1) {
        int indx = general_findPara(argc, argv, "-load_alveo_partitions") + 1;
        mode_alveo = ALVEOAPI_LOAD;
        if (argc > indx)
            nameMetaFile = argv[indx];
        else {
            printf("\033[1;31;40mPARAMETER ERROR\033[0m: -load_alveo_partitions <Project Metadata file> \n");
            return -1;
        }
        if (general_findPara(argc, argv, "-setwkr") == -1) {
            numPureWorker = 0;
        } else { //[-setwkr <numWorkers> <worker name> [<worker name>] ]
            int indx2 = general_findPara(argc, argv, "-setwkr") + 1;
            if (argc <= indx2) ParameterError("-setwkr <numWorkers> <worker name> [<worker name>] ]");
            numPureWorker = atoi(argv[indx2++]);
            if (argc < indx2 + numPureWorker) ParameterError("-setwkr <numWorkers> <worker name> [<worker name>] ]");
            for (int i = 0; i < numPureWorker; i++) {
                nameWorkers[i] = argv[indx2 + i];
            }
        }
        if ((has_driverAlone == -1) && (has_workerAlone == -1)) {
            mode_zmq = ZMQ_NONE;
        } else {
            if (has_driverAlone != -1) {
                mode_zmq = ZMQ_DRIVER;
            } else {
                mode_zmq = ZMQ_WORKER;
                if (argc > has_workerAlone + 1)
                    nodeID = atoi(argv[has_workerAlone + 1]);
                else {
                    printf(
                        "\033[1;31;40mPARAMETER ERROR\033[0m: -load_alveo_partitions -worker "
                        "\033[1;31;40m<nodeID>\033[0m missed \n");
                    exit(1);
                }
            }
        }
    } //
    else if (general_findPara(argc, argv, "-louvain_modularity_alveo") != -1)
        mode_alveo = ALVEOAPI_RUN;
    else
        mode_alveo = ALVEOAPI_NONE;

    if (has_cmd != -1)
        useCmd = true;
    else
        useCmd = false;

    if (mode_alveo == ALVEOAPI_NONE) {
        if ((has_driver == -1) && (has_worker == -1)) {
            mode_zmq = ZMQ_NONE;
        } else {
            if (has_driver != -1) {
                mode_zmq = ZMQ_DRIVER;
                if (argc > has_driver + 1)
                    strcpy(path_zmq, argv[has_driver + 1]);
                else
                    strcpy(path_zmq, "./");
            } else {
                mode_zmq = ZMQ_WORKER;
                if (argc > has_worker + 1)
                    strcpy(path_zmq, argv[has_driver + 1]);
                else
                    strcpy(path_zmq, "./");
            }
        }
    }

    if (has_opts_C_thresh != -1 && has_opts_C_thresh < (argc - 1)) {
        rec[has_opts_C_thresh] = true;
        rec[has_opts_C_thresh + 1] = true;
        opts_C_thresh = atof(argv[has_opts_C_thresh + 1]);
    } else
        opts_C_thresh = 0.0002;
#ifdef PRINTINFO
    printf("PARAMETER  opts_C_thresh = %f\n", opts_C_thresh);
#endif
    if (has_opts_minGraphSize != -1 && has_opts_minGraphSize < (argc - 1)) {
        rec[has_opts_minGraphSize] = true;
        rec[has_opts_minGraphSize + 1] = true;
        opts_minGraphSize = atoi(argv[has_opts_minGraphSize + 1]);
    } else
        opts_minGraphSize = 10;
#ifdef PRINTINFO
    printf("PARAMETER  has_opts_minGraphSize= %ld\n", opts_minGraphSize);
#endif
    if (has_opts_threshold != -1 && has_opts_threshold < (argc - 1)) {
        rec[has_opts_threshold] = true;
        rec[has_opts_threshold + 1] = true;
        opts_threshold = atof(argv[has_opts_threshold + 1]);
    } else
        opts_threshold = 0.000001;
#ifdef PRINTINFO
    printf("PARAMETER  opts_C_thresh= %f\n", opts_C_thresh);
#endif
    if (has_opts_ftype != -1 && has_opts_ftype < (argc - 1)) {
        rec[has_opts_ftype] = true;
        rec[has_opts_ftype + 1] = true;
        opts_ftype = atof(argv[has_opts_ftype + 1]);
    } else
        opts_ftype = 3;
#ifdef PRINTINFO
    printf("PARAMETER  opts_ftype = %i\n", opts_ftype);
#endif
    if (has_opts_coloring != -1) {
        rec[has_opts_coloring] = true;
        opts_coloring = true;
    }
#ifdef PRINTINFO
    printf("PARAMETER  opts_coloring = %d\n", opts_coloring);
#endif
    opts_output = false;
    if (has_opts_VF != -1) {
        rec[has_opts_VF] = true;
        opts_VF = true;
    }
#ifdef PRINTINFO
    printf("PARAMETER  opts_VF = %d\n", opts_VF);
#endif
    if (has_opts_xclbinPath != -1 && has_opts_xclbinPath < (argc - 1)) {
        rec[has_opts_xclbinPath] = true;
        rec[has_opts_xclbinPath + 1] = true;
        strcpy(opts_xclbinPath, argv[has_opts_xclbinPath + 1]);
#ifdef PRINTINFO
        printf("PARAMETER  opts_xclbinPath = %s\n", opts_xclbinPath);
#endif
    } else {
        opts_xclbinPath[0] = 0;
    }

    if (has_numThread != -1 && has_numThread < (argc - 1)) {
        rec[has_numThread] = true;
        rec[has_numThread + 1] = true;
        numThread = atoi(argv[has_numThread + 1]);
    } else
        numThread = 16;
#ifdef PRINTINFO
    printf("PARAMETER numThread = %i\n", numThread);
#endif
    if (has_num_par != -1 && has_num_par < (argc - 1)) {
        rec[has_num_par] = true;
        rec[has_num_par + 1] = true;
        num_par = atoi(argv[has_num_par + 1]);
    } else
        num_par = 2;
#ifdef PRINTINFO
    printf("PARAMETER  num_par = %i\n", num_par);
#endif
    if (has_devNeed != -1 && has_devNeed < (argc - 1)) {
        rec[has_devNeed] = true;
        rec[has_devNeed + 1] = true;
        devNeed = atoi(argv[has_devNeed + 1]);
    } else
        devNeed = 1;
#ifdef PRINTINFO
    printf("PARAMETER  devNeed = %i\n", devNeed);
#endif
    if (has_gh_par != -1 && has_gh_par < (argc - 1)) {
        rec[has_gh_par] = true;
        rec[has_gh_par + 1] = true;
        gh_par = atoi(argv[has_gh_par + 1]);
    } else
        gh_par = 1;
#ifdef PRINTINFO
    printf("PARAMETER  gh_par = %i\n", gh_par);
#endif
    if (has_flow_prune != -1) {
        rec[has_flow_prune] = true;
        flow_prune = 2;
    } else if(has_flow_fast != -1) {
        rec[has_flow_fast] = true;
        flow_prune = 2;
    } else if (has_flow_fast2 != -1) {
        rec[has_flow_fast2] = true;
        flow_prune = 3;
    } else
        flow_prune = 1;
#ifdef PRINTINFO
    printf("PARAMETER  flow_prune = %d\n", flow_prune);
#endif
    if (has_opts_output != -1 && has_opts_output < (argc - 1)) {
        opts_output = true;
        rec[has_opts_output] = true;
        rec[has_opts_output + 1] = true; 
        
        opts_outputFile = argv[has_opts_output + 1];
#ifdef PRINTINFO
        printf("PARAMETER  opts_outFile = %s\n", opts_outputFile);
#endif
        /*
        FILE* file = fopen(opts_outputFile, "w");
        if (file == NULL) {
            printf("\033[1;31;40mPARAMETER ERROR\033[0m: Cannot open the opts_outFile file: %s\n", opts_outputFile);
            exit(1);
        } else {
            fclose(file);
        }*/
    }

    if (has_server_par != -1 && has_server_par < (argc - 1)) {
        rec[has_server_par] = true;
        rec[has_server_par + 1] = true;
        server_par = atoi(argv[has_server_par + 1]);
    } else
    	server_par = 1;
#ifdef PRINTINFO
    printf("PARAMETER server_par = %i\n", server_par);
#endif

    if (has_max_num_level != -1 && has_max_num_level < (argc - 1)) {
        rec[has_max_num_level] = true;
        rec[has_max_num_level + 1] = true;
        max_num_level = atoi(argv[has_max_num_level + 1]);
    } else
    	max_num_level = MAX_NUM_PHASE;
#ifdef PRINTINFO
    printf("PARAMETER max_num_level = %i\n", max_num_level);
#endif

    if (has_max_num_iter != -1 && has_max_num_iter < (argc - 1)) {
        rec[has_max_num_iter] = true;
        rec[has_max_num_iter + 1] = true;
        max_num_iter = atoi(argv[has_max_num_iter + 1]);
    } else
    	max_num_iter = MAX_NUM_TOTITR;
#ifdef PRINTINFO
    printf("PARAMETER max_num_iter = %i\n", max_num_iter);
#endif


    if(mode_alveo == ALVEOAPI_LOAD)
    	return 0; //No need to set input matrix file if
    for (int i = 1; i < argc; i++) {
        // printf("i= %d rec[i]=%d\n", i , rec[i]);
        if (rec[i] == false) {
            has_opts_inFile = i;
            strcpy(opts_inFile, argv[has_opts_inFile]);
#ifdef PRINTINFO
            printf("PARAMETER opts_inFile = %s\n", opts_inFile);
#endif
            FILE* file = fopen(opts_inFile, "r");
            if (file == NULL) {
                printf("\033[1;31;40mPARAMETER ERROR\033[0m: Cannot open the batch file: %s\n", opts_inFile);
                exit(1);
            } else
                fclose(file);
            break;
        } else {
            if (i == argc - 1) {
                printf("\033[1;31;40mPARAMETER ERROR\033[0m: opts_inFile NOT set!!!\n");
                exit(1);
            }
        }
    }

    return 0;
}

void PrintTimeRpt(GLV* glv, int num_dev, bool isHead) {
    int num_phase = 6;
    if (isHead) {
        printf("==========");
        printf("==========");
        for (int d = 0; d < num_dev; d++) {
            printf("==Dev_%-2d==", d);
        }
        printf("=");

        for (int phs = 0; phs < num_phase; phs++) printf("==phase%-2d=", phs);
    } else {
        for (int d = 0; d < (num_dev + num_phase); d++) printf("----------");
    }
    printf("\n");

    for (int d = 0; d < num_dev; d++) {
        // 1: Get E2E time on the device
        glv->times.totTimeE2E_DEV[d] = 0;
        for (int i = 0; i < glv->times.phase; i++)
            if (glv->times.deviceID[i] == d) glv->times.totTimeE2E_DEV[d] += glv->times.eachTimeE2E[i];
        // 2-1: Print left column
        if (d == 0) {
            if (glv->times.parNo == -1)
                printf("Final Louv ");
            else
                printf("Par:%2d    ", glv->times.parNo);
        } else
            printf("          ");
        printf("Dev_%-2d:   ", d);
        // 2-2:
        for (int i = 0; i < num_dev; i++) {
            if (i == d) {
                printf(" %3.4f ", glv->times.totTimeE2E_DEV[d]);
            } else
                printf("          ");
        }
        printf(" = ");
        // 2-3
        for (int i = 0; i < glv->times.phase; i++) {
            if (glv->times.deviceID[i] != d)
                printf("          ");
            else
                printf(" + %2.3f ", glv->times.eachTimeE2E[i]);
        }
        printf("\n");
    }
}

void PrintTimeRpt(ParLV& parlv, int num_dev) {
    printf(
        "===============\033[1;35;40mE2E time Matrix for each partition's very phases on each device "
        "\033[0m====================\n");
    double totTimeOnDev[num_dev];
    for (int d = 0; d < num_dev; d++) totTimeOnDev[d] = 0;
    for (int p = 0; p < parlv.num_par; p++) {
        PrintTimeRpt(parlv.par_src[p], num_dev, p == 0);
        for (int d = 0; d < num_dev; d++) {
            totTimeOnDev[d] += parlv.par_src[p]->times.totTimeE2E_DEV[d];
        }
    }
    printf("--------------------------------------------------------------------------------\n");
    // printf("Total Par time   :   ", num_dev);//?
    for (int d = 0; d < num_dev; d++) printf(" %3.4f   ", totTimeOnDev[d]);
    printf("\n");
    // printf("============================================================\n");
    PrintTimeRpt(parlv.plv_merged, num_dev, false);
    printf("====================================================================================================\n");
}
//Summary
//Number of vertices           : 30
//Number of edges              : 15
//Number of partitions         : 10
//Partition size               : 3
//Number of nodes (machines)   : 3
//Number of Xilinx Alveo cards : 9
//Number of levels             : 3
//Delta Q tolerance            : 0.0001
//Number of iterations         : 20 [15, 10, 5]
//Number of communities        : 12 [7, 3, 2]
//Modularity                   : 0.84 [0.64, 0.77, 0.84]
//Time                         : 9 sec (Partition Compute: 7 sec, Merge: 1 sec, Merge Compute: 1 sec)
template <class T>
void PrintArrayByNum(int num, T* array){
	for(int i=0; i < num; i++){
		if(i==0)printf("[");
		else printf(", ");
		if(std::is_same<T, int>::value)
			printf("%d", array[i]);
		else if(std::is_same<T, long>::value)
			printf("%ld", array[i]);
		else if( std::is_same<T, float>::value)
			printf("%f", array[i]);
		else if( std::is_same<T, double>::value)
			printf("%lf", array[i]);
		if(i==num-1)printf("]\n");
	}
}
template <class T>
T SummArrayByNum(int num, T* array){
	T ret=0;
	for(int i=0; i < num; i++)
		ret +=array[i];
	return ret;
}

void PrintRptPartition_Summary(
		ParLV& parlv,
		//int numNode, int* card_Node,
		long opts_C_thresh
		) {
	int num_par = parlv.num_par;
	int numNode = parlv.num_server;
	int* card_Node = parlv.numServerCard;

	printf("****************************************Summary*************************************************\n");
	printf("Number of vertices           : %ld\n", parlv.plv_src->NV);
	printf("Number of edges              : %ld\n", parlv.plv_src->NE);
	printf("Number of partitions         : %d\n" , parlv.num_par);
	printf("Partition size               : < %ld\n", 64000000);
	printf("Number of nodes (machines)   : %d\n" , numNode);
	printf("Number of Xilinx Alveo cards : %d\n" , SummArrayByNum<int>(numNode, card_Node));
	printf("Delta Q tolerance            : %lf\n" , opts_C_thresh);
	printf("Number of communities        : %ld\n", parlv.plv_src->NC);
	printf("Modularity                   : %lf\n ", parlv.plv_src->Q);
	for(int p = 0; p< num_par; p++){
		printf("\tPARTITION-%d :\n ", p);
		int numLevel = parlv.par_src[p]->times.phase;
		printf("\t\tNumber of levels             : %d\n" , numLevel );

		printf("\t\tNumber of iterations         : %d "	, parlv.par_src[p]->times.totItr);
		PrintArrayByNum<int>(numLevel, parlv.par_src[p]->times.eachItrs);//[15, 10, 5]

		printf("\t\tNumber of communities        : %ld ", parlv.par_src[p]->NC);
		PrintArrayByNum<long>(numLevel, parlv.par_src[p]->times.eachClusters);//[7, 3, 2]

		printf("\t\tModularity                   : %lf ", parlv.par_src[p]->Q);
		PrintArrayByNum<double>(numLevel, parlv.par_src[p]->times.eachMod);//[7, 3, 2]//[0.64, 0.77, 0.84]
	}
	printf("Time                         : %4.3f sec ", parlv.timesPar.timeDriverExecute );
	printf(" (Partition Compute: %4.3f sec"           , parlv.timesPar.timeDriverCollect);
	printf(", Merge: %4.3f sec"                       , parlv.timesPar.timePre + parlv.timesPar.timeMerge);
	printf(", Merge Compute: %4.3f sec)"              , parlv.timesPar.timeFinal);
	printf("\n");
	printf("************************************************************************************************\n");
}

void PrintRptPartition(int mode_zmq, ParLV& parlv, int op0_numDevices, int numNode, int numPureWorker) {
    if (mode_zmq != ZMQ_WORKER) {
        printf("************************************************************************************************\n");
        printf(
            "******************************  \033[1;35;40mPartition Louvain Performance\033[0m   "
            "********************************\n");
        printf("************************************************************************************************\n");
        printf("\033[1;37;40mINFO\033[0m: Original number of vertices            : %ld\n", parlv.plv_src->NV);
        printf("\033[1;37;40mINFO\033[0m: Original number of un-direct edges     : %ld\n", parlv.plv_src->NE);
        printf("\033[1;37;40mINFO\033[0m: number of partition                    : %d \n", parlv.num_par);
        printf("\033[1;37;40mINFO\033[0m: number of device used                  : %d \n", op0_numDevices);
        printf("\033[1;37;40mINFO\033[0m: Final number of communities            : %ld\n", parlv.plv_src->NC); 
        printf("\033[1;37;40mINFO\033[0m: Final modularity                       : %lf\n",
               parlv.plv_src->Q); // com_lit.back().
        printf("************************************************************************************************\n");

        if (mode_zmq == ZMQ_NONE) {
            double totTimeOnDev[parlv.num_dev];
            double totTimeFPGA_pure = 0;
            double totTimeFPGA_wait = 0;
            double totTimeCPU = 0;
            for (int d = 0; d < op0_numDevices; d++) totTimeOnDev[d] = 0;
            for (int p = 0; p < parlv.num_par; p++) {
                for (int d = 0; d < op0_numDevices; d++) {
                    totTimeOnDev[d] += parlv.par_src[p]->times.totTimeE2E_DEV[d];
                    totTimeFPGA_pure += parlv.par_src[p]->times.totTimeE2E_DEV[d];
                }
                totTimeFPGA_wait += parlv.par_src[p]->times.totTimeE2E_2;
                totTimeCPU += parlv.par_src[p]->times.totTimeAll - parlv.par_src[p]->times.totTimeE2E;
            }
            totTimeFPGA_pure += parlv.plv_merged->times.totTimeE2E;
            totTimeFPGA_wait += parlv.plv_merged->times.totTimeE2E_2;
            totTimeCPU += parlv.plv_merged->times.totTimeAll - parlv.plv_merged->times.totTimeE2E;
            printf("\t--- Total for All CPU time (s)       : %lf\t=\t", totTimeCPU);
            for (int p = 0; p < parlv.num_par; p++)
                printf(" +%lf (par-%d) ", parlv.par_src[p]->times.totTimeAll - parlv.par_src[p]->times.totTimeE2E, p);
            printf(" +%lf (Final) \n", parlv.plv_merged->times.totTimeAll - parlv.plv_merged->times.totTimeE2E);
            printf("\t--- Total for All FPGA time (s)      : %lf\t=\t", totTimeFPGA_pure);
            for (int p = 0; p < parlv.num_par; p++) printf(" +%lf (par-%d) ", parlv.par_src[p]->times.totTimeE2E, p);
            printf(" +%lf (Final) \n", parlv.plv_merged->times.totTimeE2E);

            for (int d = 0; d < op0_numDevices; d++)
                printf("\t--- Sub-Louvain on Dev-%d             : %lf\n", d, totTimeOnDev[d]);
            printf("\t--- Fnl-Louvain on Dev-0             : %lf\n", parlv.plv_merged->times.totTimeE2E);
            printf("\t--- FPGA efficiency with %d device(s) : %2.2f\n", op0_numDevices,
                   (totTimeFPGA_pure * 100.0) /
                       (op0_numDevices * (parlv.timesPar.timeAll - parlv.timesPar.timePar_all)));
            printf(
                "************************************************************************************************\n");
        } // end ZMQ_NONE

        printf("************************************************************************************************\n");
        printf("***********************************  Louvain Summary   *****************************************\n");
        printf("************************************************************************************************\n");
        printf(
            "\033[1;31;40m    1.   Time for loading partitions \033[0m                               : "
            "\033[1;31;40m%lf\033[0m (s)\n",
            parlv.timesPar.timeDriverLoad);
        printf(
            "\033[1;31;40m    2.   Time for computing distributed Louvain:  \033[0m                  : "
            "\033[1;31;40m%lf\033[0m (s) =",
            parlv.timesPar.timeDriverExecute);
        printf(" Time for sub-Louvain&transmission + Driver Time for Merge&Final-Louvain\n");
        printf("    2.1   Time for sub-Louvain&transmission                          : %lf (s) = ",
               parlv.timesPar.timeDriverCollect);
        printf(" max( driver, worker-1, worker-2)\n");

        printf("    2.1.1 Driver Time for sub-Louvain and results receiving          : %lf (s) = ",
               parlv.timesPar.timeDriverCollect);
        printf(" %lf (sub-lv) + %lf (waiting & recv) \n", parlv.timesPar.timeWrkCompute[numNode - 1],
               parlv.timesPar.timeDriverCollect - parlv.timesPar.timeWrkCompute[numNode - 1]);
        printf("    2.1.2 Worker Time for sub-Louvain and results sending            : %lf (s) = ",
               parlv.timesPar.timeWrkCompute[0] + parlv.timesPar.timeWrkSend[0]);
        printf(" %lf (sub-lv) + %lf (send) \n", parlv.timesPar.timeWrkCompute[0], parlv.timesPar.timeWrkSend[0]);
        printf("    2.1.3 Worker Time for sub-Louvain and results sending            : %lf (s) = ",
               parlv.timesPar.timeWrkCompute[1] + parlv.timesPar.timeWrkSend[1]);
        printf(" %lf (sub-lv) + %lf (send) \n", parlv.timesPar.timeWrkCompute[1], parlv.timesPar.timeWrkSend[1]);

        printf("    2.2   Driver Time for Merge&Final-Louavin                        : %lf (s) = ",
               parlv.timesPar.time_done_mg);
        printf(" pre-Merge + Merge + Final-Louvain \n");
        printf("    2.2.1 Driver: pre-Merge                                          : %lf (s)\n",
               parlv.timesPar.timePre);
        printf("    2.2.2 Driver: Merge                                              : %lf (s)\n",
               parlv.timesPar.timeMerge);
        printf("    2.2.3 Driver: Final-Louvain                                      : %lf (s)\n",
               parlv.timesPar.timeFinal);
    }
    printf("************************************************************************************************\n");

}

void PrintRptParameters(double opts_C_thresh,   // Threshold with coloring on
                        long opts_minGraphSize, // Min |V| to enable coloring
                        double opts_threshold,  // Value of threshold
                        int opts_ftype,         // File type
                        char* opts_inFile,
                        bool opts_coloring,
                        bool opts_output,
                        char* opts_outputFile,
                        bool opts_VF,
                        char* opts_xclbinPath,
                        int numThreads,
                        int num_par,
                        int par_prune,
                        bool flow_fast,
                        int devNeed_cmd,
                        int mode_zmq,
                        char* path_zmq,
                        bool useCmd,
                        int mode_alveo,
                        xf::graph::L3::Handle::singleOP& op0) {
    printf("************************************************************************************************\n");
    printf(
        "********************************  \033[1;35;40mParameters Report \033[0m   "
        "*********************|********************\n");
    printf("************************************************************************************************\n");
    // numDevices
    if (devNeed_cmd == 0)
        printf(
            "FPGA Parameter \033[1;37;40mnumDevices    \033[0m: %-8d  \t\t\t Default=        1,       by config.json "
            "or\" \033[1;37;40m-dev\033[0m     <v> \"  \n",
            op0.numDevices);
    else
        printf(
            "FPGA Parameter \033[1;37;40mnumDevices    \033[0m: %-8d  \t\t\t Default=        1,       by "
            "command-line: \" \033[1;37;40m-dev\033[0m     <v> \"",
            op0.numDevices);
    printf(" or by config.json\n");
    printf(
        "FPGA Parameter \033[1;37;40mrequestLoad     \033[0m: %-8d  \t\t\t Default=      100,       by config.json     "
        "                  \n",
        op0.requestLoad);
    printf(
        "FPGA Parameter \033[1;37;40moperationName   \033[0m: %s    \t\t Default=louvainModularity, by config.json     "
        "                  \n",
        op0.operationName.c_str());
    printf(
        "FPGA Parameter \033[1;37;40mkernelName      \033[0m: %s      \t\t Default=kernel_louvain,  by config.json     "
        "                  \n",
        op0.kernelName.c_str());
    if (opts_xclbinPath[0] == 0)
        printf(
            "FPGA Parameter \033[1;37;40mxclbinFile      \033[0m: %s    \t  by config.json           or by \" "
            "\033[1;37;40m-x <path>\033[0m\"                    \n",
            op0.xclbinPath.c_str());
    else
        printf(
            "FPGA Parameter \033[1;37;40mxclbinFile      \033[0m: %s    \t  by command-line: \" \033[1;37;40m-x "
            "<path>\033[0m\" or by config.json                \n",
            op0.xclbinPath.c_str());
    printf(
        "FPGA Parameter \033[1;37;40mtype of xclbin  \033[0m: %s    \t\t\t Default=  normal ,       by command-line: "
        "\" \033[1;37;40m-fast\033[0m \"         \n",
        flow_fast ? "fast" : "normal");
    printf(
        "Louv Parameter \033[1;37;40mLouvain_inFile  \033[0m: %s    \t\t\t Required -f  3   ,       by command-line: "
        "\" \033[1;37;40m<name>\033[0m \"        \n",
        opts_inFile);
    printf(
        "Louv Parameter \033[1;37;40mLouvain_Output  \033[0m: %s    \t\t\t Default=  No Out ,       by command-line: "
        "\" \033[1;37;40m-o\033[0m \"            \n",
        opts_output ? "true" : "false");
    if (opts_output)
        printf(
            "Louv Parameter \033[1;37;40mhas Output File \033[0m: %s      \t\t Default=    false,       by "
            "command-line: \" \033[1;37;40m-o     <path>\033[0m \" \n",
            opts_outputFile);
    printf(
        "Louv Parameter \033[1;37;40mCPU numThreads  \033[0m: %-8d  \t\t\t Default=       16,       by command-line: "
        "\" \033[1;37;40m-thread   <v>\033[0m \" \n",
        numThreads);
    printf(
        "Louv Parameter \033[1;37;40mcoloring thrhd  \033[0m: %1.7f \t\t\t Default=   0.0002,       by command-line: "
        "\" \033[1;37;40m-d        <v>\033[0m \" \n",
        opts_C_thresh);
    printf(
        "Louv Parameter \033[1;37;40mparallel thrhd  \033[0m: %1.7f \t\t\t Default= 0.000001,       by command-line: "
        "\" \033[1;37;40m-t        <v>\033[0m \" \n",
        opts_threshold);
    printf(
        "Louv Parameter \033[1;37;40mminGraphSize    \033[0m: %-8ld  \t\t\t Default=       10,       by command-line: "
        "\" \033[1;37;40m-m        <v>\033[0m \" \n",
        opts_minGraphSize);
    printf(
        "Part Parameter \033[1;37;40mNumber of shares\033[0m: %-8d  \t\t\t Default=        2,       by command-line: "
        "\" \033[1;37;40m-par_num  <v>\033[0m \" \n",
        num_par);
    printf(
        "Part Parameter \033[1;37;40mpruning thrhd   \033[0m: %-8d  \t\t\t Default=        1,       by command-line: "
        "\" \033[1;37;40m-par_prune<v>\033[0m \" \n",
        par_prune);

    printf("************************************************************************************************\n");
}

// functions for printing info or read command lines end

void ParLV::Init(int mode) {
    st_Partitioned = false;
    st_ParLved = false;
    st_PreMerged = false;
    st_Merged = false;
    st_FinalLved = false;
    //
    st_Merged_ll = false;
    st_Merged_gh = false;
    isMergeGhost = false;
    isOnlyGL = false;
    isPrun = true;
    th_prun = 1;
    plv_src = NULL;
    plv_merged = NULL;
    plv_final = NULL;
    num_par = NV = NVl = NE = NElg = NEll = NEgl = NEgg = NEself = NV_gh = 0;
    elist = NULL;
    M_v = NULL;
    NE_list_all = 0;
    NE_list_ll = 0;
    NE_list_gl = 0;
    NE_list_gg = 0;
    NV_list_all = 0;
    NV_list_l = 0;
    NV_list_g = 0;
    num_dev = 1;
    flowMode = mode;
    num_server = 1;
}

void ParLV::Init(int mode, GLV* src, int nump, int numd) {
    Init(mode);
    plv_src = src;
    num_par = nump;
    num_dev = numd;
    st_Partitioned = true;
}

void ParLV::Init(int mode, GLV* src, int num_p, int num_d, bool isPrun, int th_prun) {
    Init(mode, src, num_p, num_d);
    this->isPrun = isPrun;
    this->th_prun = th_prun;
}

ParLV::ParLV() {
    Init(MD_FAST);
}
ParLV::~ParLV() {
    num_par = 0;
    if (elist) free(elist);
    if (M_v) free(M_v);
}
void ParLV::PrintSelf() { /*
          printf("PAR: %s is partitioned into %d share:\n", plv_src->name, num_par);//basic information
          for(int p=0; p<num_par; p++){
                  printf("Share-%d    %s \t: NV=%-8d NVl=%-8d NE=%-8d NElg=%-8d \n", p, par_src[p]->name
                  , par_src[p]->NV
                  , par_src[p]->NVl
                  , par_src[p]->NE
                  , par_src[p]->NElg
                  );//sub graph information
                  if(st_ParLved==false)
                          continue;
                  printf("Share-%d Lv(%s)\t: NV=%-8d NVl=%-8d NE=%-8d NElg=%-8d \n\n", p, par_src[p]->name
                  , par_lved[p]->NV
                  , par_lved[p]->NVl
                  , par_lved[p]->NE
                  , par_lved[p]->NElg
                  );//sub graph information
          }*/
    /*
    printf("=========================================================[ \033[1;35;40mPartitioned sub-graphs\033[0m
]======================================================================================\n");
    for(int p=0; p<num_par; p++)
            if(st_Partitioned==false)
                    break;
            else this->par_src[p]->printSimple();
printf("=========================================================[ \033[1;35;40mLouvained sub-graphs\033[0m
]======================================================================================\n");
    for(int p=0; p<num_par; p++)
            if(st_ParLved==false)
                    break;
            else this->par_lved[p]->printSimple();

    //if(st_FinalLved)
    {
            printf("=========================================================[ \033[1;35;40mMerged sub-graphs
together\033[0m ]======================================================================================\n");

            this->plv_merged->printSimple();
            printf("=========================================================[ \033[1;35;40mFinal Louvained merged
sub-graphs\033[0m ]======================================================================================\n");
            this->plv_final->printSimple();
            printf("=========================================================[ \033[1;35;40mOriginal graph with Updated
Communities\033[0m ]======================================================================================\n");
            this->plv_src->printSimple();
    //  printf("NE(%d) = NEl(%d) + NE_ll(%d) + NE_lg(%d) + NE_gg(%d)\n", NE, NEll, NElg, NEgg);
    //  printf("NEself = %d\n", NEself);
    }*/
    /*else if(st_Merged){
            this->plv_merged->printSimple();
            printf("=========================================================[ \033[1;35;40mOriginal graph with Updated
    Communities\033[0m ]======================================================================================\n");
            this->plv_src->printSimple();
    //  printf("NE(%d) = NEl(%d) + NE_ll(%d) + NE_lg(%d) + NE_gg(%d)\n", NE, NEll, NElg, NEgg);
    //  printf("NEself = %d\n", NEself);
    }else if(st_PreMerged){
            this->plv_merged->printSimple();
    //  printf("NV(%d) = NVl(%d) + NV_gh(%d)\n", NV, NVl, NV_gh);
    }*/
    //=========================================================[ LIST   END
    //]======================================================================================
    //| GLV ID: 1 | NC/NV:    42797/ 311627(41384/13%) NE:  11037306(134167   / 1%)| Colors:337       Q: 0.867658  |
    // name: ID:1_ParID:0_0_270243_th0
    //= Uniq ID ==| Numbers for C / V      (V_ghost  ) Edge Number  (ghost edges
    //)================================================================================
    printf(
        "=========================================================[\033[1;35;40m LIST Begin "
        "\033[0m]======================================================================================\n");
    printf(
        "= Uniq ID ==| Numbers for  C / V     ( V_ghost ) Edge Number  (ghost edges  "
        ")=================================================================================\n");
    printf(
        "=========================================================[\033[1;35;40m Partitioned sub-graphs "
        "\033[0m]============================================================================\n");

    for (int p = 0; p < num_par; p++)
        if (st_Partitioned == false)
            break;
        else
            this->par_src[p]->printSimple();
    printf(
        "=========================================================[\033[1;35;40m Louvained sub-graphs "
        "\033[0m]==================================================================================\n");

    for (int p = 0; p < num_par; p++)
        if (st_ParLved == false)
            break;
        else
            this->par_lved[p]->printSimple();

    if (st_FinalLved) {
        this->plv_src->printSimple();
        printf(
            "=========================================================[\033[1;35;40m Merged sub-graphs together "
            "\033[0m]============================================================================\n");
        this->plv_merged->printSimple();
        printf(
            "=========================================================[\033[1;35;40m Original graph with Updated "
            "Communities \033[0m]===============================================================\n");
        this->plv_final->printSimple();
        // printf("NE(%d) = NEl(%d) + NE_ll(%d) + NE_lg(%d) + NE_gg(%d)\n", NE, NEll, NElg, NEgg);
        // printf("NEself = %d\n", NEself);
    } else if (st_Merged) {
        this->plv_merged->printSimple();
        printf(
            "=========================================================[\033[1;35;40m Original graph with Updated "
            "Communities \033[0m]===============================================================\n");
        this->plv_src->printSimple();
        // printf("NE(%d) = NEl(%d) + NE_ll(%d) + NE_lg(%d) + NE_gg(%d)\n", NE, NEll, NElg, NEgg);
        // printf("NEself = %d\n", NEself);
    } else if (st_PreMerged) {
        this->plv_merged->printSimple();
        // printf("NV(%d) = NVl(%d) + NV_gh(%d)\n", NV, NVl, NV_gh);
    }
    printf(
        "==========================================================[\033[1;35;40m LIST   END "
        "\033[0m]===========================================================================================\n");
}

void ParLV::UpdateTimeAll() {
    timesPar.timeAll =
        +timesPar.timePar_all + timesPar.timeLv_all + timesPar.timePre + timesPar.timeMerge + timesPar.timeFinal;
};

int ParLV::partition(GLV* glv_src, int& id_glv, int num, long th_size, int th_maxGhost) {
    assert(glv_src);
    assert(glv_src->G);
    num_par = num;
    if (num_par >= MAX_PARTITION) {
        printf("\033[1;31;40mERROR\033[0m: exe_LV_SETM wrong number of partition %d which should be small than %d!\n",
               num_par, MAX_PARTITION);
        return -1;
    }
    long vsize = glv_src->NV / num_par;
    long start = 0;
    long end = start + vsize;
    off_src[0] = 0;
    for (int i = 0; i < num_par; i++) {
        if (th_maxGhost > 0)
            par_src[i] = stt[i].ParNewGlv_Prun(glv_src->G, start, end, id_glv, th_maxGhost);
        else
            par_src[i] = stt[i].ParNewGlv(glv_src->G, start, end, id_glv);
        // par_list.push_back(pt_par[i]);
        start = end;
        end = start + vsize;
        off_src[i + 1] = start;
    }
    return 0;
}

int GetScl(long v) {
    int ret = 0;
    while (v > 0) {
        v = v >> 1;
        ret++;
    }
    return ret;
}

void ParLV::PreMerge() {
    if (st_PreMerged == true) return;
    assert(num_par > 0);
    // assert(st_ParLved==true);
    if (st_ParLved == false) return;
    off_lved[0] = off_src[0] = 0;
    NV = NVl = NE = NElg = 0;
    max_NV = max_NVl = max_NE = max_NElg = 0;
    for (int p = 0; p < num_par; p++) {
        NV += par_lved[p]->NV;
        NVl += par_lved[p]->NVl;
        NE += par_lved[p]->NE;
        NElg += par_lved[p]->NElg;
        max_NV = max_NV > par_lved[p]->NV ? max_NV : par_lved[p]->NV;
        max_NVl = max_NVl > par_lved[p]->NVl ? max_NVl : par_lved[p]->NVl;
        max_NE = max_NE > par_lved[p]->NE ? max_NE : par_lved[p]->NE;
        max_NElg = max_NElg > par_lved[p]->NElg ? max_NElg : par_lved[p]->NElg;
        off_lved[p + 1] = NVl;
        off_src[p + 1] = off_src[p] + par_src[p]->NVl;
    }
    scl_NV = GetScl(max_NV);
    scl_NE = GetScl(max_NE);
    scl_NVl = GetScl(max_NVl);
    scl_NElg = GetScl(max_NElg);
    NV_gh = CheckGhost(); // + NVl;;
    NV = NV_gh + NVl;
    elist = (edge*)malloc(sizeof(edge) * (NE));
    M_v = (long*)malloc(sizeof(long) * (NV));
    assert(M_v);
    assert(elist);
    memset(M_v, 0, sizeof(long) * (NV));
    NE_list_all = 0;
    NE_list_ll = 0;
    NE_list_gl = 0;
    NE_list_gg = 0;
    NV_list_all = 0;
    NV_list_l = 0;
    NV_list_g = 0;
    for (int p = 0; p < num_par; p++) {
        GLV* G_src = par_src[p];
        for (int v = 0; v < G_src->NVl; v++) {
            long base_src = off_src[p];
            long base_lved = off_lved[p];
            long c_src = G_src->C[v];
            long c_mg;
            if (c_src < par_lved[p]->NVl)
                c_mg = c_src + base_lved;
            else
                c_mg = p_v_new[p][c_src];
            G_src->C[v] = c_mg;
// M_v[v+base_lved] = c_mg;
#ifdef DBG_PAR_PRINT
            printf("DBGPREMG:p=%d  v=%d base_src=%d  base_lved=%d C1=%d, isLocal%d, c_mg=%d\n", p, v, base_src,
                   base_lved, G_src->C[v], c_src < par_lved[p]->NVl, c_mg);
#endif
        }
    }
    st_PreMerged = true;
}

int ParLV::AddGLV(GLV* plv) {
    assert(plv);
    par_src[num_par] = plv;
    num_par++;
    return num_par;
}

long ParLV::FindGhostInLocalC(long me) {
    long e_org = -me - 1;
    int idx = 0;
    // 1. find #p
    for (int p = 0; p < num_par; p++) {
        if (off_src[p] <= e_org && e_org < off_src[p + 1]) {
            idx = p;
            break;
        }
    }
    // 2.
    long address = e_org - off_src[idx];
    long m_src = par_src[idx]->M[address];
    // assert(m_org ==  m_src);
    long c_src = par_src[idx]->C[address];
    long c_src_m = c_src + off_lved[idx];
#ifdef PRINTINFO
    printf("e_org=%-4ld - %-4ld = address:%-4ld; c_src:%-4ld+off%-4ld=c_src_m%-4ld\n", e_org, off_src[idx], address,
           c_src, off_lved[idx], c_src_m);
#endif
    return c_src_m;
}

int ParLV::FindParIdx(long e_org) {
    int idx = 0;
    // 1. find #p
    for (int p = 0; p < num_par; p++) {
        if (off_src[p] <= e_org && e_org < off_src[p + 1]) {
            idx = p;
            break;
        }
    }
    return idx;
}

int ParLV::FindParIdxByID(int id) {
    if (!this->st_Partitioned) return -1;
    for (int p = 0; p < num_par; p++)
        if (this->par_lved[p]->ID == id) return p;
    if (!this->st_ParLved) return -1;
    for (int p = 0; p < num_par; p++)
        if (this->par_src[p]->ID == id) return p;
    return -1;
}

pair<long, long> ParLV::FindCM_1hop(int idx, long e_org) {
    // 2.
    pair<long, long> ret;
    long addr_v = e_org - off_src[idx];
    long c_src_sync = par_src[idx]->C[addr_v];
    long c_lved_new = c_src_sync; // key logic
    long m_lved_new = par_lved[idx]->M[c_lved_new];
    ret.first = c_lved_new;
    ret.second = m_lved_new;
    return ret;
}

pair<long, long> ParLV::FindCM_1hop(long e_org) {
    // 2.
    int idx = FindParIdx(e_org);
    pair<long, long> ret;
    long addr_v = e_org - off_src[idx];
    long c_src_sync = par_src[idx]->C[addr_v];
    long c_lved_new = c_src_sync; // key logic
    long m_lved_new = par_lved[idx]->M[c_lved_new];
    ret.first = c_lved_new;
    ret.second = m_lved_new;
    return ret;
}

long ParLV::FindC_nhop(long m_g) {
    assert(m_g < 0);
    long m_next = m_g;
    int cnt = 0;

    do {
        long e_org = -m_next - 1;
        int idx = FindParIdx(e_org);
        long v_src = e_org - off_src[idx]; // dbg
        pair<long, long> cm = FindCM_1hop(idx, e_org);
        long c_lved_new = cm.first;
        long m_lved_new = cm.second;
        cnt++;

        if (m_lved_new >= 0)
            return c_lved_new + off_lved[idx];
        else if (m_lved_new == m_g) {
            return m_g;
        } else { // m_lved_new<0;
            m_next = m_lved_new;
        }

    } while (cnt < 2 * num_par);
    return m_g; // no local community for the ghost which should be add as a new community
}

//#define DBG_PAR_PRINT
long FindOldOrAddNew(map<long, long>& map_v, long& NV, long v) {
    map<long, long>::iterator iter;
    int ret;
    iter = map_v.find(v);
    if (iter == map_v.end()) {
        ret = NV++; // add new
#ifdef DBG_PAR_PRINT
        printf("DBG_PAR_PRINT, new:%d ", ret);
#endif
    } else {
        ret = iter->second; // find old
#ifdef DBG_PAR_PRINT
        printf("DBG_PAR_PRINT, old:%d ", ret);
#endif
    }
    return ret;
}

long ParLV::CheckGhost() {
    long NV_gh_new = 0;
    for (int p = 0; p < num_par; p++) {
        GLV* G_src = par_src[p];
        GLV* G_lved = par_lved[p];
        long* vtxPtr = G_lved->G->edgeListPtrs;
        edge* vtxInd = G_lved->G->edgeList;
        p_v_new[p] = (long*)malloc(sizeof(long) * (G_lved->NV));
        assert(p_v_new[p]);
        for (int v = G_lved->NVl; v < G_lved->NV; v++) {
            long mv = G_lved->M[v];
            long v_new = FindC_nhop(mv);
            if (v_new == mv) {
                p_v_new[p][v] = FindOldOrAddNew(m_v_gh, NV_gh_new, v_new) + this->NVl;
#ifdef DBG_PAR_PRINT
                printf("CheckGhost: p=%-2d  v=%-6d mv=%-6d  v_new=%-6d NV_gh=%d\n", p, v, mv, p_v_new[p][v], NV_gh_new);
#endif
            } else {
                p_v_new[p][v] = v_new;
#ifdef DBG_PAR_PRINT
                printf("CheckGhost: p=%-2d  v=%-6d mv=%-6d  v_new=%-6d  isNVL%d\n", p, v, mv, v_new, v_new < this->NVl);
#endif
            }
        }
    }
    return NV_gh_new;
}

long ParLV::MergingPar2_ll() {
    // 1.create new edge list;
    long num_e_dir = 0;
    NEll = 0;
    NEself = 0;
    // long num_c_g   = 0;
    for (int p = 0; p < num_par; p++) {
        GLV* G_src = par_src[p];
        GLV* G_lved = par_lved[p];
        long* vtxPtr = G_lved->G->edgeListPtrs;
        edge* vtxInd = G_lved->G->edgeList;
        for (int v = 0; v < G_lved->NVl; v++) {
            assert(G_lved->M[v] >= 0);
            long adj1 = vtxPtr[v];
            long adj2 = vtxPtr[v + 1];
            int degree = adj2 - adj1;
            long off_local = off_lved[p];
            long v_new = v + off_local;
            long e_new;
            p_v_new[p][v] = v_new;
            // M_v[v_new] = v_new;
            for (int d = 0; d < degree; d++) {
                long e = vtxInd[adj1 + d].tail;
                long me = G_lved->M[e];
                bool isGhost = me < 0;
                if (v < e || isGhost) continue;
                e_new = e + off_local;
                double w = vtxInd[adj1 + d].weight;
                elist[num_e_dir].head = v_new;
                elist[num_e_dir].tail = e_new;
                elist[num_e_dir].weight = w;
                assert(v_new < this->NVl);
                assert(e_new < this->NVl);
                num_e_dir++;
                NEll++;
                if (v_new == e_new) NEself++;
#ifdef DBG_PAR_PRINT
                printf(
                    "LOCAL: p=%-2d v=%-8ld mv=%-8ld v_new=%-8ld, e=%-8ld me=%-8ld e_new=%-8ld w=%-3.0f NE=%-8ld "
                    "NEself=%-8ld NEll=%-8ld \n",
                    p, v, 0, v_new, e, me, e_new, w, num_e_dir, NEself, NEll);
#endif
                // AddEdge(elist, v, e, w, M_v);
            } // for d
        }     // for v
    }
    st_Merged_ll = true;
    return num_e_dir;
}

long ParLV::MergingPar2_gh() {
    long num_e_dir = 0;

    for (int p = 0; p < num_par; p++) {
        GLV* G_src = par_src[p];
        GLV* G_lved = par_lved[p];
        long* vtxPtr = G_lved->G->edgeListPtrs;
        edge* vtxInd = G_lved->G->edgeList;

        for (int v = G_lved->NVl; v < G_lved->NV; v++) {
            long mv = G_lved->M[v]; // should be uniqe in the all sub-graph
            assert(mv < 0);
            // combination of possible connection:
            // 1.1   C(head_gh)==Normal C and tail is local;
            // <local,
            // local>
            // 1.2   C(head_gh)==Normal C and tail is head_gh itself;
            // <local,
            // local>
            // 1.3.1 C(head_gh)==Normal C,and tail is other ghost in current sub graph and its C is normal
            // <local,
            // local>
            // 1.3.2 C(head_gh)==Normal C,and tail is other ghost in current sub graph and its C is other ghost <local,
            // m(tail_ghost)>,
            // 2     C(head_gh) is other ghost

            long v_new = p_v_new[p][v]; /*
         if(v_new <0 ){
                 if(isOnlyGL)
                         continue;
                 v_new = FindOldOrAddNew(m_v_gh, this->NV_gh, v_new);
                 v_new += this->NVl;// Allocated a new community for local
         }*/

            long adj1 = vtxPtr[v];
            long adj2 = vtxPtr[v + 1];
            int degree = adj2 - adj1;
            long off_local = off_lved[p];
            // trace v

            for (int d = 0; d < degree; d++) {
                double w = vtxInd[adj1 + d].weight;
                long e = vtxInd[adj1 + d].tail;
                long me = G_lved->M[e];
                bool isGhost = me < 0;
                if (v < e) continue;
                long e_new;
                if (me >= 0)
                    e_new = e + off_local;
                else if (me == mv) {
                    // assert (me == mv);
                    e_new = v_new;
                    NEself++;
                    NEgg++;
                } else {
                    e_new = p_v_new[p][e];
                }
                elist[NEll + num_e_dir].head = v_new;
                elist[NEll + num_e_dir].tail = e_new;
                elist[NEll + num_e_dir].weight = w;
                // M_v[v_new] = v_new;
                num_e_dir++;
#ifdef DBG_PAR_PRINT
                printf(
                    "GHOST: p=%-2d v=%-8ld mv=%-8ld v_new=%-8ld, e=%-8ld me=%-8ld e_new=%-8ld w=%-3.0f NE=%-8ld "
                    "NEself=%-8ld NEgg=%-8ld \n",
                    p, v, mv, v_new, e, me, e_new, w, num_e_dir, NEself, NEgg);
#endif
            }
            // 1 findDesC;
        }
    } // for sub graph ;

    st_Merged_gh = true;
    return num_e_dir;
}
GLV* ParLV::MergingPar2(int& id_glv) {
    long num_e_dir = 0;
    // CheckGhost();
    num_e_dir += MergingPar2_ll();
    num_e_dir += MergingPar2_gh();
    NE = num_e_dir;

    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GLV* glv = new GLV(id_glv);
    glv->SetName_ParLvMrg(num_par, plv_src->ID);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO: PAR\033[0m: NV( %-8d) = NVl ( %-8d) + NV_gh( %-8d) \n", NV, NVl, NV_gh);
    printf(
        "\033[1;37;40mINFO: PAR\033[0m: NE( %-8d) = NEll( %-8d) + NElg ( %-8d) + NEgl( %-8d) + NEgg( %-8d) + NEself( "
        "%-8d) \n",
        NE, NEll, NElg, NEgl, NEgg, NEself);
#endif
    GetGFromEdge_selfloop(Gnew, elist, this->NVl + this->NV_gh, num_e_dir);
    glv->SetByOhterG(Gnew);
    // glv->SetM(M_v);
    st_Merged = true;
    plv_merged = glv;
    return glv;
}

GLV* ParLV::FinalLouvain(char* opts_xclbinPath,
                         int numThreads,
                         int& id_glv,
                         long minGraphSize,
                         double threshold,
                         double C_threshold,
                         bool isParallel,
                         int numPhase) {
    if (st_Merged == false) return NULL;
    bool hasGhost = false;
    plv_final = LouvainGLV_general(hasGhost, this->flowMode, 0, plv_merged, opts_xclbinPath, numThreads, id_glv,
                                   minGraphSize, threshold, C_threshold, isParallel, numPhase);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO: PAR\033[0m: Community of plv_merged is updated\n");
#endif
    // assert(plv_merged->NV==plv_src->NV);
    for (int p = 0; p < num_par; p++) {
        for (long v_sub = 0; v_sub < par_src[p]->NVl; v_sub++) {
            long v_orig = v_sub + off_src[p];
            long v_merged = par_src[p]->C[v_sub];
            plv_src->C[v_orig] = plv_merged->C[v_merged];
#ifdef DBG_PAR_PRINT
            printf("DBG_FINAL: p=%d  v_sub=%d v_orig=%d v_final=%d C_final=%d\n", p, v_sub, v_orig, v_merged,
                   plv_src->C[v_orig]);
#endif
        }
    }
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO: PAR\033[0m: Community of plv_src is updated\n");
#endif
    st_FinalLved = true;
    return plv_final;
}
double ParLV::TimeStar() {
    return timesPar.time_star = getTime();
}
double ParLV::TimeDonePar() {
    return timesPar.time_done_par = getTime();
}
double ParLV::TimeDoneLv() {
    return timesPar.time_done_lv = getTime();
}
double ParLV::TimeDonePre() {
    return timesPar.time_done_pre = getTime();
}
double ParLV::TimeDoneMerge() {
    return timesPar.time_done_mg = getTime();
}
double ParLV::TimeDoneFinal() {
    timesPar.time_done_fnl = getTime();
    return timesPar.time_done_fnl;
}

double ParLV::TimeAll_Done() {
    timesPar.timePar_all = timesPar.time_done_par - timesPar.time_star;
    timesPar.timeLv_all = timesPar.time_done_lv - timesPar.time_done_par;
    timesPar.timePre = timesPar.time_done_pre - timesPar.time_done_lv;
    timesPar.timeMerge = timesPar.time_done_mg - timesPar.time_done_pre;
    timesPar.timeFinal = timesPar.time_done_fnl - timesPar.time_done_mg;
    timesPar.timeAll = timesPar.time_done_fnl - timesPar.time_star;
    return timesPar.timeAll;
}

void ParLV::PrintTime() {
    printf("\033[1;37;40mINFO\033[0m: Total time for partition orignal       : %lf\n", timesPar.timePar_all);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Louvain subs  : %lf\n", timesPar.timeLv_all);
    for (int d = 0; d < num_dev; d++) { // for parlv.timeLv_dev[d]
        printf("\033[1;37;40m    \033[0m: Total time for Louvain on dev-%1d        : %lf\t = ", d,
               timesPar.timeLv_dev[d]);
        for (int p = d; p < num_par; p += num_dev) printf("+ %3.4f ", timesPar.timeLv[p]);
        printf("\n");
    }
    printf("\033[1;37;40mINFO\033[0m: Total time for partition pre-Merge     : %lf\n", timesPar.timePre);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Merge         : %lf\n", timesPar.timeMerge);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Final Louvain : %lf\n", timesPar.timeFinal);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition All flow      : %lf\n", timesPar.timeAll);
}

void ParLV::PrintTime2() {
    // Final number of clusters       : 225
    // Final modularity               :
    printf("\033[1;37;40mINFO\033[0m: Final number of clusters               : %ld\n", plv_src->com_list.back().NC);
    printf("\033[1;37;40mINFO\033[0m: Final modularity                       : %lf\n", plv_src->com_list.back().Q);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition + Louvain     : %lf\n", timesPar.timePar_all);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition pre-Merge     : %lf\n", timesPar.timePre);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Merge         : %lf\n", timesPar.timeMerge);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Final Louvain : %lf\n", timesPar.timeFinal);
    printf("\033[1;37;40mINFO\033[0m: Total time for partition All flow      : %lf\n", timesPar.timeAll);
}
void ParLV::CleanTmpGlv() {
    for (int p = 0; p < num_par; p++) {
        delete (par_src[p]);
        delete (par_lved[p]);
    }
    delete (plv_merged);
}
////////////////////////////////////////////////////////////////////////////////////////////////

void ParLV_general_batch_thread(int flowMode,
                                GLV* plv_orig,
                                int id_dev,
                                int num_dev,
                                int num_par,
                                double* timeLv,
                                GLV* par_src[],
                                GLV* par_lved[],
                                char* xclbinPath,
                                int numThreads,
                                long minGraphSize,
                                double threshold,
                                double C_threshold,
                                bool isParallel,
                                int numPhase) {
    long vsize = plv_orig->NV / num_par;
    GLV* glv_t;
    int id_glv = id_dev * 64;

    for (int p = id_dev; p < num_par; p += num_dev) {
        double time_par = getTime();
        long start = p * vsize;
        long end = (p == num_par - 1) ? plv_orig->NV : start + vsize;
        GLV* tmp = par_general(plv_orig, id_glv, start, end, true, 1);
        par_src[p] = tmp;
        start = end;
    }

    for (int p = id_dev; p < num_par; p += num_dev) {
        double time1 = getTime();
        glv_t = LouvainGLV_general(true, flowMode, 0, par_src[p], xclbinPath, numThreads, id_glv, minGraphSize,
                                   threshold, C_threshold, isParallel, numPhase);
        par_lved[p] = glv_t;
        // pushList(glv_t);
        timeLv[p] = getTime() - time1;
    }
}

GLV* par_general(GLV* src, SttGPar* pstt, int& id_glv, long start, long end, bool isPrun, int th_prun) {
    GLV* des;
    if (isPrun) {
#ifdef PRINTINFO
        printf("\033[1;37;40mINFO\033[0m: Partition of \033[1;31;40mPruning\033[0m is used and th_maxGhost=%d \n",
               th_prun);
#endif
        des = pstt->ParNewGlv_Prun(src->G, start, end, id_glv, th_prun);
    } else {
#ifdef PRINTINFO
        printf("\033[1;37;40mINFO\033[0m: Partition of \033[1;31;40mNoraml\033[0m is used\n");
#endif
        des = pstt->ParNewGlv(src->G, start, end, id_glv);
    }
    des->SetName_par(des->ID, src->ID, start, end, isPrun ? 0 : th_prun);
    pstt->PrintStt();
    return des;
}

GLV* par_general(GLV* src, int& id_glv, long start, long end, bool isPrun, int th_prun) {
    SttGPar stt;
    return par_general(src, &stt, id_glv, start, end, isPrun, th_prun);
}

GLV* LouvainGLV_general_par(int flowMode,
                            ParLV& parlv,
                            char* xclbinPath,
                            int numThreads,
                            int& id_glv,
                            long minGraphSize,
                            double threshold,
                            double C_threshold,
                            bool isParallel,
                            int numPhase) {
    parlv.TimeStar();
    long vsize = parlv.plv_src->NV / parlv.num_par;
    long start = 0;
    long end = start + vsize;
    // ParLV parlv;
    // parlv.Init(glv_src, num_par, num_dev);//should never release resource and object who pointed; Just work as a
    // handle
    for (int p = 0; p < parlv.num_par; p++) {
        TimePointType l_par_start = chrono::high_resolution_clock::now();
        TimePointType l_par_end;
        GLV* tmp = par_general(parlv.plv_src, &(parlv.stt[p]), id_glv, start, end, parlv.isPrun, parlv.th_prun);
        getDiffTime(l_par_start, l_par_end, parlv.timesPar.timePar[p]);
        parlv.par_src[p] = tmp;
        start = end;
        end = (p == parlv.num_par - 2) ? parlv.plv_src->NV : start + vsize;
    }
    parlv.st_Partitioned = true;
    parlv.TimeDonePar();

#ifndef _SINGLE_THREAD_MULTI_DEV_
    std::thread td[parlv.num_dev];
    {
        for (int dev = 0; dev < parlv.num_dev; dev++) {
            parlv.timesPar.timeLv_dev[dev] = getTime();
            bool hasGhost = true;
            td[dev] = std::thread(LouvainGLV_general_batch_thread, hasGhost, flowMode, dev, id_glv, parlv.num_dev,
                                  parlv.num_par, parlv.timesPar.timeLv, parlv.par_src, parlv.par_lved, xclbinPath,
                                  numThreads, minGraphSize, threshold, C_threshold, isParallel, numPhase);
        }

        for (int dev = 0; dev < parlv.num_dev; dev++) {
            td[dev].join();
            parlv.timesPar.timeLv_dev[dev] = getTime() - parlv.timesPar.timeLv_dev[dev];
        }
    }
#else
    {
        for (int dev = 0; dev < parlv.num_dev; dev++)
            parlv.timeLv_dev[dev] = LouvainGLV_general_batch(
                dev, parlv.num_dev, parlv.num_par, parlv.timeLv, parlv.par_src, parlv.par_lved, xclbinPath, numThreads,
                id_glv, minGraphSize, threshold, C_threshold, isParallel, numPhase);
    }
#endif
    parlv.st_ParLved = true;

    parlv.TimeDoneLv();

    parlv.PreMerge();

    parlv.TimeDonePre();

    parlv.MergingPar2(id_glv);

    parlv.TimeDoneMerge();

    GLV* glv_final =
        parlv.FinalLouvain(xclbinPath, numThreads, id_glv, minGraphSize, threshold, C_threshold, isParallel, numPhase);

    parlv.TimeDoneFinal();

    return glv_final;
}

GLV* LouvainGLV_general_par_OneDev(int flowMode,
                                   ParLV& parlv,
                                   char* xclbinPath,
                                   int numThreads,
                                   int& id_glv,
                                   long minGraphSize,
                                   double threshold,
                                   double C_threshold,
                                   bool isParallel,
                                   int numPhase) {
    parlv.TimeStar();
    long vsize = parlv.plv_src->NV / parlv.num_par;
    long start = 0;
    long end = start + vsize;
    // ParLV parlv;
    // parlv.Init(glv_src, num_par, num_dev);//should never release resource and object who pointed; Just work as a
    // handle
    for (int p = 0; p < parlv.num_par; p++) {
        double time_par = getTime();
        GLV* tmp = par_general(parlv.plv_src, &(parlv.stt[p]), id_glv, start, end, parlv.isPrun, parlv.th_prun);
        parlv.timesPar.timePar[p] = getTime() - time_par;
        parlv.par_src[p] = tmp;
        start = end;
        end = (p == parlv.num_par - 2) ? parlv.plv_src->NV : start + vsize;
    }
    parlv.st_Partitioned = true;
    parlv.TimeDonePar();

    const int id_dev = 0;
#ifdef PRINTINFO
    printf("INFO: using one device(%2d) for Louvain \n", id_dev);
#endif
    parlv.timesPar.timeLv_dev[id_dev] = getTime();
    for (int p = 0; p < parlv.num_par; p++) {
        double time1 = getTime();
        int id_glv_dev = id_glv + p;
        bool hasGhost = true;
        GLV* glv_t = LouvainGLV_general(hasGhost, flowMode, id_dev, parlv.par_src[p], xclbinPath, numThreads,
                                        id_glv_dev, minGraphSize, threshold, C_threshold, isParallel, numPhase);
        parlv.par_lved[p] = glv_t;
        // pushList(glv_t);
        parlv.timesPar.timeLv[p] = getTime() - time1;
    }
    parlv.timesPar.timeLv_dev[id_dev] = getTime() - parlv.timesPar.timeLv_dev[id_dev];

    parlv.st_ParLved = true;

    parlv.TimeDoneLv();

    parlv.PreMerge();

    parlv.TimeDonePre();

    parlv.MergingPar2(id_glv);

    parlv.TimeDoneMerge();

    GLV* glv_final =
        parlv.FinalLouvain(xclbinPath, numThreads, id_glv, minGraphSize, threshold, C_threshold, isParallel, numPhase);

    parlv.TimeDoneFinal();

    return glv_final;
}

void BackAnnotateC(int num_par,
                   GLV* par_src[],
                   long* off_src,
                   long* C_mg_lved,
                   // OUTPUT
                   GLV* plv_src) {
    for (int p = 0; p < num_par; p++) {
        for (long v_sub = 0; v_sub < par_src[p]->NVl; v_sub++) {
            long v_orig = v_sub + off_src[p];
            long v_merged = par_src[p]->C[v_sub];
            plv_src->C[v_orig] = C_mg_lved[v_merged];
        }
    }
}

void BackAnnotateC(ParLV& parlv,
                   // OUTPUT
                   GLV* plv_src) {
    BackAnnotateC(parlv.num_par, parlv.par_src, parlv.off_src, parlv.plv_merged->C, plv_src);
}
void BackAnnotateC(ParLV& parlv) {
    BackAnnotateC(parlv.num_par, parlv.par_src, parlv.off_src, parlv.plv_merged->C, parlv.plv_src);
}

void ParLouvainMeger(int flowMode,
                     ParLV& parlv,
                     char* xclbinPath,
                     int numThreads,
                     int& id_glv,
                     long minGraphSize,
                     double threshold,
                     double C_threshold,
                     bool isParallel,
                     int numPhase) {
    parlv.TimeStar();
    long vsize = parlv.plv_src->NV / parlv.num_par;

    // ParLV parlv;
    // parlv.Init(glv_src, num_par, num_dev);//should never release resource and object who pointed; Just work as a
    // handle

    const int id_dev = 0;

    parlv.timesPar.timeLv_dev[id_dev] = getTime();

    const char* path_driver = "./";
    //#pragma omp parallel for
    for (int p = 0; p < parlv.num_par; p++) {
        long start = vsize * p;
        long end = (p == parlv.num_par - 1) ? parlv.plv_src->NV : (start + vsize);
#ifdef PRINTINFO
        printf("INFO: start 1 partition for Louvain \n");
#endif
        double time_par = getTime();
        GLV* tmp = par_general(parlv.plv_src, &(parlv.stt[p]), id_glv, start, end, parlv.isPrun, parlv.th_prun);
        parlv.timesPar.timePar[p] = getTime() - time_par;
        parlv.par_src[p] = tmp;
    }
    parlv.st_Partitioned = true;
    parlv.TimeDonePar();
    bool useInt = false;
    SaveGLVBinBatch(parlv.par_src, parlv.num_par, path_driver, useInt);

    // BEGIN: worker /////////////////////////////////////////////////////////////////////////////////////////////////
    int id_glv_wkr = 0;
    const char* path_worker = "./";
    int num_par_worker =
        DEMOzmq_get_num_par(parlv.num_par); // just demo function, should be changed when porting to real zmq
    char path_driver_rcv[1024];
    DEMOzmq_get_path(path_driver_rcv, path_driver);
    GLV* glv_par_src_worker[MAX_PARTITION];
    GLV* glv_par_lved_worker[MAX_PARTITION];
    // Worker: Loading from disk
    for (int p = 0; p < num_par_worker; p++) {
        char fileName[1024];
        DEMOzmq_get_name(fileName, parlv.par_src[p]->name); // just demo function;
        glv_par_src_worker[p] = LoadGLVBin(path_driver_rcv, fileName, id_glv_wkr);
        glv_par_src_worker[p]->SetName(strcat(fileName, "_wkr"));
        glv_par_src_worker[p]->printSimple();
    }
    // Worker: Running Louvain
    for (int p = 0; p < num_par_worker; p++) {
        bool hasGhost = true;
        glv_par_lved_worker[p] =
            LouvainGLV_general(hasGhost, flowMode, id_dev, glv_par_src_worker[p], xclbinPath, numThreads, id_glv_wkr,
                               minGraphSize, threshold, C_threshold, isParallel, numPhase);
    }
    // Worker: Saving result
    SaveGLVBinBatch_OnlyC(glv_par_src_worker, num_par_worker, path_worker);
    // bool useInt=false;
    SaveGLVBinBatch(glv_par_lved_worker, num_par_worker, path_worker, useInt);

    // END  worker ////////////////////////////////////////////////////////
    parlv.timesPar.timeLv_dev[id_dev] = getTime() - parlv.timesPar.timeLv_dev[id_dev];

    parlv.st_ParLved = true;

    // Driver: Loading from disk
    char path_work_rcv[1024];
    DEMOzmq_get_path(path_work_rcv, path_worker);
    for (int p = 0; p < num_par_worker; p++) {
        // load par_src
        char fileName_src[1024];
        DEMOzmq_get_name(fileName_src, glv_par_src_worker[p]->name); // just demo function
        // parlv.par_src[p] = LoadGLVBin(path_work_rcv, fileName_src, id_glv);
        LoadGLVBin_OnlyC(path_work_rcv, fileName_src, parlv.par_src[p]);
        // load par_lved
        char fileName_lved[1024];
        DEMOzmq_get_name(fileName_lved, glv_par_lved_worker[p]->name); // just demo function
        parlv.par_lved[p] = LoadGLVBin(path_work_rcv, fileName_lved, id_glv);
    }
#ifdef PRINTINFO
    parlv.PrintSelf();
#endif
    parlv.TimeDoneLv();

    parlv.PreMerge();

    parlv.TimeDonePre();

    parlv.MergingPar2(id_glv);

    parlv.TimeDoneMerge();
}

GLV* LouvainGLV_general_par(int mode,
                            GLV* glv_orig,
                            int num_par,
                            int num_dev,
                            int isPrun,
                            int th_prun,
                            char* xclbinPath,
                            int numThreads,
                            int& id_glv,
                            long minGraphSize,
                            double threshold,
                            double C_threshold,
                            bool isParallel,
                            int numPhase) {
    GLV* glv_final;
    const int n_dev = 1;

    // 0) Init
    ParLV parlv_1;
    parlv_1.Init(mode, glv_orig, num_par, n_dev, isPrun, th_prun);
    parlv_1.timesPar.timeAll = getTime();
    {
        // 1) created plv_merged
        ParLouvainMeger(mode, parlv_1, xclbinPath, numThreads, id_glv, minGraphSize, threshold, C_threshold, isParallel,
                        numPhase);
        // 3) updating plv_merged
        if (false) {
            parlv_1.plv_final =
                LouvainGLV_general(false, parlv_1.flowMode, 0, parlv_1.plv_merged, xclbinPath, numThreads, id_glv,
                                   minGraphSize, threshold, C_threshold, isParallel, numPhase);
            glv_final = parlv_1.plv_final;
        } else {
            // Init
            ParLV parlv_2;
            // created plv_merged
            parlv_2.Init(mode, parlv_1.plv_merged, 2, n_dev, isPrun, th_prun);
            ParLouvainMeger(mode, parlv_2, xclbinPath, numThreads, id_glv, minGraphSize, threshold, C_threshold,
                            isParallel, numPhase);
            // updating plv_merged
            parlv_2.plv_final =
                LouvainGLV_general(false, parlv_2.flowMode, 0, parlv_2.plv_merged, xclbinPath, numThreads, id_glv,
                                   minGraphSize, threshold, C_threshold, isParallel, numPhase);
// plv_merged -> plv_src
#ifdef PRINTINFO
            printf("MULTI-LEVEL PARTITION: \n");
#endif
            BackAnnotateC(parlv_2);
            glv_final = parlv_2.plv_final;
#ifdef PRINTINFO
            parlv_2.PrintSelf();
#endif
            parlv_2.TimeAll_Done();
#ifdef PRINTINFO
            parlv_2.PrintTime();
#endif
            parlv_2.CleanTmpGlv();
            parlv_1.plv_final = glv_final;
            parlv_1.plv_merged->NC = glv_final->NC;
            parlv_1.plv_merged->PushFeature(0, 0, 0.0, true);
        }

// plv_merged -> plv_src
#ifdef PRINTINFO
        printf("MULTI-LEVEL PARTITION: \n");
#endif
        BackAnnotateC(parlv_1);
    }
    parlv_1.timesPar.timeAll = getTime() - parlv_1.timesPar.timeAll;
#ifdef PRINTINFO
    parlv_1.PrintSelf();
    parlv_1.PrintTime();
#endif
    parlv_1.CleanTmpGlv();
    glv_orig->NC = glv_final->NC;
    glv_orig->PushFeature(0, 0, 0.0, true);
    return glv_final;
}

void SaveParLV(char* name, ParLV* p_parlv) {
    assert(name);
    FILE* fp = fopen(name, "wb");
    char* ptr = (char*)p_parlv;
    fwrite(ptr, sizeof(ParLVVar), 1, fp);
    fclose(fp);
}

void LoadParLV(char* name, ParLV* p_parlv) {
    assert(name);
    FILE* fp = fopen(name, "rb");
    char* ptr = (char*)p_parlv;
    int flowMode = p_parlv->flowMode;
    fread(ptr, sizeof(ParLVVar), 1, fp);
    p_parlv->flowMode = flowMode;
    fclose(fp);
}

void Louvain_thread_core(xf::graph::L3::Handle* handle0,
                         int flowMode,
                         GLV* glv_src,
                         GLV* glv,
                         bool opts_coloring,
                         long opts_minGraphSize,
                         double opts_threshold,
                         double opts_C_thresh,
                         int numThreads) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__ 
              << " numThreads=" << numThreads << " flowMode=" << flowMode << std::endl;
#endif
    xf::graph::L3::louvainModularity(handle0[0], flowMode, glv_src, glv, opts_coloring, opts_minGraphSize,
                                     opts_threshold, opts_C_thresh, numThreads);
}

GLV* L3_LouvainGLV_general(int& id_glv,
                           xf::graph::L3::Handle* handle0,
                           int flowMode,
                           GLV* glv_src,
                           bool opts_coloring,
                           long opts_minGraphSize,
                           double opts_threshold,
                           double opts_C_thresh,
                           int numThreads) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FUNCTION__ << " numThreads=" << numThreads << std::endl;
#endif
    GLV* glv_iter;
    std::thread td;

    glv_iter = glv_src->CloneSelf(id_glv);
    assert(glv_iter);
    glv_iter->SetName_lv(glv_iter->ID, glv_src->ID);

    td = std::thread(Louvain_thread_core, handle0, flowMode, glv_src, glv_iter, opts_coloring, opts_minGraphSize,
                     opts_threshold, opts_C_thresh, numThreads);
    td.join();
    return glv_iter;
}

/*

*/
void Server_SubLouvain(xf::graph::L3::Handle* handle0,
                       ParLV& parlv,
                       int& id_glv,
                       bool opts_coloring,
                       long opts_minGraphSize,
                       double opts_threshold,
                       double opts_C_thresh,
                       int numThreads) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__ 
              << " handle0=" << handle0 << " parlv.num_par=" << parlv.num_par 
              << " parlv.num_dev=" << parlv.num_dev
              << std::endl;
#endif
    parlv.timesPar.timeLv_all = getTime();
    GLV* glv[parlv.num_par];
    std::thread td[parlv.num_par];
    for (int p = 0; p < parlv.num_par; p++) {
        glv[p] = parlv.par_src[p]->CloneSelf(id_glv);
    }
    int parCnt = 0;

    while (parCnt < parlv.num_par) {
        // Can only launch as many threads as numDevices
        for (int dev=0; dev < parlv.num_dev; dev++) {
            std::cout << "INFO:     start Louvain_thread_core thread " << parCnt+dev
                      << std::endl; 
            parlv.timesPar.timeLv[parCnt+dev] = getTime();
            assert(glv[parCnt+dev]);
            td[parCnt+dev] = std::thread(Louvain_thread_core, handle0, parlv.flowMode, 
                                         parlv.par_src[parCnt+dev], glv[parCnt+dev], opts_coloring,
                                         opts_minGraphSize, opts_threshold, opts_C_thresh, numThreads);
            parlv.par_lved[parCnt+dev] = glv[parCnt+dev];
            char tmp_name[1024];
            strcpy(tmp_name, parlv.par_src[parCnt+dev]->name);
            parlv.par_lved[parCnt+dev]->SetName(strcat(tmp_name, "_wrk_lv"));
        }
        
        for (int dev=0; dev < parlv.num_dev; dev++) {
            td[parCnt+dev].join();
            parlv.timesPar.timeLv[parCnt+dev] = getTime() - parlv.timesPar.timeLv[parCnt+dev];
        }
        parCnt += parlv.num_dev;
    }

    parlv.st_ParLved = true;
    parlv.timesPar.timeLv_all = getTime() - parlv.timesPar.timeLv_all;
}

GLV* Driver_Merge_Final(xf::graph::L3::Handle* handle0,
                        ParLV& parlv,
                        int& id_glv,
                        bool opts_coloring,
                        long opts_minGraphSize,
                        double opts_threshold,
                        double opts_C_thresh,
                        int numThreads) {
    parlv.timesPar.timePre = getTime();
    parlv.PreMerge();
    parlv.timesPar.timePre = getTime() - parlv.timesPar.timePre;
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m: Total time for partition pre-Merge : %lf\n", parlv.timesPar.timePre);
#endif
    parlv.timesPar.timeMerge = getTime();
    parlv.MergingPar2(id_glv);
    parlv.timesPar.timeMerge = getTime() - parlv.timesPar.timeMerge;
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Merge : %lf\n", parlv.timesPar.timeMerge);
#endif
    parlv.timesPar.timeFinal = getTime();

// parlv.PrintSelf();
#ifdef PRINTINFO
    parlv.plv_merged->printSimple();
#endif
    if (parlv.st_Merged == false) return NULL;
    assert(parlv.plv_merged);
    GLV* glv_final = parlv.plv_merged->CloneSelf(id_glv);
    assert(glv_final);
    glv_final->SetName_lv(glv_final->ID, parlv.plv_merged->ID);

    // parlv.timesPar.timeFinal = getTime();
    Louvain_thread_core(handle0, parlv.flowMode, parlv.plv_merged, glv_final, opts_coloring, opts_minGraphSize,
                        opts_threshold, opts_C_thresh, numThreads);
    for (int p = 0; p < parlv.num_par; p++) {
        for (long v_sub = 0; v_sub < parlv.par_src[p]->NVl; v_sub++) {
            long v_orig = v_sub + parlv.off_src[p];
            long v_merged = parlv.par_src[p]->C[v_sub];
            parlv.plv_src->C[v_orig] = parlv.plv_merged->C[v_merged];
        }
    }
    parlv.timesPar.timeFinal = getTime() - parlv.timesPar.timeFinal;
#ifdef PRINTINFO
    printf("Total time for final : %lf \n", parlv.timesPar.timeFinal);
#endif
    parlv.plv_src->PushFeature(0, 0, 0.0, true);

    return glv_final;
}

void LouvainProcess_part1(int& nodeID, ParLV& parlv, char* tmp_msg_d2w, ParLV& parlv_wkr) 
{
#ifndef NDEBUG
    std::cout << "DEBUG:" << __FILE__ << "::" << __FUNCTION__ 
              << " parlv_wkr num_par=" << parlv_wkr.num_par << std::endl;
#endif

    // this will be initialized by message again
    parlv_wkr.Init(parlv.flowMode, NULL, parlv.num_par, parlv.num_dev, parlv.isPrun, parlv.th_prun);
    char path_driver[1024];
    char names[32][256];

    MessageParser_D2W(tmp_msg_d2w, parlv_wkr, path_driver, names, nodeID);

    // parlv.PrintSelf();
    // worker: load file////////////////
    TimePointType l_load_start = chrono::high_resolution_clock::now();
    TimePointType l_load_end;

    int id_glv_wkr = 0;
    char* path_worker = (char*)"./";
    // Worker: Loading from disk
    for (int p = 0; p < parlv_wkr.num_par; p++) {
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Loading partition file %s%s \033[0m\n",  path_driver, names[p]);
#endif
        parlv_wkr.par_src[p] = LoadGLVBin(path_driver, names[p], id_glv_wkr);
        parlv_wkr.par_src[p]->SetName(names[p]);
        parlv_wkr.st_Partitioned = true;
#ifdef PRINTINFO
        parlv_wkr.par_src[p]->printSimple();
#endif
    }

    getDiffTime(l_load_start, l_load_end, parlv_wkr.timesPar.timeWrkLoad[0]);
} 

void LouvainProcess_part2(int nodeID,
                          xf::graph::L3::Handle* handle0,
                          long opts_minGraphSize,
                          double opts_threshold,
                          double opts_C_thresh,
                          int numThreads,
                          char* tmp_msg_w2d,
                          ParLV& parlv_wkr,
                          Node* worker_node) {
    int id_glv = 0;
    bool opts_coloring = true;
    char* path_worker = (char*)"./";
    // Louvain
    TimePointType l_compute_start = chrono::high_resolution_clock::now();
    TimePointType l_compute_end;
    Server_SubLouvain(handle0, parlv_wkr, id_glv, opts_coloring, opts_minGraphSize, opts_threshold, opts_C_thresh,
                      numThreads);
    getDiffTime(l_compute_start, l_compute_end, parlv_wkr.timesPar.timeWrkCompute[0]);

    // worker: send(save) file////////////////
    TimePointType l_send_start = chrono::high_resolution_clock::now();
    TimePointType l_send_end;

    // Worker: Saving result
    for (int i = 0; i < parlv_wkr.num_par; i++) {
        sendGLV_OnlyC(headGLVBin, worker_node, parlv_wkr.par_src[i]);
        sendGLV(headGLVBin, worker_node, parlv_wkr.par_lved[i]);
    }

    getDiffTime(l_send_start, l_send_end, parlv_wkr.timesPar.timeWrkSend[0]);

    MessageGen_W2D(tmp_msg_w2d, parlv_wkr, nodeID);

    parlv_wkr.st_ParLved = true;
#ifdef PRINTINFO
    parlv_wkr.PrintSelf();
#endif
} // mode_zmq==ZMQ_WORKER

void Driver_Partition(ParLV& parlv, int& id_glv) {
    long vsize = parlv.plv_src->NV / parlv.num_par;
    const int id_dev = 0;
    long start = 0;
    long end = start + vsize;
#pragma omp parallel for
    for (int p = 0; p < parlv.num_par; p++) {
        parlv.timesPar.timePar[p] = getTime();
        parlv.par_src[p] = par_general(parlv.plv_src, &(parlv.stt[p]), id_glv, start, end, parlv.isPrun, parlv.th_prun);
        start = end;
        end = (p == parlv.num_par - 2) ? parlv.plv_src->NV : start + vsize;
        parlv.timesPar.timePar[p] = getTime() - parlv.timesPar.timePar[p];
    }
    parlv.st_Partitioned = true;
}

GLV* UpdateCwithFinal(xf::graph::L3::Handle* handle0,
                      int flowMode,
                      GLV* glv_orig,
                      int num_dev,
                      bool isPrun,
                      int par_prune,
                      int& id_glv,
                      bool opts_coloring,
                      long opts_minGraphSize,
                      double opts_threshold,
                      double opts_C_thresh,
                      int numThreads) {
    const long MaxSize = 64000000;
    const long safeSize = MaxSize * 0.9;
    long NV = glv_orig->NV;
    long NE = glv_orig->NE;
#ifdef PRINTINFO
    printf("INFO:CHECK SIZE for glv_orig NV=%ld NE=%ld MaxSize=%ld\n", NV, NE, MaxSize);
#endif
    if (NV > MaxSize || NE > MaxSize) {
        int num_par = (NE + safeSize - 1) / safeSize;
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Now doing Partition for merged graph with num_par=%d \033[0m\n", num_par);
#endif
        ParLV parlv;
        parlv.Init(flowMode, glv_orig, num_par, num_dev, isPrun, par_prune);
        GLV* glv_final = LouvainGLV_general_top(handle0, parlv, id_glv, opts_coloring, opts_minGraphSize,
                                                opts_threshold, opts_C_thresh, numThreads);
        return glv_final;
    } else {
#ifdef PRINTINFO
        printf("INFO:CHECK SIZE for glv_orig successful, final Louvain will be done!\n");
#endif
        GLV* glv_final = glv_orig->CloneSelf(id_glv);
        glv_final->SetName_lv(glv_final->ID, glv_orig->ID);
        Louvain_thread_core(handle0, flowMode, glv_orig, glv_final, opts_coloring, opts_minGraphSize, opts_threshold,
                            opts_C_thresh, numThreads);
        return glv_final;
    }
}

GLV* Driver_Merge_Final_LoacalPar(xf::graph::L3::Handle* handle0,
                                  ParLV& parlv,
                                  int& id_glv,
                                  bool opts_coloring,
                                  long opts_minGraphSize,
                                  double opts_threshold,
                                  double opts_C_thresh,
                                  int numThreads) {
    parlv.timesPar.timePre = getTime();
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Now doing PreMerging... \033[0m\n");
#endif
    parlv.PreMerge();
    parlv.timesPar.timePre = getTime() - parlv.timesPar.timePre;
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m: Total time for partition pre-Merge : %lf\n", parlv.timesPar.timePre);
#endif
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Now doing Merging... \033[0m\n");
#endif
    parlv.timesPar.timeMerge = getTime();
    parlv.MergingPar2(id_glv);
    parlv.timesPar.timeMerge = getTime() - parlv.timesPar.timeMerge;
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m: Total time for partition Merge : %lf\n", parlv.timesPar.timeMerge);
#endif
    parlv.timesPar.timeFinal = getTime();

// parlv.PrintSelf();
#ifdef PRINTINFO
    parlv.plv_merged->printSimple();
#endif
    if (parlv.st_Merged == false) return NULL;
    assert(parlv.plv_merged);
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Now doing Final Louvain... \033[0m\n");
#endif
    /*GLV* glv_final = UpdateCwithFinal(handle0, parlv.flowMode,
                                      parlv.plv_merged, // C will be updated
                                      parlv.num_dev, parlv.isPrun, 1, id_glv, opts_coloring, opts_minGraphSize,
                                      opts_threshold, opts_C_thresh, numThreads);*/
    GLV* glv_final = parlv.plv_merged;//->CloneSelf(id_glv);
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Now doing BackAnnotationg... \033[0m\n");
#endif

    BackAnnotateC(parlv);
    // printf( "Total time for final : %lf \n" , parlv.timesPar.timeFinal);
    // parlv.plv_src->PushFeature(0, 0, 0.0, true);

    parlv.timesPar.timeFinal = getTime() - parlv.timesPar.timeFinal;

    return glv_final;
}

GLV* LouvainGLV_general_top(xf::graph::L3::Handle* handle0,
                            ParLV& parlv,
                            int& id_glv,
                            bool opts_coloring,
                            long opts_minGraphSize,
                            double opts_threshold,
                            double opts_C_thresh,
                            int numThreads) {
    GLV* glv_final;
    TimePointType l_start = chrono::high_resolution_clock::now();
    TimePointType l_end;
    {
        //////////Step-1:  partition the graph ////////////////////////////
        TimePointType l_par_start = chrono::high_resolution_clock::now();
        TimePointType l_par_end;
        Driver_Partition(parlv, id_glv);
        getDiffTime(l_par_start, l_par_end, parlv.timesPar.timePar_all);
        //////////Step-2: file save to disk ////////////////////////////
        parlv.timesPar.timeDriverSend = 0;
        //////////Step-3: worker: load file////////////////
        parlv.timesPar.timeWrkLoad[0] = 0;
        /////////////////////////////////

        //////////Step-4: Actual compute //////////////////////
        TimePointType l_compute_start = chrono::high_resolution_clock::now();
        TimePointType l_compute_end;
        Server_SubLouvain(handle0, parlv, id_glv, opts_coloring, opts_minGraphSize, opts_threshold, opts_C_thresh,
                          numThreads);
        getDiffTime(l_compute_start, l_compute_end, parlv.timesPar.timeWrkCompute[0]);
        //////////Step-5 save & send distribute results
        parlv.timesPar.timeWrkSend[0] = 0;
        ///////////////Step-6 Merge and Final louvain on driver ///////////////
        TimePointType l_merge_start = chrono::high_resolution_clock::now();
        TimePointType l_merge_end;
        glv_final = Driver_Merge_Final_LoacalPar(handle0, parlv, id_glv, opts_coloring, opts_minGraphSize,
                                                 opts_threshold, opts_C_thresh, numThreads);
        getDiffTime(l_merge_start, l_merge_end, parlv.timesPar.time_done_mg);
    }
    getDiffTime(l_start, l_end, parlv.timesPar.timeAll);
#ifdef PRINTINFO
    printf("Total time for final : %lf \n", parlv.timesPar.timeFinal);
    printf("Total time for all flow : %lf \n", parlv.timesPar.timeAll);
#endif
    parlv.plv_src->PushFeature(0, 0, 0.0, true);

    return glv_final;
}

/*
    TODO: delete the function below once new function is verified.
*/
int create_alveo_partitions_org(char* inFile, int par_num, int par_prune, char* pathName_proj, ParLV& parlv) {
    assert(inFile);
    assert(pathName_proj);
    int id_glv = 0;
    GLV* glv_src = CreateByFile_general(inFile, id_glv);
    if (glv_src == NULL) return -1;
    //////////////////////////// Set the name for partition project////////////////////////////
    char path_proj[1024];
    char name_proj[256];
    strcpy(name_proj, NameNoPath(pathName_proj));
    PathNoName(path_proj, pathName_proj);

#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project: path = %s name = %s\n", path_proj, name_proj);
#endif
    //////////////////////////// Initial data structure ////////////////////////////
    // ParLV parlv;
    parlv.plv_src = glv_src;
    parlv.num_par = par_num;
    parlv.th_prun = par_prune;
    //////////////////////////// partition ////////////////////////////
    parlv.timesPar.timePar_all = getTime();

    Driver_Partition(parlv, id_glv);

    parlv.timesPar.timePar_all = getTime() - parlv.timesPar.timePar_all;
    //////////////////////////// re-name and saving ////////////////////////////
    for (int p = 0; p < parlv.num_par; p++) {
        char nm[1024];
        sprintf(nm, "%s_%1d%1d%1d.par", name_proj, p / 100, (p / 10) % 10, p % 10);
        parlv.par_src[p]->SetName(nm);
    }
#ifdef PRINTINFO
    parlv.PrintSelf();
#endif
    bool useInt = false;
    TimePointType l_save_start = chrono::high_resolution_clock::now();
    TimePointType l_save_end;
    SaveGLVBinBatch(parlv.par_src, parlv.num_par, path_proj, useInt);
    getDiffTime(l_save_start, l_save_end, parlv.timesPar.timePar_save);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project: path=%s name=%s\n", path_proj, name_proj);
#endif
    //////////////////////////// save <metadata>.par file for loading //////////////////
    // Format: -create_alveo_partitions <inFile> -par_num <par_num> -par_prune <par_prun> -name <ProjectFile>
    char* meta = (char*)malloc(4096);
    char pathName_tmp[1024];
    sprintf(pathName_tmp, "%s%s.par.proj", path_proj, name_proj);
    sprintf(meta, "-create_alveo_partitions %s -par_num %d -par_prune %d -name %s -time_par %f -time_save %f\n", inFile,
            par_num, par_prune, pathName_proj, parlv.timesPar.timePar_all, parlv.timesPar.timePar_save);
    FILE* fp = fopen(pathName_tmp, "w");
    fwrite(meta, sizeof(char), strlen(meta), fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project Meta Data saved in file \033[1;37;40m%s\033[0m\n", pathName_tmp);
    printf(" \t\t Meta Data in file is: %s\n", meta);
#endif
    sprintf(pathName_tmp, "%s%s.par.parlv", path_proj, name_proj);
    SaveParLV(pathName_tmp, &parlv);
    sprintf(pathName_tmp, "%s%s.par.src", path_proj, name_proj);
    SaveHead<GLVHead>(pathName_tmp, (GLVHead*)parlv.plv_src);

    return 0;
}
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
){
	//printf("DBG_TGPAR: NV=%d NE=%d \n", G->numVertices, G->numEdges);
    long* off_glb   = G->edgeListPtrs; //in GSQL, maybe �degree� can be much easier.
    edge* edges_glb = G->edgeList;
    long  cnt_e     = 0;
    long  cnt_v     = 0;
    offsets_tg[0]   = off_glb[0 + start_vertex] - off_glb[start_vertex];//0;
    for(long v_glb = start_vertex; v_glb < end_vertex; v_glb++){//Scanning nodes within a range
        offsets_tg[cnt_v + 1] = off_glb[v_glb + 1 ] - off_glb[start_vertex];
        long degree = off_glb[ v_glb + 1] - off_glb[v_glb];
        for( long e = 0; e < degree; e++ ){
            edges_tg[cnt_e].head   = edges_glb[off_glb[v_glb] + e].head;
            edges_tg[cnt_e].tail   = edges_glb[off_glb[v_glb] + e].tail;
            edges_tg[cnt_e].weight = edges_glb[off_glb[v_glb] + e].weight;
            dgr_tail_tg[cnt_e]     = off_glb[edges_tg[cnt_e].tail+1] - off_glb[edges_tg[cnt_e].tail];
            cnt_e++;
        }
        cnt_v++;
    }//end for
}
GLV* par_general_4TG(long start_vertexInGlb, long* offsets_tg, edge* edgelist_tg, long* drglist_tg, long start_parInGlb, long stride_par, int& id_glv, int th_maxGhost){
	SttGPar stt;
	//printf("\033[1;37;40mINFO\033[0m: Partition of \033[1;31;40mPruning\033[0m is used and th_maxGhost=%d \n", th_maxGhost);
	//printf("DBG_4TG start_vertexInGlb=%d, start_parInGlb=%d, stride_par=%d\n", start_vertexInGlb, start_parInGlb, stride_par);
	GLV* ret=  stt.ParNewGlv_Prun(start_vertexInGlb,  offsets_tg, edgelist_tg, drglist_tg, start_parInGlb, stride_par, id_glv, th_maxGhost);
	return ret;
}


int xai_save_partition(long* offsets_tg, edge* edgelist_tg, long* drglist_tg,
		long  start_vertex,     // If a vertex is smaller than star_vertex, it is a ghost
		long  end_vertex,	    // If a vertex is larger than star_vertex-1, it is a ghost
		char* path_prefix,      // For saving the partition files like <path_prefix>_xxx.par
							    // Different server can have different path_prefix
		int par_prune,          // Can always be set with value '1'
		long NV_par_recommand,  // Allow to partition small graphs not bigger than FPGA limitation
		long NV_par_max		    //  64*1000*1000;
		) 
{
#ifndef NDEBUG    
    std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
    int status = 0;

	long NV_server = end_vertex - start_vertex;
	GLV*  parlv_par_src[MAX_PARTITION];
	int id_glv = 0;//fun2c_call();
	long average_stride = NV_par_recommand;
    long start_vertext_par = start_vertex;
    int p=0;
    while(start_vertext_par != end_vertex){
		long NV_par = (end_vertex - start_vertext_par ) > average_stride ? average_stride: end_vertex - start_vertext_par;
		do{
			parlv_par_src[p] = par_general_4TG(
				//Input data from TG
				start_vertex,
				offsets_tg,
				edgelist_tg,
				drglist_tg,
				//Partition parameters
				start_vertext_par,    // Start vertex for each partition
				NV_par,               // Total local vertex. Number of ghost vertex is not available until partition finish
				id_glv,               // ID counter's reference for GLV objects created. Any integer with any value is OK here;
				par_prune             // Ghost prunning parameter, always be '1'. It can be set by command-line
			);
			long diff_NV = NV_par_max - parlv_par_src[p]->NV;
			if(diff_NV <0){
				NV_par -= diff_NV;
				delete(parlv_par_src[p]);
				parlv_par_src[p] = NULL;
			}
		} while(parlv_par_src[p] == NULL);//If the partition is too big to load on FPGA, reduce the NV_par until partition is small enough
		char nm[1024];
		sprintf(nm, "_%1d%1d%1d.par", p / 100, (p / 10) % 10, p % 10);
		parlv_par_src[p]->SetName(nm);
		char pathName[1024];
		strcpy(pathName, path_prefix);
		strcat(pathName, nm);
		status = SaveGLVBin(pathName, parlv_par_src[p], false);
        if (status < 0) 
            return status;
		start_vertext_par += NV_par;
		p++;
	}
	return p;
}

/*
    Return value:
        -1: Cannot create partition files
*/
int create_alveo_partitions(char* inFile, int num_partition, int par_prune, char* pathName_proj, ParLV& parlv) 
{
    std::cout << "DEBUG: " << __FUNCTION__ << " inFile" << inFile 
              << " num_partition=" << num_partition << std::endl;

    int status;

    assert(inFile);
    assert(pathName_proj);
    int id_glv = 0;
    GLV* glv_src = CreateByFile_general(inFile, id_glv);
    if (glv_src == NULL) return -1;
    //////////////////////////// Set the name for partition project////////////////////////////
    char path_proj[1024];
    char name_proj[256];
    strcpy(name_proj, NameNoPath(pathName_proj));
    PathNoName(path_proj, pathName_proj);

#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project: path = %s name = %s\n", path_proj, name_proj);
#endif
    //////////////////////////// Initial data structure ////////////////////////////
    // ParLV parlv;
    parlv.plv_src = glv_src;
    parlv.num_par = num_partition;
    parlv.th_prun = par_prune;
    //////////////////////////// partition ////////////////////////////
    parlv.timesPar.timePar_all = getTime();

    long NV            = glv_src->NV;
	int num_server     = parlv.num_server;

	//For simulation: create server-level partition
	long start_vertex[3];
	long end_vertex[3];
	int vInServer[3];
	for(int i_svr=0; i_svr<num_server; i_svr++){
		start_vertex[i_svr] =  i_svr* (NV / num_server);
		if(i_svr!=num_server-1)
			end_vertex[i_svr] = start_vertex[i_svr] + NV / num_server;
		else
			end_vertex[i_svr] = NV;
		vInServer[i_svr] = end_vertex[i_svr] - start_vertex[3];
	}
	//For simulation: create server-level partition
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

		//For compatibility, when num_server is 1, no 'srv<n>' surfix used
		char pathName_proj_svr[1024];
		if(num_server>1)
			sprintf(pathName_proj_svr, "%s_svr%d", pathName_proj, i_svr);//louvain_partitions_svr0_000.par
		else
			strcpy(pathName_proj_svr, pathName_proj);                    //louvain_partitions_000.par

		long NV_par_recommand;//(long)(64000000.0 * 0.80);
		long NV_par_max = 64*1000*1000;
		if(parlv.num_par>1)
			NV_par_recommand = (NV + parlv.num_par-1) / parlv.num_par;//allow to partition small graph with -par_num
		else
			NV_par_recommand = (long)((float)NV_par_max * 0.80);//20% space for ghosts.
		parInServer[i_svr] = xai_save_partition(
				offsets_tg,  edgelist_tg,  drglist_tg,
    			start_vertex[i_svr],
				end_vertex[i_svr],
				pathName_proj_svr, // num_server==1? <dir>/louvain_partitions_ : louvain_partitions_svr<num_server>
				par_prune,         // always be '1'
				NV_par_recommand,  // Allow to partition small graphs not bigger than FPGA limitation
				NV_par_max		   //  64*1000*1000;
    			);
        if (parInServer[i_svr] < 0)
            return -1;

		num_partition +=parInServer[i_svr] ;
    	free(offsets_tg);
    	free(edgelist_tg);
    	free(drglist_tg);
    }
    parlv.st_Partitioned = true;
    parlv.timesPar.timePar_all = getTime() - parlv.timesPar.timePar_all;

    //////////////////////////// save <metadata>.par file for loading //////////////////
    // Format: -create_alveo_partitions <inFile> -par_num <par_num> -par_prune <par_prun> -name <ProjectFile>
    char* meta = (char*)malloc(4096);
    char pathName_tmp[1024];
    sprintf(pathName_tmp, "%s%s.par.proj", path_proj, name_proj);
    sprintf(meta, "-create_alveo_partitions %s -par_num %d -par_prune %d -name %s -time_par %f -time_save %f ", inFile,
    		num_partition, par_prune, pathName_proj, parlv.timesPar.timePar_all, parlv.timesPar.timePar_save);
    ///////////////////////////////////////////////////////////////////////
    //adding : -server_par <num_server> <num_par on server0> �..<num_par on server?>
    char tmp_str[128];
    sprintf(tmp_str, "-server_par %d ",  num_server);
    strcat(meta, tmp_str);
    for(int i_svr = 0; i_svr < num_server; i_svr++){
    	 sprintf(tmp_str, "%d ",  parInServer[i_svr]);
    	 strcat(meta, tmp_str);
    }///////////////////////////////////////////////////////////////////////
    strcat(meta, "\n");

    FILE* fp = fopen(pathName_tmp, "w");
    fwrite(meta, sizeof(char), strlen(meta), fp);
    fclose(fp);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project Meta Data saved in file \033[1;37;40m%s\033[0m\n", pathName_tmp);
    printf(" \t\t Meta Data in file is: %s\n", meta);
#endif
    sprintf(pathName_tmp, "%s%s.par.parlv", path_proj, name_proj);
    SaveParLV(pathName_tmp, &parlv);
    sprintf(pathName_tmp, "%s%s.par.src", path_proj, name_proj);
    SaveHead<GLVHead>(pathName_tmp, (GLVHead*)parlv.plv_src);

    return 0;
}

int Parser_ParProjFile(std::string projFile, ParLV& parlv, char* path, char* name, char* name_inFile) {
    // Format: -create_alveo_partitions <inFile> -par_num <par_num> -par_prune <par_prune> -name <ProjectFile>
    assert(path);
    assert(name);
    assert(name_inFile);
    FILE* fp = fopen(projFile.c_str(), "r");
    if (fp == NULL) {
        printf("\033[1;31;40mERROR\033[0m: Project Metadata file %s failed for open \n", projFile.c_str());
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* fdata = (char*)malloc(fsize * sizeof(char));
    assert(fdata);
    fread(fdata, fsize, sizeof(char), fp);
    fclose(fp);
    myCmd ps;
    ps.cmd_Getline(fdata);
    if (ps.argc < 8) {
        printf("\033[1;31;40mERROR\033[0m: Partition Project Meta wrong\n");
        free(fdata);
        return -1;
    }


    if (strcmp("-create_alveo_partitions", ps.argv[0]) != 0) {
        printf("\033[1;31;40mERROR\033[0m: MessageParser_D2W: Unknow head%s. -create_alveo_partitions is required\n",
               ps.argv[0]);
        free(fdata);
        return -1;
    }
    strcpy(name_inFile, ps.argv[1]);
    char* str_par_num = ps.argv[3];
    char* str_par_prune = ps.argv[5];
    char* nameProj = ps.argv[7];
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: The graph matrix is   : %s \033[0m\n",  name_inFile);
#endif
    if (ps.argc > 9) {
        parlv.timesPar.timePar_all = atof(ps.argv[9]);
        parlv.timesPar.timePar_save = atof(ps.argv[11]);
    } else {
        parlv.timesPar.timePar_all = 0;
        parlv.timesPar.timePar_save = 0;
    }
    //////////////////////////////////////////////////////////////////////////
    //for multi-server partition [-server_par <num_server> <num_par on server0> �..<num_par on server?>]
    int idx_server = ps.cmd_findPara("-server_par");
    if (idx_server > -1){
    	parlv.num_par = 0;
    	parlv.num_server = atoi(ps.argv[idx_server+1]);
    	for(int i_svr = 0; i_svr < parlv.num_server; i_svr++){
    		parlv.parInServer[i_svr] = atoi(ps.argv[idx_server+ 2 + i_svr]);
    		parlv.num_par += parlv.parInServer[i_svr];
    	}
    }
    ///////////////////////////////////////////////////////////////////////////
    if(nameProj[0]=='/')
    	PathNoName(path, nameProj);//Absolute name used in project file, so abstract the path use project name
    else
    	PathNoName(path, projFile.c_str());//Use project file path for abstraction the path
    strcpy(name, NameNoPath(nameProj));
    int id_glv = 0;
    char namePath_tmp[1024];
    sprintf(namePath_tmp, "%s/%s.par.parlv", path, name);
    LoadParLV(namePath_tmp, &parlv);
    sprintf(namePath_tmp, "%s/%s.par.src", path, name);
    parlv.plv_src = new GLV(id_glv);
    LoadHead<GLVHead>(namePath_tmp, (GLVHead*)parlv.plv_src);
#ifdef PRINTINFO
    printf("\033[1;37;40mINFO\033[0m:Partition Project: path = %s name = %s\n", path, name);
#endif
    if (parlv.num_par != atoi(str_par_num)) {
        printf("\033[1;31;40mWARNING\033[0m: parlv.num_par(%d) != %d; Value in file will be used\n", parlv.num_par, atoi(str_par_num));
        //getchar();
    }
    parlv.num_par = atoi(str_par_num);
    parlv.th_prun = atoi(str_par_prune);

    int cnt_file = 0;
    //for (int p = 0; p < parlv.num_par; p++)
    int p=0;
    int i_svr=0;
    int p_svr=0;
    while (p < parlv.num_par){
        parlv.par_src[p] = new GLV(id_glv);
        char nm[1024];
        if(parlv.num_server==1)
        	sprintf(nm, "%s_%1d%1d%1d.par", name, p / 100, (p / 10) % 10, p % 10);
        else{
        	sprintf(nm, "%s_svr%d_%1d%1d%1d.par", name,i_svr, p_svr / 100, (p_svr / 10) % 10, p_svr % 10);
        }
        parlv.par_src[p]->SetName(nm);
#ifndef NDEBUG        
        printf("Checking: %s\n", parlv.par_src[p]->name);
#endif        
        char pathName[1024];
        strcpy(pathName, path);
        FILE* fp = fopen(strcat(pathName, parlv.par_src[p]->name), "r");
        if (fp != NULL) {
            fclose(fp);
            cnt_file++;
        } else
            printf("\033[1;31;40mERROR\033[0m: Partition Share %s not Found\n", pathName);

        if(p_svr==parlv.parInServer[i_svr]-1){
            i_svr++;
            p_svr=0;
        }else
        	p_svr++;
        p++;
    }//
    free(fdata);
    if (cnt_file != parlv.num_par)
        return -1;
    else
        return 0;
}

/*
    For both driver; no zmq communications occur
*/
int load_alveo_partitions_DriverSelf( 
    std::string projFile, int numNode, int numPureWorker, 
    // output
    ParLV& parlv, ParLV& parlv_wkr, char* name_inFile) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FILE__ << "::" << __FUNCTION__ 
              << " numNode=" << numNode << " numPureWorker=" << numPureWorker 
              << std::endl;
#endif

    // ParLV parlv;
    char path_proj[1024];
    char name_proj[1024];
    if (Parser_ParProjFile(projFile,  parlv, path_proj, name_proj, name_inFile) != 0) {
        printf("\033[1;31;40mERROR\033[0m: load_alveo_partitions Failed\n");
        return -1;
    }
    int id_glv = 0;

    char msg_d2w[numPureWorker][MAX_LEN_MESSAGE];
    char msg_driver[MAX_LEN_MESSAGE];
    { //////////////////////////// CreateSendMessage for driver itself ////////////////////////////
        TimePointType l_send_start = chrono::high_resolution_clock::now();
        TimePointType l_send_end;
        int stride = parlv.num_par / numNode;
        MessageGen_D2W(msg_driver, parlv, path_proj, stride * (numNode - 1), parlv.num_par, 0);
        getDiffTime(l_send_start, l_send_end, parlv.timesPar.timeDriverSend);
    }
    int nodeID = 0;
    char msg_w2d[numNode][MAX_LEN_MESSAGE];
    { LouvainProcess_part1(nodeID, parlv, msg_driver, parlv_wkr); }

    return 0;
}

int load_alveo_partitions_WorkerSelf( // for both driver; no zmq communications occur
    std::string projFile, int numNode, int numPureWorker,  
    // output
    ParLV& parlv, char* name_inFile, int nodeID, char* msg_worker) 
{
    char path_proj[1024];
    char name_proj[1024];
    if (Parser_ParProjFile(projFile, parlv, path_proj, name_proj, name_inFile) != 0) {
        printf("\033[1;31;40mERROR\033[0m: load_alveo_partitions Failed\n");
        return -1;
    }
    int id_glv = 0;
    char msg_d2w[numPureWorker][MAX_LEN_MESSAGE];
    { //////////////////////////// CreateSendMessage for driver itself ////////////////////////////
        TimePointType l_send_start = chrono::high_resolution_clock::now();
        TimePointType l_send_end;
        int stride = parlv.num_par / numNode;
#ifndef NDEBUG
        printf(" load_alveo_partitions_WorkerSelf %d,  %d, %d,  %d,   %d\n", parlv.num_par, numNode, stride * (nodeID - 1), stride * nodeID, nodeID);
#endif
        MessageGen_D2W(msg_worker, parlv, path_proj, stride * (nodeID - 1), stride * nodeID, nodeID);
        getDiffTime(l_send_start, l_send_end, parlv.timesPar.timeDriverSend);
    }
    return 0;
}

GLV* louvain_modularity_alveo(xf::graph::L3::Handle* handle0,
                              ParLV& parlv,     // To collect time and necessary data
                              ParLV& parlv_wkr, // Driver's self data for sub-louvain
                              long opts_minGraphSize,
                              double opts_threshold,
                              double opts_C_thresh,
                              int numThreads,
                              int numNode,
                              int numPureWorker,
                              char** nameWorkers // To enalbe all workers for Louvain
                              ) 
{
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FUNCTION__ << " handle0=" << handle0 << std::endl;
#endif
    int id_glv = 0;
    bool opts_coloring = true;
    int nodeID = 0;
    GLV* glv_final;

    TimePointType l_start = chrono::high_resolution_clock::now();
    TimePointType l_end;

    Driver* drivers = new Driver[numPureWorker];
    ConnectWorkers(drivers, numPureWorker, nameWorkers);

    enalbeAllWorkerLouvain(drivers, numPureWorker);

    TimePointType l_driver_collect_start = chrono::high_resolution_clock::now();
    TimePointType l_driver_collect_end;
    char msg_w2d[numNode][MAX_LEN_MESSAGE];
    { /////////////// Louvain process on both driver and worker ///////////////
        TimePointType l_par_start = chrono::high_resolution_clock::now();
        TimePointType l_par_end;
        int id_glv = 0;
        bool opts_coloring = true;
        char* path_worker = (char*)"./";
        // Louvain
        TimePointType l_compute_start = chrono::high_resolution_clock::now();
        TimePointType l_compute_end;
        Server_SubLouvain(handle0, parlv_wkr, id_glv, opts_coloring, opts_minGraphSize, opts_threshold, opts_C_thresh,
                          numThreads);
        getDiffTime(l_compute_start, l_compute_end, parlv_wkr.timesPar.timeWrkCompute[0]);

        // worker: send(save) file////////////////
        parlv_wkr.timesPar.timeWrkSend[0] = 0;

        MessageGen_W2D(msg_w2d[numNode - 1], parlv_wkr, nodeID);

        parlv_wkr.st_ParLved = true;
#ifdef PRINTINFO
        parlv_wkr.PrintSelf();
#endif
        getDiffTime(l_par_start, l_par_end, parlv.timesPar.timeLv_all);
    }

    { /////////////// Receive messages and Load file for final Louvain in driver ///////////////
        TimePointType l_receive_start = chrono::high_resolution_clock::now();
        TimePointType l_receive_end;
        // Receive and Parse  messages from worker and driver
        int num_par_sub = 0;
        int num_par_per_worker = parlv.num_par / numNode;

#pragma omp parallel for
        for (int i = 0; i < numPureWorker; i++) {
            for (int j = 0; j < num_par_per_worker; j++) {
                receiveGLV_OnlyC(&drivers[i], parlv.par_src[j + i * num_par_per_worker]);
                parlv.par_lved[j + i * num_par_per_worker] = receiveGLV(&drivers[i], id_glv);
#ifdef PRINTINFO
                printf("INFO: Received ID : %d\n", j + i * num_par_per_worker);
#endif
            }
            drivers[i].receive(msg_w2d[i], MAX_LEN_MESSAGE);
#ifdef PRINTINFO_2
            printf("INFO: Received from worker:%s (requester%d): %s\n", nameWorkers[i], i, msg_w2d[i]);
#endif
        }

        for (int i = 0; i < numNode; i++) {
            num_par_sub += MessageParser_W2D(i, msg_w2d[i], parlv);
        }

        for (int p = numPureWorker * num_par_per_worker; p < num_par_sub; p++) {
            parlv.par_src[p] = parlv_wkr.par_src[p - numPureWorker * num_par_per_worker];
            parlv.par_src[p]->ID = p + 1;

            parlv.par_lved[p] = parlv_wkr.par_lved[p - numPureWorker * num_par_per_worker];
            parlv.par_lved[p]->ID = p;
        }
        parlv.num_server = numNode;

        parlv.st_ParLved = true;
#ifdef PRINTINFO
        parlv.PrintSelf();
#endif
        delete[] drivers;
        getDiffTime(l_receive_start, l_receive_end, parlv.timesPar.timeDriverRecv);
    }
    getDiffTime(l_driver_collect_start, l_driver_collect_end, parlv.timesPar.timeDriverCollect);
    { /////////////// Merge and Final louvain on driver ///////////////
        TimePointType l_merge_start = chrono::high_resolution_clock::now();
        TimePointType l_merge_end;
        if (parlv.plv_src->C != NULL) free(parlv.plv_src->C);
        parlv.plv_src->C = (long*)malloc(sizeof(long) * parlv.plv_src->NV);

        glv_final = Driver_Merge_Final_LoacalPar(handle0, parlv, id_glv, opts_coloring, opts_minGraphSize,
                                                 opts_threshold, opts_C_thresh, numThreads);

        getDiffTime(l_merge_start, l_merge_end, parlv.timesPar.time_done_mg);
    }

    getDiffTime(l_start, l_end, parlv.timesPar.timeAll);

// parlv.plv_src->PushFeature(0, 0, 0.0, true);// calculate Q of results, no need to be included in total time
#ifdef PRINTINFO
    printf("Total time for all flow : %lf \n", parlv.timesPar.timeAll);
#endif
    return glv_final;
    // delete( glv_final);
}

extern "C" int create_alveo_partitions(int argc, char* argv[]) {
//    for (int i = 0; i < argc; ++i)
//        std::cout << "internal create partitions arg " << i << " = " << argv[i] << std::endl;
    //--------------- Parse Input parameters
#ifndef NDEBUG
    std::cout << "DEBUG: " << __FUNCTION__ << std::endl;
#endif
    double opts_C_thresh;   // Threshold with coloring on
    long opts_minGraphSize; // Min |V| to enable coloring
    double opts_threshold;  // Value of threshold
    int opts_ftype;         // File type
    char opts_inFile[4096];
    bool opts_coloring;
    bool opts_output;
    std::string opts_outputFile;
    bool opts_VF;
    char opts_xclbinPath[4096];

    int flow_fast = 2;
    int flowMode = 1;
    int num_par;
    bool isPrun = true;
    int par_prune = 1;
    int numThreads = NUMTHREAD; // using fixed number of thread instead of
    int devNeed_cmd = 3;
    int mode_zmq = ZMQ_NONE;
    char path_zmq[1024]; // default will be set as "./"
    bool useCmd = false;
    int mode_alveo = ALVEOAPI_NONE;
    char nameProj[1024];
    int numPureWorker;
    std::string nameMetaFile;
    char* nameWorkers[128];
    int nodeID;
    int server_par = 1;
    int status = 0;
    //int max_num_level;
    //int max_num_iter;
    host_ParserParameters(argc, argv, opts_C_thresh, opts_minGraphSize, opts_threshold, opts_ftype, opts_inFile,
                          opts_coloring, opts_output, opts_outputFile, opts_VF, opts_xclbinPath, numThreads, num_par,
                          par_prune, flow_fast, devNeed_cmd, mode_zmq, path_zmq, useCmd, mode_alveo, nameProj,
                          nameMetaFile, numPureWorker, nameWorkers, nodeID, server_par, glb_max_num_level, glb_max_num_iter);

    if (flow_fast == 1) {
        flowMode = 1; // normal kernel MD_NORMAL
    } else if (flow_fast == 2) {
        flowMode = 2; // fast kernel  MD_FAST
    } else {
        flowMode = 3; // fast renumber
    }
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: The graph matrix is   : %s \033[0m\n",  opts_inFile);
#endif
    if (mode_alveo == ALVEOAPI_PARTITION) {
        ParLV parlv;
        parlv.Init(flowMode, NULL, num_par, devNeed_cmd, isPrun, par_prune);
        parlv.num_server = server_par;
        status = create_alveo_partitions(opts_inFile, num_par, par_prune, nameProj, parlv);
        std::cout << "DEBUG: status=" << status << std::endl;
        if (status < 0)
            return status;

        printf("************************************************************************************************\n");
        printf("*****************************  Louvain Partition Summary   *************************************\n");
        printf("************************************************************************************************\n");

        std::cout << "Input graph                        : " << opts_inFile << std::endl;
        std::cout << "Number of servers                  : " << server_par << std::endl;
        std::cout << "Output Alveo partition project     : " << nameProj << std::endl;
        std::cout << "Number of partitions               : " << num_par << std::endl;
        printf("Time for partitioning the graph    : %lf = ",
               (parlv.timesPar.timePar_all + parlv.timesPar.timePar_save));
        printf(" partitioning +  saving \n");
        printf("    Time for partition             : %lf (s)\n",
               parlv.timesPar.timePar_all);
        printf("    Time for saving                : %lf (s)\n",
               parlv.timesPar.timePar_save);
        printf("************************************************************************************************\n");
#ifdef PRINTINFO
        printf("Deleting... \n");
#endif
        parlv.CleanTmpGlv();
        parlv.plv_src->printSimple();

        return 0;
    } else {
        printf("\033[1;31;40mERROR\033[0m: mode_alveo!=ALVEOAPI_PARTITION\n");
        return 1;
    }
    return 0;
}

void LouvainGLV_general_top_zmq_worker_new_part1(xf::graph::L3::Handle* handle0,
                                                 ParLV& parlv,
                                                 long opts_minGraphSize,
                                                 double opts_threshold,
                                                 double opts_C_thresh,
                                                 int numThreads,
                                                 int nodeID,
                                                 ParLV& parlv_wkr,
                                                 char* LoadCommand) {
#ifndef NDEBUG
    std::cout << "DEBUG:" << __FILE__ << "::" << __FUNCTION__ 
              << " parlv_wkr num_par=" << parlv_wkr.num_par 
              << " LoadCommand=" << LoadCommand << std::endl;
// LoadCommand= parlv_req -num 3 -path as-skitter-par9/  louvain_partitions_000.par 
//              louvain_partitions_001.par louvain_partitions_002.par -node 1              
#endif
    char MSG_LOAD_START[MAX_LEN_MESSAGE]; // 4096 usually// for dirver itself
    if (LoadCommand == NULL) {
        printf("Listen to the driver\n");
        Worker worker(5555);
        worker.receive(MSG_LOAD_START, 4096);
        printf("Received %s\n", MSG_LOAD_START);

        parlv.timesPar.timeAll = getTime();
        LouvainProcess_part1(nodeID, parlv, MSG_LOAD_START, parlv_wkr);
        char MSG_LOAD_DONE[MAX_LEN_MESSAGE]; // 4096 usually
        MessageGen_W2D(MSG_LOAD_DONE, nodeID);
        printf("MSG_LOAD_DONE to the driver\n");
        worker.send(MSG_LOAD_DONE, MAX_LEN_MESSAGE, 0);
    } else {
        strcpy(MSG_LOAD_START, LoadCommand);
        Worker worker(5555);
        // zmq_recv (responder, MSG_LOAD_START, 4096, 0);
        printf("Using Command for loading %s\n", MSG_LOAD_START);

        parlv.timesPar.timeAll = getTime();
        LouvainProcess_part1(nodeID, parlv, MSG_LOAD_START, parlv_wkr);
        char MSG_LOAD_DONE[MAX_LEN_MESSAGE]; // 4096 usually
        MessageGen_W2D(MSG_LOAD_DONE, nodeID);
        printf("\n\n\033[1;31;40mWorker-%d\033[0m LOADING DONE\n", nodeID);
        printf("MSG_LOAD_DONE to the driver\n");
        worker.receive(MSG_LOAD_START, 4096);
        worker.send(MSG_LOAD_DONE, MAX_LEN_MESSAGE, 0);
    }
    std::cout << "DEBUG: end of " << __FILE__ << "::" << __FUNCTION__ 
              << " parlv_wkr num_par=" << parlv_wkr.num_par << std::endl;
}

void LouvainGLV_general_top_zmq_worker_new_part2(xf::graph::L3::Handle* handle0,
                                                 long opts_minGraphSize,
                                                 double opts_threshold,
                                                 double opts_C_thresh,
                                                 int numThreads,
                                                 int nodeID,
                                                 ParLV& parlv_wkr) 
{
#ifndef NDEBUG
    std::cout << __FILE__ << "::" << __FUNCTION__ 
              << " parlv_wkr.num_par=" << parlv_wkr.num_par << std::endl;
#endif

    {
        char MSG_LV_START[MAX_LEN_MESSAGE]; // 4096 usually// for dirver itself
        char MSG_LV_DONE[MAX_LEN_MESSAGE];  // 4096 usually
        printf("Listen to the driver\n");
        Worker worker_node(5555);

        worker_node.receive(MSG_LV_START, 4096);
        MessageParser_D2W(MSG_LV_START);

        LouvainProcess_part2(nodeID, handle0, opts_minGraphSize, opts_threshold, opts_C_thresh, numThreads, MSG_LV_DONE,
                             parlv_wkr, &worker_node);

        worker_node.send(MSG_LV_DONE, MAX_LEN_MESSAGE, 0);
        parlv_wkr.timesPar.timeAll = getTime() - parlv_wkr.timesPar.timeAll;
        printf("***************************************\n");
        printf("INFO: Louvain process %d subgraph done!\n", parlv_wkr.num_par);
        printf("***************************************\n");
    }
#ifdef PRINTINFO
    printf("Total time for all flow : %lf \n", parlv_wkr.timesPar.timeAll);
#endif
}

int host_writeOut(const char* opts_inFile, long NV_begin, long* C_orig) {
    if ((opts_inFile == 0) || (C_orig == 0)) {
        printf("\033[1;31;40mERROR: Function host_writeOut got invalid input buff; Writing out "
            "results failed!\n\033[0m");
        return -1;
    }
    char outFile[4096];
    sprintf(outFile, "%s.clustInfo", opts_inFile);
#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: Cluster information will be stored in file: %s\n\033[0m", outFile);
#endif
    FILE* out = fopen(outFile, "w");
    for (long i = 0; i < NV_begin; i++) {
        fprintf(out, "%ld %ld\n", i, C_orig[i]);
    }
    fclose(out);
    return 0;
}

/*
    --load_alveo_partitions nameMetaFile
    -dev numDevices
    -fast -> flow_fast -> flowMode;
    -setwkr numPureWorker nameWorkers
    -workerAlone worker_number -> nodeID
*/

extern "C" float load_alveo_partitions(
    char* xclbinPath, bool flow_fast, unsigned int numDevices, 
    unsigned int num_par, char* alveoProject, 
    int mode_zmq, int numPureWorker, char* nameWorkers[128], unsigned int nodeID,
    char* opts_outputFile)
{
#ifndef NDEBUG    
    std::cout << "DEBUG: " << __FUNCTION__ <<  " xclbinPath=" << xclbinPath 
              << " flow_fast=" << flow_fast << " numDevices=" << numDevices 
              << " num_par=" << num_par << " alveoProject=" << alveoProject 
              << " mode_zmq=" << mode_zmq << " numPureWorker=" << numPureWorker
              << " nodeID=" << nodeID
              << " opts_outputFile=" << opts_outputFile
              << std::endl;

    for (int i=0; i<numPureWorker; i++)
        std::cout << "DEBUG: nameWorker " << i << "=" << nameWorkers[i] << std::endl;

#endif    
    int mode_alveo = ALVEOAPI_LOAD;
    std::string opName = "louvainModularity";
    std::string kernelName = "kernel_louvain";
    int requestLoad = 100;
    bool isPrun = true;
    const int cuNm = 1;

    double opts_C_thresh = 0.0002;   // Threshold with coloring on
    long opts_minGraphSize = 10; // Min |V| to enable coloring
    double opts_threshold = 0.000001;  // Value of threshold
    int par_prune = 1;
    int numThreads = 16;
    bool opts_coloring = false; 
    int status;
    int flowMode = 1;

    int numNode = numPureWorker + 1;
    if (flow_fast) {
        flowMode = 2; // fast kernel  MD_FAST
    } else {
        flowMode = 1; // normal kernel MD_NORMAL
    }

    xf::graph::L3::Handle::singleOP op0;
    xf::graph::L3::Handle handle0;
    //----------------- Set parameters of op0 again some of those will be covered by command-line
    op0.operationName = opName;
    op0.setKernelName((char*)kernelName.c_str());
    op0.requestLoad = requestLoad;
    op0.xclbinPath = xclbinPath;
    op0.numDevices = numDevices;
    std::cout << "INFO: numNode: " << numNode << std::endl;
    std::cout << "INFO: numDevices requested: " << op0.numDevices << std::endl;
    
    //----------------- enable handle0--------
    handle0.addOp(op0);
    status = handle0.setUp();

    if (status < 0)
        return status;

#ifdef PRINTINFO_2
    printf("\033[1;37;40mINFO: xclbin file is        : %s with flow mode %d\033[0m\n",  op0.xclbinPath.c_str(), flowMode);
    printf("\033[1;37;40mINFO: The project file is   : %s\033[0m\n", nameMetaFile);
#endif

    (handle0.oplouvainmod)->loadGraph(NULL, flowMode, opts_coloring, 
        opts_minGraphSize, opts_C_thresh, numThreads);

    if (mode_alveo == ALVEOAPI_LOAD) {
        if (mode_zmq == ZMQ_DRIVER) {
            //-----------------------------------------------------------------
            // DRIVER
            //-----------------------------------------------------------------
            char inFile[1024];
            ParLV parlv_drv;
            parlv_drv.Init(flowMode, NULL, num_par, numDevices, isPrun, par_prune);
            ParLV parlv_wkr;
            parlv_wkr.Init(flowMode, NULL, num_par, numDevices, isPrun, par_prune);

            // API FOR LOADING
            {
                TimePointType l_load_start = chrono::high_resolution_clock::now();
                TimePointType l_load_end;
                if(load_alveo_partitions_DriverSelf(alveoProject, numNode, numPureWorker, parlv_drv, parlv_wkr, inFile)!=0)
                    return -1;
                getDiffTime(l_load_start, l_load_end, parlv_drv.timesPar.timeDriverLoad);
            }

            {
                const char* char_tmp = "shake hands from Driver";
                Driver* drivers = new Driver[numPureWorker];
                ConnectWorkers(drivers, numPureWorker, nameWorkers);

                for (int i = 0; i < numPureWorker; i++) {
#ifdef PRINTINFO
                    printf("Driver shake hands with worker %d\n", (i + 1));
#endif
                    drivers[i].send(char_tmp, MAX_LEN_MESSAGE, 0);
                }

                isAllWorkerLoadingDone(drivers, numPureWorker);
                delete[] drivers;
            }
#ifdef PRINTINFO
            printf("\n\n\033[1;31;40mDriver's LOADING DONE \033[0m\n");
            printf("\n\n\033[1;31;40mPlease Wait for Workers' Done\033[0m\n");
            printf("\n\033[1;31;40mTO START LOUVAIN PUT ANY KEY: \033[0m");
            // getchar();

            // TODO Add synchronization here
            printf("\n\033[1;31;40mTO RUNNING LOUVAIN \033[0m\n");
#endif
            // API FOR RUNNING LOUVAIN
            GLV* glv_final;
            {
                TimePointType l_execute_start = chrono::high_resolution_clock::now();
                TimePointType l_execute_end;
                glv_final = louvain_modularity_alveo(&handle0, parlv_drv, parlv_wkr, opts_minGraphSize, opts_threshold,
                                                     opts_C_thresh, numThreads, numNode, numPureWorker, nameWorkers);
                getDiffTime(l_execute_start, l_execute_end, parlv_drv.timesPar.timeDriverExecute);
            }
            glv_final->PushFeature(0, 0, 0.0, true);
            parlv_drv.plv_src->Q = glv_final->Q;
            parlv_drv.plv_src->NC = glv_final->NC;

            PrintRptPartition(mode_zmq, parlv_drv, op0.numDevices, numNode, numPureWorker);
            PrintRptPartition_Summary( parlv_drv,opts_C_thresh);
            std::string outputFileName(opts_outputFile);
            if (!outputFileName.empty()) {
            	host_writeOut(outputFileName.c_str(), parlv_drv.plv_src->NV, parlv_drv.plv_src->C);
            } else{
#ifdef PRINTINFO_2
                printf("\033[1;37;40mINFO: Please use -o <output file> to store Cluster information\033[0m\n");
#endif
            }
#ifdef PRINTINFO
            printf("Deleting orignal graph... \n");
#endif
            //parlv_drv.CleanTmpGlv();
#ifdef PRINTINFO
            parlv_drv.plv_src->printSimple();
#endif
            //delete (parlv_drv.plv_src);

            glv_final->printSimple();
            double ret = glv_final->Q;
           // delete (glv_final);

           // handle0.free();
            return ret;
        } else if (mode_zmq == ZMQ_WORKER) {
            //-----------------------------------------------------------------
            // WORKER
            //-----------------------------------------------------------------
            ParLV parlv_wkr;
            parlv_wkr.Init(flowMode, NULL, num_par, numDevices, isPrun, par_prune);
            parlv_wkr.timesPar.timeAll = getTime();
            char LoadCommand[MAX_LEN_MESSAGE];
            {
                char inFile[1024];
                ParLV parlv_tmp;
                parlv_tmp.Init(flowMode, NULL, num_par, numDevices, isPrun, par_prune);

                if (load_alveo_partitions_WorkerSelf(
                    alveoProject, numNode, numPureWorker, parlv_tmp,  
                    inFile, nodeID, LoadCommand) != 0)
                    return -1;

                LouvainGLV_general_top_zmq_worker_new_part1(&handle0, parlv_tmp, opts_minGraphSize, opts_threshold,
                                                            opts_C_thresh, numThreads, nodeID, parlv_wkr, LoadCommand);
            }
            {
                LouvainGLV_general_top_zmq_worker_new_part2(&handle0, opts_minGraphSize, opts_threshold,
                                                            opts_C_thresh, numThreads, nodeID, parlv_wkr);
            }
            handle0.free();
        }
    }

    return 0;
}


/*

*/
float load_alveo_partitions_wrapper(int argc, char* argv[]) {
//    for (int i = 0; i < argc; ++i)
//        std::cout << "internal load partitions arg " << i << " = " << argv[i] << std::endl;
    //----------------- config.json Parser ----------------------------------
    std::string opName = "louvainModularity";
    std::string kernelName = "kernel_louvain";
    int requestLoad = 100;
    std::string xclbinPath = "/proj/autoesl/ryanw/kernel_louvain_pruning.xclbin";
    int numDevices = 2;
    const int cuNm = 1;

    //--------------- Parse Input parameters
    double opts_C_thresh;   // Threshold with coloring on
    long opts_minGraphSize; // Min |V| to enable coloring
    double opts_threshold;  // Value of threshold
    int opts_ftype;         // File type
    char opts_inFile[4096];
    bool opts_coloring;
    bool opts_output;
    std::string opts_outputFile;
    bool opts_VF;
    char opts_xclbinPath[4096];

    int flow_fast = 2;
    int flowMode = 1;
    int num_par;
    bool isPrun = true;
    int par_prune = 1;
    int numThreads = NUMTHREAD; // using fixed number of thread instead of
    //int numThreads = 1; // using fixed number of thread instead of
    int devNeed_cmd = 1;
    int mode_zmq = ZMQ_NONE;
    char path_zmq[1024]; // default will be set as "./"
    bool useCmd = false;
    int mode_alveo = ALVEOAPI_NONE;
    char nameProj[1024];
    int numPureWorker;
    std::string nameMetaFile;
    char* nameWorkers[128];
    int nodeID;
    int status;
    int server_par=1;
    float retVal = 0.0;

    host_ParserParameters(argc, argv, opts_C_thresh, opts_minGraphSize, opts_threshold, opts_ftype, opts_inFile,
                          opts_coloring, opts_output, opts_outputFile, opts_VF, opts_xclbinPath, numThreads, num_par,
                          par_prune, flow_fast, devNeed_cmd, mode_zmq, path_zmq, useCmd, mode_alveo, nameProj,
                          nameMetaFile, numPureWorker, nameWorkers, nodeID, server_par, glb_max_num_level, glb_max_num_iter);

    if (devNeed_cmd > 0)
        numDevices = devNeed_cmd;

    retVal = load_alveo_partitions((char *)xclbinPath.c_str(), flow_fast, numDevices, 
                                   num_par, (char *)nameMetaFile.c_str(), 
                                   mode_zmq, numPureWorker, nameWorkers, nodeID,
                                   (char *)opts_outputFile.c_str());

    return retVal;
}

float louvain_modularity_alveo(int argc, char* argv[]) {
#ifdef PRINTINFO
    printf("\033[1;31;40mMUST DO LOADING BEFORE RUNNING\033[0m\n");
#endif
    return load_alveo_partitions_wrapper(argc, argv);
}

/* 
Compute modularity based on user provided cluster information file
*/
int compute_modularity(char* inFile, char* inClusterInfoFile, int offset) 
{
    std::cout << "INFO: Computing modularity..." << std::endl;
    std::cout << "INFO: inFile " << inFile << std::endl;
    std::cout << "INFO: inClusterInfoFile=" << inClusterInfoFile << std::endl;
    std::cout << "INFO: offset=" << offset << std::endl;

    ifstream ifsInClusterInfoFile;
    long vertexID, clusterID;
    
    int id_glv = 0;
    GLV* glv_src = CreateByFile_general(inFile, id_glv);   
    if (glv_src == NULL) 
        return -1;

    std::cout << "INFO: number of vertices= " << glv_src->NV << std::endl;

    if (glv_src->C != NULL) 
        free(glv_src->C);
    glv_src->C = (long*)malloc(sizeof(long) * glv_src->NV);

    ifsInClusterInfoFile.open(inClusterInfoFile);
    long lineCnt = 0;
    while (ifsInClusterInfoFile >> vertexID >> clusterID) {
        glv_src->C[vertexID-offset] = clusterID-offset;
        lineCnt++;
    }
    ifsInClusterInfoFile.close();
    std::cout << "INFO: " << lineCnt << " lines read from " << inClusterInfoFile << std::endl;
    
    glv_src->PushFeature(0, 0, 0.0, true);
    glv_src->printSimple();

    return 0;
}
