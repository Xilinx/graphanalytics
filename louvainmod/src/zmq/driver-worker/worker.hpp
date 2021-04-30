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
#include <iostream>
#include <zmq.h>
#include <string>
#include <cassert>
#include <functional>
#include <chrono>
#include "node.hpp"
using namespace std;

class Worker : public Node {
   public:
    Worker(int port = 5555) {
        context = zmq_ctx_new();
        this->socket = zmq_socket(context, ZMQ_REP);
        string host = "tcp://*:" + to_string(port);
        int rc = zmq_bind(this->socket, host.c_str());
        assert(rc == 0);
        cout << "Start server at: " << host << endl;
    }
    ~Worker() {
        if (this->socket != nullptr) zmq_close(this->socket);
        if (context != nullptr) zmq_ctx_destroy(context);
    }

    void listen(function<string(string)> fn, bool verbose = false) {
        while (1) {
            zmq_msg_recv(&this->msg, this->socket, 0);
            if
                constexpr(Node::verbose) cout
                    << "Receive message: " << reinterpret_cast<char*>(zmq_msg_data(&this->msg)) << endl;
            string str = fn(reinterpret_cast<const char*>(zmq_msg_data(&this->msg)));
            zmq_send(this->socket, str.c_str(), 1 + str.size(), 0);
        }
    }
};
