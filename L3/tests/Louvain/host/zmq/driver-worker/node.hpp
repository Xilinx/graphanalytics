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

#include <zmq.h>
#include <cassert>
#include <iostream>
#include "utils.hpp"
using namespace std;
#ifndef __NODE_HPP__
#define __NODE_HPP__

/**
 * @brief Node, presents a socket on a server
 * It sends or receive a message to or from other Nodes
 */
class Node {
   public:
    Node() {
        int rc = zmq_msg_init(&msg);
        assert(rc == 0);
    }

    /**
      * @brief initialize a node with given 0MQ Context and Socket
      *
      * @param p_context, 0MQ Context
      * @param p_socket, 0MQ Socket
      *
      */
    Node(void* p_context, void* p_socket) : Node() {
        this->context = p_context;
        this->socket = p_socket;
    }

    virtual ~Node() { zmq_msg_close(&msg); };

    /**
      * @brief send given array, no.bytes and flag
      *
      * @param ptr, pointer to contents
      * @param nbytes, no.bytes to send
      * @param flags, 0MQ flag for send function
      */
    int send(const void* ptr, size_t nbytes, int flags = ZMQ_SNDMORE) const {
        int rc = 0;
        if (ptr == nullptr || nbytes == 0) {
            rc = zmq_send(socket, ptr, 0, 0);
        } else {
            rc = zmq_send(socket, ptr, nbytes, flags);
        }
        if
            constexpr(verbose) cout << "Send " << NB(nbytes) << " successfully." << endl;
        return rc;
    };

    /**
      * @brief send given c++ string
      *
      * @param str, string
      * @param flags, 0MQ flag for send function
      */
    int send(const string str, int flag = ZMQ_SNDMORE) const { return send(str.c_str(), str.size() + 1, flag); }

    /**
      * @brief send given scalar within multiple sends
      *
      * @param val, scalar type
      * @param flags, 0MQ flag for send function
      */
    template <typename T>
    int send(T val, typename enable_if<!is_pointer<T>::value, int>::type flags = ZMQ_SNDMORE) const {
        return send((void*)&val, sizeof(val), flags);
    }

    /**
      * @brief send given c string
      *
      * @param ptr, c string pointer
      */
    int send(const char* ptr = nullptr) const { return send(ptr, ptr == nullptr ? 0 : strlen(ptr) + 1, 0); }

    /**
      * @brief receive given array and no.bytes
      *
      * @param ptr, pointer to fill with received contents
      * @param nbytes, no.bytes to receive
      * @param flags, 0MQ flag for receive function, default 0
      */
    int receive(void* ptr, size_t nbytes, int flags = 0) {
        int rc = zmq_msg_recv(&msg, socket, flags);
        if (rc != -1 && ptr != nullptr) {
            size_t ms = zmq_msg_size(&msg);
            size_t nCopy = ms > nbytes ? nbytes : ms;
            memcpy(ptr, zmq_msg_data(&msg), nCopy);
        } else {
            cout << "ERROR : Receive " << NB(nbytes) << " failed!" << endl;
        }
        if
            constexpr(verbose) cout << "Received " << NB(nbytes) << " successfully." << endl;
        return rc;
    }

    /**
      * @brief receive a scalar value with given type
      *
      * @param val the reference of the variable to receive value
      * @param flags 0MQ flag for receive function, default 0
      */
    template <typename T>
    int receive(const T& val, typename enable_if<!is_pointer<T>::value, int>::type flags = 0) {
        return receive((void*)&val, sizeof(val), flags);
    }

    /**
      * @brief receive a c++ string
      * @param flags 0MQ flag for receive function, default 0
      * @return a c++ string
      */
    string receive(int flags = 0) {
        int rc = zmq_msg_recv(&msg, this->socket, flags);
        if (rc > 0) {
            string str(reinterpret_cast<char*>(zmq_msg_data(&msg)));
            if
                constexpr(verbose) cout << "Receive message: " << str << endl;
            return move(str);
        } else {
            return "";
        }
    }

   protected:
    void *context, *socket;
    zmq_msg_t msg;
#ifdef VERBOSE
    static constexpr bool verbose = true;
#else
    static constexpr bool verbose = false;
#endif
};
#endif
