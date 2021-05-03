// xilinxlouvain.h
#ifndef _XILINXLOUVAIN_H_
#define _XILINXLOUVAIN_H_

enum {
	ALVEOAPI_NONE=0,
	ALVEOAPI_PARTITION,
	ALVEOAPI_LOAD,
	ALVEOAPI_RUN
};

int create_alveo_partitions(int argc, char *argv[]);
int load_alveo_partitions(int argc, char *argv[]);
int louvain_modularity_alveo(int argc, char *argv[]);

#endif
