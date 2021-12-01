/* 
 * File:   findCommunities.h
 * Author: dliddell
 *
 * Created on November 5, 2021, 1:10 PM
 */

#ifndef FINDCOMMUNITIES_H
#define FINDCOMMUNITIES_H

#include "graph_binary.h"
#include "community.h"
#include "CsrBuilder.h"
#include <string>


// Enable this macro to dump the graph at the start of the algorithm
//#define FIND_COMMUNITIES_DUMP_GRAPH

// Wrapper class for Lefebvre/Guillaume "Find Communities" Louvain implementation, which is command-line based
// instead of a library suitable for a regression environment.
//
class FindCommunities {
public:
    using VertexId = unsigned int;
    using EdgeIndex = unsigned long;
    using FcCsrBuilder = CsrBuilder<Graph, VertexId, EdgeIndex>;

private:    
    Graph graph_;
    FcCsrBuilder csrBuilder;
    
public:
    FindCommunities(VertexId numVertices, EdgeIndex numEdges) : csrBuilder(numVertices, numEdges, graph_)
    {
        graph_.nb_nodes = numVertices;
    }
    
    void addEdge(VertexId start, VertexId end, double weight) { csrBuilder.addEdge(start, end, weight); }

    void finishData() {
        csrBuilder.emitCsr();
        graph_.nb_links=graph_.degrees[graph_.nb_nodes-1];
        // Compute total weight
        graph_.total_weight=0;
        for (unsigned int i=0 ; i<graph_.nb_nodes ; i++) {
            graph_.total_weight += (double)graph_.weighted_degree(i);
        }
    }
    
    double louvain(const std::string &communityFileName, bool verbose = false) {
        srand(0x12345);
        double precision = 1e-6;
        int display_level = -2;
        
        // From main_community.cpp: main()
        
//        Community c(filename, filename_w, type, -1, precision);
        Community c(graph_, -1, precision);
#ifdef FIND_COMMUNITIES_DUMP_GRAPH
        c.g.display();
#endif
//        if (filename_part!=NULL)
//          c.init_partition(filename_part);
        Graph g;
        bool improvement=true;
        double mod=c.modularity(), new_mod;
        int level=0;

        do {
          if (verbose) {
            cerr << "level " << level << ":\n";
//            display_time("  start computation");
            cerr << "  network size: " 
                 << c.g.nb_nodes << " nodes, " 
                 << c.g.nb_links << " links, "
                 << c.g.total_weight << " weight." << endl;
          }

          improvement = c.one_level();
          new_mod = c.modularity();
          if (++level==display_level)
            g.display();
          if (display_level==-1)
            c.display_partition();
          g = c.partition2graph_binary();
          c = Community(g, -1, precision);

          if (verbose)
            cerr << "  modularity increased from " << mod << " to " << new_mod << endl;

          mod=new_mod;
//          if (verbose)
//            display_time("  end computation");

//          if (filename_part!=NULL && level==1) // do at least one more computation if partition is provided
//            improvement=true;
        } while(improvement);

        return new_mod;
    }
};


template<>
void CsrBuilder<Graph, FindCommunities::VertexId, FindCommunities::EdgeIndex>::createOffsetList(
        FindCommunities::VertexId size)
{
    csrObject_.degrees.resize(size - 1, 0);  // element i is the end of edges for vertex i and start of vertex i+1
}

template<>
FindCommunities::EdgeIndex &FindCommunities::FcCsrBuilder::getOffsetListAt(FindCommunities::EdgeIndex index) {
    // Create a place to treat as start of vertex 0, which doesn't exist in degrees[]
    static FindCommunities::EdgeIndex dummyVertex0Start = 0;
    
    // CsrBuilder expects index to be start of vertex "index," so back up 1, as degrees[index] is end of vertex
    return (index == 0) ? dummyVertex0Start : csrObject_.degrees[index - 1];
}

template<>
void FindCommunities::FcCsrBuilder::createEdgeList(FindCommunities::EdgeIndex size) {
    csrObject_.links.resize(size);
    csrObject_.weights.resize(size, 0.0);
}

template<>
void FindCommunities::FcCsrBuilder::setEdgeListAt(
        FindCommunities::EdgeIndex index, FindCommunities::VertexId, FindCommunities::VertexId tail, double weight) {
    csrObject_.links[index] = tail;
    csrObject_.weights[index] = weight;
}

#ifdef CSR_BUILDER_DEBUG
template<>
FindCommunities::FcCsrBuilder::Edge FindCommunities::FcCsrBuilder::getEdgeListAt(FindCommunities::EdgeIndex index) {
    return Edge(-1, csrObject_.links[index], csrObject_.weights[index]);
}
#endif

#endif /* FINDCOMMUNITIES_H */

