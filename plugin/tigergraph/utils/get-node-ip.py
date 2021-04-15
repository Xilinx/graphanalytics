#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os.path import expanduser
from pathlib import Path
import re
import sys

nodeID = sys.argv[1]
tigergraphHome = expanduser("~tigergraph")

tigergraphConfig = Path(tigergraphHome) / ".tg.cfg"
if tigergraphConfig.exists():
    with tigergraphConfig.open() as fp:
        tigergraphConfigJson = json.load(fp)
else:
    print('ERROR: Tigergraph Enterprise 3.x or above is not installed on this node.')
    exit(1)

for host in tigergraphConfigJson['System']['HostList']:
    if host['ID'] == nodeID:
        print(host['Hostname'])


