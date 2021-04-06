/* 
 * File:   oldUdfs.hpp
 * Author: dliddell
 *
 * Created on March 19, 2021, 11:44 AM
 */

#ifndef OLDUDFS_HPP
#define OLDUDFS_HPP

inline double udf_bfs_fpga(int64_t sourceID,
                           ListAccum<int64_t>& offsetsList,
                           ListAccum<int64_t>& indicesList,
                           ListAccum<float>& weightsList,
                           ListAccum<int64_t>& predecent,
                           ListAccum<int64_t>& distance) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    uint32_t* predecentTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    uint32_t* distanceTmp = xf::graph::internal::aligned_alloc<uint32_t>(((numVertices + 15) / 16) * 16);
    memset(predecentTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    memset(distanceTmp, -1, sizeof(uint32_t) * (((numVertices + 15) / 16) * 16));
    xf::graph::Graph<uint32_t, uint32_t> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = 1;
        count++;
    }
    int res = bfs_fpga_wrapper(numVertices, numEdges, sourceID, g, predecentTmp, distanceTmp);

    for (int i = 0; i < numVertices; ++i) {
        if (predecentTmp[i] == (uint32_t)(-1)) {
            predecent += -1;
            distance += -1;
        } else {
            predecent += predecentTmp[i];
            distance += distanceTmp[i];
        }
    }
    g.freeBuffers();
    free(predecentTmp);
    free(distanceTmp);
    return res;
}

inline double udf_load_xgraph_fpga(ListAccum<int64_t>& offsetsList,
                                   ListAccum<int64_t>& indicesList,
                                   ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_shortest_ss_pos_wt_fpga(int64_t sourceID,
                                          int64_t numEdges,
                                          int64_t numVertices,
                                          ListAccum<int64_t>& predecent,
                                          ListAccum<float>& distance) {
    uint32_t length = ((numVertices + 1023) / 1024) * 1024;
    float** result;
    uint32_t** pred;
    result = new float*[1];
    pred = new uint32_t*[1];
    result[0] = xf::graph::internal::aligned_alloc<float>(length);
    pred[0] = xf::graph::internal::aligned_alloc<uint32_t>(length);
    memset(result[0], 0, length * sizeof(float));
    memset(pred[0], 0, length * sizeof(uint32_t));

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = shortest_ss_pos_wt_fpga_wrapper(numVertices, sourceID, 1, g, result, pred);

    for (int i = 0; i < numVertices; ++i) {
        predecent += pred[0][i];
        distance += result[0][i];
    }
    free(result[0]);
    free(pred[0]);
    delete[] result;
    delete[] pred;
    return res;
}

inline double udf_load_xgraph_pageRank_wt_fpga(ListAccum<int64_t>& offsetsList,
                                               ListAccum<int64_t>& indicesList,
                                               ListAccum<float>& weightsList) {
    int numEdges = indicesList.size();
    int numVertices = offsetsList.size() - 1;
    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int count = 0;
    while (count < numEdges) {
        if (count < offsetsList.size()) {
            int value0 = (int)(offsetsList.get(count));
            g.offsetsCSR[count] = value0;
        }
        int value = (int)(indicesList.get(count));
        float value1 = (float)(weightsList.get(count));
        g.indicesCSR[count] = value;
        g.weightsCSR[count] = value1;
        count++;
    }
    int res = load_xgraph_pageRank_wt_fpga_wrapper(numVertices, numEdges, g);

    g.freeBuffers();
    return res;
}

inline double udf_pageRank_wt_fpga(
    int64_t numVertices, int64_t numEdges, float alpha, float tolerance, int64_t maxIter, ListAccum<float>& rank) {
    float* rankValue = new float[numVertices];

    xf::graph::Graph<uint32_t, float> g("CSR", numVertices, numEdges);

    int res = pageRank_wt_fpga_wrapper(alpha, tolerance, maxIter, g, rankValue);

    for (int i = 0; i < numVertices; ++i) {
        rank += rankValue[i];
    }
    delete[] rankValue;
    return res;
}


#endif /* OLDUDFS_HPP */

