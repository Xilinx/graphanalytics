import xilCosineSim as xcs
import random as rand

# Setup
VectorLength = 200
NumVectors = 5000
MaxValue = 16383
testVec = []

if __name__ == '__main__':
    # Create options for cosine similarity
    opt = xcs.options()
    opt.vecLength = VectorLength
    opt.xclbinPath = f"/proj/xsjhdstaff3/sachink/ghe/graphanalytics/cosinesim/staging/xclbin/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin"
    testVecIdx = rand.randint(0, NumVectors - 1) # select one index as the test vector

    # Create cosinesim object, datatype is int, size 4 bytes
    cs = xcs.cosinesim(opt, 4)

    # Initialize cosine similarity
    cs.startLoadPopulation(NumVectors)

    # Create vector embeddings
    for vecNum in range(NumVectors):
        # Get a vector
        vecBuf = cs.getPopulationVectorBuffer(vecNum)

        # Fill the vector
        for vecIdx in range(VectorLength):
            val = rand.randint(int(-MaxValue/2), int(MaxValue/2))
            vecBuf.fill(val)  # fill cpp managed memory
            if vecNum == testVecIdx:
                testVec.append(val)

        # Finish filling the vector
        cs.finishCurrentPopulationVector(vecBuf)

    # Finishing embedding creation
    cs.finishLoadPopulationVectors()

    # Finally, Call Cosine similarity match API
    result = cs.matchTargetVector(10, testVec)

    print("Results:")
    print("Similarity   Vector #")
    print("----------   --------")
    for item in result:
        print(f"{item.similarity:.6f}       {item.index}")
