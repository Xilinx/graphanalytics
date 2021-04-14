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
project(xilinx-cosinesim)

# Linking variables
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

set(CS_FILE_PATH "${CMAKE_SOURCE_DIR}/../staging/")
install(DIRECTORY "${CS_FILE_PATH}/lib/" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/lib)

install(FILES "${CS_FILE_PATH}/xclbin/cosinesim_32bit_xilinx_u50_gen3x16_xdma_201920_3.xclbin" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/xclbin)
install(FILES "${CS_FILE_PATH}/include/cosinesim.hpp" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/include)

install(FILES "${CS_FILE_PATH}/src/cosinesim_loader.cpp" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim/src)
install(FILES "${CS_FILE_PATH}/examples.zip" DESTINATION ${CMAKE_INSTALL_PREFIX}/cosinesim)


#version 

set(CPACK_PACKAGE_NAME "xilinx-cosinesim")
set(PROJ_VERSION_RELEASE "1.0.0")
# --- LSB Release ---
find_program(LSB_RELEASE lsb_release)
find_program(UNAME uname)

execute_process(
    COMMAND ${LSB_RELEASE} -is
    OUTPUT_VARIABLE LINUX_FLAVOR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${LSB_RELEASE} -rs
    OUTPUT_VARIABLE CPACK_REL_VER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${UNAME} -r
    OUTPUT_VARIABLE LINUX_KERNEL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND ${UNAME} -m
    OUTPUT_VARIABLE CPACK_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (${LINUX_FLAVOR} MATCHES "^(Ubuntu|Debian)")
    set(CPACK_GENERATOR "DEB")
    set(PACKAGE_KIND "DEB")


elseif (${LINUX_FLAVOR} MATCHES "^(RedHat|CentOS|Amazon)")
    set(CPACK_GENERATOR "RPM")
    set(PACKAGE_KIND "RPM")
 

endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJ_VERSION_RELEASE}_${CPACK_REL_VER}-${CPACK_ARCH}")

message("-- ${CMAKE_BUILD_TYPE} ${PACKAGE_KIND} package")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "xrt (>= 2.8.0),xrm (>= 1.2.0),python3 (>= 3.5.0)")

set(CPACK_RPM_PACKAGE_DEPENDS "xrt (>= 2.8.0), xrm (>= 1.2.0),python3 (>= 3.5)")
set(CPACK_PACKAGE_NAME "xilinx-cosinesim")
#set(CPACK_PACKAGE_VERSION "1.0.0_18.04-x86_64")
set(CPACK_PACKAGE_CONTACT "Xilinx Cosinesim Development Team")
set(CPACK_PACKAGE_VENDOR "Xilinx")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Xilinx Cosinesim Acceleration")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/opt" "/opt/xilinx" "/usr/lib64" "/opt/xilinx/apps" "/opt/xilinx/apps/graphanalytics"  )
include(CPack)
