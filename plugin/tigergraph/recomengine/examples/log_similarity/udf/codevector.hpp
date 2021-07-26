/*
 * Copyright 2020-2021 Xilinx, Inc.
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

#ifndef CODEVECTOR_HPP
#define CODEVECTOR_HPP

#include "xilinxRecomEngineImpl.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <chrono>
#include <array>
#include <sstream>

namespace logSimilarityDemo {

using Mutex = xilRecom::Mutex;
using Lock = xilRecom::Lock;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point;
//extern t_time_point timer_start_time;

inline t_time_point &getTimerStartTime() {
    static t_time_point s_startTime;
    return s_startTime;
}


typedef std::int32_t CosineVecValue; ///< A value for an element of a cosine similarity vector
typedef std::array<CosineVecValue,50> CosineArray;
typedef std::unordered_map< std::string , CosineArray > dict;


inline dict &get_glove_dict() {
    static dict glove_dict;
    return glove_dict;
}

inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

inline cleanup(std::string& line)
{
    std::vector< std::string > stopwords =
    {"!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", ":", ";", "<", "=", ">", "?", "@", "[", "\\", "]", "^", "_", "`", "{", "|", "}", "~",
    " i " ,  " me " ,  " my " ,  " myself " ,  " we " ,  " our " ,  " ours " ,  " ourselves " ,  " you " ,  " your " ,  " yours " ,  " yourself " ,
    " yourselves " ,  " he " ,  " him " ,  " his " ,  " himself " ,  " she " ,  " her " ,  " hers " ,  " herself " ,  " it " ,  " its " ,  " itself " ,
    " they " ,  " them " ,  " their " ,  " theirs " ,  " themselves " ,  " what " ,  " which " ,  " who " ,  " whom " ,  " this " ,  " that " ,  " these " ,
    " those " ,  " am " ,  " is " ,  " are " ,  " was " ,  " were " ,  " be " ,  " been " ,  " being " ,  " have " ,  " has " ,  " had " ,  " having " ,  " do " ,
    " does " ,  " did " ,  " doing " ,  " a " ,  " an " ,  " the " ,  " and " ,  " but " ,  " if " ,  " or " ,  " because " ,  " as " ,  " until " ,  " while " ,
    " of " ,  " at " ,  " by " ,  " for " ,  " with " ,  " about " ,  " against " ,  " between " ,  " into " ,  " through " ,  " during " ,  " before " ,
    " after " ,  " above " ,  " below " ,  " to " ,  " from " ,  " up " ,  " down " ,  " in " ,  " out " ,  " on " ,  " off " ,  " over " ,  " under " ,  " again " ,
    " further " ,  " then " ,  " once " ,  " here " ,  " there " ,  " when " ,  " where " ,  " why " ,  " how " ,  " all " ,  " any " ,  " both " ,  " each " ,
    " few " ,  " more " ,  " most " ,  " other " ,  " some " ,  " such " ,  " no " ,  " nor " ,  " not " ,  " only " ,  " own " ,  " same " ,  " so " ,  " than " ,
    " too " ,  " very " ,  " s " ,  " t " ,  " can " ,  " will " ,  " just " ,  " don " ,  " should " ,  " now " };

    std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c){ return std::tolower(c); });
    for (auto i : stopwords)
    {
        replaceAll(line, i, " ");
    }

}

inline std::vector<CosineVecValue> buildGloVeEmbedding(const std::string msg) {

    // Pre-processing
    std::string line = msg;
    cleanup(line);

    // Compute message embedding using word embeddings
    std::stringstream ss(line);
    std::string word;
    CosineArray vec={};
    dict::const_iterator ge;
    int word_cnt =0;
    dict glove_dict = get_glove_dict();
    while(ss >> word)
    {
        ge = glove_dict.find (word);
        if ( ge != glove_dict.end() )
            std::transform(ge->second.begin(), ge->second.end(), vec.begin(), vec.begin(), std::plus<float>());
        word_cnt++;
    }
    for(auto e : vec)
    {
        e /= word_cnt;
        // testout2 << e << " ";
    }
    std::vector<CosineVecValue> result(vec.begin(), vec.end());
    return result;
}


} // namespace

#endif /* CODEVECTOR_HPP */