/* 
 * File:   islands.h
 * Author: dliddell
 *
 * Created on November 3, 2021, 12:27 PM
 */

#ifndef ISLANDS_H
#define ISLANDS_H

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <functional>
#include <exception>
#include <cmath>
#include <iostream>

class Islands {
public:
    using VertexId = std::int64_t;  // integer representing a vertex
    using EdgeIndex = std::uint64_t;
    
    struct Options {
        unsigned communitySize_ = 10;
        unsigned numLevels_ = 3;
        double internalConnectionProbability_ = 0.5;
        unsigned numEdgesPerConnection_ = 2;
        VertexId firstVertexId_ = 1;
    };
    
    struct Edge {
        VertexId src_ = 0;
        VertexId dest_ = 0;
        double weight_ = 1.0;
        
        Edge() = default;
        Edge(VertexId src, VertexId dest, double weight) : src_(src), dest_(dest), weight_(weight) {}
    };
    
    // Function for handling an edge produced by the graph generator.  The function should return true to continue
    // generation, or false to abort.
    using HandleEdgeFunc = std::function<bool (const Edge &edge)>;
    
    Islands(const Options &options) : options_(options) {}
    
    // Returns the number of vertices and edges of the graph to be generated, based on Options
    void getGraphSize(VertexId &numVertices, EdgeIndex &numEdges) const;

    // Generate the graph, calling the supplied function for edge edge.  Returns true if graph creation runs to
    // completion, or false if handleEdgeFunc ever returned false.
    bool generate(const HandleEdgeFunc &handleEdgeFunc);
    
private:
    class AbortException : public std::exception {
    public:
        AbortException() = default;
        virtual const char* what() const noexcept override { return "Edge writing aborted"; }
    };
    

    // Given a number of vertices, returns the total possible number of undirected edges between any two vertices.
    // Note that the data sizes here are intended for a single level of community, not the graph as a whole.
    static unsigned numPossibleEdges(unsigned numVertices) { return numVertices * (numVertices - 1) / 2; }
    
    // Given a bool vector and a predicate function, returns a random element of the vector for which
    // the predicate is true.  Returns -1 if the predicate is false for all elements.
    //
    // To make the algo more efficient, the caller is expected to keep track of how many elements are left
    // to choose from.
    //
    static int randIf(const std::vector<bool> &src_list, unsigned num_candidates, const std::function<bool(bool)> &func);

    // Class that keeps track of what edges have been created for a community of a given size.
    //
    class EdgeTracker {
    private:
        unsigned size_ = 0;
        std::vector<bool> map_;  // 2-D adjacency matrix, where a cell is false if available to use, true if used
        unsigned numCandidates_ = 0;
    public:
        EdgeTracker(unsigned size) :size_(size), numCandidates_(numPossibleEdges(size))
        {
            map_.resize((size - 1) * size, true);  // start with all slots used
            
            // Mark upper triangle of adjacency matrix as unused
            for (unsigned i = 0; i < size - 1; ++i)
                for (unsigned j = i + 1; j < size; ++j)
                    map_[i * size + j] = false;
        }
        
        void add(unsigned i, unsigned j) {
            if (i > j) {
                unsigned t = i;
                i = j;
                j = t;
            }
            map_[i * size_ + j] = true;
            --numCandidates_;
        }

        void getRandomUnused(unsigned &i, unsigned &j) {
            // pick from among the false elements
            unsigned index = randIf(map_, numCandidates_, [](bool elt) -> bool { return !elt; });
            i = index / size_;
            j = index % size_;
        }
    };
    
    Options options_;
    const HandleEdgeFunc *pHandleEdgeFunc_ = nullptr;  // always set during graph generation
    
    static VertexId generateRandomVertex(VertexId minValue, VertexId maxValue) {
        return (minValue == maxValue) ? minValue : VertexId(std::rand()) % (maxValue - minValue) + minValue;
    }
    
    // Makes a compound edge between two communities.
    //
    // Makes external_count simple edges between communities src and dest, picking random simple vertices
    // for each simple edge.  If level_pow is 1, the communities are simple vertices, so make a single simple edge between
    // the communities instead.
    //
    void makeEdge(VertexId levelPow, VertexId offset, VertexId src, VertexId dest)
    {
        const unsigned numEdges = (levelPow <= 1) ? 1 : options_.numEdgesPerConnection_;

        for (unsigned i = 0; i < numEdges; ++i) {
            VertexId srcIndex = generateRandomVertex(offset + src * levelPow, offset + (src + 1) * levelPow - 1);
            VertexId destIndex = generateRandomVertex(offset + dest * levelPow, offset + (dest + 1) * levelPow - 1);
            if (!(*pHandleEdgeFunc_)(Edge(srcIndex, destIndex, 1.0)))
                throw AbortException();
        }
    }

