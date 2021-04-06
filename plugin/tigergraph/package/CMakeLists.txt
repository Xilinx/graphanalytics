# Copyright (C) 2021 Xilinx, Inc
#
# Licensed under the Apache License, Version 2.0 (the "License"). You may
# not use this file except in compliance with the License. A copy of the
# License is located at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

cmake_minimum_required(VERSION 2.8.11)
project(xilinx-recomengine-tigergraph)

# Linking variables
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")
set(PROJ_FILE_PATH "${CMAKE_SOURCE_DIR}/../staging/")

install(DIRECTORY "${PROJ_FILE_PATH}/examples" DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install(DIRECTORY "${PROJ_FILE_PATH}/QueryUdf" DESTINATION ${CMAKE_INSTALL_PREFIX}/)


set(CPACK_PACKAGE_NAME "xilinx-recomengine-tigergraph")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "Xilinx Recommendation Engine Development Team")
set(CPACK_PACKAGE_VENDOR "Xilinx")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Xilinx Recommendation Engine for tigergraph plugin ")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/opt" "/opt/xilinx" "/usr/lib64" "/opt/xilinx/apps" "/opt/xilinx/apps/graphanalytics"  )
include(CPack)
