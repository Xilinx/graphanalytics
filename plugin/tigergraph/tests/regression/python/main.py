# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

# import modules
import os
import pyTigerGraph as tg
import subprocess as sp
import random as rand
import concurrent.futures as cf
import threading as th

# Setup
hostName = "xsjfislx14"
userName = "sachink"
passWord = "Xilinx123"
graphName = "xgraph_sachink_1K"
initScript = "./init_graph.sh"
populationSize = 1000
dataLocation = "/proj/gdba/datasets/synthea"
scriptLocation = "../../cosine_nbor_ss_dense_int"
pwd = "../regression/python"
doInit = True

# special variable
debug = False       # runs small number of tests and turns on debug messages
doFailTest = False  # tests if regression can detect a fail properly if one occurs

# connect to TG server
conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord)


def getPatient(id):
    patientList = conn.getVerticesById('patients', id)
    return [] if len(patientList) == 0 else patientList[0];


def getPatientName(patient):
    return patient['attributes']['FIRST_NAME'] + ' ' + patient['attributes']['LAST_NAME'];


def printResults(result, newPatient):
    matches = result[0]['Matches']
    print(f'Matches for patient {getPatientName(newPatient)}')
    for m in matches:
        matchingPatient = getPatient(m['Id'])
        print(f'{m["score"]} {getPatientName(matchingPatient)}')


def runQuery(patients, pIdx, thId):
    if debug:
        for p in patients:
            print(f'{p["v_id"]} {getPatientName(p)}')

    newPatient = patients[pIdx]

    if debug: print('\nSW query...')
    resultSw = conn.runInstalledQuery('client_cosinesim_match_sw', {'newPatient': newPatient['v_id'], 'topK': 10})
    if debug: printResults(resultSw, newPatient)

    if doFailTest: newPatient = patients[(pIdx + 1) % len(patients)]  # deliberately fail the test

    if debug: print('\nHW query...')
    resultHw = conn.runInstalledQuery('client_cosinesim_match_alveo',
                                      {'newPatient': newPatient['v_id'], 'topK': 10, 'numDevices': 1})
    if debug: printResults(resultHw, newPatient)

    swTime = resultSw[0]["ExecTimeInMs"]
    hwTime = resultHw[0]["ExecTimeInMs"]
    if debug: print(f'Client {thId} Query itr {pIdx}, SW: {swTime:.2f} ms, HW: {hwTime:.2f} ms, Speedup {swTime / hwTime:.2f}X')

    return resultSw, resultHw


def checkResult(resSw, resHw, thId):
    if resSw == resHw:
        return True
    else:
        print(f'\nClient {thId}: Results DO NOT match!')
        return False


def repeatedQueries(thId):
    # thId = th.current_thread().ident
    # send repeated queries
    numTargetPatients = rand.randint(2, 100)
    if debug: numTargetPatients = 1
    print(f'Client {thId}: running Queries for {numTargetPatients} Patients')
    targetPatients = conn.getVertices('patients', limit=numTargetPatients)  # get patients

    for itr in range(numTargetPatients):
        # run SW and HW queries
        (resultSw, resultHw) = runQuery(targetPatients, itr, thId)

        # check results
        checkFlag = checkResult(resultSw[0]['Matches'], resultHw[0]['Matches'], thId)
        if not checkFlag:
            raise Exception(f'Client {thId}: Results failed to match for Query iteration {itr}')

    print(f'\nClient {thId}: All results match!')


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    # initialize the graph, load queries,
    # cache SW vectors in vertices and Load FPGA memories
    # should be needed only once
    if doInit:
        database = dataLocation + "/" + str(populationSize) + "_patients/csv"
        cmd = [initScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + dataLocation]
        os.chdir(scriptLocation)
        sp.run(cmd)
        os.chdir(pwd)

        print('SW Caching...')
        resultSwCache = conn.runInstalledQuery('client_cosinesim_load_cache')

        print('Hw data load...\n')
        resultHwLoad = conn.runInstalledQuery('client_cosinesim_load_alveo', {'numDevices': 1})

    # Send repeated queries from different client threads
    numClients = rand.randint(1, 20)
    if debug: numClients = 1
    with cf.ThreadPoolExecutor(numClients) as client:
        for thId in range(numClients):
            client.submit(repeatedQueries, thId)

    # print (conn.echo())

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
