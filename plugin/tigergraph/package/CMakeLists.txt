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

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
# Linking variables
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")
set (PROJ_FILE_PATH "${CMAKE_SOURCE_DIR}/../staging/")

install(DIRECTORY "${PROJ_FILE_PATH}/examples" DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install(DIRECTORY "${PROJ_FILE_PATH}/udf" DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install(DIRECTORY "${PROJ_FILE_PATH}/jupyter-demo" DESTINATION ${CMAKE_INSTALL_PREFIX}/)
install(FILES "${PROJ_FILE_PATH}/install-plugin.sh" DESTINATION ${CMAKE_INSTALL_PREFIX}/)

set(CPACK_PACKAGE_NAME "xilinx-recomengine-tigergraph")
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


set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpmpreinst")
set(CPACK_PACKAGE_CONTACT "Xilinx Recommendation Engine Development Team")
set(CPACK_PACKAGE_VENDOR "Xilinx")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Xilinx Recommendation Engine for tigergraph plugin ")
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/opt" "/opt/xilinx" "/usr/lib64" "/opt/xilinx/apps" "/opt/xilinx/apps/graphanalytics")
include(CPack)
