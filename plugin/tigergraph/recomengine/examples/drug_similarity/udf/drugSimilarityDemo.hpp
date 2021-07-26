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
 * WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DRUGSIMILARITY_DEMO_HPP
#define DRUGSIMILARITY_DEMO_HPP

// mergeHeaders 1 name drugSimilarityDemo

// mergeHeaders 1 section include start drugSimilarityDemo DO NOT REMOVE!
#include "word2vec.h"
#include "codevector.hpp"
// mergeHeaders 1 section include end drugSimilarityDemo DO NOT REMOVE!

//#####################################################################################################################

namespace UDIMPL {

// mergeHeaders 1 section body start drugSimilarityDemo DO NOT REMOVE!

inline bool concat_uint64_to_str(string& ret_val, uint64_t val) {
    (ret_val += " ") += std::to_string(val);
    return true;
}

inline bool udf_reset_timer(bool dummy) {
    drugSimilarityDemo::getTimerStartTime() = std::chrono::high_resolution_clock::now();
    return true;
}

inline double udf_elapsed_time(bool dummy) {
    drugSimilarityDemo::t_time_point cur_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> l_durationSec = cur_time - drugSimilarityDemo::getTimerStartTime();
    double l_timeMs = l_durationSec.count() * 1e3;
    return l_timeMs;
}

inline double udf_calculate_normal(ListAccum<int64_t> vec) {
    double norm = 0.0;
    for (unsigned i =0, end = vec.size(); i < end; ++i) {
        double elt = vec.get(i);
        norm += elt * elt;
    }
    norm = std::sqrt(norm);
//    std::cout << "udf_calculate_normal: size=" << vec.size() << ", norm=" << norm << std::endl;
    return norm;
}

inline double udf_cos_theta(ListAccum<int64_t> vec_A, double norm_d_A, ListAccum<int64_t> vec_B, double norm_d_B) {
    double prod = 0.0;
    if (norm_d_A == 0 or norm_d_B == 0) {
        return 0.0;
    }
    for (unsigned i = 0, end = vec_A.size(); i < end; ++i)
        prod = prod + vec_A.get(i) * vec_B.get(i);
    double res = prod / (norm_d_A * norm_d_B);
    //std::cout << "val = " << res << std::endl;
    return res;
}


// node2vec function: given random walk sequence, this function trains vector using skip-gram model
 inline void udf_node2vec(int dimension, std::string input_file) {

    drugSimilarityDemo::Model model(dimension);
    model.sample_ = 0;
    // model.window = 10;
    int n_workers = 4;
    std::vector<drugSimilarityDemo::SentenceP> sentences;

    size_t count =0;
    const size_t max_sentence_len = 200;

    drugSimilarityDemo::SentenceP sentence(new drugSimilarityDemo::Sentence);
    std::ifstream in(input_file);
    if(in.is_open()) {
        while (true) {
            std::string s;
            in >> s;
            if (s.empty()) break;
            ++count;
            sentence->tokens_.push_back(std::move(s));
            if (count == max_sentence_len) {
                count = 0;
                sentences.push_back(std::move(sentence));
                sentence.reset(new drugSimilarityDemo::Sentence);
            }
        }
    }
    else std::cout << "ERROR: Could not open the path file " << input_file << std::endl;
    
    if (!sentence->tokens_.empty())
        sentences.push_back(std::move(sentence));

    model.build_vocab(sentences);
    model.train(sentences, n_workers);
    model.build_map();
    //model.save("/tmp/embeddings.csv");

}

inline void udf_load_node2vec(int dimension, std::string input_file) {
    drugSimilarityDemo::Model model(dimension);
    model.sample_ = 0;
    model.load(input_file);
    model.build_map();    
}

inline int udf_get_embedding_length() {
    return drugSimilarityDemo::get_embeddings_length();
}

inline void udf_set_embedding_length(int len) {
    drugSimilarityDemo::get_embeddings_length() = len;
}

inline bool udf_check_atom_embedding(std::string word) {
    drugSimilarityDemo::Map embeddings_map = drugSimilarityDemo::get_embeddings_map();

    auto lookup = embeddings_map.find(word);
    if(lookup == embeddings_map.end()) { // No embedding was created for the Vertex
        return false;
    }
    else return true;
}

inline ListAccum<int64_t> udf_get_nodeVec(std::string word) {
    ListAccum<uint64_t> result;

    drugSimilarityDemo::Map embeddings_map = drugSimilarityDemo::get_embeddings_map();

    std::ofstream out1("/tmp/found.txt", std::ofstream::out | std::ofstream::app);
    std::ofstream out2("/tmp/notfound.txt", std::ofstream::out | std::ofstream::app);
    auto lookup = embeddings_map.find(word);
    if(lookup != embeddings_map.end()) { // If embedding was created for the Vertex
        out1 << word << std::endl;
        drugSimilarityDemo::Vector embedding = lookup->second;
        float max_mag = 0;
        for (auto i: embedding) {
            if(max_mag < std::abs(i)) max_mag = std::abs(i);
        }
        // convert to int and scale up/down to avoid FPGA underflow/overflow
        float scale = 100/max_mag; // scaling to keep each element within 100, use 2 digit precision
        for (float value : embedding) {
            result += value*scale;
        }
    }
    else out2 << word << std::endl;

    return result;
}

// mergeHeaders 1 section body end drugSimilarityDemo DO NOT REMOVE!

}  // namespace UDIMPL
#endif
