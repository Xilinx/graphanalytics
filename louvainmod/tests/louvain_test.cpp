/* 
 * File:   louvain_test.cpp
 * Author: dliddell
 *
 * Created on November 5, 2021, 4:56 PM
 */


#include "islands.h"
#include "findCommunities.h"
#include <iostream>

/*
 * 
 */
int main(int argc, char** argv) {
    Islands::Options options;
    options.communitySize_ = 10;
    options.numLevels_ = 6;
    options.internalConnectionProbability_ = 0.5;
    options.numEdgesPerConnection_ = 2;
    Islands islands(options);
    Islands::VertexId numVertices = 0;
    Islands::EdgeIndex numEdges = 0;
    islands.getGraphSize(numVertices, numEdges);

    // Add 1 extra vertex, as we set Islands to start at vertex 1 (vertex 0 is empty)
    FindCommunities fc(static_cast<FindCommunities::VertexId>(numVertices + 1),
            static_cast<FindCommunities::EdgeIndex>(numEdges));
    islands.generate([&fc](const Islands::Edge &edge) -> bool {
        fc.addEdge(FindCommunities::VertexId(edge.src_), FindCommunities::VertexId(edge.dest_),
            edge.weight_);
        return true;
    });
    fc.finishData();
    double result = fc.louvain("", true);
    std::cout << "Modularity: " << result << std::endl;
    return 0;
}

