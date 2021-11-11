/**
 * Copyright (C) 2020 Xilinx, Inc
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

#include "defs.h"
#include "partitionLouvain.hpp"
#include "ctrlLV.h"

void FreeG(graphNew*& G) {
    free(G->edgeListPtrs);
    free(G->edgeList);
    free(G);
}

void printG_org(graphNew* G) {
    long vertexNum = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", vertexNum, NE);
    for (int v = 0; v < vertexNum; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        printf("v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t", v, adj1, adj2, degree);
        for (int d = 0; d < degree; d++) {
            printf(" %4ld\t", vtxInd[adj1 + d].tail);
        }
        printf("\n");
    }
}

void printG(graphNew* G, long* C, long* M, long star, long end) {
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    // printf("|==C==|==V==|==M==|=OFF=|=Dgr=|\n");
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%-4d : v=%-4d m=%-4d off=%4d, dgr=%-3d\t",C[v], v, M[v], adj1, degree);
        long m = M == NULL ? v : M[v];
        long c = C == NULL ? v : C[v];
        // printf(" c=%-4d, v=%-4d,", c, v, m, adj1, degree);
        if (m < 0)
            printf(" \033[1;31;40mc=%-5d v=%-5d m=%-5d\033[0m", c, v, m);
        else
            printf(" c=%-5d v=%-5d m=%-5d", c, v, m);
        printf(" o=%-5d d=%-4d |", adj1, degree);
        for (int d = 0; d < degree; d++) {
            //\033[1;31;40mERROR\033[0m
            long t = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            if (M != NULL && M[t] < 0)
                printf("\033[1;31;40m%5d\033[0m\/%1.0f ", t, w);
            else
                printf("%5d\/%1.0f ", t, w);
        }
        printf("\n");
    }
}
void printG(graphNew* G, long* C, long* M, long star, long end, bool isCid, bool isDir, ParLV* p_par, int idx) {
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    // printf("|==C==|==V==|==M==|=OFF=|=Dgr=|\n");
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        long m = M == NULL ? v : M[v];
        long c = C == NULL ? v : C[v];
        long c_final = c + p_par->off_lved[idx];
        // printf(" c=%-4d, v=%-4d,", c, v, m, adj1, degree);
        if (m < 0) {
            if (p_par->st_PreMerged == true) c_final = p_par->p_v_new[idx][v];
            // printf(" \033[1;31;40mc=%-5d (%-5d)   v=%-5d m=%-5d c(m)=%-5d\033[0m", c, c_final, v, m,
            //        p_par->FindC_nhop(m));
        } else
            printf(" c=%-5d (%-5d)   v=%-5d m=%-5d c(m)=%-5d", c, c_final, v, m, c);
        printf(" o=%-5d d=%-4d |", adj1, degree);
        for (int d = 0; d < degree; d++) {
            long t = vtxInd[adj1 + d].tail;
            if (isDir) {
                if (isCid) {
                    if (C[v] < C[t]) continue;
                } else {
                    if (v < t) continue;
                }
            }
            double w = vtxInd[adj1 + d].weight;
            if (M != NULL && M[t] < 0)
                printf("\033[1;31;40m%5d\033[0m\/%1.0f ", isCid ? C[t] : t, w);
            else
                printf("%5d\/%1.0f ", isCid ? C[t] : t, w);
        }
        printf("\n");
    }
}
void printG(graphNew* G, long* C, long* M, long star, long end, bool isCid, bool isDir) {
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    // printf("|==C==|==V==|==M==|=OFF=|=Dgr=|\n");
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%-4d : v=%-4d m=%-4d off=%4d, dgr=%-3d\t",C[v], v, M[v], adj1, degree);
        long m = M == NULL ? v : M[v];
        long c = C == NULL ? v : C[v];

        // printf(" c=%-4d, v=%-4d,", c, v, m, adj1, degree);
        if (m < 0)
            printf(" \033[1;31;40mc=%-5d v=%-5d m=%-5d\033[0m", c, v, m);
        else
            printf(" c=%-5d v=%-5d m=%-5d", c, v, m);
        printf(" o=%-5d d=%-4d |", adj1, degree);
        for (int d = 0; d < degree; d++) {
            long t = vtxInd[adj1 + d].tail;
            if (isDir) {
                if (isCid) {
                    if (C[v] < C[t]) continue;
                } else {
                    if (v < t) continue;
                }
            }
            double w = vtxInd[adj1 + d].weight;
            if (M != NULL && M[t] < 0)
                printf("\033[1;31;40m%5d\033[0m\/%1.0f ", isCid ? C[t] : t, w);
            else
                printf("%5d\/%1.0f ", isCid ? C[t] : t, w);
        }
        printf("\n");
    }
}
void printG(char* name, graphNew* G, long* C, long* M, long star, long end) {
    FILE* fp = fopen(name, "w");
    if (fp == NULL) return;
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    fprintf(fp, "v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    // printf("|==C==|==V==|==M==|=OFF=|=Dgr=|\n");
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%-4d : v=%-4d m=%-4d off=%4d, dgr=%-3d\t",C[v], v, M[v], adj1, degree);
        long m = M == NULL ? v : M[v];
        long c = C == NULL ? v : C[v];
        // printf(" c=%-4d, v=%-4d,", c, v, m, adj1, degree);
        if (m < 0)
            printf(" \033[1;31;40mc=%-5d v=%-5d m=%-5d\033[0m", c, v, m);
        else
            printf(" c=%-5d v=%-5d m=%-5d", c, v, m);
        if (m < 0)
            fprintf(fp, "[c=%-5d v=%-5d m=%-5d]", c, v, m);
        else
            fprintf(fp, " c=%-5d v=%-5d m=%-5d", c, v, m);
        fprintf(fp, " o=%-5d d=%-4d |", adj1, degree);
        for (int d = 0; d < degree; d++) {
            //\033[1;31;40mERROR\033[0m
            long t = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            if (M != NULL && M[t] < 0)
                fprintf(fp, "[%5d]%1.0f ", t, w);
            else
                fprintf(fp, "%5d\/%1.0f ", t, w);
            if (M != NULL && M[t] < 0)
                printf("\033[1;31;40m%5d\033[0m\/%1.0f ", t, w);
            else
                printf("%5d\/%1.0f ", t, w);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}
void printG_NOWeight(graphNew* G, long* C, long* M, long star, long end) {
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    // printf("|==C==|==V==|==M==|=OFF=|=Dgr=|\n");
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%4ld : v=%4ld\t adj1=%4ld, adj2=%4ld, degree=%4ld\t",C[v], v, adj1, adj2, degree);
        // printf("c=%-4d : v=%-4d m=%-4d off=%4d, dgr=%-3d\t",C[v], v, M[v], adj1, degree);
        long m = M == NULL ? v : M[v];
        long c = C == NULL ? v : C[v];
        // printf(" c=%-4d, v=%-4d,", c, v, m, adj1, degree);
        if (m < 0)
            printf(" \033[1;31;40mc=%-5d v=%-5d m=%-5d\033[0m", c, v, m);
        else
            printf(" c=%-5d v=%-5d m=%-5d", c, v, m);
        printf(" o=%-5d d=%-4d |", adj1, degree);
        for (int d = 0; d < degree; d++) {
            //\033[1;31;40mERROR\033[0m
            long t = vtxInd[adj1 + d].tail;
            if (M != NULL && M[t] < 0)
                printf("\033[1;31;40m%5d\033[0m ", t);
            else
                printf("%5d ", t);
        }
        printf("\n");
    }
}
void printG_old2(graphNew* G, long* C, long* M, long star, long end) {
    long NV = G->numVertices;
    long NE = G->numEdges;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    printf("v=%ld\t; e=%ld \n", NV, NE);
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    for (int v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        printf("c=%-4d : v=%-4d m=%-4d off=%4d, dgr=%-3d\t", C[v], v, M[v], adj1, degree);
        for (int d = 0; d < degree; d++) {
            printf(" %4d  ", vtxInd[adj1 + d].tail);
        }
        printf("\n");
    }
}

void printG(graphNew* G, long* C, long* M) {
    long NV = G->numVertices;
    printG(G, C, M, 0, NV);
}

void printG(graphNew* G, long* C) {
    long NV = G->numVertices;
    printG(G, C, NULL, 0, NV);
}
long CountV(graphNew* G, long star, long end) {
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    map<long, long> map_all;
    map<long, long>::iterator iter_all;
    long cnt = 0;
    for (long v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        if (map_all.find(v) == map_all.end()) map_all[cnt] = cnt++;

        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            if (map_all.find(e) == map_all.end()) map_all[cnt] = cnt++;
        } // for
    }
    return cnt;
}

long CountV(graphNew* G) {
    long NV = G->numVertices;
    return CountV(G, 0, NV);
}

long CountVGh(graphNew* G, long star, long end) {
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    if (star < 0) star = 0;
    if (end > NV) end = NV;
    map<long, long> map_all;
    map<long, long>::iterator iter_all;
    long cnt = 0;
    for (long v = star; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;

        // if(map_all.find(v) == map_all.end())
        //	map_all[cnt] = cnt++;

        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            if (e < star || e >= end) {
                if (map_all.find(e) == map_all.end()) map_all[e] = cnt++;
            }
        } // for
    }
    return cnt;
}

long CountVGh(graphNew* G) {
    long NV = G->numVertices;
    return CountVGh(G, 0, NV);
}
void CreateSubG(graphNew* G_src, long start, long size, graphNew* G_sub, long* M_sub2src, long* M_sub2ghost) {
    long end = start + size;
    long NV_s = G_src->numVertices;
    long NE_s = G_src->numEdges;
    long* off_s = G_src->edgeListPtrs;
    edge* edge_s = G_src->edgeList;

    map<long, long> clusterLocalMap; // Map each neighbor's cluster to a local number
    map<long, long>::iterator storedAlready;
    long numUniqueClusters = 0;

    long num_v_par = 0;
    long num_e_par = 0;
    long v = start;
    map<long, long> map_ghost;
    map<long, long> map_all;
    map<long, long>::iterator iter_ghost;
    map<long, long>::iterator iter_all;

    while (1) {
        long adj1 = off_s[v];
        long adj2 = off_s[v + 1];
        int dgr = adj2 - adj1;
        iter_all = map_all.find(v);
    }

    for (long v = start; v < end; v++) {
        long adj1 = off_s[v];
        long adj2 = off_s[v + 1];
        int degree = adj2 - adj1;
        for (long e = 0; e < degree; e++) {
            long head = edge_s[v + e].head;
            long tail = edge_s[v + e].tail;
            bool isHeadin = (head >= start) && (head < end);
            bool isTailin = (tail >= start) && (tail < end);
            bool isEgAllIn = isHeadin & isTailin;
            bool isEgAllOut = (~isHeadin) & (~isTailin);
            bool isOnlyHeadIn = isHeadin & (~isTailin);
            bool isOnlyTailIn = isTailin & (~isHeadin);
            long head_m;
            long tail_m;

            // Remap the head and tail
            if (isEgAllOut) {
                continue;
            } else {
                storedAlready = clusterLocalMap.find(head);
                if (storedAlready != clusterLocalMap.end()) {
                    head_m = storedAlready->second; // Renumber the cluster id
                } else {
                    clusterLocalMap[head] = numUniqueClusters; ////Does not exist, add to the map
                    head_m = numUniqueClusters;                // Renumber the cluster id
                    numUniqueClusters++;                       // Increment the number
                }
                storedAlready = clusterLocalMap.find(tail);
                if (storedAlready != clusterLocalMap.end()) {
                    tail_m = storedAlready->second; // Renumber the cluster id
                } else {
                    clusterLocalMap[tail] = numUniqueClusters; ////Does not exist, add to the map
                    tail_m = numUniqueClusters;                // Renumber the cluster id
                    numUniqueClusters++;                       // Increment the number
                }
            }
            edge_s[v + e].head = head_m;
            edge_s[v + e].tail = tail_m;
        }
    }
}

void CopyG(graphNew* G_scr, graphNew* G_des) {
    long NV = G_scr->numVertices;
    long NE = G_scr->numEdges;
    G_des->numEdges = NE;
    G_des->numVertices = NV;

    long* vtxPtr = G_scr->edgeListPtrs;
    edge* vtxInd = G_scr->edgeList;
    long* vtxPtr2 = G_des->edgeListPtrs;
    edge* vtxInd2 = G_des->edgeList;

    for (int v = 0; v < NV; v++) {
        long adj1 = vtxPtr[0 + v];
        long adj2 = vtxPtr[0 + v + 1];
        vtxPtr2[v] = adj1 - 0;
        vtxPtr2[v + 1] = adj2 - 0;
        int degree = adj2 - adj1;
        long adj1_des = vtxPtr2[v];
        for (int d = 0; d < degree; d++) {
            vtxInd2[adj1_des + d].head = vtxInd[adj1 + d].head;
            vtxInd2[adj1_des + d].tail = vtxInd[adj1 + d].tail;
            vtxInd2[adj1_des + d].weight = vtxInd[adj1 + d].weight;
        }
    }
}

graphNew* CloneGbad(graphNew* G_scr) {
    long NV = G_scr->numVertices;
    long NE = G_scr->numEdges;
    graphNew* G_des = (graphNew*)malloc(sizeof(graphNew));
    G_des->edgeListPtrs = (long*)malloc(sizeof(long) * (NV + 1));
    G_des->edgeList = (edge*)malloc(sizeof(edge) * NE);
    CopyG(G_scr, G_des);
    return G_des;
}

void CreatSubG(long head, long end_line, graphNew* G_scr, graphNew* G_des) {
    long NV = G_scr->numVertices;
    long* vtxPtr = G_scr->edgeListPtrs;
    edge* vtxInd = G_scr->edgeList;
    long NE = G_scr->numEdges;
    G_des->numVertices = G_scr->numVertices;
    G_des->numEdges = G_scr->numEdges;
    G_des->edgeListPtrs = (long*)malloc(sizeof(long) * (NV + 1));
    long base_edge = vtxPtr[head];
    long size_edge = vtxPtr[head + end_line] - base_edge;
    G_des->edgeList = (edge*)malloc(sizeof(edge) * size_edge);
    long* vtxPtr2 = G_des->edgeListPtrs;
    edge* vtxInd2 = G_des->edgeList;

    for (int v = 0; v < NV; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        if (v >= head && v < (head + end_line)) {
            vtxPtr2[v] = adj1 - base_edge;     // txPtr[head]
            vtxPtr2[v + 1] = adj2 - base_edge; // txPtr[head]
            long adj1_des = vtxPtr2[v];
            for (int d = 0; d < degree; d++) {
                vtxInd2[adj1_des + d].head = vtxInd[adj1 + d].head;
                vtxInd2[adj1_des + d].tail = vtxInd[adj1 + d].tail;
                vtxInd2[adj1_des + d].weight = vtxInd[adj1 + d].weight;
            }
        } else if (v < head) {
            vtxPtr2[v] = 0;
            vtxPtr2[v + 1] = 0;
        } else {
            vtxPtr2[v] = size_edge;
            vtxPtr2[v + 1] = size_edge;
        }
    }
}

graphNew* CloneG(graphNew* G_scr) {
    long NV = G_scr->numVertices;
    graphNew* G_des = (graphNew*)malloc(sizeof(graphNew));
    CreatSubG(0, NV, G_scr, G_des);
    return G_des;
}

void InitC(long* C, long NV) {
    assert(C);
    for (int i = 0; i < NV; i++) C[i] = i;
}

/////////////////////////////////////////////////////////////////////////////////////
/// GLV//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
GLV::GLV(int& id) {
    InitVar();
    ID = id;
    sprintf(name, "%d\0", id);
    id++;
}

GLV::~GLV() {
    FreeMem();
}

void GLV::InitVar() {
    G = 0;
    C = 0;
    M = 0;
    colors = 0;
    NC = NElg = NVl = 0;
    NV = -1;
    NE = -1;
    numColors = -1;
    numThreads = 1;
    times.parNo = -1; // No partition
};
void GLV::FreeMem() {
    if (G) FreeG(G);
    if (C) free(C);
    if (M) free(M);
    if (colors) free(colors);
}
void GLV::CleanCurrentG() {
    if (G != 0) {
        printf("GLV: Current G is not empty!!!: \n");
        printf("GLV: To be clean G in GLV: ");
        printSimple();
        // displayGraphCharacteristics(G);
        printf("GLV: FreeMem\n");
        FreeMem();
        printf("GLV: InitVar\n");
        InitVar();
    }
}

void GLV::InitByFile(char* name_file) {
    double totTimeColoring;
    CleanCurrentG();
    printf("GLV: host_PrepareGraph(3, %s, 0)\n", name_file);
    G = host_PrepareGraph(3, name_file, 0);
    SyncWithG();
    InitM();
    printf("GLV: displayGraphCharacteristics\n", name_file);
    displayGraphCharacteristics(G);
    printf("GLV: NV = %ld\t NE = %ld\t numColor = %d \n", NV, NE, numColors);
}
void GLV::InitByOhterG(graphNew* G_orig) {
    assert(G_orig);
    CleanCurrentG();
    G = CloneG(G_orig);
    SyncWithG();
    InitM();
    printf("GLV: NV = %ld\t NE = %ld\t numColor = %d \n", NV, NE, numColors);
}
void GLV::SetByOhterG(graphNew* G_src) {
    assert(G_src);
    CleanCurrentG();
    G = G_src;
    SyncWithG();
    InitM();
    printf("GLV: NV = %ld\t NE = %ld\t numColor = %d \n", NV, NE, numColors);
}
void GLV::RstNVlByM() {
    NVl = 0;
    for (int i = 0; i < NV; i++)
        if (M[i] >= 0) NVl++;
}
void GLV::RstNVElg() {
    NElg = 0;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    for (long v = 0; v < NV; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        if (M[v] < 0) continue;
        for (long k = adj1; k < adj2; k++) {
            edge e = vtxInd[k];
            if (M[e.tail] < 0) NElg++;
        }
    } // for v
}
void GLV::SetByOhterG(graphNew* G_src, long* M_src) {
    assert(G_src);
    CleanCurrentG();
    G = G_src;
    SyncWithG();
    if (M) free(M);
    M = M_src;
    RstNVlByM();
    RstNVElg();
    printf("GLV: NV = %ld\t NE = %ld\t numColor = %d \n", NV, NE, numColors);
}

void GLV::SetM(long* M_src) {
    assert(M_src);
    assert(G);
    NVl = 0;
    for (int i = 0; i < NV; i++) {
        M[i] = M_src[i];
        if (M[i] >= 0) NVl++;
    }
    RstNVElg();
}
void GLV::SetM() {
    assert(G);
    for (int i = 0; i < NV; i++) M[i] = i;
    NVl = NV;
}
void GLV::InitM(long* M_src) {
    assert(M_src);
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    if (M) {
        printf("GLV: InitM: M is not empty and will be free and re-allocated.\n");
        free(M);
    }
    M = (long*)malloc(NV * sizeof(long));
    SetM(M_src);
}
void GLV::InitM() {
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    if (M) {
        printf("GLV: InitM: M is not empty and will be free and re-allocated.\n");
        free(M);
    }
    M = (long*)malloc(NV * sizeof(long));
    SetM();
}
void GLV::SetC(long* C_src) {
    assert(C_src);
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    for (int i = 0; i < NV; i++) C[i] = C_src[i];
}
void GLV::SetC() {
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    for (int i = 0; i < NV; i++) C[i] = i;
    NC = NV;
}
void GLV::InitC(long* C_src) {
    assert(C_src);
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    if (C) {
        printf("GLV: InitC: C is not empty and will be free and re-allocated.\n");
        free(C);
    }
    C = (long*)malloc(NV * sizeof(long));
    SetC(C_src);
}
void GLV::InitC() {
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    if (C) {
        printf("GLV: InitC: C is not empty and will be free and re-allocated.\n");
        free(C);
    }
    C = (long*)malloc(NV * sizeof(long));
    SetC();
}
void GLV::ResetColor() {
    double totTimeColoring;
    numColors = Phaseloop_UsingFPGA_InitColorBuff(G, colors, numThreads, totTimeColoring);
}
void GLV::ResetC() {
    this->com_list.clear();
    InitC(); /*
     FeatureLV f1(this);
     f1.PrintFeature();
     com_list.push_back(f1);*/
}
void GLV::SyncWithG() {
    double totTimeColoring;
    assert(G);
    if (NV < G->numVertices) {
        if (C) free(C);
        C = (long*)malloc(G->numVertices * sizeof(long));
        if (colors) free(colors);
        colors = (int*)malloc(G->numVertices * sizeof(int));
    }
    NV = G->numVertices;
    NE = G->numEdges;
    ResetColor();
    ResetC();
}
void GLV::InitG(graphNew* G_src) {
    assert(G_src);
    if (G) CleanCurrentG();
    G = CloneG(G_src);
    NV = G->numVertices;
    NE = G->numEdges;
}
void GLV::SetName(char* nm) {
    strcpy(name, nm);
};
void GLV::InitColor() {
    // double totTimeColoring;
    assert(G);
    NV = G->numVertices;
    NE = G->numEdges;
    if (colors) free(colors);
    colors = (int*)malloc(G->numVertices * sizeof(int));
    ResetColor();
}
void GLV::print() {
    printSimple();
    assert(G);
    assert(C);
    assert(M);
    printG(G, C, M);
}
void GLV::printSimple() {
    // list<FeatureLV>::iterator iter = com_list.back();
    // list<FeatureLV>::iterator iter = com_list[com_list.size() - 1];
    double Q = com_list.back().Q;
    long NC = com_list.back().NC;
    if (NC == NV)
        printf("| GLV ID: %-2d| NC/NV: \033[1;37;40m%8d\033[0m/", ID, NC, NVl);
    else
        printf("| GLV ID: %-2d| NC/NV: \033[1;31;40m%8d\033[0m/", ID, NC, NVl);
    if (NV < (1)) {
        if (NV == NVl)
            printf(" \033[1;37;40m%-3d\033[0m(%-3d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
        else
            printf(" \033[1;37;40m%-3d\033[0m(%-3d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
    } else if (NV < (1000000)) {
        if (NV == NVl)
            printf(" \033[1;37;40m%-6d\033[0m(%-5d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
        else
            printf(" \033[1;37;40m%-6d\033[0m(%-5d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
    } else {
        if (NV == NVl)
            printf(" \033[1;37;40m%-6d\033[0m(%-8d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
        else
            printf(" \033[1;37;40m%-6d\033[0m(%-8d/%2d\%)", NV, (NV - NVl), (int)(100 * (float)(NV - NVl) / (float)NV));
    }
    if (NE < (1000)) {
        if (NElg == 0)
            printf(" NE: %9d(%-9d/%2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
        else
            printf(" NE: %9d(%-9d/%2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
    } else if (NE < (1000000)) {
        if (NElg == 0)
            printf(" NE: %9d(%-9d/%2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
        else
            printf(" NE: %9d(%-9d/%2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
    } else {
        if (NElg == 0)
            printf(" NE: %9d(%-9d/% 2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
        else
            printf(" NE: %9d(%-9d/%2d\%)| Colors:%-6d ", NE, NElg, (int)(100 * (float)NElg / (float)NE), numColors);
    }
    if (Q > 0)
        printf(" Q: \033[1;32;40m%1.6f\033[0m  ", Q);
    else
        printf(" Q:\033[1;37;40m%2.6f\033[0m  ", Q);
    printf("| name: %s \n", name);
}
void GLV::PushFeature(int ph, int iter, double time, bool FPGA) {
    FeatureLV f1(this);
    f1.No_phase = ph;
    f1.Num_iter = iter;
    f1.time = time;
    f1.isFPGA = FPGA;
    this->com_list.push_back(f1);
    this->NC = f1.NC;
}
void GLV::printFeature() {
    list<FeatureLV>::iterator iter = com_list.begin();
    while (iter != com_list.end()) {
        (*iter).PrintFeature();
        iter++;
    }
}
void GLV::SetName_par(int ID_par, int ID_src, long start, long end, int th) {
    char nm[256];
    sprintf(nm, "ID:%d_ParID:%d_%d_%d_th%d", ID_par, ID_src, start, end, th);
    this->SetName(nm);
}
void GLV::SetName_lv(int ID_par, int ID_src) {
    char nm[256];
    sprintf(nm, "ID:%d_lv(ID:%d)", ID_par, ID_src);
    this->SetName(nm);
}
void GLV::SetName_ParLvMrg(int num_par, int ID_src) {
    char nm[256];
    sprintf(nm, "ID:%d_Mrg(lv(Par%d(ID:%d)))", ID, num_par, ID_src);
    this->SetName(nm);
}
void GLV::SetName_loadg(int ID_curr, char* path) {
    char nm[256];
    sprintf(nm, "ID:%d_%s", ID_curr, path);
    this->SetName(nm);
}
void GLV::SetName_cat(int ID_src1, int ID_src2) {
    char nm[256];
    sprintf(nm, "ID_%d_cat(ID_%d_ID%d)))", ID, ID_src1, ID_src2);
    this->SetName(nm);
}
////////////////////////////////////////////////////////////////////////////
SttGPar::SttGPar(long s, long e) {
    assert(e > s);
    start = s;
    end = e;
    num_v = num_e = num_e_dir = num_e_ll_dir = 0;
    num_v_l = num_v_g = 0;
    num_e_ll = num_e_lg = num_e_gl = num_e_gg = 0;
}
SttGPar::SttGPar() {
    num_v = num_e = num_e_dir = num_e_ll_dir = 0;
    num_v_l = num_v_g = 0;
    num_e_ll = num_e_lg = num_e_gl = num_e_gg = 0;
}
void SttGPar::PrintStt() {
    printf("**SttGPar::PrintStt BEGIN**\n");
    printf("From %ld to %ld \n", start, end);
    num_v_l = end - start;
    printf("Total V : %ld\t Total  Vl : %ld\t Total Vg: %ld\t Vl\/V=%2.2f\%\n", num_v, num_v_l, num_v_g,
           (float)num_v_l / (float)num_v * 100.0);
    assert(num_e_lg == num_e - num_e_ll);
    printf("Total 2E: %ld\t Total  ll : %ld\t Total lg: %ld\t ll\/E=%2.2f\%\n", num_e, num_e_ll, num_e_lg,
           (float)num_e_ll / (float)num_e * 100.0);
    printf("Total|E|: %ld\t Total |ll|: %ld\t Total lg: %ld\t |ll|\/|E|=%2.2f\%\n", num_e_dir, num_e_ll_dir, num_e_lg,
           (float)num_e_ll_dir / (float)num_e_dir * 100.0);
    printf("**SttGPar::PrintStt END**\n");
}

bool SttGPar::InRange(long v) {
    return v >= start && v < end;
}
void SttGPar::AddEdge(edge* edges, long head, long tail, double weight, long* M_g) {
    long head_m = head - start;
    long off = end - start;
    long tail_m;
    map<long, long>::iterator itr;
    num_e++;
    // num_e
    if (InRange(tail)) {
        num_e_ll++;
        if (head <= tail) {
            tail_m = tail - start;
            // printf("NODIR(%ld)\t: ll:<%ld %ld> -> <%ld %ld> \t= %ld - %ld \n", num_e_dir, head, tail, head_m, tail_m,
            // tail, start);
            edges[num_e_dir].head = head_m;
            edges[num_e_dir].tail = tail_m;
            edges[num_e_dir].weight = weight;
            num_e_ll_dir++;
            num_e_dir++;
        }
    } else {
        itr = map_v_g.find(tail);
        if (itr == map_v_g.end()) {
            tail_m = num_v_g + off;
            M_g[tail_m] = -tail - 1; // using negtive to indicate it ghost
            map_v_g[tail] = num_v_g++;
            // printf("NODIR(%ld)\t: lg:<%ld %ld> -> <%ld %ld>  \t= %ld + %ld \n", num_e_dir, head, tail, head_m,
            // tail_m, num_v_g, off);
        } else {
            tail_m = itr->second + off;
            // printf("NODIR(%ld)\t: lg:<%ld %ld> -> <%ld %ld>  \t= %ld + %ld \n", num_e_dir, head, tail, head_m,
            // tail_m, itr->second, off);
        }
        edges[num_e_dir].head = head_m;
        edges[num_e_dir].tail = tail_m;
        edges[num_e_dir].weight = weight;
        num_e_lg++;
        num_e_dir++;
    }
    num_v_l = end - start;
    num_v = num_v_l + num_v_g;
}

long SttGPar::findAt(VGMinDgr& gMinDgr, long tail, long dgr, long num_vg, int th_maxGhost) {
    long index = 0;
    long low = 0, high;

    if (th_maxGhost == 1) {
        if (dgr < gMinDgr.dgrs[0] || (dgr == gMinDgr.dgrs[0] && tail < gMinDgr.tail[0]))
            return 0;
        else
            return -1;
    } else if (th_maxGhost > 1) { // th_maxGhost > 1 && num_vg < th_maxGhost
        high = num_vg < th_maxGhost ? num_vg - 1 : th_maxGhost - 1;
        if (dgr < gMinDgr.dgrs[0] || (dgr == gMinDgr.dgrs[0] && tail < gMinDgr.tail[0]))
            return 0;
        else {
            while (low <= high) {
                index = (low + high) / 2;
                if (dgr == gMinDgr.dgrs[index] && tail == gMinDgr.tail[index])
                    return -1;
                else if (dgr < gMinDgr.dgrs[index] || (dgr == gMinDgr.dgrs[index] && tail < gMinDgr.tail[index]))
                    high = index - 1;
                else if (dgr > gMinDgr.dgrs[index] || (dgr == gMinDgr.dgrs[index] && tail > gMinDgr.tail[index]))
                    low = index + 1;
            }
            return low;
        }
    } else
        return -1;
}

void SttGPar::EdgePruning(edge* edges,
                          long head,
                          long tail,
                          double weight,
                          long* M_g,
                          VGMinDgr& gMinDgr,
                          long& num_vg,
                          long& e_dgr,
                          int th_maxGhost) {
    long head_m = head - start;
    long tail_m;

    // num_e
    if (InRange(tail)) {
        num_e++;
        num_e_ll++;
        if (head <= tail) {
            tail_m = tail - start;
            // printf("NODIR(%ld)\t: ll:<%ld %ld> -> <%ld %ld> \t= %ld - %ld \n", num_e_dir, head, tail, head_m, tail_m,
            // tail, start);
            edges[num_e_dir].head = head_m;
            edges[num_e_dir].tail = tail_m;
            edges[num_e_dir].weight = weight;
            num_e_ll_dir++;
            num_e_dir++;
        }
    } else {
        // printf("tail=%ld dgr= %ld \n", tail, e_dgr);
        if (num_vg == 0) {
            gMinDgr.tail[0] = tail;
            gMinDgr.dgrs[0] = e_dgr;
            gMinDgr.wght[0] = weight;
        } else {
            long at = findAt(gMinDgr, tail, e_dgr, num_vg, th_maxGhost);
            if (at >= 0 && at < th_maxGhost) {
                long where = num_vg < th_maxGhost ? num_vg : (th_maxGhost - 1);
                for (int i = where; i > at; i--) {
                    gMinDgr.tail[i] = gMinDgr.tail[i - 1];
                    gMinDgr.dgrs[i] = gMinDgr.dgrs[i - 1];
                    gMinDgr.wght[i] = gMinDgr.wght[i - 1];
                }
                gMinDgr.tail[at] = tail;
                gMinDgr.dgrs[at] = e_dgr;
                gMinDgr.wght[at] = weight;
                //              printf("insert: at= %ld \n", at);
            }
        }
        num_vg++;
    }
}

void SttGPar::CountVPruning(graphNew* G, long st, long ed, int th_maxGhost) {
    assert(G);
    assert(ed > st);

    start = st;
    end = ed;
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    assert(end <= NV);
    long ne = vtxPtr[end] - vtxPtr[start];
    edge* elist = (edge*)malloc(sizeof(edge) * (ne));
    long* M_g = (long*)malloc(sizeof(long) * (NV));

    // long off = end - start;

    long off = end - start;
    for (int i = 0; i < NV; i++) {
        M_g[i] = i < off ? i + start : -2;
    }
    for (long v = start; v < end; v++) {
        map<long, long>::iterator itr;
        VGMinDgr gMinDgr;
        long num_vg = 0;
        long e_dgr = 0;
        long head_m, tail_m;
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            e_dgr = vtxPtr[e + 1] - vtxPtr[e];
            head_m = v - start;
            EdgePruning(elist, v, e, w, M_g, gMinDgr, num_vg, e_dgr, th_maxGhost);
        } // for
        long smallest = num_vg < th_maxGhost ? num_vg : th_maxGhost;
        for (int i = 0; i < smallest; i++) {
            itr = map_v_g.find(gMinDgr.tail[i]);
            if (itr == map_v_g.end()) {
                tail_m = num_v_g + off;
                M_g[tail_m] = -gMinDgr.tail[i] - 1;
                map_v_g[gMinDgr.tail[i]] = num_v_g++;
            } else {
                tail_m = itr->second + off;
            }
            elist[num_e_dir].head = head_m;
            elist[num_e_dir].tail = tail_m;
            elist[num_e_dir].weight = gMinDgr.wght[i];
            num_e_lg++;
            num_e_dir++;
            num_e++;
            printf("vertex= %ld\t nGhost= %ld\t sGhost= %ld\t  degree= %ld\t\n", v, num_vg, gMinDgr.tail[i],
                   gMinDgr.dgrs[i]);
        }
    }

    num_v_l = end - start;
    num_v = num_v_l + num_v_g;
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GetGFromEdge(Gnew, elist, num_v, num_e_dir);
    // printG(Gnew);
    printG(Gnew, M_g);
    FreeG(Gnew);
    free(elist);
    free(M_g);
}

void SttGPar::CountV(graphNew* G, long st, long ed, edge* elist, long* M_g) {
    assert(G);
    assert(ed > st);
    start = st;
    end = ed;
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    assert(end <= NV);
    long ne = vtxPtr[end] - vtxPtr[start];
    elist = (edge*)malloc(sizeof(edge) * (ne));
    M_g = (long*)malloc(sizeof(long) * (NV));

    // long off = end - start;

    long off = end - start;
    for (int i = 0; i < NV; i++) {
        M_g[i] = i < off ? i + start : -2;
    }
    for (long v = start; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            AddEdge(elist, v, e, w, M_g);
        } // for
    }
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GetGFromEdge(Gnew, elist, num_v, num_e_dir);
    // printG(Gnew);
    printG(Gnew, M_g);
    FreeG(Gnew);
    free(elist);
    free(M_g);
}

void SttGPar::CountV(graphNew* G, long st, long ed) {
    assert(G);
    assert(ed > st);
    start = st;
    end = ed;
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    assert(end <= NV);
    long ne = vtxPtr[end] - vtxPtr[start];
    edge* elist = (edge*)malloc(sizeof(edge) * (ne));
    long* M_g = (long*)malloc(sizeof(long) * (NV));

    // long off = end - start;

    long off = end - start;
    for (int i = 0; i < NV; i++) {
        M_g[i] = i < off ? i + start : -2;
    }
    for (long v = start; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            AddEdge(elist, v, e, w, M_g);
        } // for
    }
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GetGFromEdge(Gnew, elist, num_v, num_e_dir);
    // printG(Gnew);
    printG(Gnew, M_g);
    FreeG(Gnew);
    free(elist);
    free(M_g);
}
GLV* CloneGlv(GLV* glv_src, int& id_glv) {
    assert(glv_src);
    GLV* glv = new GLV(id_glv);
    glv->InitG(glv_src->G);
    glv->InitC(glv_src->C);
    glv->InitM(glv_src->M);
    glv->InitColor();
    return glv;
}
GLV* GLV::CloneSelf(int& id_glv) {
    GLV* glv = new GLV(id_glv);
    glv->InitG(G);
    glv->InitC(C);
    glv->InitM(M);
    glv->InitColor();
    return glv;
}
GLV* SttGPar::ParNewGlv(graphNew* G_src, long st, long ed, int& id_glv) {
    assert(G_src);
    assert(ed > st);
    start = st;
    end = ed;
    long NV = G_src->numVertices;
    long* vtxPtr = G_src->edgeListPtrs;
    edge* vtxInd = G_src->edgeList;
    assert(end <= NV);

    long ne = vtxPtr[end] - vtxPtr[start];
    edge* elist = (edge*)malloc(sizeof(edge) * (ne));
    long* M_v = (long*)malloc(sizeof(long) * (NV)); // address by v

    long off = end - start;
    for (int i = 0; i < NV; i++) {
        M_v[i] = i < off ? i + start : -2;
    }

    for (long v = start; v < end; v++) {
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            AddEdge(elist, v, e, w, M_v);
        } // for
    }
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GLV* glv = new GLV(id_glv);

    GetGFromEdge(Gnew, elist, num_v, num_e_dir);
    glv->SetByOhterG(Gnew);
    glv->SetM(M_v);
    // printG(Gnew);
    // FreeG(Gnew);
    free(elist);
    free(M_v);
    glv->NVl = ed - st;
    glv->RstNVElg();
    return glv;
}
GLV* SttGPar::ParNewGlv_Prun(graphNew* G, long st, long ed, int& id_glv, int th_maxGhost) {
    start = st;
    end = ed;
    long NV = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    assert(end <= NV);
    long ne = vtxPtr[end] - vtxPtr[start];
    edge* elist = (edge*)malloc(sizeof(edge) * (ne));
    long* M_v = (long*)malloc(sizeof(long) * (NV));

    // long off = end - start;

    long off = end - start;
    for (int i = 0; i < NV; i++) {
        M_v[i] = i < off ? i + start : -2;
    }
    for (long v = start; v < end; v++) {
        map<long, long>::iterator itr;
        VGMinDgr gMinDgr;
        long num_vg = 0;
        long e_dgr = 0;
        long head_m, tail_m;
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            e_dgr = vtxPtr[e + 1] - vtxPtr[e];
            head_m = v - start;
            EdgePruning(elist, v, e, w, M_v, gMinDgr, num_vg, e_dgr, th_maxGhost);
        } // for
        long smallest = num_vg < th_maxGhost ? num_vg : th_maxGhost;
        for (int i = 0; i < smallest; i++) {
            itr = map_v_g.find(gMinDgr.tail[i]);
            if (itr == map_v_g.end()) {
                tail_m = num_v_g + off;
                M_v[tail_m] = -gMinDgr.tail[i] - 1;
                map_v_g[gMinDgr.tail[i]] = num_v_g++;
            } else {
                tail_m = itr->second + off;
            }
            elist[num_e_dir].head = head_m;
            elist[num_e_dir].tail = tail_m;
            elist[num_e_dir].weight = gMinDgr.wght[i];
            num_e_lg++;
            num_e_dir++;
            num_e++;
            // printf("vertex= %ld\t nGhost= %ld\t sGhost= %ld\t  degree= %ld\t\n", v, num_vg, gMinDgr.tail[i],
            // gMinDgr.dgrs[i]);
        }
    }
    num_v_l = end - start;
    num_v = num_v_l + num_v_g;
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GLV* glv = new GLV(id_glv);

    GetGFromEdge(Gnew, elist, num_v, num_e_dir);
    glv->SetByOhterG(Gnew);
    glv->SetM(M_v);
    // printG(Gnew);
    // FreeG(Gnew);
    free(elist);
    free(M_v);
    return glv;
}
typedef int t_sel;
#define NOTSELECT (0)
long FindStartVertexlastround(graphNew* G, t_sel V_selected[]){
    long NV = G->numVertices;
    long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
    long v_start=-1;
    int degree_max = 1;
    for(int v = 0; v<NV; v++){
        if(V_selected[v])
            continue;
        v_start = v;
        break;
    }
        return  v_start;
}

long FindStartVertex(graphNew* G, t_sel V_selected[]){
    long NV = G->numVertices;
	long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
    long v_start=-1;
    int degree_max = 1;
    for(int v = 0; v<NV; v++){
    	if(V_selected[v])
    		continue;
        long adj1 = offsets[v];
        long adj2 = offsets[v + 1];
        int degree = adj2 - adj1;
        if(degree < degree_max )
        	continue;
        v_start = v;
        degree_max = degree;
    }
    return v_start;
}
void FindStartVertex_lowBW(
		graphNew* G,
		int num_par,
		queue<long> q_par[],
		t_sel V_selected[]){
	//try to find start vertices in graph with low bandwidth
	//using average point of vertex
    long NV = G->numVertices;
    long step = NV/num_par;
    for(int p=0;p<num_par; p++)
    {
    	long v = p * step;
    	V_selected[v] = p+1;
    	q_par[p].push(v);
    }
}
void BFSPar_abstractingEdgeList(//return: real number of partition
		graphNew* G,
		long num_par,
		long limit_v,
		long limit_e,
		t_sel V_selected[], //inout, for recording whether a vertex is selected
		//output
        edge* elist_par,
		long &num_e_dir,
		long &num_e_dir_lg,
		long &num_v_l,
		long &num_v_g,
		map<long, long> &map_v_l,//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> &map_v_g//std:map for ghost vertices, will be used for renumbering and creating M
){

}
int BFSPar_creatingEdgeLists(//return: real number of partition
		graphNew* G,
		int num_par,
		long limit_v,
		long limit_e,
		t_sel V_selected[], //inout, for recording whether a vertex is selected
		//output
        edge* elist_par[],
		long num_e_dir[],
		long num_e_dir_lg[],
		long num_v_l[],
		long num_v_g[],
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[]//std:map for ghost vertices, will be used for renumbering and creating M
){
    long NV_all = G->numVertices;
    long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
	int idx_par=0;
	num_v_l[idx_par] = 0;
	num_v_g[idx_par] = 0;
	num_e_dir[idx_par] = 0;
	num_e_dir_lg[idx_par] = 0;
	std::queue<long> q;
	q.push(FindStartVertex(G, V_selected));
	map<long, long>::iterator itr;
	while( !q.empty()){
		long v = q.front();
		q.pop();
		if(V_selected[v])
			continue;
		V_selected[v] = true;
		itr = map_v_l[idx_par].find(v);
		assert( itr ==  map_v_l[idx_par].end() );
		map_v_l[idx_par][v] = num_v_l[idx_par];
		num_v_l[idx_par]++;
        long adj1 = offsets[v];
        long adj2 = offsets[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
        	long e = indices[adj1 + d].tail;
        	if(v > e)
        		continue;
        	double w = indices[adj1 + d].weight;
        	elist_par[idx_par][num_e_dir[idx_par]].head = v;
        	elist_par[idx_par][num_e_dir[idx_par]].tail = e;
        	elist_par[idx_par][num_e_dir[idx_par]].weight = w;
        	num_e_dir[idx_par]++;
        	q.push(e);
        }
        if(num_v_l[idx_par] > limit_v || num_e_dir[idx_par] > limit_e){
        	//clean queue : adding remainders in queue as ghosts
        	while( !q.empty()){
        		long e = q.front();
        		q.pop();
        		if(V_selected[e])
        			continue;
        		num_e_dir_lg[idx_par]++;
        		map_v_g[idx_par][v] = num_v_g[idx_par];
        		num_v_g[idx_par]++;
        	}
        	//select new start vertex and push it into queue
        	q.push(FindStartVertex(G, V_selected));
        	idx_par++;
        }
	}
	return idx_par+1;
}

bool isInMap(map<long, long> &map_v, long v)
{
	map<long, long>::iterator itr = map_v.find(v);
	if(itr==map_v.end())
		return false;
	else
		return true;
}
int isInParMap(map<long, long> map_v[], int num_par, long v){
	int locate=-1;
	for(int p=0; p<num_par; p++)
		if(isInMap(map_v[p], v))
			if(locate=-1)
				locate = p;
			else
				locate = -2;
	return locate;
}
long checkAllVInParV(
		graphNew* G,
		int num_par,
		map<long, long> map_v_l[],
		map<long, long> map_v_g[]
		)
{
	long NV_all = G->numVertices;
	long NV_miss =0;
	long NV_miss_g = 0;
	for(int v=0; v<NV_all; v++){
		if( -1 ==isInParMap( map_v_l,  num_par, v)){
			NV_miss++;
			printf("isVAllInPar No_%d: v=%d not found\n", NV_miss, v);
			int par = isInParMap( map_v_g,  num_par, v);
			if( -1 != par){
				NV_miss_g++;
				printf("isVAllInPar No_%d: v=%d not found in local but exsit in Par_%d, NV_miss_g=%d\n", NV_miss, v, par, NV_miss_g);
			}
		}
	}
	if(NV_miss==0)
		printf("checkAllVInParV: All Vertices already in partition!\n");
	else
		printf("checkAllVInParV: %d Vertices not found in local! %d found in ghost\n", NV_miss, NV_miss_g);
	return NV_miss;
}
bool checkParEdges(
		int p,
		edge* edgelist,
		long num_e_dir,
		long num_e_dir_lg,
		map<long, long> &map_v_l,//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> &map_v_g//std:map for ghost vertices, will be used for renumbering and creating M
		)
{
	long num_e_ll = 0;
	long num_e_lg = 0;
	long num_e_gg = 0;
	long num_e_miss = 0;
	for(int i=0; i<num_e_dir; i++)
	{
	    edge e=edgelist[i];
    	long head = e.head;
    	long tail = e.tail;
    	bool isIn_h_l = isInMap(map_v_l,  head);
    	bool isIn_t_l = isInMap(map_v_l,  tail);
    	bool isIn_h_g = isInMap(map_v_g,  head);
    	bool isIn_t_g = isInMap(map_v_g,  tail);
#ifdef DEBUGPAR
    	printf("check_par:%3d, \t edge_%d_%d, head_%d_%d tail_%d_%d\t", i, head, tail, isIn_h_l,  isIn_h_g, isIn_t_l, isIn_t_g);
#endif
    	if(isIn_h_l && isIn_t_l ){
    		num_e_ll++;
#ifdef DEBUGPAR
    		printf("LocalEdge %d", num_e_ll );
#endif
    	}
    	else if(isIn_h_g && isIn_t_g ){
    		num_e_gg++;
#ifdef DEBUGPAR
    		printf("GG EDGE %d", num_e_gg );
#endif
    	}
    	else{
    		num_e_lg++;
#ifdef DEBUGPAR
    		printf("GhostEdge %d", num_e_lg );
#endif
    	}
    	if( (head, tail | isIn_h_l | isIn_t_l | isIn_h_g | isIn_t_g)==false){
    		num_e_miss++;
#ifdef DEBUGPAR
    		printf("MISS EDGE %d", num_e_miss );
#endif
    	}
#ifdef DEBUGPAR
    	printf(" par_%d\n", p);
#endif

	}
    printf("checkParEdges: num_e_ll = %d\n", num_e_ll );
    printf("checkParEdges: num_e_lg = %d , num_e_dir_lg=%d\n", num_e_lg, num_e_dir_lg);
    printf("checkParEdges: num_e_gg = %d\n", num_e_gg );
    printf("checkParEdges: num_e_miss = %d\n", num_e_miss );
    return num_e_gg==0;
}
void showMap(map<long, long> &map_v, int p)
{
    map<long,long>::iterator iter;
    for(iter=map_v.begin(); iter!=map_v.end(); iter++)
    	printf("par_%d, map_%d_%d\n", p, iter->first, iter->second);
    printf("\n");
}
void showMaps(
		int num_par,
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[]//std:map for ghost vertices, will be used for renumbering and creating M
								)
{
	for(int p=0; p<num_par; p++){
		printf("par_%d:local_map\n");
		showMap(map_v_l[p], p);
		printf("par_%d:ghost_map\n");
		showMap(map_v_g[p], p);
	}
}
bool checkAllEdgeInParV(
		graphNew* G,
		int num_par,
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[]//std:map for ghost vertices, will be used for renumbering and creating M
		)
{
    edge* indices = G->edgeList;
    long NE_all = G->numEdges;
	bool ret=true;
	long num_e_ll = 0;
	long num_e_lg = 0;
	long num_e_gg = 0;
	long num_miss = 0;
	long num_find = 0;
#ifdef DEBUGPAR
	showMaps( num_par, map_v_l, map_v_g);
#endif
    for(int i=0; i<NE_all*2; i++)
    {
    	edge e=indices[i];
    	long head = e.head;
    	long tail = e.tail;
    	if(head>tail)
    		continue;

    	int locate_h_l = isInParMap(map_v_l, num_par, head);
    	int locate_t_l = isInParMap(map_v_l, num_par, tail);
    	int locate_h_g = isInParMap(map_v_g, num_par, head);
    	int locate_t_g = isInParMap(map_v_g, num_par, tail);

    	bool isHeadIn_l = locate_h_l !=-1;
    	bool isTailIn_l = locate_t_l !=-1;
    	bool isHeadIn_g = locate_h_g !=-1;
    	bool isTailIn_g = locate_t_g !=-1;

    	bool isHeadIn = isHeadIn_l | isHeadIn_g;
    	bool isTailIn = isTailIn_l | isTailIn_g;
#ifdef DEBUGPAR
    	printf("check_all:%3d,\t edge_%d_%d, local_h%d_t%d, ghost_h%d_t%d>\t", i, head, tail, locate_h_l, locate_t_l, locate_h_g, locate_t_g);
    	if(locate_h_l == locate_t_l)
    		printf("LocalEdge par_%d", locate_h_l);
    	else
    		printf("GhostEdge par_%d", locate_h_l>=0?locate_h_l:locate_t_l);
    	printf(" \n");
#endif

    	if(isHeadIn && isTailIn){
    		num_find++;
    		if(locate_h_l == locate_t_l)
    			num_e_ll++;
    		else
    			num_e_lg++;
    	}
    	else
    		num_miss++;
    }
    printf("CHECK: num_e_ll = %d\n", num_e_ll );
    printf("CHECK: num_e_lg = %d\n", num_e_lg );
    printf("CHECK: num_e_gg = %d\n", num_e_gg );
    printf("CHECK: num_miss = %d\n", num_miss );
    printf("CHECK: num_find = %d\n", num_find );

    if(num_find == NE_all)
    	return true;
    else
    	return false;
}
struct HopV{
	long v;
	int hop;
};

//#define TH_PRUN

long BFSPar_AddNeighbors(
		graphNew* G,
		int p,//
		t_sel V_selected[], //inout, for recording whether a vertex is selected
		long v,
		int hop,
        long* drglist_tg,
		//output
        edge* elist_par[],
		long num_e_dir[],
		long num_e_dir_lg[],
		long num_v_l[],
		long num_v_g[],
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[],//std:map for ghost vertices, will be used for renumbering and creating M
		std::queue<HopV> q_par[],
		map<long, long> map_v_l_scaned[],
		int max_hop[]
){
    long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
#ifdef DEBUGPAR
    printf("BFS, v=%ld\n", v);
#endif
	long adj1 = offsets[v];
	long adj2 = offsets[v + 1];
	int degree = adj2 - adj1;
    bool hasAGhost = false;// th_prun = 1 ,so this flag turn ture just in first ghost add to edge
    int e_mindgr = 0;//the min degree of the edge
    int e_min = 0;//the min degree of the edge's global ID
    long num_e_min = num_e_dir[p];//the min degree of the edge's sub-graph ID

	for (int d = 0; d < degree; d++) {
		map<long, long>::iterator itr;
		bool notSelected = false;
		bool isTailLocal = false;
		bool isTailGhost = false;
		bool BeenScaned  = false;
		long e = indices[adj1 + d].tail;

		if(V_selected[e]==0){
			notSelected = true;
			V_selected[e] = p+1;
		} //atom action done

		itr = map_v_l[p].find(e);
		if( itr != map_v_l[p].end() )
			isTailLocal=true;
		itr = map_v_l_scaned[p].find(e);
		if( itr != map_v_l_scaned[p].end() )
			BeenScaned=true;
		itr = map_v_g[p].find(e);
		if( itr != map_v_g[p].end() )
			isTailGhost=true;
		if( notSelected){
			map_v_l[p][e] = num_v_l[p];
			num_v_l[p]++;
			HopV he;
			he.hop=hop+1;
			he.v = e;
			q_par[p].push(he);
#ifdef DEBUGPAR
            printf("push vertex'e %ld\n", e);
#endif
			//if(he.hop>max_hop[p])
			//	max_hop[p] = he.hop;
		}
		//add to edge ghost;
		if(BeenScaned)// && v!=e)
			continue;
		else
		{
			double w = indices[adj1 + d].weight;
#ifdef TH_PRUN
            elist_par[p][num_e_dir[p]].head = v<=e?v:e;
            elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
            elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
            printf("!prun edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
            num_e_dir[p]++;
            if(!isTailLocal){
                if( !isTailGhost){//new found ghost
                    map_v_g[p][e] = num_v_g[p];
                    num_v_g[p]++;
                    num_e_dir_lg[p]++;
                }else{
                    num_e_dir_lg[p]++;
                }
            }
#else
			if(!notSelected && !isTailLocal){
				if( !isTailGhost){//new found ghost
                    if(!hasAGhost){// add new edge
                        elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                        elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                        elist_par[p][num_e_dir[p]].weight = w;
                        num_e_min = num_e_dir[p];
    #ifdef DEBUGPAR
                        printf("edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                        num_e_dir[p]++;

    					map_v_g[p][e] = num_v_g[p];
                        e_mindgr = drglist_tg[e];
                        e_min = e;
    					num_v_g[p]++;
    					num_e_dir_lg[p]++;
                        hasAGhost = true;
                    }else{// switch the min  // because the bfs, this branch access lightly
                        //printf("b emin=%ld, e=%ld, num_v_g[p]=%d \n",e_min, e,num_v_g[p]);
                        if(drglist_tg[e] < e_mindgr || (drglist_tg[e] == e_mindgr && e < e_min)){
                            map_v_g[p].erase(e_min);
                            map_v_g[p][e] = num_v_g[p] - 1;
                            e_mindgr = drglist_tg[e];
                            e_min = e;

                            elist_par[p][num_e_min].head = v<=e?v:e;
                            elist_par[p][num_e_min].tail = v<=e?e:v;
                            elist_par[p][num_e_min].weight = w;
                        }
                    }
				}else{// add new edge
					elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                    elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                    elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
                    printf("edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                    num_e_dir[p]++;
				}
			}else{// add new edge
                elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
                printf("edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                num_e_dir[p]++;
            }
#endif
		}//add to edge ghost;
	}// for eatch e
	map_v_l_scaned[p][v] = v;
    return 0;
}


long addGhostAfterPartition(
		graphNew* G,
		int p,//
		t_sel V_selected[], //inout, for recording whether a vertex is selected
		long v,
		int hop,
        long* drglist_tg,
		//output
        edge* elist_par[],
		long num_e_dir[],
		long num_e_dir_lg[],
		long num_v_l[],
		long num_v_g[],
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[],//std:map for ghost vertices, will be used for renumbering and creating M
		std::queue<HopV> q_par[],
		map<long, long> map_v_l_scaned[],
		int max_hop[]
){
    long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
#ifdef DEBUGPAR
    printf(" addghost BFS, v=%ld\n", v);
#endif
	long adj1 = offsets[v];
	long adj2 = offsets[v + 1];
	int degree = adj2 - adj1;
    bool hasAGhost = false;
    int e_mindgr = 0;//the min degree of the edge
    int e_min = 0;//the min degree of the edge's ID
    long num_e_min = num_e_dir[p];//the min degree of the edge's sub-graph ID

	for (int d = 0; d < degree; d++) {
		map<long, long>::iterator itr;
		bool notSelected = false;
		bool isTailLocal = false;
		bool isTailGhost = false;
		bool BeenScaned  = false;
		long e = indices[adj1 + d].tail;

		//assert(V_selected[e]==0);

		// if(V_selected[e]==0){
		// 	notSelected = true;
		// 	//V_selected[e] = p+1;
		// } //atom action done

		itr = map_v_l[p].find(e);
		if( itr != map_v_l[p].end() )
			isTailLocal=true;
		itr = map_v_l_scaned[p].find(e);
		if( itr != map_v_l_scaned[p].end() )
			BeenScaned=true;
		itr = map_v_g[p].find(e);
		if( itr != map_v_g[p].end() )
			isTailGhost=true;
		// if( notSelected){
		// 	map_v_l[p][e] = num_v_l[p];
		// 	num_v_l[p]++;
		// 	HopV he;
		// 	he.hop=hop+1;
		// 	he.v = e;
		// 	q_par[p].push(he);
        // }
		//add to edge ghost;
		if(BeenScaned)// && v!=e)
			continue;
		else
		{
            double w = indices[adj1 + d].weight;

#ifdef TH_PRUN
            elist_par[p][num_e_dir[p]].head = v<=e?v:e;
            elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
            elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
            printf("!prun add edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
            num_e_dir[p]++;
            if(!isTailLocal){
                if( !isTailGhost){//new found ghost
                    map_v_g[p][e] = num_v_g[p];
                    num_v_g[p]++;
                    num_e_dir_lg[p]++;
                }else{
                    num_e_dir_lg[p]++;
                }
            }
#else
            if(!isTailLocal){
                if( !isTailGhost){//new found ghost
                    if(!hasAGhost){// add new edge
                        elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                        elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                        elist_par[p][num_e_dir[p]].weight = w;
                        num_e_min = num_e_dir[p];
    #ifdef DEBUGPAR
                        printf(" add edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                        num_e_dir[p]++;

                        map_v_g[p][e] = num_v_g[p];
                        e_mindgr = drglist_tg[e];
                        e_min = e;
                        num_v_g[p]++;
                        num_e_dir_lg[p]++;
                        hasAGhost = true;
                    }else{// switch the min // because the bfs, this branch access heavily
                        //printf("b add emin=%ld, e=%ld, num_v_g[p]=%d \n",e_min, e,num_v_g[p]);
                        if(drglist_tg[e] < e_mindgr || (drglist_tg[e] == e_mindgr && e < e_min)){
                            map_v_g[p].erase(e_min);
                            map_v_g[p][e] = num_v_g[p] - 1;
                            e_mindgr = drglist_tg[e];
                            e_min = e;

                            elist_par[p][num_e_min].head = v<=e?v:e;
                            elist_par[p][num_e_min].tail = v<=e?e:v;
                            elist_par[p][num_e_min].weight = w;
                        }
                    }
                }else{// add new edge
                    elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                    elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                    elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
                    printf(" add edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                    num_e_dir[p]++;
                }
            }else{// add new edge
                elist_par[p][num_e_dir[p]].head = v<=e?v:e;
                elist_par[p][num_e_dir[p]].tail = v<=e?e:v;
                elist_par[p][num_e_dir[p]].weight = w;
    #ifdef DEBUGPAR
                printf(" add edge (%ld %ld)\n", elist_par[p][num_e_dir[p]].head, elist_par[p][num_e_dir[p]].tail);
    #endif
                num_e_dir[p]++;
            }
#endif
        }//add to edge ghost;
	}// for eatch e
	map_v_l_scaned[p][v] = v;
    return 0;
}

bool isHopDone(
		std::queue<HopV> &q_par_p,
		int num_hop_p)
{
	bool hop_done;
	if(!q_par_p.empty()){
		HopV hv = q_par_p.front();
		if(hv.hop==num_hop_p)
			hop_done=false;
		else
			hop_done=true;
	}else
		hop_done=true;
	return hop_done;
}

bool isPartitionDone(
		std::queue<HopV> &q_par_p,
		int num_hop_p)
{
	bool hop_done;
	if(!q_par_p.empty()){
		HopV hv = q_par_p.front();
		if(hv.hop==num_hop_p)
			hop_done=false;
		else
			hop_done=true;
	}else
		hop_done=true;
	return hop_done;
}


void BFSPar_creatingEdgeLists_fixed_prune(
		int mode_start,//0: find vertices with max degrees, 1: vertices at average partition -m 1
		int mode_hop, //"-h 0": just one vertex will be scanned; "-h 1"(default): full hop
		graphNew* G,
		int num_par,// which is fixed for convenience, in future num_par maybe modified
		long limit_v,
		long limit_e,
		t_sel V_selected[], //inout, for recording whether a vertex is selected
        //int th_prun,// the threshold of edge pruning number in the graph partition
		//output
        edge* elist_par[],
		long num_e_dir[],
		long num_e_dir_lg[],
		long num_v_l[],
		long num_v_g[],
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[]//std:map for ghost vertices, will be used for renumbering and creating M
){
	const int MAX_PAR = num_par;
	std::queue<HopV> q_par[MAX_PAR];
	int num_hop[MAX_PAR];
	//long v_start[MAX_PAR];
	map<long, long> map_v_l_scaned[MAX_PAR];

    long NV_all = G->numVertices;
    long* offsets = G->edgeListPtrs;
    edge* indices = G->edgeList;
	int idx_par=0;

    // find the max vertices of the partition graph by NV/max_par
    const long MAX_PAR_VERTEX = (NV_all + MAX_PAR - 1) / MAX_PAR;

    long NE_all = G->numEdges;
    limit_v = NV_all/num_par+1;
	/************************************/
	//Step-1: finding num_par start vertices from global G,
	//and initializing queues for each partition: q_par[p].push(v_start[p]);
	/************************************/
    //FindStartVertex_lowBW(G, num_par, q_par, V_selected);

    // get drglist_tg
    long* drglist_tg=(long*)malloc(sizeof(long) * (NV_all));
    for(int v = 0 ; v < NV_all; v++ ){
        drglist_tg[v] = offsets[v+1] - offsets[v];
    }
    //find the first startvertex for partition0
	for(int p = 0 ; p < 1; p++ ){
		num_v_l[p] = 0;

		long v_start;
		if(mode_start==0)
			v_start = FindStartVertex(G, V_selected);
		else
			v_start = p*(NV_all/num_par);
		V_selected[v_start] = p+1;//true;
		HopV hv_start;
		hv_start.hop=0;
		hv_start.v = v_start;
		q_par[p].push(hv_start);
		map_v_l[p][v_start] = num_v_l[p];
		num_v_l[p]++;
		printf("par=%d_push_v_start=%d, num_v_l[%d]=%d\n", p, v_start, p, num_v_l[p]);

		num_v_g[p] = 0;
		num_e_dir[p] = 0;
		num_e_dir_lg[p] = 0;
		num_hop[p] = 0;
	}

	bool notAllQueuesEmpty = true;
	map<long, long>::iterator itr;
	/************************************/
	/*
	Step-2: based on start vertices, doing hop-search:
	/************************************/
	int cnt_hop_round=0;
	long num_v_all_l =0;
    for( int p = 0; p < num_par; p++){// loop for each partition
        bool notQueueEmpty = false;

        //2-1 growing until MAX_PAR_VERTEX
        while( num_v_l[p]<MAX_PAR_VERTEX && notAllQueuesEmpty)
        { //for hop
            notAllQueuesEmpty = (num_v_all_l<NV_all);
		
			bool hop_done=true;
            //2-1-1. growing many hops
            //2-1-2. if empty, add a new start vertex then continue growing
			do{

				if(q_par[p].empty()){
					printf("empty_par=%d num_v_all_l=%d  %d\n", p, num_v_all_l, NV_all);
					hop_done=true;
					continue;

				}
				HopV hv = q_par[p].front();
				q_par[p].pop();

				if(hv.hop>num_hop[p])
					num_hop[p] = hv.hop;

				BFSPar_AddNeighbors(
					G,
					p,//
					V_selected, //inout, for recording whether a vertex is selected
					hv.v,
					hv.hop,
                    drglist_tg,
					//output
					elist_par,
					num_e_dir,
					num_e_dir_lg,
					num_v_l,
					num_v_g,
					map_v_l,//std:map for local vertices, will be used for renumbering and creating M
					map_v_g,//std:map for ghost vertices, will be used for renumbering and creating M
					q_par,
					map_v_l_scaned,
					num_hop);
                    notQueueEmpty = !q_par[p].empty();
                    //notAllQueuesEmpty|= notQueueEmpty;
                
                 if(num_v_l[p]>=MAX_PAR_VERTEX){
                     printf("!!!! break on the max vertex!!!\n");
                     break;
                 }
                if(mode_hop==1 || mode_hop==2)
                    hop_done=isHopDone(	q_par[p], num_hop[p]);
                else
                    hop_done=true;// Always true if checking one vertext
            }while(!hop_done);

           // printf("limit_v=%d, num_v_all_l=%d, num_v_l[%d] = %d\n", MAX_PAR_VERTEX, num_v_all_l, p , num_v_l[p]);
           // printf("queueEMP=%d, notall=%d\n", !notQueueEmpty , notAllQueuesEmpty );

            if(!notQueueEmpty && notAllQueuesEmpty && (num_v_l[p]<MAX_PAR_VERTEX) ){
             //   printf("2-1-2. add a new start for the empty par-queue to continue growing the partition graph\n ");
                    long v_start;
                    if(mode_start==0)
                       
                        if(p < num_par - 1)
                            v_start = FindStartVertex(G, V_selected);
                        else{
                            v_start = FindStartVertexlastround(G, V_selected);                      
                            // printf("last round\n");
                        }
                    else
                        v_start = p*(NV_all/num_par);
                    if(v_start<0){      
                        printf(" ----all vertex selected, go to add ghost\n");                      
                        break;
                    } 
                    V_selected[v_start] = p+1;//true;
                    HopV hv_start;
                    hv_start.hop=0;
                    hv_start.v = v_start;
                    q_par[p].push(hv_start);
                    map_v_l[p][v_start] = num_v_l[p];
                    num_v_l[p]++;
                    notQueueEmpty = true;
                    //printf(" ==== Empty case par=%d_push_v_start=%d, num_v_l[%d]=%d\n", p, v_start, p, num_v_l[p]);
                                  
                
            }

#ifdef DEBUGPAR
			printf("cnt_hop_round=%d, notAllQueuesEmpty = %d\n", cnt_hop_round++, notAllQueuesEmpty);
#endif
		}//while one partition

        //2-2. add all queue vertex to the partition and ghost
        while(!q_par[p].empty() ){
            HopV hv = q_par[p].front();
			q_par[p].pop();

            addGhostAfterPartition(
                        G,
                        p,//
                        V_selected, //inout, for recording whether a vertex is selected
                        hv.v,
                        hv.hop,
                        drglist_tg,
                        //output
                        elist_par,
                        num_e_dir,
                        num_e_dir_lg,
                        num_v_l,
                        num_v_g,
                        map_v_l,//std:map for local vertices, will be used for renumbering and creating M
                        map_v_g,//std:map for ghost vertices, will be used for renumbering and creating M
                        q_par,
                        map_v_l_scaned,
                        num_hop);
        }
        num_v_all_l += num_v_l[p];
		//printf("add ghost: limit_v=%d, num_v_all_l=%d, num_v_l[%d] = %d, num_hop[p]=%d\n", limit_v, num_v_all_l, p , num_v_l[p],  num_hop[p] );

        //2-3.find the next partition's start vertex
        if(p < num_par - 1)
        {
            num_v_l[p+1] = 0;
            long v_start;
            
            if(mode_start==0)
                v_start = FindStartVertex(G, V_selected);
            else
                v_start = (p+1)*(NV_all/num_par);
            //assert(v_start>0);
            if(v_start<0){ 
                // all vertex selected, but not edges
                break;
            }
            V_selected[v_start] = p+1+1;//true;
            HopV hv_start;
            hv_start.hop=0;
            hv_start.v = v_start;
            q_par[p+1].push(hv_start);
            map_v_l[p+1][v_start] = num_v_l[p+1];
            num_v_l[p+1]++;
            printf("par=%d, _push_v_start=%d, num_v_l[%d]=%d ------------\n", p+1, v_start, p+1, num_v_l[p+1]);

            num_v_g[p+1] = 0;
            num_e_dir[p+1] = 0;
            num_e_dir_lg[p+1] = 0;
            num_hop[p+1] = 0;
        }
	}//for all partition

    free(drglist_tg);

#ifdef DEBUGPAR

	bool ret_checkParEdges=false;
	for(int p=0; p< num_par; p++){
		printf("Check par(%d) edges in vertices\n",p);
		ret_checkParEdges|=checkParEdges(p,  elist_par[p], num_e_dir[p], num_e_dir_lg[p], map_v_l[p], map_v_g[p]);
	}
	bool ret_checkAllEdges = checkAllEdgeInParV( G, num_par, map_v_l, map_v_g);
	bool ret_checkAllV = 0==checkAllVInParV( G, num_par, map_v_l, map_v_g);

	long num_v_all=0;
	long num_e_all=0;
	long num_e_all_g=0;
	printf("\n**************REPORT OF GHOST RATIO**************\n");
	for(int p = 0 ; p < num_par; p++ ){
		long num_v = num_v_l[p] + num_v_g[p];
		num_v_all+=num_v_l[p];
		num_e_all+=num_e_dir[p];
		num_e_all_g+=num_e_dir_lg[p];

		float r_v_g = (float)num_v_g[p]/(float)num_v *100.0;
		float r_e_g = (float)num_e_dir_lg[p]/(float)num_e_dir[p]*100.0;

		printf("Self statistics: par_%d NV=%d\t(%d, \t%2.2f\%), \tNE=%d\t(%d, \t%2.2f\%) ",
													  p ,  num_v,   num_v_g[p],r_v_g ,   num_e_dir[p]   , num_e_dir_lg[p], r_e_g );
		printf("\thop[%d]=%d\n",  p, num_hop[p]);        
	}
	num_e_all -= num_e_all_g/2;
	printf("\n************* REPORT OF DIFFERENCES **************\n");
	printf("VERIFYING NV: num_v_all-NV_all = %d-%d=%d\n", num_v_all, NV_all,  num_v_all-NV_all);
	printf("VERIFYING NE: num_e_all-NE_all = %d-%d=%d\n", num_e_all, NE_all,  num_e_all-NE_all);
	printf("\n*****************SUMMARY OF CHECK*****************\n");
	printf("\033[1;37;40mINFO\033[0m:Doing BFS partition mode_star=%d, mode_hop=%d, num_par=%d\n", mode_start, mode_hop, num_par);
	if(ret_checkAllEdges)
		printf("PASS: ret_checkAllEdges\n");
	else
		printf("FAIL: ret_checkAllEdges\n");
	if(ret_checkAllEdges)
		printf("PASS: ret_checkAllEdges\n");
	else
		printf("FAIL: ret_checkAllEdges\n");
	if(ret_checkAllV)
		printf("PASS: ret_checkAllV\n");
	else
		printf("FAIL: ret_checkAllV\n");
	if( num_v_all-NV_all==0)
		printf("PASS: num_v_all == NV_all\n");
	else
		printf("FAIL: num_v_all != NV_all\n");
	if( num_e_all-NE_all==0)
		printf("PASS: num_e_all == NE_all\n");
	else
		printf("FAIL: num_e_all != NE_all\n");
	printf("**************************************************\n");

#endif
}

//find the v in the map and renum use the second value of map
bool MapRenumber(map<long, long> &map_v, long v, long& renum)
{
	map<long, long>::iterator storedAlready = map_v.find(v);
	if(storedAlready != map_v.end()){
		renum = storedAlready->second;
        //printf("renum %ld to %ld\n", v, renum);
        return true;
    }else{
		return false;
    }
}
//step-2: renumbering the head and tail in edge lists
void BFSPar_renumberingEdgeLists(//return: real number of partition
		int num_par,
        edge* elist_par[],
		long num_e_dir[],
		long num_e_dir_lg[],
		long num_v_l[],
		long num_v_g[],
		map<long, long> map_v_l[],//std:map for local vertices, will be used for renumbering and creating M
		map<long, long> map_v_g[],//std:map for ghost vertices, will be used for renumbering and creating M
		long* M[]

){
    //map_v_l first value is v_global(key), second value is renum (value++)
    for( int p = 0; p < num_par; p++){
        for(int i = 0; i < num_e_dir[p]; i++){
            edge e = elist_par[p][i];
            long head = e.head;
    	    long tail = e.tail;
            long renum_h_l = 0;
            long renum_t_l = 0;
            long renum_h_g = 0;
            long renum_t_g = 0;
    	    bool isIn_h_l = MapRenumber(map_v_l[p],  head, renum_h_l);
    	    bool isIn_t_l = MapRenumber(map_v_l[p],  tail, renum_t_l);
    	    // bool isIn_h_g = isInMap(map_v_g,  head);
    	    // bool isIn_t_g = isInMap(map_v_g,  tail);
            if(isIn_h_l && isIn_t_l){// is local
                elist_par[p][i].head = renum_h_l;
                elist_par[p][i].tail = renum_t_l;
                M[p][renum_h_l] = head;
                M[p][renum_t_l] = tail;
            }else if (isIn_h_l && (!isIn_t_l)){// tail is ghost            
                MapRenumber(map_v_g[p], tail, renum_t_g);
                //printf("tail %ld, %ld \n",tail, renum_t_g+num_v_l[p]);
                elist_par[p][i].head = renum_h_l;
                elist_par[p][i].tail = renum_t_g+num_v_l[p];
                M[p][ renum_h_l ] = head;
                M[p][ renum_t_g+num_v_l[p] ] = (-1)*tail-1;
            }else if (!isIn_h_l && isIn_t_l){// head is ghost and swap
                MapRenumber(map_v_g[p], head, renum_h_g);
                //printf("h  %ld, %ld \n",head, renum_h_g+num_v_l[p]);
                elist_par[p][i].head = renum_h_g+num_v_l[p];       
                elist_par[p][i].tail = renum_t_l;
                M[p][ renum_h_g+num_v_l[p] ] = (-1)*head-1;
                M[p][ renum_t_l ] = tail;
            }else{
                printf("ERROR: in %s\n",__FUNCTION__);
            }

        }// end edgelist per partition

        printf("check edgelist: \n");
        //for(int i = 0; i < num_e_dir[p]; i++){
            //printf("check_par:%3d, \t edge_%d_%d\n", p, elist_par[p][i].head, elist_par[p][i].tail);
        //}
        
        //convert to M
        // for(int i=0; i < num_v_l[p]; i++){
        //     M[p][map_v_l[p][i]] = i;
        // }

        printf("check M: \n");
        //for(int i = 0; i < num_v_l[p]+num_v_g[p]; i++){
            //printf("check_M:%3d, \t M[%d]=%lld\n", p, i, M[p][i]);
        //}

    }//end all partition

}
//setp-3: greating sub-graph can be used for Louvain
void BFSPar_GetGFromEdgeLists(
		int num_par,
        edge* elist_par[],
		long num_e_dir[],
		long num_v_l[],
		long num_v_g[],
		long* M[],
		graphNew* G_sub[])
{

}

void test_BFSPar_creatingEdgeLists_fixed_prune(//return: real number of partition
		int mode_start,
		int mode_hop,
		GLV* src,
        ParLV* parlv,
        int id_glv,
		int num_par,
        bool isPrun, 
        int th_prun
){

    parlv->Init(parlv->flowMode, src, num_par, 1);
    graphNew* G = src->G;
     

	//const num_par = 4;
	long limit_v=0;//,
	long limit_e=0;//,
	t_sel* V_selected;//, //inout, for recording whether a vertex is selected
	//output
    edge* elist_par[num_par];//,
    long* tmp_M_v[num_par];
	long num_e_dir[num_par];//
	long num_e_dir_lg[num_par];//,
	long num_v_l[num_par];//,
	long num_v_g[num_par];//,
	map<long, long> map_v_l[num_par];//,//std:map for local vertices, will be used for renumbering and creating M
	map<long, long> map_v_g[num_par];////std:map for ghost vertices, will be used for renumbering and creating M
	for(int p = 0 ; p < num_par; p++ ){
		elist_par[p] = (edge*)malloc(sizeof(edge) * (G->numEdges));
        tmp_M_v[p] = (long*)malloc(sizeof(long) * (G->numVertices));
	}
	V_selected = (t_sel*)malloc(sizeof(t_sel) * (G->numVertices));
	for(int i=0; i<G->numVertices; i++)
		V_selected[i] = 0;//false;

        BFSPar_creatingEdgeLists_fixed_prune(
			mode_start,
			mode_hop,
			G,
			num_par,
			limit_v,
			limit_e,
	        V_selected, //[], //,inout, for recording whether a vertex is selected
			//output
	        elist_par, //[],
			num_e_dir, //[],
			num_e_dir_lg, //[],
			num_v_l, //[],
			num_v_g, //[],
			map_v_l, //[],//std:map for local vertices, will be used for renumbering and creating M
			map_v_g //[]//std:map for ghost vertices, will be used for renumbering and creating M
	    );

        BFSPar_renumberingEdgeLists(num_par, elist_par, num_e_dir, num_e_dir_lg,
        num_v_l, num_v_g, map_v_l, map_v_g, tmp_M_v);

        //2-3. generate GLV
        for (int p = 0; p < num_par; p++)
        {
            //int id_glv = p+1;
            graphNew *Gnew = (graphNew *)malloc(sizeof(graphNew));
            GLV *glv = new GLV(id_glv);
            printf("par%d num_v = %d,num_e_dir = %d  \n",p, (num_v_l[p] + num_v_g[p]), num_e_dir[p]);
            GetGFromEdge(Gnew, elist_par[p], (num_v_l[p] + num_v_g[p]), num_e_dir[p]);
            glv->SetByOhterG(Gnew);
            glv->SetM(tmp_M_v[p]);
            glv->SetName_par(glv->ID, p, p, p, (isPrun ? 0 : th_prun));

            parlv->par_src[p] = glv;
        }
        parlv->st_Partitioned = true; //?
    //}
	
    //check
    // for(int i=0; i<G->numVertices; i++){
	// 	printf(" V_selected[%d] = %d \t\n", i, V_selected[i]);
    // }


    //connect V_selected with map_v_l+map_v_l to bfs_adjacent
    
    parlv->bfs_adjacent = (bfs_selected*)malloc(sizeof(bfs_selected) * (G->numVertices));
    bfs_selected* bfs_adjacent = parlv->bfs_adjacent;
    for(int p=0; p<num_par; p++){
        for(map<long, long>::iterator itr = map_v_l[p].begin(); itr != map_v_l[p].end(); ++itr){
            int v = itr->first;
            bfs_adjacent[v].par_idx = V_selected[v]-1;
            bfs_adjacent[v].renum_in_par = itr->second;
        }
    }

    //check
    // for(int v=0; v<G->numVertices; v++)
    //     printf(" v= %d, par= %d, renum= %d\n", v, bfs_adjacent[v].par_idx, bfs_adjacent[v].renum_in_par);
    //save adjacent

        char fullName[125]="_proj.bfs.adj";
        //sprintf(fullName, "_proj.bfs.adj\0", wfileName.c_str());
        string fn = fullName;
        FILE* f = fopen(fn.c_str(), "wb");
        std::cout << "WARNING: " << fn << " will be opened for binary write." << std::endl;
        if (!f) {
            std::cerr << "ERROR: " << fn << " cannot be opened for binary write." << std::endl;
        }
        int nv = G->numVertices;
        fprintf(f, "*Vertices %d\n", G->numVertices);
        fprintf(f, "v\t par\t renum\t\n");
        for(int v=0; v<G->numVertices; v++)
            fprintf(f, " %d\t %d\t %d\t\n", v, bfs_adjacent[v].par_idx, bfs_adjacent[v].renum_in_par);

        fclose(f);

	free(V_selected);
	for(int p = 0 ; p < num_par; p++ ){
		free(elist_par[p]);
        free(tmp_M_v[p]);
    }
}

GLV* SttGPar::AbstractionPartition(graphNew* G, long st, long ed, int& id_glv, int th_maxGhost) {
    start = st;
    end = ed;
    long NV_all = G->numVertices;
    long* vtxPtr = G->edgeListPtrs;
    edge* vtxInd = G->edgeList;
    assert(end <= NV_all);
    long ne = vtxPtr[end] - vtxPtr[start];
    edge* tmp_elist = (edge*)malloc(sizeof(edge) * (ne));
    long* tmp_M_v = (long*)malloc(sizeof(long) * (NV_all));

    // long off = end - start;

    long NV_sub = end - start;
    for (int i = 0; i < NV_all; i++) {
    	tmp_M_v[i] = i < NV_sub ? i + start : -2;//pre-setting M for non-ghost
    }
    for (long v = start; v < end; v++) {//limit of NV
        map<long, long>::iterator itr;
        VGMinDgr gMinDgr;
        long num_vg = 0;
        long e_dgr = 0;
        long head_m, tail_m;
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            e_dgr = vtxPtr[e + 1] - vtxPtr[e];
            head_m = v - start;
            EdgePruning(tmp_elist, v, e, w, tmp_M_v, gMinDgr, num_vg, e_dgr, th_maxGhost);
        } // for
        long smallest = num_vg < th_maxGhost ? num_vg : th_maxGhost;
        for (int i = 0; i < smallest; i++) {
            itr = map_v_g.find(gMinDgr.tail[i]);
            if (itr == map_v_g.end()) {
                tail_m = num_v_g + NV_sub;
                tmp_M_v[tail_m] = -gMinDgr.tail[i] - 1;
                map_v_g[gMinDgr.tail[i]] = num_v_g++;
            } else {
                tail_m = itr->second + NV_sub;
            }
            tmp_elist[num_e_dir].head = head_m;
            tmp_elist[num_e_dir].tail = tail_m;
            tmp_elist[num_e_dir].weight = gMinDgr.wght[i];
            num_e_lg++;
            num_e_dir++;
            num_e++;
            // printf("vertex= %ld\t nGhost= %ld\t sGhost= %ld\t  degree= %ld\t\n", v, num_vg, gMinDgr.tail[i],
            // gMinDgr.dgrs[i]);
        }
    }
    printf("partition edgelist :\n");
    for(int i = 0; i < num_e_dir; i++){
        printf("%ld   %ld   %lf\n", tmp_elist[i].head, tmp_elist[i].tail, tmp_elist[i].weight);
    }
    for (int i = 0; i < NV_all; i++) {
        printf("M[%d]=%ld \n", i, tmp_M_v[i]);
    }

    num_v_l = end - start;
    num_v = num_v_l + num_v_g;
    graphNew* Gnew = (graphNew*)malloc(sizeof(graphNew));
    GLV* glv = new GLV(id_glv);

    GetGFromEdge(Gnew, tmp_elist, num_v, num_e_dir);
    glv->SetByOhterG(Gnew);
    glv->SetM(tmp_M_v);
    free(tmp_elist);
    free(tmp_M_v);
    return glv;
}
void SttGPar::CountV(graphNew* G, edge* elist, long* M_g) {
    long NV = G->numVertices;
    return CountV(G, 0, NV, elist, M_g);
}
void SttGPar::CountV(graphNew* G) {
    long NV = G->numVertices;
    return CountV(G, 0, NV);
}
void GetGFromEdge(graphNew* G, edge* edgeListTmp, long num_v, long num_e_dir) {
    // Parse the first line:

    long NV = num_v, ED = num_e_dir * 2, NE = num_e_dir;

    // printf("Done reading from edges.\n");
    printf("|V|= %ld, |E|= %ld \n", NV, NE);

    // Remove duplicate entries:

    // Allocate for Edge Pointer and keep track of degree for each vertex
    long* edgeListPtr = (long*)malloc((NV + 1) * sizeof(long));
#pragma omp parallel for
    for (long i = 0; i <= NV; i++) edgeListPtr[i] = 0; // For first touch purposes

#pragma omp parallel for
    for (long i = 0; i < NE; i++) {
        __sync_fetch_and_add(&edgeListPtr[edgeListTmp[i].head + 1], 1); // Plus one to take care of the zeroth location
        __sync_fetch_and_add(&edgeListPtr[edgeListTmp[i].tail + 1], 1);
    }
    double time1, time2;
    //////Build the EdgeListPtr Array: Cumulative addition
    time1 = omp_get_wtime();
    for (long i = 0; i < NV; i++) {
        edgeListPtr[i + 1] += edgeListPtr[i]; // Prefix Sum:
    }
    // The last element of Cumulative will hold the total number of characters
    time2 = omp_get_wtime();
    // printf("Done cumulative addition for edgeListPtrs:  %9.6lf sec.\n", time2 - time1);
    printf("Sanity Check: 2|E| = %ld, edgeListPtr[NV]= %ld\n", NE * 2, edgeListPtr[NV]);

    //  printf("About to allocate memory for graph data structures\n");
    time1 = omp_get_wtime();
    edge* edgeList = (edge*)malloc((2 * NE) * sizeof(edge)); // Every edge stored twice
    assert(edgeList != 0);
    // Keep track of how many edges have been added for a vertex:
    long* added = (long*)malloc(NV * sizeof(long));
    assert(added != 0);
#pragma omp parallel for
    for (long i = 0; i < NV; i++) added[i] = 0;
    time2 = omp_get_wtime();
    // printf("Time for allocating memory for edgeList = %lf\n", time2 - time1);

    time1 = omp_get_wtime();

    // printf("About to build edgeList...\n");
    // Build the edgeList from edgeListTmp:
    //#pragma omp parallel for
    for (long i = 0; i < NE; i++) {
        long head = edgeListTmp[i].head;
        long tail = edgeListTmp[i].tail;
        double weight = edgeListTmp[i].weight;

        long Where = edgeListPtr[head] + __sync_fetch_and_add(&added[head], 1);
        edgeList[Where].head = head;
        edgeList[Where].tail = tail;
        edgeList[Where].weight = weight;
        //printf("where %ld, %ld   %ld   %lf\n", Where,edgeList[Where].head, edgeList[Where].tail, edgeList[Where].weight); 

        // added[head]++;
        // Now add the counter-edge:
        Where = edgeListPtr[tail] + __sync_fetch_and_add(&added[tail], 1);
        edgeList[Where].head = tail;
        edgeList[Where].tail = head;
        edgeList[Where].weight = weight;
        // added[tail]++;
    }
    time2 = omp_get_wtime();
    //// printf("Time for building edgeList = %lf\n", time2 - time1);

    G->sVertices = NV;
    G->numVertices = NV;
    G->numEdges = NE;
    G->edgeListPtrs = edgeListPtr;
    G->edgeList = edgeList;

    // free(edgeListTmp);
    free(added);
}

long GetGFromEdge_selfloop(graphNew* G, edge* edgeListTmp, long num_v, long num_e_dir) {
    long NV = num_v, ED = num_e_dir * 2, NE = num_e_dir;
    // Remove duplicate entries:
    /* long NewEdges = removeEdges(NV, NE, edgeListTmp);
     if (NewEdges < NE) {
       printf("GetGFromEdge_selfloop: Number of duplicate entries detected: %ld\n", NE-NewEdges);
       NE = NewEdges; //Only look at clean edges
     }*/
    printf("|V|= %ld, |E|= %ld \n", NV, NE);

    long* edgeListPtr = (long*)malloc((NV + 1) * sizeof(long));
#pragma omp parallel for
    for (long i = 0; i <= NV; i++) edgeListPtr[i] = 0; // For first touch purposes

#pragma omp parallel for
    for (long i = 0; i < NE; i++) {
        __sync_fetch_and_add(&edgeListPtr[edgeListTmp[i].head + 1], 1); // Plus one to take care of the zeroth location
        if (edgeListTmp[i].head != edgeListTmp[i].tail) __sync_fetch_and_add(&edgeListPtr[edgeListTmp[i].tail + 1], 1);
    }
    double time1, time2;
    //////Build the EdgeListPtr Array: Cumulative addition
    time1 = omp_get_wtime();
    for (long i = 0; i < NV; i++) {
        edgeListPtr[i + 1] += edgeListPtr[i]; // Prefix Sum:
    }
    // The last element of Cumulative will hold the total number of characters
    time2 = omp_get_wtime();
    // printf("Done cumulative addition for edgeListPtrs:  %9.6lf sec.\n", time2 - time1);
    printf("Sanity Check: 2|E| = %ld, edgeListPtr[NV]= %ld  GetGFromEdge_selfloop\n", NE * 2, edgeListPtr[NV]);

    //  printf("About to allocate memory for graph data structures\n");
    time1 = omp_get_wtime();
    edge* edgeList = (edge*)malloc((2 * NE) * sizeof(edge)); // Every edge stored twice
    assert(edgeList != 0);
    // Keep track of how many edges have been added for a vertex:
    long* added = (long*)malloc(NV * sizeof(long));
    assert(added != 0);
#pragma omp parallel for
    for (long i = 0; i < NV; i++) added[i] = 0;
    time2 = omp_get_wtime();
    // printf("Time for allocating memory for edgeList = %lf\n", time2 - time1);

    time1 = omp_get_wtime();

    // printf("About to build edgeList...\n");
    // Build the edgeList from edgeListTmp:
    //#pragma omp parallel for
    for (long i = 0; i < NE; i++) {
        long head = edgeListTmp[i].head;
        long tail = edgeListTmp[i].tail;
        double weight = edgeListTmp[i].weight;

        long Where = edgeListPtr[head] + __sync_fetch_and_add(&added[head], 1);
        edgeList[Where].head = head;
        edgeList[Where].tail = tail;
        edgeList[Where].weight = weight;
        // added[head]++;
        // Now add the counter-edge:
        // printf("GetGFromEdge_selfloop e=%-6d head=%-6d tail=%-6d w=%-4.0f where(%d)=%d + added[head](%d) \n",i, head,
        // tail, weight, Where, edgeListPtr[head], added[head]);
        if (head != tail) {
            Where = edgeListPtr[tail] + __sync_fetch_and_add(&added[tail], 1);
            edgeList[Where].head = tail;
            edgeList[Where].tail = head;
            edgeList[Where].weight = weight;
        }
        // added[tail]++;
    }
    time2 = omp_get_wtime();
    //// printf("Time for building edgeList = %lf\n", time2 - time1);

    G->sVertices = NV;
    G->numVertices = NV;
    G->numEdges = NE;
    G->edgeListPtrs = edgeListPtr;
    G->edgeList = edgeList;

    // free(edgeListTmp);
    free(added);
    return NE;
}
//////////////////////////////////////////////////////////////////
double FeatureLV::ComputeQ(GLV* glv) {
    assert(glv->G);
    NV = glv->G->numVertices;
    NE = glv->G->numEdges;
    long* vtxPtr = glv->G->edgeListPtrs;
    edge* vtxInd = glv->G->edgeList;
    long* tot_m = (long*)malloc(sizeof(long) * NV);
    for (int v = 0; v < NV; v++) tot_m[v] = 0;
    m = 0;
    for (int v = 0; v < NV; v++) {
        long cid = glv->C[v];
        long adj1 = vtxPtr[v];
        long adj2 = vtxPtr[v + 1];
        int degree = adj2 - adj1;
        for (int d = 0; d < degree; d++) {
            long e = vtxInd[adj1 + d].tail;
            double w = vtxInd[adj1 + d].weight;
            long cide = glv->C[e];
            m += w;
            tot_m[cide] += w;
            if (cide == cid) totalIn += w;
        }
    }
    NC = 0;
    for (int v = 0; v < NV; v++) {
        totalTot += tot_m[v] * tot_m[v];
        if (tot_m[v]) NC++;
    }
    Q = totalIn / m - totalTot / (m * m);
    free(tot_m);
    return Q;
}
double FeatureLV::ComputeQ2(GLV* glv) {
    assert(glv->G);
    NV = glv->G->numVertices;
    NE = glv->G->numEdges;
    long* vtxPtr = glv->G->edgeListPtrs;
    edge* vtxInd = glv->G->edgeList;
    long* tot_m = (long*)malloc(sizeof(long) * NV);
    for (int v = 0; v < NV; v++) tot_m[v] = 0;
    m = 0;
    for (int e = 0; e < NE; e++) {
        long head = vtxInd[e].head;
        long tail = vtxInd[e].tail;
        double w = vtxInd[e].weight;
        long cid = glv->C[head];
        long cide = glv->C[tail];
        m += w;
        tot_m[cide] += w;
        if (cide == cid) totalIn += w;
    }
    NC = 0;
    for (int v = 0; v < NV; v++) {
        totalTot += tot_m[v] * tot_m[v];
        if (tot_m[v]) NC++;
    }
    Q = totalIn / m - totalTot / (m * m);
    free(tot_m);
    return Q;
}
void FeatureLV::PrintFeature() {
    printf("NC=%-8d  NV=%-8d  NE=%-8d  ", NC, NV, NE);
    printf("Q=%-2.6f   m=%-8.1f    totalTot=%-14.1f  totalIn=%-8.1f ", Q, m, totalTot, totalIn);
    printf("No_phase=%-2d   Num_iter=%-2d    time=%-8.1f  %s \n", No_phase, Num_iter, time,
           isFPGA == true ? "FPGA" : "CPU");
}
FeatureLV::FeatureLV(GLV* glv) {
    init();
    ComputeQ(glv);
}
void FeatureLV::init() {
    NV = NE = 0;
    totalTot = totalIn = m = 0;
    Q = -1;
    NC = 0;
    No_phase = Num_iter = 0;
    time = 0;
    isFPGA = true;
}
FeatureLV::FeatureLV() {
    init();
} // FeatureLV()
/*
//louvainPhase.cpp///////////////////////////////////////////////////
int Phaseloop_UsingFPGA_InitColorBuff(
                graphNew *G,
                int *colors,
                int numThreads,
                double &totTimeColoring){
#pragma omp parallel for
    for (long i = 0; i < G->numVertices; i++) {
      colors[i] = -1;
    }
    double tmpTime;
    int numColors = algoDistanceOneVertexColoringOpt(G, colors, numThreads, &tmpTime) + 1;
    totTimeColoring += tmpTime;
    return numColors;
}*/
