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

import sys
import random

in_file = sys.argv[1]

out_file_ref = "ref-names.csv"
out_file_new = "new-names.csv"
out_fh_ref = open(out_file_ref, 'w')
out_fh_new = open(out_file_new, 'w')

ref_cnt = 0
new_cnt = 0

print('Id,Name', file=out_fh_ref)
print('Id,Name', file=out_fh_new)
with open(in_file) as in_fh:
    for line in in_fh:
        name = line.split()[0]
        if len(name) > 11:
            print(str(ref_cnt) + ',' + name, file=out_fh_ref)
            ref_cnt += 1
            # save 1/3 to new names with 1 to 3 mod
            write_new = random.randint(1,3)
            chars_new = random.randint(1,3)
            if write_new == 1:
                char_list = list(name)
                for i in range(chars_new):
                    idx = random.randint(0,len(name)-1)
                    if char_list[idx] == 'Z':
                        char_list[idx] = 'A'
                    else:
                        char_list[idx] = chr(ord(char_list[idx]) + 1)
                new_name = "".join(char_list)
                #print('new-name=', new_name)
                print(str(new_cnt) + ',' + new_name, file=out_fh_new)
                new_cnt += 1
            
out_fh_ref.close()
out_fh_new.close()

print('INFO:', ref_cnt, 'names saved to', out_file_ref)
print('INFO:', new_cnt, 'names saved to', out_file_new)
