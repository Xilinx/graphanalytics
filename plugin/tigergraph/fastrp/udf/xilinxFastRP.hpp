/*
 * Copyright 2019-2021 Xilinx, Inc.
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

#ifndef XILINX_FASTRP_HPP
#define XILINX_FASTRP_HPP

// mergeHeaders 1 name xilinxFastRP
// mergeHeaders 1 section include start xilinxFastRP DO NOT REMOVE!
#include "xilinxFastRPImpl.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <gle/engine/cpplib/headers.hpp>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <math.h>
#include <functional>
// mergeHeaders 1 section include end xilinxFastRP DO NOT REMOVE!

namespace UDIMPL {

// mergeHeaders 1 section body start xilinxFastRP DO NOT REMOVE!
inline ListAccum<float> extract_list(string weights){
  ListAccum<float> wghts;
  string current_weight;
  std::stringstream s_stream(weights);
  while (s_stream.good()) {
    std::getline(s_stream, current_weight, ',');
    wghts.data_.push_back(std::stof(current_weight));
  }
  return wghts;
}

inline float fastrp_rand_func(int64_t v_id, int64_t emb_idx, int64_t seed, int64_t s){
  std::hash<std::string> hasher;
  auto hash = hasher(std::to_string(v_id) + "," + std::to_string(emb_idx) + "," + std::to_string(seed));

  std::mt19937 gen(hash);
  std::uniform_real_distribution<float> distribution(0.0, 1.0);
  float p1 = 0.5 / s, p2 = p1, p3 = 1 - 1.0 / s;
  float v1 = sqrt(s), v2 = -v1, v3 = 0.0;

  float random_value = distribution(gen);
  if (random_value <= p1)
    return v1;
  else if (random_value <= p1 + p2)
    return v2;
  else
    return v3;
}

inline int udf_xilinx_fastrp()
{

}

// mergeHeaders 1 section body end xilinxFastRP DO NOT REMOVE!

}

#endif /* XILINX_FASTRP_HPP */