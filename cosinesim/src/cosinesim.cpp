#include <vector>
#include <cstdint>
#include "cosinesim.hpp"

namespace xilinx_apps {
namespace cosinesim {

class PrivateImpl : public ImplBase {

public:
	int indexDeviceCuNm, indexSplitNm, indexNumVertices, indexVecLength;
	const int splitNm = 3;    // kernel has 4 PUs, the input data should be splitted into 4 parts
	const int channelsPU = 4; // each PU has 4 HBM channels
	const int cuNm = 2;
	const int deviceNeeded = 1;
	const int channelW = 16;
	int oldVectorRowIndex;
	int32_t** numVerticesPU = new int32_t*[deviceNeeded * cuNm]; // vertex numbers in each PU

	//TODO: where to define vecLength

	int32_t numEdges;
	int vecLength;
	int64_t numVertices;

	//std::vector<int> loadOldVectorCnt(channelsPU,0);
	std::vector<int> loadPopulationCnt;

	//xf::graph::Graph<int32_t, int32_t>** g = new xf::graph::Graph<int32_t, int32_t>*[deviceNeeded * cuNm];
	std::vector<xf::graph::Graph<int32_t, int32_t>*> g;

private:
	CosineSimBase* cosinesimPtr;

public:
	PrivateImpl(CosineSimBase* ptr){
		cosinesimPtr = ptr;
		indexDeviceCuNm=0;
		indexSplitNm=0;
		indexNumVertices=0;
		indexVecLength=3;
		oldVectorRowIndex=0;
		vecLength = ptr->getOptions().vecLength;
		numEdges = vecLength - 3;
		numVertices =ptr->getOptions().numVertices;
	}

	~PrivateImpl(){}
	void startLoadPopulation(){
		indexDeviceCuNm=0;
		indexSplitNm=0;
		indexNumVertices=0;
		indexVecLength=3;
		oldVectorRowIndex=0;
		//initialize numVerticesPU
		//numVerticesPU = new std::vector<std::vector<int32_t>> (deviceNeeded * cuNm,std::vector<int32_t>(splitNm));
		//numVerticesPU.resize(deviceNeeded*cuNm);
		//for(int i=0;i<deviceNeeded*cuNm; i++){
		//	numVerticesPU[i].resize(splitNm);
		//}
		loadPopulationCnt.resize(channelsPU);
		g.resize(deviceNeeded*cuNm);

		for(auto& cnt: loadPopulationCnt){
			cnt = 0;
		}
		int general = ((numVertices + deviceNeeded * cuNm * splitNm * channelsPU - 1)/(deviceNeeded * cuNm * splitNm * channelsPU)) * channelsPU;
		int rest = numVertices - general * (deviceNeeded * cuNm * splitNm - 1);
		for (int i = 0; i < deviceNeeded * cuNm; ++i) {
			for (int j = 0; j < splitNm; ++j) {
				numVerticesPU[i][j] = general;
			}
		}
		numVerticesPU[deviceNeeded * cuNm - 1][splitNm - 1] = rest;

		for(int i=0;i<deviceNeeded*cuNm; i++){
			g[i] = new xf::graph::Graph<int32_t, int32_t>("Dense", 4 * splitNm, numEdges, numVerticesPU[i]);
		}

		vecLength = cosinesimPtr->getOptions().vecLength;
		numEdges = vecLength - 3;
		numVertices = cosinesimPtr->getOptions().numVertices;
	}

	void *getPopulationVectorBuffer(RowIndex &rowIndex){


		int32_t edgeAlign8 = ((numEdges + channelW - 1) / channelW) * channelW;

		if(indexDeviceCuNm < deviceNeeded * cuNm) {
			int subChNm;
			indexDeviceCuNm = indexSplitNm == splitNm ? ++indexDeviceCuNm : indexDeviceCuNm;
			if(indexSplitNm == splitNm)  indexSplitNm =0;
			else {
				if(indexNumVertices==numVerticesPU[indexDeviceCuNm][indexSplitNm]){
					//inner loop has finished.
					indexSplitNm++;
					indexNumVertices =0;
				} else {
					if(indexVecLength == vecLength) {
						indexNumVertices++;
						indexVecLength =0;
					} else {
						indexVecLength++;
					}


				}
			}
			subChNm = (numVerticesPU[indexDeviceCuNm][indexSplitNm] + channelsPU - 1) / channelsPU;


			rowIndex = ++oldVectorRowIndex;
			loadPopulationCnt[indexNumVertices / subChNm] += 1;
			return &(g[indexDeviceCuNm]->weightsDense[indexSplitNm * channelsPU + indexNumVertices / subChNm][loadPopulationCnt[indexNumVertices / subChNm] * edgeAlign8 + indexVecLength - 3]);
		}
		return nullptr;
	};



}; // class PrivateImpl


extern "C" {
    ImplBase *createImpl(CosineSimBase* ptr){
    	return new PrivateImpl(ptr);
    }

    void destroyImpl(ImplBase *pImpl){
    	delete pImpl;
    }
}


} //cosinesim
} //xilinx_app
