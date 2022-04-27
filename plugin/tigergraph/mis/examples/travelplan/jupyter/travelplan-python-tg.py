"""
 * Copyright 2021 Xilinx, Inc.
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
"""

# Imports
import time
from pathlib import Path, PurePosixPath
import pyTigerGraph as tg

# Login Setup
hostName = "localhost"                              # TG server hostname
userName = "tigergraph"                             # TG user name
passWord = "tigergraph"                             # TG user password

# Local (client) Query Path Setup
localRepoLocation = Path("/opt/xilinx/apps")
exampleLocation = Path("graphanalytics/integration/Tigergraph-3.x/mis/0.6/examples/travelplan/") # NOTE: modify path according to the mis version installed
queryFileLocation = localRepoLocation / exampleLocation / "query"

# Server Data Path Setup
serverRepoLocation = PurePosixPath("/opt/xilinx/apps")
serverDataLocation = serverRepoLocation / PurePosixPath(exampleLocation) / "data"
tp2woInfile = serverDataLocation / "travelplan2workorders100.csv"
tp2trInfile = serverDataLocation / "travelplan2trucks100.csv"


## Create Graph: connect to TG server and create graph
def create_graph(graphName):
    conn = tg.TigerGraphConnection(host='http://' + hostName, graphname='', username=userName, password=passWord, useCert=False)
    print("\n--------- Creating New graph ----------")
    print(conn.gsql(f'create graph {graphName}()', options=[]))

    # connect to TG server with new graph
    print(f'Using graph {graphName}')
    conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord, useCert=False)
    
    return conn


## Create Schema
def create_schema(conn, graphName):
    print("\n--------- Creating New Schema ----------")
    schemaFile = queryFileLocation / "schema.gsql"

    with open(schemaFile) as fh:
        qStrRaw = fh.read()
        qStr = qStrRaw.replace('@graph', graphName)
        print(conn.gsql(qStr))
    print("\n----------------- Done -----------------")
        
    # print a sneak peek of the schema
    schema = conn.getSchema(False)
    print(f"\nTotal {len(schema['VertexTypes'])} Vertices created:")
    i = 1
    for v in schema['VertexTypes']:
        print(f" {i}. {v['Name']}")
        i+=1

    print(f"\nwith the following edges:")
    i = 1
    for e in schema['EdgeTypes']:
        print(f" {i}. {e['Name']}: {e['FromVertexTypeName']} - {e['ToVertexTypeName']}")
        i+=1


## Load Data
def load_data(conn, graphName):
    print("\n--------- Loading data into graph ----------")
    loadFile = queryFileLocation / "load.gsql"

    with open(loadFile) as fh:
        qStrRaw = fh.read()
        
        qStr = qStrRaw.replace('@graph', graphName)
        print(conn.gsql(qStr))
        print(conn.gsql(f'USE GRAPH {graphName}\n RUN LOADING JOB load_tp2tr USING tp2tr_infile="{tp2trInfile}"'))
        print(conn.gsql(f"USE GRAPH {graphName}\n DROP JOB load_tp2tr"))
        print(conn.gsql(f'USE GRAPH {graphName}\n RUN LOADING JOB load_tp2wo USING tp2wo_infile="{tp2woInfile}"'))
        print(conn.gsql(f"USE GRAPH {graphName}\n DROP JOB load_tp2wo"))
    print("------------------- Done -------------------")


## Build Edges
def build_edges(conn, graphName):
    print("\n--- Installing and running Build edges query ---")
    buildEdgesFile = queryFileLocation / "build_edges.gsql"
    with open(buildEdgesFile) as fh:
        qStrRaw = fh.read()
        qStr = qStrRaw.replace('@graph', graphName)
        print(conn.gsql(qStr))

    print('Building edges for travelplan vertices\n'+'-'*38)

    result = conn.runInstalledQuery('build_edges', timeout=240000000)
    for res in result:
        for k in res:
                print(k, ":", res[k])

    print('Waiting for Graph to stabilize...')
    tp2tp_edges = 0
    while(True):
        time.sleep(10)
        edges = conn.getEdgeCountFrom(edgeType='tp2tp')
        if(edges == tp2tp_edges):
            break
        tp2tp_edges = edges
    print('Done!')


