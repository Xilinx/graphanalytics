
#include "ParLV.h"
#include <iostream>
#include <string>

GLV* LoadGLVBin(char* name, int& id_glv);

std::ostream &operator<<(std::ostream &os, const edge &e) {
    os << '(' << e.head << ',' << e.tail << ',' << e.weight << ')';
    return os;
}


template <typename T>
void dumpList(const std::string &title, const T *pItems, long count) {
    std::cout << title << ":";
    for (long i = 0; i < count; ++i) {
        if (i % 10 == 0) {
            std::cout << std::endl << "    " << i << ": ";
        }
        std::cout << pItems[i] << ' ';
    }
    std::cout << std::endl;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <.par file>" << std::endl;
        return 1;
    }
    
    int glvId = 0;
    GLV *glv = LoadGLVBin(argv[1], glvId);
    if (glv == nullptr) {
        std::cout << "ERROR: Failed to read .par file " << argv[1] << " (LoadGLVBin returned null pointer)."
            << std::endl;
        return 2;
    }
    
    graphNew* g = glv->G;
    long nv = g->numVertices;
    long ne = g->numEdges;
    long ne_undir = g->edgeListPtrs[nv];
    long nc = glv->NC;
    double Q = glv->Q;
    long nvl = glv->NVl;
    long nelg = glv->NElg;
    std::cout << "NV: " << nv << std::endl
        << "NE: " << ne << std::endl
        << "Unidirectional NE: " << ne_undir << std::endl
        << "NC: " << nc << std::endl
        << "Q: " << Q << std::endl
        << "Local NV: " << nvl << std::endl
        << "Local-Ghost NE: " << nelg << std::endl
        << std::endl;
    
    dumpList("Edge List Pointers (Offsets)", g->edgeListPtrs, nv + 1);
    dumpList("Edge List (head,tail,weight)", g->edgeList, ne_undir);
    dumpList("M", glv->M, nv);
    dumpList("C", glv->C, nv);

    return 0;
}
