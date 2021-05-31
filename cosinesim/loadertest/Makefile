# Copyright 2020 Xilinx, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WANCUNCUANTIES ONCU CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# To run, set LD_LIBRARY_PATH to include .

all: libapi.so libapi_loader.a demost demody

CXXFLAGS += -fPIC -std=c++11 -O0 -g -Wall -Wno-unknown-pragmas -Wno-unused-label -w
LDFLAGS += -lstdc++ -ldl -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE

TG_APP_ROOT = /home2/tigergraph/tigergraph/app/3.1.0
TG_TOOL_PATH = $(TG_APP_ROOT)/.syspre
TG_LIB_PATH2 = $(TG_APP_ROOT)/bin
TG_CXXFLAGS = -Wall -fmessage-length=0 -Wno-unused -fpermissive -fno-omit-frame-pointer -ldl -DRELEASE -O3 \
    -DREMOVETESTASSERT -D__STDC_FORMAT_MACROS -fPIC -std=c++11 --sysroot=$(TG_TOOL_PATH)

libapi.so: api.o
	g++ --version; \
	g++ $(CXXFLAGS) -rdynamic -shared -o $@ $^ $(LDFLAGS) -Wl,--export-dynamic

libapi_loader.a: api_loader.o
	ar ru $@ $^
	ranlib $@
	
demost: main.cpp
	g++ -o $@ $(CXXFLAGS) $^ -L . -lapi
	
demody: main.cpp
	g++ -o $@ $(CXXFLAGS) $^ -L . -lapi_loader -ldl

demohy: main.cpp
	export PATH=$(TG_TOOL_PATH)/usr/bin; \
	export LD_LIBRARY_PATH=$(TG_TOOL_PATH)/usr/lib_ld3:$(TG_TOOL_PATH)/usr/lib_ld3; \
	g++ --version;
	g++ -o $@ $(TG_CXXFLAGS) -DHYBRID_DEMO=1 $^  -L $(TG_LIB_PATH2) -Wl,-rpath,$(TG_LIB_PATH2) -ljemalloc -L . -ldl

runhy: demohy
	export PATH=.:$(TG_TOOL_PATH)/usr/bin; \
	export LD_LIBRARY_PATH=.:$(TG_LIB_PATH2):$(TG_TOOL_PATH)/usr/lib_ld3; \
	export LD_PRELOAD=$(TG_LIB_PATH2)/libjemalloc.so; \
	export MALLOC_CONF=prof:true,prof_active:false; \
	demohy; \
	/usr/bin/ldd demohy

#	/tools/baton/totalview/totalview/bin/totalview demohy; 
	
clean:
	rm -rf libapi.so libapi_loader.a *.o demost demody demohy
