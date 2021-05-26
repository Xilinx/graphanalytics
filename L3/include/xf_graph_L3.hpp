/*
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#pragma once

#ifndef _XF_GRAPH_L3_HPP_
#define _XF_GRAPH_L3_HPP_

#include "xf_graph_L3_handle.hpp"

namespace xf {
namespace graph {
namespace L3 {

/**
 * @brief The Multi-cards' single source cosine similarity API for dense graph.
 *
 * @param handle Graph library L3 handle
 * @param deviceNm FPGA card ID
 * @param sourceNUM Input, sourceWeights buffer length of source vertex
 * @param sourceWeights Input, weights of the source vertex's out members
 * @param topK Input, the output similarity buffer length
 * @param gr Input, CSR graph of IDs' type of int32_t and weights' type of int32_t
 * @param resultID Output, the topK highest similarity IDs
 * @param similarity Output, similarity values corresponding to theirs IDs
 *
 */
int cosineSimilaritySSDenseMultiCardBlocking(xf::graph::L3::Handle& handle,
                                             int32_t deviceNm,
                                             int32_t sourceNUM,
                                             int32_t* sourceWeights,
                                             int32_t* sourceCoeffs,
                                             int32_t topK,
                                             xf::graph::Graph<int32_t, int32_t>** gr,
                                             int32_t* resultID,
                                             float* similarity);

/**
 * @brief The Non-blocking Multi-cards' single source cosine similarity API for dense graph.
 *
 * @param handle Graph library L3 handle
 * @param deviceNm FPGA card ID
 * @param sourceNUM Input, sourceWeights buffer length of source vertex
 * @param sourceWeights Input, weights of the source vertex's out members
 * @param topK Input, the output similarity buffer length
 * @param gr Input, CSR graph of IDs' type of int32_t and weights' type of int32_t
 * @param resultID Output, the topK highest similarity IDs
 * @param similarity Output, similarity values corresponding to theirs IDs
 *
 */
std::vector<event<int> > cosineSimilaritySSDenseMultiCard(xf::graph::L3::Handle& handle,
                                                          int32_t deviceNm,
                                                          int32_t sourceNUM,
                                                          int32_t* sourceWeights,
                                                          int32_t* sourceCoeffs,
                                                          int32_t topK,
                                                          xf::graph::Graph<int32_t, int32_t>** g,
                                                          int32_t** resultID,
                                                          float** similarity);


#ifdef LOUVAINMOD
void louvainModularity(xf::graph::L3::Handle& handle,
							 int flowMode,
		                     GLV* glv,
							 GLV* pglv,
							 bool opts_coloring,
							 long opts_minGraphSize,
							 double opts_threshold,
							 double opts_C_thresh,
							 int numThreads
                             );
#endif                             
} // L3
} // graph
} // xf
#endif
