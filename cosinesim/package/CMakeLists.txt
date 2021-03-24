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
project(cosinesim)

# Linking variables
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

set(AML_FILE_PATH "${CMAKE_SOURCE_DIR}/../staging/") 
install(FILES "${AML_FILE_PATH}/lib/libXilinxCosineSim_loader.a" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/lib)
install(FILES "${AML_FILE_PATH}/lib/libXilinxCosineSim.so" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/lib)

install(FILES "${AML_FILE_PATH}/xclbin/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/xclbin)
install(FILES "${AML_FILE_PATH}/include/cosinesim.hpp" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/include)

install(FILES "${AML_FILE_PATH}/src/cosinesim_loader.cpp" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/src)
install(FILES "${AML_FILE_PATH}/examples.zip" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim)


set(CPACK_PACKAGE_NAME "cosinesim")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "Xilinx Cosinesim Development Team")
set(CPACK_PACKAGE_VENDOR "Xilinx")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Xilinx Cosinesim Acceleration")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/opt" "/opt/xilinx" "/usr/lib64" "/opt/xilinx/apps" "/opt/xilinx/apps/graphanalytics"  )
include(CPack)
