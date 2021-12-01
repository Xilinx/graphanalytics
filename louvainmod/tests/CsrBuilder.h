/* 
 * File:   CsrBuilder.h
 * Author: dliddell
 *
 * Created on November 5, 2021, 6:03 PM
 */

#ifndef CSRBUILDER_H
#define CSRBUILDER_H

#include <cstdint>
#include <vector>

// Enable this macro to dump debugging info
//#define CSR_BUILDER_DEBUG 1


// Class that generically builds a CSR-like output from .mtx-like input without regard to the actual
// format of the CSR output.
//
// To use the class, create specializations for the CsrObject interface functions.  Then construct an object
// of this class, add .mtx entries by calling addEdge(), and finally call emitCsr() to fill the CsrObject.
//
template <typename CsrObject, typename VertexId = std::uint64_t, typename EdgeIndex = std::uint64_t>
class CsrBuilder {
public:
    struct Edge {
        VertexId head_ = 0;
        VertexId tail_ = 0;
        double weight_ = 1.0;
        
        Edge() = default;
        Edge(VertexId head, VertexId tail, double weight = 1.0) : head_(head), tail_(tail), weight_(weight) {}
    };

private:
    VertexId numVertices_ = 0;
    EdgeIndex numEdges_ = 0;
    CsrObject &csrObject_;
    std::vector<Edge> mtxEdgeList_; // Unsorted list of edges, in one direction only, no duplicates
    EdgeIndex nextEdgeIndex_ = 0;
    
public:
    //
    // CsrObject interface.  Implement specializations of these functions
    //
    
    // Allocates the offset list (map from vertex ID to first edge index) with the given number of elements,
    // setting all elements to 0.  The size allocated will be 1 more than the number of vertices.
    void createOffsetList(VertexId size) { csrObject_.offsetList_.resize(size, 0); }
    
    // Returns a writable reference to an element of the offset list
    EdgeIndex &getOffsetListAt(EdgeIndex index) { return csrObject_.offsetList_[index]; }
    
    // Allocates the edge list to the given number of elements and initializes the elements.  The size will be
    // 2 * numElements_ because every edge between two vertices is stored twice, one in each direction.
    void createEdgeList(EdgeIndex size) { csrObject_.edgeList_.resize(size); }
    
    // Given an index and an edge, set the element of the edge list to the edge.
    void setEdgeListAt(EdgeIndex index, VertexId head, VertexId tail, double weight)
        { csrObject_.edgeList_[index] = Edge(head, tail, weight); }
    
#ifdef CSR_BUILDER_DEBUG
    // Given an index, return the edge at that index (for debugging only)
    Edge getEdgeListAt(EdgeIndex index) { return csrObject_.edgeList_[index]; }
#endif
    
    
    //
    // CSR Builder API
    //
    
    CsrBuilder(VertexId numVertices, EdgeIndex numEdges, CsrObject &csrObject)
    : numVertices_(numVertices), numEdges_(numEdges), csrObject_(csrObject), mtxEdgeList_(numEdges)
    {}

    void addEdge(VertexId start, VertexId end, double weight) {
        // Eliminate self-edges
        if (start == end)
            return;
        mtxEdgeList_[nextEdgeIndex_++] = Edge(start, end, weight);
    }

    void emitCsr() {
        // algorithm from grappolo

        // Allocate for Edge Pointer and keep track of degree for each vertex
//        long* edgeListPtr = (long*)malloc((NV + 1) * sizeof(long));
        createOffsetList(numVertices_ + 1);
//    #pragma omp parallel for
//        for (long i = 0; i <= numVertices_; i++) edgeListPtr[i] = 0; // For first touch purposes

    #pragma omp parallel for
        for (long i = 0; i < numEdges_; i++) {
//            __sync_fetch_and_add(&edgeListPtr[edgeList_[i].head + 1], 1); // Plus one to take care of the zeroth location
//            __sync_fetch_and_add(&edgeListPtr[edgeList_[i].tail + 1], 1);
            __sync_fetch_and_add(&getOffsetListAt(mtxEdgeList_[i].head_ + 1), 1); // Plus one to take care of the zeroth location
            __sync_fetch_and_add(&getOffsetListAt(mtxEdgeList_[i].tail_ + 1), 1);
        }

        //////Build the EdgeListPtr Array: Cumulative addition
        for (long i = 0; i < numVertices_; i++) {
//            edgeListPtr[i + 1] += edgeListPtr[i]; // Prefix Sum:
            getOffsetListAt(i + 1) += getOffsetListAt(i); // Prefix Sum:
        }
        // The last element of Cumulative will hold the total number of characters
        printf("Sanity Check: 2|E| = %ld, edgeListPtr[NV]= %ld\n", numEdges_ * 2, getOffsetListAt(numVertices_));

        /*---------------------------------------------------------------------*/
        /* Allocate memory for G & Build it                                    */
        /*---------------------------------------------------------------------*/
//        Edge* edgeList = (Edge*)malloc((2 * numEdges_) * sizeof(Edge)); // Every edge stored twice
        createEdgeList(2 * numEdges_);
        // Keep track of how many edges have been added for a vertex:
        std::vector<long> added(numVertices_, 0);
    // Build the edgeList from edgeListTmp:
    #pragma omp parallel for
        for (long i = 0; i < numEdges_; i++) {
            const VertexId head = mtxEdgeList_[i].head_;
            const VertexId tail = mtxEdgeList_[i].tail_;
            const double weight = mtxEdgeList_[i].weight_;

//            long Where = edgeListPtr[head] + __sync_fetch_and_add(&added[head], 1);
            EdgeIndex where = getOffsetListAt(head) + __sync_fetch_and_add(&added[head], 1);
//            edgeList[Where].head = head;
//            edgeList[Where].tail = tail;
//            edgeList[Where].weight = weight;
            setEdgeListAt(where, head, tail, weight);
            // added[head]++;
            // Now add the counter-edge:
//            Where = edgeListPtr[tail] + __sync_fetch_and_add(&added[tail], 1);
            where = getOffsetListAt(tail) + __sync_fetch_and_add(&added[tail], 1);
//            edgeList[Where].head = tail;
//            edgeList[Where].tail = head;
//            edgeList[Where].weight = weight;
            setEdgeListAt(where, tail, head, weight);
            // added[tail]++;
        }
        
#if CSR_BUILDER_DEBUG
        std::cout << "Vertices" << std::endl;
        for (long i = 0; i < numVertices_ + 1; ++i)
            std::cout << ' ' << getOffsetListAt(i);
        std::cout << std::endl;
        std::cout << "Edges" << std::endl;
        for (long i = 0; i < 2 * numEdges_; ++i) {
            const Edge e = getEdgeListAt(i);
            std::cout << ' ' << e.head_ << ':' << e.tail_ << ':' << e.weight_;
        }
        std::cout << std::endl;
#endif
    }
    
};


#endif /* CSRBUILDER_H */

