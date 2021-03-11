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
import threading as th

# Setup
hostName = "xsjfislx14"                             # TG server hostname
userName = "sachink"                                # TG user name
passWord = "Xilinx123"                              # TG user password
graphName = "xgraph_sachink_1K"                     # TG graph name
initScript = "./init_graph.sh"                      # initialization script: loads graphs, installs queries and caches SW and HW data
dataLocation = "/proj/gdba/datasets/synthea"        # location of synthea generated data
scriptLocation = "../../cosine_nbor_ss_dense_int"   # location of initialization script
pwd = "../regression/python"
populationSize = 1000                               # size of the total patient population data
topK = 10                                           # number of highest scoring patient matches
numDevices = 1                                      # number of FPGA devices to distribute the queries to
doInit = True                                       # used to turn initialization on or off, can be set to false once data is loaded

# other variables
debug = False                                       # runs small number of tests and turns on debug messages
doFailTest = False                                  # tests if regression can detect a fail properly if one occurs

# connect to TG server
conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord)


def getPatient(id):
    patientList = conn.getVerticesById('patients', id)
    return [] if len(patientList) == 0 else patientList[0]


def getPatientName(patient):
    return patient['attributes']['FIRST_NAME'] + ' ' + patient['attributes']['LAST_NAME']


def runQuery(patients, pIdx, thId):
    if debug:
        for p in patients:
            print(f'{p["v_id"]} {getPatientName(p)}')

    newPatient = patients[pIdx]
    numMatches = topK + 10

    if debug: print('\nSW query...')
    resultSw = conn.runInstalledQuery('client_cosinesim_match_sw', {'newPatient': newPatient['v_id'], 'topK': numMatches})
    if debug: printResults(resultSw, newPatient)

    if doFailTest: newPatient = patients[(pIdx + 1) % len(patients)]  # deliberately fail the test

    if debug: print('\nHW query...')
    resultHw = conn.runInstalledQuery('client_cosinesim_match_alveo',
                                      {'newPatient': newPatient['v_id'], 'topK': numMatches, 'numDevices': numDevices})
    if debug: printResults(resultHw, newPatient)

    swTime = resultSw[0]["ExecTimeInMs"]
    hwTime = resultHw[0]["ExecTimeInMs"]
    if debug: print(f'\nClient {thId} Query itr {pIdx}, SW: {swTime:.2f} ms, HW: {hwTime:.2f} ms, Speedup {swTime / hwTime:.2f}X')

    return resultSw, resultHw


def checkResult(resSw, resHw):
    if resSw == resHw:
        return True
    else:
        return False


def checkResultLowPrecision(resSw, resHw):
    swPairs = []
    hwPairs = []
    for i in range(len(resSw)):
        swPairs.append(f"{str(resSw[i]['score'])[:4]}{resSw[i]['Id']}")
        hwPairs.append(f"{str(resHw[i]['score'])[:4]}{resHw[i]['Id']}")
    swPairs.sort(reverse=True)
    hwPairs.sort(reverse=True)
    if swPairs[:topK] == hwPairs[:topK]:
        return True
    else:
        return False


def printResults(result, newPatient):
    matches = result[0]['Matches']
    print(f'Matches for patient {getPatientName(newPatient)}')
    for m in matches:
        matchingPatient = getPatient(m['Id'])
        print(f'{m["score"]} {getPatientName(matchingPatient)}')


def repeatedQueries(thId):
    # thId = th.current_thread().ident
    # send repeated queries
    numTargetPatients = rand.randint(2, 100)
    if debug: numTargetPatients = 50
    print(f'Client {thId}: running Queries for {numTargetPatients} Patients')
    targetPatients = conn.getVertices('patients', limit=numTargetPatients)  # get patients
    if not targetPatients:
        print('Error: Target Patient list empty', file=sys.stderr)
        exit()

    checkFlag = True
    swTime = 0
    hwTime = 0
    for itr in range(numTargetPatients):
        # run SW and HW queries
        (resultSw, resultHw) = runQuery(targetPatients, itr, thId)
        swTime += resultSw[0]["ExecTimeInMs"]
        hwTime += resultHw[0]["ExecTimeInMs"]

        # check results
        checkFlag = checkResultLowPrecision(resultSw[0]['Matches'], resultHw[0]['Matches'])
        if not checkFlag:
            print(f'Client {thId}: Results failed to match for {itr}th Patient Query', file=sys.stderr)
            break

    if checkFlag: print(f'Client {thId}: All results match. SW: {swTime:.2f} ms, HW: {hwTime:.2f} ms, Speedup {swTime / hwTime:.2f}X')


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    print(f'Data size: Total {populationSize} Patients')
    # initialize the graph, load queries,
    # cache SW vectors in vertices and Load FPGA memories
    # should be needed only once
    if doInit:
        database = dataLocation + "/" + str(populationSize) + "_patients/csv"
        cmd = [initScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + database]
        os.chdir(scriptLocation)
        sp.run(cmd)
        os.chdir(pwd)

        print('SW Caching...')
        resultSwCache = conn.runInstalledQuery('client_cosinesim_load_cache')

        print('Hw data load...\n')
        resultHwLoad = conn.runInstalledQuery('client_cosinesim_load_alveo', {'numDevices': numDevices})

    # Send repeated queries from different client threads
    numClients = rand.randint(1, 20)
    if debug: repeatedQueries(0)
    else:
        print(f'Spawned {numClients} Clients\n')
        with cf.ThreadPoolExecutor(numClients) as client:
            for thId in range(numClients):
                client.submit(repeatedQueries, thId)
            print()

    # print (conn.echo())

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
