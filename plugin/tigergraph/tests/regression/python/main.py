# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import pyTigerGraph as tg
import subprocess as sp
import os
import random as rand
import sys


def getPatient(conn, id):
    patientList = conn.getVerticesById('patients', id)
    return [] if len(patientList) == 0 else patientList[0];


def getPatientName(patient):
    return patient['attributes']['FIRST_NAME'] + ' ' + patient['attributes']['LAST_NAME'];


def printResults(conn, result, newPatient):
    matches = result[0]['Matches']
    print(f'Matches for patient {getPatientName(newPatient)}')
    for m in matches:
        matchingPatient = getPatient(conn, m['Id'])
        print(f'{m["score"]} {getPatientName(matchingPatient)}')


def runQuery(conn, patients, p_idx, debug, doFailTest):
    if debug:
        for p in patients:
            print(f'{p["v_id"]} {getPatientName(p)}')

    newPatient = patients[p_idx]

    if debug: print('\nSW query...')
    resultSw = conn.runInstalledQuery('client_cosinesim_match_sw', {'newPatient': newPatient['v_id'], 'topK': 10})
    if debug: printResults(conn, resultSw, newPatient)

    if doFailTest: newPatient = patients[(p_idx + 1) % len(patients)]  # deliberately fail the test

    if debug: print('\nHW query...')
    resultHw = conn.runInstalledQuery('client_cosinesim_match_alveo',
                                      {'newPatient': newPatient['v_id'], 'topK': 10, 'numDevices': 1})
    if debug: printResults(conn, resultHw, newPatient)

    swTime = resultSw[0]["ExecTimeInMs"]
    hwTime = resultHw[0]["ExecTimeInMs"]
    print(f'SW: {swTime:.2f} ms, HW: {hwTime:.2f} ms, Speedup {swTime / hwTime:.2f}X')

    return resultSw, resultHw


def checkResult(resSw, resHw):
    if resSw == resHw:
        return True
    else:
        print('\nResults DO NOT match!')
        return False


# Press the green button in the gutter to run the script.
if __name__ == '__main__':

    # Setup
    hostName = "xsjfislx14"
    userName = "sachink"
    passWord = "Xilinx123"
    graphName = "xgraph_sachink_1K"
    initScript = "./init_graph.sh"
    dataLocation = "/proj/gdba/datasets/synthea/1000_patients/csv"
    scriptLocation = "../../cosine_nbor_ss_dense_int"
    pwd = "../regression/python"
    doInit = False

    # special variable
    debug = False       # runs small number of tests and turns on debug messages
    doFailTest = False  # tests if regression can detect a fail properly if one occurs

    # connect to TG server
    conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord)

    # initialize the graph, load queries,
    # cache SW vectors in vertices and Load FPGA memories
    # should be needed only once
    if doInit:
        cmd = [initScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + dataLocation]
        os.chdir(scriptLocation)
        sp.run(cmd)
        os.chdir(pwd)

        print('SW Caching...')
        resultSw = conn.runInstalledQuery('client_cosinesim_load_cache')

        print('Hw data load...\n')
        resultLoad = conn.runInstalledQuery('client_cosinesim_load_alveo', {'numDevices': 1})

    # send repeated queries
    numTargetPatients = rand.randint(2, 10)
    if debug: numTargetPatients = 1
    print(f'Running Queries for {numTargetPatients} Patients')
    targetPatients = conn.getVertices('patients', limit=numTargetPatients)  # get patients

    for itr in range(numTargetPatients):
        print(f'\nQuery iteration {itr}...')

        # run SW and HW queries
        (resultSw, resultHw) = runQuery(conn, targetPatients, itr, debug, doFailTest)

        # check results
        checkFlag = checkResult(resultSw[0]['Matches'], resultHw[0]['Matches'])
        if not checkFlag:
            raise Exception(f'Results failed to match for Query iteration {itr}')

    print(f'\nAll results match!')
    # print (conn.echo())

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
