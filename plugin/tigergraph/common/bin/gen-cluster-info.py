#!/usr/bin/env python3

# -*- coding: utf-8 -*-

import subprocess
import json
import sys
import re
import os
import socket
from pathlib import Path

pluginConfigFile = Path(sys.argv[1])

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
command = 'ifconfig'
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
print('curNodeHostname=', curNodeHostname, 'curNodeIp=', curNodeIp)
print('nodeIps', nodeIps)

pluginConfigDict = {'curNodeHostname': curNodeHostname,
                    'curNodeIp': curNodeIp,
                    'nodeIps': ' '.join(nodeIps)}

with pluginConfigFile.open(mode='w') as fh:
    json.dump(pluginConfigDict, fh, indent=4)

