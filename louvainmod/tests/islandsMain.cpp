/* 
 * File:   islandsMain.cpp
 * Author: dliddell
 *
 * Created on November 4, 2021, 1:13 PM
 */

#include "islands.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <fstream>

class ArgParser {
    int argc_ = 0;
    const char **argv_ = nullptr;
    int nextArg_ = 1;
    std::vector<int> positionalArgs_;
public:
    ArgParser(int argc, const char *argv[]) : argc_(argc), argv_(argv) {}
    
    // Returns the next command-line switch string (starting with -), or nullptr if no more switches
    const char *getNextSwitch() {
        while (nextArg_ < argc_) {
            int curArg = nextArg_++;
            const char *const argStr = argv_[curArg];
            if (argStr[0] == '-')
                return argStr;
            positionalArgs_.push_back(curArg);
        }
        return nullptr;
    }
    
    const char *getSwitchArgStr(const char *switchStr) {
        if (nextArg_ == argc_) {
            std::cout << "ERROR: Command-line switch " << switchStr << " requires an argument." << std::endl;
            std::exit(1);
        }
        int curArg = nextArg_++;
        const char *const argStr = argv_[curArg];
        return argStr;
    }
    
    const int getSwitchArgInt(const char *switchStr) {
        const char *argStr = getSwitchArgStr(switchStr);
        return std::atoi(argStr);
    }
    
    const double getSwitchArgDouble(const char *switchStr) {
        const char *argStr = getSwitchArgStr(switchStr);
        return std::atof(argStr);
    }
    
    // Call this after processing all switches to get the indexes of all positional args
    const std::vector<int> &getPositionalArgs() const { return positionalArgs_; }
};


void printUsage(const char *progNameStr) {
    std::cout
        << "Produces an .mtx file for a graph of hierarchical communities." << std::endl
        << "Usage: " << progNameStr << " [options] outputFileName" << std::endl
        << "  -s <size>         : Number of vertices per community (default=10)" << std::endl
        << "  -l <levels>       : Number of levels of communities, makes 10^levels vertices (default=3)" << std::endl
        << "  -i <internalProb> : Probability of two vertices in a community being connected (default=0.5)" << std::endl
        << "  -e <edgesPerConn> : How many edges to create between two communities (default=2)" << std::endl
        << "  -h                : Prints this help message" << std::endl;
}


int main(int argc, const char *argv[]) {
    int size = 10;
    int levels = 3;
    double internalProbability = 0.5;
    int numEdgesPerConnection = 2;
    
    ArgParser argParser(argc, argv);
    for (const char *switchStr = argParser.getNextSwitch(); switchStr != nullptr;
            switchStr = argParser.getNextSwitch())
    {
        if (std::strlen(switchStr) != 2) {
            std::cout << "ERROR: Bad command-line switch " << switchStr << "." << std::endl;
            printUsage(argv[0]);
            return 2;
        }
        switch (switchStr[1]) {
        case 'h': printUsage(argv[0]); return 0;
        case 's': size = argParser.getSwitchArgInt(switchStr); break;
        case 'l': levels = argParser.getSwitchArgInt(switchStr); break;
        case 'i': internalProbability = argParser.getSwitchArgDouble(switchStr); break;
        case 'e': numEdgesPerConnection = argParser.getSwitchArgInt(switchStr); break;
        default:
            std::cout << "ERROR: Unknown command-line switch " << switchStr << "." << std::endl;
            return 3;
        }
    }
    
    const std::vector<int> &positionalArgs = argParser.getPositionalArgs();
    if (positionalArgs.size() != 1) {
        std::cout << "ERROR: Program takes exactly one positional argument, but " << positionalArgs.size()
            << " supplied." << std::endl;
        return 4;
    }
    
    std::string outFileName(argv[positionalArgs[0]]);
    
    Islands::Options options;
    options.communitySize_ = size;
    options.numLevels_ = levels;
    options.internalConnectionProbability_ = internalProbability;
    options.numEdgesPerConnection_ = numEdgesPerConnection;
    Islands islands(options);
    
    std::ofstream ofs(outFileName);
    if (ofs.fail()) {
        std::cout << "ERROR: couldn't open file " << outFileName << " for writing." << std::endl;
        return 5;
    }
    
    Islands::VertexId numVertices = 0;
    std::uint64_t numEdges = 0;
    islands.getGraphSize(numVertices, numEdges);
    ofs << "*Vertices " << numVertices << std::endl;
    ofs << "*Edges " << numEdges << std::endl;
    
    islands.generate([&ofs](const Islands::Edge &edge) -> bool {
        ofs << edge.src_ << ' ' << edge.dest_ << ' ' << edge.weight_ << std::endl;
        return true;
    });
    ofs.close();
    return 0;
}

