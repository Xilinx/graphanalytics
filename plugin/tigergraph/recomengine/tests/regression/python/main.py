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


# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

# import modules
import sys
import os
import pyTigerGraph as tg
import subprocess as sp
import random as rand
import concurrent.futures as cf
import time
import threading as th
import argparse
import getpass


# Setup
scriptPath = os.path.dirname(os.path.realpath(__file__))
repoRootRelative = scriptPath + '../../../../..'
repoRoot =  os.path.dirname(os.path.realpath(repoRootRelative))
print(scriptPath, repoRoot)

# Initialization script: loads graphs, installs queries and caches SW and HW data
initGraphScript = repoRoot + '/plugin/tigergraph/examples/synthea/init_graph.sh'
installQueryScript = repoRoot + '/plugin/tigergraph/examples/synthea/install_query.sh'

dataLocation = "/proj/gdba/datasets/synthea"        # Location of synthea generated data

# compute load variables
populationSize = 1000                               # Size of the total patient population data
topK = 10                                           # Number of highest scoring patient matches
maxQueries = 100                                    # Maximum number of repeated queries run by each client, (2, maxQueries) are chosen at random

# other variables
debug = False                                       # Runs small number of tests and turns on debug messages, ignores setup above
doFailTest = False                                  # Tests if regression can detect a fail properly if one occurs
extraK = 80                                         # Total of topK + extraK are returned by both SW and HW as order of same score patients need not be same in both
                                                    #   if mismatches occur for same patient for all Clients, try increasing this value and try again
matchPrecision = 6                                  # Number of decimal points to compare SW and HW results up to
                                                    #   if set too low, more extraK padding might be needed to avoid false mismatches

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


def checkResult(resSw, resHw):
    if resSw == resHw:
        return True
    else:
        return False


def checkResultLowPrecision(resSw, resHw):
    swPairs = []
    hwPairs = []
    for i in range(len(resSw)):
        swPairs.append(f"{str(resSw[i]['score'])[:matchPrecision]}{resSw[i]['Id']}")
        hwPairs.append(f"{str(resHw[i]['score'])[:matchPrecision]}{resHw[i]['Id']}")
    swPairs.sort(reverse=True)
    hwPairs.sort(reverse=True)
    if swPairs[:topK] == hwPairs[:topK]:
        return True
    else:
        return False


def runQuery(patients, pIdx, thId):
    global verbose

    if debug:
        for p in patients:
            print(f'{p["v_id"]} {getPatientName(p)}')

    newPatient = patients[pIdx]
    numMatches = topK + extraK

    if debug: print('\nSW query...')
    resultSw = conn.runInstalledQuery('client_cosinesim_match_sw', {'newPatient': newPatient['v_id'], 'topK': numMatches}, timeout=240000000)
    if debug: printResults(resultSw, newPatient)

    if doFailTest and (rand.random() > 0.9) : newPatient = patients[(pIdx + 1) % len(patients)]  # deliberately fail the test

    if debug: print('\nHW query... ')
    resultHw = conn.runInstalledQuery('client_cosinesim_match_alveo',
                                      {'newPatient': newPatient['v_id'], 'topK': numMatches}, timeout=240000000)
    if debug: printResults(resultHw, newPatient)

    swTime = resultSw[0]["ExecTimeInMs"]
    hwTime = resultHw[0]["ExecTimeInMs"]
    if debug: print(f'\nClient {thId} Query itr {pIdx}, SW: {swTime:.2f} ms, HW: {hwTime:.2f} ms, Speedup {swTime / hwTime:.2f}X')

    return resultSw, resultHw


