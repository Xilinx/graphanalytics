import xilCosineSim as xcs
import random as rand

# Setup
VectorLength = 200
NumVectors = 1000
MaxValue = 16383
testVec = []

if __name__ == '__main__':
    # Create options for cosine similarity
    opt = xcs.options()
    opt.vecLength = VectorLength
    testVecIdx = rand.randint(0, NumVectors - 1) # select one index as the test vector

    # Create cosinesim object, datatype is int, size 4 bytes
    cs = xcs.cosinesim(opt, 4)

    print("Loading population vectors into Alveo card...")
    # Initialize cosine similarity
    cs.startLoadPopulation(NumVectors)

    # Create vector embeddings
    for vecId in range(NumVectors):
        # Get a vector
        vecBuf = cs.getPopulationVectorBuffer(vecId)

        # Fill the vector
        valVec = xcs.buildRandData(int(MaxValue/2), int(-MaxValue/2), VectorLength)
        if vecId == testVecIdx:
            testVec += valVec
        vecBuf.append(valVec)

        # Finish filling the vector
        cs.finishCurrentPopulationVector(vecBuf)

    # Finishing embedding creation
    cs.finishLoadPopulation()

    # Finally, Call Cosine similarity match API
    result = cs.matchTargetVector(10, testVec)

    print("Results:")
    print("Similarity   Vector #")
    print("----------   --------")
    for item in result:
        print('{:.6f}'.format(item.similarity) + "       " + str(item.index))