    unsigned numConnectionsPerCommunity() const {
        unsigned numCons = unsigned(numPossibleEdges(options_.communitySize_) * options_.internalConnectionProbability_);
        if (numCons < options_.communitySize_ - 1)
            numCons = options_.communitySize_ - 1;
        return numCons;
    }
    
    // level_pow: size to the power of the level.  For level 0, level_pow is 1, for level 1, level_pow is size, etc.
    // size: the number of vertices in a community (compound vertex of the next level up)
    // offset: starting simple vertex number of the square to generate
    // internal_probability: the probability that any two vertices of a community are connected (0 to 1, with a lower bound
    //                       of size/C(size, 2), as each community needs to have all its vertices connected at least once
    // external_count: the number of edges to create between two communities when it has been determined that two communities
    //                 should be connected
    void emitSquare(VertexId levelPow, VertexId offset) {
        const unsigned size = options_.communitySize_;
        const VertexId nextLevelPow = levelPow / size;  // level_pow of members of current community

        // If we're not at the smallest community level, make communities for each compound vertex of this community
        if (nextLevelPow > 1)
            for (unsigned i = 0; i < size; ++i)
                emitSquare(nextLevelPow, offset + i * nextLevelPow);

        // Create a minimal spanning tree for the community
        std::vector<bool> usedVertexList(size, false);
        usedVertexList[0] = true;
        EdgeTracker edgeTracker(size);  // keep track of what edges we've already made
        for (unsigned i = 1; i < size; ++i) {
            // Pick a vertex not in the list as the dest, pick a vertex from the used list as a src
            unsigned src = randIf(usedVertexList, i, [](bool isUsed) -> bool { return isUsed; });
            unsigned dest = randIf(usedVertexList, size - i, [](bool isUsed) -> bool { return !isUsed; });
            
            // Make an edge from src to dest and remember that the dest is now used
            makeEdge(nextLevelPow, offset, src, dest);
            usedVertexList[dest] = true;
            edgeTracker.add(src, dest);
        }

        // If our density hasn't been reached yet, determine how many more edges to create (excluding edges
        // already made)
        unsigned numEdgesToCreate = numConnectionsPerCommunity() - (size - 1);
        if (numEdgesToCreate < 1)
            return;

        // Create additional internal edges until we've reached our target probability
        for (unsigned i = 0; i < numEdgesToCreate; ++i) {
            unsigned src = 0;
            unsigned dest = 0;
            edgeTracker.getRandomUnused(src, dest);
            makeEdge(nextLevelPow, offset, src, dest);
            edgeTracker.add(src, dest);
        }
    }
};

//#####################################################################################################################

inline void Islands::getGraphSize(Islands::VertexId &numVertices, EdgeIndex &numEdges) const {
    numVertices = VertexId(std::pow(options_.communitySize_, options_.numLevels_));
    const unsigned numConnectionsPerCom = numConnectionsPerCommunity();
//    std::cout << "numCommConnections: " << numConnectionsPerCom << std::endl;
    
    EdgeIndex totalNumCommConnections = 0;
    EdgeIndex numConnectionsAtLevel = 1;
    for (unsigned i = 0; i < options_.numLevels_ - 1; ++i) {
        totalNumCommConnections += numConnectionsAtLevel;
        numConnectionsAtLevel *= options_.communitySize_;
    }
//    std::cout << "totalNumCommConnections: " << totalNumCommConnections << std::endl;
    
    // Each 0'th-level community has numCommConnections edges, all other levels have that * numEdgesPerConnection
    numEdges = EdgeIndex(numConnectionsPerCom) * (numVertices/options_.communitySize_
            + options_.numEdgesPerConnection_ * totalNumCommConnections);
//    std::cout << "numEdges: " << numEdges << std::endl;
}


inline bool Islands::generate(const Islands::HandleEdgeFunc &handleEdgeFunc) {
    pHandleEdgeFunc_ = &handleEdgeFunc;
    try {
        std::srand(0x12345);
        VertexId levelPow = VertexId(std::pow(options_.communitySize_, options_.numLevels_));
        emitSquare(levelPow, options_.firstVertexId_);
        return true;
    }
    catch (const AbortException &) {}
    return false;
}


inline int Islands::randIf(const std::vector<bool> &vec, unsigned numCandidates, const std::function<bool(bool)> &func)
{
    // Randomly pick a candidate.
    int choice = std::rand() % int(numCandidates);

    // Find the index of the candidate in the list
    int index = 0;
    for (int i = 0, end = int(vec.size()); i < end; ++i)
        if (func(vec[i])) {
            if (index == choice)
                return i;
            ++index;
        }

    // We should never get here, but if we do, return -1 to indicate that there is a discrepancy between numCandidates
    // and the actual number of candidate elements in the vector
    return -1;
}

#endif /* ISLANDS_H */

