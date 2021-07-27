"""
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
"""

import time
import random as rand
from pathlib import Path, PurePosixPath
import pyTigerGraph as tg
import os

# Login Setup
hostName = "localhost"                             # TG server hostname
userName = "tigergraph"                                 # TG user name
passWord = "tigergraph"                              # TG password

loadGraph = True
loadCache = True
loadFPGA  = True

populationSize = 1000                               # Size of the total patient population
topK = 10                                           # Number of highest scoring patient matches
numDevices = 1                                      # Number of FPGA devices to distribute the queries to

# Path Setup
notebookLocation = Path(os.getcwd() + "/..")
queryFileLocation = notebookLocation / "query"

serverInstallLocation = PurePosixPath("/opt/xilinx/apps/graphanalytics/integration/Tigergraph-3.x/1.1/examples/synthea")
serverDataLocation = serverInstallLocation / "1000_patients/csv"

# Utility Methods
def getPatient(id):
    patientList = conn.getVerticesById('patients', id)
    return [] if len(patientList) == 0 else patientList[0]

def getPatientName(patient):
    return patient['attributes']['FIRST_NAME'] + ' ' + patient['attributes']['LAST_NAME']

def printResults(result, newPatient):
    matches = result[0]['Matches']
    print(f'Matches for patient {getPatientName(newPatient)}')
    for m in matches:
        matchingPatient = getPatient(m['Id'])
        print(f'{m["score"]} {getPatientName(matchingPatient)}')

if __name__ == '__main__':

	if loadGraph:
		# Create New Graph
		# connect to TG server and create graph
		graphName = f'xgraph_{userName}_{populationSize}'   # TG graph name
		conn = tg.TigerGraphConnection(host='http://' + hostName, graphname='', username=userName, password=passWord, useCert=False)
		print("\n--------- Creating New graph ----------")
		print(conn.gsql(f'create graph {graphName}()', options=[]))

		# connect to TG server with new graph
		print(f'Using graph {graphName}')
		conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord, useCert=False)
		
		# Create Graph Schema
		print("\n--------- Creating New Schema ----------")
		schemaFile = queryFileLocation / "schema_xgraph.gsql"

		with open(schemaFile) as fh:
			qStrRaw = fh.read()
			qStr = qStrRaw.replace('@graph', graphName)
			print(conn.gsql(qStr))
			
		
		# Load graph data
		print("\n--------- Loading data into graph ----------")
		loadFile = queryFileLocation / "load_xgraph.gsql"

		with open(loadFile) as fh:
			qStrRaw = fh.read()
			qStrRaw = qStrRaw.replace('@graph', graphName)
			qStr    = qStrRaw.replace('$sys.data_root', str(serverDataLocation))
			print(conn.gsql(qStr, options=[]))
			print(conn.gsql(f"USE GRAPH {graphName}\n RUN LOADING JOB load_xgraph"))
			print(conn.gsql(f"USE GRAPH {graphName}\n DROP JOB load_xgraph"))
			
		
		# Install Queries
		print("\n--------- Installing Queries ----------")
		baseQFile = queryFileLocation / "base.gsql"
		clientQFile = queryFileLocation / "client.gsql"

		with open(baseQFile) as bfh, open(clientQFile) as cfh:
			print("installing base queries ...")
			qStrRaw = bfh.read()
			qStr = qStrRaw.replace('@graph', graphName)
			print(conn.gsql(qStr))
			
			print("\ninstalling client queries ...")
			qStrRaw = cfh.read()
			qStr = qStrRaw.replace('@graph', graphName)
			print(conn.gsql(qStr))
		
	else:
		# connect to TG server with existing graph
		print(f'Using graph {graphName}')
		conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord, gsqlVersion='3.1.1')
		print(f'Found graph {graphName}')
	
	
	# Create Embeddings
	if loadCache or loadGraph:
		print('Creating patient embeddings and storing them in patient vertices...')
		tStart = time.perf_counter()
		conn.runInstalledQuery('client_cosinesim_embed_vectors', timeout=240000000)
		conn.runInstalledQuery('client_cosinesim_embed_normals', timeout=240000000)
		print(f'completed in {time.perf_counter() - tStart:.4f} sec')
		

	# Send embeddings to FPGA
	if loadFPGA or loadGraph:
		print('Loading data into FPGA memory...')
		conn.runInstalledQuery('client_cosinesim_set_num_devices', {'numDevices': numDevices}, timeout=240000000)
		tStart = time.perf_counter()
		resultHwLoad = conn.runInstalledQuery('client_cosinesim_load_alveo', timeout=240000000)
		print(f'completed in {time.perf_counter() - tStart:.4f} sec\n')
	
	
	# Check status
	status = conn.runInstalledQuery('client_cosinesim_get_alveo_status', timeout=240000000)
	isInit = status[0]["IsInitialized"]
	numDev = status[0]["NumDevices"]
	print(f'FPGA Init: {isInit}, Dev: {numDev}\n')
	
	# Compute Cosine Similarity
	print('Running Query...')
	# pick a random patient out of 100
	targetPatients = conn.getVertices('patients', limit=100)
	targetPatient = targetPatients[rand.randint(0,99)]

	# run similarity on the choosen patient
	resultHW = conn.runInstalledQuery('client_cosinesim_match_alveo',
									  {'newPatient': targetPatient['v_id'], 'topK': topK}, timeout=240000000)
									  
	printResults(resultHW, targetPatient)
	resHWTime = resultHW[0]["ExecTimeInMs"]
	print(f'\nQuery completed in {resHWTime:.2f} msec')
