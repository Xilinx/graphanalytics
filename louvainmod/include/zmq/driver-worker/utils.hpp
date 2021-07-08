/*
 * Copyright 2019 Xilinx, Inc.
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
#include <sstream>
#include <cstring>
#include <functional>
#include <chrono>
using namespace std;
string NB(size_t bytes) {
    stringstream ss;
    const char* unit[] = {"KB", "MB", "GB", "TB"};
    if (bytes < 1024) {
        ss << bytes << ' ';
        ss << "B";
    } else {
        double b = bytes / 1024.0;
        int u = 0;

        while (b >= 1024) {
            b /= 1024.0;
            u++;
        }
        ss << b << ' ';
        ss << unit[u];
    }
    return ss.str();
}

double benchmark(function<void()> fn, const char* task = nullptr) {
    auto start = chrono::high_resolution_clock::now();

    if (task != nullptr) {
        cout << "****************************************************" << endl;
        cout << "Start: " << task << " @ " << chrono::system_clock::to_time_t(chrono::system_clock::now()) << endl;
    }

    fn();

    if (task != nullptr) {
        cout << "Finish: " << task << " @ " << chrono::system_clock::to_time_t(chrono::system_clock::now()) << endl;
    }

    auto stop = chrono::high_resolution_clock::now();
    chrono::duration<double> diff = stop - start;
    double time = diff.count();

    if (task != nullptr) {
        cout << "Execution time for \"" << task << "\" is " << time << " seconds." << endl;
        cout << "====================================================" << endl;
    }
    return time;
}
