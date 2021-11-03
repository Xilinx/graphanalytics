 Copyright (C) 2021 Xilinx, Inc

 Licensed under the Apache License, Version 2.0 (the "License"). You may
 not use this file except in compliance with the License. A copy of the
 License is located at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 License for the specific language governing permissions and limitations
 under the License.


# Xilinx FuzzyMatch Product
This product provides fuzzymatch acceleration solution capable of calculating similarity between two strings by making use of the Levenshtein edit distance between them. 

## Kernel Feature and Limit
* Run fuzzy match on input string against list of target pattern strings
* Maximum length of blacklist/query string: 35 
* If similarity is larger or equal 90%, return Match

## Folder Layout

* `xclbin`: FPGA binary
* `example/cpp`:  C++ API example test code
* `example/python`: python example test code  
* `example/data`: Input data for example
* `lib`: The host code library that allows communication with FPGA
* `include`: C++ header for using the host code library

## Run Example Tests Instructions

 Start and login to your F1 instance.
 In your AWS F1 terminal, source the XRT configuration script

```
source /opt/xilinx/xrt/setup.sh
```
 check out the C++ and Python API examples

```
* C++ API example code. Run `make help` for instructions
```
cd example/cpp
````
* Python API example code. Check example/python/README.md for instructions
```
cd example/python
```
