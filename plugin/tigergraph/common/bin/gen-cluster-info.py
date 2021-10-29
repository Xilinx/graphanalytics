#!/usr/bin/env python3

# -*- coding: utf-8 -*-
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

import subprocess
import json
import sys
import re
import os
import socket
from pathlib import Path

pluginConfigFile = Path(sys.argv[1])
tgDataRoot = sys.argv[2]
deviceName = sys.argv[3]

# get IP addresses from TG configuration
command = 'gadmin config dump'
p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
p_out = p.stdout.read().decode('utf-8')
p_returncode = p.wait()
if p_returncode != 0:
    print(p_out)

nodeIps = []
config_dict = json.loads(p_out)
print('INFO: Current cluster node IP configuration:')
for h in config_dict['System']['HostList']:
    nodeIps.append(h['Hostname'])

# Get current node's hostname and IP 
# A host can have multiple IPs depends network configuation
# Get all IPs assigned to the current node and pick the one used by TigerGraph
re_inet = re.compile('inet (\S+)')
command = '/usr/sbin/ifconfig'
p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
p_out = p.stdout.readlines()
p_returncode = p.wait()
if p_returncode != 0:
    print(p_out)

for line in p_out:
    lineStr = line.decode('utf-8')
    m = re_inet.findall(lineStr)
    if m and m[0] in nodeIps:
        curNodeIp = m[0]

curNodeHostname = socket.gethostname()
print('DEBUG: curNodeHostname=', curNodeHostname, 'curNodeIp=', curNodeIp)
print('DEBUG: nodeIps', nodeIps)

# Get number of U50 devices
print('DEBUG: Searching device', deviceName)
re_u50 = re.compile(deviceName)
command = '/opt/xilinx/xrt/bin/xbutil scan'
p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
p_out = p.stdout.readlines()
p_returncode = p.wait()
if p_returncode != 0:
    print(p_out)

numDevices = 0
for line in p_out:
    lineStr = line.decode('utf-8')
    m = re_u50.findall(lineStr)
    if m:
        numDevices += 1

curNodeHostname = socket.gethostname()
print('DEBUG: numDevices=', numDevices)

pluginConfigDict = {'curNodeHostname': curNodeHostname,
                    'curNodeIp'  : curNodeIp,
                    'nodeIps'    : ' '.join(nodeIps),
                    'numNodes'   : len(nodeIps),
                    'deviceName' : deviceName,
                    'numDevices' : numDevices,
                    'xGraphStore': tgDataRoot + '/xgstore'}

with pluginConfigFile.open(mode='w') as fh:
    json.dump(pluginConfigDict, fh, indent=4)

