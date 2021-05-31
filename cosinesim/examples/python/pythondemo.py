"""
 * Copyright 2020 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 """


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