def repeatedQueries(thId, num_target_patients):
    global verbose

    # thId = th.current_thread().ident
    # send repeated queries
    if num_target_patients < 0:
        numTargetPatients = rand.randint(2, maxQueries)
    else:
        numTargetPatients = num_target_patients

    if debug: 
        numTargetPatients = 4

    print(f'INFO: Client {thId}: running Queries for {numTargetPatients} patients')
    targetPatients = conn.getVertices('patients', limit=numTargetPatients)  # get patients
    if not targetPatients:
        print('ERROR: Target Patient list empty', file=sys.stderr)
        return

    checkFlag = True
    swTime = 0
    hwTime = 0
    for itr in range(numTargetPatients):
        if verbose > 3:
            print(f'INFO:    Finding match for target pateint {itr} in SW and HW')
        # run SW and HW queries
        (resultSw, resultHw) = runQuery(targetPatients, itr, thId)
        swTime += resultSw[0]["ExecTimeInMs"]
        hwTime += resultHw[0]["ExecTimeInMs"]

        # check results
        #checkFlag = checkResult(resultSw[0]['Matches'], resultHw[0]['Matches'])
        checkFlag = checkResultLowPrecision(resultSw[0]['Matches'], resultHw[0]['Matches'])
        if checkFlag:
            if verbose > 3:
                print('INFO:    SW and HW results match')
        else:
            print(f'ERROR: Client {thId}: Results failed to match for {itr}th Patient Query', file=sys.stderr)
            break
        

    if checkFlag: 
        print(f'INFO Client {thId}: All {numTargetPatients} results match.' + \
              f' SW: {swTime/numTargetPatients:.2f} ms/query,' + \
              f' HW: {hwTime/numTargetPatients:.2f} ms/query,' + \
              f' Speedup {swTime / hwTime:.2f}X')


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    global conn, userName, graphName, verbose

    parser = argparse.ArgumentParser()
    parser.add_argument('--host', dest='host', default='localhost',
                        help='Specify the hostname of TigerGraph server')
    parser.add_argument('--user', dest='user', default=None,
                        help='Specify username on TigerGraph server')
    parser.add_argument('--graph', dest='graphName', default=None,
                        help='Used to turn initialization on or off, can be set to false once data is loaded.')
    parser.add_argument('--noInitGraph', action='store_true', dest='noInitGraph', 
                        help='Used to turn off graph initialization (schema and DLM)')
    parser.add_argument('--noInstallQuery', action='store_true', dest='noInstallQuery', 
                        help='Used to turn off installing queries')
    parser.add_argument('--noLoadGraph', action='store_true', dest='noLoadGraph', 
                         help='Used to turn off loading data onto FPGAs')
    parser.add_argument('--maxClients', dest='maxClients', type=int, default=20, 
                        help='Maximum number of clients to spawn, (1, maxClients) are chosen at random')
    parser.add_argument('--numTargetPatients', dest='numTargetPatients', type=int, default=-1, 
                        help='Number of target patients')   
    parser.add_argument('--numDevices', dest='numDevices', type=int, default=1, 
                        help='Number of FPGA devices to distribute the queries to')
    parser.add_argument('--verbose', dest='verbose', type=int, default=0, 
                        help='Verbose level for messages')
    parser.add_argument('--dataSource', dest='dataSource', default=None, 
                        help='Verbose level for messages')
                             
    args = parser.parse_args()

    hostName = args.host
    if args.user is None:
        userName = getpass.getuser()
    else:
        userName = args.user
    passWord = "Xilinx123"
    if args.graphName is None:
        graphName = f'xgraph_{userName}_{populationSize}'   # TG graph name
    else:
        graphName = args.graphName
    maxClients = args.maxClients
    numTargetPatients = args.numTargetPatients
    numDevices = args.numDevices
    verbose = args.verbose
    if args.dataSource is None:
        dataSource = dataLocation + "/" + str(populationSize) + "_patients/csv"
    else:
        dataSource = args.dataSource

    print(f'INFO: Running queries as {userName}')

    # connect to TG server
    conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord)

    # initialize the graph, load queries,
    # cache SW vectors in vertices and Load FPGA memories
    # should be needed only once
    if not args.noInitGraph:
        print(f'INFO: Data size: Total {populationSize} Patients')
        print(f'INFO: Creating graph {graphName}...')
        cmd = [initGraphScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + dataSource]
        tStart = time.perf_counter()
        sp.run(cmd)
        print(f'INFO: Creating graph completed in {time.perf_counter() - tStart:.4f} sec')

    if not args.noInstallQuery:
        print(f'INFO: Installing queries...')
        cmd = [installQueryScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + dataSource]
        tStart = time.perf_counter()
        sp.run(cmd)
        print(f'INFO: Installing query completed in {time.perf_counter() - tStart:.4f} sec')

    if not args.noInitGraph:
        print('\nINFO: SW caching...')
        tStart = time.perf_counter()
        resultSwCache = conn.runInstalledQuery('client_cosinesim_embed_vectors', timeout=240000000)
        resultSwCache = conn.runInstalledQuery('client_cosinesim_embed_normals', timeout=240000000)
        print(f'INFO: SW caching completed in {time.perf_counter() - tStart:.4f} sec')


    print(f'INFO: Using graph {graphName}')

    if not args.noLoadGraph:
        # set up numDevcies
        print('\nINFO: Setting number of FPGA devices')
        result = conn.runInstalledQuery('client_cosinesim_set_num_devices', {'numDevices': numDevices})
        print('INFO: Loading graph data onto FPGA devices')
        resultHwLoad = conn.runInstalledQuery('client_cosinesim_load_alveo')
        print(f'INFO: Load graph completed in {resultHwLoad[0]["ExecTimeInMs"]/1000:.4f} sec\n')
    
    result = conn.runInstalledQuery('client_cosinesim_get_alveo_status')
    if result[0]['IsInitialized']:
        print(f'INFO: {result[0]["NumDevices"]} FPGA devices initialized')
    else:
        print('ERROR: failed to initialize FPGA devices. Exiting...')
        exit(-1)

    # Send repeated queries from different client threads
    numClients = rand.randint(1, maxClients)
    
    if debug: 
        repeatedQueries(0, 1)
    else:
        print(f'INFO: Spawned {numClients} Clients\n')
        with cf.ThreadPoolExecutor(numClients) as client:
            for thId in range(numClients):
                client.submit(repeatedQueries, thId, numTargetPatients)

    # print (conn.echo())

    print('INFO: all tests completed.\n')
