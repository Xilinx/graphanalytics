# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import pyTigerGraph as tg
import subprocess as sp
import os


def getPatient(conn, id):
    patientList = conn.getVerticesById('patients', id)
    return [] if len(patientList) == 0 else patientList[0];


def getPatientName(patient):
    return patient['attributes']['FIRST_NAME'] + ' ' + patient['attributes']['LAST_NAME'];


def printResults(conn, result):
    matches = result[0]['Matches']
    print(f'Query time: {result[0]["ExecTimeInMs"]} ms')
    print(f'Matches for patient {getPatientName(newPatient)}')
    for m in matches:
        matchingPatient = getPatient(conn, m['Id'])
        print(f'{m["score"]} {getPatientName(matchingPatient)}')


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
    doInit = True

    # initialize the graph
    if doInit:
        cmd = [initScript, "-u " + userName, "-p " + passWord, "-g " + graphName, "-s " + dataLocation]
        os.chdir(scriptLocation)
        sp.run(cmd)
        os.chdir(pwd)

    conn = tg.TigerGraphConnection(host='http://' + hostName, graphname=graphName, username=userName, password=passWord)
    patients = conn.getVertices('patients', limit=10)
    for p in patients:
        print(f'{p["v_id"]} {getPatientName(p)}')
    newPatient = patients[0]
    print()
    print('SW Caching...')
    resultSw = conn.runInstalledQuery('client_cosinesim_load_cache')
    print()
    print('SW query...')
    resultSw = conn.runInstalledQuery('client_cosinesim_match_sw', {'newPatient': newPatient['v_id'], 'topK': 10})
    printResults(conn, resultSw)
    print()
    print('Hw data load...')
    resultLoad = conn.runInstalledQuery('client_cosinesim_load_alveo', {'numDevices': 1})
    print()
    print('HW query...')
    resultHw = conn.runInstalledQuery('client_cosinesim_match_alveo', {'newPatient': newPatient['v_id'], 'topK': 10, 'numDevices': 1})
    printResults(conn, resultHw)

    print()
    if resultSw[0]['Matches'] == resultHw[0]['Matches']:
        print('Results match!')
    else:
        print('Results DO NOT match!')

    # print (conn.echo())

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
