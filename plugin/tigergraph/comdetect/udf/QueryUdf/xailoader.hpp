/*
 * xai_loader.h
 * Copyright (c) 2019, Xilinx, Inc.  All Rights Reserved.
 * Class for loading shared libraries for interfacing with Xilinx Alveo cards
 */

#ifndef _XAI_LOADER_H
#define _XAI_LOADER_H

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <mutex>

#include "xai.h"


namespace xai {

  typedef std::chrono::time_point<std::chrono::high_resolution_clock> t_time_point, *pt_time_point;

  class LoaderException : public std::exception {
  public:
    LoaderException(const std::string& msg) : _msg("XAI library loading error: " + msg) { }
    virtual ~LoaderException() throw() { }
    virtual const char * what() const throw() { return _msg.c_str(); }
  private:
    std::string _msg;
  };

  class Loader {
  public:
    Loader(): _host_libname(""), _lib(nullptr), _sym(nullptr), _xai_handle(nullptr),
    _fptr_open(nullptr), _fptr_cache_reset(nullptr), _fptr_cache(nullptr),
    _fptr_cache_flush(nullptr),_fptr_execute(nullptr), _fptr_close(nullptr),
    _fptr_create_partitions(nullptr), _fptr_load_partitions(nullptr),
    _fptr_execute_louvain(nullptr)
    { }
    ~Loader() { }

    void check_error(std::string& errmsg) {
      char* err = dlerror();
      if (err != NULL) {
        errmsg += err;
        throw LoaderException(errmsg);
      }
    }

    bool load_library(const std::string& host_libname)
    {
      if(_lib)
      return true;

      char *err = dlerror();
      _host_libname = host_libname;
      std::string errmsg=host_libname;

      //_lib = dlopen(host_libname.c_str(), RTLD_LAZY | RTLD_GLOBAL);
      _lib = dlopen(host_libname.c_str(), RTLD_LAZY);
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_open");
      _fptr_open = (t_fp_xai_open)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_cache_reset");
      _fptr_cache_reset = (t_fp_xai_cache_reset)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_cache");
      _fptr_cache = (t_fp_xai_cache)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_cache_flush");
      _fptr_cache_flush = (t_fp_xai_cache_flush)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_execute");
      _fptr_execute = (t_fp_xai_execute)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_close");
      _fptr_close = (t_fp_xai_close)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_create_partitions");
      _fptr_create_partitions = (t_fp_xai_create_partitions)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_load_partitions");
      _fptr_load_partitions = (t_fp_xai_load_partitions)_sym;
      check_error(errmsg);

      _sym = (void *) dlsym(_lib, "xai_execute_louvain");
      _fptr_execute_louvain = (t_fp_xai_execute_louvain)_sym;
      check_error(errmsg);

      print();
      return true;
    }

    void print() {
      std::cout << "Function pointers: " << (void*)_fptr_open << " " << (void*)_fptr_execute << " " << (void*)_fptr_close << " "
                << (void*)_fptr_cache << " " << (void*)_fptr_create_partitions << " " << (void*)_fptr_load_partitions << " "
                << (void*)_fptr_execute_louvain << " " << std::flush;

    }

    xaiHandle open(p_xai_context context) {
      _xai_handle = _fptr_open(context);
      return _xai_handle;
    }

    bool cache_reset(xaiHandle xaiInstance) {
      int res = _fptr_cache_reset(xaiInstance);
      return true;
    }

    bool cache(xaiHandle xaiInstance, SPATIAL_dataType* arg_Y, unsigned int numElements) {
      int res = _fptr_cache(xaiInstance, arg_Y, numElements);
      return true;
    }

    bool cache_flush(xaiHandle xaiInstance) {
      int res = _fptr_cache_flush(xaiInstance);
      return true;
    }

    int execute(xaiHandle xaiInstance, t_xai_algorithm algo, SPATIAL_dataType* arg_X, p_xai_id_value_pair result) {
      return _fptr_execute(xaiInstance, algo, arg_X, result);
    }

    void close(xaiHandle xaiInstance) {
      if(_lib) {
        int status = _fptr_close(xaiInstance);
        dlclose(_lib);
        _lib = nullptr;
      }
    }

    int create_partitions(int argc, char** argv) {
      return _fptr_create_partitions(argc, argv);
    }

    int load_partitions(int argc, char** argv) {
      return _fptr_load_partitions(argc, argv);
    }

    int execute_louvain(int argc, char** argv) {
      std::cout << "In execute louvain\n" << std::flush;
      return _fptr_execute_louvain(argc, argv);
    }

    bool isOpen() {
      return _lib != nullptr;
    }

  private:
    std::string _host_libname;
    void* _lib;
    void* _sym;
    xaiHandle _xai_handle;
    t_fp_xai_open _fptr_open;
    t_fp_xai_cache_reset _fptr_cache_reset;
    t_fp_xai_cache _fptr_cache;
    t_fp_xai_cache_flush _fptr_cache_flush;
    t_fp_xai_execute _fptr_execute;
    t_fp_xai_close _fptr_close;
    t_fp_xai_create_partitions _fptr_create_partitions;
    t_fp_xai_load_partitions _fptr_load_partitions;
    t_fp_xai_execute_louvain _fptr_execute_louvain;
  }; // class Loader


  // Open Alveo parameters
  extern const char* host_libname;
  extern const char* xclbin_filename;

  // Load alveo parameters
  extern bool tg_partition;
  extern bool use_saved_partition;
  extern std::string graph_file;
  extern std::string louvain_project;
  extern std::string num_partitions;
  extern std::string num_devices;

  // Mutexes
  extern std::mutex writeMutex;
  extern std::mutex writeMutexOpenAlveo;

  // States
  extern bool executedLouvain;
  extern bool loadedAlveo;
  extern bool openedAlveo;

  // Construct loader and start timer
  extern Loader xaiLoader;
  extern t_time_point timer_start_time;

}

#endif // _XAI_LOADER_H
