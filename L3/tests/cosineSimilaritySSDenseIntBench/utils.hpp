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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

class ArgParser {
public:
  ArgParser(int &argc, const char **argv) {
    for (int i = 1; i < argc; ++i)
      mTokens.push_back(std::string(argv[i]));
  }
  bool getCmdOption(const std::string option, std::string &value) const {
    std::vector<std::string>::const_iterator itr;
    itr = std::find(this->mTokens.begin(), this->mTokens.end(), option);
    if (itr != this->mTokens.end() && ++itr != this->mTokens.end()) {
      value = *itr;
      return true;
    }
    return false;
  }

private:
  std::vector<std::string> mTokens;
};

// Compute time difference
unsigned long diff(const struct timeval *newTime,
                   const struct timeval *oldTime) {
  return (newTime->tv_sec - oldTime->tv_sec) * 1000000 +
         (newTime->tv_usec - oldTime->tv_usec);
}

template <typename T> T *aligned_alloc(std::size_t num) {
  void *ptr = NULL;
  if (posix_memalign(&ptr, 4096, num * sizeof(T)))
    throw std::bad_alloc();
  return reinterpret_cast<T *>(ptr);
}

template <int splitNm>
int checkData(std::string goldenFile, int32_t *kernelID,
              float *kernelSimilarity) {
  int err = 0;
  char line[1024] = {0};
  std::fstream goldenfstream(goldenFile.c_str(), std::ios::in);
  if (!goldenfstream) {
    std::cout << "Error: " << goldenFile << " file doesn't exist !"
              << std::endl;
    exit(1);
  }

  std::unordered_map<int32_t, float> ref_map;
  int golden_num = 0;
  while (goldenfstream.getline(line, sizeof(line))) {
    std::stringstream data(line);
    std::string tmp;
    int tmp_id;
    float tmp_data;

    data >> tmp_id;
    data >> tmp;
    tmp_data = std::stof(tmp);
    ref_map.insert(std::make_pair(tmp_id, tmp_data));
    golden_num++;
  }

  int index = 0;
  for (int i = 0; i < golden_num; i++)
    std::cout << "kernel id=" << kernelID[i]
              << " kernel_similarity=" << kernelSimilarity[i] << std::endl;

  index = 0;
  while (index < golden_num) {
    auto it = ref_map.find((int32_t)kernelID[index]);
    if (it != ref_map.end()) {
      float ref_result = it->second;
      if (std::abs(kernelSimilarity[index] - ref_result) > 0.000001) {
        std::cout << "Error: id=" << kernelID[index]
                  << " golden_similarity=" << ref_result
                  << " kernel_similarity=" << kernelSimilarity[index]
                  << std::endl;
        err++;
      }
    } else {
      std::cout << "Error: not find! id=" << kernelID[index]
                << " kernel_similarity=" << kernelSimilarity[index]
                << std::endl;
      err++;
    }
    index++;
  }

  return err;
}

template <int PUNUM>
void readInCoeffs(std::string coeffsFile, // input: file stream
                  unsigned int length,    // input: numbers
                  unsigned int requestNm, // input: request
                  int32_t **buffer) {     // output: output buffers

  std::fstream coeffsFstream(coeffsFile.c_str(), std::ios::in);
  if (!coeffsFstream) {
    std::cout << "Error: " << coeffsFile << " file doesn't exist !"
              << std::endl;
    exit(1);
  }

  int id = 0;
  int row = 0;
  char line[1024] = {0};
  if (!coeffsFstream) {
    for (int j = 0; j < length; ++j) {
      for (int i = 0; i < requestNm; ++i)
        buffer[i][j] = 1;
    }
  } else {
    while (coeffsFstream.getline(line, sizeof(line))) {
      std::stringstream data(line);
      int tmp;
      data >> tmp;
      for (int i = 0; i < requestNm; ++i)
        buffer[i][id] = tmp;
      id++;
    }
  }
  coeffsFstream.close();
}
