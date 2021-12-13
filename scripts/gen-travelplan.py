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

# command line examples:
# 100 travelplans, 10 trucks, 300 work orders
# ./gen-travelplan.py 100 10 300

import sys
import random

total_travelplans = int(sys.argv[1])
total_trucks = int(sys.argv[2])
total_workorders = int(sys.argv[3])
out_file_tp2tr = "travelplan2trucks" + str(total_travelplans) + ".csv"
out_file_tp2wo = "travelplan2workorders" + str(total_travelplans) + ".csv"
out_fh_tp2tr = open(out_file_tp2tr, 'w')
out_fh_tp2wo = open(out_file_tp2wo, 'w')

# generate travelplan to truck map
print("TravelPlan,Truck", file=out_fh_tp2tr)
# The travelpaln to trum mapping is N:1
# The same truck can be mapped to multiple travelplans 
for i in range(total_travelplans):
    # Randomly pick a truck for each travelplan
    truck_sel = random.randint(0,total_trucks-1)
    print('tp' + str(i) + ',' + 'tr' + str(truck_sel), file=out_fh_tp2tr)
    print('tp' + str(i) + ',' + 'tr' + str(truck_sel))

# generate travelplan to work order map
print("TravelPlan,WorkOrder", file=out_fh_tp2wo)
# The travelplan and work order mapping is N:M
# Same travelplan can be mapped to multiple work orders. 
# Same work order can can be mapped to multiple travelplans. 
total_tr2wo = 0
for i in range(total_travelplans):
    # limit up to 6 workorders to the same travelplan for now
    m_workorders = random.randint(1,6)
    for m in range(m_workorders):
        # randomly pick a work order
        workorder_sel = random.randint(0,total_workorders-1)
        print('tp' + str(i) + ',' + 'wo' + str(workorder_sel), file=out_fh_tp2wo)
        print('tp' + str(i) + ',' + 'wo' + str(workorder_sel))
        total_tr2wo += 1

out_fh_tp2tr.close()
out_fh_tp2wo.close()

print('INFO:', total_travelplans, 'rows saved to travelplan-truck mapping file', out_file_tp2tr)
print('INFO:', total_tr2wo, 'rows saved to travelplan-workorder mapping file', out_file_tp2wo)
