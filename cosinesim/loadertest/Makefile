
# To run, set LD_LIBRARY_PATH to include .

all: libapi.so libapi_loader.a demost demody

CXXFLAGS += -fPIC -std=c++11 -O0 -g -Wall -Wno-unknown-pragmas -Wno-unused-label -w
LDFLAGS += -lstdc++ -ldl -lrt -Wno-unused-label -Wno-narrowing -DVERBOSE

libapi.so: api.o
	g++ $(CXXFLAGS) -rdynamic -shared -o $@ $^ $(LDFLAGS) -Wl,--export-dynamic

libapi_loader.a: api_loader.o
	ar ru $@ $^
	ranlib $@
	
demost: main.cpp
	g++ -o $@ $(CXXFLAGS) $^ -L . -lapi
	
demody: main.cpp
	g++ -o $@ $(CXXFLAGS) $^ -L . -lapi_loader -ldl
	
clean:
	rm -rf libapi.so libapi_loader.a *.o demost demody