## Install all queries
def install_queries(conn, graphName):
    print("\n--------- Installing Queries ----------")
    queryFiles = [queryFileLocation / "tg_supply_chain_schedule.gsql",
                  queryFileLocation / "xlnx_supply_chain_schedule.gsql"]

    for qf in queryFiles:
        with open(qf) as fh:
            print(f"installing queries in {qf}...")
            qStrRaw = fh.read()
            qStr = qStrRaw.replace('@graph', graphName)
            print(conn.gsql(qStr))

    print("--------- All queries installed ----------")


## Generate CSR
def generate_csr(conn):
    tStart = time.perf_counter()
    print('\nAssign unique IDs to Travel Plan vertices\n'+'-'*41)
    result = conn.runInstalledQuery('assign_ids', {'v_type': "travel_plan"}, timeout=240000000)
    for res in result:
        for k in res:
                print(k, ":", res[k])
                
    print('\nBuild Row Ptr and Column Idx arrays\n'+'-'*35)
    result = conn.runInstalledQuery('build_csr', {'v_type': "travel_plan", 'e_type': "tp2tp"}, timeout=240000000)
    for res in result:
        for k in res:
                print(k, ":", res[k])

    print(f'completed in {time.perf_counter() - tStart:.4f} sec')


## MIS TG: CPU only
def mis_tg(conn):
    print('\nRunning Queries on TG CPU\n'+'-'*25)
    tStart = time.perf_counter()
    global tDuration_cpu
    result = conn.runInstalledQuery('tg_supply_chain_schedule',
                                    {'v_type': "travel_plan", 
                                     'e_type': "tp2tp",
                                     'max_iter_per_schedule': 100,
                                     'num_schedule': 0,             # 0 corresponds to all possible schedules
                                     'print_accum': True,
                                     'sched_file_path': "",
                                     'sched_metadata_path': ""},
                                    timeout=240000000)
    tDuration_cpu = 1000*(time.perf_counter() - tStart)
    print(f"Round Trip time: {tDuration_cpu:.2f} msec")
    return result


## MIS Xilinx: FPGA only
def mis_xlnx(conn):
    print('\nRunning Queries on FPGA\n'+'-'*23)
    tStart = time.perf_counter()
    global tDuration_alv
    result = conn.runInstalledQuery('supply_chain_schedule_alveo',
                                    {'v_type': "travel_plan", 
                                     'e_type': "tp2tp",
                                     'num_schedule': 0,             # 0 corresponds to all possible schedules
                                     'print_accum': True,
                                     'sched_file_path': "",
                                     'sched_metadata_path': ""},
                                    timeout=240000000)
    tDuration_alv = 1000*(time.perf_counter() - tStart)
    print(f"Round Trip time: {tDuration_alv:.2f} msec")
    return result


## main
if __name__ == '__main__':

    # set graph name
    graphname = f'travelplan_graph_{userName}'   # TG graph name
    tg_conn = create_graph(graphname)
    
    # create TG database
    create_schema(tg_conn, graphname)
    load_data(tg_conn, graphname)
    build_edges(tg_conn, graphname)
    install_queries(tg_conn, graphname)
    generate_csr(tg_conn)
    
    # run TigerGraph Supply Chain Optimization query
    result = mis_tg(tg_conn)
    for res in result:
        for k in res:
            if k == '@@mis_mdata':
                print("Schedule Sizes :", [x['sz'] for x in res[k]])
            else:
                print(k, ":", res[k])
    
    # run Xilinx Supply Chain Optimization query
    result = mis_xlnx(tg_conn)
    for res in result:
        for k in res:
            if k == '@@mis_mdata':
                print("Schedule sizes :", res[k])
            elif k == 'StatusMessage':
                print("Status :", res[k])
                if res[k] != "Success!":
                    print(f"\nError: Please (re)run the CSR generation method above (generate_csr()) and run mis_xlnx() again!\n")
            else:
                print(k, ":", res[k])
    
    # compute speedup
    print(f"\nSpeedup = {tDuration_cpu/tDuration_alv:.2f}X")
    

